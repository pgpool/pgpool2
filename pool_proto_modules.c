/* -*-pgsql-c-*- */
/*
 * $Header$
 * 
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2010	PgPool Global Development Group
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
 *---------------------------------------------------------------------
 * pool_proto_modules.c: modules corresponding to message protocols.
 * used by pool_process_query()
 *---------------------------------------------------------------------
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
#include "pool_timestamp.h"
#include "pool_proto_modules.h"
#include "pool_rewrite_query.h"
#include "pool_relcache.h"
#include "pool_stream.h"
#include "pool_config.h"
#include "parser/pool_string.h"
#include "pool_session_context.h"
#include "pool_query_context.h"

int force_replication;
int replication_was_enabled;		/* replication mode was enabled */
int master_slave_was_enabled;	/* master/slave mode was enabled */
int internal_transaction_started;		/* to issue table lock command a transaction
												   has been started internally */
int mismatch_ntuples;	/* number of updated tuples */
char *copy_table = NULL;  /* copy table name */
char *copy_schema = NULL;  /* copy table name */
char copy_delimiter; /* copy delimiter char */
char *copy_null = NULL; /* copy null string */
void (*pending_function)(PreparedStatementList *p, Portal *portal) = NULL;
Portal *pending_prepared_portal = NULL;
Portal *unnamed_statement = NULL;
Portal *unnamed_portal = NULL;
int select_in_transaction = 0; /* non 0 if select query is in transaction */
int execute_select = 0; /* non 0 if select query is in transaction */

/* non 0 if "BEGIN" query with extended query protocol received */
int receive_extended_begin = 0;

/*
 * Non 0 if allow to close internal transaction.  This variable was
 * introduced on 2008/4/3 not to close an internal transaction when
 * Sync message is received after receiving Parse message. This hack
 * is for PHP-PDO.
 */
static int allow_close_transaction = 1;

PreparedStatementList prepared_list; /* prepared statement name list */

int is_select_pgcatalog = 0;
int is_select_for_update = 0; /* 1 if SELECT INTO or SELECT FOR UPDATE */
bool is_parallel_table = false;

/*
 * last query string sent to simpleQuery()
 */
char query_string_buffer[QUERY_STRING_BUFFER_LEN];

/*
 * query string produced by nodeToString() in simpleQuery().
 * this variable only usefull when enable_query_cache is true.
 */
char *parsed_query = NULL;

static int check_errors(POOL_CONNECTION_POOL *backend, int backend_id);
static void generate_error_message(char *prefix, int specific_error, char *query);
static int is_temp_table(POOL_CONNECTION_POOL *backend, Node *node);

/*
 * Process Query('Q') message
 * Query messages include an SQL string.
 */
 POOL_STATUS SimpleQuery(POOL_CONNECTION *frontend,
						 POOL_CONNECTION_POOL *backend, char *query)
{
	char *string, *string1;
	int len;
	static char *sq = "show pool_status";
	int commit;
	List *parse_tree_list;
	Node *node = NULL, *node1;
	POOL_STATUS status;

	POOL_SESSION_CONTEXT *session_context;
	POOL_QUERY_CONTEXT *query_context;

	POOL_MEMORY_POOL *old_context = NULL;
	Portal *portal;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("SimpleQuery: cannot get session context");
		return POOL_END;
	}

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

	/* save last query string for logging purpose */
	strncpy(query_string_buffer, string, sizeof(query_string_buffer));

	/* show ps status */
	query_ps_status(string, backend);

	/* log query to log file if necessary */
	if (pool_config->log_statement)
	{
		pool_log("statement: %s", string);
	}
	else
	{
		pool_debug("statement2: %s", string);
	}

	/* Create query context */
	query_context = pool_init_query_context();
	if (!query_context)
	{
		pool_error("SimpleQuery: pool_init_query_context failed");
		return POOL_END;
	}

	/* Start query processing */
	session_context->in_progress = true;

	/* parse SQL string */
	parse_tree_list = raw_parser(string);

	if (parse_tree_list != NIL)
	{
		/*
		 * XXX: Currently we only process the first element of the parse tree.
		 * rest of multiple statements are silently dicarded.
		 */
		node = (Node *) lfirst(list_head(parse_tree_list));

		if (pool_config->enable_query_cache &&
			SYSDB_STATUS == CON_UP &&
			IsA(node, SelectStmt) &&
			!(is_select_pgcatalog = IsSelectpgcatalog(node, backend)))
		{
			if (pool_execute_query_cache_lookup(frontend, backend, node) != POOL_CONTINUE)
			{
				pool_query_context_destroy(query_context);
				return POOL_ERROR;
			}
		}

		if (PARALLEL_MODE)
		{
			bool parallel = true;

			is_parallel_table = is_partition_table(backend,node);
			status = pool_do_parallel_query(frontend, backend, node, &parallel, &string, &len);
			if (parallel)
			{
				pool_query_context_destroy(query_context);
				return status;
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
			int stime = 5;	/* XXX give arbitrary time to allow closing idle connections */

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
			pool_query_context_destroy(query_context);
			return POOL_CONTINUE;
		}

		if (IsA(node, PrepareStmt) || IsA(node, DeallocateStmt) ||
			IsA(node, VariableSetStmt) || IsA(node, DiscardStmt))
		{
			/*
			 * Before we did followings only when frontend != NULL,
			 * which was wrong since if, for example, reset_query_list
			 * contains "DISCARD ALL", then it does not register
			 * pending function and it causes trying to DEALLOCATE non
			 * existing prepared statment(2009/4/3 Tatsuo).
			 */
			if (IsA(node, PrepareStmt))
			{
				pending_function = add_prepared_list;
				portal = create_portal();
				if (portal == NULL)
				{
					pool_error("SimpleQuery: create_portal() failed");
					return POOL_END;
				}

				/* switch memory context */
				old_context = pool_memory;
				pool_memory = portal->prepare_ctxt;

				portal->portal_name = NULL;
				portal->stmt = copyObject(node);
				portal->sql_string = NULL;
				pending_prepared_portal = portal;
			}
			else if (IsA(node, DeallocateStmt))
			{
				pending_function = del_prepared_list;
				portal = create_portal();
				if (portal == NULL)
				{
					pool_error("SimpleQuery: create_portal() failed");
					return POOL_END;
				}

				/* switch memory context */
				old_context = pool_memory;
				pool_memory = portal->prepare_ctxt;

				portal->portal_name = NULL;
				portal->stmt = copyObject(node);
				portal->sql_string = NULL;
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
			if (old_context)
				pool_memory = old_context;

			/* end of wrong if (see 2009/4/3 comment above) */
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

		/*
		 * Start query context
		 */
		pool_start_query(query_context, string1, node1);

		/*
		 * Decide where to send query
		 */
		pool_where_to_send(query_context, string1, node1);

		/*
		 * determine if we need to lock the table
		 * to keep SERIAL data consistency among servers
		 * conditions:
		 * - replication is enabled
		 * - protocol is V3
		 * - statement is INSERT
		 * - either "INSERT LOCK" comment exists or insert_lock directive specified
		 */
		if (!RAW_MODE)
		{
			/* start a transaction if needed */
			if (start_internal_transaction(frontend, backend, (Node *)node) != POOL_CONTINUE)
				return POOL_END;

			/* check if need lock */
			if (need_insert_lock(backend, string, node))
			{
				/* if so, issue lock command */
				status = insert_lock(frontend, backend, string, (InsertStmt *)node);
				if (status != POOL_CONTINUE)
				{
					free_parser();
					return status;
				}
			}
		}
		else if (REPLICATION && query == NULL && start_internal_transaction(frontend, backend, node))
		{
			free_parser();
			return POOL_ERROR;
		}
	}
	else
	{
		/*
		 * Unable to parse the query. Probably syntax error or the
		 * query is too new and our parser cannot understand. Treat as
		 * if it were an ordinaly SET command(thus replicated).
		 */
		char *p = "SET DATESTYLE TO ISO";
		parse_tree_list = raw_parser(p);
		node = (Node *) lfirst(list_head(parse_tree_list));
		pool_where_to_send(query_context, p, node);
		free_parser();
		node = NULL;
	}

	if (MAJOR(backend) == PROTO_MAJOR_V2 && is_start_transaction_query(node))
	{
		TSTATE(backend) = 'T';
	}

	if (!RAW_MODE)
	{
		/* check if query is "COMMIT" or "ROLLBACK" */
		commit = is_commit_query(node);

		/*
		 * Query is not commit/rollback
		 */
		if (!commit)
		{
			char		*rewrite_query;

			if (node)
		   	{
				Portal *portal = NULL;

				if (IsA(node, PrepareStmt))
				{
					portal = pending_prepared_portal;
					portal->num_tsparams = 0;
				}
				else if (IsA(node, ExecuteStmt))
					portal = lookup_prepared_statement_by_statement(
							&prepared_list, ((ExecuteStmt *) node)->name);

				/* rewrite `now()' to timestamp literal */
				rewrite_query = rewrite_timestamp(backend, node, false, portal);
				if (rewrite_query != NULL)
				{
					string = rewrite_query;
					len = strlen(string) + 1;
				}

			}

			/*
			 * Optimization effort: If there's only one session, we do
			 * not need to wait for the master node's response, and
			 * could execute the query concurrently.
			 */
			if (pool_config->num_init_children == 1)
			{
				/* Send query to all DB nodes at once */
				status = pool_send_and_wait(query_context, string, len, 0, 0);
				free_parser();
				return status;
			}

			/* Send the query to master node */
			if (pool_send_and_wait(query_context, string, len, 1, MASTER_NODE_ID) != POOL_CONTINUE)
			{
				free_parser();
				return POOL_END;
			}
		}

		/* Send query to other than master node */
		if (pool_send_and_wait(query_context, string, len, -1, MASTER_NODE_ID) != POOL_CONTINUE)
		{
			free_parser();
			return POOL_END;
		}

		/* Send "COMMIT" or "ROLLBACK" to only master node if query is "COMMIT" or "ROLLBACK" */
		if (commit)
		{
			if (pool_send_and_wait(query_context, string, len, 1, MASTER_NODE_ID) != POOL_CONTINUE)
			{
				free_parser();
				return POOL_END;
			}
		}
		free_parser();
	}
	else
	{
		if (pool_send_and_wait(query_context, string, len, 1, MASTER_NODE_ID) != POOL_CONTINUE)
		{
			free_parser();
			return POOL_END;
		}
		free_parser();
	}
	return POOL_CONTINUE;
}

/*
 * process EXECUTE (V3 only)
 */
POOL_STATUS Execute(POOL_CONNECTION *frontend,
						   POOL_CONNECTION_POOL *backend)
{
	char *string;		/* portal name + null terminate + max_tobe_returned_rows */
	int len;
	int i;
	char kind;
	int status, commit = 0;
	Portal *portal;
	char *string1 = NULL;
	PrepareStmt *p_stmt;
	POOL_STATUS ret;
	int specific_error = 0;

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
		Node *node;

		p_stmt = (PrepareStmt *)portal->stmt;

		string1 = portal->sql_string;
		pool_debug("Execute: query: %s", string1);
		node = (Node *)p_stmt->query;
		strncpy(query_string_buffer, string1, sizeof(query_string_buffer));

 		if ((IsA(node, PrepareStmt) || IsA(node, DeallocateStmt) ||
 			 IsA(node, VariableSetStmt)) &&
  			MASTER_SLAVE && TSTATE(backend) != 'E')
		{
			/*
			 * PREPARE, DEALLOCATE, SET, DISCARD
			 * should be executed on all nodes.  So we set
			 * force_replication.
			 */
			force_replication = 1;
		}
		/*
		 * JDBC driver sends "BEGIN" query internally if
		 * setAutoCommit(false).  But it does not send Sync message
		 * after "BEGIN" query.  In extended query protocol,
		 * PostgreSQL returns ReadyForQuery when a client sends Sync
		 * message.  Problem is, pgpool can't know the transaction
		 * state without receiving ReadyForQuery. So we remember that
		 * we need to send Sync message internally afterward, whenever
		 * we receive BEGIN in extended protocol.
		 */
		else if (IsA(node, TransactionStmt) && MASTER_SLAVE)
		{
			TransactionStmt *stmt = (TransactionStmt *) node;

			if (stmt->kind == TRANS_STMT_BEGIN ||
				stmt->kind == TRANS_STMT_START)
				/* Remember we need to send sync later in extended protocol */
				receive_extended_begin = 1;
		}

#ifdef REMOVED
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
			execute_select = 1;
		}
#endif
/*
		else if (REPLICATION && start_internal_transaction(backend, (Node *)p_stmt->query))
		{
			return POOL_END;
		}
*/
		/* check if query is "COMMIT" or "ROLLBACK" */
		commit = is_commit_query((Node *)p_stmt->query);
	}

#ifdef REMOVED
	if (MASTER_SLAVE)
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
#endif

	if (REPLICATION || PARALLEL_MODE)
	{
		/*
		 * Query is not commit/rollback
		 */
		if (!commit)
		{
			/* Send the query to master node */
			per_node_statement_log(backend, MASTER_NODE_ID, string1);
			if (send_execute_message(backend, MASTER_NODE_ID, len, string) != POOL_CONTINUE)
				return POOL_END;

			if (wait_for_query_response(frontend, MASTER(backend), string, MAJOR(backend)) != POOL_CONTINUE)
			{
				/* Cancel current transaction */
				CancelPacket cancel_packet;

				cancel_packet.protoVersion = htonl(PROTO_CANCEL);
				cancel_packet.pid = MASTER_CONNECTION(backend)->pid;
				cancel_packet.key= MASTER_CONNECTION(backend)->key;
				cancel_request(&cancel_packet);

				return POOL_END;
			}


			/* Check specific errors */
			specific_error = check_errors(backend, MASTER_NODE_ID);
			if (specific_error)
			{
				/* log error message */
				generate_error_message("Execute: ", specific_error, string);
			}
		}

		/* send query to other nodes */
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i) || IS_MASTER_NODE_ID(i))
				continue;

			if (specific_error)
			{
				char msg[1024] = "pgpoool_error_portal"; /* large enough */
				int len = strlen(msg);

				memset(msg + len, 0, sizeof(int));
				if (send_execute_message(backend, i, len + 5, msg))
					return POOL_END;
			}
			else
			{
				per_node_statement_log(backend, i, string1);
				if (send_execute_message(backend, i, len, string) != POOL_CONTINUE)
					return POOL_END;
			}
		}

		/* Wait for nodes other than the master node */
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i) || IS_MASTER_NODE_ID(i))
				continue;

			if (wait_for_query_response(frontend, CONNECTION(backend, i), string, MAJOR(backend)) != POOL_CONTINUE)
			{
				/* Cancel current transaction */
				CancelPacket cancel_packet;

				cancel_packet.protoVersion = htonl(PROTO_CANCEL);
				cancel_packet.pid = MASTER_CONNECTION(backend)->pid;
				cancel_packet.key= MASTER_CONNECTION(backend)->key;
				cancel_request(&cancel_packet);

				return POOL_END;
			}
		}

		/* send "COMMIT" or "ROLLBACK" to only master node if query is "COMMIT" or "ROLLBACK" */
		if (commit)
		{
			per_node_statement_log(backend, MASTER_NODE_ID, string1);

			if (send_execute_message(backend, MASTER_NODE_ID, len, string) != POOL_CONTINUE)
				return POOL_END;

			if (wait_for_query_response(frontend, MASTER(backend), string, MAJOR(backend)) != POOL_CONTINUE)
			{
				/* Cancel current transaction */
				CancelPacket cancel_packet;

				cancel_packet.protoVersion = htonl(PROTO_CANCEL);
				cancel_packet.pid = MASTER_CONNECTION(backend)->pid;
				cancel_packet.key= MASTER_CONNECTION(backend)->key;
				cancel_request(&cancel_packet);

				return POOL_END;
			}
		}
	}
	else
	{
		per_node_statement_log(backend, MASTER_NODE_ID, string1);

		if (send_execute_message(backend, MASTER_NODE_ID, len, string) != POOL_CONTINUE)
			return POOL_END;

		if (wait_for_query_response(frontend, MASTER(backend), string, MAJOR(backend)) != POOL_CONTINUE)
		{
				/* Cancel current transaction */
				CancelPacket cancel_packet;

				cancel_packet.protoVersion = htonl(PROTO_CANCEL);
				cancel_packet.pid = MASTER_CONNECTION(backend)->pid;
				cancel_packet.key= MASTER_CONNECTION(backend)->key;
				cancel_request(&cancel_packet);

				return POOL_END;
		}
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
	}
	if (ret != POOL_CONTINUE)
		return ret;

	status = SimpleForwardToFrontend(kind, frontend, backend);
	if (status != POOL_CONTINUE)
		return status;

	return POOL_CONTINUE;
}

/*
 * process Parse (V3 only)
 */
POOL_STATUS Parse(POOL_CONNECTION *frontend,
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
	char per_node_statement_log_buffer[1024];

	/* read Parse packet */
	if (pool_read(frontend, &len, sizeof(len)) < 0)
		return POOL_END;

	len = ntohl(len) - 4;
	string = pool_read2(frontend, len);

	pool_debug("Parse: statement name <%s>", string);

	name = string;
	stmt = string + strlen(string) + 1;

	parse_tree_list = raw_parser(stmt);
	if (parse_tree_list == NIL)
	{
		/* free_parser(); */
		;
	}
	else
	{
		/* Save last query string for logging purpose */
		snprintf(query_string_buffer, sizeof(query_string_buffer), "Parse: %s", stmt);

		node = (Node *) lfirst(list_head(parse_tree_list));

		insert_stmt_with_lock = need_insert_lock(backend, stmt, node);

		/* Special treatment for master/slave + temp tables */
		if (MASTER_SLAVE)
		{
			/* Is there "NO LOAD BALANCE" comment? */
			if (!strncasecmp(stmt, NO_LOAD_BALANCE, NO_LOAD_BALANCE_COMMENT_SZ) ||
				/* or the table used in a query is a temporary one ? */
				is_temp_table(backend, node))
			{
				/*
				 * From now on, let only master handle queries.  This is
				 * typically usefull for using temp tables in master/slave
				 * mode
				 */
				master_slave_was_enabled = 1;
				MASTER_SLAVE = 0;
				master_slave_dml = 1;
			}
		}

		portal = create_portal();
		if (portal == NULL)
		{
			pool_error("Parse: create_portal() failed");
			free_parser();
			return POOL_END;
		}

		/* switch memory context */
		old_context = pool_memory;
		pool_memory = portal->prepare_ctxt;

		/* translate Parse message to PrepareStmt */
		p_stmt = palloc(sizeof(PrepareStmt));
		p_stmt->type = T_PrepareStmt;

		/* XXX: there's a confusion here. Someone mixed up statement
		 * name with portal name. It is regarded that statment name ==
		 * portal name. Someday we should fix this. Sigh.
		 */
		p_stmt->name = pstrdup(name);
		p_stmt->query = copyObject(node);
		portal->stmt = (Node *)p_stmt;
		portal->portal_name = NULL;
		portal->sql_string = pstrdup(stmt);

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

		/*
		 * Switch to old memory context. Caution. Now we are in parser
		 * memory context.
		 * Palloced memories will be gone if free_parser() called!
		 */
		pool_memory = old_context;

		if (REPLICATION)
		{
			char		*rewrite_query;
			bool		 rewrite_to_params = true;

			/*
			 * rewrite `now()'.
			 * if stmt is unnamed, we rewrite `now()' to timestamp constant.
			 * else we rewrite `now()' to params and expand that at Bind
			 * message.
			 */
			if (*name == '\0')
				rewrite_to_params = false;
			portal->num_tsparams = 0;
			rewrite_query = rewrite_timestamp(backend, node, rewrite_to_params, portal);
			if (rewrite_query != NULL)
			{
				int alloc_len = len - strlen(stmt) + strlen(rewrite_query);
				string = palloc(alloc_len);
				strcpy(string, name);
				strcpy(string + strlen(name) + 1, rewrite_query);
				memcpy(string + strlen(name) + strlen(rewrite_query) + 2,
						stmt + strlen(stmt) + 1,
						len - (strlen(name) + strlen(stmt) + 2));

				len = len - strlen(stmt) + strlen(rewrite_query);
				name = string;
				stmt = string + strlen(name) + 1;
				pool_debug("rewrite query  %s %s len=%d", name, stmt, len);
			}
		}

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

					/* send sync message */
					send_extended_protocol_message(backend, i, "S", 0, "");
				}

				kind = pool_read_kind(backend);
				if (kind != 'Z')
				{
					free_parser();
					return POOL_END;
				}

				if (ReadyForQuery(frontend, backend, 0) != POOL_CONTINUE)
				{
					free_parser();
					return POOL_END;
				}
			}

			if (is_strict_query(node))
				start_internal_transaction(frontend, backend, node);

			if (insert_stmt_with_lock)
			{
				/* start a transaction if needed and lock the table */
				status = insert_lock(frontend, backend, stmt, (InsertStmt *)node);
				if (status != POOL_CONTINUE)
				{
					free_parser();
					return status;
				}
			}
		}
	}

	/* send to master node */
	snprintf(per_node_statement_log_buffer, sizeof(per_node_statement_log_buffer), "Parse: %s", stmt);
	per_node_statement_log(backend, MASTER_NODE_ID, per_node_statement_log_buffer);
	if (send_extended_protocol_message(backend, MASTER_NODE_ID,
									   "P", len, string))
	{
		free_parser();
		return POOL_END;
	}

	/*
	 * Cannot call free_parser() here. Since "string" might be allocated in parser context.
	 * free_parser();
	 */

	if (REPLICATION || PARALLEL_MODE || MASTER_SLAVE)
	{
		/*
		 * We must synchronize because Parse message acquires table
		 * locks.
		 */
		pool_debug("Parse: waiting for master completing the query");
		if (wait_for_query_response(frontend, MASTER(backend), string, MAJOR(backend)) != POOL_CONTINUE)
		{
			/* Cancel current transaction */
			CancelPacket cancel_packet;

			cancel_packet.protoVersion = htonl(PROTO_CANCEL);
			cancel_packet.pid = MASTER_CONNECTION(backend)->pid;
			cancel_packet.key= MASTER_CONNECTION(backend)->key;
			cancel_request(&cancel_packet);
			free_parser();
			return POOL_END;
		}

		/*
		 * We must check deadlock error because a aborted transaction
		 * by detecting deadlock isn't same on all nodes.
		 * If a transaction is aborted on master node, pgpool send a
		 * error query to another nodes.
		 */
		deadlock_detected = detect_deadlock_error(MASTER(backend), MAJOR(backend));
		if (deadlock_detected < 0)
		{
			free_parser();
			return POOL_END;
		}
		else
		{
			/*
			 * Check if other than deadlock error detected.  If so, emit
			 * log. This is usefull when invalid encoding error occurs. In
			 * this case, PostgreSQL does not report what statement caused
			 * that error and make users confused.
			 */
			per_node_error_log(backend, MASTER_NODE_ID, stmt, "Parse(): Error or notice message from backend: ", true);
		}

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
			{
				if (deadlock_detected)
				{
					pool_log("Parse: received deadlock error message from master node");

					per_node_statement_log(backend, i, POOL_ERROR_QUERY);

					if (send_simplequery_message(CONNECTION(backend, i),
												 strlen(POOL_ERROR_QUERY)+1,
												 POOL_ERROR_QUERY,
												 MAJOR(backend)))
					{
						free_parser();
						return POOL_END;
					}
				}
				else
				{
					snprintf(per_node_statement_log_buffer, sizeof(per_node_statement_log_buffer), "Parse: %s", stmt);
					per_node_statement_log(backend, i, per_node_statement_log_buffer);

					if (send_extended_protocol_message(backend, i,"P", len, string))
					{
						free_parser();
						return POOL_END;
					}
				}
			}
		}

		/* wait for DB nodes completing query except master node */
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i) || IS_MASTER_NODE_ID(i))
				continue;

			pool_debug("Parse: waiting for %dth backend completing the query", i);
			if (wait_for_query_response(frontend, CONNECTION(backend, i), string, MAJOR(backend)) != POOL_CONTINUE)
			{
				/* Cancel current transaction */
				CancelPacket cancel_packet;

				cancel_packet.protoVersion = htonl(PROTO_CANCEL);
				cancel_packet.pid = MASTER_CONNECTION(backend)->pid;
				cancel_packet.key= MASTER_CONNECTION(backend)->key;
				cancel_request(&cancel_packet);
				free_parser();
				return POOL_END;
			}

			/*
			 * Check if error (or notice response) from backend is
			 * detected.  If so, emit log. This is usefull when
			 * invalid encoding error occurs. In this case, PostgreSQL
			 * does not report what statement caused that error and
			 * make users confused.
			 */
			per_node_error_log(backend, i, stmt, "Parse(): Error or notice message from backend: ", true);
		}
	}

	/*
	 * Ok. we are safe to call free_parser();
	 */
	free_parser();

	for (;;)
	{
		POOL_STATUS ret;
		ret = read_kind_from_backend(frontend, backend, &kind);

		if (ret != POOL_CONTINUE)
			return ret;

		SimpleForwardToFrontend(kind, frontend, backend);
		pool_flush(frontend);

		/* Ignore warning messages */
		if (kind != 'N')
			break;
	}
	return POOL_CONTINUE;
}

/*
 * Process ReadyForQuery('Z') message.
 *
 * - if the global error status "mismatch_ntuples" is set, send an error query
 *	 to all DB nodes to abort transaction.
 * - internal transaction is closed
 */
POOL_STATUS ReadyForQuery(POOL_CONNECTION *frontend,
								 POOL_CONNECTION_POOL *backend, int send_ready)
{
	StartupPacket *sp;
	char psbuf[1024];
	int i;
	int len;
	signed char state;

	/*
	 * If the numbers of update tuples are differ, we need to abort transaction
	 * by using do_error_command. This only works with PROTO_MAJOR_V3.
	 */
	if (mismatch_ntuples && MAJOR(backend) == PROTO_MAJOR_V3)
	{
		int i;
		signed char state;
		char kind;

		/*
		 * XXX: discard rest of ReadyForQuery packet
		 */
		if (pool_read_message_length(backend) < 0)
			return POOL_END;

		state = pool_read_kind(backend);
		if (state < 0)
			return POOL_END;

		pool_debug("ReadyForQuery: transaction state: %c", state);

		for (i = 0; i < NUM_BACKENDS; i++)
		{
			if (VALID_BACKEND(i))
			{
				/* abort transaction on all nodes. */
				do_error_command(CONNECTION(backend, i), PROTO_MAJOR_V3);
			}
		}

		/* loop through until we get ReadyForQuery */
		for(;;)
		{
			kind = pool_read_kind(backend);
			if (kind < 0)
				return POOL_END;

			if (kind == 'Z')
				break;

			/* put the message back to read buffer */
			for (i=0;i<NUM_BACKENDS;i++)
			{
				if (VALID_BACKEND(i))
				{
					pool_unread(CONNECTION(backend,i), &kind, 1);
				}
			}

			/* discard rest of the packet */
			if (pool_discard_packet(backend) != POOL_CONTINUE)
			{
				pool_error("ReadyForQuery: pool_discard_packet failed");
				return POOL_END;
			}
		}
		mismatch_ntuples = 0;
	}

	/*
	 * if a transaction is started for insert lock, we need to close
	 * the transaction.
	 */
	if (internal_transaction_started && allow_close_transaction)
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

		if (end_internal_transaction(frontend, backend) != POOL_CONTINUE)
			return POOL_ERROR;
	}

	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		if ((len = pool_read_message_length(backend)) < 0)
			return POOL_END;

		pool_debug("ReadyForQuery: message length: %d", len);

		/*
		 * Do not check transaction state in master/slave mode.
		 * Because SET, PREPARE, DEALLOCATE are replicated.
		 * If these queries are executed inside a transaction block,
		 * transation state will be inconsistent. But it is no problem.
		 */
		if (master_slave_dml)
		{
			char kind, kind1;

			if (pool_read(MASTER(backend), &kind, sizeof(kind)))
				return POOL_END;

			for (i = 0; i < NUM_BACKENDS; i++)
			{
				if (!VALID_BACKEND(i) || IS_MASTER_NODE_ID(i))
					continue;

				if (pool_read(CONNECTION(backend, i), &kind1, sizeof(kind)))
					return POOL_END;
			}
			state = kind;
		}
		else
		{
			state = pool_read_kind(backend);
			if (state < 0)
				return POOL_END;
		}

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
		pool_write(frontend, "Z", 1);

		if (MAJOR(backend) == PROTO_MAJOR_V3)
		{
			len = htonl(len);
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, &state, 1);
		}
		pool_flush(frontend);
	}

	pool_unset_query_in_progress();

	/* end load balance mode */
	if (in_load_balance)
		end_load_balance();

#ifdef REMOVED
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
#endif

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


POOL_STATUS FunctionCall(POOL_CONNECTION *frontend,
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

POOL_STATUS ProcessFrontendResponse(POOL_CONNECTION *frontend,
										   POOL_CONNECTION_POOL *backend)
{
	char fkind;
	char kind;
	POOL_STATUS status;
	int i;

	if (pool_read_buffer_is_empty(frontend) && frontend->no_forward != 0)
		return POOL_CONTINUE;

	if (pool_read(frontend, &fkind, 1) < 0)
	{
		pool_log("ProcessFrontendResponse: failed to read kind from frontend. frontend abnormally exited");
		return POOL_END;
	}

	pool_debug("read kind from frontend %c(%02x)", fkind, fkind);

	/*
	 * If we have received BEGIN in extended protocol before, we need
	 * to send a sync message to know the transaction stare.
	 */
	if (receive_extended_begin)
	{
		receive_extended_begin = 0;

		/* send sync message */
		send_extended_protocol_message(backend, MASTER_NODE_ID, "S", 0, "");

		kind = pool_read_kind(backend);
		if (kind != 'Z')
			return POOL_END;
		if (ReadyForQuery(frontend, backend, 0) != POOL_CONTINUE)
			return POOL_END;
	}

	switch (fkind)
	{

		case 'X':  /* Terminate message*/
			if (MAJOR(backend) == PROTO_MAJOR_V3)
			{
				int len;
				pool_read(frontend, &len, sizeof(len));
			}
			return POOL_END;

		case 'Q':  /* Query message*/
			allow_close_transaction = 1;
			status = SimpleQuery(frontend, backend, NULL);
			break;

		case 'E':  /* Execute message */
			allow_close_transaction = 1;
			status = Execute(frontend, backend);
			break;

		case 'P':  /* Parse message */
			allow_close_transaction = 0;

			if (MASTER_SLAVE &&
				(TSTATE(backend) != 'I' || receive_extended_begin))
			{
				pool_debug("kind: %c master_slave_dml enabled", fkind);
				master_slave_was_enabled = 1;
				MASTER_SLAVE = 0;
				master_slave_dml = 1;
			}

			status = Parse(frontend, backend);
			break;

		case 'S':  /* Sync message */
			receive_extended_begin = 0;
			/* fall through */

		default:
			if ((MAJOR(backend) == PROTO_MAJOR_V3) &&
			    (fkind == 'S' || fkind == 'H' || fkind == 'D' || fkind == 'f'||
				 fkind == 'C' || fkind == 'B' || fkind == 'F' || fkind == 'd' || fkind == 'c'))
			{
				if (MASTER_SLAVE &&
					(TSTATE(backend) != 'I' || receive_extended_begin))
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

POOL_STATUS CopyInResponse(POOL_CONNECTION *frontend,
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

POOL_STATUS CopyOutResponse(POOL_CONNECTION *frontend,
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

POOL_STATUS CopyDataRows(POOL_CONNECTION *frontend,
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

/*
 * Check various errors from backend.  return values: 0: no error 1:
 * deadlock detected 2: serialization error detected 3: query cancel
 * detected: 4
 */
static int check_errors(POOL_CONNECTION_POOL *backend, int backend_id)
{

	/*
	 * Check dead lock error on the master node and abort
	 * transactions on all nodes if so.
	 */
	if (detect_deadlock_error(CONNECTION(backend, backend_id), MAJOR(backend)) == SPECIFIED_ERROR)
		return 1;

	/*
	 * Check serialization failure error and abort
	 * transactions on all nodes if so. Otherwise we allow
	 * data inconsistency among DB nodes. See following
	 * scenario: (M:master, S:slave)
	 *
	 * M:S1:BEGIN;
	 * M:S2:BEGIN;
	 * S:S1:BEGIN;
	 * S:S2:BEGIN;
	 * M:S1:SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
	 * M:S2:SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
	 * S:S1:SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
	 * S:S2:SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
	 * M:S1:UPDATE t1 SET i = i + 1;
	 * S:S1:UPDATE t1 SET i = i + 1;
	 * M:S2:UPDATE t1 SET i = i + 1; <-- blocked
	 * S:S1:COMMIT;
	 * M:S1:COMMIT;
	 * M:S2:ERROR:  could not serialize access due to concurrent update
	 * S:S2:UPDATE t1 SET i = i + 1; <-- success in UPDATE and data becomes inconsistent!
	 */
	if (detect_serialization_error(CONNECTION(backend, backend_id), MAJOR(backend)) == SPECIFIED_ERROR)
		return 2;

	/*
	 * check "SET TRANSACTION ISOLATION LEVEL must be called before any query" error.
	 * This happens in following scenario:
	 *
	 * M:S1:BEGIN;
	 * S:S1:BEGIN;
	 * M:S1:SELECT 1; <-- only sent to MASTER
	 * M:S1:SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
	 * S:S1:SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
	 * M: <-- error
	 * S: <-- ok since no previous SELECT is sent. kind mismatch error occurs!
	 */
	if (detect_active_sql_transaction_error(CONNECTION(backend, backend_id), MAJOR(backend)) == SPECIFIED_ERROR)
		return 3;

	/* check query cancel error */
	if (detect_query_cancel_error(CONNECTION(backend, backend_id), MAJOR(backend)) == SPECIFIED_ERROR)
		return 4;

	return 0;
}

static void generate_error_message(char *prefix, int specific_error, char *query)
{
	static char *error_messages[] = {
		"received deadlock error message from master node. query: %s",
		"received serialization failure error message from master node. query: %s",
		"received SET TRANSACTION ISOLATION LEVEL must be called before any query error. query: %s",
		"received query cancel error message from master node. query: %s"
	};

	String *msg;

	if (specific_error < 1 || specific_error > sizeof(error_messages)/sizeof(char *))
	{
		pool_error("generate_error_message: invalid specific_error: %d", specific_error);
		return;
	}

	specific_error--;

	msg = init_string(prefix);
	string_append_char(msg, error_messages[specific_error]);
	pool_error(msg->data, query);
	free_string(msg);
}

/*
 * Judge the table used in a query represented by node is a temporary
 * table or not.
 */
static int is_temp_table(POOL_CONNECTION_POOL *backend, Node *node)
{
/*
 * Query to know if pg_class has relistemp column or not.
 * PostgreSQL 8.4 or later has this.
 */
#define HASRELITEMPPQUERY "SELECT count(*) FROM pg_catalog.pg_class AS c, pg_attribute AS a WHERE c.relname = 'pg_class' AND a.attrelid = c.oid AND a.attname = 'relistemp'"

/*
 * Query to know if the target table is a temporary one.
 * This query is valid through PostgreSQL 7.3 to 8.3.
 */
#define ISTEMPQUERY83 "SELECT count(*) FROM pg_class AS c, pg_namespace AS n WHERE c.relname = '%s' AND c.relnamespace = n.oid AND n.nspname ~ '^pg_temp_'"

/*
 * Query to know if the target table is a temporary one.
 * This query is valid PostgreSQL 8.4 or later.
 */
#define ISTEMPQUERY84 "SELECT count(*) FROM pg_catalog.pg_class AS c WHERE c.relname = '%s' AND c.relistemp"

	char *str;
	int hasrelistemp;
	int result;
	static POOL_RELCACHE *hasrelistemp_cache;
	static POOL_RELCACHE *relcache;
	char *query;

	/*
	 * For version 2 protocol, we cannot support the checking since
	 * the underlying infrastructure (do_query) does not support the
	 * protocol. So we just return false.
	 */
	if (MAJOR(backend) == PROTO_MAJOR_V2)
		return 0;

	/* For SELECT, it's hard to extract table names. So we always return 0 */
	if (IsA(node, SelectStmt))
	{
		return 0;
	}

	/* Obtain table name */
	if (IsA(node, InsertStmt))
		str = nodeToString(((InsertStmt *)node)->relation);
	else if (IsA(node, UpdateStmt))
		str = nodeToString(((UpdateStmt *)node)->relation);
	else if (IsA(node, DeleteStmt))
		str = nodeToString(((DeleteStmt *)node)->relation);
	else		/* Unknown statement */
		str = NULL;

	if (str == NULL)
	{
			return 0;
	}

	/*
	 * Check backend version
	 */
	if (!hasrelistemp_cache)
	{
		hasrelistemp_cache = pool_create_relcache(32, HASRELITEMPPQUERY,
										int_register_func, int_unregister_func,
										false);
		if (hasrelistemp_cache == NULL)
		{
			pool_error("is_temp_table: pool_create_relcache error");
			return false;
		}
	}

	hasrelistemp = pool_search_relcache(hasrelistemp_cache, backend, "pg_class")==0?0:1;
	if (hasrelistemp)
		query = ISTEMPQUERY84;
	else
		query = ISTEMPQUERY83;

	/*
	 * If relcache does not exist, create it.
	 */
	if (!relcache)
	{
		relcache = pool_create_relcache(32, query,
										int_register_func, int_unregister_func,
										true);
		if (relcache == NULL)
		{
			pool_error("is_temp_table: pool_create_relcache error");
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
 * Make per DB node statement log
 */
void per_node_statement_log(POOL_CONNECTION_POOL *backend, int node_id, char *query)
{
	POOL_CONNECTION_POOL_SLOT *slot = backend->slots[node_id];

	if (pool_config->log_per_node_statement)
		pool_log("DB node id: %d backend pid: %d statement: %s", node_id, ntohl(slot->pid), query);
}

/*
 * Check kind and produce error message
 * All data read in this function is returned to stream.
 */
void per_node_error_log(POOL_CONNECTION_POOL *backend, int node_id, char *query, char *prefix, bool unread)
{
	POOL_CONNECTION_POOL_SLOT *slot = backend->slots[node_id];
	char *message;

	if (pool_extract_error_message(true, CONNECTION(backend, node_id), MAJOR(backend), true, &message) > 0)
	{
		pool_log("%s: DB node id: %d backend pid: %d statement: %s message: %s",
				 prefix, node_id, ntohl(slot->pid), query, message);
	}
}
