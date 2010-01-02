/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2009	PgPool Global Development Group
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

#include "pool.h"
#include "pool_signal.h"
#include "pool_proto_modules.h"

#ifndef FD_SETSIZE
#define FD_SETSIZE 512
#endif

#define INIT_STATEMENT_LIST_SIZE 8

#define ACTIVE_SQL_TRANSACTION_ERROR_CODE "25001"		/* SET TRANSACTION ISOLATION LEVEL must be called before any query */
#define DEADLOCK_ERROR_CODE "40P01"
#define SERIALIZATION_FAIL_ERROR_CODE "40001"
#define QUERY_CANCEL_ERROR_CODE "57014"
#define ADMIN_SHUTDOWN_ERROR_CODE "57P01"
#define CRASH_SHUTDOWN_ERROR_CODE "57P02"

static int reset_backend(POOL_CONNECTION_POOL *backend, int qcnt);
static POOL_STATUS do_command(POOL_CONNECTION *frontend, POOL_CONNECTION *backend, char *query, int protoMajor, int pid, int key, int no_ready_for_query);
static POOL_STATUS do_error_execute_command(POOL_CONNECTION_POOL *backend, int node_id, int major);
static char *get_insert_command_table_name(InsertStmt *node);
static void reset_prepared_list(PreparedStatementList *p);
static int send_deallocate(POOL_CONNECTION_POOL *backend, PreparedStatementList *p, int n);
static int is_cache_empty(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
static POOL_STATUS ParallelForwardToFrontend(char kind, POOL_CONNECTION *frontend, POOL_CONNECTION *backend, char *database, bool send_to_frontend);
static void query_cache_register(char kind, POOL_CONNECTION *frontend, char *database, char *data, int data_len);
static int extract_ntuples(char *message);
static int detect_error(POOL_CONNECTION *master, char *error_code, int major, char class, bool unread);
static int detect_postmaster_down_error(POOL_CONNECTION *master, int major);

static bool is_internal_transaction_needed(Node *node);
static int compare(const void *p1, const void *p2);

/* timeout sec for pool_check_fd */
static int timeoutsec;

int in_load_balance;	/* non 0 if in load balance mode */
int selected_slot;		/* selected DB node */
int master_slave_dml;	/* non 0 if master/slave mode is specified in config file */

/*
 * main module for query processing
 */
POOL_STATUS pool_process_query(POOL_CONNECTION *frontend,
							   POOL_CONNECTION_POOL *backend,
							   int connection_reuse,
							   int first_ready_for_query_received)
{
	char kind;	/* packet kind (backend) */
	char fkind;	/* packet kind (frontend) */
	short num_fields = 0;
	fd_set	readmask;
	fd_set	writemask;
	fd_set	exceptmask;
	int fds;
	POOL_STATUS status;
	int state;	/* 0: ok to issue commands 1: waiting for "ready for query" response */
	int qcnt;
	int i;

	frontend->no_forward = connection_reuse;
	qcnt = 0;
	state = 0;

	for (;;)
	{
		kind = 0;
		fkind = 0;

		if (state == 0 && connection_reuse)
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
				return POOL_END;
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
				TSTATE(backend) = 'I';
				frontend->no_forward = 0;
				return POOL_CONTINUE;
			}

		}

		/*
		 * if all backends do not have any pending data in the
		 * receiving data cache, then issue select(2) to wait for new
		 * data arrival
		 */
		if (is_cache_empty(frontend, backend))
		{
			struct timeval timeoutdata;
			struct timeval *timeout;
			int num_fds, was_error = 0;

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

			/*
			 * Do not read a message from frontend while backends process a query.
			 */
			if (!connection_reuse && !in_progress)
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
				pool_config->client_idle_limit_in_recovery > 0)
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
					continue;

				pool_error("select() failed. reason: %s", strerror(errno));
				return POOL_ERROR;
			}

			/* select timeout */
			if (fds == 0)
			{
				if (*InRecovery == 0 && pool_config->client_idle_limit > 0)
				{
					idle_count++;

					if (idle_count > pool_config->client_idle_limit)
					{
						pool_log("pool_process_query: child connection forced to terminate due to client_idle_limit(%d) reached", pool_config->client_idle_limit);
						return POOL_END;
					}
				}
				else if (*InRecovery > 0 && pool_config->client_idle_limit_in_recovery > 0)
				{
					idle_count_in_recovery++;

					if (idle_count_in_recovery > pool_config->client_idle_limit_in_recovery)
					{
						pool_log("pool_process_query: child connection forced to terminate due to client_idle_limit_in_recovery(%d) reached", pool_config->client_idle_limit_in_recovery);
						return POOL_END;
					}
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
						pool_log("FATAL ERROR: VALID_BACKEND returns non 0 but connection slot is empty. backend id:%d RAW_MODE:%d in_load_balance:%d LOAD_BALANCE_STATUS:%d status:%d",
								 i, RAW_MODE, in_load_balance, LOAD_BALANCE_STATUS(i), BACKEND_INFO(i).backend_status);
						was_error = 1;
						break;
					}

					if (FD_ISSET(CONNECTION(backend, i)->fd, &readmask))
					{
						/*
						 * admin shutdown postmaster or postmaster goes down
						 */
						if (detect_postmaster_down_error(CONNECTION(backend, i), MAJOR(backend)) == SPECIFIED_ERROR)
						{
							/* detach backend node. */
							was_error = 1;
							if (!VALID_BACKEND(i))
								break;
							notice_backend_error(i);
							sleep(5);
							break;
						}
						status = read_kind_from_backend(frontend, backend, &kind);
						if (status != POOL_CONTINUE)
							return status;
						break;
					}
				}
			}

			if (was_error)
				continue;

			if (!connection_reuse && !in_progress)
			{
				if (FD_ISSET(frontend->fd, &exceptmask))
					return POOL_END;
				else if (FD_ISSET(frontend->fd, &readmask))
				{
					status = ProcessFrontendResponse(frontend, backend);
					if (status != POOL_CONTINUE)
						return status;

					continue;
				}
				if (kind == 0)
					continue;
			}

			if (FD_ISSET(MASTER(backend)->fd, &exceptmask))
			{
				return POOL_ERROR;
			}
		}
		else
		{
			if (frontend->len > 0 && !in_progress)
			{
				status = ProcessFrontendResponse(frontend, backend);
				if (status != POOL_CONTINUE)
					return status;

				continue;
			}
		}

		/* this is the synchronous point */
		if (kind == 0)
		{
			status = read_kind_from_backend(frontend, backend, &kind);
			if (status != POOL_CONTINUE)
				return status;
		}

		/* reload config file */
		if (got_sighup)
		{
			pool_get_config(get_config_file_name(), RELOAD_CONFIG);
			if (pool_config->enable_pool_hba)
				load_hba(get_hba_file_name());
			if (pool_config->parallel_mode)
				pool_memset_system_db_info(system_db_info->info);
			got_sighup = 0;
		}

		first_ready_for_query_received = 0;

		/*
		 * Process backend Response
		 */

		/*
		 * Sanity check
		 */
		if (kind == 0)
		{
			pool_error("pool_process_query: kind is 0!");
			return POOL_ERROR;
		}

		pool_debug("pool_process_query: kind from backend: %c", kind);

		if (MAJOR(backend) == PROTO_MAJOR_V3)
		{
			switch (kind)
			{
				case 'G':
					/* CopyIn response */
					status = CopyInResponse(frontend, backend);
					break;
				case 'S':
					/* Parameter Status */
					status = ParameterStatus(frontend, backend);
					break;
				case 'Z':
					/* Ready for query */
					status = ReadyForQuery(frontend, backend, 1);
					break;
				default:
					status = SimpleForwardToFrontend(kind, frontend, backend);
					if (pool_flush(frontend))
						return POOL_END;
					break;
			}
		}
		else
		{
			switch (kind)
			{
				case 'A':
					/* Notification  response */
					status = NotificationResponse(frontend, backend);
					break;

				case 'B':
					/* BinaryRow */
					status = BinaryRow(frontend, backend, num_fields);
					break;

				case 'C':
					/* Complete command response */
					status = CompleteCommandResponse(frontend, backend);
					break;

				case 'D':
					/* AsciiRow */
					status = AsciiRow(frontend, backend, num_fields);
					break;

				case 'E':
					/* Error Response */
					status = ErrorResponse(frontend, backend);
					break;

				case 'G':
					/* CopyIn Response */
					status = CopyInResponse(frontend, backend);
					break;

				case 'H':
					/* CopyOut Response */
					status = CopyOutResponse(frontend, backend);
					break;

				case 'I':
					/* Empty Query Response */
					status = EmptyQueryResponse(frontend, backend);
					break;

				case 'N':
					/* Notice Response */
					status = NoticeResponse(frontend, backend);
					break;

				case 'P':
					/* CursorResponse */
					status = CursorResponse(frontend, backend);
					break;

				case 'T':
					/* RowDescription */
					status = RowDescription(frontend, backend, &num_fields);
					break;

				case 'V':
					/* FunctionResultResponse and FunctionVoidResponse */
					status = FunctionResultResponse(frontend, backend);
					break;

				case 'Z':
					/* Ready for query */
					status = ReadyForQuery(frontend, backend, 1);
					break;

				default:
					pool_error("Unknown message type %c(%02x)", kind, kind);
					exit(1);
			}
		}

		if (status != POOL_CONTINUE)
			return status;

		if (kind == 'Z' && frontend->no_forward && state == 1)
		{
			state = 0;
		}

	}
	return POOL_CONTINUE;
}


/*
 * set_fd,isset_fs,zero_fd are used
 * for check fd in parallel mode
 */

/* used only in pool_parallel_exec */
#define BITS (8 * sizeof(long int))

static void set_fd(unsigned long fd ,unsigned long *setp)
{
	unsigned long tmp = fd / FD_SETSIZE;
	unsigned long rem = fd % FD_SETSIZE;
	setp[tmp] |= (1UL<<rem);
}

/* used only in pool_parallel_exec */
static int isset_fd(unsigned long fd, unsigned long *setp)
{
	unsigned long tmp = fd / FD_SETSIZE;
	unsigned long rem = fd % FD_SETSIZE;
	return (setp[tmp] & (1UL<<rem)) != 0;
}

/* used only in pool_parallel_exec */
static void zero_fd(unsigned long *setp)
{
	unsigned long *tmp = setp;
	int i = FD_SETSIZE / BITS;
	while(i)
	{
		i--;
		*tmp = 0;
		tmp++;
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
	unsigned long donemask[FD_SETSIZE / BITS];
	static char *sq = "show pool_status";
	POOL_STATUS status;
	struct timeval timeout;
	int num_fds;
	int used_count = 0;
	int error_flag = 0;
	unsigned long datacount = 0;

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	len = strlen(string) + 1;

	if (is_drop_database(node))
	{
		int stime = 5;  /* XXX give arbitrary time to allow closing idle connections */

		pool_debug("Query: sending HUP signal to parent");

		kill(getppid(), SIGHUP);        /* send HUP signal to parent */

		/* we need to loop over here since we will get HUP signal while sleeping */
		while (stime > 0)
			stime = sleep(stime);
	}

	/* process status reporting? */
	if (strncasecmp(sq, string, strlen(sq)) == 0)
	{
		pool_debug("process reporting");
		process_reporting(frontend, backend);
		in_progress = 0;
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

		if (pool_write_and_flush(CONNECTION(backend, i), string, len) < 0)
		{
			return POOL_END;
		}

		/*
		 * in "strict mode" we need to wait for backend completing the query.
		 * note that this is not applied if "NO STRICT" is specified as a comment.
		 */
		if (is_strict_query(node))
		{
			pool_debug("waiting for backend %d completing the query", i);
			if (synchronize(CONNECTION(backend, i)))
				return POOL_END;
		}
	}

	if (!is_cache_empty(frontend, backend))
	{
		return POOL_END;
	}

	zero_fd(donemask);

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
				if(!isset_fd(fd,donemask))
				{
					FD_SET(fd, &readmask);
					FD_SET(fd, &exceptmask);
					pool_debug("pool_parallel_query:  %d th FD_SET: %d",i, CONNECTION(backend, i)->fd);
				}
			}
		}

		pool_debug("pool_parallel_query: num_fds: %d", num_fds);

		fds = select(num_fds, &readmask, &writemask, &exceptmask, NULL);

		if (fds == -1)
		{
			if (errno == EINTR)
				continue;

				pool_error("select() failed. reason: %s", strerror(errno));
			return POOL_ERROR;
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
				status = read_kind_from_one_backend(frontend, backend, &kind,i);
				if (status != POOL_CONTINUE)
					return status;

				if (used_count == 0)
				{
					status = ParallelForwardToFrontend(kind,
														frontend,
														CONNECTION(backend, i),
														backend->info->database,
														send_to_frontend);
					pool_debug("pool_parallel_exec: kind from backend: %c", kind);
				}
				else
				{
					status = ParallelForwardToFrontend(kind,
														frontend,
														CONNECTION(backend, i),
														backend->info->database,
														false);
					pool_debug("pool_parallel_exec: dummy kind from backend: %c", kind);
				}

				if (status != POOL_CONTINUE)
					return status;

				if(kind == 'C' || kind == 'E' || kind == 'c')
				{
					if(used_count == NUM_BACKENDS -1)
						return POOL_CONTINUE;

					used_count++;
					set_fd(CONNECTION(backend, i)->fd, donemask);
					continue;
				}

				/* get body of protocol */
				for(;;)
				{
					if (pool_read(CONNECTION(backend, i), &kind, 1) < 0)
					{
						pool_error("pool_parallel_exec: failed to read kind from %d th backend", i);
						return POOL_ERROR;
					}

					/*
					 * Sanity check
					 */
					if (kind == 0)
					{
						pool_error("pool_parallel_exec: kind is 0!");
						return POOL_ERROR;
					}

					if((kind == 'E' ) &&
						used_count != NUM_BACKENDS -1)
					{
						if(error_flag ==0)
						{
							pool_debug("pool_parallel_exec: kind from backend: %c", kind);

							status = ParallelForwardToFrontend(kind,
															frontend,
															CONNECTION(backend, i),
															backend->info->database,
															send_to_frontend);
							error_flag++;
						} else {
							pool_debug("pool_parallel_exec: dummy from backend: %c", kind);
							status = ParallelForwardToFrontend(kind,
															frontend,
															CONNECTION(backend, i),
															backend->info->database,
															false);
						}
						used_count++;
						set_fd(CONNECTION(backend, i)->fd, donemask);
						break;
					}

					if((kind == 'c' || kind == 'C') &&
					   used_count != NUM_BACKENDS -1)
					{
						pool_debug("pool_parallel_exec: dummy from backend: %c", kind);
						status = ParallelForwardToFrontend(kind,
															frontend,
															CONNECTION(backend, i),
															backend->info->database,
															false);
						used_count++;
						set_fd(CONNECTION(backend, i)->fd, donemask);
						break;
					}
					if((kind == 'C' || kind == 'c' || kind == 'E') &&
						used_count == NUM_BACKENDS -1)
					{
						pool_debug("pool_parallel_exec: kind from backend: D %lu", datacount);

						if(error_flag == 0)
						{
							pool_debug("pool_parallel_exec: kind from backend: %c", kind);
							status = ParallelForwardToFrontend(kind,
															frontend,
															CONNECTION(backend, i),
															backend->info->database,
															send_to_frontend);
						} else {
							pool_debug("pool_parallel_exec: dummy from backend: %c", kind);
							status = ParallelForwardToFrontend(kind,
															frontend,
															CONNECTION(backend, i),
															backend->info->database,
															false);
						}
						return POOL_CONTINUE;
					}

					if(kind == 'D')
						datacount++;
					else
						pool_debug("pool_parallel_exec: kind from backend: %c", kind);

					status = ParallelForwardToFrontend(kind,
														frontend,
														CONNECTION(backend, i),
														backend->info->database,
														send_to_frontend);

					if (status != POOL_CONTINUE)
					{
						return status;
					}
					else
					{
						pool_flush(frontend);
					}
				}
			}
		}
	}
}



/*
 * send SimpleQuery message to a node.
 */
POOL_STATUS send_simplequery_message(POOL_CONNECTION *backend, int len, char *string, int major)
{
	/* forward the query to the backend */
	pool_write(backend, "Q", 1);

	if (major == PROTO_MAJOR_V3)
	{
		int sendlen = htonl(len + 4);
		pool_write(backend, &sendlen, sizeof(sendlen));
	}

	if (pool_write_and_flush(backend, string, len) < 0)
	{
		return POOL_END;
	}

	return POOL_CONTINUE;
}

/*
 * Wait for query response from single node. This checks frontend
 * connection by writing dummy parameter status packet every 30
 * seccond, and if the connection broke, returns error since there's
 * no point in that waiting until backend returns response.
 */
POOL_STATUS wait_for_query_response(POOL_CONNECTION *frontend, POOL_CONNECTION *backend, char *string, int protoVersion)
{
#define DUMMY_PARAMETER "pgpool_dummy_param"
#define DUMMY_VALUE "pgpool_dummy_value"

	int status;
	int plen;

	pool_debug("wait_for_query_response: waiting for backend %d completing the query", backend->db_node_id);

	for (;;)
	{
		/* Check to see if data from backend is ready */
		pool_set_timeout(30);
		status = pool_check_fd(backend);
		pool_set_timeout(0);

		if (status < 0)	/* error ? */
		{
			pool_error("wait_for_query_response: backend error occured while waiting for backend response");
			return POOL_END;
		}
		else if (status > 0)		/* data is not ready */
		{
			if (protoVersion == PROTO_MAJOR_V3)
			{
				/* Write dummy parameter staus packet to check if the socket to frontend is ok */
				if (pool_write(frontend, "S", 1) < 0)
					return POOL_END;
				plen = sizeof(DUMMY_PARAMETER)+sizeof(DUMMY_VALUE)+sizeof(plen);
				plen = htonl(plen);
				if (pool_write(frontend, &plen, sizeof(plen)) < 0)
					return POOL_END;
				if (pool_write(frontend, DUMMY_PARAMETER, sizeof(DUMMY_PARAMETER)) < 0)
					return POOL_END;
				if (pool_write(frontend, DUMMY_VALUE, sizeof(DUMMY_VALUE)) < 0)
					return POOL_END;
				if (pool_flush_it(frontend) < 0)
				{
					pool_error("wait_for_query_response: frontend error occured while waiting for backend reply");
					return POOL_END;
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
				if (pool_write(frontend, "N", 1) < 0)
					return POOL_END;
				if (pool_write(frontend, notice_message, strlen(notice_message)+1) < 0)
					return POOL_END;
				if (pool_flush_it(frontend) < 0)
				{
					pool_error("wait_for_query_response: frontend error occured while waiting for backend reply");
					return POOL_END;
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
	if (pool_write_and_flush(cp, &sendlen, sizeof(sendlen)) < 0)
	{
		return POOL_ERROR;
	}

	return POOL_CONTINUE;
}

POOL_STATUS send_execute_message(POOL_CONNECTION_POOL *backend,
										int node_id, int len, char *string)
{
	return send_extended_protocol_message(backend, node_id, "E", len, string);
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

			pool_error("pool_check_fd: select() failed. reason %s", strerror(errno));
			break;
		}
		else if (fds == 0)		/* timeout */
			return 1;

		if (FD_ISSET(fd, &exceptmask))
		{
			pool_error("pool_check_fd: exception occurred");
			break;
		}
		return 0;
	}
	return -1;
}

/*
 * Process "show pool_status" query.
 */
void process_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	static char *cursorname = "blank";
	static short num_fields = 3;
	static char *field_names[] = {"item", "value", "description"};
	static int oid = 0;
	static short fsize = -1;
	static int mod = 0;
	short n;
	int i, j;
	short s;
	int len;
	short colnum;

	static unsigned char nullmap[2] = {0xff, 0xff};
	int nbytes = (num_fields + 7)/8;

#define POOLCONFIG_MAXNAMELEN 32
#define POOLCONFIG_MAXVALLEN 512
#define POOLCONFIG_MAXDESCLEN 64

	typedef struct {
		char name[POOLCONFIG_MAXNAMELEN+1];
		char value[POOLCONFIG_MAXVALLEN+1];
		char desc[POOLCONFIG_MAXDESCLEN+1];
	} POOL_REPORT_STATUS;

/*
 * Report data buffer.
 * 128 is the max number of configuration items.
 * In addition, we need MAX_NUM_BACKENDS*4
 * for backend descriptions.
 */
#define MAXITEMS (128 + MAX_NUM_BACKENDS*4)		

	static POOL_REPORT_STATUS status[MAXITEMS];

	short nrows;
	int size;
	int hsize;

	i = 0;

	strncpy(status[i].name, "listen_addresses", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->listen_addresses);
	strncpy(status[i].desc, "host name(s) or IP address(es) to listen to", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "port", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->port);
	strncpy(status[i].desc, "pgpool accepting port number", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "socket_dir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->socket_dir);
	strncpy(status[i].desc, "pgpool socket directory", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "num_init_children", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->num_init_children);
	strncpy(status[i].desc, "# of children initially pre-forked", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "child_life_time", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->child_life_time);
	strncpy(status[i].desc, "if idle for this seconds, child exits", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "connection_life_time", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->connection_life_time);
	strncpy(status[i].desc, "if idle for this seconds, connection closes", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "client_idle_limit", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->client_idle_limit);
	strncpy(status[i].desc, "if idle for this seconds, child connection closes", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "child_max_connections", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->child_max_connections);
	strncpy(status[i].desc, "if max_connections received, chile exits", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "max_pool", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->max_pool);
	strncpy(status[i].desc, "max # of connection pool per child", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "authentication_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->authentication_timeout);
	strncpy(status[i].desc, "maximum time in seconds to complete client authentication", POOLCONFIG_MAXNAMELEN);
	i++;

	strncpy(status[i].name, "logdir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->logdir);
	strncpy(status[i].desc, "logging directory", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "pid_file_name", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->pid_file_name);
	strncpy(status[i].desc, "path to pid file", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "backend_socket_dir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->backend_socket_dir);
	strncpy(status[i].desc, "Unix domain socket directory for the PostgreSQL server", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "replication_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->replication_mode);
	strncpy(status[i].desc, "non 0 if operating in replication mode", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "load_balance_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->load_balance_mode);
	strncpy(status[i].desc, "non 0 if operating in load balancing mode", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "replication_stop_on_mismatch", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->replication_stop_on_mismatch);
	strncpy(status[i].desc, "stop replication mode on fatal error", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "replicate_select", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->replicate_select);
	strncpy(status[i].desc, "non 0 if SELECT statement is replicated", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "reset_query_list", POOLCONFIG_MAXNAMELEN);
	*(status[i].value) = '\0';
	for (j=0;j<pool_config->num_reset_queries;j++)
	{
		int len;
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		strncat(status[i].value, pool_config->reset_query_list[j], len);
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		strncat(status[i].value, ";", len);
	}
	strncpy(status[i].desc, "queries issued at the end of session", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "print_timestamp", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->print_timestamp);
	strncpy(status[i].desc, "if true print time stamp to each log line", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "master_slave_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->master_slave_mode);
	strncpy(status[i].desc, "if true, operate in master/slave mode", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "connection_cache", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->connection_cache);
	strncpy(status[i].desc, "if true, cache connection pool", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "health_check_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->health_check_timeout);
	strncpy(status[i].desc, "health check timeout", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "health_check_period", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->health_check_period);
	strncpy(status[i].desc, "health check period", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "health_check_user", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->health_check_user);
	strncpy(status[i].desc, "health check user", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "failover_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->failover_command);
	strncpy(status[i].desc, "failover command", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "failback_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->failover_command);
	strncpy(status[i].desc, "failback command", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "insert_lock", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->insert_lock);
	strncpy(status[i].desc, "insert lock", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "ignore_leading_white_space", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->ignore_leading_white_space);
	strncpy(status[i].desc, "ignore leading white spaces", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "replication_enabled", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->replication_enabled);
	strncpy(status[i].desc, "non 0 if actually operating in replication mode", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "master_slave_enabled", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->master_slave_enabled);
	strncpy(status[i].desc, "non 0 if actually operating in master/slave", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "num_reset_queries", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->num_reset_queries);
	strncpy(status[i].desc, "number of queries in reset_query_list", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "pcp_port", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->pcp_port);
	strncpy(status[i].desc, "PCP port # to bind", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "pcp_socket_dir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->pcp_socket_dir);
	strncpy(status[i].desc, "PCP socket directory", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "pcp_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->pcp_timeout);
	strncpy(status[i].desc, "PCP timeout for an idle client", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "log_statement", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->log_statement);
	strncpy(status[i].desc, "if non 0, logs all SQL statements", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "log_connections", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->log_connections);
	strncpy(status[i].desc, "if true, print incoming connections to the log", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "log_hostname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->log_hostname);
	strncpy(status[i].desc, "if true, resolve hostname for ps and log print", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "enable_pool_hba", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->enable_pool_hba);
	strncpy(status[i].desc, "if true, use pool_hba.conf for client authentication", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "recovery_user", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->recovery_user);
	strncpy(status[i].desc, "online recovery user", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "recovery_password", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->recovery_password);
	strncpy(status[i].desc, "online recovery password", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "recovery_1st_stage_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->recovery_1st_stage_command);
	strncpy(status[i].desc, "execute a command in first stage.", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "recovery_2nd_stage_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->recovery_2nd_stage_command);
	strncpy(status[i].desc, "execute a command in second stage.", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "recovery_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->recovery_timeout);
	strncpy(status[i].desc, "max time in seconds to wait for the recovering node's postmaster", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "client_idle_limit_in_recovery", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->client_idle_limit_in_recovery);
	strncpy(status[i].desc, "if idle for this seconds, child connection closes in recovery 2nd statge", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "parallel_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->parallel_mode);
	strncpy(status[i].desc, "if non 0, run in parallel query mode", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "enable_query_cache", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->enable_query_cache);
	strncpy(status[i].desc, "if non 0, use query cache", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "pgpool2_hostname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->pgpool2_hostname);
	strncpy(status[i].desc, "pgpool2 hostname", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "system_db_hostname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_hostname);
	strncpy(status[i].desc, "system DB hostname", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "system_db_port", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->system_db_port);
	strncpy(status[i].desc, "system DB port number", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "system_db_dbname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_dbname);
	strncpy(status[i].desc, "system DB name", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "system_db_schema", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_schema);
	strncpy(status[i].desc, "system DB schema name", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "system_db_user", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_user);
	strncpy(status[i].desc, "user name to access system DB", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "system_db_password", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_password);
	strncpy(status[i].desc, "password to access system DB", POOLCONFIG_MAXDESCLEN);
	i++;

	for (j = 0; j < NUM_BACKENDS; j++)
	{
		if (BACKEND_INFO(j).backend_port == 0)
			continue;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "backend_hostname%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", BACKEND_INFO(j).backend_hostname);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "backend #%d hostname", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "backend_port%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", BACKEND_INFO(j).backend_port);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "backend #%d port number", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "backend_weight%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%f", BACKEND_INFO(j).backend_weight);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "weight of backend #%d", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "backend status%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", BACKEND_INFO(j).backend_status);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "status of backend #%d", j);
		i++;
	}

	nrows = i;

	if (MAJOR(backend) == PROTO_MAJOR_V2)
	{
		/* cursor response */
		pool_write(frontend, "P", 1);
		pool_write(frontend, cursorname, strlen(cursorname)+1);
	}

	/* row description */
	pool_write(frontend, "T", 1);

	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		len = sizeof(num_fields) + sizeof(len);

		for (i=0;i<num_fields;i++)
		{
			char *f = field_names[i];
			len += strlen(f)+1;
			len += sizeof(oid);
			len += sizeof(colnum);
			len += sizeof(oid);
			len += sizeof(s);
			len += sizeof(mod);
			len += sizeof(s);
		}

		len = htonl(len);
		pool_write(frontend, &len, sizeof(len));
	}

	n = htons(num_fields);
	pool_write(frontend, &n, sizeof(short));

	for (i=0;i<num_fields;i++)
	{
		char *f = field_names[i];

		pool_write(frontend, f, strlen(f)+1);		/* field name */

		if (MAJOR(backend) == PROTO_MAJOR_V3)
		{
			pool_write(frontend, &oid, sizeof(oid));	/* table oid */
			colnum = htons(i);
			pool_write(frontend, &colnum, sizeof(colnum));	/* column number */
		}

		pool_write(frontend, &oid, sizeof(oid));		/* data type oid */
		s = htons(fsize);
		pool_write(frontend, &s, sizeof(fsize));		/* field size */
		pool_write(frontend, &mod, sizeof(mod));		/* modifier */

		if (MAJOR(backend) == PROTO_MAJOR_V3)
		{
			s = htons(0);
			pool_write(frontend, &s, sizeof(fsize));	/* field format (text) */
		}
	}
	pool_flush(frontend);

	if (MAJOR(backend) == PROTO_MAJOR_V2)
	{
		/* ascii row */
		for (i=0;i<nrows;i++)
		{
			pool_write(frontend, "D", 1);
			pool_write_and_flush(frontend, nullmap, nbytes);

			size = strlen(status[i].name);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, status[i].name, size);

			size = strlen(status[i].value);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, status[i].value, size);

			size = strlen(status[i].desc);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, status[i].desc, size);
		}
	}
	else
	{
		/* data row */
		for (i=0;i<nrows;i++)
		{
			pool_write(frontend, "D", 1);
			len = sizeof(len) + sizeof(nrows);
			len += sizeof(int) + strlen(status[i].name);
			len += sizeof(int) + strlen(status[i].value);
			len += sizeof(int) + strlen(status[i].desc);
			len = htonl(len);
			pool_write(frontend, &len, sizeof(len));
			s = htons(3);
			pool_write(frontend, &s, sizeof(s));

			len = htonl(strlen(status[i].name));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, status[i].name, strlen(status[i].name));

			len = htonl(strlen(status[i].value));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, status[i].value, strlen(status[i].value));

			len = htonl(strlen(status[i].desc));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, status[i].desc, strlen(status[i].desc));
		}
	}

	/* complete command response */
	pool_write(frontend, "C", 1);
	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		len = htonl(sizeof(len) + strlen("SELECT")+1);
		pool_write(frontend, &len, sizeof(len));
	}
	pool_write(frontend, "SELECT", strlen("SELECT")+1);

	/* ready for query */
	pool_write(frontend, "Z", 1);
	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		len = htonl(sizeof(len) + 1);
		pool_write(frontend, &len, sizeof(len));
		pool_write(frontend, "I", 1);
	}

	pool_flush(frontend);
}

/*
 * send "terminate"(X) message to all backends, indicating that
 * backend should prepare to close connection to frontend (actually
 * pgpool). Note that caller must be protecedt from a signal
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
			pool_write(CONNECTION(backend, i), "X", 1);

			if (MAJOR(backend) == PROTO_MAJOR_V3)
			{
				len = htonl(4);
				pool_write(CONNECTION(backend, i), &len, sizeof(len));
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
static POOL_STATUS ParallelForwardToFrontend(char kind, POOL_CONNECTION *frontend, POOL_CONNECTION *backend, char *database, bool send_to_frontend)
{
	int len;
	char *p;
	int status;

	if (send_to_frontend)
	{
		pool_write(frontend, &kind, 1);
	}

	status = pool_read(backend, &len, sizeof(len));
	if (status < 0)
	{
		pool_error("ParallelForwardToFrontend: error while reading message length");
		return POOL_END;
	}

	if (send_to_frontend)
	{
		pool_write(frontend, &len, sizeof(len));
	}

	len = ntohl(len) - 4 ;

	if (len <= 0)
		return POOL_CONTINUE;

	p = pool_read2(backend, len);
	if (p == NULL)
		return POOL_END;

	status = POOL_CONTINUE;
	if (send_to_frontend)
	{
		status = pool_write(frontend, p, len);
		if (pool_config->enable_query_cache && SYSDB_STATUS == CON_UP && status == 0)
		{
			query_cache_register(kind, frontend, database, p, len);
		}
	}

	return status;
}

POOL_STATUS SimpleForwardToFrontend(char kind, POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	int len, len1 = 0;
	char *p = NULL;
	char *p1 = NULL;
	char *p2 = NULL;
	int status;
	int sendlen;
	int i;
	int command_ok_row_count = 0;
	int delete_or_update = 0;
	char kind1;
	POOL_STATUS ret;

	/*
	 * Check if packet kind == 'C'(Command complete), '1'(Parse
	 * complete), '3'(Close complete). If so, then register or
	 * unregister pending prepared statement.
	 */
	if ((kind == 'C' || kind == '1' || kind == '3') &&
		pending_function)
	{
		pending_function(&prepared_list, pending_prepared_portal);
		if (pending_prepared_portal &&
			pending_prepared_portal->stmt &&
			IsA(pending_prepared_portal->stmt, DeallocateStmt))
		{
			free(pending_prepared_portal->portal_name);
			pending_prepared_portal->portal_name = NULL;
			pool_memory_delete(pending_prepared_portal->prepare_ctxt, 0);
			free(pending_prepared_portal);
		}
	}
	else if (kind == 'E' && pending_function)
	{
		/* An error occurred with PREPARE or DEALLOCATE command.
		 * Free pending portal object.
		 */
		if (pending_prepared_portal)
		{
			free(pending_prepared_portal->portal_name);
			pending_prepared_portal->portal_name = NULL;
			pool_memory_delete(pending_prepared_portal->prepare_ctxt, 0);
			free(pending_prepared_portal);
		}
	}
	else if (kind == 'C' && select_in_transaction)
	{
		select_in_transaction = 0;
		execute_select = 0;
	}

	/*
	 * Remove a pending function if a received message is not
	 * NoticeResponse.
	 */
	if (kind != 'N')
	{
		pending_function = NULL;
		pending_prepared_portal = NULL;
	}

	status = pool_read(MASTER(backend), &len, sizeof(len));
	len = ntohl(len);
	len -= 4;
	len1 = len;

	p = pool_read2(MASTER(backend), len);
	if (p == NULL)
		return POOL_END;
	p1 = malloc(len);
	if (p1 == NULL)
	{
		pool_error("SimpleForwardToFrontend: malloc failed");
		return POOL_ERROR;
	}
	memcpy(p1, p, len);

	if (kind == 'C')	/* packet kind is "Command Complete"? */
	{
		command_ok_row_count = extract_ntuples(p);

		/*
		 * if we are in the parallel mode, we have to sum up the number
		 * of affected rows
		 */
		if (PARALLEL_MODE && is_parallel_table &&
			(strstr(p, "UPDATE") || strstr(p, "DELETE")))
		{
			delete_or_update = 1;
		}
	}

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
		{
			status = pool_read(CONNECTION(backend, i), &len, sizeof(len));
			if (status < 0)
			{
				pool_error("SimpleForwardToFrontend: error while reading message length");
				return POOL_END;
			}

			len = ntohl(len);
			len -= 4;

			p = pool_read2(CONNECTION(backend, i), len);
			if (p == NULL)
				return POOL_END;

			if (len != len1)
			{
				pool_debug("SimpleForwardToFrontend: length does not match between backends master(%d) %d th backend(%d) kind:(%c)",
 						   len, i, len1, kind);
			}

			if (kind == 'C')	/* packet kind is "Command Complete"? */
			{
				int n = extract_ntuples(p);

				/*
				 * if we are in the parallel mode, we have to sum up the number
				 * of affected rows
				 */
				if (delete_or_update)
				{
					command_ok_row_count += n;
				}
				else if (command_ok_row_count != n) /* mismatch update rows */
				{
					mismatch_ntuples = 1;
				}
			}
		}
	}

	if (mismatch_ntuples)
	{
		String *msg = init_string("pgpool detected difference of the number of inserted, updated or deleted tuples. Possible last query was: \"");
		string_append_char(msg, query_string_buffer);
		string_append_char(msg, "\"");
		pool_send_error_message(frontend, MAJOR(backend),
								"XX001", msg->data, "",
								"check data consistency between master and other db node",  __FILE__, __LINE__);
		pool_error(msg->data);
		free_string(msg);
	}
	else
	{
		if (delete_or_update)
		{
			char tmp[32];

			strncpy(tmp, p1, 7);
			sprintf(tmp+7, "%d", command_ok_row_count);

			p2 = strdup(tmp);
			if (p2 == NULL)
			{
				pool_error("SimpleForwardToFrontend: malloc failed");
				free(p1);
				return POOL_ERROR;
			}

			free(p1);
			p1 = p2;
			len1 = strlen(p2) + 1;
		}

		pool_write(frontend, &kind, 1);
		sendlen = htonl(len1+4);
		pool_write(frontend, &sendlen, sizeof(sendlen));
		pool_write(frontend, p1, len1);
	}

	/* save the received result for each kind */
	if (pool_config->enable_query_cache && SYSDB_STATUS == CON_UP)
	{
		query_cache_register(kind, frontend, backend->info->database, p1, len1);
	}

	free(p1);
	if (status)
		return POOL_END;

	if (kind == 'A')	/* notification response */
	{
		pool_flush(frontend);	/* we need to immediately notice to frontend */
	}
	else if (kind == 'E')		/* error response? */
	{
		int i;
		int res1;
		char *p1;

		/*
		 * check if the error was PANIC or FATAL. If so, we just flush
		 * the message and exit since the backend will close the
		 * channel immediately.
		 */
		for (;;)
		{
			char e;

			e = *p++;
			if (e == '\0')
				break;

			if (e == 'S' && (strcasecmp("PANIC", p) == 0 || strcasecmp("FATAL", p) == 0))
			{
				pool_flush(frontend);
				return POOL_END;
			}
			else
			{
				while (*p++)
					;
				continue;
			}
		}

		if (select_in_transaction)
		{
			int i;

			in_load_balance = 0;
			REPLICATION = 1;
			for (i = 0; i < NUM_BACKENDS; i++)
			{
				if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
				{
					/*
					 * We must abort transaction to sync transaction state.
					 * If the error was caused by an Execute message,
					 * we must send invalid Execute message to abort
					 * transaction.
					 *
					 * Because extended query protocol ignores all
					 * messages before receiving Sync message inside error state.
					 */
					if (execute_select)
						do_error_execute_command(backend, i, PROTO_MAJOR_V3);
					else
						do_error_command(CONNECTION(backend, i), PROTO_MAJOR_V3);
				}
			}
			select_in_transaction = 0;
			execute_select = 0;
		}

		for (i = 0;i < NUM_BACKENDS; i++)
		{
			if (VALID_BACKEND(i))
			{
				POOL_CONNECTION *cp = CONNECTION(backend, i);

				/* We need to send "sync" message to backend in extend mode
				 * so that it accepts next command.
				 * Note that this may be overkill since client may send
				 * it by itself. Moreover we do not need it in non-extend mode.
				 * At this point we regard it is not harmful since error response
				 * will not be sent too frequently.
				 */
				pool_write(cp, "S", 1);
				res1 = htonl(4);
				if (pool_write_and_flush(cp, &res1, sizeof(res1)) < 0)
				{
					return POOL_END;
				}
			}
		}

		while ((ret = read_kind_from_backend(frontend, backend, &kind1)) == POOL_CONTINUE)
		{
			if (kind1 == 'Z') /* ReadyForQuery? */
				break;

			ret = SimpleForwardToFrontend(kind1, frontend, backend);
			if (ret != POOL_CONTINUE)
				return ret;
			pool_flush(frontend);
		}

		if (ret != POOL_CONTINUE)
			return ret;

		for (i = 0; i < NUM_BACKENDS; i++)
		{
			if (VALID_BACKEND(i))
			{
				status = pool_read(CONNECTION(backend, i), &res1, sizeof(res1));
				if (status < 0)
				{
					pool_error("SimpleForwardToFrontend: error while reading message length");
					return POOL_END;
				}
				res1 = ntohl(res1) - sizeof(res1);
				p1 = pool_read2(CONNECTION(backend, i), res1);
				if (p1 == NULL)
					return POOL_END;
			}
		}
	}
	return POOL_CONTINUE;
}

POOL_STATUS SimpleForwardToBackend(char kind, POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	int len;
	int sendlen;
	char *p;
	int i;
	char *name;
	POOL_STATUS ret;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			if (pool_write(CONNECTION(backend, i), &kind, 1))
				return POOL_END;
		}
	}

	if (pool_read(frontend, &sendlen, sizeof(sendlen)))
	{
		return POOL_END;
	}

	len = ntohl(sendlen) - 4;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			if (pool_write(CONNECTION(backend,i), &sendlen, sizeof(sendlen)))
				return POOL_END;
		}
	}

	if (len == 0)
		return POOL_CONTINUE;
	else if (len < 0)
	{
		pool_error("SimpleForwardToBackend: invalid message length");
		return POOL_END;
	}

	p = pool_read2(frontend, len);
	if (p == NULL)
		return POOL_END;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			if (pool_write_and_flush(CONNECTION(backend, i), p, len))
				return POOL_END;
		}
	}

	if (kind == 'B') /* Bind message */
	{
		Portal *portal = NULL;
		char *stmt_name, *portal_name;

		portal_name = p;
		stmt_name = p + strlen(portal_name) + 1;

		pool_debug("bind message: portal_name %s stmt_name %s", portal_name, stmt_name);

		if (*stmt_name == '\0')
			portal = unnamed_statement;
		else
		{
			portal = lookup_prepared_statement_by_statement(&prepared_list, stmt_name);
		}

		if (*portal_name == '\0'){
			unnamed_portal = portal;
		}
		else if (portal)
		{
			if (portal->portal_name)
				free(portal->portal_name);
			portal->portal_name = strdup(portal_name);
		}
	}

	/* Close message with prepared statement name. */
	else if (kind == 'C' && *p == 'S' && *(p + 1))
	{
		POOL_MEMORY_POOL *old_context = pool_memory;
		DeallocateStmt *deallocate_stmt;

		pending_prepared_portal = create_portal();
		if (pending_prepared_portal == NULL)
		{
			pool_error("SimpleForwardToBackend: malloc failed: %s", strerror(errno));
			return POOL_END;
		}

		pool_memory = pending_prepared_portal->prepare_ctxt;
		name = pstrdup(p+1);
		if (name == NULL)
		{
			pool_error("SimpleForwardToBackend: malloc failed: %s", strerror(errno));
			pool_memory = old_context;
			return POOL_END;
		}

		/* Translate from Close message to DEALLOCATE statement.*/
		deallocate_stmt = palloc(sizeof(DeallocateStmt));
		if (deallocate_stmt == NULL)
		{
			pool_error("SimpleForwardToBackend: malloc failed: %s", strerror(errno));
			pool_memory = old_context;
			return POOL_END;
		}
		deallocate_stmt->name = name;
		pending_prepared_portal->stmt = (Node *)deallocate_stmt;
		pending_prepared_portal->portal_name = NULL;
		pending_function = del_prepared_list;
		pool_memory = old_context;
	}

	if (kind == 'B' || kind == 'D' || kind == 'C')
	{
		int i;
		char kind1;

		for (i = 0;i < NUM_BACKENDS; i++)
		{
			if (VALID_BACKEND(i))
			{
				POOL_CONNECTION *cp = CONNECTION(backend, i);

				/*
				 * send "Flush" message so that backend notices us
				 * the completion of the command
				 */
				pool_write(cp, "H", 1);
				sendlen = htonl(4);
				if (pool_write_and_flush(cp, &sendlen, sizeof(sendlen)) < 0)
				{
					return POOL_END;
				}
			}
		}

		/*
		 * Describe message with a portal name will receive two messages.
		 * 1. ParameterDescription
		 * 2. RowDescriptions or NoData
		 * So we read one message here.
		 */
		if (kind == 'D' && *p == 'S')
		{
			ret = read_kind_from_backend(frontend, backend, &kind1);
			if (ret != POOL_CONTINUE)
				return ret;
			SimpleForwardToFrontend(kind1, frontend, backend);
			if (pool_flush(frontend))
				return POOL_END;
		}

		/*
		 * Forward to frontend until a NOTICE message received.
		 */
		for (;;)
		{
			ret = read_kind_from_backend(frontend, backend, &kind1);
			if (ret != POOL_CONTINUE)
				return ret;
			SimpleForwardToFrontend(kind1, frontend, backend);
			if (pool_flush(frontend) < 0)
				return POOL_ERROR;

			if (kind1 != 'N')
				break;
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
	{
		return POOL_END;
	}

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
				return POOL_END;

			name = p;
			value = p + strlen(name) + 1;

			pool_debug("%d th backend: name: %s value: %s", i, name, value);

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
	in_progress = 0;

	/* End load balance mode */
	if (in_load_balance)
		end_load_balance();

	if (master_slave_dml)
	{
		MASTER_SLAVE = 1;
		master_slave_was_enabled = 0;
		master_slave_dml = 0;
		if (force_replication)
		{
			force_replication = 0;
			REPLICATION = 0;
			replication_was_enabled = 0;
		}
	}

	internal_transaction_started = 0;
	mismatch_ntuples = 0;
	select_in_transaction = 0;
	execute_select = 0;
	receive_extended_begin = 0;
}

/*
 * Reset backend status. return values are:
 * 0: no query was issued 1: a query was issued 2: no more queries remain -1: error
 */
static int reset_backend(POOL_CONNECTION_POOL *backend, int qcnt)
{
	char *query;
	int qn;

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
		if (prepared_list.cnt == 0)
		{
			/*
			 * Either no prepared objects were created or DISCARD ALL
			 * or DEALLOCATE ALL is on the reset_query_list and they
			 * were executed.  The latter causes call to
			 * reset_prepared_list which removes all prepared objects.
			 */
			reset_prepared_list(&prepared_list);
			return 2;
		}

		/* Delete from prepared list */
		if (send_deallocate(backend, &prepared_list, 0))
		{
			/* Deallocate failed. We are in unknown state. Ask caller
			 * to reset backend connection.
			 */
			reset_prepared_list(&prepared_list);
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
		del_prepared_list(&prepared_list, prepared_list.portal_list[0]);
		return 1;
	}

	query = pool_config->reset_query_list[qcnt];

	/* if transaction state is idle, we don't need to issue ABORT */
	if (TSTATE(backend) == 'I' && !strcmp("ABORT", query))
		return 0;

	pool_set_timeout(10);

	if (SimpleQuery(NULL, backend, query) != POOL_CONTINUE)
	{
		pool_set_timeout(0);
		return -1;
	}

	pool_set_timeout(0);
	return 1;
}

/*
 * return non 0 if load balance is possible
 */
int load_balance_enabled(POOL_CONNECTION_POOL *backend, Node* node, char *sql)
{
	return (pool_config->load_balance_mode &&
			(DUAL_MODE || pool_config->parallel_mode) &&
			MAJOR(backend) == PROTO_MAJOR_V3 &&
			TSTATE(backend) == 'I' &&
			is_select_query(node, sql) &&
			!is_sequence_query(node));
}


/*
 * returns non 0 if the SQL statement can be load
 * balanced. Followings are statemnts go into this category.
 *
 * - SELECT/WITH without FOR UPDATE/SHARE
 * - COPY TO STDOUT
 *
 * note that for SELECT INTO, this function returns 0
 */
int is_select_query(Node *node, char *sql)
{
	if (node == NULL)
		return 0;

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
		return 0;

	if (pool_config->ignore_leading_white_space)
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
			return 0;

		return (*sql == 's' || *sql == 'S' || *sql == '(' ||
				*sql == 'w' || *sql == 'W' || *sql == 't' || *sql == 'T');
	}
	else if (IsA(node, CopyStmt))
	{
		CopyStmt *copy_stmt = (CopyStmt *)node;
		return (copy_stmt->is_from == FALSE &&
				copy_stmt->filename == NULL);
	}
	return 0;
}

/*
 * returns non 0 if SQL is SELECT statement including nextval() or
 * setval() call
 */
int is_sequence_query(Node *node)
{
	SelectStmt *select_stmt;
	ListCell *lc;

	if (node == NULL || !IsA(node, SelectStmt))
		return 0;

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
						return 1;
					else if (strncasecmp(v->val.str, "SETVAL", 6) == 0)
						return 1;
				}
			}
		}
	}

	return 0;
}

/*
 * returns non 0 if SQL is transaction starting command (START
 * TRANSACTION or BEGIN)
 */
int is_start_transaction_query(Node *node)
{
	TransactionStmt *stmt;

	if (node == NULL || !IsA(node, TransactionStmt))
		return 0;

	stmt = (TransactionStmt *)node;
	return stmt->kind == TRANS_STMT_START || stmt->kind == TRANS_STMT_BEGIN;
}

/*
 * returns non 0 if SQL is transaction commit or abort command (END
 * TRANSACTION or ROLLBACK or ABORT)
 */
int is_commit_query(Node *node)
{
	TransactionStmt *stmt;

	if (node == NULL || !IsA(node, TransactionStmt))
		return 0;

	stmt = (TransactionStmt *)node;
	return stmt->kind == TRANS_STMT_COMMIT || stmt->kind == TRANS_STMT_ROLLBACK;
}

/*
 * start load balance mode
 */
void start_load_balance(POOL_CONNECTION_POOL *backend)
{
#ifdef NOT_USED
	double total_weight,r;
	int i;

	/* save backend connection slots */
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			slots[i] = CONNECTION_SLOT(backend, i);
		}
	}
#endif

	/* temporarily turn off replication mode */
	if (REPLICATION)
		replication_was_enabled = 1;
	if (MASTER_SLAVE)
		master_slave_was_enabled = 1;

	REPLICATION = 0;
	MASTER_SLAVE = 0;

#ifdef NOTUSED
	backend->slots[0] = slots[selected_slot];
#endif
	LOAD_BALANCE_STATUS(backend->info->load_balancing_node) = LOAD_SELECTED;
	selected_slot = backend->info->load_balancing_node;

	/* start load balancing */
	in_load_balance = 1;
}

/*
 * Finish load balance mode
 */
void end_load_balance(void)
{
	in_load_balance = 0;
	LOAD_BALANCE_STATUS(selected_slot) = LOAD_UNSELECTED;

	/* turn on replication mode */
	REPLICATION = replication_was_enabled;
	MASTER_SLAVE = master_slave_was_enabled;

	replication_was_enabled = 0;
	master_slave_was_enabled = 0;

	pool_debug("end_load_balance: end load balance mode");
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
		char msgbuf[MAXMSGBUF];
		int len;
		int thislen;
		int sendlen;

		len = 0;

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
		if (*detail != '\0')
		{
			thislen = snprintf(msgbuf, MAXMSGBUF, "D%s", detail);
			thislen = Min(thislen, MAXMSGBUF);
			memcpy(data +len, msgbuf, thislen+1);
			len += thislen + 1;
		}

		/* hint */
		if (*hint != '\0')
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
		pool_error("send_error_message: unknown protocol major %d", protoMajor);

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
static POOL_STATUS do_command(POOL_CONNECTION *frontend, POOL_CONNECTION *backend,
							  char *query, int protoMajor, int pid, int key, int no_ready_for_query)
{
	int len;
	int status;
	char kind;
	char *string;
	int deadlock_detected = 0;

	pool_debug("do_command: Query: %s", query);

	/* send the query to the backend */
	if (send_simplequery_message(backend, strlen(query)+1, query, protoMajor) != POOL_CONTINUE)
		return POOL_END;

	/*
	 * Wait for response from badckend while polling frontend connection is ok.
	 * If not, cancel the transaction.
	 */
	if (wait_for_query_response(frontend, backend, query, protoMajor) != POOL_CONTINUE)
	{
		/* Cancel current transaction */
		CancelPacket cancel_packet;

		cancel_packet.protoVersion = htonl(PROTO_CANCEL);
		cancel_packet.pid = pid;
		cancel_packet.key= key;
		cancel_request(&cancel_packet);
		return POOL_END;
	}

 	/*
	 * We must check deadlock error here. If a deadlock error is
	 * detected by a backend, other backend might not be noticed the
	 * error.  In this case caller should send an error query to the
	 * backend to abort the transaction. Otherwise the transaction
	 * state might vary among backends(idle in transaction vs. abort).
	 */
	deadlock_detected = detect_deadlock_error(backend, protoMajor);
	if (deadlock_detected < 0)
		return POOL_END;

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
		status = pool_read(backend, &kind, sizeof(kind));
		if (status < 0)
		{
			pool_error("do_command: error while reading message kind");
			return POOL_END;
		}

		pool_debug("do_command: kind: %c", kind);

		if (kind == 'Z')		/* Ready for Query? */
			break;		/* get out the loop without reading message lenghth */

		if (protoMajor == PROTO_MAJOR_V3)
		{
			if (pool_read(backend, &len, sizeof(len)) < 0)
			{
				pool_error("do_command: error while reading message length");
				return POOL_END;
			}
			len = ntohl(len) - 4;
			
			if (kind != 'N' && kind != 'E' && kind != 'S' && kind != 'C')
			{
				pool_error("do_command: error, kind is not N, E, S or C(%02x)", kind);
				return POOL_END;
			}
			string = pool_read2(backend, len);
			if (string == NULL)
			{
				pool_error("do_command: error while reading rest of message");
				return POOL_END;
			}
		}
		else
		{
			string = pool_read_string(backend, &len, 0);
			if (string == NULL)
			{
				pool_error("do_command: error while reading rest of message");
				return POOL_END;
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
	status = pool_read(backend, &kind, sizeof(kind));
	if (status < 0)
	{
		pool_error("do_command: error while reading message kind");
		return POOL_END;
	}

	if (kind == 'E')
	{
		pool_log("do_command: backend does not successfully complete command %s status %c", query, kind);
	}

	/*
	 * read command tag of CommandComplete response
	 */
	if (protoMajor == PROTO_MAJOR_V3)
	{
		if (pool_read(backend, &len, sizeof(len)) < 0)
			return POOL_END;
		len = ntohl(len) - 4;
		string = pool_read2(backend, len);
		if (string == NULL)
			return POOL_END;
		pool_debug("command tag: %s", string);
	}
	else
	{
		string = pool_read_string(backend, &len, 0);
		if (string == NULL)
			return POOL_END;
	}

	if (kind == 'N') /* warning? */
		goto retry_read_packet;

	/*
	 * Expecting ReadyForQuery
	 */
	status = pool_read(backend, &kind, sizeof(kind));
	if (status < 0)
	{
		pool_error("do_command: error while reading message kind");
		return POOL_END;
	}

	if (kind != 'Z')
	{
		pool_error("do_command: backend returns %c while expecting ReadyForQuery", kind);
		return POOL_END;
	}
#endif

	if (no_ready_for_query)
		return POOL_CONTINUE;

	if (protoMajor == PROTO_MAJOR_V3)
	{
		/* read packet lenghth for ready for query */
		if (pool_read(backend, &len, sizeof(len)) < 0)
		{
			pool_error("do_command: error while reading message length");
			return POOL_END;
		}

		/* read transaction state */
		status = pool_read(backend, &kind, sizeof(kind));
		if (status < 0)
		{
			pool_error("do_command: error while reading transaction status");
			return POOL_END;
		}

		/* set transaction state */
		pool_debug("do_command: transaction state: %c", kind);
		backend->tstate = kind;
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
POOL_STATUS do_error_command(POOL_CONNECTION *backend, int major)
{
	char *error_query = POOL_ERROR_QUERY;
	int status, len;
	char kind;
	char *string;

	if (send_simplequery_message(backend, strlen(error_query) + 1, error_query, major) != POOL_CONTINUE)
	{
		return POOL_END;
	}

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
		status = pool_read(backend, &kind, sizeof(kind));
		if (status < 0)
		{
			pool_error("do_error_command: error while reading message kind");
			return POOL_END;
		}

		pool_debug("do_error_command: kind: %c", kind);

		if (major == PROTO_MAJOR_V3)
		{
			if (pool_read(backend, &len, sizeof(len)) < 0)
			{
				pool_error("do_error_command: error while reading message length");
				return POOL_END;
			}
			len = ntohl(len) - 4;
			string = pool_read2(backend, len);
			if (string == NULL)
			{
				pool_error("do_error_command: error while reading rest of message");
				return POOL_END;
			}
		}
		else
		{
			string = pool_read_string(backend, &len, 0);
			if (string == NULL)
			{
				pool_error("do_error_command: error while reading rest of message");
				return POOL_END;
			}
		}
	} while (kind != 'E');

#ifdef NOT_USED
	/*
	 * Expecting ErrorResponse
	 */
	status = pool_read(backend, &kind, sizeof(kind));
	if (status < 0)
	{
		pool_error("do_command: error while reading message kind");
		return POOL_END;
	}

	/*
	 * read command tag of CommandComplete response
	 */
	if (major == PROTO_MAJOR_V3)
	{
		if (pool_read(backend, &len, sizeof(len)) < 0)
			return POOL_END;
		len = ntohl(len) - 4;
		string = pool_read2(backend, len);
		if (string == NULL)
			return POOL_END;
		pool_debug("command tag: %s", string);
	}
	else
	{
		string = pool_read_string(backend, &len, 0);
		if (string == NULL)
			return POOL_END;
	}
#endif
	return POOL_CONTINUE;
}

/*
 * Send invalid portal execution to abort transaction.
 * We need to sync transaction status in transaction block.
 * SELECT query is sent to master only.
 * If SELECT is error, we must abort transaction on other nodes.
 */
static POOL_STATUS do_error_execute_command(POOL_CONNECTION_POOL *backend, int node_id, int major)
{
	int status;
	char kind;
	char *string;
	char msg[1024] = "pgpoool_error_portal"; /* large enough */
	int len = strlen(msg);

	memset(msg + len, 0, sizeof(int));
	if (send_execute_message(backend, node_id, len + 5, msg))
	{
		return POOL_END;
	}

	/*
	 * Expecting ErrorResponse
	 */
	status = pool_read(CONNECTION(backend, node_id), &kind, sizeof(kind));
	if (status < 0)
	{
		pool_error("do_error_execute_command: error while reading message kind");
		return POOL_END;
	}

	/*
	 * read command tag of CommandComplete response
	 */
	if (major == PROTO_MAJOR_V3)
	{
		if (pool_read(CONNECTION(backend, node_id), &len, sizeof(len)) < 0)
			return POOL_END;
		len = ntohl(len) - 4;
		string = pool_read2(CONNECTION(backend, node_id), len);
		if (string == NULL)
			return POOL_END;
		pool_debug("command tag: %s", string);
	}
	else
	{
		string = pool_read_string(CONNECTION(backend, node_id), &len, 0);
		if (string == NULL)
			return POOL_END;
	}

	return POOL_CONTINUE;
}

/*
 * Transmit an arbitrary Query to a specific node.
 * This function is only used in parallel mode
 */
POOL_STATUS OneNode_do_command(POOL_CONNECTION *frontend, POOL_CONNECTION *backend, char *query, char *database)
{
	int len,sendlen;
	int status;
	char kind;
	bool notice = false;

	pool_debug("OneNode_do_command: Query: %s", query);

	/* send the query to the backend */
	pool_write(backend, "Q", 1);
	len = strlen(query)+1;

	sendlen = htonl(len + 4);
	pool_write(backend, &sendlen, sizeof(sendlen));

	if (pool_write_and_flush(backend, query, len) < 0)
	{
		return POOL_END;
	}

	for(;;)
	{
		status = pool_read(backend, &kind, sizeof(kind));
		if (status < 0)
		{
			pool_error("OneNode_do_command: error while reading message kind");
			return POOL_END;
		}
		
		if (kind == 'N' && strstr(query,"dblink")) {
			notice = true;
			status = ParallelForwardToFrontend(kind, frontend, backend, database, false);
		} else {
			if(notice)
				status = ParallelForwardToFrontend(kind, frontend, backend, database, false);
			else
				status = ParallelForwardToFrontend(kind, frontend, backend, database, true);
		}
		if (kind == 'C' || kind =='E')
		{
			break;
		}
	}
	/*
	 * Expecting ReadyForQuery
	 *
	 */
	status = pool_read(backend, &kind, sizeof(kind));

	if(notice)
				pool_send_error_message(frontend, 3, "XX000",
										"pgpool2 sql restriction(notice from dblink)",query,"", 
										__FILE__,__LINE__);

	if (status < 0)
	{
		pool_error("OneNode_do_command: error while reading message kind");
		return POOL_END;
	}

	if (kind != 'Z')
	{
		pool_error("OneNode_do_command: backend does not return ReadyForQuery");
		return POOL_END;
	}


	status = ParallelForwardToFrontend(kind, frontend, backend, database, true);
	pool_flush(frontend);

		return status;
}

/*
 * Free POOL_SELECT_RESULT object
 */
void free_select_result(POOL_SELECT_RESULT *result)
{
	int i;

	if (result->nullflags)
		free(result->nullflags);

	if (result->data)
	{
		for(i=0;i<result->numrows;i++)
		{
			if (result->data[i])
				free(result->data[i]);
		}
		free(result->data);
	}

	if (result->rowdesc)
	{
		if (result->rowdesc->attrinfo)
		{
			for(i=0;i<result->rowdesc->num_attrs;i++)
			{
				if (result->rowdesc->attrinfo[i].attrname)
					free(result->rowdesc->attrinfo[i].attrname);
			}
			free(result->rowdesc->attrinfo);
		}
		free(result->rowdesc);
	}
}

/*
 * Send a SELECT to one DB node. This function works for V3 only.
 */
POOL_STATUS do_query(POOL_CONNECTION *backend, char *query, POOL_SELECT_RESULT **result)
{
#define DO_QUERY_ALLOC_NUM 1024	/* memory allocation unit for POOL_SELECT_RESULT */

	int i;
	int len;
	char kind;
	char *packet;
	char *p;
	short num_fields;
	int num_data;
	int intval;
	short shortval;

	POOL_SELECT_RESULT *res;
	RowDesc *rowdesc;
	AttrInfo *attrinfo;

	res = malloc(sizeof(*res));
	if (!res)
	{
		pool_error("pool_query: malloc failed");
		return POOL_ERROR;
	}
	rowdesc = malloc(sizeof(*rowdesc));
	if (!rowdesc)
	{
		pool_error("pool_query: malloc failed");
		return POOL_ERROR;
	}
	memset(res, 0, sizeof(*res));
	memset(rowdesc, 0, sizeof(*rowdesc));
	*result = res;

	res->rowdesc = rowdesc;

	num_data = 0;

	res->nullflags = malloc(DO_QUERY_ALLOC_NUM*sizeof(int));
	if (!res->nullflags)
	{
		pool_error("do_query: malloc failed");
		return POOL_ERROR;
	}
	res->data = malloc(DO_QUERY_ALLOC_NUM*sizeof(char *));
	if (!res->data)
	{
		pool_error("do_query: malloc failed");
		return POOL_ERROR;
	}

	/* send a query to the backend */
	if (send_simplequery_message(backend, strlen(query) + 1, query, PROTO_MAJOR_V3) != POOL_CONTINUE)
	{
		return POOL_END;
	}

	/*
	 * Continue to read packets until we get Ready for command('Z')
	 *
	 * XXX: we ignore other than Z here. Even notice messages are not sent
	 * to the frontend. May be it's ok since the error was caused by
	 * our internal use of SQL command (otherwise users will be
	 * confused).
	 */
	for(;;)
	{
		if (pool_read(backend, &kind, sizeof(kind)) < 0)
		{
			pool_error("do_query: error while reading message kind");
			return POOL_END;
		}

		pool_debug("do_query: kind: %c", kind);

		if (pool_read(backend, &len, sizeof(len)) < 0)
		{
			pool_error("do_query: error while reading message length");
			return POOL_END;
		}
		len = ntohl(len) - 4;
		packet = pool_read2(backend, len);
		if (packet == NULL)
		{
			pool_error("do_query: error while reading rest of message");
			return POOL_END;
		}

		switch (kind)
		{
			case 'Z':	/* Ready for query */
				return POOL_CONTINUE;
				break;

			case 'T':	/* Row Description */
				p = packet;
				memcpy(&shortval, p, sizeof(short));
				num_fields = ntohs(shortval);		/* number of fields */
				pool_debug("num_fileds: %d", num_fields);

				if (num_fields > 0)
				{
					rowdesc->num_attrs = num_fields;
					attrinfo = malloc(sizeof(*attrinfo)*num_fields);
					if (!attrinfo)
					{
						pool_error("do_query: malloc failed");
						return POOL_ERROR;
					}
					rowdesc->attrinfo = attrinfo;

					p += sizeof(num_fields);

					/* extract attribute info */
					for (i = 0;i<num_fields;i++)
					{
						len = strlen(p) + 1;
						attrinfo->attrname = malloc(len);
						if (!attrinfo->attrname)
						{
							pool_error("do_query: malloc failed");
							return POOL_ERROR;
						}
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

						attrinfo++;
					}
				}
				break;

			case 'D':	/* data row */
				p = packet;

				memcpy(&shortval, p, sizeof(short));
				num_fields = htons(shortval);
				p += sizeof(short);

				if (num_fields > 0)
				{
					res->numrows++;

					for (i=0;i<num_fields;i++)
					{
						memcpy(&intval, p, sizeof(int));
						len = htonl(intval);
						p += sizeof(int);

						res->nullflags[num_data] = len;

						if (len > 0)	/* NOT NULL? */
						{
							res->data[num_data] = malloc(len + 1);
							if (!res->data[num_data])
							{
								pool_error("do_query: malloc failed");
								return POOL_ERROR;
							}
							memcpy(res->data[num_data], p, len);
							*(res->data[num_data] + len) = '\0';

							p += len;
						}

						num_data++;

						if (num_data % DO_QUERY_ALLOC_NUM == 0)
						{
							res->nullflags = realloc(res->nullflags,
													 (num_data/DO_QUERY_ALLOC_NUM +1)*DO_QUERY_ALLOC_NUM*sizeof(int));
							if (!res->nullflags)
							{
								pool_error("do_query: malloc failed");
								return POOL_ERROR;
							}
							res->data = realloc(res->data,
												(num_data/DO_QUERY_ALLOC_NUM +1)*DO_QUERY_ALLOC_NUM*sizeof(char *));
							if (!res->data)
							{
								pool_error("do_query: malloc failed");
								return POOL_ERROR;
							}
						}
					}
				}
				break;

			default:
				break;
		}

	}
	return POOL_CONTINUE;
}

/*
 * Judge if we need to lock the table
 * to keep SERIAL consistency among servers
 */
int need_insert_lock(POOL_CONNECTION_POOL *backend, char *query, Node *node)
{
/*
 * Query to know if the target table has SERIAL column or not.
 * This query is valid through PostgreSQL 7.3 to 8.3.
 */
#define NEXTVALQUERY "SELECT count(*) FROM pg_catalog.pg_attrdef AS d, pg_catalog.pg_class AS c WHERE d.adrelid = c.oid AND d.adsrc ~ 'nextval' AND c.relname = '%s'"

	char *str;
	int result;
	static POOL_RELCACHE *relcache;

	/*
	 * for version 2 protocol, we cannot check if it's actually uses
	 * SERIAL data types or not since the underlying infrastructure
	 * (do_query) does not support the protocol. So we just return
	 * false.
	 */
	if (MAJOR(backend) == PROTO_MAJOR_V2)
		return 0;

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
		pool_error("need_insert_lock: get_insert_command_table_name failed");
		return 0;
	}

	/*
	 * If relcache does not exist, create it.
	 */
	if (!relcache)
	{
		relcache = pool_create_relcache(32, NEXTVALQUERY,
										int_register_func, int_unregister_func,
										false);
		if (relcache == NULL)
		{
			pool_error("need_insert_lock: pool_create_relcache error");
			return false;
		}
	}

	/*
	 * Search relcache.
	 */
	result = pool_search_relcache(relcache, backend, str)==0?0:1;
	return result;
}

/*
 * if a transaction has not already started, start a new one.
 * issue LOCK TABLE IN SHARE ROW EXCLUSIVE MODE
 */
POOL_STATUS insert_lock(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char *query, InsertStmt *node)
{
	char *table;
	char qbuf[1024];
	POOL_STATUS status;
	int i, deadlock_detected = 0;

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

	/* issue lock table command */
	snprintf(qbuf, sizeof(qbuf), "LOCK TABLE %s IN SHARE ROW EXCLUSIVE MODE", table);

	status = do_command(frontend, MASTER(backend), qbuf, MAJOR(backend), MASTER_CONNECTION(backend)->pid,
						MASTER_CONNECTION(backend)->key, 0);
	if (status == POOL_END)
	{
		internal_transaction_started = 0;
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
				status = do_command(frontend, CONNECTION(backend, i), qbuf, PROTO_MAJOR_V3, 
									MASTER_CONNECTION(backend)->pid, MASTER_CONNECTION(backend)->key, 0);

			if (status != POOL_CONTINUE)
			{
				internal_transaction_started = 0;
				return POOL_END;
			}
		}
	}

	return POOL_CONTINUE;
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
 * obtain table name in INSERT statement
 */
static char *get_insert_command_table_name(InsertStmt *node)
{
	char *table = nodeToString(node->relation);

	pool_debug("get_insert_command_table_name: extracted table name: %s", table);
	return table;
}

/* judge if this is a DROP DATABASE command */
int is_drop_database(Node *node)
{
	return (IsA(node, DropdbStmt)) ? 1 : 0;
}

/*
 * check if any pending data remains.  Also if there's some pending data in
 * frontend AND no processing any Query, then returns 0.
 * XXX: is this correct thing?
*/
static int is_cache_empty(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	int i;

	if (frontend->len > 0 && !in_progress)
		return 0;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		if (CONNECTION(backend, i)->len > 0)
			return 0;
	}

	return 1;
}

/*
 * check if query is needed to wait completion
 */
int is_strict_query(Node *node)
{
	switch (node->type)
	{
		case T_SelectStmt:
		{
			SelectStmt *stmt = (SelectStmt *)node;
			return (stmt->intoClause || stmt->lockingClause) ? 1 : 0;
		}

		case T_UpdateStmt:
		case T_InsertStmt:
		case T_DeleteStmt:
		case T_LockStmt:
			return 1;

		default:
			return 0;
	}

	return 0;
}

int check_copy_from_stdin(Node *node)
{
	if (copy_schema)
		free(copy_schema);
	if (copy_table)
		free(copy_table);
	if (copy_null)
		free(copy_null);

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
				copy_schema = strdup(relation->schemaname);
			else
				copy_schema = strdup("public");
			copy_table = strdup(relation->relname);

			copy_delimiter = '\t'; /* default delimiter */
			copy_null = strdup("\\N"); /* default null string */

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
						free(copy_null);
					v = (Value *)elem->arg;
					copy_null = strdup(v->val.str);
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
POOL_STATUS read_kind_from_one_backend(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char *kind, int node)
{
	if (VALID_BACKEND(node))
	{
		char k;
		if (pool_read(CONNECTION(backend, node), &k, 1) < 0)
		{
			pool_error("read_kind_from_one_backend: failed to read kind from %d th backend", node);
			return POOL_ERROR;
		}

		pool_debug("read_kind_from_one_backend: read kind from %d th backend %c", node, k);

		*kind = k;
		return POOL_CONTINUE;
	}
	else
	{
		pool_error("read_kind_from_one_backend: %d th backend is not valid", node);
		return POOL_ERROR;
	}
}

/*
 * read_kind_from_backend: read kind from backends.
 * the "frontend" parameter is used to send "kind mismatch" error message to the frontend.
 * the out parameter "decided_kind" is the packet kind decided by this function.
 * this function uses "decide by majority" method if kinds from all backends do not agree.
 */
POOL_STATUS read_kind_from_backend(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char *decided_kind)
{
	int i;
	unsigned char kind_list[MAX_NUM_BACKENDS];	/* records each backend's kind */
	unsigned char kind_map[256]; /* records which kind gets majority.
								  *	256 is the number of distinct values expressed by unsigned char
								 */
	unsigned char kind;
	int trust_kind;	/* decided kind */
	int max_kind = 0;
	double max_count = 0;
	int degenerate_node_num = 0;		/* number of backends degeneration requested */
	int degenerate_node[MAX_NUM_BACKENDS];		/* degeneration requested backend list */

	memset(kind_map, 0, sizeof(kind_map));

	for (i=0;i<NUM_BACKENDS;i++)
	{
		/* initialize degenerate record */
		degenerate_node[i] = 0;

		if (VALID_BACKEND(i))
		{
			do
			{
				char *p, *value;
				int len;

				if (pool_read(CONNECTION(backend, i), &kind, 1) < 0)
				{
					pool_error("read_kind_from_backend: failed to read kind from %d th backend", i);
					return POOL_ERROR;
				}

				/*
				 * Read and discard parameter status
				 */
				if (kind != 'S')
				{
					break;
				}

				if (pool_read(CONNECTION(backend, i), &len, sizeof(len)) < 0)
				{
					pool_error("read_kind_from_backend: failed to read parameter status packet length from %d th backend", i);
					return POOL_ERROR;
				}
				len = htonl(len) - 4;
				p = pool_read2(CONNECTION(backend, i), len);
				if (p == NULL)
				{
					pool_error("read_kind_from_backend: failed to read parameter status packet from %d th backend", i);
				}
				value = p + strlen(p) + 1;
				pool_debug("read_kind_from_backend: parameter name: %s value: %s", p, value);
			} while (kind == 'S');

			kind_list[i] = kind;

			pool_debug("read_kind_from_backend: read kind from %d th backend %c NUM_BACKENDS: %d", i, kind_list[i], NUM_BACKENDS);

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

#ifdef NOT_USED
	/* register kind map */
	for (i = 0; i < NUM_BACKENDS; i++)
	{
		/* initialize degenerate record */
		degenerate_node[i] = 0;

		/* kind is signed char.
		 * We must check negative number.
		 */
		int id = kind_list[i] + 128;

		if (kind_list[i] == -1)
			continue;

		kind_map[id]++;
		if (kind_map[id] > max_count)
		{
			max_kind = kind_list[i];
			max_count = kind_map[id];
		}
	}
#endif

	if (max_count != NUM_BACKENDS)
	{
		/*
		 * not all backends agree with kind. We need to do "decide by majority"
		 */

		if (max_count <= NUM_BACKENDS / 2.0)
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
			if (kind_list[i] != 0 && trust_kind != kind_list[i])
			{
				/* degenerate */
				pool_error("read_kind_from_backend: %d th kind %c does not match with master or majority connection kind %c",
						   i, kind_list[i], trust_kind);
				degenerate_node[degenerate_node_num++] = i;
			}
		}
	}
	else
		trust_kind = kind_list[MASTER_NODE_ID];

	*decided_kind = trust_kind;

	if (degenerate_node_num)
	{
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

					if (pool_extract_error_message(CONNECTION(backend, i), MAJOR(backend), true, &m) == POOL_CONTINUE)
					{
						string_append_char(msg, m);
						string_append_char(msg, "]");
					}
					else
					{
						string_append_char(msg, "unknown message]");
					}
				}
				else
				{
					snprintf(buf, sizeof(buf), " %d[%c]", i, kind_list[i]);
					string_append_char(msg, buf);
				}
			}
		}

		pool_send_error_message(frontend, MAJOR(backend), "XX000",
								msg->data, "",
								"check data consistency among db nodes",
								__FILE__, __LINE__);
		pool_error(msg->data);

		free_string(msg);

		if (pool_config->replication_stop_on_mismatch)
		{
			degenerate_backend_set(degenerate_node, degenerate_node_num);
			child_exit(1);
		}
		else
			return POOL_ERROR;
	}

	return POOL_CONTINUE;
}

/*
 * Create portal object
 * Return object is allocated from heap memory.
 */
Portal *create_portal(void)
{
	Portal *p;

	if ((p = malloc(sizeof(Portal))) == NULL)
		return NULL;

	p->prepare_ctxt = pool_memory_create(PREPARE_BLOCK_SIZE);
	if (p->prepare_ctxt == NULL)
	{
		free(p);
		return NULL;
	}
	return p;
}

void init_prepared_list(void)
{
	prepared_list.cnt = 0;
	prepared_list.size = INIT_STATEMENT_LIST_SIZE;
	prepared_list.portal_list = malloc(sizeof(Portal *) * prepared_list.size);
	if (prepared_list.portal_list == NULL)
	{
		pool_error("init_prepared_list: malloc failed: %s", strerror(errno));
		exit(1);
	}
}

void add_prepared_list(PreparedStatementList *p, Portal *portal)
{
	if (p->cnt == p->size)
	{
		p->size *= 2;
		p->portal_list = realloc(p->portal_list, sizeof(Portal *) * p->size);
		if (p->portal_list == NULL)
		{
			pool_error("add_prepared_list: realloc failed: %s", strerror(errno));
			exit(1);
		}
	}
	p->portal_list[p->cnt++] = portal;
}

void add_unnamed_portal(PreparedStatementList *p, Portal *portal)
{
	if (unnamed_statement)
	{
		pool_memory_delete(unnamed_statement->prepare_ctxt, 0);
		free(unnamed_statement);
	}

	unnamed_portal = NULL;
	unnamed_statement = portal;
}

void del_prepared_list(PreparedStatementList *p, Portal *portal)
{
	int i;
	DeallocateStmt *s = (DeallocateStmt *)portal->stmt;

	/* DEALLOCATE ALL? */
	if (s->name == NULL)
	{
		reset_prepared_list(p);
	}
	else
	{
		for (i = 0; i < p->cnt; i++)
		{
			PrepareStmt *p_stmt = (PrepareStmt *)p->portal_list[i]->stmt;
			if (strcmp(p_stmt->name, s->name) == 0)
				break;
		}

		if (i == p->cnt)
			return;

		pool_memory_delete(p->portal_list[i]->prepare_ctxt, 0);
		free(p->portal_list[i]->portal_name);
		free(p->portal_list[i]);
		if (i != p->cnt - 1)
		{
			memmove(&p->portal_list[i], &p->portal_list[i+1],
					sizeof(Portal *) * (p->cnt - i - 1));
		}
		p->cnt--;
	}
}

void delete_all_prepared_list(PreparedStatementList *p, Portal *portal)
{
	reset_prepared_list(p);
}

static void reset_prepared_list(PreparedStatementList *p)
{
	int i;

	if (p)
	{
		for (i = 0; i < p->cnt; i++)
		{
			pool_memory_delete(p->portal_list[i]->prepare_ctxt, 0);
			free(p->portal_list[i]->portal_name);
			free(p->portal_list[i]);
		}
		if (unnamed_statement)
		{
			pool_memory_delete(unnamed_statement->prepare_ctxt, 0);
			free(unnamed_statement);
		}
		unnamed_portal = NULL;
		unnamed_statement = NULL;
		p->cnt = 0;
	}
}

Portal *lookup_prepared_statement_by_statement(PreparedStatementList *p, const char *name)
{
	int i;

	/* unnamed portal? */
	if (name == NULL || name[0] == '\0' || (name[0] == '\"' && name[1] == '\"'))
		return unnamed_statement;

	for (i = 0; i < p->cnt; i++)
	{
		PrepareStmt *p_stmt = (PrepareStmt *)p->portal_list[i]->stmt;
		if (strcmp(p_stmt->name, name) == 0)
			return p->portal_list[i];
	}

	return NULL;
}

Portal *lookup_prepared_statement_by_portal(PreparedStatementList *p, const char *name)
{
	int i;

	/* unnamed portal? */
	if (name == NULL || name[0] == '\0' || (name[0] == '\"' && name[1] == '\"'))
		return unnamed_portal;

	for (i = 0; i < p->cnt; i++)
	{
		if (p->portal_list[i]->portal_name &&
			strcmp(p->portal_list[i]->portal_name, name) == 0)
			return p->portal_list[i];
	}

	return NULL;
}

/*
 * Send DEALLOCATE message to backend by using SimpleQuery.
 */
static int send_deallocate(POOL_CONNECTION_POOL *backend, PreparedStatementList *p,
					int n)
{
	char *query;
	int len;
	PrepareStmt *p_stmt;

	if (p->cnt <= n)
		return 1;

	p_stmt = (PrepareStmt *)p->portal_list[n]->stmt;
	len = strlen(p_stmt->name) + 14; /* "DEALLOCATE \"" + "\"" + '\0' */
	query = malloc(len);
	if (query == NULL)
	{
		pool_error("send_deallocate: malloc failed");
		return -1;
	}
	sprintf(query, "DEALLOCATE \"%s\"", p_stmt->name);

	if (SimpleQuery(NULL, backend, query) != POOL_CONTINUE)
	{
		free(query);
		return 1;
	}
	free(query);

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

	str = malloc(len + 1);
	if (str == NULL)
	{
		pool_error("parse_copy_data: malloc failed");
		return NULL;
	}

	/* buf is terminated by '\n'. */
	/* skip '\n' in for loop.     */
	for (i = 0, j = 0; i < len - 1; i++)
	{
		if (buf[i] == '\\' && i != len - 2) /* escape */
		{
			if (buf[i+1] == delimiter)
			{
				i++;
				str[j++] = buf[i];
			}
			else
			{
				str[j++] = buf[i];
			}
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
		p = malloc(j);
		if (p == NULL)
		{
			pool_error("parse_copy_data: malloc failed");
			return NULL;
		}
		strcpy(p, str);
		p[j] = '\0';
		pool_debug("parse_copy_data: divide key value is %s", p);
	}

	free(str);
	return p;
}

static void
query_cache_register(char kind, POOL_CONNECTION *frontend, char *database, char *data, int data_len)
{
	static int inside_T;			/* flag to see the result data sequence */
	int result;

	if (is_select_pgcatalog || is_select_for_update)
		return;

	if (kind == 'T' && parsed_query)
	{
		result = pool_query_cache_register(kind, frontend, database, data, data_len, parsed_query);
		if (result < 0)
		{
			pool_error("pool_query_cache_register: query cache registration failed");
			inside_T = 0;
		}
		else
		{
			inside_T = 1;
		}
	}
	else if ((kind == 'D' || kind == 'C' || kind == 'E') && inside_T)
	{
		result = pool_query_cache_register(kind, frontend, database, data, data_len, NULL);
		if (kind == 'C' || kind == 'E' || result < 0)
		{
			if (result < 0)
				pool_error("pool_query_cache_register: query cache registration failed");
			else
				pool_debug("pool_query_cache_register: query cache saved");

			inside_T = 0;
			free(parsed_query);
			parsed_query = NULL;
		}
	}
}

void query_ps_status(char *query, POOL_CONNECTION_POOL *backend)
{
	StartupPacket *sp;
	char psbuf[1024];
	int i;

	if (*query == '\0')
		return;

	sp = MASTER_CONNECTION(backend)->sp;
	i = snprintf(psbuf, sizeof(psbuf), "%s %s %s ",
				 sp->user, sp->database, remote_ps_data);

	/* skip spaces */
	while (*query && isspace(*query))
		query++;

	for (; i< sizeof(psbuf); i++)
	{
		if (!*query || isspace(*query))
			break;

		psbuf[i] = toupper(*query++);
	}
	psbuf[i] = '\0';

	set_ps_display(psbuf, false);
}

/* compare function for bsearch() */
static int compare(const void *p1, const void *p2)
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
		T_InsertStmt,
		T_DeleteStmt,
		T_UpdateStmt,
		T_SelectStmt,
		T_AlterTableStmt,
		T_AlterDomainStmt,
		T_GrantStmt,
		T_GrantRoleStmt,
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
		T_RemoveFuncStmt,
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
		*/
		T_CreateSeqStmt,
		T_AlterSeqStmt,
		T_VariableSetStmt,		/* SET */
		T_CreateTrigStmt,
		T_DropPropertyStmt,
		T_CreatePLangStmt,
		T_DropPLangStmt,
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
		T_DropCastStmt,
		T_CreateOpClassStmt,
		T_CreateOpFamilyStmt,
		T_AlterOpFamilyStmt,
		T_RemoveOpClassStmt,
		T_RemoveOpFamilyStmt,
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
		T_AlterTSConfigurationStmt
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

POOL_STATUS start_internal_transaction(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, Node *node)
{
	int i;

	if (TSTATE(backend) != 'I')
		return POOL_CONTINUE;

	/* if we are not in a transaction block,
	 * start a new transaction
	 */
	if (is_internal_transaction_needed(node))
	{
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i))
			{
				if (do_command(frontend, CONNECTION(backend, i), "BEGIN", MAJOR(backend), 
							   MASTER_CONNECTION(backend)->pid,	MASTER_CONNECTION(backend)->key, 0) != POOL_CONTINUE)
					return POOL_END;
			}
		}

		/* mark that we started new transaction */
		internal_transaction_started = 1;
	}
	return POOL_CONTINUE;
}


POOL_STATUS end_internal_transaction(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	int i;
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

	/* We need to commit from secondary to master. */
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
		{
			/* COMMIT success? */
			if (do_command(frontend, CONNECTION(backend, i), "COMMIT", MAJOR(backend), 
						   MASTER_CONNECTION(backend)->pid,	MASTER_CONNECTION(backend)->key, 1) != POOL_CONTINUE)
			{
				internal_transaction_started = 0;
				POOL_SETMASK(&oldmask);
				return POOL_END;
			}
		}
	}

	/* commit on master */
	if (do_command(frontend, MASTER(backend), "COMMIT", MAJOR(backend), 
				   MASTER_CONNECTION(backend)->pid,	MASTER_CONNECTION(backend)->key, 1) != POOL_CONTINUE)
	{
		internal_transaction_started = 0;
		POOL_SETMASK(&oldmask);
		return POOL_END;
	}

	internal_transaction_started = 0;
	POOL_SETMASK(&oldmask);
	return POOL_CONTINUE;
}

/*
 * Extract the number of tuples from CommandComplete message
 */
static int extract_ntuples(char *message)
{
	char *rows;

	if ((rows = strstr(message, "UPDATE")) || (rows = strstr(message, "DELETE")))
		rows +=7;
	else if ((rows = strstr(message, "INSERT")))
	{
		rows += 7;
		while (*rows && *rows != ' ') rows++;
	}
	else
		return 0;

	return atoi(rows);
}

static int detect_postmaster_down_error(POOL_CONNECTION *backend, int major)
{
	int r =  detect_error(backend, ADMIN_SHUTDOWN_ERROR_CODE, major, 'E', false);
	if (r == SPECIFIED_ERROR)
	{
		pool_debug("detect_stop_postmaster_error: receive admin shutdown error from a node.");
		return r;
	}

	r = detect_error(backend, CRASH_SHUTDOWN_ERROR_CODE, major, 'N', false);
	if (r == SPECIFIED_ERROR)
	{
		pool_debug("detect_stop_postmaster_error: receive crash shutdown error from a node.");
	}
	return r;
}

int detect_active_sql_transaction_error(POOL_CONNECTION *backend, int major)
{
	int r =  detect_error(backend, ACTIVE_SQL_TRANSACTION_ERROR_CODE, major, 'E', true);
	if (r == SPECIFIED_ERROR)
	{
		pool_debug("detect_active_sql_transaction_error: receive SET TRANSACTION ISOLATION LEVEL must be called before any query error from a node.");
	}
	return r;
}

int detect_deadlock_error(POOL_CONNECTION *backend, int major)
{
	int r =  detect_error(backend, DEADLOCK_ERROR_CODE, major, 'E', true);
	if (r == SPECIFIED_ERROR)
		pool_debug("detect_deadlock_error: received deadlock error message from backend");
	return r;
}

int detect_serialization_error(POOL_CONNECTION *backend, int major)
{
	int r =  detect_error(backend, SERIALIZATION_FAIL_ERROR_CODE, major, 'E', true);
	if (r == SPECIFIED_ERROR)
		pool_debug("detect_serialization_error: received serialization failure message from backend");
	return r;
}

int detect_query_cancel_error(POOL_CONNECTION *backend, int major)
{
	int r =  detect_error(backend, QUERY_CANCEL_ERROR_CODE, major, 'E', true);
	if (r == SPECIFIED_ERROR)
		pool_debug("detect_query_cancel_error: received query cancel error message from backend");
	return r;
}

/*
 * detect_error: Detect specified error from error code.
 */
static int detect_error(POOL_CONNECTION *backend, char *error_code, int major, char class, bool unread)
{
	int is_error = 0;
	char kind;
	int readlen = 0, len;
	static char buf[8192]; /* memory space is large enough */
	char *p, *str;

	if (pool_read(backend, &kind, sizeof(kind)))
		return POOL_END;
	readlen += sizeof(kind);
	p = buf;
	memcpy(p, &kind, sizeof(kind));
	p += sizeof(kind);

	pool_debug("detect_error: kind: %c", kind);

	/* Specified class? */
	if (kind == class)
	{
		/* read actual message */
		if (major == PROTO_MAJOR_V3)
		{
			char *e;

			if (pool_read(backend, &len, sizeof(len)) < 0)
				return POOL_END;
			readlen += sizeof(len);
			memcpy(p, &len, sizeof(len));
			p += sizeof(len);

			len = ntohl(len) - 4;
			str = malloc(len);

			if (!str)
			{
				pool_error("detect_error: malloc failed");
				return POOL_END;
			}

			pool_read(backend, str, len);
			readlen += len;

			if (readlen >= sizeof(buf))
			{
				pool_error("detect_error: not enough buffer space");
				free(str);
				return POOL_END;
			}

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
			free(str);
		}
		else
		{
			str = pool_read_string(backend, &len, 0);
			readlen += len;

			if (readlen >= sizeof(buf))
			{
				pool_error("detect_error: not enough buffer space");
				return POOL_END;
			}

			memcpy(p, str, len);
		}
	}
	if (unread || !is_error)
	{
		/* put a message to read buffer */
		if (pool_unread(backend, buf, readlen) != 0)
			is_error = -1;
	}

	return is_error;
}

/*
 * pool_error_message: Extract human readble message from ERROR/NOTICE
 * reponse packet and return it. Packet kind should have already been
 * read, and it should be either 'E' or 'N'. The returned string is
 * placed in static buffer. Message larger than the buffer will be
 * silently truncated. Be warned that next call to this function will
 * break the buffer.  If unread is true, the packet will be returned
 * to the stream.
 */
POOL_STATUS pool_extract_error_message(POOL_CONNECTION *backend, int major, bool unread, char **message)
{
	int readlen = 0, len;
	static char buf[8192]; /* unread buffer */
	static char message_buf[8192];		/* mesasge buffer */
	char *p, *str;

	p = buf;

	/* read actual message */
	if (major == PROTO_MAJOR_V3)
	{
		char *e;

		if (pool_read(backend, &len, sizeof(len)) < 0)
			return POOL_END;
		readlen += sizeof(len);
		memcpy(p, &len, sizeof(len));
		p += sizeof(len);

		len = ntohl(len) - 4;
		str = malloc(len);
		if (!str)
		{
			pool_error("pool_extract_error_message: malloc failed");
			return POOL_END;
		}

		pool_read(backend, str, len);
		readlen += len;

		if (readlen >= sizeof(buf))
		{
			pool_error("pool_extract_error_message: not enough buffer space");
			free(str);
			return POOL_END;
		}

		memcpy(p, str, len);

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
				len = Max(sizeof(message_buf)-1, strlen(e));
				memcpy(message_buf, e, len);
				message_buf[sizeof(message_buf)] = '\0';
				break;
			}
			else
				e = e + strlen(e) + 1;
		}
		free(str);
	}
	else
	{
		str = pool_read_string(backend, &len, 0);
		len = Max(sizeof(message_buf)-1, len);
		readlen += len;

		if (readlen >= sizeof(buf))
		{
			pool_error("pool_extract_error_message: not enough buffer space");
			return POOL_END;
		}

		memcpy(p, str, len);
		memcpy(message_buf, str, len);
		message_buf[sizeof(message_buf)] = '\0';
	}

	if (unread)
	{
		/* Put the message to read buffer */
		if (pool_unread(backend, buf, readlen) != 0)
			return POOL_END;
	}

	*message = message_buf;
	return POOL_CONTINUE;
}

/*
 * read message length and rest of the packet then discard it
 */
POOL_STATUS pool_discard_packet(POOL_CONNECTION_POOL *cp)
{
	int status, len, i;
	char kind;
	char *string;
	POOL_CONNECTION *backend;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
		{
			continue;
		}

		backend = CONNECTION(cp, i);

		status = pool_read(backend, &kind, sizeof(kind));
		if (status < 0)
		{
			pool_error("pool_discard_packet: error while reading message kind");
			return POOL_END;
		}

		pool_debug("pool_discard_packet: kind: %c", kind);

		if (MAJOR(cp) == PROTO_MAJOR_V3)
		{
			if (pool_read(backend, &len, sizeof(len)) < 0)
			{
				pool_error("pool_discard_packet: error while reading message length");
				return POOL_END;
			}
			len = ntohl(len) - 4;
			string = pool_read2(backend, len);
			if (string == NULL)
			{
				pool_error("pool_discard_packet: error while reading rest of message");
				return POOL_END;
			}
		}
		else
		{
			string = pool_read_string(backend, &len, 0);
			if (string == NULL)
			{
				pool_error("pool_discard_packet: error while reading rest of message");
				return POOL_END;
			}
		}
	}
	return POOL_CONTINUE;
}
