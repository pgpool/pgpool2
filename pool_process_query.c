/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2007	PgPool Global Development Group
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

#include "parser/parser.h"
#include "parser/pg_list.h"
#include "parser/parsenodes.h"
#include "pool_rewrite_query.h"

#ifndef FD_SETSIZE
#define FD_SETSIZE 512
#endif

#define INIT_STATEMENT_LIST_SIZE 8

#define DEADLOCK_ERROR_CODE "40P01"
#define ADMIN_SHUTDOWN_ERROR_CODE "57P01"
#define CRASH_SHUTDOWN_ERROR_CODE "57P02"

#define POOL_ERROR_QUERY "send invalid query from pgpool to abort transaction"

typedef struct {
	char *portal_name;
	Node *stmt;
} Portal;

/*
 * prepared statement list
 */
typedef struct {
	int size;
	int cnt;
	Portal **portal_list;
} PreparedStatementList;


static POOL_STATUS NotificationResponse(POOL_CONNECTION *frontend, 
										POOL_CONNECTION_POOL *backend);

static POOL_STATUS SimpleQuery(POOL_CONNECTION *frontend, 
						 POOL_CONNECTION_POOL *backend, char *query);

static POOL_STATUS Execute(POOL_CONNECTION *frontend, 
						   POOL_CONNECTION_POOL *backend);

static POOL_STATUS Parse(POOL_CONNECTION *frontend,
						 POOL_CONNECTION_POOL *backend);

#ifdef NOT_USED
static POOL_STATUS Sync(POOL_CONNECTION *frontend, 
						   POOL_CONNECTION_POOL *backend);
#endif

static POOL_STATUS ReadyForQuery(POOL_CONNECTION *frontend, 
								 POOL_CONNECTION_POOL *backend, int send_ready);

static POOL_STATUS CompleteCommandResponse(POOL_CONNECTION *frontend, 
										   POOL_CONNECTION_POOL *backend);

static POOL_STATUS CopyInResponse(POOL_CONNECTION *frontend, 
								  POOL_CONNECTION_POOL *backend);

static POOL_STATUS CopyOutResponse(POOL_CONNECTION *frontend, 
								   POOL_CONNECTION_POOL *backend);

static POOL_STATUS CopyDataRows(POOL_CONNECTION *frontend,
								POOL_CONNECTION_POOL *backend, int copyin);

static POOL_STATUS CursorResponse(POOL_CONNECTION *frontend, 
								  POOL_CONNECTION_POOL *backend);

static POOL_STATUS EmptyQueryResponse(POOL_CONNECTION *frontend,
									  POOL_CONNECTION_POOL *backend);

static int RowDescription(POOL_CONNECTION *frontend, 
						  POOL_CONNECTION_POOL *backend,
						  short *result);

static POOL_STATUS AsciiRow(POOL_CONNECTION *frontend, 
							POOL_CONNECTION_POOL *backend,
							short num_fields);

static POOL_STATUS BinaryRow(POOL_CONNECTION *frontend, 
							 POOL_CONNECTION_POOL *backend,
							 short num_fields);

static POOL_STATUS FunctionCall(POOL_CONNECTION *frontend, 
								POOL_CONNECTION_POOL *backend);

static POOL_STATUS FunctionResultResponse(POOL_CONNECTION *frontend, 
										  POOL_CONNECTION_POOL *backend);

static POOL_STATUS ProcessFrontendResponse(POOL_CONNECTION *frontend, 
										   POOL_CONNECTION_POOL *backend);

static POOL_STATUS send_simplequery_message(POOL_CONNECTION *backend, int len, char *string, int major);
static POOL_STATUS wait_for_query_response(POOL_CONNECTION *backend, char *string);
static POOL_STATUS send_extended_protocol_message(POOL_CONNECTION_POOL *backend,
												  int node_id, char *kind,
												  int len, char *string);
static POOL_STATUS send_execute_message(POOL_CONNECTION_POOL *backend,
										int node_id, int len, char *string);
static int synchronize(POOL_CONNECTION *cp);
static void process_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
static int reset_backend(POOL_CONNECTION_POOL *backend, int qcnt);

static int is_select_query(Node *node, char *sql);
static int is_start_transaction_query(Node *node);
static int is_commit_query(Node *node);
static int is_sequence_query(Node *node);
static int load_balance_enabled(POOL_CONNECTION_POOL *backend, Node* node, char *sql);
static void start_load_balance(POOL_CONNECTION_POOL *backend);
static void end_load_balance(POOL_CONNECTION_POOL *backend);
static POOL_STATUS do_command(POOL_CONNECTION *backend, char *query, int protoMajor, int no_ready_for_query);
static POOL_STATUS do_error_command(POOL_CONNECTION *backend, int major);
static int need_insert_lock(POOL_CONNECTION_POOL *backend, char *query, Node *node);
static POOL_STATUS insert_lock(POOL_CONNECTION_POOL *backend, char *query, InsertStmt *node);
static char *get_insert_command_table_name(InsertStmt *node);

static void add_prepared_list(PreparedStatementList *p, Portal *portal);
static void add_unnamed_portal(PreparedStatementList *p, Portal *portal);
static void del_prepared_list(PreparedStatementList *p, Portal *portal);
static void delete_all_prepared_list(PreparedStatementList *p, Portal *portal);
static void reset_prepared_list(PreparedStatementList *p);
static int send_deallocate(POOL_CONNECTION_POOL *backend, PreparedStatementList *p, int n);
static char *parse_copy_data(char *buf, int len, char delimiter, int col_id);
static Portal *lookup_prepared_statement_by_statement(PreparedStatementList *p, const char *name);
static Portal *lookup_prepared_statement_by_portal(PreparedStatementList *p, const char *name);

#ifdef NOT_USED
static POOL_CONNECTION_POOL_SLOT *slots[MAX_CONNECTION_SLOTS];
#endif

int in_load_balance;	/* non 0 if in load balance mode */
int selected_slot;		/* selected DB node */
int master_slave_dml;	/* non 0 if master/slave mode is specified in config file */
static int force_replication;
static int replication_was_enabled;		/* replication mode was enabled */
static int master_slave_was_enabled;	/* master/slave mode was enabled */
static int internal_transaction_started;		/* to issue table lock command a transaction
												   has been started internally */
static int mismatch_ntuples;
static char *copy_table = NULL;  /* copy table name */
static char *copy_schema = NULL;  /* copy table name */
static char copy_delimiter; /* copy delimiter char */
static char *copy_null = NULL; /* copy null string */
static void (*pending_function)(PreparedStatementList *p, Portal *portal) = NULL;
static Portal *pending_prepared_portal = NULL;
static Portal *unnamed_statement = NULL;
static Portal *unnamed_portal = NULL;
static int select_in_transaction = 0; /* non 0 if select query is in transaction */

static PreparedStatementList prepared_list; /* prepared statement name list */
static int is_drop_database(Node *node);		/* returns non 0 if this is a DROP DATABASE command */

static int is_cache_empty(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
static int is_strict_query(Node *node); /* returns non 0 if this is strict query */
static int check_copy_from_stdin(Node *node); /* returns non 0 if this is a COPY FROM STDIN */
static POOL_STATUS read_kind_from_one_backend(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char *kind, int node);
static POOL_STATUS read_kind_from_backend(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char *kind);
static POOL_STATUS ParallelForwardToFrontend(char kind, POOL_CONNECTION *frontend, POOL_CONNECTION *backend, char *database, bool send_to_frontend);

static void query_ps_status(char *query, POOL_CONNECTION_POOL *backend);		/* show ps status */

static int is_select_pgcatalog = 0;
static int is_select_for_update = 0; /* also for SELECT ... INTO */
static bool is_parallel_table = false;
static char *parsed_query = NULL;
static POOL_MEMORY_POOL *prepare_memory_context = NULL;
static int receive_sync = 0;

static void query_cache_register(char kind, POOL_CONNECTION *frontend, char *database, char *data, int data_len);
static POOL_STATUS start_internal_transaction(POOL_CONNECTION_POOL *backend, Node *node);
static POOL_STATUS end_internal_transaction(POOL_CONNECTION_POOL *backend);
static int extract_ntuples(char *message);
static int detect_error(POOL_CONNECTION *master, char *error_code, int major, bool unread);
static int detect_deadlock_error(POOL_CONNECTION *master, int major);
static int detect_postmaster_down_error(POOL_CONNECTION *master, int major);
static bool is_partition_table(POOL_CONNECTION_POOL *backend, Node *node);

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
		 * if all backends and frontend do not have any pending data
		 * in the receiving data cache, then issue select(2) to wait for new data arrival
		 */
		if (is_cache_empty(frontend, backend))
		{
			struct timeval timeout;
			int num_fds, was_error = 0;

		    /*
			 * frontend idle counter. depends on the following
			 * select(2) call's time out is 1 second.
			 */
			int frontend_idle_count = 0;

		SELECT_RETRY:
			if (pool_config->client_idle_limit > 0)
			{
				timeout.tv_sec = 1;
				timeout.tv_usec = 0;
			}

			FD_ZERO(&readmask);
			FD_ZERO(&writemask);
			FD_ZERO(&exceptmask);

			if (!connection_reuse)
			{
				FD_SET(frontend->fd, &readmask);
				FD_SET(frontend->fd, &exceptmask);
			}

			num_fds = 0;

			if (!VALID_BACKEND(backend->info->load_balancing_node))
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

			if (connection_reuse)
			{
				num_fds = Max(frontend->fd + 1, num_fds);
			}

			/*
			pool_debug("pool_process_query: num_fds: %d", num_fds);
			*/

			if (pool_config->client_idle_limit == 0)
				fds = select(num_fds, &readmask, &writemask, &exceptmask, NULL);
			else
				fds = select(num_fds, &readmask, &writemask, &exceptmask, &timeout);

			if (fds == -1)
			{
				if (errno == EINTR)
					continue;

				pool_error("select() failed. reason: %s", strerror(errno));
				return POOL_ERROR;
			}

			if (pool_config->client_idle_limit > 0 && fds == 0)
			{
				frontend_idle_count++;
				if (frontend_idle_count > pool_config->client_idle_limit)
				{
					pool_log("pool_process_query: child connection forced to terminate due to client_idle_limit(%d) reached", pool_config->client_idle_limit);
					return POOL_END;
				}

				goto SELECT_RETRY;
			}

			for (i = 0; i < NUM_BACKENDS; i++)
			{
				if (VALID_BACKEND(i) && FD_ISSET(CONNECTION(backend, i)->fd, &readmask))
				{
					if (detect_postmaster_down_error(CONNECTION(backend, i), MAJOR(backend)))
					{
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

			if (was_error)
				continue;

			if (!connection_reuse)
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
			if (frontend->len > 0)
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
		 * Prrocess backend Response
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
					/* Paramter Status */
					status = ParameterStatus(frontend, backend);
					break;
				case 'Z':
					/* Ready for query */
					status = ReadyForQuery(frontend, backend, 1);
					break;
				default:
					status = SimpleForwardToFrontend(kind, frontend, backend);
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


/* using only in pool_parallel_exec */
#define BITS (8 * sizeof(long int))

static void set_fd(unsigned long fd ,unsigned long *setp)
{
	unsigned long tmp = fd / FD_SETSIZE;
	unsigned long rem = fd % FD_SETSIZE;
	setp[tmp] |= (1UL<<rem);
}

/* using only in pool_parallel_exec */
static int isset_fd(unsigned long fd, unsigned long *setp)
{
	unsigned long tmp = fd / FD_SETSIZE;
	unsigned long rem = fd % FD_SETSIZE;
	return (setp[tmp] & (1UL<<rem)) != 0;
}

/* using only in pool_parallel_exec */
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

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	len = strlen(string) + 1;

	if (is_drop_database(node))
	{
		int stime = 5;  /* XXX give arbitary time to allow closing idle connections */

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
		return POOL_CONTINUE;
	}

	/* In this loop,forward the query to the backend */
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
	/* In this loop,get data from backend */
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

		/* get header of protcol */
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

				/* get body of protcol */
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

static POOL_STATUS SimpleQuery(POOL_CONNECTION *frontend, 
						 POOL_CONNECTION_POOL *backend, char *query)
{
	char *string, *string1;
	int len;
	static char *sq = "show pool_status";
	int i, commit;
	List *parse_tree_list;
	Node *node = NULL, *node1;
	POOL_STATUS status;
	int deadlock_detected = 0;

	force_replication = 0;
	if (query == NULL)	/* need to read query from frontend? */
	{
		/* read actual query */
		if (MAJOR(backend) == PROTO_MAJOR_V3)
		{
			if (pool_read(frontend, &len, sizeof(len)) < 0)
				return POOL_END;
			len = ntohl(len) - 4;
			string = pool_read2(frontend, len);
		}
		else
			string = pool_read_string(frontend, &len, 0);

		if (string == NULL)
			return POOL_END;
	}
	else
	{
		len = strlen(query)+1;
		string = query;
	}

	/* show ps status */
	query_ps_status(string, backend);

	/* log query to log file if neccessary */
	if (pool_config->log_statement)
	{
		pool_log("statement: %s", string);
	}
	else
	{
		pool_debug("statement2: %s", string);
	}

	parse_tree_list = raw_parser(string);

	if (parse_tree_list != NIL)
	{
		node = (Node *) lfirst(list_head(parse_tree_list));

		if (PARALLEL_MODE)
			is_parallel_table = is_partition_table(backend,node);

		if (pool_config->enable_query_cache &&
			SYSDB_STATUS == CON_UP &&
			IsA(node, SelectStmt) &&
			!(is_select_pgcatalog = IsSelectpgcatalog(node, backend)))
		{
			SelectStmt *select = (SelectStmt *)node;

			if (! (select->intoClause || select->lockingClause))
			{
				parsed_query = strdup(nodeToString(node));
				if (parsed_query == NULL)
				{
					pool_error("pool_process_query: malloc failed");
					return POOL_ERROR;
				}

				if (parsed_query)
				{
					if (pool_query_cache_lookup(frontend, parsed_query, backend->info->database, TSTATE(backend)) == POOL_CONTINUE)
					{
						free(parsed_query);
						parsed_query = NULL;
						free_parser();
						return POOL_CONTINUE;
					}
				}
				is_select_for_update = 0;
			}
			else
			{
				is_select_for_update = 1;
			}
		}

		if (pool_config->parallel_mode)
		{
			/* analze query */
			RewriteQuery *r_query = is_parallel_query(node,backend);

			if(r_query->is_loadbalance)
			{
				if(r_query->r_code ==  SEND_LOADBALANCE_ENGINE)
				{
					/* use rewrited query */
					string = r_query->rewrite_query;
					/* change query length */
					len = strlen(string)+1;
				}
				pool_debug("SimpleQuery: loadbalance_query =%s",string);
			}
			else if (r_query->is_parallel)
			{
				/* call parallel exe engine */
				POOL_STATUS stats = pool_parallel_exec(frontend,backend,r_query->rewrite_query, node,true);
				free_parser();
				return stats;
			}
			else if(!r_query->is_pg_catalog)
			{
				r_query = rewrite_query_stmt(node,frontend,backend,r_query);
				/* rewrite query phase */
				if(r_query->type == T_InsertStmt)
				{
					free_parser();

					if(r_query->r_code != INSERT_DIST_NO_RULE)
						return r_query->status;
				}
				else if(r_query->type == T_SelectStmt)
				{
					free_parser();
					return r_query->status;
				}
 			}
		}

		/* check COPY FROM STDIN
		 * if true, set copy_* variable
		 */
		check_copy_from_stdin(node);

		/*
		 * if this is DROP DATABASE command, send USR1 signal to parent and
		 * ask it to close all idle connections.
		 * XXX This is overkill. It would be better to close the idle
		 * connection for the database which DROP DATABASE command tries
		 * to drop. This is impossible at this point, since we have no way
		 * to pass such info to other processes.
		 */
		if (is_drop_database(node))
		{
			int stime = 5;	/* XXX give arbitary time to allow closing idle connections */

			pool_debug("Query: sending SIGUSR1 signal to parent");

			Req_info->kind = CLOSE_IDLE_REQUEST;
			kill(getppid(), SIGUSR1);		/* send USR1 signal to parent */

			/* we need to loop over here since we will get USR1 signal while sleeping */
			while (stime > 0)
			{
				stime = sleep(stime);
			}
		}

		/* process status reporting? */
		if (IsA(node, VariableShowStmt) && strncasecmp(sq, string, strlen(sq)) == 0)
		{
			StartupPacket *sp;
			char psbuf[1024];

			pool_debug("process reporting");
			process_reporting(frontend, backend);

			/* show ps status */
			sp = MASTER_CONNECTION(backend)->sp;
			snprintf(psbuf, sizeof(psbuf), "%s %s %s idle",
					 sp->user, sp->database, remote_ps_data);
			set_ps_display(psbuf, false);

			free_parser();
			return POOL_CONTINUE;
		}

		if (IsA(node, PrepareStmt) || IsA(node, DeallocateStmt) || IsA(node, VariableSetStmt))
		{
			if (MASTER_SLAVE)
				force_replication = 1;

			if (frontend)
			{
				POOL_MEMORY_POOL *old_context;
				Portal *portal;

				if (prepare_memory_context == NULL)
				{
					prepare_memory_context = pool_memory_create();
					if (prepare_memory_context == NULL)
					{
						pool_error("Simple Query: pool_memory_create() failed");
						return POOL_ERROR;
					}
				}
				/* switch memory context */
				old_context = pool_memory;
				pool_memory = prepare_memory_context;

				if (IsA(node, PrepareStmt))
				{
					pending_function = add_prepared_list;
					portal = malloc(sizeof(Portal));
					portal->portal_name = NULL;
					portal->stmt = copyObject(node);
					pending_prepared_portal = portal;
				}
				else if (IsA(node, DeallocateStmt))
				{
					pending_function = del_prepared_list;
					portal = malloc(sizeof(Portal));
					portal->portal_name = NULL;
					portal->stmt = copyObject(node);
					pending_prepared_portal = portal;
				}
				else if (IsA(node, DiscardStmt))
				{
					DiscardStmt *stmt = (DiscardStmt *)node;
					if (stmt->target == DISCARD_ALL || stmt->target == DISCARD_PLANS)
					{
						pending_function = delete_all_prepared_list;
						pending_prepared_portal = NULL;
					}
				}

				/* switch old memory context */
				pool_memory = old_context;
			}
		}

		if (frontend && IsA(node, ExecuteStmt))
		{
			Portal *portal;
			PrepareStmt *p_stmt;
			ExecuteStmt *e_stmt = (ExecuteStmt *)node;

			portal = lookup_prepared_statement_by_statement(&prepared_list,
															e_stmt->name);
			if (!portal)
			{
				string1 = string;
				node1 = node;
			}
			else
			{
				p_stmt = (PrepareStmt *)portal->stmt;
				string1 = nodeToString(p_stmt->query);
				node1 = (Node *)p_stmt->query;
			}
		}
		else
		{
			string1 = string;
			node1 = node;
		}

		/* load balance trick */
		if (load_balance_enabled(backend, node1, string1))
			start_load_balance(backend);
		else if (MASTER_SLAVE)
		{
			pool_debug("SimpleQuery: set master_slave_dml query: %s", string);
			master_slave_was_enabled = 1;
			MASTER_SLAVE = 0;
			master_slave_dml = 1;
			if (force_replication)
			{
				replication_was_enabled = 0;
				REPLICATION = 1;
			}
		}
		else if (REPLICATION &&
				 !pool_config->replicate_select &&
				 is_select_query(node1, string1) &&
				 !is_sequence_query(node1))
		{
			selected_slot = MASTER_NODE_ID;
			replication_was_enabled = 1;
			REPLICATION = 0;
			LOAD_BALANCE_STATUS(MASTER_NODE_ID) = LOAD_SELECTED;
			in_load_balance = 1;
			select_in_transaction = 1;
		}


		/*
		 * determine if we need to lock the table
		 * to keep SERIAL data consistency among servers
		 * conditions:
		 * - replication is enabled
		 * - protocol is V3
		 * - statement is INSERT
		 * - either "INSERT LOCK" comment exists or insert_lock directive specified
		 */
		if (REPLICATION && need_insert_lock(backend, string, node))
		{
			/* start a transaction if needed and lock the table */
			status = insert_lock(backend, string, (InsertStmt *)node);
			if (status != POOL_CONTINUE)
			{
				
				free_parser();
				return status;
			}
		}
		else if (REPLICATION && query == NULL && start_internal_transaction(backend, node))
		{
			free_parser();
			return POOL_ERROR;
		}
	}
	else
	{  /* syntax error */
		if (MASTER_SLAVE)
		{
			pool_debug("SimpleQuery: set master_slave_dml query: %s", string);
			master_slave_was_enabled = 1;
			MASTER_SLAVE = 0;
			master_slave_dml = 1;
		}		
	}

	if (MAJOR(backend) == PROTO_MAJOR_V2 && is_start_transaction_query(node))
	{
		TSTATE(backend) = 'T';
	}

	if (REPLICATION || PARALLEL_MODE)
	{
		/* check if query is "COMMIT" */
		commit = is_commit_query(node);
		free_parser();

		/* send query to master node */
		if (!commit)
		{
			if (send_simplequery_message(MASTER(backend), len, string, MAJOR(backend)) != POOL_CONTINUE)
				return POOL_END;

			if (wait_for_query_response(MASTER(backend), string) != POOL_CONTINUE)
				return POOL_END;

			/*
			 * We must check deadlock error because a aborted transaction
			 * by detecting deadlock isn't same on all nodes.
			 * If a transaction is aborted on master node, pgpool send a
			 * error query to another nodes.
			 */
			deadlock_detected = detect_deadlock_error(MASTER(backend), MAJOR(backend));
			if (deadlock_detected < 0)
				return POOL_END;
		}
		if (deadlock_detected)
		{
			string = POOL_ERROR_QUERY;
			len = strlen(string) + 1;
		}

		/* send query to other nodes */
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i) || IS_MASTER_NODE_ID(i))
				continue;

			if (send_simplequery_message(CONNECTION(backend, i), len, string, MAJOR(backend)) != POOL_CONTINUE)
				return POOL_END;
		}

		/* wait for response excepted for MASTER node */
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i) || IS_MASTER_NODE_ID(i))
				continue;

			if (wait_for_query_response(CONNECTION(backend, i), string) != POOL_CONTINUE)
				return POOL_END;
		}

		if (commit)
		{
			if (send_simplequery_message(MASTER(backend), len, string, MAJOR(backend)) != POOL_CONTINUE)
				return POOL_END;

			if (wait_for_query_response(MASTER(backend), string) != POOL_CONTINUE)
				return POOL_END;

			TSTATE(backend) = 'I';
		}
	}
	else
	{
		free_parser();
		if (send_simplequery_message(MASTER(backend), len, string, MAJOR(backend)) != POOL_CONTINUE)
			return POOL_END;

		if (wait_for_query_response(MASTER(backend), string) != POOL_CONTINUE)
			return POOL_END;
	}

	return POOL_CONTINUE;
}

/* 
 * send SimpleQuery message to single node.
 */
static POOL_STATUS send_simplequery_message(POOL_CONNECTION *backend, int len, char *string, int major)
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
 * wait for query response from single node.
 */
static POOL_STATUS wait_for_query_response(POOL_CONNECTION *backend, char *string)
{
	/*
	 * we need to wait for backend completing the query.
	 */
	pool_debug("waiting for backend %d completing the query", backend->db_node_id);
	if (synchronize(backend))
		return POOL_END;

	return POOL_CONTINUE;
}

/*
 * process EXECUTE (V3 only)
 */
static POOL_STATUS Execute(POOL_CONNECTION *frontend, 
						   POOL_CONNECTION_POOL *backend)
{
	char *string;		/* portal name + null terminate + max_tobe_returned_rows */
	int len;
	int i;
	char kind;
	int status, commit = 0;
	Portal *portal;
	char *string1;
	PrepareStmt *p_stmt;
	int deadlock_detected = 0;
	POOL_STATUS ret;

	/* read Execute packet */
	if (pool_read(frontend, &len, sizeof(len)) < 0)
		return POOL_END;

	len = ntohl(len) - 4;
	string = pool_read2(frontend, len);

	pool_debug("Execute: portal name <%s>", string);


	portal = lookup_prepared_statement_by_portal(&prepared_list,
												 string);

	/* load balance trick */
	if (portal)
	{
		POOL_MEMORY_POOL *pool_mem;
		Node *node;

		p_stmt = (PrepareStmt *)portal->stmt;

		pool_mem = pool_memory_create();
		string1 = nodeToString(p_stmt->query);
		node = (Node *)p_stmt->query;

		if ((IsA(node, PrepareStmt) || IsA(node, DeallocateStmt) || IsA(node, VariableSetStmt)) &&
			MASTER_SLAVE)
		{
			force_replication = 1;
		}

		if (load_balance_enabled(backend, node, string1))
			start_load_balance(backend);
		else if (REPLICATION &&
				 !pool_config->replicate_select &&
				 is_select_query((Node *)p_stmt->query, string1) &&
				 !is_sequence_query((Node *)p_stmt->query))
		{
			selected_slot = MASTER_NODE_ID;
			replication_was_enabled = 1;
			REPLICATION = 0;
			LOAD_BALANCE_STATUS(MASTER_NODE_ID) = LOAD_SELECTED;
			in_load_balance = 1;
			select_in_transaction = 1;
		}
/*
		else if (REPLICATION && start_internal_transaction(backend, (Node *)p_stmt->query))
		{
			return POOL_END;
		}
*/

		commit = is_commit_query((Node *)p_stmt->query);
		pool_memory_delete(pool_mem, 0);
	}
	else if (MASTER_SLAVE)
	{
		master_slave_was_enabled = 1;
		MASTER_SLAVE = 0;
		master_slave_dml = 1;
		if (force_replication)
		{
			replication_was_enabled = 0;
			REPLICATION = 1;
		}
	}

	if (REPLICATION || PARALLEL_MODE)
	{
		/* send query to master node */
		if (!commit)
		{
			if (send_execute_message(backend, MASTER_NODE_ID, len, string) != POOL_CONTINUE)
				return POOL_END;

			pool_debug("waiting for backend completing the query");
			if (synchronize(CONNECTION(backend, MASTER_NODE_ID)))
				return POOL_END;

			/*
			 * We must check deadlock error because a aborted transaction
			 * by detecting deadlock isn't same on all nodes.
			 * If a transaction is aborted on master node, pgpool send a
			 * error query to another nodes.
			 */
			deadlock_detected = detect_deadlock_error(MASTER(backend), MAJOR(backend));
			if (deadlock_detected < 0)
				return POOL_END;
		}

		/* send query to other nodes */
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i) || IS_MASTER_NODE_ID(i))
				continue;
			if (deadlock_detected)
			{
				if (send_simplequery_message(CONNECTION(backend, i),
											 strlen(POOL_ERROR_QUERY)+1,
											 POOL_ERROR_QUERY,
											 MAJOR(backend)))
					return POOL_END;
			}
			else if (send_execute_message(backend, i, len, string) != POOL_CONTINUE)
				return POOL_END;
		}

		/* wait for nodes excepted for master node */
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i) || IS_MASTER_NODE_ID(i))
				continue;

			pool_debug("waiting for backend completing the query");
			if (synchronize(CONNECTION(backend, i)))
				return POOL_END;
		}

		if (commit)
		{
			if (send_execute_message(backend, MASTER_NODE_ID, len, string) != POOL_CONTINUE)
				return POOL_END;

			pool_debug("waiting for backend completing the query");
			if (synchronize(MASTER(backend)))
				return POOL_END;
		}
	}
	else
	{
		if (send_execute_message(backend, MASTER_NODE_ID, len, string) != POOL_CONTINUE)
			return POOL_END;

		pool_debug("waiting for backend completing the query");
		if (synchronize(CONNECTION(backend, MASTER_NODE_ID)))
			return POOL_END;
	}

	while ((ret = read_kind_from_backend(frontend, backend, &kind)) == POOL_CONTINUE)
	{
		/*
		 * forward message until receiving CommandComplete,
		 * ErrorResponse, EmptyQueryResponse or PortalSuspend.
		 */
		if (kind == 'C' || kind == 'E' || kind == 'I' || kind == 's')
			break;

		status = SimpleForwardToFrontend(kind, frontend, backend);
		if (status != POOL_CONTINUE)
			return status;
		if (pool_flush(frontend))
			return POOL_END;
	}
	if (ret != POOL_CONTINUE)
		return ret;

	status = SimpleForwardToFrontend(kind, frontend, backend);
	if (status != POOL_CONTINUE)
		return status;
	if (pool_flush(frontend))
		return POOL_END;

	/* end load balance mode */
	if (in_load_balance)
		end_load_balance(backend);

	if (master_slave_dml)
	{
		MASTER_SLAVE = 1;
		master_slave_was_enabled = 0;
		master_slave_dml = 0;
		if (force_replication)
		{
			REPLICATION = 0;
			replication_was_enabled = 0;
			force_replication = 0;
		}
	}

	return POOL_CONTINUE;
}


/*
 * Extended query protocol has to send Flush message.
 */
static POOL_STATUS send_extended_protocol_message(POOL_CONNECTION_POOL *backend,
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

static POOL_STATUS send_execute_message(POOL_CONNECTION_POOL *backend,
										int node_id, int len, char *string)
{
	return send_extended_protocol_message(backend, node_id, "E", len, string);
}


/*
 * process Parse (V3 only)
 */
static POOL_STATUS Parse(POOL_CONNECTION *frontend, 
						 POOL_CONNECTION_POOL *backend)
{
	char kind;
	int len;
	char *string;
	int i;
	Portal *portal;
	POOL_MEMORY_POOL *old_context;
	PrepareStmt *p_stmt;
	char *name, *stmt;
	List *parse_tree_list;
	Node *node = NULL;
	int deadlock_detected = 0;
	int insert_stmt_with_lock = 0;
	POOL_STATUS status;

	/* read Parse packet */
	if (pool_read(frontend, &len, sizeof(len)) < 0)
		return POOL_END;

	len = ntohl(len) - 4;
	string = pool_read2(frontend, len);

	pool_debug("Parse: portal name <%s>", string);

	name = string;
	stmt = string + strlen(string) + 1;

	parse_tree_list = raw_parser(stmt);
	if (parse_tree_list == NIL)
	{
		free_parser();
	}
	else
	{
		node = (Node *) lfirst(list_head(parse_tree_list));

		insert_stmt_with_lock = need_insert_lock(backend, stmt, node);

		if (prepare_memory_context == NULL)
		{
			prepare_memory_context = pool_memory_create();
			if (prepare_memory_context == NULL)
			{
				pool_error("Simple Query: pool_memory_create() failed");
				return POOL_ERROR;
			}
		}
		/* switch memory context */
		old_context = pool_memory;
		pool_memory = prepare_memory_context;

		portal = malloc(sizeof(Portal));
		/* translate Parse message to PrepareStmt */
		p_stmt = palloc(sizeof(PrepareStmt));
		p_stmt->type = T_PrepareStmt;
		p_stmt->name = pstrdup(name);
		p_stmt->query = copyObject(node);
		portal->stmt = (Node *)p_stmt;
		portal->portal_name = NULL;

		if (*name)
		{
			pending_function = add_prepared_list;
			pending_prepared_portal = portal;
		}
		else /* unnamed statement */
		{
			pending_function = add_unnamed_portal;
			pfree(p_stmt->name);
			p_stmt->name = NULL;
			pending_prepared_portal = portal;
		}

		/* switch old memory context */
		pool_memory = old_context;

		if (REPLICATION)
		{
			char kind;

			if (TSTATE(backend) != 'T')
			{
				/* synchronize transaction state */
				for (i = 0; i < NUM_BACKENDS; i++)
				{
					if (!VALID_BACKEND(i))
						continue;

					send_extended_protocol_message(backend, i, "S", 0, "");
				}

				kind = pool_read_kind(backend);
				if (kind != 'Z')
					return POOL_END;
				if (ReadyForQuery(frontend, backend, 0) != POOL_CONTINUE)
					return POOL_END;
			}

			if (is_strict_query(node))
				start_internal_transaction(backend, node);

			if (insert_stmt_with_lock)
			{
				/* start a transaction if needed and lock the table */
				status = insert_lock(backend, stmt, (InsertStmt *)node);
				if (status != POOL_CONTINUE)
				{
					return status;
				}
			}
		}
		free_parser();
	}

	/* send to master node */
	if (send_extended_protocol_message(backend, MASTER_NODE_ID,
									   "P", len, string))
		return POOL_END;

	if (REPLICATION || PARALLEL_MODE || MASTER_SLAVE)
	{
		/* We must synchronize because Parse message acquires table
		 * locks.
		 */
		pool_debug("waiting for master completing the query");
		if (synchronize(MASTER(backend)))
			return POOL_END;

		/*
		 * We must check deadlock error because a aborted transaction
		 * by detecting deadlock isn't same on all nodes.
		 * If a transaction is aborted on master node, pgpool send a
		 * error query to another nodes.
		 */
		deadlock_detected = detect_deadlock_error(MASTER(backend), MAJOR(backend));
		if (deadlock_detected < 0)
			return POOL_END;

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
			{
				if (deadlock_detected)
				{
					if (send_simplequery_message(CONNECTION(backend, i),
												 strlen(POOL_ERROR_QUERY)+1,
												 POOL_ERROR_QUERY,
												 MAJOR(backend)))
						return POOL_END;
				}
				else if (send_extended_protocol_message(backend, i,
														"P", len, string))
					return POOL_END;
			}
		}

		/* wait for nodes excepted for master node */
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i) || IS_MASTER_NODE_ID(i))
				continue;

			pool_debug("waiting for backend completing the query");
			if (synchronize(CONNECTION(backend, i)))
				return POOL_END;
		}
	}

	for (;;)
	{
		POOL_STATUS ret;
		ret = read_kind_from_backend(frontend, backend, &kind);

		if (ret != POOL_CONTINUE)
			return ret;
			
		SimpleForwardToFrontend(kind, frontend, backend);
		if (pool_flush(frontend) < 0)
			return POOL_ERROR;

		if (kind != 'N')
			break;
	}
	return POOL_CONTINUE;
}

#ifdef NOT_USED
/*
 * process Sync (V3 only)
 */
static POOL_STATUS Sync(POOL_CONNECTION *frontend, 
						   POOL_CONNECTION_POOL *backend)
{
	char *string;		/* portal name + null terminate + max_tobe_returned_rows */
	int len;
	int sendlen;

	/* read Sync packet */
	if (pool_read(frontend, &len, sizeof(len)) < 0)
		return POOL_END;

	len = ntohl(len) - 4;
	string = pool_read2(frontend, len);

	/* forward the query to the backend */
	pool_write(MASTER(backend), "S", 1);

	sendlen = htonl(len + 4);
	pool_write(MASTER(backend), &sendlen, sizeof(sendlen));
	if (pool_write_and_flush(MASTER(backend), string, len) < 0)
	{
		return POOL_END;
	}

	if (REPLICATION)
	{
		/*
		 * we need to wait for master completing the query.
		 */
		pool_debug("waiting for master completing the query");
		if (synchronize(MASTER(backend)))
			return POOL_END;

		pool_write(SECONDARY(backend), "S", 1);
		sendlen = htonl(len + 4);
		pool_write(SECONDARY(backend), &sendlen, sizeof(sendlen));
		if (pool_write_and_flush(SECONDARY(backend), string, len) < 0)
		{
			return POOL_END;
		}

		/* we need to wait for secondary completing the query */
		if (synchronize(SECONDARY(backend)))
			return POOL_END;
	}
	return POOL_CONTINUE;
}
#endif

static POOL_STATUS ReadyForQuery(POOL_CONNECTION *frontend, 
								 POOL_CONNECTION_POOL *backend, int send_ready)
{
	StartupPacket *sp;
	char psbuf[1024];
	int i;
	int len;
	signed char state;

	if (mismatch_ntuples)
	{
		int len, i;
		signed char state;
		char kind;

		/* If the numbers of update tuples are difference, we need to abort transaction. */
		if (MAJOR(backend) == PROTO_MAJOR_V3)
		{
			if ((len = pool_read_message_length(backend)) < 0)
				return POOL_END;

			pool_debug("ReadyForQuery: message length: %d", len);

			len = htonl(len);

			state = pool_read_kind(backend);
			if (state < 0)
				return POOL_END;

			/* set transaction state */
			pool_debug("ReadyForQuery: transaction state: %c", state);
		}

		for (i = 0; i < NUM_BACKENDS; i++)
		{
			if (VALID_BACKEND(i))
			{
				/* abort transaction on all nodes. */
				do_error_command(CONNECTION(backend, i), PROTO_MAJOR_V3);
			}
		}
		kind = pool_read_kind(backend);
		if (kind != 'Z') /* ReadyForQuery? */
		{
			return POOL_END;
		}

		mismatch_ntuples = 0;
	}

	/* if a transaction is started for insert lock, we need to close it. */
	if (internal_transaction_started && receive_sync == 0)
	{
		int len;
		signed char state;

		if (MAJOR(backend) == PROTO_MAJOR_V3)
		{
			if ((len = pool_read_message_length(backend)) < 0)
				return POOL_END;

			pool_debug("ReadyForQuery: message length: %d", len);

			len = htonl(len);

			state = pool_read_kind(backend);
			if (state < 0)
				return POOL_END;

			/* set transaction state */
			pool_debug("ReadyForQuery: transaction state: %c", state);
		}

		if (end_internal_transaction(backend) != POOL_CONTINUE)
			return POOL_ERROR;
	}

	receive_sync = 0;

	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		if ((len = pool_read_message_length(backend)) < 0)
			return POOL_END;

		pool_debug("ReadyForQuery: message length: %d", len);

		state = pool_read_kind(backend);
		if (state < 0)
			return POOL_END;

		/* set transaction state */
		pool_debug("ReadyForQuery: transaction state: %c", state);

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i))
				continue;

			CONNECTION(backend, i)->tstate = state;
		}
	}

	if (send_ready)
	{
		pool_flush(frontend);

		pool_write(frontend, "Z", 1);

		if (MAJOR(backend) == PROTO_MAJOR_V3)
		{
			len = htonl(len);
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, &state, 1);
		}

		if (pool_flush(frontend))
			return POOL_END;
	}

	/* end load balance mode */
	if (in_load_balance)
		end_load_balance(backend);

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

#ifdef NOT_USED
	return ProcessFrontendResponse(frontend, backend);
#endif

	sp = MASTER_CONNECTION(backend)->sp;
	if (MASTER(backend)->tstate == 'T')
		snprintf(psbuf, sizeof(psbuf), "%s %s %s idle in transaction", 
				 sp->user, sp->database, remote_ps_data);
	else
		snprintf(psbuf, sizeof(psbuf), "%s %s %s idle", 
				 sp->user, sp->database, remote_ps_data);
	set_ps_display(psbuf, false);

	return POOL_CONTINUE;
}

static POOL_STATUS CompleteCommandResponse(POOL_CONNECTION *frontend, 
										   POOL_CONNECTION_POOL *backend)
{
	int i;
	char *string = NULL;
	char *string1 = NULL;
	int len, len1 = 0;

	/* read command tag */
	string = pool_read_string(MASTER(backend), &len, 0);
	if (string == NULL)
		return POOL_END;
	len1 = len;
	string1 = strdup(string);

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i) || IS_MASTER_NODE_ID(i))
			continue;

		/* read command tag */
		string = pool_read_string(CONNECTION(backend, i), &len, 0);
		if (string == NULL)
			return POOL_END;

		if (len != len1)
		{
			pool_debug("Complete Command Response: message length does not match between master(%d \"%s\",) and %d th server (%d \"%s\",)",
					   len, string, len1, string1);
			
			free(string1);
			return POOL_END;
		}
	}
	/* forward to the frontend */
	pool_write(frontend, "C", 1);
	pool_debug("Complete Command Response: string: \"%s\"", string1);
	if (pool_write(frontend, string1, len1) < 0)
	{
		free(string1);
		return POOL_END;
	}

	free(string1);
	return pool_flush(frontend);
}

static int RowDescription(POOL_CONNECTION *frontend, 
						  POOL_CONNECTION_POOL *backend,
						  short *result)
{
	short num_fields, num_fields1 = 0;
	int oid, mod;
	int oid1, mod1;
	short size, size1;
	char *string;
	int len, len1;
	int i;

	pool_read(MASTER(backend), &num_fields, sizeof(short));
	num_fields1 = num_fields;
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
		{
			/* # of fields (could be 0) */
			pool_read(CONNECTION(backend, i), &num_fields, sizeof(short));
			if (num_fields != num_fields1)
			{
				pool_error("RowDescription: num_fields deos not match between backends master(%d) and %d th backend(%d)",
						   num_fields, i, num_fields1);
				return POOL_FATAL;
			}
		}
	}

	/* forward it to the frontend */
	pool_write(frontend, "T", 1);
	pool_write(frontend, &num_fields, sizeof(short));
	num_fields = ntohs(num_fields);
	for (i = 0;i<num_fields;i++)
	{
		int j;

		/* field name */
		string = pool_read_string(MASTER(backend), &len, 0);
		if (string == NULL)
			return POOL_END;
		len1 = len;
		if (pool_write(frontend, string, len) < 0)
			return POOL_END;

		for (j=0;j<NUM_BACKENDS;j++)
		{
			if (VALID_BACKEND(j) && !IS_MASTER_NODE_ID(j))
			{
				string = pool_read_string(CONNECTION(backend, j), &len, 0);
				if (string == NULL)
					return POOL_END;

				if (len != len1)
				{
					pool_error("RowDescription: field length deos not match between backends master(%d) and %d th backend(%d)",
							   ntohl(len), ntohl(len1));
					return POOL_FATAL;
				}
			}
		}

		/* type oid */
		pool_read(MASTER(backend), &oid, sizeof(int));
		oid1 = oid;
		pool_debug("RowDescription: type oid: %d", ntohl(oid));
		for (j=0;j<NUM_BACKENDS;j++)
		{
			if (VALID_BACKEND(j) && !IS_MASTER_NODE_ID(j))
			{
				pool_read(CONNECTION(backend, j), &oid, sizeof(int));

				/* we do not regard oid mismatch as fatal */
				if (oid != oid1)
				{
					pool_debug("RowDescription: field oid deos not match between backends master(%d) and %d th backend(%d)",
							   ntohl(oid), j, ntohl(oid1));
				}
			}
		}
		if (pool_write(frontend, &oid1, sizeof(int)) < 0)
			return POOL_END;

		/* size */
		pool_read(MASTER(backend), &size, sizeof(short));
		size1 = size;
		for (j=0;j<NUM_BACKENDS;j++)
		{
			if (VALID_BACKEND(j) && !IS_MASTER_NODE_ID(j))
			{
				pool_read(CONNECTION(backend, j), &size, sizeof(short));
				if (size1 != size1)
				{
					pool_error("RowDescription: field size deos not match between backends master(%d) and %d th backend(%d)",
							   ntohs(size), j, ntohs(size1));
					return POOL_FATAL;
				}
			}
		}
		pool_debug("RowDescription: field size: %d", ntohs(size));
		pool_write(frontend, &size1, sizeof(short));

		/* modifier */
		pool_read(MASTER(backend), &mod, sizeof(int));
		pool_debug("RowDescription: modifier: %d", ntohs(mod));
		mod1 = mod;
		for (j=0;j<NUM_BACKENDS;j++)
		{
			if (VALID_BACKEND(j) && !IS_MASTER_NODE_ID(j))
			{
				pool_read(CONNECTION(backend, j), &mod, sizeof(int));
				if (mod != mod1)
				{
					pool_debug("RowDescription: modifier deos not match between backends master(%d) and %d th backend(%d)",
							   ntohl(mod), j, ntohl(mod1));
				}
			}
		}
		if (pool_write(frontend, &mod1, sizeof(int)) < 0)
			return POOL_END;
	}

	*result = num_fields;

	return pool_flush(frontend);
}

static POOL_STATUS AsciiRow(POOL_CONNECTION *frontend, 
							POOL_CONNECTION_POOL *backend,
							short num_fields)
{
	static char nullmap[8192], nullmap1[8192];
	int nbytes;
	int i, j;
	unsigned char mask;
	int size, size1 = 0;
	char *buf = NULL;
	char msgbuf[1024];

	pool_write(frontend, "D", 1);

	nbytes = (num_fields + 7)/8;

	if (nbytes <= 0)
		return POOL_CONTINUE;

	/* NULL map */
	pool_read(MASTER(backend), nullmap, nbytes);
	memcpy(nullmap1, nullmap, nbytes);
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
		{
			pool_read(CONNECTION(backend, i), nullmap, nbytes);
			if (memcmp(nullmap, nullmap1, nbytes))
			{
				/* XXX: NULLMAP maybe different among
				   backends. If we were a paranoid, we have to treat
				   this as a fatal error. However in the real world
				   we'd better to adapt this situation. Just throw a
				   log... */
				pool_debug("AsciiRow: NULLMAP differ between master and %d th backend", i);
			}
		}
	}

	if (pool_write(frontend, nullmap1, nbytes) < 0)
		return POOL_END;

	mask = 0;

	for (i = 0;i<num_fields;i++)
	{
		if (mask == 0)
			mask = 0x80;

		/* NOT NULL? */
		if (mask & nullmap[i/8])
		{
			/* field size */
			if (pool_read(MASTER(backend), &size, sizeof(int)) < 0)
				return POOL_END;
			/* forward to frontend */
			pool_write(frontend, &size, sizeof(int));
			size1 = size;
			for (j=0;j<NUM_BACKENDS;j++)
			{
				if (VALID_BACKEND(j) && !IS_MASTER_NODE_ID(j))
				{
					/* field size */
					if (pool_read(CONNECTION(backend, j), &size, sizeof(int)) < 0)
						return POOL_END;
				}
				/* XXX: field size maybe different among
				   backends. If we were a paranoid, we have to treat
				   this as a fatal error. However in the real world
				   we'd better to adapt this situation. Just throw a
				   log... */
				if (size != size1)
					pool_debug("AsciiRow: %d th field size does not match between master(%d) and %d th backend(%d)",
							   i, ntohl(size), j, ntohl(size1));

				buf = NULL;
				size = ntohl(size) - 4;

				/* read and send actual data only when size > 0 */
				if (size > 0)
				{
					buf = pool_read2(CONNECTION(backend, j), size);
					if (buf == NULL)
						return POOL_END;

					if (IS_MASTER_NODE_ID(j))
					{
						pool_write(frontend, buf, size);
						snprintf(msgbuf, Min(sizeof(msgbuf), size+1), "%s", buf);
						pool_debug("AsciiRow: len: %d data: %s", size, msgbuf);
					}
				}
			}
		}

		mask >>= 1;
	}

	if (pool_flush(frontend))
		return POOL_END;

	return POOL_CONTINUE;
}

static POOL_STATUS BinaryRow(POOL_CONNECTION *frontend, 
							 POOL_CONNECTION_POOL *backend,
							 short num_fields)
{
	static char nullmap[8192], nullmap1[8192];
	int nbytes;
	int i, j;
	unsigned char mask;
	int size, size1 = 0;
	char *buf = NULL;

	pool_write(frontend, "B", 1);

	nbytes = (num_fields + 7)/8;

	if (nbytes <= 0)
		return POOL_CONTINUE;

	/* NULL map */
	pool_read(MASTER(backend), nullmap, nbytes);
	if (pool_write(frontend, nullmap, nbytes) < 0)
		return POOL_END;
	memcpy(nullmap1, nullmap, nbytes);
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
		{
			pool_read(CONNECTION(backend, i), nullmap, nbytes);
			if (memcmp(nullmap, nullmap1, nbytes))
			{
				/* XXX: NULLMAP maybe different among
				   backends. If we were a paranoid, we have to treat
				   this as a fatal error. However in the real world
				   we'd better to adapt this situation. Just throw a
				   log... */
				pool_debug("BinaryRow: NULLMAP differ between master and %d th backend", i);
			}
		}
	}

	mask = 0;

	for (i = 0;i<num_fields;i++)
	{
		if (mask == 0)
			mask = 0x80;

		/* NOT NULL? */
		if (mask & nullmap[i/8])
		{
			/* field size */
			if (pool_read(MASTER(backend), &size, sizeof(int)) < 0)
				return POOL_END;			
			for (j=0;j<NUM_BACKENDS;j++)
			{
				if (VALID_BACKEND(j) && !IS_MASTER_NODE_ID(j))
				{
					/* field size */
					if (pool_read(CONNECTION(backend, i), &size, sizeof(int)) < 0)
						return POOL_END;			

					/* XXX: field size maybe different among
					   backends. If we were a paranoid, we have to treat
					   this as a fatal error. However in the real world
					   we'd better to adapt this situation. Just throw a
					   log... */
					if (size != size1)
						pool_debug("BinaryRow: %d th field size does not match between master(%d) and %d th backend(%d)",
								   i, ntohl(size), j, ntohl(size1));
				}

				buf = NULL;

				/* forward to frontend */
				if (IS_MASTER_NODE_ID(j))
					pool_write(frontend, &size, sizeof(int));
				size = ntohl(size) - 4;

				/* read and send actual data only when size > 0 */
				if (size > 0)
				{
					buf = pool_read2(CONNECTION(backend, j), size);
					if (buf == NULL)
						return POOL_END;

					if (IS_MASTER_NODE_ID(j))
					{
						pool_write(frontend, buf, size);
					}
				}
			}

			mask >>= 1;
		}
	}

	if (pool_flush(frontend))
		return POOL_END;

	return POOL_CONTINUE;
}

static POOL_STATUS CursorResponse(POOL_CONNECTION *frontend, 
								  POOL_CONNECTION_POOL *backend)
{
	char *string = NULL;
	char *string1 = NULL;
	int len, len1 = 0;
	int i;

	/* read cursor name */
	string = pool_read_string(MASTER(backend), &len, 0);
	if (string == NULL)
		return POOL_END;
	len1 = len;
	string1 = strdup(string);

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
		{
			/* read cursor name */
			string = pool_read_string(CONNECTION(backend, i), &len, 0);
			if (string == NULL)
				return POOL_END;
			if (len != len1)
			{
				pool_error("CursorResponse: length does not match between master(%d) and %d th backend(%d)",
						   len, i, len1);
				pool_error("CursorResponse: master(%s) %d th backend(%s)", string1, string);
				free(string1);
				return POOL_END;
			}
		}
	}

	/* forward to the frontend */
	pool_write(frontend, "P", 1);
	if (pool_write(frontend, string1, len1) < 0)
	{
		free(string1);
		return POOL_END;
	}
	free(string1);

	if (pool_flush(frontend))
		return POOL_END;

	return POOL_CONTINUE;
}

POOL_STATUS ErrorResponse(POOL_CONNECTION *frontend, 
						  POOL_CONNECTION_POOL *backend)
{
	char *string = NULL;
	int len;
	int i;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			/* read error message */
			string = pool_read_string(CONNECTION(backend, i), &len, 0);
			if (string == NULL)
				return POOL_END;
		}
	}

	/* forward to the frontend */
	pool_write(frontend, "E", 1);
	if (pool_write_and_flush(frontend, string, len) < 0)
		return POOL_END;

	/* change transaction state */
	if (TSTATE(backend) == 'T')
		TSTATE(backend) = 'E';
	else
		TSTATE(backend) = 'I';
			
	return POOL_CONTINUE;
}

POOL_STATUS NoticeResponse(POOL_CONNECTION *frontend, 
								  POOL_CONNECTION_POOL *backend)
{
	char *string = NULL;
	int len;
	int i;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			/* read notice message */
			string = pool_read_string(CONNECTION(backend, i), &len, 0);
			if (string == NULL)
				return POOL_END;
		}
	}

	/* forward to the frontend */
	pool_write(frontend, "N", 1);
	if (pool_write_and_flush(frontend, string, len) < 0)
	{
		return POOL_END;
	}
	return POOL_CONTINUE;
}

static POOL_STATUS CopyInResponse(POOL_CONNECTION *frontend, 
								  POOL_CONNECTION_POOL *backend)
{
	POOL_STATUS status;

	/* forward to the frontend */
	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		if (SimpleForwardToFrontend('G', frontend, backend) != POOL_CONTINUE)
			return POOL_END;
		if (pool_flush(frontend) != POOL_CONTINUE)
			return POOL_END;
	}
	else
		if (pool_write_and_flush(frontend, "G", 1) < 0)
			return POOL_END;

	status = CopyDataRows(frontend, backend, 1);
	return status;
}

static POOL_STATUS CopyOutResponse(POOL_CONNECTION *frontend, 
								   POOL_CONNECTION_POOL *backend)
{
	POOL_STATUS status;

	/* forward to the frontend */
	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		if (SimpleForwardToFrontend('H', frontend, backend) != POOL_CONTINUE)
			return POOL_END;
		if (pool_flush(frontend) != POOL_CONTINUE)
			return POOL_END;
	}
	else
		if (pool_write_and_flush(frontend, "H", 1) < 0)
			return POOL_END;

	status = CopyDataRows(frontend, backend, 0);
	return status;
}

static POOL_STATUS CopyDataRows(POOL_CONNECTION *frontend,
								POOL_CONNECTION_POOL *backend, int copyin)
{
	char *string = NULL;
	int len;
	int i;
	DistDefInfo *info = NULL;

#ifdef DEBUG
	int j = 0;
	char buf[1024];
#endif

	if (copyin && pool_config->parallel_mode == TRUE)
	{
		info = pool_get_dist_def_info(MASTER_CONNECTION(backend)->sp->database,
									  copy_schema,
									  copy_table);
	}

	for (;;)
	{
		if (copyin)
		{
			if (MAJOR(backend) == PROTO_MAJOR_V3)
			{
				char kind;
				int sendlen;
				char *p, *p1;

				if (pool_read(frontend, &kind, 1) < 0)
					return POOL_END;

				if (info && kind == 'd')
				{
					int id;
					if (pool_read(frontend, &sendlen, sizeof(sendlen)))
					{
						return POOL_END;
					}

					len = ntohl(sendlen) - 4;

					if (len <= 0)
						return POOL_CONTINUE;

					p = pool_read2(frontend, len);
					if (p == NULL)
						return POOL_END;

					/* copy end ? */
					if (len == 3 && memcmp(p, "\\.\n", 3) == 0)
					{
						for (i=0;i<NUM_BACKENDS;i++)
						{
							if (VALID_BACKEND(i))
							{
								if (pool_write(CONNECTION(backend, i), &kind, 1))
									return POOL_END;
								if (pool_write(CONNECTION(backend, i), &sendlen, sizeof(sendlen)))
									return POOL_END;
								if (pool_write(CONNECTION(backend, i), p, len))
									return POOL_END;
							}
						}
					}
					else
					{
						p1 = parse_copy_data(p, len, copy_delimiter, info->dist_key_col_id);

						if (!p1)
						{
							pool_error("CopyDataRow: cannot parse data");
							return POOL_END;
						}
						else if (strcmp(p1, copy_null) == 0)
						{
							pool_error("CopyDataRow: key parameter is NULL");
							free(p1);
							return POOL_END;
						}

						id = pool_get_id(info, p1);
						pool_debug("CopyDataRow: copying id: %d", id);
						free(p1);
						if (!VALID_BACKEND(id))
						{
							exit(1);
						}
						if (pool_write(CONNECTION(backend, id), &kind, 1))
						{
							return POOL_END;
						}
						if (pool_write(CONNECTION(backend, id), &sendlen, sizeof(sendlen)))
						{
							return POOL_END;
						}
						if (pool_write_and_flush(CONNECTION(backend, id), p, len))
						{
							return POOL_END;
						}
					}
				}
				else
				{
					SimpleForwardToBackend(kind, frontend, backend);
				}

				/* CopyData? */
				if (kind == 'd')
					continue;
				else
				{
					pool_debug("CopyDataRows: copyin kind other than d (%c)", kind);
					break;
				}
			}
			else
				string = pool_read_string(frontend, &len, 1);
		}
		else
		{
			/* CopyOut */
			if (MAJOR(backend) == PROTO_MAJOR_V3)
			{
				signed char kind;

				if ((kind = pool_read_kind(backend)) < 0)
					return POOL_END;

				SimpleForwardToFrontend(kind, frontend, backend);

				/* CopyData? */
				if (kind == 'd')
					continue;
				else
					break;
			}
			else
			{
				for (i=0;i<NUM_BACKENDS;i++)
				{
					if (VALID_BACKEND(i))
					{
						string = pool_read_string(CONNECTION(backend, i), &len, 1);
					}
				}
			}
		}

		if (string == NULL)
			return POOL_END;

#ifdef DEBUG
		strncpy(buf, string, len);
		pool_debug("copy line %d %d bytes :%s:", j++, len, buf);
#endif

		if (copyin)
		{
			for (i=0;i<NUM_BACKENDS;i++)
			{
				if (VALID_BACKEND(i))
				{
					pool_write(CONNECTION(backend, i), string, len);
				}
			}
		}
		else
			pool_write(frontend, string, len);		

		if (len == PROTO_MAJOR_V3)
		{
			/* end of copy? */
			if (string[0] == '\\' &&
				string[1] == '.' &&
				string[2] == '\n')
			{
				break;
			}
		}
	}

	if (copyin)
	{
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i))
			{
				if (pool_flush(CONNECTION(backend, i)) <0)
					return POOL_END;

				if (synchronize(CONNECTION(backend, i)))
					return POOL_END;
			}
		}
	}
	else
		if (pool_flush(frontend) <0)
			return POOL_END;

	return POOL_CONTINUE;
}

static POOL_STATUS EmptyQueryResponse(POOL_CONNECTION *frontend,
									  POOL_CONNECTION_POOL *backend)
{
	char c;
	int i;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			if (pool_read(CONNECTION(backend, i), &c, sizeof(c)) < 0)
				return POOL_END;
		}
	}

	pool_write(frontend, "I", 1);
	return pool_write_and_flush(frontend, "", 1);
}

static POOL_STATUS NotificationResponse(POOL_CONNECTION *frontend, 
										POOL_CONNECTION_POOL *backend)
{
	int pid, pid1;
	char *condition, *condition1 = NULL;
	int len, len1 = 0;
	int i;
	POOL_STATUS status;

	pool_write(frontend, "A", 1);

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			if (pool_read(CONNECTION(backend, i), &pid, sizeof(pid)) < 0)
				return POOL_ERROR;
			condition = pool_read_string(CONNECTION(backend, i), &len, 0);
			if (condition == NULL)
				return POOL_END;

			if (IS_MASTER_NODE_ID(i))
			{
				pid1 = pid;
				len1 = len;
				condition1 = strdup(condition);
			}
		}
	}

	pool_write(frontend, &pid1, sizeof(pid1));
	status = pool_write_and_flush(frontend, condition1, len1);
	free(condition1);
	return status;
}

static POOL_STATUS FunctionCall(POOL_CONNECTION *frontend, 
								POOL_CONNECTION_POOL *backend)
{
	char dummy[2];
	int oid;
	int argn;
	int i;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			pool_write(CONNECTION(backend, i), "F", 1);
		}
	}

	/* dummy */
	if (pool_read(frontend, dummy, sizeof(dummy)) < 0)
		return POOL_ERROR;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			pool_write(CONNECTION(backend, i), dummy, sizeof(dummy));
		}
	}

	/* function object id */
	if (pool_read(frontend, &oid, sizeof(oid)) < 0)
		return POOL_ERROR;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			pool_write(CONNECTION(backend, i), &oid, sizeof(oid));
		}
	}

	/* number of arguments */
	if (pool_read(frontend, &argn, sizeof(argn)) < 0)
		return POOL_ERROR;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			pool_write(CONNECTION(backend, i), &argn, sizeof(argn));
		}
	}

	argn = ntohl(argn);

	for (i=0;i<argn;i++)
	{
		int len;
		char *arg;

		/* length of each argument in bytes */
		if (pool_read(frontend, &len, sizeof(len)) < 0)
			return POOL_ERROR;

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i))
			{
				pool_write(CONNECTION(backend, i), &len, sizeof(len));
			}
		}

		len = ntohl(len);

		/* argument value itself */
		if ((arg = pool_read2(frontend, len)) == NULL)
			return POOL_ERROR;

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i))
			{
				pool_write(CONNECTION(backend, i), arg, len);
			}
		}
	}

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			if (pool_flush(CONNECTION(backend, i)))
				return POOL_ERROR;
		}
	}
	return POOL_CONTINUE;
}

static POOL_STATUS FunctionResultResponse(POOL_CONNECTION *frontend, 
										  POOL_CONNECTION_POOL *backend)
{
	char dummy;
	int len;
	char *result = 0;
	int i;

	pool_write(frontend, "V", 1);

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			if (pool_read(CONNECTION(backend, i), &dummy, 1) < 0)
				return POOL_ERROR;
		}
	}
	pool_write(frontend, &dummy, 1);

	/* non empty result? */
	if (dummy == 'G')
	{
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i))
			{
				/* length of result in bytes */
				if (pool_read(CONNECTION(backend, i), &len, sizeof(len)) < 0)
					return POOL_ERROR;
			}
		}
		pool_write(frontend, &len, sizeof(len));

		len = ntohl(len);

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i))
			{
				/* result value itself */
				if ((result = pool_read2(MASTER(backend), len)) == NULL)
					return POOL_ERROR;
			}
		}
		pool_write(frontend, result, len);
	}

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			/* unused ('0') */
			if (pool_read(MASTER(backend), &dummy, 1) < 0)
				return POOL_ERROR;
		}
	}
	pool_write(frontend, "0", 1);

	return pool_flush(frontend);
}

static POOL_STATUS ProcessFrontendResponse(POOL_CONNECTION *frontend, 
										   POOL_CONNECTION_POOL *backend)
{
	char fkind;
	POOL_STATUS status;
	int i;

	if (frontend->len <= 0 && frontend->no_forward != 0)
		return POOL_CONTINUE;

	if (pool_read(frontend, &fkind, 1) < 0)
	{
		pool_error("ProcessFrontendResponse: failed to read kind from frontend. fronend abnormally exited");
		return POOL_ERROR;
	}

	pool_debug("read kind from frontend %c(%02x)", fkind, fkind);

	switch (fkind)
	{
		case 'X':
			if (MAJOR(backend) == PROTO_MAJOR_V3)
			{
				int len;
				pool_read(frontend, &len, sizeof(len));
			}
			return POOL_END;

		case 'Q':
			status = SimpleQuery(frontend, backend, NULL);
			break;

/*
		case 'S':
			status = Sync(frontend, backend);
			break;
*/
		case 'E':
			status = Execute(frontend, backend);
		break;

		case 'P':
			status = Parse(frontend, backend);
			break;

		default:
			if (MAJOR(backend) == PROTO_MAJOR_V3)
			{
				if (MASTER_SLAVE)
				{
					pool_debug("kind: %c master_slave_dml enabled", fkind);
					master_slave_was_enabled = 1;
					MASTER_SLAVE = 0;
					master_slave_dml = 1;
				}

				status = SimpleForwardToBackend(fkind, frontend, backend);
				for (i=0;i<NUM_BACKENDS;i++)
				{
					if (VALID_BACKEND(i))
					{
						if (pool_flush(CONNECTION(backend, i)))
							status = POOL_ERROR;
					}
				}
			}
			else if (MAJOR(backend) == PROTO_MAJOR_V2 && fkind == 'F')
				status = FunctionCall(frontend, backend);
			else
			{
				pool_error("ProcessFrontendResponse: unknown message type %c(%02x)", fkind, fkind);
				status = POOL_ERROR;
			}
			break;
	}

	if (status != POOL_CONTINUE)
		status = POOL_ERROR;
	return status;
}

static int timeoutmsec;

/*
 * enable read timeout
 */
void pool_enable_timeout()
{
	timeoutmsec = pool_config->replication_timeout;
}

/*
 * disable read timeout
 */
void pool_disable_timeout()
{
	timeoutmsec = 0;
}

/*
 * wait until read data is ready
 */
static int synchronize(POOL_CONNECTION *cp)
{
	return pool_check_fd(cp, 1);
}

/*
 * wait until read data is ready
 * if notimeout is non 0, wait forever.
 */
int pool_check_fd(POOL_CONNECTION *cp, int notimeout)
{
	fd_set readmask;
	fd_set exceptmask;
	int fd;
	int fds;
	struct timeval timeout;
	struct timeval *tp;

	fd = cp->fd;

	for (;;)
	{
		FD_ZERO(&readmask);
		FD_ZERO(&exceptmask);
		FD_SET(fd, &readmask);
		FD_SET(fd, &exceptmask);

		if (notimeout || timeoutmsec == 0)
			tp = NULL;
		else
		{
			timeout.tv_sec = pool_config->replication_timeout / 1000;
			timeout.tv_usec = (pool_config->replication_timeout - (timeout.tv_sec * 1000))*1000;
			tp = &timeout;
		}

		fds = select(fd+1, &readmask, NULL, &exceptmask, tp);

		if (fds == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
				continue;

			pool_error("pool_check_fd: select() failed. reason %s", strerror(errno));
			break;
		}

		if (FD_ISSET(fd, &exceptmask))
		{
			pool_error("pool_check_fd: exception occurred");
			break;
		}

		if (fds == 0)
		{
			pool_error("pool_check_fd: data is not ready tp->tv_sec %d tp->tp_usec %d", 
					   pool_config->replication_timeout / 1000,
					   (pool_config->replication_timeout - (timeout.tv_sec * 1000))*1000);
			break;
		}
		return 0;
	}
	return -1;
}

static void process_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
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

#define MAXITEMS 128

	POOL_REPORT_STATUS status[MAXITEMS];

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

	strncpy(status[i].name, "backend_socket_dir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->backend_socket_dir);
	strncpy(status[i].desc, "Unix domain socket directory for the PostgreSQL server", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "replication_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->replication_mode);
	strncpy(status[i].desc, "non 0 if operating in replication mode", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "replication_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->replication_timeout);
	strncpy(status[i].desc, "if secondary does not respond in this milli seconds, abort the session", POOLCONFIG_MAXDESCLEN);
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

void pool_send_frontend_exits(POOL_CONNECTION_POOL *backend)
{
	int len;
	int i;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
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
			 * famouse "lost synchronization with server, resettin g
			 * connection" message)
			 */
			pool_flush_it(CONNECTION(backend, i));
		}
	}
}

/*
 * -------------------------------------------------------
 * V3 functions
 * -------------------------------------------------------
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
			DeallocateStmt *s = (DeallocateStmt *)pending_prepared_portal->stmt;
			POOL_MEMORY_POOL *old_context = pool_memory;

			free(pending_prepared_portal->portal_name);
			pool_memory = prepare_memory_context;
			pfree(s->name);
			pfree(s);
			pool_memory = old_context;
			free(pending_prepared_portal);
		}
	}
	else if (kind == 'E' && pending_function)
	{
		/* PREPARE or DEALLOCATE is error.
		 * Free pending portal object.
		 */
		free(pending_prepared_portal);
	}
	else if (kind == 'C' && select_in_transaction)
		select_in_transaction = 0;

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
		pool_send_error_message(frontend, MAJOR(backend),
								"XX001", "pgpool detected difference of the number of update tuples", "",
								"check data consistency between master and other db node",  __FILE__, __LINE__);
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
		status = pool_write(frontend, p1, len1);
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
					do_error_command(CONNECTION(backend, i), PROTO_MAJOR_V3);
				}
			}
			select_in_transaction = 0;
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
				 * At this point we regard it is not harmfull since error resonse
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
			if (pool_flush(frontend))
				return POOL_END;
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

	if (kind == 'S') /* Sync message? */
	{
		receive_sync = 1;
	}

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

	else if (kind == 'C' && *p == 'S' && *(p + 1))
	{
		name = strdup(p+1);
		if (name == NULL)
		{
			pool_error("SimpleForwardToBackend: malloc failed: %s", strerror(errno));
			return POOL_END;
		}
		pending_function = del_prepared_list;
		pending_prepared_portal = NULL;
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
		 * Describe message with a portal name receive two messages.
		 * 1. ParameterDescription
		 * 2. RowDescriptions or NoData
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
 * reset backend status. return values are:
 * 0: no query was issued 1: a query was issued 2: no more queries remain -1: error
 */
static int reset_backend(POOL_CONNECTION_POOL *backend, int qcnt)
{
	char *query;
	int qn;

	qn = pool_config->num_reset_queries;

	if (qcnt >= qn)
	{
		if (qcnt >= qn + prepared_list.cnt)
		{
			reset_prepared_list(&prepared_list);
			return 2;
		}

		send_deallocate(backend, &prepared_list, qcnt - qn);
		return 1;
	}

	query = pool_config->reset_query_list[qcnt];

	/* if transaction state is idle, we don't need to issue ABORT */
	if (TSTATE(backend) == 'I' && !strcmp("ABORT", query))
		return 0;

	if (SimpleQuery(NULL, backend, query) != POOL_CONTINUE)
		return -1;

	return 1;
}

/*
 * return non 0 if load balance is possible
 */
static int load_balance_enabled(POOL_CONNECTION_POOL *backend, Node* node, char *sql)
{
	return (pool_config->load_balance_mode &&
			(DUAL_MODE || pool_config->parallel_mode) &&
			MAJOR(backend) == PROTO_MAJOR_V3 &&
			TSTATE(backend) == 'I' &&
			is_select_query(node, sql) &&
			!is_sequence_query(node));
}


/*
 * return non 0 if SQL is SELECT statement.
 */
static int is_select_query(Node *node, char *sql)
{
	SelectStmt *select_stmt;

	if (node == NULL || !IsA(node, SelectStmt))
		return 0;

	select_stmt = (SelectStmt *)node;
	if (select_stmt->intoClause || select_stmt->lockingClause)
		return 0;

	if (pool_config->ignore_leading_white_space)
	{
		/* ignore leading white spaces */
		while (*sql && isspace(*sql))
			sql++;
	}

	return (*sql == 's' || *sql == 'S' || *sql == '(');
}

/*
 * return non 0 if SQL is SELECT statement.
 */
static int is_sequence_query(Node *node)
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

static int is_start_transaction_query(Node *node)
{
	TransactionStmt *stmt;

	if (node == NULL || !IsA(node, TransactionStmt))
		return 0;

	stmt = (TransactionStmt *)node;
	return stmt->kind == TRANS_STMT_START || stmt->kind == TRANS_STMT_BEGIN;
}

static int is_commit_query(Node *node)
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
static void start_load_balance(POOL_CONNECTION_POOL *backend)
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
 * finish load balance mode
 */
static void end_load_balance(POOL_CONNECTION_POOL *backend)
{
	in_load_balance = 0;
	LOAD_BALANCE_STATUS(selected_slot) = LOAD_UNSELECTED;

#ifdef NOT_USED
	/* restore backend connection slots */

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			CONNECTION_SLOT(backend, i) = slots[i];
		}
	}
#endif

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
#define MAXDATA	1024
#define MAXMSGBUF 128

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
		thislen = snprintf(msgbuf, MAXMSGBUF, "SERROR");
		memcpy(data +len, msgbuf, thislen+1);
		len += thislen + 1;

		/* code */
		thislen = snprintf(msgbuf, MAXMSGBUF, "C%s", code);
		memcpy(data +len, msgbuf, thislen+1);
		len += thislen + 1;

		/* message */
		thislen = snprintf(msgbuf, MAXMSGBUF, "M%s", message);
		memcpy(data +len, msgbuf, thislen+1);
		len += thislen + 1;

		/* detail */
		if (*detail != '\0')
		{
			thislen = snprintf(msgbuf, MAXMSGBUF, "D%s", detail);
			memcpy(data +len, msgbuf, thislen+1);
			len += thislen + 1;
		}

		/* hint */
		if (*hint != '\0')
		{
			thislen = snprintf(msgbuf, MAXMSGBUF, "H%s", hint);
			memcpy(data +len, msgbuf, thislen+1);
			len += thislen + 1;
		}

		/* file */
		thislen = snprintf(msgbuf, MAXMSGBUF, "F%s", file);
		memcpy(data +len, msgbuf, thislen+1);
		len += thislen + 1;

		/* line */
		thislen = snprintf(msgbuf, MAXMSGBUF, "L%d", line);
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
 * sends q query in sync manner.
 * this function sends a query and wait for CommandComplete/ReadyForQuery.
 * if an error occured, it returns with POOL_ERROR.
 * this function does NOT handle SELECT/SHOW quries.
 * if no_ready_for_query is non 0, returns without reading the packet
 * length for ReadyForQuery. This mode is necessary when called from ReadyForQuery().
 */
static POOL_STATUS do_command(POOL_CONNECTION *backend, char *query, int protoMajor,
							  int no_ready_for_query)
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
	 * We must check deadlock error because a aborted transaction
	 * by detecting deadlock isn't same on all nodes.
	 * If a transaction is aborted on master node, pgpool send a
	 * error query to another nodes.
	 */
	deadlock_detected = detect_deadlock_error(backend, protoMajor);
	if (deadlock_detected < 0)
		return POOL_END;

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

	if (kind != 'C')
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
		pool_error("do_command: backend does not return ReadyForQuery");
		return POOL_END;
	}

	if (no_ready_for_query)
		return POOL_CONTINUE;

	if (protoMajor == PROTO_MAJOR_V3)
	{
		if (pool_read(backend, &len, sizeof(len)) < 0)
			return POOL_END;

		status = pool_read(backend, &kind, sizeof(kind));
		if (status < 0)
		{
			pool_error("do_command: error while reading transaction status");
			return POOL_END;
		}

		/* set transaction state */
		pool_debug("ReadyForQuery: transaction state: %c", kind);
		backend->tstate = kind;
	}

	return deadlock_detected ? POOL_DEADLOCK : POOL_CONTINUE;
}

/*
 * Send syntax error query to abort transaction.
 * We need to sync transaction status in transaction block.
 * SELECT query is sended to master only.
 * If SELECT is error, we must abort transaction on other nodes.
 */
static POOL_STATUS do_error_command(POOL_CONNECTION *backend, int major)
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

	return POOL_CONTINUE;
}

POOL_STATUS OneNode_do_command(POOL_CONNECTION *frontend, POOL_CONNECTION *backend, char *query, char *database)
{
    int len,sendlen;
    int status;
    char kind;

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
        status = pool_read_parallel(backend, &kind, sizeof(kind));
        if (status < 0)
        {
            pool_error("OneNode_do_command: error while reading message kind");
            return POOL_END;
        }

        status = ParallelForwardToFrontend(kind, frontend, backend, database, true);
        if (kind == 'C' || kind =='E')
        {
            break;
        }
    }
    /*
     *      * Expecting ReadyForQuery
     *           */
    status = pool_read_parallel(backend, &kind, sizeof(kind));
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
 * judge if we need to lock the table
 * to keep SERIAL consistency among servers
 */
static int need_insert_lock(POOL_CONNECTION_POOL *backend, char *query, Node *node)
{
	if (pool_config->ignore_leading_white_space)
	{
		/* ignore leading white spaces */
		while (*query && isspace(*query))
			query++;
	}

	/*
	 * either insert_lock directive specified and without "NO INSERT LOCK" comment
	 * or "INSERT LOCK" comment exists?
	 */
	if ((pool_config->insert_lock && strncasecmp(query, NO_LOCK_COMMENT, NO_LOCK_COMMENT_SZ)) ||
		strncasecmp(query, LOCK_COMMENT, LOCK_COMMENT_SZ) == 0)
	{
		/* INSERT STATEMENT? */
		if (IsA(node, InsertStmt))
			return 1;
	}

	return 0;
}

/*
 * if a transaction has not already started, start a new one.
 * issue LOCK TABLE IN SHARE ROW EXCLUSIVE MODE
 */
static POOL_STATUS insert_lock(POOL_CONNECTION_POOL *backend, char *query, InsertStmt *node)
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

	snprintf(qbuf, sizeof(qbuf), "LOCK TABLE %s IN SHARE ROW EXCLUSIVE MODE", table);

	if (start_internal_transaction(backend, (Node *)node) != POOL_CONTINUE)
		return POOL_END;

	status = POOL_CONTINUE;

	/* issue lock table command */
	table = get_insert_command_table_name(node);
	snprintf(qbuf, sizeof(qbuf), "LOCK TABLE %s IN SHARE ROW EXCLUSIVE MODE", table);

	status = do_command(MASTER(backend), qbuf, MAJOR(backend), 0);
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
				status = do_command(CONNECTION(backend, i), POOL_ERROR_QUERY, PROTO_MAJOR_V3, 0);
			else
				status = do_command(CONNECTION(backend, i), qbuf, PROTO_MAJOR_V3, 0);

			if (status != POOL_CONTINUE)
			{
				internal_transaction_started = 0;
				return POOL_END;
			}
		}
	}

	return POOL_CONTINUE;
}

static bool is_partition_table(POOL_CONNECTION_POOL *backend, Node *node)
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
static int is_drop_database(Node *node)
{
	return (IsA(node, DropdbStmt)) ? 1 : 0;
}

/*
 * check if any pending data remains
 */
static int is_cache_empty(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	int i;

	if (frontend->len > 0)
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
static int is_strict_query(Node *node)
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

static int check_copy_from_stdin(Node *node)
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

static POOL_STATUS read_kind_from_one_backend(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char *kind, int node)
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
 * read kind from backends
 */
static POOL_STATUS read_kind_from_backend(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char *kind)
{
	int i;
	char kind_list[MAX_NUM_BACKENDS];
	char kind_map[256]; /* 256 is the number of sizeof(char) */
	int max_kind = 0;
	double max_count = 0;
	int degenerate_node_num = 0;
	int degenerate_node[MAX_NUM_BACKENDS];

	memset(kind_list, -1, sizeof(kind_list));
	memset(kind_map, 0, sizeof(kind_map));

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			if (pool_read(CONNECTION(backend, i), &kind_list[i], 1) < 0)
			{
				pool_error("pool_process_query: failed to read kind from %d th backend", i);
				return POOL_ERROR;
			}

			pool_debug("read_kind_from_backend: read kind from %d th backend %c NUM_BACKENDS: %d", i, kind_list[i], NUM_BACKENDS);
		}
	}

	/* register kind map */
	for (i = 0; i < NUM_BACKENDS; i++)
	{
		/* kind is singed char.
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

	if (max_count != NUM_BACKENDS)
	{
		int trust_kind;

		if (max_count <= NUM_BACKENDS / 2.0)
		{
			/* The group belonging to master is trusted. */
			trust_kind = kind_list[MASTER_NODE_ID];
		}
		else /* max_count > NUM_BACKENDS / 2.0 */
		{
			trust_kind = max_kind;
		}

		for (i = 0; i < NUM_BACKENDS; i++)
		{
			if (kind_list[i] != -1 && trust_kind != kind_list[i])
			{
				/* degenerate */
				pool_error("pool_process_query: %d th kind %c does not match with master connection kind %c",
						   i, kind_list[i], trust_kind);
				degenerate_node[degenerate_node_num++] = i;
			}
		}
	}

	if (degenerate_node_num)
	{
		pool_send_error_message(frontend, MAJOR(backend), "XX000", 
								"kind mismatch between backends", "",
								"check data consistency between master and other db node",
								__FILE__, __LINE__);
		if (pool_config->replication_stop_on_mismatch)
		{
			degenerate_backend_set(degenerate_node, degenerate_node_num);
			child_exit(1);
		}
		else
			return POOL_ERROR;
	}

	*kind = kind_list[MASTER_NODE_ID];
	return POOL_CONTINUE;
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

static void add_prepared_list(PreparedStatementList *p, Portal *portal)
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

static void add_unnamed_portal(PreparedStatementList *p, Portal *portal)
{
	POOL_MEMORY_POOL *old_context;

	if (unnamed_statement)
	{
		PrepareStmt *p_stmt = (PrepareStmt *)unnamed_statement->stmt;
		if (p_stmt->name == NULL)
		{
			old_context = pool_memory;
			pool_memory = prepare_memory_context;
			pfree(p_stmt->query);
			pfree(p_stmt);
			pool_memory = old_context;
		}
		free(unnamed_statement);
	}

	unnamed_portal = NULL;
	unnamed_statement = portal;
}

static void del_prepared_list(PreparedStatementList *p, Portal *portal)
{
	int i;
	DeallocateStmt *s = (DeallocateStmt *)portal->stmt;
	POOL_MEMORY_POOL *old_context;

	for (i = 0; i < p->cnt; i++)
	{
		PrepareStmt *p_stmt = (PrepareStmt *)p->portal_list[i]->stmt;
		if (strcmp(p_stmt->name, s->name) == 0)
			break;
	}
	
	if (i == p->cnt)
		return;

	old_context = pool_memory;
	pool_memory = prepare_memory_context;
	pfree((PrepareStmt *)p->portal_list[i]->stmt);
	pool_memory = old_context;
	free(p->portal_list[i]->portal_name);
	free(p->portal_list[i]);
	if (i != p->cnt - 1)
	{
		memmove(&p->portal_list[i], &p->portal_list[i+1],
				sizeof(Portal *) * (p->cnt - i - 1));
	}
	p->cnt--;
}

static void delete_all_prepared_list(PreparedStatementList *p, Portal *portal)
{
	reset_prepared_list(p);
}

static void reset_prepared_list(PreparedStatementList *p)
{
	int i;

	if (prepare_memory_context)
	{
		for (i = 0; i < p->cnt; i++)
		{
			free(p->portal_list[i]->portal_name);
			free(p->portal_list[i]);
		}
		pool_memory_delete(prepare_memory_context, 0);
		prepare_memory_context = NULL;
		free(unnamed_statement);
		unnamed_portal = NULL;
		unnamed_statement = NULL;
	}
	p->cnt = 0;
}

static Portal *lookup_prepared_statement_by_statement(PreparedStatementList *p, const char *name)
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

static Portal *lookup_prepared_statement_by_portal(PreparedStatementList *p, const char *name)
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
		pool_error("send_deallocate: malloc failed: %s", strerror(errno));
		exit(1);
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
static char *
parse_copy_data(char *buf, int len, char delimiter, int col_id)
{
	int i, j, field = 0;
	char *str, *p = NULL;

	str = malloc(len + 1);

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
			pool_error("parse_copy_data: malloc failed: %s", strerror(errno));
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

static void query_ps_status(char *query, POOL_CONNECTION_POOL *backend)
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

static POOL_STATUS start_internal_transaction(POOL_CONNECTION_POOL *backend, Node *node)
{
	int i;

	/* if we are not in a transaction block,
	 * start a new transaction
	 */
	if (TSTATE(backend) == 'I' &&
		(IsA(node, InsertStmt) || IsA(node, UpdateStmt) ||
		 IsA(node, DeleteStmt) || IsA(node, SelectStmt)))
	{
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i))
			{
				if (do_command(CONNECTION(backend, i), "BEGIN", MAJOR(backend), 0) != POOL_CONTINUE)
					return POOL_END;
			}
		}

		/* mark that we started new transaction */
		internal_transaction_started = 1;
	}
	return POOL_CONTINUE;
}


static POOL_STATUS end_internal_transaction(POOL_CONNECTION_POOL *backend)
{
	int i;
#ifdef HAVE_SIGPROCMASK
	sigset_t oldmask;
#else
	int	oldmask;
#endif

	/*
	 * We must block all signals. If pgpool SIGTERM, SIGINT or SIGQUIT
	 * is delivered, it possibly causes data consistensy.
	 */
	POOL_SETMASK2(&BlockSig, &oldmask);

	/* We need to commit from secondary to master. */
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
		{
			if (do_command(CONNECTION(backend, i), "COMMIT", MAJOR(backend), 1) != POOL_CONTINUE)
			{
				internal_transaction_started = 0;
				POOL_SETMASK(&oldmask);
				return POOL_END;
			}
		}
	}

	/* commit on master */
	if (do_command(MASTER(backend), "COMMIT", MAJOR(backend), 1) != POOL_CONTINUE)
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
	int r =  detect_error(backend, ADMIN_SHUTDOWN_ERROR_CODE, major, false);
	if (r)
	{
		pool_debug("detect_stop_postmaster_error: receive admin shutdown error from a node.");
		return r;
	}

	r = detect_error(backend, CRASH_SHUTDOWN_ERROR_CODE, major, false);
	if (r == 1)
	{
		pool_debug("detect_stop_postmaster_error: receive crash shutdown error from a node.");
	}
	return r;
}

static int detect_deadlock_error(POOL_CONNECTION *backend, int major)
{
	int r =  detect_error(backend, DEADLOCK_ERROR_CODE, major, true);
	if (r == 1)
		pool_debug("detect_deadlock_error: receive deadlock error from master node.");
	return r;
}

/*
 * detect_error: Detect specified error from error code.
 */
static int detect_error(POOL_CONNECTION *backend, char *error_code, int major, bool unread)
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

	if (kind == 'E') /* ErrorResponseMessage? */
	{
		/* read actual query */
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
			pool_read(backend, str, len);
			readlen += len;
			memcpy(p, str, len);

			/*
			 * Checks error code which it is formatted 'Cxxxxxx'
			 * (xxxxxx is error code).
			 */
			e = str;
			while (*e)
			{
				if (*e == 'C')
				{
					if (strcmp(e+1, error_code) == 0) /* specified error? */
					{
						is_error = 1;
					}
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
			memcpy(p, str, len);
		}
	}
	if (unread || !is_error)
	{
		if (pool_unread(backend, buf, readlen) != 0)
			is_error = -1;
	}

	return is_error;
}
