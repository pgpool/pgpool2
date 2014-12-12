/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2013	PgPool Global Development Group
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of the
 * author not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. The author makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * pool_process_query.c: query processing stuff
 *
 */
#include "config.h"
#include <errno.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif


#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <ctype.h>
#include <regex.h>

#include "pool.h"
#include "pool_config.h"
#include "rewrite/pool_timestamp.h"
#include "protocol/pool_proto_modules.h"
#include "protocol/protocol_defs.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/elog.h"
#include "utils/pool_relcache.h"
#include "utils/pool_stream.h"
#include "context/pool_session_context.h"
#include "context/pool_query_context.h"
#include "utils/pool_select_walker.h"
#include "query_cache/pool_memqcache.h"
#include "utils/pool_signal.h"

#ifndef FD_SETSIZE
#define FD_SETSIZE 512
#endif

#define ACTIVE_SQL_TRANSACTION_ERROR_CODE "25001"		/* SET TRANSACTION ISOLATION LEVEL must be called before any query */
#define DEADLOCK_ERROR_CODE "40P01"
#define SERIALIZATION_FAIL_ERROR_CODE "40001"
#define QUERY_CANCEL_ERROR_CODE "57014"
#define ADMIN_SHUTDOWN_ERROR_CODE "57P01"
#define CRASH_SHUTDOWN_ERROR_CODE "57P02"

static int reset_backend(POOL_CONNECTION_POOL *backend, int qcnt);
static char *get_insert_command_table_name(InsertStmt *node);
static int send_deallocate(POOL_CONNECTION_POOL *backend, POOL_SENT_MESSAGE_LIST msglist, int n);
static bool is_cache_empty(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
static void ParallelForwardToFrontend(char kind, POOL_CONNECTION *frontend, POOL_CONNECTION *backend, char *database, bool send_to_frontend);
static bool is_panic_or_fatal_error(const char *message, int major);
static int detect_error(POOL_CONNECTION *master, char *error_code, int major, char class, bool unread);
static int detect_postmaster_down_error(POOL_CONNECTION *master, int major);
static bool is_internal_transaction_needed(Node *node);
static bool pool_has_insert_lock(void);
static POOL_STATUS add_lock_target(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char* table);
static bool has_lock_target(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char* table, bool for_update);
static POOL_STATUS insert_oid_into_insert_lock(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char* table);
static POOL_STATUS read_packets_and_process(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, int reset_request, int *state, short *num_fields, bool *cont);
static bool is_all_slaves_command_complete(unsigned char *kind_list, int num_backends, int master);

/* timeout sec for pool_check_fd */
static int timeoutsec;

/*
 * Main module for query processing
 * reset_request: if non 0, call reset_backend to execute reset queries
 */
POOL_STATUS pool_process_query(POOL_CONNECTION *frontend,
							   POOL_CONNECTION_POOL *backend,
							   int reset_request)
{
	short num_fields = 0;	/* the number of fields in a row (V2 protocol) */
	POOL_STATUS status;
	int qcnt;
	int i;
	MemoryContext ProcessQueryContext = AllocSetContextCreate(QueryContext,
										 "pool_process_query",
										 ALLOCSET_DEFAULT_MINSIZE,
										 ALLOCSET_DEFAULT_INITSIZE,
										 ALLOCSET_DEFAULT_MAXSIZE);

	/*
	 * This variable is used while processing reset_request (i.e.:
	 * reset_request == 1).  If state is 0, then we call
	 * reset_backend. And we set state to 1 so that we wait for ready
	 * for query message from badckends.
	 */
	int state;

    frontend->no_forward = reset_request;
	qcnt = 0;
	state = 0;


	/* Try to connect memcached */
	if (pool_config->memory_cache_enabled && !pool_is_shmem_cache())
	{
		memcached_connect();
	}

	for (;;)
	{

		MemoryContextSwitchTo(ProcessQueryContext);
		MemoryContextResetAndDeleteChildren(ProcessQueryContext);

		/* Are we requested to send reset queries? */
		if (state == 0 && reset_request)
		{
			int st;

			/* send query for resetting connection such as "ROLLBACK" "RESET ALL"... */
			st = reset_backend(backend, qcnt);

			if (st < 0)		/* error? */
			{
				/* probably we don't need this, since caller will
				 * close the connection to frontend after returning with POOL_END. But I
				 * guess I would like to be a paranoid...
				 */
				frontend->no_forward = 0;
                ereport(ERROR,
                    (errmsg("unable to reset backend status")));
			}

			else if (st == 0)	/* no query issued? */
			{
				qcnt++;
				continue;
			}

			else if (st == 1)	/* more query remains */
			{
				state = 1;
				qcnt++;
				continue;
			}

			else	/* no more query(st == 2) */
			{
				for (i=0;i<NUM_BACKENDS;i++)
				{
					if (VALID_BACKEND(i))
						TSTATE(backend, i) = 'I';
				}
				frontend->no_forward = 0;
				return POOL_CONTINUE;
			}
		}

		check_stop_request();

		/*
		 * If we are in recovery and client_idle_limit_in_recovery is -1, then
		 * exit immediately.
		 */
		if (*InRecovery > RECOVERY_INIT && pool_config->client_idle_limit_in_recovery == -1)
		{
            ereport(ERROR,
                (pool_error_code("57000"),
                 errmsg("connection terminated due to online recovery"),
                     errdetail("child connection forced to terminate due to client_idle_limit = -1")));
		}

		/*
		 * If we are not processing a query, now is the time to
		 * extract retrieve pending data from buffer stack if any.
		 */
		if (!pool_is_query_in_progress())
		{
			for (i=0;i<NUM_BACKENDS;i++)
			{
				int plen;
				if (VALID_BACKEND(i) && pool_stacklen(CONNECTION(backend, i)) > 0)
					pool_pop(CONNECTION(backend, i), &plen);
			}
		}

		/*
		 * If we are prcessing query, process it.
		 */
		if (pool_is_query_in_progress())
		{
			status = ProcessBackendResponse(frontend, backend, &state, &num_fields);
			if (status != POOL_CONTINUE)
				return status;
		}

		/*
		 * If frontend and all backends do not have any pending data in
		 * the receiving data cache, then issue select(2) to wait for new
		 * data arrival
		 */
		else if (is_cache_empty(frontend, backend))
		{
			bool cont = true;
			status = read_packets_and_process(frontend, backend, reset_request, &state, &num_fields, &cont);
			if (status != POOL_CONTINUE)
				return status;
			else if (!cont)		/* Detected admin shutdown */
				return status;
		}
		else
		{
			if ((pool_ssl_pending(frontend) || !pool_read_buffer_is_empty(frontend)) &&
				!pool_is_query_in_progress())
			{
				/* We do not read anything from frontend after receiving X packet.
				 * Just emit log message. This will guard us from buggy frontend.
				 */
				if (reset_request)
				{
                    elog(LOG,"garbage data from frontend after receiving terminate message ignored");
					pool_discard_read_buffer(frontend);
				}
				else
				{
					status = ProcessFrontendResponse(frontend, backend);
					if (status != POOL_CONTINUE)
						return status;
				}
			}

			/*
			 * ProcessFrontendResponse() may start query
			 * processing. We need to re-check
			 * pool_is_query_in_progress() here.
			 */
			if (pool_is_query_in_progress())
			{
				status = ProcessBackendResponse(frontend, backend, &state, &num_fields);
				if (status != POOL_CONTINUE)
					return status;
			}
			else
			{
				/* Ok, query is not in progress.
				 * ProcessFrontendResponse() may consume all pending
				 * data.  Check if we have any pending data. If not,
				 * call read_packets_and_process() and wait for data
				 * arrival.
				 */
				if (is_cache_empty(frontend, backend))
				{
					bool cont = true;
					status = read_packets_and_process(frontend, backend, reset_request, &state, &num_fields, &cont);
					if (status != POOL_CONTINUE)
						return status;
					else if (!cont)		/* Detected admin shutdown */
						return status;
				}
				else
				{
					/* If we have pending data in master, we need to process it */
					if (pool_ssl_pending(MASTER(backend)) ||
						!pool_read_buffer_is_empty(MASTER(backend)))
					{
						status = ProcessBackendResponse(frontend, backend, &state, &num_fields);
						if (status != POOL_CONTINUE)
							return status;
					}
					else
					{
						for (i=0;i<NUM_BACKENDS;i++)
						{
							if (!VALID_BACKEND(i))
								continue;

							if (pool_ssl_pending(CONNECTION(backend, i)) ||
								!pool_read_buffer_is_empty(CONNECTION(backend, i)))
							{
								/* If we have pending data in master, we need to process it */
								if (IS_MASTER_NODE_ID(i))
								{
									status = ProcessBackendResponse(frontend, backend, &state, &num_fields);
									if (status != POOL_CONTINUE)
										return status;
									break;
								}
								else
								{
									char kind;
									int len;
									char *string;

									/* If master does not have pending
									 * data, we discard one packet from
									 * other backend */
									pool_read_with_error(CONNECTION(backend, i), &kind, sizeof(kind),
											"reading message kind from backend");

									if (kind == 'A')
									{
										if(MASTER_SLAVE)
										{
											int sendlen;
											/*
											 * In master slave mode we may send the query to the standby
											 * node and the NOTIFY comes back only from primary node. But since
											 * we have sent the query to the standby, so the current MASTER_NODE_ID
											 * will be pointing to the standby node. And we will get stuck if we
											 * keep waiting for the current master node (standby) in this case to
											 * send us the NOTIFY message.
											 * see "0000116: LISTEN Notifications Not Reliably Delivered Using JDBC4 Demonstrator"
											 * for the scenario
											 */
											pool_read_with_error(CONNECTION(backend, i), &len, sizeof(len),
																 "reading message length from backend");
											len = ntohl(len) - 4;
											string = pool_read2(CONNECTION(backend,i), len);
											if (string == NULL)
												ereport(ERROR,
													(errmsg("unable to forward NOTIFY message to frontend"),
														 errdetail("read from backend failed")));

											pool_write(frontend, &kind, 1);
											sendlen = htonl(len+4);
											pool_write(frontend, &sendlen, sizeof(sendlen));
											pool_write_and_flush(frontend, string, len);
										}
										else
										{
											/*
											 * In replication mode, NOTIFY is sent to all backends.
											 * However the order of arrival of 'Notification response'
											 * is not necessarily the master first and then slaves.
											 * So if it arrives slave first, we should try to read from master,
											 * rather than just discard it.
											 */
											pool_unread(CONNECTION(backend, i), &kind, sizeof(kind));
											ereport(LOG,
												(errmsg("pool process query"),
													 errdetail("received %c packet from backend %d. Don't dicard and read %c packet from master", kind, i, kind)));

											pool_read_with_error(CONNECTION(backend, MASTER_NODE_ID), &kind, sizeof(kind),
													"reading message kind from backend");

											pool_unread(CONNECTION(backend, MASTER_NODE_ID), &kind, sizeof(kind));
										}
									}
									else
									{
                                        ereport(LOG,
                                                (errmsg("pool process query"),
                                                    errdetail("discard %c packet from backend %d", kind, i)));


										if (MAJOR(backend) == PROTO_MAJOR_V3)
										{
											pool_read_with_error(CONNECTION(backend, i), &len, sizeof(len),
													"reading message length from backend");
											len = ntohl(len) - 4;
											string = pool_read2(CONNECTION(backend, i), len);
											if (string == NULL)
                                                ereport(FATAL,
                                                    (return_code(2),
                                                         errmsg("unable to process query"),
                                                            errdetail("error while reading rest of message from backend %d", i)));
										}
										else
										{
											string = pool_read_string(CONNECTION(backend, i), &len, 0);
											if (string == NULL)
                                                ereport(FATAL,
                                                    (return_code(2),
                                                         errmsg("unable to process query"),
                                                            errdetail("error while reading rest of message from backend %d", i)));
										}
									}
								}
							}
						}
					}
				}
			}
		}

		/* reload config file */
		if (got_sighup)
		{
            MemoryContext oldContext = MemoryContextSwitchTo(TopMemoryContext);
			pool_get_config(get_config_file_name(), RELOAD_CONFIG);
            MemoryContextSwitchTo(oldContext);
			if (pool_config->enable_pool_hba)
				load_hba(get_hba_file_name());
			if (pool_config->parallel_mode)
				pool_memset_system_db_info(system_db_info->info);
			got_sighup = 0;
		}
	}
}

/*
 * This function transmits to a parallel Query, and does processing
 * that receives the result to each back end.
 */
POOL_STATUS pool_parallel_exec(POOL_CONNECTION *frontend,
									  POOL_CONNECTION_POOL *backend, char *string,
									  Node *node,bool send_to_frontend)
{
	int len;
	int fds;
	int i;
	char kind;
	fd_set readmask;
	fd_set writemask;
	fd_set exceptmask;
	fd_set donemask;
 	static char *sq_config = "show pool_status";
 	static char *sq_pools = "show pool_pools";
 	static char *sq_processes = "show pool_processes";
 	static char *sq_nodes = "show pool_nodes";
 	static char *sq_version = "show pool_version";
	struct timeval timeout;
	int num_fds;
	int used_count = 0;
	int error_flag = 0;
	unsigned long datacount = 0;
	POOL_SESSION_CONTEXT *session_context;

	/* Get session context */
	session_context = pool_get_session_context(false);

	pool_setall_node_to_be_sent(session_context->query_context);

	len = strlen(string) + 1;

	if (is_drop_database(node))
	{
		int stime = 5;  /* XXX give arbitrary time to allow closing idle connections */
		ereport(DEBUG1,
			(errmsg("executing parallel query"),
				 errdetail("sending HUP signal to parent")));

		pool_signal_parent(SIGHUP); /* send HUP signal to parent */

		/* we need to loop over here since we will get HUP signal while sleeping */
		while (stime > 0)
			stime = sleep(stime);
	}

	/* process status reporting? */
 	if (strncasecmp(sq_config, string, strlen(sq_config)) == 0)
 	{
		ereport(DEBUG1,
			(errmsg("executing parallel query"),
				 errdetail("config reporting")));
 		config_reporting(frontend, backend);
 		pool_unset_query_in_progress();
 		return POOL_CONTINUE;
 	}
 
 	if (strncasecmp(sq_pools, string, strlen(sq_pools)) == 0)
	{
		ereport(DEBUG1,
			(errmsg("executing parallel query"),
				 errdetail("pools reporting")));
		pools_reporting(frontend, backend);
		pool_unset_query_in_progress();
		return POOL_CONTINUE;
	}

 	if (strncasecmp(sq_processes, string, strlen(sq_processes)) == 0)
	{
		ereport(DEBUG1,
			(errmsg("executing parallel query"),
				 errdetail("process reporting")));
		processes_reporting(frontend, backend);
		pool_unset_query_in_progress();
		return POOL_CONTINUE;
	}

 	if (strncasecmp(sq_nodes, string, strlen(sq_nodes)) == 0)
 	{
		ereport(DEBUG1,
			(errmsg("executing parallel query"),
				 errdetail("nodes reporting")));
 		nodes_reporting(frontend, backend);
 		pool_unset_query_in_progress();
 		return POOL_CONTINUE;
 	}
 
 	if (strncasecmp(sq_version, string, strlen(sq_version)) == 0)
 	{
		ereport(DEBUG1,
			(errmsg("executing parallel query"),
				 errdetail("version reporting")));
 		version_reporting(frontend, backend);
 		pool_unset_query_in_progress();
 		return POOL_CONTINUE;
 	}
 
	/* In this loop,forward the query to the all backends */
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		pool_write(CONNECTION(backend, i), "Q", 1);

		if (MAJOR(backend) == PROTO_MAJOR_V3)
		{
			int sendlen = htonl(len + 4);
			pool_write(CONNECTION(backend, i), &sendlen, sizeof(sendlen));
 		}

		pool_write_and_flush(CONNECTION(backend, i), string, len);

		/*
		 * in "strict mode" we need to wait for backend completing the query.
		 * note that this is not applied if "NO STRICT" is specified as a comment.
		 */
		if (is_strict_query(node))
		{
			ereport(DEBUG1,
				(errmsg("executing parallel query"),
					 errdetail("waiting for backend %d completing the query", i)));
			if (synchronize(CONNECTION(backend, i)))
                ereport(ERROR,
					(errmsg("executing parallel query failed"),
                         errdetail("failed to synchronize")));
		}
	}

	if (!is_cache_empty(frontend, backend))
	{
		return POOL_END;
	}

	FD_ZERO(&donemask);
	/* In this loop, receive data from the all backends and send data to frontend */
	for (;;)
	{
		FD_ZERO(&readmask);
		FD_ZERO(&writemask);
		FD_ZERO(&exceptmask);
		num_fds = 0;

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i))
			{
				int fd = CONNECTION(backend,i)->fd;
				num_fds = Max(fd + 1, num_fds);
				if(!FD_ISSET(fd,&donemask))
				{
					FD_SET(fd, &readmask);
					FD_SET(fd, &exceptmask);
					ereport(DEBUG1,
						(errmsg("executing parallel query"),
							 errdetail("FD_SET:%d = %d",i, CONNECTION(backend, i)->fd)));
				}
			}
		}

		ereport(DEBUG1,
			(errmsg("executing parallel query"),
				 errdetail("num_fds: %d", num_fds)));

		fds = select(num_fds, &readmask, &writemask, &exceptmask, NULL);

		if (fds == -1)
		{
			if (errno == EINTR)
				continue;
            ereport(FATAL,
                (return_code(2),
                     errmsg("unable to execute parallel query"),
                        errdetail("select() system call failed with reason: %s", strerror(errno))));

		 }

		if (fds == 0)
		{
			return POOL_CONTINUE;
		}

		/* get header of protocol */
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i) ||
				!FD_ISSET(CONNECTION(backend, i)->fd, &readmask))
			{
				continue;
			}
			else
			{
				read_kind_from_one_backend(frontend, backend, &kind,i);

				if (used_count == 0)
				{
					ParallelForwardToFrontend(kind,
												frontend,
												CONNECTION(backend, i),
												backend->info->database,
												send_to_frontend);
					ereport(DEBUG1,
						(errmsg("executing parallel query"),
							 errdetail("backend kind ='%c'", kind)));

				}
				else
				{
					ParallelForwardToFrontend(kind,
														frontend,
														CONNECTION(backend, i),
														backend->info->database,
														false);
					ereport(DEBUG1,
						(errmsg("executing parallel query"),
							 errdetail("backend kind dummy ='%c'", kind)));
				}

				if(kind == 'C' || kind == 'E' || kind == 'c')
				{
					if(used_count == NUM_BACKENDS -1)
						return POOL_CONTINUE;

					used_count++;
					FD_SET(CONNECTION(backend, i)->fd, &donemask);
					continue;
				}

				/* get body of protocol */
				for(;;)
				{
					pool_read(CONNECTION(backend, i), &kind, 1);

					/*
					 * Sanity check
					 */
					if (kind == 0)
					{
                        ereport(FATAL,
                            (return_code(2),
                                 errmsg("unable to execute parallel query"),
                                    errdetail("invalid message kind. backend message kind is 0")));

					}

					if((kind == 'E' ) &&
						used_count != NUM_BACKENDS -1)
					{
						if(error_flag ==0)
						{
							ereport(DEBUG1,
								(errmsg("executing parallel query"),
									 errdetail("backend kind ='%c'", kind)));

							ParallelForwardToFrontend(kind,
															frontend,
															CONNECTION(backend, i),
															backend->info->database,
															send_to_frontend);
							error_flag++;
						} else {
							ereport(DEBUG1,
								(errmsg("executing parallel query"),
									 errdetail("backend kind dummy ='%c'", kind)));

							ParallelForwardToFrontend(kind,
															frontend,
															CONNECTION(backend, i),
															backend->info->database,
															false);
						}
						used_count++;
						FD_SET(CONNECTION(backend, i)->fd, &donemask);
						break;
					}

					if((kind == 'c' || kind == 'C') &&
					   used_count != NUM_BACKENDS -1)
					{
						ereport(DEBUG1,
							(errmsg("executing parallel query"),
								 errdetail("backend kind dummy ='%c'", kind)));

						ParallelForwardToFrontend(kind,
															frontend,
															CONNECTION(backend, i),
															backend->info->database,
															false);
						used_count++;
						FD_SET(CONNECTION(backend, i)->fd, &donemask);
						break;
					}
					if((kind == 'C' || kind == 'c' || kind == 'E') &&
						used_count == NUM_BACKENDS -1)
					{
						if(error_flag == 0)
						{
							ereport(DEBUG1,
								(errmsg("executing parallel query"),
									 errdetail("backend kind ='%c'", kind)));
							ParallelForwardToFrontend(kind,
														frontend,
														CONNECTION(backend, i),
														backend->info->database,
														send_to_frontend);
						} else {
							ereport(DEBUG1,
								(errmsg("executing parallel query"),
									 errdetail("backend kind dummy ='%c'", kind)));
							ParallelForwardToFrontend(kind,
														frontend,
														CONNECTION(backend, i),
														backend->info->database,
														false);
						}
						return POOL_CONTINUE;
					}

					if(kind == 'D')
						datacount++;

					ParallelForwardToFrontend(kind,
													frontend,
													CONNECTION(backend, i),
													backend->info->database,
													send_to_frontend);
					pool_flush(frontend);
				}
			}
		}
	}
}



/*
 * send simple query message to a node.
 */
void send_simplequery_message(POOL_CONNECTION *backend, int len, char *string, int major)
{
	/* forward the query to the backend */
	pool_write(backend, "Q", 1);

	if (major == PROTO_MAJOR_V3)
	{
		int sendlen = htonl(len + 4);
		pool_write(backend, &sendlen, sizeof(sendlen));
	}
	pool_write_and_flush(backend, string, len);
}

/*
 * wait_for_query_response_with_trans_cleanup():
 * this function is the wrapper over the wait_for_query_response() function and performs
 * and additional step of canceling the transaction in case of an error from wait_for_query_response()
 */

void wait_for_query_response_with_trans_cleanup(POOL_CONNECTION *frontend, POOL_CONNECTION *backend, int protoVersion, int pid, int key)
{
    PG_TRY();
    {
        wait_for_query_response(frontend, backend, protoVersion);
    }
    PG_CATCH();
    {
        /* Cancel current transaction */
        CancelPacket cancel_packet;
        
        cancel_packet.protoVersion = htonl(PROTO_CANCEL);
        cancel_packet.pid = pid;
        cancel_packet.key= key;
        cancel_request(&cancel_packet);
        
        PG_RE_THROW();
    }
    PG_END_TRY();
}
/*
 * Wait for query response from single node. If frontend is not NULL,
 * also check frontend connection by writing dummy parameter status
 * packet every 30 seccond, and if the connection broke, returns error
 * since there's no point in that waiting until backend returns
 * response.
 */
POOL_STATUS wait_for_query_response(POOL_CONNECTION *frontend, POOL_CONNECTION *backend, int protoVersion)
{
#define DUMMY_PARAMETER "pgpool_dummy_param"
#define DUMMY_VALUE "pgpool_dummy_value"

	int status;
	int plen;

	ereport(DEBUG1,
		(errmsg("waiting for query response"),
			 errdetail("waiting for backend:%d to complete the query", backend->db_node_id)));
	for (;;)
	{
		/* Check to see if data from backend is ready */
		pool_set_timeout(30);
		status = pool_check_fd(backend);
		pool_set_timeout(0);

		if (status < 0)	/* error ? */
		{
            ereport(ERROR,
                    (errmsg("backend error occured while waiting for backend response")));
		}
		else if (frontend != NULL && status > 0)
		{
			/*
			 * If data from backend is not ready, check frontend connection by sending dummy
			 * parameter status packet.
			 */
			if (protoVersion == PROTO_MAJOR_V3)
			{
				/* Write dummy parameter staus packet to check if the socket to frontend is ok */
				pool_write(frontend, "S", 1);
				plen = sizeof(DUMMY_PARAMETER)+sizeof(DUMMY_VALUE)+sizeof(plen);
				plen = htonl(plen);
				pool_write(frontend, &plen, sizeof(plen));
				pool_write(frontend, DUMMY_PARAMETER, sizeof(DUMMY_PARAMETER));
				pool_write(frontend, DUMMY_VALUE, sizeof(DUMMY_VALUE));
				if (pool_flush_it(frontend) < 0)
				{
                    ereport(ERROR,
						(errmsg("unable to to flush data to frontend"),
                             errdetail("frontend error occured while waiting for backend reply")));
				}

			} else		/* Protocol version 2 */
			{
/*
 * If you want to monitor client connection even if you are using V2 protocol,
 * define following
 */
#undef SEND_NOTICE_ON_PROTO2
#ifdef SEND_NOTICE_ON_PROTO2
				static char *notice_message = {"keep alive checking from pgpool-II"};

				/* Write notice message packet to check if the socket to frontend is ok */
				pool_write(frontend, "N", 1);
				pool_write(frontend, notice_message, strlen(notice_message)+1);
				if (pool_flush_it(frontend) < 0)
				{
                    ereport(ERROR,
                        (errmsg("unable to to flush data to frontend"),
                             errdetail("frontend error occured while waiting for backend reply")));

				}
#endif
			}
		}
		else
			break;
	}

	return POOL_CONTINUE;
}


/*
 * Extended query protocol has to send Flush message.
 */
POOL_STATUS send_extended_protocol_message(POOL_CONNECTION_POOL *backend,
												  int node_id, char *kind,
												  int len, char *string)
{
	POOL_CONNECTION *cp = CONNECTION(backend, node_id);
	int sendlen;

	/* forward the query to the backend */
	pool_write(cp, kind, 1);
	sendlen = htonl(len + 4);
	pool_write(cp, &sendlen, sizeof(sendlen));
	pool_write(cp, string, len);

	/*
	 * send "Flush" message so that backend notices us
	 * the completion of the command
	 */
	pool_write(cp, "H", 1);
	sendlen = htonl(4);
	pool_write_and_flush(cp, &sendlen, sizeof(sendlen));
	
	return POOL_CONTINUE;
}

/*
 * wait until read data is ready
 */
int synchronize(POOL_CONNECTION *cp)
{
	return pool_check_fd(cp);
}

/*
 * set timeout in seconds for pool_check_fd
 * if timeoutval < 0, we assume no timeout(wait forever).
 */
void pool_set_timeout(int timeoutval)
{
	if (timeoutval > 0)
		timeoutsec = timeoutval;
	else
		timeoutsec = 0;
}

/*
 * Wait until read data is ready.
 * return values: 0: normal 1: data is not ready -1: error
 */
int pool_check_fd(POOL_CONNECTION *cp)
{
	fd_set readmask;
	fd_set exceptmask;
	int fd;
	int fds;
	struct timeval timeout;
	struct timeval *timeoutp;

	/*
	 * If SSL is enabled, we need to check SSL internal buffer
	 * is empty or not first. Otherwise select(2) will stuck.
	 */
	if (pool_ssl_pending(cp))
	{
		return 0;
	}
		
	fd = cp->fd;

	if (timeoutsec > 0)
	{
		timeout.tv_sec = timeoutsec;
		timeout.tv_usec = 0;
		timeoutp = &timeout;
	}
	else
		timeoutp = NULL;

	for (;;)
	{
		FD_ZERO(&readmask);
		FD_ZERO(&exceptmask);
		FD_SET(fd, &readmask);
		FD_SET(fd, &exceptmask);

		fds = select(fd+1, &readmask, NULL, &exceptmask, timeoutp);
		if (fds == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
				continue;
			
			ereport(WARNING,
					(errmsg("waiting for reading data. select failed with error: \"%s\"", strerror(errno))));
			break;
		}
		else if (fds == 0)		/* timeout */
			return 1;

		if (FD_ISSET(fd, &exceptmask))
		{
			ereport(WARNING,
					(errmsg("waiting for reading data. exception occurred in select ")));
			break;
		}
		return 0;
	}
	return -1;
}

/*
 * send "terminate"(X) message to all backends, indicating that
 * backend should prepare to close connection to frontend (actually
 * pgpool). Note that caller must be proteceted from a signal
 * interruption while calling this function. Otherwise the number of
 * valid backends might be changed by failover/failback.
 */
void pool_send_frontend_exits(POOL_CONNECTION_POOL *backend)
{
	int len;
	int i;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		/*
		 * send a terminate message to backend if there's an existing
		 * connection
		 */
		if (VALID_BACKEND(i) && CONNECTION_SLOT(backend, i))
		{
			pool_write_noerror(CONNECTION(backend, i), "X", 1);

			if (MAJOR(backend) == PROTO_MAJOR_V3)
			{
				len = htonl(4);
				pool_write_noerror(CONNECTION(backend, i), &len, sizeof(len));
			}

			/*
			 * XXX we cannot call pool_flush() here since backend may already
			 * close the socket and pool_flush() automatically invokes fail
			 * over handler. This could happen in copy command (remember the
			 * famous "lost synchronization with server, resetting
			 * connection" message)
			 */
			pool_set_nonblock(CONNECTION(backend, i)->fd);
			pool_flush_it(CONNECTION(backend, i));
			pool_unset_nonblock(CONNECTION(backend, i)->fd);
		}
	}
}

/*
 * -------------------------------------------------------
 * V3 functions
 * -------------------------------------------------------
 */

/*
 * This function transmits to a parallel Query to each backend,
 * and receives the results from backends .
 *
 */
static void ParallelForwardToFrontend(char kind, POOL_CONNECTION *frontend, POOL_CONNECTION *backend, char *database, bool send_to_frontend)
{
	int len;
	char *p;

	if (send_to_frontend)
	{
		pool_write(frontend, &kind, 1);
	}

	pool_read(backend, &len, sizeof(len));

	if (send_to_frontend)
	{
		pool_write(frontend, &len, sizeof(len));
	}

	len = ntohl(len) - 4 ;

	if (len > 0)
	{
		p = pool_read2(backend, len);
		if (p == NULL)
			ereport(ERROR,
				(errmsg("parallel forward to Frontend failed"),
					 errdetail("read from backedn failed")));

		if (send_to_frontend)
			pool_write(frontend, p, len);
	}
}

POOL_STATUS SimpleForwardToFrontend(char kind, POOL_CONNECTION *frontend,
									POOL_CONNECTION_POOL *backend)
{
	int len, len1 = 0;
	char *p = NULL;
	char *p1 = NULL;
	int sendlen;
	int i;

#ifdef NOT_USED
	/* 
	 * The response of Execute command will be EmptyQueryResponse(I),
	 * if Bind error occurs.
	 */
	else if ((kind == 'C' || kind == 'I') && select_in_transaction)
	{
		select_in_transaction = 0;
		execute_select = 0;
	}
#endif

	pool_read(MASTER(backend), &len, sizeof(len));

	len = ntohl(len);
	len -= 4;
	len1 = len;

	p = pool_read2(MASTER(backend), len);
	if (p == NULL)
        ereport(ERROR,
            (errmsg("unable to forward message to frontend"),
                 errdetail("read from backend failed")));
	p1 = palloc(len);
	memcpy(p1, p, len);

	/*
	 * If we received a notification message in master/slave mode,
	 * other backends will not receive the message.
	 * So we should skip other nodes otherwise we will hung in pool_read.
	 */
	if (!MASTER_SLAVE || kind != 'A')
	{
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
			{
				pool_read(CONNECTION(backend, i), &len, sizeof(len));

				len = ntohl(len);
				len -= 4;

				p = pool_read2(CONNECTION(backend, i), len);
				if (p == NULL)
                    ereport(ERROR,
                        (errmsg("unable to forward message to frontend"),
                             errdetail("read on backend node failed")));

				if (len != len1)
				{
					ereport(DEBUG1,
						(errmsg("SimpleForwardToFrontend: length does not match between backends master(%d) %d th backend(%d) kind:(%c)",
							   len, i, len1, kind)));
				}
			}
		}
	}

	pool_write(frontend, &kind, 1);
	sendlen = htonl(len1+4);
	pool_write(frontend, &sendlen, sizeof(sendlen));
	pool_write_and_flush(frontend, p1, len1);

	/* save the received result to buffer for each kind */
	if (pool_config->memory_cache_enabled)
	{
		if (pool_is_cache_safe() && !pool_is_cache_exceeded())
		{
			memqcache_register(kind, frontend, p1, len1);
		}
	}

	/* error response? */
	if (kind == 'E')
	{
		/*
		 * check if the error was PANIC or FATAL. If so, we just flush
		 * the message and exit since the backend will close the
		 * channel immediately.
		 */
		if (is_panic_or_fatal_error(p1, MAJOR(backend)))
            ereport(ERROR,
				(errmsg("unable to forward message to frontend"),
                     errdetail("FATAL error occured on backend")));
	}

	pfree(p1);
	return POOL_CONTINUE;
}

POOL_STATUS SimpleForwardToBackend(char kind, POOL_CONNECTION *frontend,
								   POOL_CONNECTION_POOL *backend,
								   int len, char *contents)
{
	int sendlen;
	int i;
	sendlen = htonl(len + 4);

	if (len == 0)
	{
		/* We assume that we can always forward to every node
		 * regardless running mode. Am I correct?
		 */
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i))
			{
#ifdef NOT_USED
				snprintf(msgbuf, sizeof(msgbuf), "%c message", kind);
				per_node_statement_log(backend, i, msgbuf);
#endif

				pool_write(CONNECTION(backend, i), &kind, 1);
				pool_write_and_flush(CONNECTION(backend,i), &sendlen, sizeof(sendlen));
			}
		}
		return POOL_CONTINUE;
	}
	else if (len < 0)
        ereport(ERROR,
            (errmsg("unable to forward message to frontend"),
                 errdetail("invalid message length:%d for message:%c", len, kind)));


	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
#ifdef NOT_USED
			snprintf(msgbuf, sizeof(msgbuf), "%c message", kind);
			per_node_statement_log(backend, i, msgbuf);
#endif

			pool_write(CONNECTION(backend, i), &kind, 1);
			pool_write(CONNECTION(backend,i), &sendlen, sizeof(sendlen));
			pool_write_and_flush(CONNECTION(backend, i), contents, len);
		}
	}

	return POOL_CONTINUE;
}

POOL_STATUS ParameterStatus(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	int len, len1 = 0;
	int *len_array;
	int sendlen;
	char *p;
	char *name;
	char *value;
	POOL_STATUS status;
	char parambuf[1024];		/* parameter + value string buffer. XXX is this enough? */
	int i;

	pool_write(frontend, "S", 1);

	len_array = pool_read_message_length2(backend);

	if (len_array == NULL)
        ereport(ERROR,
            (errmsg("unable to process parameter status"),
                 errdetail("read on backend failed")));

	len = len_array[MASTER_NODE_ID];
	sendlen = htonl(len);
	pool_write(frontend, &sendlen, sizeof(sendlen));

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			len = len_array[i];
			len -= 4;

			p = pool_read2(CONNECTION(backend, i), len);
			if (p == NULL)
                ereport(ERROR,
                    (errmsg("unable to process parameter status"),
                         errdetail("could not read from backend")));
 
			name = p;
			value = p + strlen(name) + 1;
			ereport(DEBUG1,
				(errmsg("process parameter status"),
					 errdetail("backend:%d name:\"%s\" value:\"%s\"", i, name, value)));

			if (IS_MASTER_NODE_ID(i))
			{
				len1 = len;
				memcpy(parambuf, p, len);
				pool_add_param(&CONNECTION(backend, i)->params, name, value);
			}

#ifdef DEBUG
			pool_param_debug_print(&MASTER(backend)->params);
#endif
		}
	}

	status = pool_write(frontend, parambuf, len1);
	return status;
}

/*
 * Reset all state variables
 */
void reset_variables(void)
{
	if(pool_get_session_context(true))
		pool_unset_query_in_progress();
}


/*
 * if connection_cache == 0, we don't need reset_query.
 * but we need reset prepared list.
 */
void reset_connection(void)
{
	reset_variables();
	pool_clear_sent_message_list();
}


/*
 * Reset backend status. return values are:
 * 0: no query was issued 1: a query was issued 2: no more queries remain -1: error
 */
static int
    reset_backend(POOL_CONNECTION_POOL *backend, int qcnt)
{
	char *query;
	int qn;
	int i;
	bool need_to_abort;
	POOL_SESSION_CONTEXT *session_context;
	POOL_TEMP_QUERY_CACHE *cache;

	/* Get session context */
	session_context = pool_get_session_context(false);
	/* Set reset context */
	session_context->reset_context = true;

	/*
	 * Reset all state variables
	 */
	reset_variables();

	qn = pool_config->num_reset_queries;

	/*
	 * After execution of all SQL commands in the reset_query_list, we
	 * remove all prepared objects in the prepared_list.
	 */
	if (qcnt >= qn)
	{
		if (session_context->message_list.size == 0)
			return 2;

		char kind = session_context->message_list.sent_messages[0]->kind;
		char *name = session_context->message_list.sent_messages[0]->name;

		if ((kind == 'P' || kind == 'Q') && *name != '\0')
		{
			/* Deallocate a prepared statement */
			if (send_deallocate(backend, session_context->message_list, 0))
			{
				/* Deallocate failed. We are in unknown state. Ask caller
				 * to reset backend connection.
				 */
#ifdef NOT_USED
				reset_prepared_list(&prepared_list);
#endif
				pool_remove_sent_message(kind, name);
				return -1;
			}
			/*
			 * If DEALLOCATE returns ERROR response, instead of
			 * CommandComplete, del_prepared_list is not called and the
			 * prepared object keeps on sitting on the prepared list. This
			 * will cause infinite call to reset_backend.  So we call
			 * del_prepared_list() again. This is harmless since trying to
			 * remove same prepared object will be ignored.
			 */
#ifdef NOT_USED
			del_prepared_list(&prepared_list, prepared_list.portal_list[0]);
#endif
			pool_remove_sent_message(kind, name);
			return 1;
		}
		else
		{
			pool_remove_sent_message(kind, name);
			return 0;
		}
	}

	query = pool_config->reset_query_list[qcnt];
	if (!strcmp("ABORT", query))
	{
		/* If transaction state are all idle, we don't need to issue ABORT */
		need_to_abort = false;

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i) && TSTATE(backend, i) != 'I')
				need_to_abort = true;
		}

		if (!need_to_abort)
			return 0;
	}

	pool_set_timeout(10);

	if (SimpleQuery(NULL, backend, strlen(query)+1, query) != POOL_CONTINUE)
	{
		pool_set_timeout(0);
		return -1;
	}

	pool_set_timeout(0);

	cache = pool_get_current_cache();
	if (cache)
	{
		pool_discard_temp_query_cache(cache);
		/*
		 * Reset temp_cache pointer in the current query context
		 * so that we don't double free memory.
		 */
		session_context->query_context->temp_cache = NULL;
	}
		
	return 1;
}

/*
 * Returns true if the SQL statement is regarded as read SELECT from syntax's
 * point of view. However callers need to do aditional checking such as if the
 * SELECT does not have write functions or not to make sure that the SELECT is
 * sematically read SELECT.
 *
 * For followings this function returns true:
 * - SELECT/WITH without FOR UPDATE/SHARE
 * - COPY TO STDOUT
 * - EXPLAIN
 * - EXPLAIN ANALYZE and query is SELECT not including writing functions
 *
 * Note that for SELECT this function returns false.
 */
bool is_select_query(Node *node, char *sql)
{
	if (node == NULL)
		return false;

	/*
	 * 2009/5/1 Tatsuo says: This test is not bogus. As of 2.2, pgpool
	 * sets Portal->sql_string to NULL for SQL command PREPARE.
	 * Usually this is ok, since in most cases SQL command EXECUTE
	 * follows anyway. Problem is, some applications mix PREPARE with
	 * extended protocol command "EXECUTE" and so on. Execute() seems
	 * to think this never happens but it is not real. Someday we
	 * should extract actual query string from PrepareStmt->query and
	 * set it to Portal->sql_string.
	 */
	if (sql == NULL)
		return false;

	if (!pool_config->allow_sql_comments && pool_config->ignore_leading_white_space)
	{
		/* ignore leading white spaces */
		while (*sql && isspace(*sql))
			sql++;
	}

	if (IsA(node, SelectStmt))
	{
		SelectStmt *select_stmt;

		select_stmt = (SelectStmt *)node;

		if (select_stmt->intoClause || select_stmt->lockingClause)
			return false;

		if (!pool_config->allow_sql_comments)
			/* '\0' and ';' signify empty query */
			return (*sql == 's' || *sql == 'S' || *sql == '(' ||
					*sql == 'w' || *sql == 'W' || *sql == 't' || *sql == 'T' ||
					*sql == '\0' || *sql == ';');
		else
			return true;
	}
	else if (IsA(node, CopyStmt))
	{
		CopyStmt *copy_stmt = (CopyStmt *)node;
		return (copy_stmt->is_from == FALSE &&
				copy_stmt->filename == NULL);
	}
	else if (IsA(node, ExplainStmt))
	{
		ExplainStmt * explain_stmt = (ExplainStmt *)node;
		Node *query = explain_stmt->query;
		ListCell *lc;
		bool analyze = false;

		/* Check to see if this is EXPLAIN ANALYZE */
		foreach (lc, explain_stmt->options)
		{
			DefElem    *opt = (DefElem *) lfirst(lc);

			if (strcmp(opt->defname, "analyze") == 0)
			{
				analyze = true;
				break;
			}
		}

		if (IsA(query, SelectStmt))
		{
			/*
			 * If query is SELECT and there's no ANALYZE option, we
			 * can always load balance.
			 */
			if (!analyze)
				return true;
			/*
			 * If ANALYZE, we need to check function calls.
			 */
			if (pool_has_function_call(query))
				return false;
			return true;
		}
		else
		{
			/*
			 * Other than SELECT can be load balance only if ANALYZE
			 * is not specified.
			 */
			if (!analyze)
				return true;
		}
	}
	return false;
}

#ifdef NOT_USED
/*
 * returns true if SQL is SELECT statement including nextval() or
 * setval() call
 */
bool is_sequence_query(Node *node)
{
	SelectStmt *select_stmt;
	ListCell *lc;

	if (node == NULL || !IsA(node, SelectStmt))
		return false;

	select_stmt = (SelectStmt *)node;
	foreach (lc, select_stmt->targetList)
	{
		if (IsA(lfirst(lc), ResTarget))
		{
			ResTarget *t;
			FuncCall *fc;
			ListCell *c;

			t = (ResTarget *) lfirst(lc);
			if (IsA(t->val, FuncCall))
			{
				fc = (FuncCall *) t->val;
				foreach (c, fc->funcname)
				{
					Value *v = lfirst(c);
					if (strncasecmp(v->val.str, "NEXTVAL", 7) == 0)
						return true;
					else if (strncasecmp(v->val.str, "SETVAL", 6) == 0)
						return true;
				}
			}
		}
	}

	return false;
}
#endif

/*
 * Returns true if SQL is transaction commit or rollback command (COMMIT,
 * END TRANSACTION, ROLLBACK or ABORT)
 */
bool is_commit_or_rollback_query(Node *node)
{
	return is_commit_query(node) || is_rollback_query(node);
}

/*
 * Returns true if SQL is transaction commit command (COMMIT or END
 * TRANSACTION)
 */
bool is_commit_query(Node *node)
{
	TransactionStmt *stmt;

	if (node == NULL || !IsA(node, TransactionStmt))
		return false;

	stmt = (TransactionStmt *)node;
	return stmt->kind == TRANS_STMT_COMMIT;
}

/*
 * Returns true if SQL is transaction rollback command (ROLLBACK or
 * ABORT)
 */
bool is_rollback_query(Node *node)
{
	TransactionStmt *stmt;

	if (node == NULL || !IsA(node, TransactionStmt))
		return false;

	stmt = (TransactionStmt *)node;
	return stmt->kind == TRANS_STMT_ROLLBACK;
}

/*
 * send error message to frontend
 */
void pool_send_error_message(POOL_CONNECTION *frontend, int protoMajor,
							 char *code,
							 char *message,
							 char *detail,
							 char *hint,
							 char *file,
							 int line)
{
	pool_send_severity_message(frontend, protoMajor, code, message, detail, hint, file, "ERROR", line);
}

/*
 * send fatal message to frontend
 */
void pool_send_fatal_message(POOL_CONNECTION *frontend, int protoMajor,
							 char *code,
							 char *message,
							 char *detail,
							 char *hint,
							 char *file,
							 int line)
{
	pool_send_severity_message(frontend, protoMajor, code, message, detail, hint, file, "FATAL", line);
}

/*
 * send severity message to frontend
 */
void pool_send_severity_message(POOL_CONNECTION *frontend, int protoMajor,
							 char *code,
							 char *message,
							 char *detail,
							 char *hint,
							 char *file,
							 char *severity,
							 int line)
{
/*
 * Buffer length for each message part
 */
#define MAXMSGBUF 256
/*
 * Buffer length for result message buffer.
 * Since msg is consisted of 7 parts, msg buffer should be large
 * enough to hold those message parts
*/
#define MAXDATA	(MAXMSGBUF+1)*7+1

	pool_set_nonblock(frontend->fd);

	if (protoMajor == PROTO_MAJOR_V2)
	{
		pool_write(frontend, "E", 1);
		pool_write_and_flush(frontend, message, strlen(message)+1);
	}
	else if (protoMajor == PROTO_MAJOR_V3)
	{
		char data[MAXDATA];
		char msgbuf[MAXMSGBUF+1];
		int len;
		int thislen;
		int sendlen;

		len = 0;
		memset(data, 0, MAXDATA);

		pool_write(frontend, "E", 1);

		/* error level */
		thislen = snprintf(msgbuf, MAXMSGBUF, "S%s", severity);
		thislen = Min(thislen, MAXMSGBUF);
		memcpy(data +len, msgbuf, thislen+1);
		len += thislen + 1;

		/* code */
		thislen = snprintf(msgbuf, MAXMSGBUF, "C%s", code);
		thislen = Min(thislen, MAXMSGBUF);
		memcpy(data +len, msgbuf, thislen+1);
		len += thislen + 1;

		/* message */
		thislen = snprintf(msgbuf, MAXMSGBUF, "M%s", message);
		thislen = Min(thislen, MAXMSGBUF);
		memcpy(data +len, msgbuf, thislen+1);
		len += thislen + 1;

		/* detail */
		if (detail && *detail != '\0')
		{
			thislen = snprintf(msgbuf, MAXMSGBUF, "D%s", detail);
			thislen = Min(thislen, MAXMSGBUF);
			memcpy(data +len, msgbuf, thislen+1);
			len += thislen + 1;
		}

		/* hint */
		if (hint && *hint != '\0')
		{
			thislen = snprintf(msgbuf, MAXMSGBUF, "H%s", hint);
			thislen = Min(thislen, MAXMSGBUF);
			memcpy(data +len, msgbuf, thislen+1);
			len += thislen + 1;
		}

		/* file */
		thislen = snprintf(msgbuf, MAXMSGBUF, "F%s", file);
		thislen = Min(thislen, MAXMSGBUF);
		memcpy(data +len, msgbuf, thislen+1);
		len += thislen + 1;

		/* line */
		thislen = snprintf(msgbuf, MAXMSGBUF, "L%d", line);
		thislen = Min(thislen, MAXMSGBUF);
		memcpy(data +len, msgbuf, thislen+1);
		len += thislen + 1;

		/* stop null */
		len++;
		*(data + len - 1) = '\0';

		sendlen = len;
		len = htonl(len + 4);
		pool_write(frontend, &len, sizeof(len));
		pool_write_and_flush(frontend, data, sendlen);
	}
	else
		ereport(ERROR,
				(errmsg("send_error_message: unknown protocol major %d", protoMajor)));

	pool_unset_nonblock(frontend->fd);
}

void pool_send_readyforquery(POOL_CONNECTION *frontend)
{
	int len;
	pool_write(frontend, "Z", 1);
	len = 5;
	len = htonl(len);
	pool_write(frontend, &len, sizeof(len));
	pool_write(frontend, "I", 1);
	pool_flush(frontend);
}

/*
 * Send a query to a backend in sync manner.
 * This function sends a query and waits for CommandComplete/ReadyForQuery.
 * If an error occured, it returns with POOL_ERROR.
 * This function does NOT handle SELECT/SHOW queries.
 * If no_ready_for_query is non 0, returns without reading the packet
 * length for ReadyForQuery. This mode is necessary when called from ReadyForQuery().
 */
POOL_STATUS do_command(POOL_CONNECTION *frontend, POOL_CONNECTION *backend,
					   char *query, int protoMajor, int pid, int key, int no_ready_for_query)
{
	int len;
	char kind;
	char *string;
	int deadlock_detected = 0;
	ereport(DEBUG1,
		(errmsg("do_command: Query:\"%s\"", query)));
	
	/* send the query to the backend */
	send_simplequery_message(backend, strlen(query)+1, query, protoMajor);

	/*
	 * Wait for response from badckend while polling frontend connection is ok.
	 * If not, cancel the transaction.
	 */
    wait_for_query_response_with_trans_cleanup(frontend,
                                               backend,
                                               protoMajor,
                                               pid,
                                               key);

 	/*
	 * We must check deadlock error here. If a deadlock error is
	 * detected by a backend, other backend might not be noticed the
	 * error.  In this case caller should send an error query to the
	 * backend to abort the transaction. Otherwise the transaction
	 * state might vary among backends (idle in transaction vs. abort).
	 */
	deadlock_detected = detect_deadlock_error(backend, protoMajor);

	/*
	 * Continue to read packets until we get ReadForQuery (Z).
	 * Until that we may recieve one of:
	 *
	 * N: Notice response
	 * E: Error response
	 * C: Comand complete
	 *
	 * XXX: we ignore Notice and Error here. Even notice/error
	 * messages are not sent to the frontend. May be it's ok since the
	 * error was caused by our internal use of SQL command (otherwise users
	 * will be confused).
	 */
	for(;;)
	{
		pool_read(backend, &kind, sizeof(kind));
		ereport(DEBUG1,
			(errmsg("do_command: kind: '%c'", kind)));

		if (kind == 'Z')		/* Ready for Query? */
			break;		/* get out the loop without reading message lenghth */

		if (protoMajor == PROTO_MAJOR_V3)
		{
			pool_read(backend, &len, sizeof(len));

			len = ntohl(len) - 4;

			if (kind != 'N' && kind != 'E' && kind != 'S' && kind != 'C' && kind != 'A')
			{
                ereport(ERROR,
					(errmsg("do command failed"),
                         errdetail("kind is not N, E, S, C or A(%02x)", kind)));
                
			}
			string = pool_read2(backend, len);
			if (string == NULL)
			{
                ereport(ERROR,
					(errmsg("do command failed"),
                         errdetail("unable to read message from backend")));
			}

			/*
			 * It is possible that we receive a notification response
			 * ('A') from one of backends prior to "ready for query"
			 * response if LISTEN and NOTIFY are issued in a same
			 * connection. So we need to save notification response to
			 * stack buffer so that we could retrieve it later on.
			 */
			if (kind == 'A')
			{
				int nlen = htonl(len+4);
				pool_push(backend, &kind, sizeof(kind));
				pool_push(backend, &nlen, sizeof(nlen));
				pool_push(backend, string, len);
			}
		}
		else
		{
			string = pool_read_string(backend, &len, 0);
			if (string == NULL)
			{
                ereport(ERROR,
                        (errmsg("do command failed"),
                         errdetail("unable to read reset message from backend")));
			}
			if (kind == 'C')
			{
				if (!strncmp(string, "BEGIN", 5))
					backend->tstate = 'T';
				if (!strncmp(string, "COMMIT", 6) ||
					!strncmp(string, "ROLLBACK", 8))
					backend->tstate = 'I';
			}
		}

		if(kind == 'E')
		{
			if (is_panic_or_fatal_error(string, protoMajor))
			{
                ereport(ERROR,
                        (errmsg("do command failed"),
                         errdetail("backend error: \"%s\"",string)));
			}
		}
	}

/*
 * until 2008/11/12 we believed that we never had packets other than
 * 'Z' after receiving 'C'. However a counter example was presented by
 * a poor customer. So we replaced the whole thing with codes
 * above. In a side effect we were be able to get ride of nasty
 * "goto". Congratulations.
 */
#ifdef NOT_USED
	/*
	 * Expecting CompleteCommand
	 */
retry_read_packet:
	pool_read(backend, &kind, sizeof(kind));

	if (kind == 'E')
	{
        ereport(LOG,
                (errmsg("do command failed"),
                 errdetail("backend does not successfully complete command %s status %c", query, kind)));
	}

	/*
	 * read command tag of CommandComplete response
	 */
	if (protoMajor == PROTO_MAJOR_V3)
	{
		pool_read(backend, &len, sizeof(len));
		len = ntohl(len) - 4;
		string = pool_read2(backend, len);
		if (string == NULL)
            ereport(ERROR,
                (errmsg("do command failed"),
                     errdetail("read from backend failed")));

        ereport(DEBUG2,
                (errmsg("do command: command tag: %s", string)));
	}
	else
	{
		string = pool_read_string(backend, &len, 0);
		if (string == NULL)
            ereport(ERROR,
                    (errmsg("do command failed"),
                     errdetail("read from backend failed")));
	}

	if (kind == 'N') /* warning? */
		goto retry_read_packet;

	/*
	 * Expecting ReadyForQuery
	 */
	pool_read(backend, &kind, sizeof(kind));

	if (kind != 'Z')
	{
        ereport(ERROR,
                (errmsg("do command failed"),
                 errdetail("backend returns %c while expecting ReadyForQuery", kind)));
	}
#endif

	if (no_ready_for_query)
		return POOL_CONTINUE;

	if (protoMajor == PROTO_MAJOR_V3)
	{
		/* read packet lenghth for ready for query */
		pool_read(backend, &len, sizeof(len));

		/* read transaction state */
		pool_read(backend, &kind, sizeof(kind));

		/* set transaction state */
        
		backend->tstate = kind;

        ereport(DEBUG2,
                (errmsg("do_command: transaction state: %c", kind)));
	}

	return deadlock_detected ? POOL_DEADLOCK : POOL_CONTINUE;
}

/*
 * Send a syntax error query to abort transaction and receive response
 * from backend and discard it until we get Error response.
 *
 * We need to sync transaction status in transaction block.
 * SELECT query is sent to master only.
 * If SELECT is error, we must abort transaction on other nodes.
 */
void do_error_command(POOL_CONNECTION *backend, int major)
{
	char *error_query = POOL_ERROR_QUERY;
	int len;
	char kind;
	char *string;

	send_simplequery_message(backend, strlen(error_query) + 1, error_query, major);
	/*
	 * Continue to read packets until we get Error response (E).
	 * Until that we may recieve one of:
	 *
	 * N: Notice response
	 * C: Comand complete
	 *
	 * XXX: we ignore Notice here. Even notice messages are not sent
	 * to the frontend. May be it's ok since the error was caused by
	 * our internal use of SQL command (otherwise users will be
	 * confused).
	 */
	do
	{
		pool_read(backend, &kind, sizeof(kind));

        ereport(DEBUG2,
                (errmsg("do_error_command: kind: %c", kind)));

		if (major == PROTO_MAJOR_V3)
		{
			pool_read(backend, &len, sizeof(len));
			len = ntohl(len) - 4;
			string = pool_read2(backend, len);
			if (string == NULL)
                ereport(ERROR,
                    (errmsg("do error command failed"),
                         errdetail("read from backend failed")));
		}
		else
		{
			string = pool_read_string(backend, &len, 0);
			if (string == NULL)
                ereport(ERROR,
                        (errmsg("do error command failed"),
                         errdetail("read from backend failed")));
		}
	} while (kind != 'E');

#ifdef NOT_USED
	/*
	 * Expecting ErrorResponse
	 */
	pool_read(backend, &kind, sizeof(kind));

	/*
	 * read command tag of CommandComplete response
	 */
	if (major == PROTO_MAJOR_V3)
	{
		pool_read(backend, &len, sizeof(len));
		len = ntohl(len) - 4;
		string = pool_read2(backend, len);
		if (string == NULL)
            ereport(ERROR,
                (errmsg("do error command failed"),
                     errdetail("read from backend failed")));
	}
	else
	{
		string = pool_read_string(backend, &len, 0);
		if (string == NULL)
            ereport(ERROR,
                (errmsg("do error command failed"),
                     errdetail("read from backend failed")));
	}
#endif
}

/*
 * Send invalid portal execution to specified DB node to abort current
 * transaction.  Pgpool-II sends a SELECT query to master node only in
 * load balance mode. Problem is, if the query failed, master node
 * goes to abort status while other nodes remain normal status. To
 * sync transaction status in each node, we send error query to other
 * than master node to ket them go into abort status.
 */
void do_error_execute_command(POOL_CONNECTION_POOL *backend, int node_id, int major)
{
	char kind;
	char *string;
	char msg[1024] = "pgpool_error_portal"; /* large enough */
	int len = strlen(msg);
	char buf[8192]; /* memory space is large enough */
	char *p;
	int readlen = 0;

	p = buf;
	memset(msg + len, 0, sizeof(int));
	send_extended_protocol_message(backend, node_id, "E", len + 5, msg);

	/*
	 * Discard responses from backend until ErrorResponse received.
	 * Note that we need to preserve non-error responses
	 * (i.e. ReadyForQuery) and put back them using pool_unread().
	 * Otherwise, ReadyForQuery response of DEALLOCATE. This could
	 * happen if PHP PDO used. (2010/04/21)
	 */
	do {
		pool_read(CONNECTION(backend, node_id), &kind, sizeof(kind));

		/*
		 * read command tag of CommandComplete response
		 */
		if (kind != 'E')
		{
			readlen += sizeof(kind);
			memcpy(p, &kind, sizeof(kind));
			p += sizeof(kind);
		}

		if (major == PROTO_MAJOR_V3)
		{
			pool_read(CONNECTION(backend, node_id), &len, sizeof(len));

			if (kind != 'E')
			{
				readlen += sizeof(len);
				memcpy(p, &len, sizeof(len));
				p += sizeof(len);
			}

			len = ntohl(len) - 4;
			string = pool_read2(CONNECTION(backend, node_id), len);
			if (string == NULL)
                ereport(ERROR,
                    (errmsg("do error execute command failed"),
                         errdetail("read from backend failed")));
			if (kind != 'E')
			{
				readlen += len;
				if (readlen >= sizeof(buf))
				{
                    ereport(ERROR,
						(errmsg("do error execute command failed"),
                             errdetail("not enough space in buffer")));

				}
				memcpy(p, string, len);
				p += sizeof(len);
			}
		}
		else
		{
			string = pool_read_string(CONNECTION(backend, node_id), &len, 0);
			if (string == NULL)
                ereport(ERROR,
                    (errmsg("do error execute command failed"),
                         errdetail("read from backend failed")));

			if (kind != 'E')
			{
				readlen += len;
				if (readlen >= sizeof(buf))
				{
                    ereport(ERROR,
						(errmsg("do error execute command failed"),
                             errdetail("not enough space in buffer")));
				}
				memcpy(p, string, len);
				p += sizeof(len);
			}
		}
	} while (kind != 'E');

    if (readlen > 0)
    {
        /* put messages back to read buffer */
        pool_unread(CONNECTION(backend, node_id), buf, readlen);
    }
}

/*
 * Transmit an arbitrary Query to a specific node.
 * This function is only used in parallel mode
 */
POOL_STATUS OneNode_do_command(POOL_CONNECTION *frontend, POOL_CONNECTION *backend, char *query, char *database)
{
	int len,sendlen;
	char kind;
	bool notice = false;

	/* send the query to the backend */
	pool_write(backend, "Q", 1);
	len = strlen(query)+1;

	sendlen = htonl(len + 4);
	pool_write(backend, &sendlen, sizeof(sendlen));

	pool_write_and_flush(backend, query, len);

	for(;;)
	{
		pool_read(backend, &kind, sizeof(kind));

		if (kind == 'N' && strstr(query,"dblink")) {
			notice = true;
			ParallelForwardToFrontend(kind, frontend, backend, database, false);
		} else
		{
			if(notice)
				ParallelForwardToFrontend(kind, frontend, backend, database, false);
			else
				ParallelForwardToFrontend(kind, frontend, backend, database, true);
		}
		if (kind == 'C' || kind =='E')
		{
			break;
		}
	}
	/*
	 * Expecting ReadyForQuery
	 */
	if(notice)
        pool_send_error_message(frontend, 3, "XX000",
                                "pgpool2 sql restriction(notice from dblink)",query,"",
                                __FILE__,__LINE__);
	pool_read(backend, &kind, sizeof(kind));

	if (kind != 'Z')
        ereport(ERROR,
			(errmsg("One node do command failed"),
                 errdetail("backend does not return ReadyForQuery")));

	ParallelForwardToFrontend(kind, frontend, backend, database, true);
	pool_flush(frontend);

    return POOL_CONTINUE;
}

/*
 * Free POOL_SELECT_RESULT object
 */
void free_select_result(POOL_SELECT_RESULT *result)
{
	int i, j;
	int index;

	if (result == NULL)
		return;

	if (result->nullflags)
		pfree(result->nullflags);

	if (result->data)
	{
		index = 0;
		for(i=0;i<result->numrows;i++)
		{
			for (j=0;j<result->rowdesc->num_attrs;j++)
			{
				if (result->data[index])
					pfree(result->data[index]);
				index++;
			}
		}
		pfree(result->data);
	}

	if (result->rowdesc)
	{
		if (result->rowdesc->attrinfo)
		{
			for(i=0;i<result->rowdesc->num_attrs;i++)
			{
				if (result->rowdesc->attrinfo[i].attrname)
					pfree(result->rowdesc->attrinfo[i].attrname);
			}
			pfree(result->rowdesc->attrinfo);
		}
		pfree(result->rowdesc);
	}

	pfree(result);
}

/*
 * Send a query to one DB node and wait for it's completion.  The quey
 * can be SELECT or any other type of query. However at this moment,
 * the only client calls this function other than SELECT is
 * insert_lock(), and the qury is either LOCK or SELECT for UPDATE.
 * Note: After the introduction of elog API the return type of do_query is changed
 * to void. and now ereport is thrown in case of error occured within the function
 */
void do_query(POOL_CONNECTION *backend, char *query, POOL_SELECT_RESULT **result, int major)
{
#define DO_QUERY_ALLOC_NUM 1024	/* memory allocation unit for POOL_SELECT_RESULT */

/*
 * State transition control bits. We expect all following events have
 * been occur before finish do_query() in extended protocol mode.
 * Note that "Close Compplete" should occur twice, because two close
 * requests(one for prepared statement and the other for portal) have
 * been sent.
 */
#define PARSE_COMPLETE_RECEIVED			(1 << 0)
#define BIND_COMPLETE_RECEIVED			(1 << 1)
#define CLOSE_COMPLETE_RECEIVED			(1 << 2)
#define COMMAND_COMPLETE_RECEIVED		(1 << 3)
#define ROW_DESCRIPTION_RECEIVED		(1 << 4)
#define DATA_ROW_RECEIVED				(1 << 5)
#define STATE_COMPLETED		(PARSE_COMPLETE_RECEIVED | BIND_COMPLETE_RECEIVED |\
							 CLOSE_COMPLETE_RECEIVED | COMMAND_COMPLETE_RECEIVED | \
							 ROW_DESCRIPTION_RECEIVED | DATA_ROW_RECEIVED)

	int i;
	int len;
	char kind;
	char *packet = NULL;
	char *p = NULL;
	short num_fields = 0;
	int num_data;
	int intval;
	short shortval;

	POOL_SELECT_RESULT *res;
	RowDesc *rowdesc;
	AttrInfo *attrinfo;

	int nbytes;
	static char nullmap[8192];
	unsigned char mask = 0;
	bool doing_extended;
	int num_close_complete;
	int state;

	doing_extended = pool_get_session_context(true) && pool_is_doing_extended_query_message();

	ereport(DEBUG1,
			(errmsg("do_query: extended:%d query:\"%s\"", doing_extended, query)));

	*result = NULL;
	res = palloc0(sizeof(*res));
	rowdesc = palloc0(sizeof(*rowdesc));
	*result = res;

	res->rowdesc = rowdesc;

	num_data = 0;

	res->nullflags = palloc(DO_QUERY_ALLOC_NUM*sizeof(int));
	res->data = palloc0(DO_QUERY_ALLOC_NUM*sizeof(char *));

	/*
	 * Send a query to the backend. We use extended query proctocol
	 * with named statement/portal if we are processing exetended
	 * query since simple query breaks unnamed statements/portals.
	 * The name of named statment/unamed statement are "pgpool_PID"
	 * where PID is the process id of itself.
	 */
	if (pool_get_session_context(true) && pool_is_doing_extended_query_message())
	{
		static char prepared_name[256];
		static int pname_len;
		int qlen;

		if (pname_len == 0)
		{
			snprintf(prepared_name, sizeof(prepared_name), "pgpool%d", getpid());
			pname_len = strlen(prepared_name)+1;
		}

		qlen = strlen(query)+1;

		/*
		 * Send parse message
		 */
		pool_write(backend, "P", 1);
		len = 4 + pname_len + qlen + sizeof(int16);
		len = htonl(len);
		pool_write(backend, &len, sizeof(len));
		pool_write(backend, prepared_name, pname_len);	/* statement */
		pool_write(backend, query, qlen);		/* query */
		shortval = 0;
		pool_write(backend, &shortval, sizeof(shortval));		/* num parameters */

		/*
		 * Send bind message
		 */
		pool_write(backend, "B", 1);
		len = 4 + pname_len + pname_len + sizeof(int16) + sizeof(int16) + sizeof(int16) + sizeof(int16);
		len = htonl(len);
		pool_write(backend, &len, sizeof(len));
		pool_write(backend, prepared_name, pname_len);	/* portal */
		pool_write(backend, prepared_name, pname_len);	/* statement */
		shortval = 0;
		pool_write(backend, &shortval, sizeof(shortval));		/* num parameter format code */
		pool_write(backend, &shortval, sizeof(shortval));		/* num parameter values */
		shortval = htons(1);
		pool_write(backend, &shortval, sizeof(shortval));		/* num result format */
		shortval = 0;
		pool_write(backend, &shortval, sizeof(shortval));		/* result format (text) */

		/*
		 * Send close statement message
		 */
		pool_write(backend, "C", 1);
		len = 4 + 1 + pname_len;
		len = htonl(len);
		pool_write(backend, &len, sizeof(len));
		pool_write(backend, "S", 1);
		pool_write(backend, prepared_name, pname_len);

		/*
		 * Send descrive message if the query is SELECT.
		 */
		if (!strcasecmp(query, "SELECT"))
		{
			/*
			 * Send descrive message
			 */
			pool_write(backend, "D", 1);
			len = 4 + 1 + pname_len;
			len = htonl(len);
			pool_write(backend, &len, sizeof(len));
			pool_write(backend, "P", 1);
			pool_write(backend, prepared_name, pname_len);
		}

		/*
		 * Send execute message
		 */
		pool_write(backend, "E", 1);
		len = 4 + pname_len + 4;
		len = htonl(len);
		pool_write(backend, &len, sizeof(len));
		pool_write(backend, prepared_name, pname_len);
		len = htonl(0);
		pool_write(backend, &len, sizeof(len));

		/*
		 * Send close portal message
		 */
		pool_write(backend, "C", 1);
		len = 4 + 1 + pname_len;
		len = htonl(len);
		pool_write(backend, &len, sizeof(len));
		pool_write(backend, "P", 1);
		pool_write(backend, prepared_name, pname_len);

		/*
		 * Send flush message
		 */
		pool_write(backend, "H", 1);
		len = htonl(sizeof(len));
		pool_write_and_flush(backend, &len, sizeof(len));
	}
	else
	{
		send_simplequery_message(backend, strlen(query) + 1, query, major);
	}

	/*
	 * Continue to read packets until we get Ready for command('Z')
	 *
	 * XXX: we ignore other than Z here. Even notice messages are not sent
	 * to the frontend. May be it's ok since the error was caused by
	 * our internal use of SQL command (otherwise users will be
	 * confused).
	 */

	num_close_complete = 0;
	state = 0;

	for(;;)
	{
		pool_read(backend, &kind, sizeof(kind));

		ereport(DEBUG1,
				(errmsg("do_query: kind: '%c'", kind)));

		if (kind ==  'E')
		{
			char *message = NULL;

			if (pool_extract_error_message(false, backend, major, true, &message))
			{
				/*
				 * This is fatal. Because: If we operate extended
				 * query, backend would not accept subsequent commands
				 * until "sync" message issued. However, if sync
				 * message is issued, unnamed statement/unnamed portal
				 * will disappear and will cause lots of problems.  If
				 * we do not operate extended query, ongoing
				 * transaction is aborted, and subsequent query would
				 * not accepted.  In summary there's no transparent
				 * way for frontend to handle error case. The only way
				 * is closing this session.
				 */
                ereport(FATAL,
                    (return_code(1),
                        errmsg("Backend throw an error message"),
                         errdetail("Exiting current session because of an error from backend"),
                            errhint("BACKEND Error: \"%s\"",message?message:"")));
			}
		}

		if (major == PROTO_MAJOR_V3)
		{
			pool_read(backend, &len, sizeof(len));

			len = ntohl(len) - 4;

			if (len > 0)
			{
				packet = pool_read2(backend, len);
				if (packet == NULL)
                    ereport(ERROR,
                            (errmsg("do query failed"),
                             errdetail("error while reading rest of message")));

			}
		}
		else
		{
			mask = 0;

			if (kind == 'C' || kind == 'E' || kind == 'N' || kind == 'P')
			{
				if  (pool_read_string(backend, &len, 0) == NULL)
				{
                    ereport(ERROR,
						(errmsg("do query failed"),
                             errdetail("error while reading string of message type %c", kind)));
				}
			}
		}

		switch (kind)
		{
			case 'Z':	/* Ready for query */
				ereport(DEBUG1,
					(errmsg("do_query: received READY FOR QUERY ('%c')",kind)));
				if (!doing_extended)
					return;
				break;

			case 'C':	/* Command Complete */
				ereport(DEBUG1,
						(errmsg("do_query: received COMMAND COMPLETE ('%c')",kind)));
				state |= COMMAND_COMPLETE_RECEIVED;
				/*
				 * "Comand Complete" implies data row received status
				 * if the query was SELECT.  If there's no row
				 * returned, "command complete" comes without row
				 * data.
				 */
				state |= DATA_ROW_RECEIVED;
				/*
				 * For other than SELECT, ROW_DESCRIPTION_RECEIVED
				 * should be set because we didn't issue DESCRIBE
				 * message.
				 */
				state |= ROW_DESCRIPTION_RECEIVED;
				break;

			case '1':	/* Parse complete */
				ereport(DEBUG1,
						(errmsg("do_query: received PARSE COMPLETE ('%c')",kind)));

				state |= PARSE_COMPLETE_RECEIVED;
				break;

			case '2':	/* Bind complete */
				ereport(DEBUG1,
						(errmsg("do_query: received BIND COMPLETE ('%c')",kind)));
				state |= BIND_COMPLETE_RECEIVED;
				break;

			case '3':	/* Close complete */
				ereport(DEBUG1,
						(errmsg("do_query: received CLOSE COMPLETE ('%c')",kind)));
				num_close_complete++;
				if (num_close_complete >= 2)
					state |= CLOSE_COMPLETE_RECEIVED;
				break;

			case 'T':	/* Row Description */
				state |= ROW_DESCRIPTION_RECEIVED;
				ereport(DEBUG1,
						(errmsg("do_query: received ROW DESCRIPTION ('%c')",kind)));

				if (major == PROTO_MAJOR_V3)
				{
					if(packet)
					{
						p = packet;
						memcpy(&shortval, p, sizeof(short));
						p += sizeof(num_fields);
					}
					else
					{
						ereport(ERROR,
							(errmsg("do query failed, no data received for ROW DESCRIPTION ('%c') packet",kind)));
					}
				}
				else
				{
					pool_read(backend, &shortval, sizeof(short));
				}
				num_fields = ntohs(shortval);		/* number of fields */
				ereport(DEBUG1,
						(errmsg("do_query: row description: num_fileds: %d", num_fields)));

				if (num_fields > 0)
				{
					rowdesc->num_attrs = num_fields;
					attrinfo = palloc(sizeof(*attrinfo)*num_fields);
					rowdesc->attrinfo = attrinfo;

					/* extract attribute info */
					for (i = 0;i<num_fields;i++)
					{
						if (major == PROTO_MAJOR_V3)
						{
							len = strlen(p) + 1;
							attrinfo->attrname = palloc(len);
							memcpy(attrinfo->attrname, p, len);
							p += len;
							memcpy(&intval, p, sizeof(int));
							attrinfo->oid = htonl(intval);
							p += sizeof(int);
							memcpy(&shortval, p, sizeof(short));
							attrinfo->attrnumber = htons(shortval);
							p += sizeof(short);
							memcpy(&intval, p, sizeof(int));
							attrinfo->typeoid = htonl(intval);
							p += sizeof(int);
							memcpy(&shortval, p, sizeof(short));
							attrinfo->size = htons(shortval);
							p += sizeof(short);
							memcpy(&intval, p, sizeof(int));
							attrinfo->mod = htonl(intval);
							p += sizeof(int);
							p += sizeof(short);		/* skip format code since we use "text" anyway */
						}
						else
						{
							p = pool_read_string(backend, &len, 0);
							attrinfo->attrname = palloc(len);
							memcpy(attrinfo->attrname, p, len);
							pool_read(backend, &intval, sizeof(int));
							attrinfo->typeoid = ntohl(intval);
							pool_read(backend, &shortval, sizeof(short));
							attrinfo->size = ntohs(shortval);
							pool_read(backend, &intval, sizeof(int));
							attrinfo->mod = ntohl(intval);
						}

						attrinfo++;
					}
				}
				break;

			case 'D':	/* data row */
				state |= DATA_ROW_RECEIVED;
				ereport(DEBUG1,
						(errmsg("do_query: received DATA ROW ('%c')",kind)));

				if (major == PROTO_MAJOR_V3)
				{
					p = packet;
					if(p)
					{
						memcpy(&shortval, p, sizeof(short));
						num_fields = htons(shortval);
						p += sizeof(short);
					}
					else
						ereport(ERROR,
							(errmsg("do query failed"),
								 errdetail("error while reading data")));

				}

				if (num_fields > 0)
				{
 					if (major == PROTO_MAJOR_V2)
 					{
 						nbytes = (num_fields + 7)/8;
 
 						if (nbytes <= 0)
                            ereport(ERROR,
								(errmsg("do query failed"),
                                     errdetail("error while reading null bitmap")));

 
 						pool_read(backend, nullmap, nbytes);
 					}

					res->numrows++;

					for (i=0;i<num_fields;i++)
					{
						if (major == PROTO_MAJOR_V3)
						{
							memcpy(&intval, p, sizeof(int));
							len = htonl(intval);
							p += sizeof(int);

							res->nullflags[num_data] = len;

							if (len > 0)	/* NOT NULL? */
							{
								res->data[num_data] = palloc(len + 1);
								memcpy(res->data[num_data], p, len);
								*(res->data[num_data] + len) = '\0';

								p += len;
							}
						}
						else
						{
							if (mask == 0)
								mask = 0x80;

							/* NOT NULL? */
							if (mask & nullmap[i/8])
							{
								/* field size */
								pool_read(backend, &len, sizeof(int));
								len = ntohl(len) - 4;

								res->nullflags[num_data] = len;

								if (len > 0)
								{
									p = pool_read2(backend, len);
									res->data[num_data] = palloc(len + 1);
									memcpy(res->data[num_data], p, len);
									*(res->data[num_data] + len) = '\0';
									if (res->data[num_data] == NULL)
                                        ereport(ERROR,
											(errmsg("do query failed"),
                                                 errdetail("error while reading field data")));
								}
							}
							else
							{
								res->nullflags[num_data] = -1;
							}

							mask >>= 1;
						}

						num_data++;

						if (num_data % DO_QUERY_ALLOC_NUM == 0)
						{
							res->nullflags = repalloc(res->nullflags,
													 (num_data/DO_QUERY_ALLOC_NUM +1)*DO_QUERY_ALLOC_NUM*sizeof(int));
							res->data = repalloc(res->data,
												(num_data/DO_QUERY_ALLOC_NUM +1)*DO_QUERY_ALLOC_NUM*sizeof(char *));
						}
					}
				}
				break;

			default:
				break;
		}

		if (doing_extended && (state & STATE_COMPLETED) == STATE_COMPLETED)
		{
			ereport(DEBUG1,
					(errmsg("do_query: all state completed. returning")));
			break;
		}
	}
}

/*
 * Judge if we need to lock the table
 * to keep SERIAL consistency among servers
 * Return values are:
 * 0: lock is not neccessary
 * 1: table lock is required
 * 2: row lock against sequence table is required
 * 3: row lock against insert_lock table is required
 */
int need_insert_lock(POOL_CONNECTION_POOL *backend, char *query, Node *node)
{
/*
 * Query to know if the target table has SERIAL column or not.
 * This query is valid through PostgreSQL 7.3 or higher.
 */
#define NEXTVALQUERY "SELECT count(*) FROM pg_catalog.pg_attrdef AS d, pg_catalog.pg_class AS c WHERE d.adrelid = c.oid AND d.adsrc ~ 'nextval' AND c.relname = '%s'"

#define NEXTVALQUERY2 "SELECT count(*) FROM pg_catalog.pg_attrdef AS d, pg_catalog.pg_class AS c WHERE d.adrelid = c.oid AND d.adsrc ~ 'nextval' AND c.oid = pgpool_regclass('%s')"

#define NEXTVALQUERY3 "SELECT count(*) FROM pg_catalog.pg_attrdef AS d, pg_catalog.pg_class AS c WHERE d.adrelid = c.oid AND d.adsrc ~ 'nextval' AND c.oid = to_regclass('%s')"

	char *str;
	int result;
	static POOL_RELCACHE *relcache;

	/* INSERT statement? */
	if (!IsA(node, InsertStmt))
		return 0;

	/* need to ignore leading white spaces? */
	if (pool_config->ignore_leading_white_space)
	{
		/* ignore leading white spaces */
		while (*query && isspace(*query))
			query++;
	}

	/* is there "NO_LOCK" comment? */
	if (strncasecmp(query, NO_LOCK_COMMENT, NO_LOCK_COMMENT_SZ) == 0)
		return 0;

	/* is there "LOCK" comment? */
	if (strncasecmp(query, LOCK_COMMENT, LOCK_COMMENT_SZ) == 0)
		return 1;

	if (pool_config->insert_lock == 0)	/* insert_lock is specified? */
		return 0;

	/*
	 * if insert_lock is true, then check if the table actually uses
	 * SERIAL data type
	 */

	/* obtain table name */
	str = get_insert_command_table_name((InsertStmt *)node);
	if (str == NULL)
	{
		ereport(LOG,
				(errmsg("unable to get table name from insert statement while checking if table locking is required")));
		return 0;
	}

	/*
	 * If relcache does not exist, create it.
	 */
	if (!relcache)
	{
		char *query;

		/* PostgreSQL 9.4 or later has to_regclass() */
		if (pool_has_to_regclass())
		{
			query = NEXTVALQUERY3;
		}
		else if (pool_has_pgpool_regclass())
		{
			query = NEXTVALQUERY2;
		}
		else
		{
			query = NEXTVALQUERY;
		}

		relcache = pool_create_relcache(pool_config->relcache_size, query,
										int_register_func, int_unregister_func,
										false);
		if (relcache == NULL)
		{
			ereport(LOG,
					(errmsg("unable to create relcache while checking if table locking is required")));
			return 0;
		}
	}

	/*
	 * Search relcache.
	 */
#ifdef USE_TABLE_LOCK
	result = pool_search_relcache(relcache, backend, str)==0?0:1;
#elif USE_SEQUENCE_LOCK
	result = pool_search_relcache(relcache, backend, str)==0?0:2;
#else
	result = pool_search_relcache(relcache, backend, str)==0?0:3;
#endif
	return result;
}

/*
 * insert lock to synchronize sequence number
 * lock_kind are:
 * 1: Issue LOCK TABLE IN SHARE ROW EXCLUSIVE MODE
 * 2: Issue row lock against sequence table
 * 3: Issue row lock against pgpool_catalog.insert_lock table
 * "lock_kind == 2" is deprecated because PostgreSQL disallows 
 * SELECT FOR UPDATE/SHARE on sequence tables since 2011/06/03.
 * See following threads for more details: 
 * [HACKERS] pgpool versus sequences
 * [ADMIN] 'SGT DETAIL: Could not open file "pg_clog/05DC": ...
 */
POOL_STATUS insert_lock(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char *query, InsertStmt *node, int lock_kind)
{
	char *table, *p;
	int len = 0;
	char qbuf[1024];
	int i, deadlock_detected = 0;
	char *adsrc;
	char seq_rel_name[MAX_NAME_LEN+1];
	char rel_name[MAX_NAME_LEN+1];
	regex_t preg;
	size_t nmatch = 2;
	regmatch_t pmatch[nmatch];
	static POOL_RELCACHE *relcache;
	POOL_SELECT_RESULT *result;
	POOL_STATUS status = POOL_CONTINUE;

	/* insert_lock can be used in V3 only */
	if (MAJOR(backend) != PROTO_MAJOR_V3)
		return POOL_CONTINUE;

	/* get table name */
	table = get_insert_command_table_name(node);

	/* could not get table name. probably wrong SQL command */
	if (table == NULL)
	{
		return POOL_CONTINUE;
	}

	/* trim quotes */
	p = table;
	for (i=0; *p; p++)
	{
		if (*p != '"')
			rel_name[i++] = *p;
	}
	rel_name[i] = '\0';

	/* table lock for insert target table? */
	if (lock_kind == 1)
	{
		/* Issue lock table command */
		snprintf(qbuf, sizeof(qbuf), "LOCK TABLE %s IN SHARE ROW EXCLUSIVE MODE", table);
	}
	/* row lock for sequence table? */
	else if (lock_kind == 2)
	{
		/*
		 * If relcache does not exist, create it.
		 */
		if (!relcache)
		{
			char *query;

			/* PostgreSQL 9.4 or later has to_regclass() */
			if (pool_has_to_regclass())
			{
				query = SEQUENCETABLEQUERY3;
			}
			else if (pool_has_pgpool_regclass())
			{
				query = SEQUENCETABLEQUERY2;
			}
			else
			{
				query = SEQUENCETABLEQUERY;
			}

			relcache = pool_create_relcache(pool_config->relcache_size, query,
											string_register_func, string_unregister_func,
											false);
			if (relcache == NULL)
			{
                ereport(FATAL,
					(return_code(2),
                        errmsg("unable to get insert lock on table"),
							errdetail("error while creating relcache")));
			}
		}

		/*
		 * Search relcache.
		 */
		adsrc = pool_search_relcache(relcache, backend, table);
		if (adsrc == NULL)
		{
			/* could not get adsrc */
			return POOL_CONTINUE;
		}
		ereport(DEBUG1,
			(errmsg("getting insert lock on table"),
				 errdetail("adsrc: \"%s\"", adsrc)));

		if (regcomp(&preg, "nextval\\(+'(.+)'", REG_EXTENDED|REG_NEWLINE) != 0)
            ereport(FATAL,
				(return_code(2),
                     errmsg("unable to get insert lock on table"),
                      errdetail("regex compile failed")));

		if (regexec(&preg, adsrc, nmatch, pmatch, 0) == 0)
		{
			/* pmatch[0] is "nextval('...'", pmatch[1] is sequence name */
			if (pmatch[1].rm_so >= 0 && pmatch[1].rm_eo >= 0)
			{
				len = pmatch[1].rm_eo - pmatch[1].rm_so;
				strncpy(seq_rel_name, adsrc + pmatch[1].rm_so, len);
				*(seq_rel_name + len) = '\0';
			}
		}
		regfree(&preg);

		if (len == 0)
            ereport(FATAL,
				(return_code(2),
                     errmsg("unable to get insert lock on table"),
						errdetail("regex no match: pg_attrdef is %s", adsrc)));

		ereport(DEBUG1,
			(errmsg("getting insert lock on table"),
				 errdetail("seq rel name: %s", seq_rel_name)));
		snprintf(qbuf, sizeof(qbuf), "SELECT 1 FROM %s FOR UPDATE", seq_rel_name);
	}
	/* row lock for insert_lock table? */
	else
	{
		if (pool_has_insert_lock())
		{
			if (pool_has_pgpool_regclass())
				snprintf(qbuf, sizeof(qbuf), ROWLOCKQUERY2, rel_name);
			else
				snprintf(qbuf, sizeof(qbuf), ROWLOCKQUERY, rel_name);
		}
		else
		{
			/* issue lock table command if insert_lock table does not exist */
			lock_kind = 1;
			snprintf(qbuf, sizeof(qbuf), "LOCK TABLE %s IN SHARE ROW EXCLUSIVE MODE", table);
		}
	}

	per_node_statement_log(backend, MASTER_NODE_ID, qbuf);

	if (lock_kind == 1)
	{
		if (pool_get_session_context(true) && pool_is_doing_extended_query_message())
		{
			do_query(MASTER(backend), qbuf, &result, MAJOR(backend));
		}
		else
		{
			status = do_command(frontend, MASTER(backend), qbuf, MAJOR(backend), MASTER_CONNECTION(backend)->pid,
								MASTER_CONNECTION(backend)->key, 0);
		}
	}
	else if (lock_kind == 2)
	{
		do_query(MASTER(backend), qbuf, &result, MAJOR(backend));
	}
	else
	{
		POOL_SELECT_RESULT *result;
		/* issue row lock command */
		do_query(MASTER(backend), qbuf, &result, MAJOR(backend));
		if (status == POOL_CONTINUE)
		{
			/* does oid exist in insert_lock table? */
			if (result && result->numrows == 0)
			{
				free_select_result(result);
				result = NULL;

				/* insert a lock target row into insert_lock table */
				status = add_lock_target(frontend, backend, rel_name);
				if (status == POOL_CONTINUE)
				{
					per_node_statement_log(backend, MASTER_NODE_ID, qbuf);

					/* issue row lock command */
					do_query(MASTER(backend), qbuf, &result, MAJOR(backend));

					if (!(result && result->data[0] && !strcmp(result->data[0], "1")))
						ereport(FATAL,
							(return_code(2),
								 errmsg("failed to get insert lock"),
									errdetail("unexpected data from backend")));
				}
			}
		}

		if (result)
			free_select_result(result);

		if (status != POOL_CONTINUE)
		{
			/* try to lock table finally, if row lock failed */
			lock_kind = 1;
			snprintf(qbuf, sizeof(qbuf), "LOCK TABLE %s IN SHARE ROW EXCLUSIVE MODE", table);
			per_node_statement_log(backend, MASTER_NODE_ID, qbuf);

			if (pool_get_session_context(true) && pool_is_doing_extended_query_message())
			{
				do_query(MASTER(backend), qbuf, &result, MAJOR(backend));
				if (result)
					free_select_result(result);
			}
			else
			{
				status = do_command(frontend, MASTER(backend), qbuf, MAJOR(backend), MASTER_CONNECTION(backend)->pid,
									MASTER_CONNECTION(backend)->key, 0);
			}
		}
	}

	if (status == POOL_END)
	{
		return POOL_END;
	}
	else if (status == POOL_DEADLOCK)
		deadlock_detected = 1;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
		{
			if (deadlock_detected)
				status = do_command(frontend, CONNECTION(backend, i), POOL_ERROR_QUERY, PROTO_MAJOR_V3,
									MASTER_CONNECTION(backend)->pid, MASTER_CONNECTION(backend)->key, 0);
			else
			{
				if (lock_kind == 1)
				{
					per_node_statement_log(backend, i, qbuf);
					if (pool_get_session_context(true) && pool_is_doing_extended_query_message())
					{
						do_query(MASTER(backend), qbuf, &result, MAJOR(backend));
						if (result)
							free_select_result(result);
					}
					else
					{
						status = do_command(frontend, CONNECTION(backend, i), qbuf, PROTO_MAJOR_V3, 
											MASTER_CONNECTION(backend)->pid, MASTER_CONNECTION(backend)->key, 0);
					}
				}
				else if (lock_kind == 2)
				{
					POOL_SELECT_RESULT *result;
					per_node_statement_log(backend, i, qbuf);
					do_query(CONNECTION(backend,i), qbuf, &result, MAJOR(backend));
				}
			}

			if (status != POOL_CONTINUE)
			{
				return POOL_END;
			}
		}
	}

	return POOL_CONTINUE;
}

/*
 * Judge if we have insert_lock table or not.
 */
static bool pool_has_insert_lock(void)
{
/*
 * Query to know if insert_lock table exists.
 */

	bool result;
	static POOL_RELCACHE *relcache;
	POOL_CONNECTION_POOL *backend;

	backend = pool_get_session_context(false)->backend;

	if (!relcache)
	{
		relcache = pool_create_relcache(pool_config->relcache_size, HASINSERT_LOCKQUERY,
										int_register_func, int_unregister_func,
										false);
		if (relcache == NULL)
            ereport(FATAL,
                    (return_code(2),
                     errmsg("unable to get insert lock status"),
                     errdetail("error while creating relcache")));

	}
	result = pool_search_relcache(relcache, backend, "insert_lock")==0?0:1;
	return result;
}

/*
 * Insert a lock target row into insert_lock table.
 * This function is called after the transaction has been started.
 * Protocol is V3 only.
 * Return POOL_CONTINUE if the row is inserted successfully
 * or the row already exists, the others return POOL_ERROR.
 */
static POOL_STATUS add_lock_target(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char* table)
{
	/* lock the row where reloid is 0 to avoid "duplicate key violates..." error when insert new oid */
	if (!has_lock_target(frontend, backend, NULL, true))
	{
        ereport(LOG,
                (errmsg("add lock target: not lock the row where reloid is 0")));

		per_node_statement_log(backend, MASTER_NODE_ID, "LOCK TABLE pgpool_catalog.insert_lock IN SHARE ROW EXCLUSIVE MODE");

		if(do_command(frontend, MASTER(backend), "LOCK TABLE pgpool_catalog.insert_lock IN SHARE ROW EXCLUSIVE MODE",
							PROTO_MAJOR_V3, MASTER_CONNECTION(backend)->pid, MASTER_CONNECTION(backend)->key, 0) != POOL_CONTINUE)
            ereport(ERROR,
                    (errmsg("unable to add lock target"),
                     errdetail("do_command returned DEADLOCK status")));

        if (has_lock_target(frontend, backend, NULL, false))
        {
            ereport(DEBUG1,
                    (errmsg("add lock target: reloid 0 already exists in insert_lock table")));
        }
        else
        {
            if (insert_oid_into_insert_lock(frontend, backend, NULL) != POOL_CONTINUE)
            {
                ereport(ERROR,
                        (errmsg("unable to start the internal transaction"),
                         errdetail("failed to insert 0 into insert_lock table")));
            }
        }
	}

	/* does insert_lock table contain the oid of the table? */
	if (has_lock_target(frontend, backend, table, false))
	{
		ereport(DEBUG1,
				(errmsg("add_lock_target: \"%s\" oid already exists in insert_lock table", table)));

		return POOL_CONTINUE;
	}

	/* insert the oid of the table into insert_lock table */
	if (insert_oid_into_insert_lock(frontend, backend, table) != POOL_CONTINUE)
        ereport(ERROR,
			(errmsg("unable to start the internal transaction"),
                 errdetail("could not insert \"%s\" oid into insert_lock table", table)));


	return POOL_CONTINUE;
}

/*
 * Judge if insert_lock table contains the oid of the specified table or not.
 * If the table name is NULL, this function checks whether oid 0 exists.
 * If lock is true, this function locks the row of the table oid.
 */
static bool has_lock_target(POOL_CONNECTION *frontend,
							POOL_CONNECTION_POOL *backend,
							char* table, bool lock)
{
	char *suffix;
	char qbuf[QUERY_STRING_BUFFER_LEN];
	POOL_SELECT_RESULT *result;

	suffix = lock ? " FOR UPDATE" : "";

	if (table)
	{
		if (pool_has_pgpool_regclass())
			snprintf(qbuf, sizeof(qbuf), "SELECT 1 FROM pgpool_catalog.insert_lock WHERE reloid = pgpool_regclass('%s')%s", table, suffix);
		else
			snprintf(qbuf, sizeof(qbuf), "SELECT 1 FROM pgpool_catalog.insert_lock WHERE reloid = (SELECT oid FROM pg_catalog.pg_class WHERE relname = '%s' ORDER BY oid LIMIT 1)%s", table, suffix);
	}
	else
	{
		snprintf(qbuf, sizeof(qbuf), "SELECT 1 FROM pgpool_catalog.insert_lock WHERE reloid = 0%s", suffix);
	}

	per_node_statement_log(backend, MASTER_NODE_ID, qbuf);
	do_query(MASTER(backend), qbuf, &result, MAJOR(backend));
	if (result && result->data[0] && !strcmp(result->data[0], "1"))
	{
		free_select_result(result);
		return true;
	}

	if (result)
		free_select_result(result);

	return false;
}

/*
 * Insert the oid of the specified table into insert_lock table.
 */
static POOL_STATUS insert_oid_into_insert_lock(POOL_CONNECTION *frontend,
											   POOL_CONNECTION_POOL *backend,
											   char* table)
{
	char qbuf[QUERY_STRING_BUFFER_LEN];
	POOL_STATUS status;

	if (table)
	{
		if (pool_has_pgpool_regclass())
			snprintf(qbuf, sizeof(qbuf), "INSERT INTO pgpool_catalog.insert_lock VALUES (pgpool_regclass('%s'))", table);
		else
			snprintf(qbuf, sizeof(qbuf), "INSERT INTO pgpool_catalog.insert_lock SELECT oid FROM pg_catalog.pg_class WHERE relname = '%s' ORDER BY oid LIMIT 1", table);
	}
	else
	{
		snprintf(qbuf, sizeof(qbuf), "INSERT INTO pgpool_catalog.insert_lock VALUES (0)");
	}

	per_node_statement_log(backend, MASTER_NODE_ID, qbuf);
	status = do_command(frontend, MASTER(backend), qbuf, PROTO_MAJOR_V3,
						MASTER_CONNECTION(backend)->pid, MASTER_CONNECTION(backend)->key, 0);
	return status;
}

bool is_partition_table(POOL_CONNECTION_POOL *backend, Node *node)
{
	DistDefInfo *info = NULL;
	RangeVar *var = NULL;;

	if (IsA(node, UpdateStmt))
	{
		UpdateStmt *update = (UpdateStmt*) node;

		if(!IsA(update->relation,RangeVar))
			return false;

		var = (RangeVar *) update->relation;
	}
	else if (IsA(node, DeleteStmt))
	{
		DeleteStmt *delete = (DeleteStmt*) node;

		if(!IsA(delete->relation,RangeVar))
			return false;

		var = (RangeVar *) delete->relation;
	} else
		return false;

	info = pool_get_dist_def_info(MASTER_CONNECTION(backend)->sp->database,
									  var->schemaname,
									  var->relname);
	if(info)
		return true;
	else
		return false;
}

/*
 * Obtain table name in INSERT statement.
 * The table name is stored in the static buffer of this function.
 * So subsequent call to this function will destroy previous result.
 */
static char *get_insert_command_table_name(InsertStmt *node)
{
	POOL_SESSION_CONTEXT *session_context;
	static char table[128];
	char *p;

	session_context = pool_get_session_context(true);
	if (!session_context)
		return NULL;
	p = nodeToString(node->relation);
	if (p == NULL)
	{
		ereport(DEBUG1,
				(errmsg("unable to get table name from insert command")));
		return NULL;
	}
	strlcpy(table, p, sizeof(table));
	pfree(p);

	ereport(DEBUG2,
            (errmsg("table name in insert command is \"%s\"", table)));
	return table;
}

/* judge if this is a DROP DATABASE command */
int is_drop_database(Node *node)
{
	return (node && IsA(node, DropdbStmt)) ? 1 : 0;
}

/*
 * check if any pending data remains.
*/
static bool is_cache_empty(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	int i;

	/*
	 * If SSL is enabled, we need to check SSL internal buffer
	 * is empty or not first.
	 */
	if (pool_ssl_pending(frontend))
		return false;

	if (!pool_read_buffer_is_empty(frontend))
		return false;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		/*
		 * If SSL is enabled, we need to check SSL internal buffer
		 * is empty or not first.
		 */
		if (pool_ssl_pending(CONNECTION(backend, i)))
			return false;

		if (!pool_read_buffer_is_empty(CONNECTION(backend, i)))
			return false;
	}

	return true;
}

/*
 * check if query is needed to wait completion
 */
bool is_strict_query(Node *node)
{
	switch (node->type)
	{
		case T_SelectStmt:
		{
			SelectStmt *stmt = (SelectStmt *)node;
			return stmt->intoClause != NULL|| stmt->lockingClause != NIL;
		}

		case T_UpdateStmt:
		case T_InsertStmt:
		case T_DeleteStmt:
		case T_LockStmt:
			return true;

		default:
			return false;
	}

	return false;
}

int check_copy_from_stdin(Node *node)
{
	if (copy_schema)
		pfree(copy_schema);
	if (copy_table)
		pfree(copy_table);
	if (copy_null)
		pfree(copy_null);

	copy_schema = copy_table = copy_null = NULL;

	if (IsA(node, CopyStmt))
	{
		CopyStmt *stmt = (CopyStmt *)node;
		if (stmt->is_from == TRUE && stmt->filename == NULL)
		{
			RangeVar *relation = (RangeVar *)stmt->relation;
			ListCell *lc;

			/* query is COPY FROM STDIN */
			if (relation->schemaname)
				copy_schema = MemoryContextStrdup(TopMemoryContext,relation->schemaname);
			else
				copy_schema = MemoryContextStrdup(TopMemoryContext,"public");

			copy_table = MemoryContextStrdup(TopMemoryContext,relation->relname);

			copy_delimiter = '\t'; /* default delimiter */
			copy_null = MemoryContextStrdup(TopMemoryContext,"\\N"); /* default null string */

			/* look up delimiter and null string. */
			foreach (lc, stmt->options)
			{
				DefElem *elem = lfirst(lc);
				Value *v;

				if (strcmp(elem->defname, "delimiter") == 0)
				{
					v = (Value *)elem->arg;
					copy_delimiter = v->val.str[0];
				}
				else if (strcmp(elem->defname, "null") == 0)
				{
					if (copy_null)
						pfree(copy_null);
					v = (Value *)elem->arg;
					copy_null = MemoryContextStrdup(TopMemoryContext,v->val.str);
				}
			}
		}
		return 1;
	}

	return 0;
}

/*
 * read kind from one backend
 */
void read_kind_from_one_backend(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char *kind, int node)
{
	if (VALID_BACKEND(node))
	{
		char k;
		pool_read(CONNECTION(backend, node), &k, 1);
		*kind = k;
	}
	else
	{
		pool_dump_valid_backend(node);
        ereport(ERROR,
            (errmsg("unable to read message kind from backend"),
                 errdetail("%d th backend is not valid", node)));
	}
}

/*
 * returns true if all slaves status are 'C' (Command Complete)
 */
static bool is_all_slaves_command_complete(unsigned char *kind_list, int num_backends, int master)
{
	int i;
	int ok = true;

	for (i=0;i<num_backends;i++)
	{
		if (i == master || kind_list[i] == 0)
			continue;
		if (kind_list[i] != 'C')
		{
			ok = false;
			break;
		}
	}
	return ok;
}
		
/*
 * read_kind_from_backend: read kind from backends.
 * the "frontend" parameter is used to send "kind mismatch" error message to the frontend.
 * the out parameter "decided_kind" is the packet kind decided by this function.
 * this function uses "decide by majority" method if kinds from all backends do not agree.
 */
void read_kind_from_backend(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char *decided_kind)
{
	int i;
	unsigned char kind_list[MAX_NUM_BACKENDS];	/* records each backend's kind */
	unsigned char kind_map[256];                /* records which kind gets majority.
                                                 *	256 is the number of distinct values expressed by unsigned char
                                                 */
	unsigned char kind;
	int trust_kind;                              /* decided kind */
	int max_kind = 0;
	double max_count = 0;
	int degenerate_node_num = 0;                /* number of backends degeneration requested */
	int degenerate_node[MAX_NUM_BACKENDS];      /* degeneration requested backend list */

	POOL_SESSION_CONTEXT *session_context = pool_get_session_context(false);
	POOL_QUERY_CONTEXT *query_context = session_context->query_context;

	int num_executed_nodes = 0;
	int first_node = -1;

	memset(kind_map, 0, sizeof(kind_map));

	if (MASTER_SLAVE)
	{
		read_kind_from_one_backend(frontend, backend, (char *)&kind, MASTER_NODE_ID);

		/*
		 * If we received a notification message in master/slave mode,
		 * other backends will not receive the message.
		 * So we should skip other nodes otherwise we will hung in pool_read.
		 */
		if (kind == 'A')	
		{
			*decided_kind = 'A';
			
			ereport(LOG,
				(errmsg("reading backend data packet kind"),
					 errdetail("received notification message for master node %d",
							   MASTER_NODE_ID)));

			return;
		}
		pool_unread(CONNECTION(backend, MASTER_NODE_ID), &kind, sizeof(kind));
	}

	for (i=0;i<NUM_BACKENDS;i++)
	{
		/* initialize degenerate record */
		degenerate_node[i] = 0;

		if (VALID_BACKEND(i))
		{
			num_executed_nodes++;

			if (first_node < 0)
				first_node = i;

			do
			{
				char *p, *value;
				int len;

				pool_read(CONNECTION(backend, i), &kind, 1);
				ereport(DEBUG2,
					(errmsg("reading backend data packet kind"),
						 errdetail("backend:%d kind:'%c'",i, kind)));

				/*
				 * Read and discard parameter status
				 */
				if (kind != 'S')
				{
					break;
				}

				pool_read(CONNECTION(backend, i), &len, sizeof(len));
				len = htonl(len) - 4;
				p = pool_read2(CONNECTION(backend, i), len);
				if (p)
				{
					value = p + strlen(p) + 1;
					ereport(DEBUG1,
						(errmsg("reading backend data packet kind"),
							 errdetail("parameter name: %s value: \"%s\"", p, value)));

					if (IS_MASTER_NODE_ID(i))
						pool_add_param(&CONNECTION(backend, i)->params, p, value);
				}
				else
					ereport(WARNING,
						(errmsg("failed to read parameter status packet from backend %d", i),
							 errdetail("read from backend failed")));

			} while (kind == 'S');

#ifdef DEALLOCATE_ERROR_TEST
			if (i == 1 && kind == 'C' &&
				pending_function && pending_prepared_portal &&
				IsA(pending_prepared_portal->stmt, DeallocateStmt))
				kind = 'E';
#endif

			kind_list[i] = kind;

			ereport(DEBUG1,
				(errmsg("reading backend data packet kind"),
					 errdetail("backend:%d of %d kind = '%c'", i,NUM_BACKENDS, kind_list[i])));

			kind_map[kind]++;

			if (kind_map[kind] > max_count)
			{
				max_kind = kind_list[i];
				max_count = kind_map[kind];
			}
		}
		else
			kind_list[i] = 0;
	}

	if (max_count != num_executed_nodes)
	{
		/*
		 * not all backends agree with kind. We need to do "decide by majority"
		 */

		/*
		 * However here is a special case. In master slave mode, if
		 * master gets an error at commit, while other slaves are
		 * normal at commit, we don't need to degenrate any backend
		 * because it is likely that the error was caused by a
		 * deferred trigger.
		 */
		if (MASTER_SLAVE && query_context->parse_tree &&
			is_commit_query(query_context->parse_tree) &&
			kind_list[MASTER_NODE_ID] == 'E' &&
			is_all_slaves_command_complete(kind_list, NUM_BACKENDS, MASTER_NODE_ID))
		{
			*decided_kind = kind_list[MASTER_NODE_ID];
			ereport(LOG,
				(errmsg("reading backend data packet kind. Error on master while all slaves are normal"),
					 errdetail("do not degenerate because it is likely caused by a delayed commit")));

			return;
		}
		else if (max_count <= NUM_BACKENDS / 2.0)
		{
			/* no one gets majority. We trust master node's kind */
			trust_kind = kind_list[MASTER_NODE_ID];
		}
		else /* max_count > NUM_BACKENDS / 2.0 */
		{
			/* trust majority's kind */
			trust_kind = max_kind;
		}

		for (i = 0; i < NUM_BACKENDS; i++)
		{
			if (session_context->in_progress && !pool_is_node_to_be_sent(query_context, i))
				continue;

			if (kind_list[i] != 0 && trust_kind != kind_list[i])
			{
				/* degenerate */
				ereport(WARNING,
					(errmsg("packet kind of backend %d ['%c'] does not match with master/majority nodes packet kind ['%c']",i,kind_list[i],trust_kind)));
				degenerate_node[degenerate_node_num++] = i;
			}
		}
	}
	else if (first_node == -1)
	{
        ereport(FATAL,
            (return_code(2),
                 errmsg("failed to read kind from backend"),
                    errdetail("couldn't find first node. All backend down?")));
	}
	else
		trust_kind = kind_list[first_node];

	*decided_kind = trust_kind;

	if (degenerate_node_num)
	{
        int retcode = 2;
		String *msg = init_string("kind mismatch among backends. ");

		string_append_char(msg, "Possible last query was: \"");
		string_append_char(msg, query_string_buffer);
		string_append_char(msg, "\" kind details are:");

		for (i=0;i<NUM_BACKENDS;i++)
		{
			char buf[32];

			if (kind_list[i])
			{

				if (kind_list[i] == 'E' || kind_list[i] == 'N')
				{
					char *m;

					snprintf(buf, sizeof(buf), " %d[%c: ", i, kind_list[i]);
					string_append_char(msg, buf);

					if (pool_extract_error_message(false, CONNECTION(backend, i), MAJOR(backend), true, &m) == 1)
					{
						string_append_char(msg, m);
						string_append_char(msg, "]");
						pfree(m);
					}
					else
					{
						string_append_char(msg, "unknown message]");
					}
					
					/*
					 * If the error was caused by DEALLOCATE then print original query
					 */
					if 	(kind_list[i] == 'E')
					{
						List *parse_tree_list;
						Node *node;

						parse_tree_list = raw_parser(query_string_buffer);

						if (parse_tree_list != NIL)
						{
							node = (Node *) lfirst(list_head(parse_tree_list));

							if (IsA(node, DeallocateStmt))
							{
								POOL_SENT_MESSAGE *sent_msg;
								DeallocateStmt *d = (DeallocateStmt *)node;

								sent_msg = pool_get_sent_message('Q', d->name);
								if (!sent_msg)
									sent_msg = pool_get_sent_message('P', d->name);
								if (sent_msg)
								{
									if (sent_msg->query_context->original_query)
									{
										string_append_char(msg, "[");
										string_append_char(msg, sent_msg->query_context->original_query);
										string_append_char(msg, "]");
									}
								}
							}
						}
						/* free_parser(); */
					}
				}
				else
				{
					snprintf(buf, sizeof(buf), " %d[%c]", i, kind_list[i]);
					string_append_char(msg, buf);
				}
			}
		}

		if (pool_config->replication_stop_on_mismatch)
		{
			degenerate_backend_set(degenerate_node, degenerate_node_num);
            retcode = 1;
		}
        ereport(FATAL,
            (return_code(retcode),
                 errmsg("failed to read kind from backend"),
                    errdetail("%s",msg->data),
                        errhint("check data consistency among db nodes")));

	}

	return;
}

/*
 * Send DEALLOCATE message to backend by using SimpleQuery.
 */
static int send_deallocate(POOL_CONNECTION_POOL *backend,
						   POOL_SENT_MESSAGE_LIST msglist, int n)
{
	int len;
	char *name;
	char *query;

	if (msglist.size <= n)
		return 1;

	name = msglist.sent_messages[n]->name;

	len = strlen(name) + 14; /* "DEALLOCATE \"" + "\"" + '\0' */
	query = palloc(len);
	sprintf(query, "DEALLOCATE \"%s\"", name);

	if (SimpleQuery(NULL, backend, strlen(query)+1, query) != POOL_CONTINUE)
	{
		pfree(query);
		return 1;
	}
	pfree(query);

	return 0;
}

/*
 * parse_copy_data()
 *   Parses CopyDataRow string.
 *   Returns divide key value. If cannot parse data, returns NULL.
 */
char *
parse_copy_data(char *buf, int len, char delimiter, int col_id)
{
	int i, j, field = 0;
	char *str, *p = NULL;

	str = palloc(len + 1);

	/* buf is terminated by '\n'. */
	/* skip '\n' in for loop.     */
	for (i = 0, j = 0; i < len - 1; i++)
	{
		if (buf[i] == '\\' && i != len - 2) /* escape */
		{
			if (buf[i+1] == delimiter)
				i++;

			str[j++] = buf[i];
		}
		else if (buf[i] == delimiter) /* delimiter */
		{
			if (field == col_id)
			{
				break;
			}
			else
			{
				field++;
				j = 0;
			}
		}
		else
		{
			str[j++] = buf[i];
		}
	}

	if (field == col_id)
	{
		str[j] = '\0';
		p = palloc(j+1);
		strcpy(p, str);
		p[j] = '\0';
		ereport(DEBUG1,
			(errmsg("parsing copy data"),
				 errdetail("divide key value is %s", p)));
	}

	pfree(str);
	return p;
}

void query_ps_status(char *query, POOL_CONNECTION_POOL *backend)
{
	StartupPacket *sp;
	char psbuf[1024];
	int i = 0;

	if (*query == '\0')
		return;

	sp = MASTER_CONNECTION(backend)->sp;
    if(sp)
        i = snprintf(psbuf, sizeof(psbuf) - 1, "%s %s %s ",
				 sp->user, sp->database, remote_ps_data);

	/* skip spaces */
	while (*query && isspace(*query))
		query++;

	for (; i< sizeof(psbuf)-1; i++)
	{
		if (!*query || isspace(*query))
			break;

		psbuf[i] = toupper(*query++);
	}
	psbuf[i] = '\0';

	set_ps_display(psbuf, false);
}

/* compare function for bsearch() */
int compare(const void *p1, const void *p2)
{
	int 	v1,	v2;

	v1 = *(NodeTag *) p1;
	v2 = *(NodeTag *) p2;
	return (v1 > v2) ? 1 : ((v1 == v2) ? 0 : -1);
}

/* return true if needed to start a transaction for the nodetag */
static bool is_internal_transaction_needed(Node *node)
{
	static NodeTag nodemap[] = {
		T_PlannedStmt,
		T_InsertStmt,
		T_DeleteStmt,
		T_UpdateStmt,
		T_SelectStmt,
		T_AlterTableStmt,
		T_AlterDomainStmt,
		T_GrantStmt,
		T_GrantRoleStmt,
		/*
		T_AlterDefaultPrivilegesStmt,	Our parser does not support yet
		*/
		T_ClosePortalStmt,
		T_ClusterStmt,
		T_CopyStmt,
		T_CreateStmt,	/* CREAE TABLE */
		T_DefineStmt,	/* CREATE AGGREGATE, OPERATOR, TYPE */
		T_DropStmt,		/* DROP TABLE etc. */
		T_TruncateStmt,
		T_CommentStmt,
		T_FetchStmt,
		T_IndexStmt,	/* CREATE INDEX */
		T_CreateFunctionStmt,
		T_AlterFunctionStmt,
		/*
		T_DoStmt,		Our parser does not support yet
		*/
		T_RenameStmt,	/* ALTER AGGREGATE etc. */
		T_RuleStmt,		/* CREATE RULE */
		T_NotifyStmt,
		T_ListenStmt,
		T_UnlistenStmt,
		T_ViewStmt,		/* CREATE VIEW */
		T_LoadStmt,
		T_CreateDomainStmt,
		/*
		  T_CreatedbStmt,	CREATE DATABASE/DROP DATABASE cannot execute inside a transaction block
		  T_DropdbStmt,
		  T_VacuumStmt,
		  T_ExplainStmt,
		*/
		T_CreateSeqStmt,
		T_AlterSeqStmt,
		T_VariableSetStmt,		/* SET */
		T_CreateTrigStmt,
		T_CreatePLangStmt,
		T_CreateRoleStmt,
		T_AlterRoleStmt,
		T_DropRoleStmt,
		T_LockStmt,
		T_ConstraintsSetStmt,
		T_ReindexStmt,
		T_CreateSchemaStmt,
		T_AlterDatabaseStmt,
		T_AlterDatabaseSetStmt,
		T_AlterRoleSetStmt,
		T_CreateConversionStmt,
		T_CreateCastStmt,
		T_CreateOpClassStmt,
		T_CreateOpFamilyStmt,
		T_AlterOpFamilyStmt,
		T_PrepareStmt,
		T_ExecuteStmt,
		T_DeallocateStmt,
		T_DeclareCursorStmt,
/*
		T_CreateTableSpaceStmt,	CREATE/DROP TABLE SPACE cannot execute inside a transaction block
		T_DropTableSpaceStmt,
*/
		T_AlterObjectSchemaStmt,
		T_AlterOwnerStmt,
		T_DropOwnedStmt,
		T_ReassignOwnedStmt,
		T_CompositeTypeStmt,	/* CREATE TYPE */
		T_CreateEnumStmt,
		T_AlterTSDictionaryStmt,
		T_AlterTSConfigurationStmt,
		T_CreateFdwStmt,
		T_AlterFdwStmt,
		T_CreateForeignServerStmt,
		T_AlterForeignServerStmt,
		T_CreateUserMappingStmt,
		T_AlterUserMappingStmt,
		T_DropUserMappingStmt,
		/*
		T_AlterTableSpaceOptionsStmt,	Our parser does not support yet
		*/
	};

	if (bsearch(&nodeTag(node), nodemap, sizeof(nodemap)/sizeof(nodemap[0]), sizeof(NodeTag), compare) != NULL)
	{
		/*
		 * Check CREATE INDEX CONCURRENTLY. If so, do not start transaction
		 */
		if (IsA(node, IndexStmt))
		{
			if (((IndexStmt *)node)->concurrent)
				return false;
		}

		/*
		 * Check CLUSTER with no option. If so, do not start transaction
		 */
		else if (IsA(node, ClusterStmt))
		{
			if (((ClusterStmt *)node)->relation == NULL)
				return false;
		}

		/*
		 * REINDEX DATABASE or SYSTEM cannot be executed in a transaction block
		 */
		else if (IsA(node, ReindexStmt))
		{
			if (((ReindexStmt *)node)->kind == OBJECT_DATABASE ||
				((ReindexStmt *)node)->do_system)
				return false;
		}

		return true;

	}
	return false;
}

/*
 * Start an internal transaction if necessary.
 */
POOL_STATUS start_internal_transaction(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, Node *node)
{
	int i;

	/* If we are not in a transaction block,
	 * start a new transaction
	 */
	if (is_internal_transaction_needed(node))
	{
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i) && !INTERNAL_TRANSACTION_STARTED(backend, i) &&
				TSTATE(backend, i) == 'I')
			{
				per_node_statement_log(backend, i, "BEGIN");

				if (do_command(frontend, CONNECTION(backend, i), "BEGIN", MAJOR(backend), 
							   MASTER_CONNECTION(backend)->pid,	MASTER_CONNECTION(backend)->key, 0) != POOL_CONTINUE)
                    ereport(ERROR,
                            (errmsg("unable to start the internal transaction"),
                             errdetail("do_command returned DEADLOCK status")));

				/* Mark that we started new transaction */
				INTERNAL_TRANSACTION_STARTED(backend, i) = true;
				pool_unset_writing_transaction();
			}
		}
	}
	return POOL_CONTINUE;
}

/*
 * End internal transaction.
 */
POOL_STATUS end_internal_transaction(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	int i;
	int len;
	char tstate;
#ifdef HAVE_SIGPROCMASK
	sigset_t oldmask;
#else
	int	oldmask;
#endif

	/*
	 * We must block all signals. If pgpool SIGTERM, SIGINT or SIGQUIT
	 * is delivered, it could cause data inconsistency.
	 */
	POOL_SETMASK2(&BlockSig, &oldmask);

    PG_TRY();
    {
        /* We need to commit from secondary to master. */
        for (i=0;i<NUM_BACKENDS;i++)
        {
            if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i) &&
                TSTATE(backend, i) != 'I' &&
                INTERNAL_TRANSACTION_STARTED(backend, i))
            {
                if (MAJOR(backend) == PROTO_MAJOR_V3)
                {
                    /*
                     * Skip rest of Ready for Query packet
                     */
                    pool_read(CONNECTION(backend, i), &len, sizeof(len));
                    pool_read(CONNECTION(backend, i), &tstate, sizeof(tstate));
                }

                per_node_statement_log(backend, i, "COMMIT");
                PG_TRY();
                {
                    if (do_command(frontend, CONNECTION(backend, i), "COMMIT", MAJOR(backend),
                                   MASTER_CONNECTION(backend)->pid,	MASTER_CONNECTION(backend)->key, 1) != POOL_CONTINUE)
                    {
                        ereport(ERROR,
							(errmsg("unable to COMMIT the transaction"),
                                 errdetail("do_command returned DEADLOCK status")));
                    }
                }
                PG_CATCH();
                {
                    INTERNAL_TRANSACTION_STARTED(backend, i) = false;
                    PG_RE_THROW();
                }
                PG_END_TRY();
                
                INTERNAL_TRANSACTION_STARTED(backend, i) = false;
            }
        }

        /* Commit on master */
        if (TSTATE(backend, MASTER_NODE_ID) != 'I' &&
                INTERNAL_TRANSACTION_STARTED(backend, MASTER_NODE_ID))
        {
            if (MAJOR(backend) == PROTO_MAJOR_V3)
            {
                /*
                 * Skip rest of Ready for Query packet
                 */
                pool_read(CONNECTION(backend, MASTER_NODE_ID), &len, sizeof(len));
                pool_read(CONNECTION(backend, MASTER_NODE_ID), &tstate, sizeof(tstate));
            }

            per_node_statement_log(backend, MASTER_NODE_ID, "COMMIT");
            PG_TRY();
            {
                if (do_command(frontend, MASTER(backend), "COMMIT", MAJOR(backend),
                               MASTER_CONNECTION(backend)->pid,	MASTER_CONNECTION(backend)->key, 1) != POOL_CONTINUE)
                {
                    ereport(ERROR,
                            (errmsg("unable to COMMIT the transaction"),
                             errdetail("do_command returned DEADLOCK status")));
                }
            }
            PG_CATCH();
            {
                INTERNAL_TRANSACTION_STARTED(backend, MASTER_NODE_ID) = false;
                PG_RE_THROW();
            }
            PG_END_TRY();
            INTERNAL_TRANSACTION_STARTED(backend, MASTER_NODE_ID) = false;
        }
    }
    PG_CATCH();
    {
        POOL_SETMASK(&oldmask);
        PG_RE_THROW();
    }
    PG_END_TRY();

	POOL_SETMASK(&oldmask);
	return POOL_CONTINUE;
}

/*
 * Returns true if error message contains PANIC or FATAL.
 */
static bool is_panic_or_fatal_error(const char *message, int major)
{
	if (major == PROTO_MAJOR_V3)
	{
		for (;;)
		{
			char id;

			id = *message++;
			if (id == '\0')
				break;

			if (id == 'S' && (strcasecmp("PANIC", message) == 0 || strcasecmp("FATAL", message) == 0))
				return true;
			else
			{
				while (*message++)
					;
				continue;
			}
		}
	}
	else
	{
		if (strncmp(message, "PANIC", 5) == 0 || strncmp(message, "FATAL", 5) == 0)
			return true;
	}
	return false;
}

static int detect_postmaster_down_error(POOL_CONNECTION *backend, int major)
{
	int r =  detect_error(backend, ADMIN_SHUTDOWN_ERROR_CODE, major, 'E', false);
	if (r < 0)
	{
		ereport(LOG,
			(errmsg("detecting postmaster down error")));
		return r;
	}
	if (r == SPECIFIED_ERROR)
	{
		ereport(DEBUG1,
			(errmsg("detecting postmaster down error"),
				 errdetail("receive admin shutdown error from a node")));
		return r;
	}

	r = detect_error(backend, CRASH_SHUTDOWN_ERROR_CODE, major, 'N', false);
	if (r < 0)
	{
		ereport(LOG,
				(errmsg("detect_postmaster_down_error: detect_error error")));
		return r;
	}
	if (r == SPECIFIED_ERROR)
	{
		ereport(DEBUG1,
				(errmsg("detect_postmaster_down_error: receive crash shutdown error from a node.")));
	}
	return r;
}

int detect_active_sql_transaction_error(POOL_CONNECTION *backend, int major)
{
	int r =  detect_error(backend, ACTIVE_SQL_TRANSACTION_ERROR_CODE, major, 'E', true);
	if (r == SPECIFIED_ERROR)
	{
		ereport(DEBUG1,
			(errmsg("detecting active sql transaction error"),
				 errdetail("receive SET TRANSACTION ISOLATION LEVEL must be called before any query error from a node")));
	}
	return r;
}

int detect_deadlock_error(POOL_CONNECTION *backend, int major)
{
	int r =  detect_error(backend, DEADLOCK_ERROR_CODE, major, 'E', true);
	if (r == SPECIFIED_ERROR)
		ereport(DEBUG1,
			(errmsg("detecting deadlock error"),
				 errdetail("received deadlock error message from backend")));
	return r;
}

int detect_serialization_error(POOL_CONNECTION *backend, int major, bool unread)
{
	int r =  detect_error(backend, SERIALIZATION_FAIL_ERROR_CODE, major, 'E', unread);
	if (r == SPECIFIED_ERROR)
		ereport(DEBUG1,
			(errmsg("detecting serialization error"),
				 errdetail("received serialization failure message from backend")));
	return r;
}

int detect_query_cancel_error(POOL_CONNECTION *backend, int major)
{
	int r =  detect_error(backend, QUERY_CANCEL_ERROR_CODE, major, 'E', true);
	if (r == SPECIFIED_ERROR)
		ereport(DEBUG1,
			(errmsg("detecting query cancel error"),
				 errdetail("received query cancel error message from backend")));
	return r;
}

/*
 * detect_error: Detect specified error from error code.
 * returns 0 in case of sucess or 1 in cause of specified error.
 * and throws an ereport for all other errors
 */
static int detect_error(POOL_CONNECTION *backend, char *error_code, int major, char class, bool unread)
{
	int is_error = 0;
	char kind;
	int readlen = 0, len;
	static char buf[8192]; /* memory space is large enough */
	char *p, *str;

	if (pool_read(backend, &kind, sizeof(kind)))
		return -1;
	readlen += sizeof(kind);
	p = buf;
	memcpy(p, &kind, sizeof(kind));
	p += sizeof(kind);

	ereport(DEBUG1,
			(errmsg("detect error: kind: %c", kind)));


	/* Specified class? */
	if (kind == class)
	{
		/* read actual message */
		if (major == PROTO_MAJOR_V3)
		{
			char *e;

			if (pool_read(backend, &len, sizeof(len)) < 0)
				return -1;
			readlen += sizeof(len);
			memcpy(p, &len, sizeof(len));
			p += sizeof(len);

			len = ntohl(len) - 4;
			str = palloc(len);

			pool_read(backend, str, len);
			readlen += len;

			if (readlen >= sizeof(buf))
                ereport(ERROR,
                    (errmsg("unable to detect error"),
                         errdetail("not enough space in buffer")));

			memcpy(p, str, len);

			/*
			 * Checks error code which is formatted 'Cxxxxxx'
			 * (xxxxxx is error code).
			 */
			e = str;
			while (*e)
			{
				if (*e == 'C')
				{/* specified error? */
					is_error = (strcmp(e+1, error_code) == 0) ? SPECIFIED_ERROR : 0;
					break;
				}
				else
					e = e + strlen(e) + 1;
			}
			pfree(str);
		}
		else
		{
			str = pool_read_string(backend, &len, 0);
			readlen += len;

			if (readlen >= sizeof(buf))
                ereport(ERROR,
                        (errmsg("unable to detect error"),
                         errdetail("not enough space in buffer")));

			memcpy(p, str, len);
		}
	}
	if (unread || !is_error)
	{
		/* put a message to read buffer */
		pool_unread(backend, buf, readlen);
	}

	return is_error;
}

/*
 * pool_extract_error_message: Extract human readable message from
 * ERROR/NOTICE reponse packet and return it. If "read_kind" is true,
 * kind will be read in this function. If "read_kind" is false, kind
 * should have been already read and it should be either 'E' or
 * 'N'. The returned string is in palloc'd buffer. Callers must pfree
 * it if it becomes neccessary.
 *
 * If "unread" is true, the packet will be returned to the stream.
 *
 * Return values are: 
 * 0: not error or notice message 
 * 1: succeeded to extract error message 
 * -1: error)
 */
int pool_extract_error_message(bool read_kind, POOL_CONNECTION *backend, int major, bool unread, char **message)
{
	char kind;
	bool ret = 1;
	int readlen = 0, len;
	StringInfo str_buf;             /* unread buffer */
	StringInfo str_message_buf;		/* message buffer */
	MemoryContext oldContext = CurrentMemoryContext;
	char *str;

	str_buf = makeStringInfo();
	str_message_buf = makeStringInfo();

    PG_TRY();
    {
        if (read_kind)
        {
            len = sizeof(kind);

            pool_read(backend, &kind, len);

            readlen += len;
			appendStringInfoChar(str_buf, kind);

            if (kind != 'E' && kind != 'N')
            {
                pool_unread(backend, str_buf->data, readlen);
				pfree(str_message_buf->data);
				pfree(str_message_buf);
				pfree(str_buf->data);
				pfree(str_buf);
                ereport(ERROR,
                    (errmsg("unable to extract error message"),
                        errdetail("invalid message kind \"%c\"",kind)));
            }
        }

        /* read actual message */
        if (major == PROTO_MAJOR_V3)
        {
            char *e;

            pool_read(backend, &len, sizeof(len));
            readlen += sizeof(len);
			appendBinaryStringInfo(str_buf, (const char *)&len, sizeof(len));

            len = ntohl(len) - 4;
            str = palloc(len);

            pool_read(backend, str, len);
            readlen += len;
			appendBinaryStringInfo(str_buf, str, len);

            /*
             * Checks error code which is formatted 'Mxxxxxx'
             * (xxxxxx is error message).
             */
            e = str;
            while (*e)
            {
                if (*e == 'M')
                {
                    e++;
					appendStringInfoString(str_message_buf, e);
                    break;
                }
                else
                    e = e + strlen(e) + 1;
            }
            pfree(str);
        }
        else
        {
            str = pool_read_string(backend, &len, 0);
            readlen += len;
			appendStringInfoString(str_message_buf, str);
        }

        if (unread)
        {
            /* Put the message to read buffer */
            pool_unread(backend, str_buf->data, readlen);
        }

        *message = str_message_buf->data;
		pfree(str_message_buf);
		pfree(str_buf->data);
		pfree(str_buf);
	}
	PG_CATCH();
	{
		MemoryContextSwitchTo(oldContext);
		FlushErrorState();
		ret = -1;
	}
	PG_END_TRY();
	return ret;
}

/*
 * read message kind and rest of the packet then discard it
 */
POOL_STATUS pool_discard_packet(POOL_CONNECTION_POOL *cp)
{
	int i;
	char kind;
	POOL_CONNECTION *backend;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
		{
			continue;
		}

		backend = CONNECTION(cp, i);

		pool_read(backend, &kind, sizeof(kind));
        
        ereport(DEBUG2,
                (errmsg("pool_discard_packet: kind: %c", kind)));
	}

	return pool_discard_packet_contents(cp);
}

/*
 * read message length and rest of the packet then discard it
 */
POOL_STATUS pool_discard_packet_contents(POOL_CONNECTION_POOL *cp)
{
	int len, i;
	char *string;
	POOL_CONNECTION *backend;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
		{
			continue;
		}

		backend = CONNECTION(cp, i);

		if (MAJOR(cp) == PROTO_MAJOR_V3)
		{
			pool_read(backend, &len, sizeof(len));
			len = ntohl(len) - 4;
			string = pool_read2(backend, len);
			if (string == NULL)
                ereport(ERROR,
					(errmsg("pool discard packet contents failed"),
                         errdetail("error while reading rest of message")));

		}
		else
		{
			string = pool_read_string(backend, &len, 0);
			if (string == NULL)
                ereport(ERROR,
					(errmsg("pool discard packet contents failed"),
                         errdetail("error while reading rest of message")));
		}
	}
	return POOL_CONTINUE;
}

/*
 * Read packet from either frontend or backend and process it.
 */
static POOL_STATUS read_packets_and_process(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, int reset_request, int *state, short *num_fields, bool *cont)
{
	fd_set	readmask;
	fd_set	writemask;
	fd_set	exceptmask;
	int fds;
	struct timeval timeoutdata;
	struct timeval *timeout;
	int num_fds, was_error = 0;
	POOL_STATUS status;
	int i;

	/*
	 * frontend idle counters. depends on the following
	 * select(2) call's time out is 1 second.
	 */
	int idle_count = 0;	/* for other than in recovery */
	int idle_count_in_recovery = 0;	/* for in recovery */

SELECT_RETRY:
	FD_ZERO(&readmask);
	FD_ZERO(&writemask);
	FD_ZERO(&exceptmask);

	num_fds = 0;

	if (!reset_request)
	{
		FD_SET(frontend->fd, &readmask);
		FD_SET(frontend->fd, &exceptmask);
		num_fds = Max(frontend->fd + 1, num_fds);
	}

	/*
	 * If we are in load balance mode and the selected node is
	 * down, we need to re-select load_balancing_node.  Note
	 * that we cannnot use VALID_BACKEND macro here.  If
	 * in_load_balance == 1, VALID_BACKEND macro may return 0.
	 */
	if (pool_config->load_balance_mode &&
		BACKEND_INFO(backend->info->load_balancing_node).backend_status == CON_DOWN)
	{
		/* select load balancing node */
		backend->info->load_balancing_node = select_load_balancing_node();
	}

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			num_fds = Max(CONNECTION(backend, i)->fd + 1, num_fds);
			FD_SET(CONNECTION(backend, i)->fd, &readmask);
			FD_SET(CONNECTION(backend, i)->fd, &exceptmask);
		}
	}

	/*
	 * wait for data arriving from frontend and backend
	 */
	if (pool_config->client_idle_limit > 0 ||
		pool_config->client_idle_limit_in_recovery > 0 ||
		pool_config->client_idle_limit_in_recovery == -1)
	{
		timeoutdata.tv_sec = 1;
		timeoutdata.tv_usec = 0;
		timeout = &timeoutdata;
	}
	else
		timeout = NULL;

	fds = select(num_fds, &readmask, &writemask, &exceptmask, timeout);

	if (fds == -1)
	{
		if (errno == EINTR)
			goto SELECT_RETRY;

        ereport(FATAL,
            (errmsg("unable to read data"),
                 errdetail("select() system call failed with reason \"%s\"",strerror(errno))));
	}

	/* select timeout */
	if (fds == 0)
	{
		if (*InRecovery == RECOVERY_INIT && pool_config->client_idle_limit > 0)
		{
			idle_count++;

			if (idle_count > pool_config->client_idle_limit)
			{
                ereport(ERROR,
                        (pool_error_code("57000"),
                        errmsg("unable to read data"),
                         errdetail("child connection forced to terminate due to client_idle_limit:%d is reached",
                                   pool_config->client_idle_limit)));
            }
		}
		else if (*InRecovery > RECOVERY_INIT && pool_config->client_idle_limit_in_recovery > 0)
		{
			idle_count_in_recovery++;

			if (idle_count_in_recovery > pool_config->client_idle_limit_in_recovery)
			{
                ereport(ERROR,
                    (pool_error_code("57000"),
                    errmsg("unable to read data"),
                         errdetail("child connection forced to terminate due to client_idle_limit_in_recovery:%d is reached",pool_config->client_idle_limit_in_recovery)));
			}
		}
		else if (*InRecovery > RECOVERY_INIT && pool_config->client_idle_limit_in_recovery == -1)
		{
			/*
			 * If we are in recovery and client_idle_limit_in_recovery is -1, then
			 * exit immediately.
			 */
            ereport(ERROR,
                (pool_error_code("57000"),
                errmsg("connection terminated due to online recovery"),
                     errdetail("child connection forced to terminate due to client_idle_limitis:-1")));
		}
		goto SELECT_RETRY;
	}

	for (i = 0; i < NUM_BACKENDS; i++)
	{
		if (VALID_BACKEND(i))
		{
			/*
			 * make sure that connection slot exists
			 */
			if (CONNECTION_SLOT(backend, i) == 0)
			{
				ereport(LOG,
					(errmsg("error occured while reading and processing packets"),
						errdetail("FATAL ERROR: VALID_BACKEND returns non 0 but connection slot is empty. backend id:%d RAW_MODE:%d LOAD_BALANCE_STATUS:%d status:%d",
							   i, RAW_MODE, LOAD_BALANCE_STATUS(i), BACKEND_INFO(i).backend_status)));
				was_error = 1;
				break;
			}

			if (FD_ISSET(CONNECTION(backend, i)->fd, &readmask))
			{
				int r;
				/*
				 * connection was terminated due to confilct with recovery
				 */
				r = detect_serialization_error(CONNECTION(backend, i), MAJOR(backend), false);
				if (r == SPECIFIED_ERROR)
				{
                    ereport(FATAL,
                            (pool_error_code(SERIALIZATION_FAIL_ERROR_CODE),
                                errmsg("connection was terminated due to conflict with recovery"),
                                 errdetail("User was holding a relation lock for too long."),
                                    errhint("In a moment you should be able to reconnect to the database and repeat your command.")));

				}

				/*
				 * admin shutdown postmaster or postmaster goes down
				 */
				r = detect_postmaster_down_error(CONNECTION(backend, i), MAJOR(backend));
				if (r == SPECIFIED_ERROR)
				{
					ereport(LOG,
						(errmsg("reading and processing packets"),
							 errdetail("postmaster on DB node %d was shutdown by administrative command", i)));
					/* detach backend node. */
					was_error = 1;
					if (!VALID_BACKEND(i))
						break;
					notice_backend_error(i);
					sleep(5);
					break;
				}
				else if (r < 0)
				{
					/*
					 * This could happen after detecting backend errors and before actually
					 * detaching the backend. In this case reading from backend socket will
					 * return EOF and it's better to close this session. So returns POOL_END.
					 */ 
                    ereport(LOG,
                        (errmsg("unable to read and process data"),
                             errdetail("detect_postmaster_down_error returns error on backend %d. Going to close this session.", i)));
                    return POOL_END;
				}
			}
		}
	}

	if (was_error)
	{
		*cont = false;
		return POOL_CONTINUE;
	}

	if (!reset_request)
	{
		if (FD_ISSET(frontend->fd, &exceptmask))
            ereport(ERROR,
                (errmsg("unable to read from frontend socket"),
                     errdetail("exception occured on frontend socket")));
    
		else if (FD_ISSET(frontend->fd, &readmask))
		{
			status = ProcessFrontendResponse(frontend, backend);
			if (status != POOL_CONTINUE)
				return status;
		}
	}

	if (FD_ISSET(MASTER(backend)->fd, &exceptmask))
        ereport(FATAL,
            (errmsg("unable to read from backend socket"),
                 errdetail("exception occured on backend socket")));

	else if (FD_ISSET(MASTER(backend)->fd, &readmask))
	{
		status = ProcessBackendResponse(frontend, backend, state, num_fields);
		if (status != POOL_CONTINUE)
			return status;
	}
	return POOL_CONTINUE;
}

/*
 * Debugging aid for VALID_BACKEND macro.
 */
void pool_dump_valid_backend(int backend_id)
{
    ereport(LOG,
            (errmsg("RAW_MODE:%d REAL_MASTER_NODE_ID:%d pool_is_node_to_be_sent_in_current_query:%d my_backend_status:%d",
                    RAW_MODE, REAL_MASTER_NODE_ID, pool_is_node_to_be_sent_in_current_query(backend_id),
                    *my_backend_status[backend_id])));
}
