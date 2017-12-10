/* -*-pgsql-c-*- */
/*
 * $Header$
 * 
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2015	PgPool Global Development Group
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
#include "pool_lobj.h"
#include "pool_select_walker.h"
#include "pool_memqcache.h"

char *copy_table = NULL;  /* copy table name */
char *copy_schema = NULL;  /* copy table name */
char copy_delimiter; /* copy delimiter char */
char *copy_null = NULL; /* copy null string */

/*
 * Non 0 if allow to close internal transaction.  This variable was
 * introduced on 2008/4/3 not to close an internal transaction when
 * Sync message is received after receiving Parse message. This hack
 * is for PHP-PDO.
 */
static int allow_close_transaction = 1;

int is_select_pgcatalog = 0;
int is_select_for_update = 0; /* 1 if SELECT INTO or SELECT FOR UPDATE */
bool is_parallel_table = false;

/*
 * last query string sent to simpleQuery()
 */
char query_string_buffer[QUERY_STRING_BUFFER_LEN];

/*
 * query string produced by nodeToString() in simpleQuery().
 * this variable only useful when enable_query_cache is true.
 */
char *parsed_query = NULL;

static int check_errors(POOL_CONNECTION_POOL *backend, int backend_id);
static void generate_error_message(char *prefix, int specific_error, char *query);
static POOL_STATUS parse_before_bind(POOL_CONNECTION *frontend,
									 POOL_CONNECTION_POOL *backend,
									 POOL_SENT_MESSAGE *message);
static int* find_victim_nodes(int *ntuples, int nmembers, int master_node, int *number_of_nodes);
static int extract_ntuples(char *message);
static POOL_STATUS close_standby_transactions(POOL_CONNECTION *frontend,
											  POOL_CONNECTION_POOL *backend);

/*
 * Process Query('Q') message
 * Query messages include an SQL string.
 */
POOL_STATUS SimpleQuery(POOL_CONNECTION *frontend,
						POOL_CONNECTION_POOL *backend, int len, char *contents)
{
	static char *sq_config = "pool_status";
	static char *sq_pools = "pool_pools";
	static char *sq_processes = "pool_processes";
 	static char *sq_nodes = "pool_nodes";
 	static char *sq_version = "pool_version";
 	static char *sq_cache = "pool_cache";
	int commit;
	List *parse_tree_list;
	Node *node = NULL;
	POOL_STATUS status;
	char *string;
	int lock_kind;
	bool is_likely_select = false;
	int specific_error = 0;

	POOL_SESSION_CONTEXT *session_context;
	POOL_QUERY_CONTEXT *query_context;
	POOL_MEMORY_POOL *old_context;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("SimpleQuery: cannot get session context");
		return POOL_END;
	}

	/* save last query string for logging purpose */
	strlcpy(query_string_buffer, contents, sizeof(query_string_buffer));

	/* show ps status */
	query_ps_status(contents, backend);

	/* log query to log file if necessary */
	if (pool_config->log_statement)
	{
		pool_log("statement: %s", contents);
	}
	else
	{
		pool_debug("statement2: %s", contents);
	}

	/*
	 * Fetch memory cache if possible
	 */
	is_likely_select = pool_is_likely_select(contents);

	/*
	 * If memory query cache enabled and the query seems to be a
	 * SELECT use query cache if possible. However if we are in an
	 * explicit transaction and we had writing query before, we should
	 * not use query cache. This means that even the writing query is
	 * not anything related to the table which is used the SELECT, we
	 * do not use cache. Of course we could analyze the SELECT to see
	 * if it uses the table modified in the transaction, but it will
	 * need parsing query and accessing to system catalog, which will
	 * add significant overhead. Moreover if we are in aborted 
	 * transaction, commands should be ignored, so we should not use
	 * query cache. 
	 */
	if (pool_config->memory_cache_enabled && is_likely_select &&
		!pool_is_writing_transaction() &&
		TSTATE(backend, MASTER_SLAVE ? PRIMARY_NODE_ID : REAL_MASTER_NODE_ID) != 'E')
	{
		bool foundp;

		/* If the query is SELECT from table to cache, try to fetch cached result. */
		status = pool_fetch_from_memory_cache(frontend, backend, contents, &foundp);

		if (status != POOL_CONTINUE)
			return status;

		if (foundp)
		{
			pool_ps_idle_display(backend);
			pool_set_skip_reading_from_backends();
			pool_stats_count_up_num_cache_hits();
			return POOL_CONTINUE;
		}
	}

	/* Create query context */
	query_context = pool_init_query_context();
	if (!query_context)
	{
		pool_error("SimpleQuery: pool_init_query_context failed");
		return POOL_END;
	}

	/* switch memory context */
	if (pool_memory == NULL)
		pool_memory = pool_memory_create(PARSER_BLOCK_SIZE);
	old_context = pool_memory_context_switch_to(query_context->memory_context);

	/* parse SQL string */
	parse_tree_list = raw_parser(contents);

	if (parse_tree_list == NIL)
	{
		/* is the query empty? */
		if (*contents == '\0' || *contents == ';')
		{
			/*
			 * JBoss sends empty queries for checking connections.
			 * We replace the empty query with SELECT command not
			 * to affect load balance.
			 * [Pgpool-general] Confused about JDBC and load balancing
			 */
			parse_tree_list = raw_parser(POOL_DUMMY_READ_QUERY);
		}
		else
		{
			/*
			 * Unable to parse the query. Probably syntax error or the
			 * query is too new and our parser cannot understand. Treat as
			 * if it were an DELETE command. Note that the DELETE command
			 * does not execute, instead the original query will be sent
			 * to backends, which may or may not cause an actual syntax errors.
			 * The command will be sent to all backends in replication mode
			 * or master/primary in master/slave mode.
			 */
			if (!strcmp(remote_host, "[local]"))
			{
				pool_log("SimpleQuery: Unable to parse the query: \"%s\" from local client", contents);
			}
			else
			{
				pool_log("SimpleQuery: Unable to parse the query: \"%s\" from client %s(%s)", contents, remote_host, remote_port);
			}
			parse_tree_list = raw_parser(POOL_DUMMY_WRITE_QUERY);
			query_context->is_parse_error = true;
		}
	}

	if (parse_tree_list != NIL)
	{
		/*
		 * XXX: Currently we only process the first element of the parse tree.
		 * rest of multiple statements are silently discarded.
		 */
		node = (Node *) lfirst(list_head(parse_tree_list));

		if (pool_config->enable_query_cache &&
			SYSDB_STATUS == CON_UP &&
			IsA(node, SelectStmt) &&
			!(is_select_pgcatalog = IsSelectpgcatalog(node, backend)))
		{
			/*
			 * POOL_CONTINUE of here represent cache found, and messages were
			 * sent to frontend already.
			 */
			if (pool_execute_query_cache_lookup(frontend, backend, node) == POOL_CONTINUE)
			{
				pool_query_context_destroy(query_context);
				pool_set_skip_reading_from_backends();
				pool_memory_context_switch_to(old_context);
				return POOL_CONTINUE;
			}
		}

		/*
		 * Start query context
		 */
		pool_start_query(query_context, contents, len, node);

		/*
		 * If the query is DROP DATABASE, after executing it, cache files directory must be discarded.
		 * So we have to get the DB's oid before it will be DROPped.
		 */
		if (pool_config->memory_cache_enabled && is_drop_database(node))
		{
			DropdbStmt *stmt = (DropdbStmt *)node;
			query_context->dboid = pool_get_database_oid_from_dbname(stmt->dbname);
			if (query_context->dboid != 0)
			{
				pool_debug("DB's oid to discard its cache directory: dboid = %d", query_context->dboid);
			}
		}

		/*
		 * Check if multi statement query
		 */
		if (parse_tree_list && list_length(parse_tree_list) > 1)
		{
			query_context->is_multi_statement = true;
		}
		else
		{
			query_context->is_multi_statement = false;
		}

		if (PARALLEL_MODE)
		{
			bool parallel = true;

			is_parallel_table = is_partition_table(backend,node);
			status = pool_do_parallel_query(frontend, backend, node, &parallel, &contents, &len);
			if (parallel)
			{
				pool_query_context_destroy(query_context);
				pool_memory_context_switch_to(old_context);
				return status;
			}
		}

		/* check COPY FROM STDIN
		 * if true, set copy_* variable
		 */
		check_copy_from_stdin(node);

		/* status reporting? */
		if (IsA(node, VariableShowStmt))
		{
			bool is_valid_show_command = false;
			VariableShowStmt *vnode = (VariableShowStmt *)node;

			if (!strcmp(sq_config, vnode->name))
            {
				is_valid_show_command = true;
                pool_debug("config reporting");
                config_reporting(frontend, backend);
            }
			else if (!strcmp(sq_pools, vnode->name))
            {
				is_valid_show_command = true;
                pool_debug("pools reporting");
                pools_reporting(frontend, backend);
            }
			else if (!strcmp(sq_processes, vnode->name))
            {
				is_valid_show_command = true;
                pool_debug("processes reporting");
                processes_reporting(frontend, backend);
            }
			else if (!strcmp(sq_nodes, vnode->name))
            {
				is_valid_show_command = true;
                pool_debug("nodes reporting");
                nodes_reporting(frontend, backend);
            }
			else if (!strcmp(sq_version, vnode->name))
            {
				is_valid_show_command = true;
                pool_debug("version reporting");
                version_reporting(frontend, backend);
            }
			else if (!strcmp(sq_cache, vnode->name))
            {
				is_valid_show_command = true;
                pool_debug("cache reporting");
                cache_reporting(frontend, backend);
            }

			if (is_valid_show_command)
			{
				pool_ps_idle_display(backend);
				pool_query_context_destroy(query_context);
				pool_set_skip_reading_from_backends();
				pool_memory_context_switch_to(old_context);
				return POOL_CONTINUE;
			}
		}

		/*
		 * If the table is to be cached, set is_cache_safe TRUE and register table oids.
		 */ 
		if (pool_config->memory_cache_enabled && query_context->is_multi_statement == false)
		{
			bool is_select_query = false;
			int num_oids;
			int *oids;
			int i;

			/* Check if the query is actually SELECT */
			if (is_likely_select && IsA(node, SelectStmt))
			{
				is_select_query = true;
			}

			/* Switch the flag of is_cache_safe in session_context */
			if (is_select_query && !query_context->is_parse_error &&
				pool_is_allow_to_cache(query_context->parse_tree,
									   query_context->original_query))
			{
				pool_set_cache_safe();
			}
			else
			{
				pool_unset_cache_safe();
			}

			/* If table is to be cached and the query is DML, save the table oid */
			if (!is_select_query && !query_context->is_parse_error)
			{
				num_oids = pool_extract_table_oids(node, &oids);

				if (num_oids > 0)
				{
					/* Save to oid buffer */
					for (i=0;i<num_oids;i++)
					{
						pool_add_dml_table_oid(oids[i]);
					}
				}
			}
		}

		/*
		 * Decide where to send query
		 */
		pool_where_to_send(query_context, query_context->original_query,
						   query_context->parse_tree);

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
			register_node_operation_request(CLOSE_IDLE_REQUEST, NULL, 0);

			/* we need to loop over here since we will get USR1 signal while sleeping */
			while (stime > 0)
			{
				stime = sleep(stime);
			}
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
		if (!RAW_MODE && !STREAM)
		{
			/*
			 * If there's only one node to send the command, there's no
			 * point to start a transaction.
			 */
			if (pool_multi_node_to_be_sent(query_context))
			{
				/* start a transaction if needed */
				if (start_internal_transaction(frontend, backend, (Node *)node) != POOL_CONTINUE)
					return POOL_END;

				/* check if need lock */
				lock_kind = need_insert_lock(backend, contents, node);
				if (lock_kind)
				{
					/* if so, issue lock command */
					status = insert_lock(frontend, backend, contents, (InsertStmt *)node, lock_kind);
					if (status != POOL_CONTINUE)
					{
						pool_query_context_destroy(query_context);
						return status;
					}
				}
			}
		}
		else if (REPLICATION && contents == NULL && start_internal_transaction(frontend, backend, node))
		{
			pool_query_context_destroy(query_context);
			return POOL_ERROR;
		}
	}

	if (MAJOR(backend) == PROTO_MAJOR_V2 && is_start_transaction_query(node))
	{
		int i;

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if(VALID_BACKEND(i))
				TSTATE(backend, i) = 'T';
		}
	}

	if (node)
	{
		POOL_SENT_MESSAGE *msg = NULL;

		if (IsA(node, PrepareStmt))
		{
			msg = pool_create_sent_message('Q', len, contents, 0,
										   ((PrepareStmt *)node)->name,
										   query_context);
			if (!msg)
			{
				pool_error("SimpleQuery: cannot create query message: %s", strerror(errno));
				return POOL_END;
			}
			session_context->uncompleted_message =  msg;
		}
	}

	string = query_context->original_query;

	if (!RAW_MODE)
	{
		/* check if query is "COMMIT" or "ROLLBACK" */
		commit = is_commit_or_rollback_query(node);

		/*
		 * Query is not commit/rollback
		 */
		if (!commit)
		{
			char *rewrite_query;

			if (node)
		   	{
				POOL_SENT_MESSAGE *msg = NULL;

				if (IsA(node, PrepareStmt))
				{
					msg = session_context->uncompleted_message;
				}
				else if (IsA(node, ExecuteStmt))
				{
					msg = pool_get_sent_message('Q', ((ExecuteStmt *)node)->name);
					if (!msg)
						msg = pool_get_sent_message('P', ((ExecuteStmt *)node)->name);
				}

				/* rewrite `now()' to timestamp literal */
				rewrite_query = rewrite_timestamp(backend, query_context->parse_tree, false, msg);

				/*
				 * If the query is BEGIN READ WRITE or
				 * BEGIN ... SERIALIZABLE in master/slave mode,
				 * we send BEGIN to slaves/standbys instead.
				 * original_query which is BEGIN READ WRITE is sent to primary.
				 * rewritten_query which is BEGIN is sent to standbys.
				 */
				if (pool_need_to_treat_as_if_default_transaction(query_context))
				{
					rewrite_query = pstrdup("BEGIN");
				}

				if (rewrite_query != NULL)
				{
					query_context->rewritten_query = rewrite_query;
					query_context->rewritten_length = strlen(rewrite_query) + 1;
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
				status = pool_send_and_wait(query_context, 0, 0);
				/* free_parser(); */
				return status;
			}

			/* Send the query to master node */
			if (pool_send_and_wait(query_context, 1, MASTER_NODE_ID) != POOL_CONTINUE)
			{
				pool_query_context_destroy(query_context);
				return POOL_END;
			}

			/* Check specific errors */
			specific_error = check_errors(backend, MASTER_NODE_ID);
			if (specific_error)
			{
				/* log error message */
				generate_error_message("SimpleQuery: ", specific_error, contents);
			}
		}

		if (specific_error)
		{
			char msg[1024] = POOL_ERROR_QUERY; /* large enough */
			int len = strlen(msg);

			memset(msg + len, 0, sizeof(int));

			/* send query to other nodes */
			query_context->rewritten_query = msg;
			query_context->rewritten_length = len;
			if (pool_send_and_wait(query_context, -1, MASTER_NODE_ID) != POOL_CONTINUE)
				return POOL_END;
		}
		else
		{
			/*
			 * Send the query to other than master node.
			 */
			if (pool_send_and_wait(query_context, -1, MASTER_NODE_ID) != POOL_CONTINUE)
			{
				pool_query_context_destroy(query_context);
				return POOL_END;
			}
		}

		/* Send "COMMIT" or "ROLLBACK" to only master node if query is "COMMIT" or "ROLLBACK" */
		if (commit)
		{
			if (pool_send_and_wait(query_context, 1, MASTER_NODE_ID) != POOL_CONTINUE)
			{
				pool_query_context_destroy(query_context);
				return POOL_END;
			}
		}
		/* free_parser(); */
	}
	else
	{
		if (pool_send_and_wait(query_context, 1, MASTER_NODE_ID) != POOL_CONTINUE)
		{
			pool_query_context_destroy(query_context);
			return POOL_END;
		}
		/* free_parser(); */
	}

	/* switch memory context */
	pool_memory_context_switch_to(old_context);

	return POOL_CONTINUE;
}

/*
 * process EXECUTE (V3 only)
 */
POOL_STATUS Execute(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend,
					int len, char *contents)
{
	int commit = 0;
	char *query = NULL;
	Node *node;
	int specific_error = 0;
	POOL_SESSION_CONTEXT *session_context;
	POOL_QUERY_CONTEXT *query_context;
	POOL_SENT_MESSAGE *bind_msg;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("Execute: cannot get session context");
		return POOL_END;
	}

	pool_debug("Execute: portal name <%s>", contents);

	bind_msg = pool_get_sent_message('B', contents);
	if (!bind_msg)
	{
		pool_error("Execute: cannot get bind message");
		return POOL_END;
	}
	if(!bind_msg->query_context)
	{
		pool_error("Execute: cannot get query context");
		return POOL_END;
	}
	if (!bind_msg->query_context->original_query)
	{
		pool_error("Execute: cannot get original query");
		return POOL_END;
	}
	if (!bind_msg->query_context->parse_tree)
	{
		pool_error("Execute: cannot get parse tree");
		return POOL_END;
	}

	query_context = bind_msg->query_context;
	node = bind_msg->query_context->parse_tree;
	query = bind_msg->query_context->original_query;

	strlcpy(query_string_buffer, query, sizeof(query_string_buffer));

	pool_debug("Execute: query string = <%s>", query);

	/*
	 * Fetch memory cache if possible
	 */
	if (pool_config->memory_cache_enabled && pool_is_likely_select(query) &&
		!pool_is_writing_transaction() &&
		(TSTATE(backend, MASTER_SLAVE ? PRIMARY_NODE_ID : REAL_MASTER_NODE_ID) != 'E'))
	{
		bool foundp;
		POOL_STATUS status;
		char *search_query = NULL;
		int len;
		char *tmp;
#define STR_ALLOC_SIZE 1024

		len = strlen(query)+1;
		search_query = (char *)malloc(len);
		if (search_query == NULL)
		{
			pool_error("Execute: malloc failed");
			return POOL_END;
		}
		strcpy(search_query, query);

		/*
		 * Add bind message's info to query to search.
		 */
		if (query_context->is_cache_safe && bind_msg->param_offset && bind_msg->contents)
		{
			/* Extract binary contents from bind message */
			char *query_in_bind_msg = bind_msg->contents + bind_msg->param_offset;
			char hex_str[4];  /* 02X chars + white space + null end */
			int i;
			int alloc_len;

			alloc_len = (len/STR_ALLOC_SIZE+1)*STR_ALLOC_SIZE;
			search_query = realloc(search_query, alloc_len);
			if (search_query == NULL)
			{
				pool_error("Execute: realloc failed");
				return POOL_END;
			}

			for (i = 0; i < bind_msg->len - bind_msg->param_offset; i++)
			{
				int hexlen;

				snprintf(hex_str, sizeof(hex_str), (i == 0) ? " %02X" : "%02X", 0xff & query_in_bind_msg[i]);
				hexlen = strlen(hex_str);

				if ((len+hexlen) >= alloc_len)
				{
					alloc_len += STR_ALLOC_SIZE;
					search_query = realloc(search_query, alloc_len);
					if (search_query == NULL)
					{
						pool_error("Execute: realloc failed");
						return POOL_END;
					}
				}
				strcat(search_query, hex_str);
				len += hexlen;
			}

			query_context->query_w_hex = search_query;

			/*
			 * When a transaction is committed, query_context->temp_cache->query is used
			 * to create md5 hash to search for query cache.
			 * So overwrite the query text in temp cache to the one with the hex of bind message.
			 * If not, md5 hash will be created by the query text without bind message, and
			 * it will happen to find cache never or to get a wrong result.
			 * 
			 * However, It is possible that temp_cache does not exist.
			 * Consider following scenario:
			 * - In the previous execute cache is overflowed, and
			 *   temp_cache discarded.
			 * - In the subsequent bind/execute uses the same portal
			 */
			if (query_context->temp_cache)
			{
				tmp = (char *)malloc(sizeof(char) * strlen(search_query) + 1);
				if (tmp == NULL)
				{
					pool_error("Execute: malloc failed");
					return POOL_END;
				}
				free(query_context->temp_cache->query);
				query_context->temp_cache->query = tmp;
				strcpy(query_context->temp_cache->query, search_query);
			}
		}

		/* If the query is SELECT from table to cache, try to fetch cached result. */
		status = pool_fetch_from_memory_cache(frontend, backend, search_query, &foundp);

		if (status != POOL_CONTINUE)
			return status;

		if (foundp)
		{
			pool_ps_idle_display(backend);
			pool_set_skip_reading_from_backends();
			pool_stats_count_up_num_cache_hits();
			pool_unset_query_in_progress();
			return POOL_CONTINUE;
		}
	}

	session_context->query_context = query_context;
	/*
	 * Calling pool_where_to_send here is dangerous because the node
	 * parse/bind has been sent could be change by
	 * pool_where_to_send() and it leads to "portal not found"
	 * etc. errors.
	 */

	/* check if query is "COMMIT" or "ROLLBACK" */
	commit = is_commit_or_rollback_query(node);

	if (REPLICATION || PARALLEL_MODE)
	{
		/*
		 * Query is not commit/rollback
		 */
		if (!commit)
		{
			/* Send the query to master node */
			if (pool_extended_send_and_wait(query_context, "E", len, contents, 1, MASTER_NODE_ID) != POOL_CONTINUE)
			{
				return POOL_END;
			}

			/* Check specific errors */
			specific_error = check_errors(backend, MASTER_NODE_ID);
			if (specific_error)
			{
				/* log error message */
				generate_error_message("Execute: ", specific_error, contents);
			}
		}

		if (specific_error)
		{
			char msg[1024] = "pgpool_error_portal"; /* large enough */
			int len = strlen(msg);

			memset(msg + len, 0, sizeof(int));

			/* send query to other nodes */
			if (pool_extended_send_and_wait(query_context, "E", len, msg, -1, MASTER_NODE_ID) != POOL_CONTINUE)
				return POOL_END;
		}
		else
		{
			if (pool_extended_send_and_wait(query_context, "E", len, contents, -1, MASTER_NODE_ID) != POOL_CONTINUE)
				return POOL_END;
		}
		
		/* send "COMMIT" or "ROLLBACK" to only master node if query is "COMMIT" or "ROLLBACK" */
		if (commit)
		{
			if (pool_extended_send_and_wait(query_context, "E", len, contents, 1, MASTER_NODE_ID) != POOL_CONTINUE)
			{
				return POOL_END;
			}
		}
	}
	else
	{
		if (pool_extended_send_and_wait(query_context, "E", len, contents, 1, MASTER_NODE_ID) != POOL_CONTINUE)
		{
			return POOL_END;
		}
		if (pool_extended_send_and_wait(query_context, "E", len, contents, -1, MASTER_NODE_ID) != POOL_CONTINUE)
		{
			return POOL_END;
		}
	}

	return POOL_CONTINUE;
}

/*
 * process Parse (V3 only)
 */
POOL_STATUS Parse(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend,
				  int len, char *contents)
{
	int deadlock_detected = 0;
	int insert_stmt_with_lock = 0;
	char *name;
	char *stmt;
	List *parse_tree_list;
	Node *node = NULL;
	POOL_SENT_MESSAGE *msg;
	POOL_STATUS status;
	POOL_MEMORY_POOL *old_context;
	POOL_SESSION_CONTEXT *session_context;
	POOL_QUERY_CONTEXT *query_context;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("Parse: cannot get session context");
		return POOL_END;
	}

	/* Create query context */
	query_context = pool_init_query_context();

	pool_debug("Parse: statement name <%s>", contents);

	name = contents;
	stmt = contents + strlen(contents) + 1;

	/* switch memory context */
	if (pool_memory == NULL)
		pool_memory = pool_memory_create(PARSER_BLOCK_SIZE);
	old_context = pool_memory_context_switch_to(query_context->memory_context);

	/* parse SQL string */
	parse_tree_list = raw_parser(stmt);

	if (parse_tree_list == NIL)
	{
		/* is the query empty? */
		if (*stmt == '\0' || *stmt == ';')
		{
			/*
			 * JBoss sends empty queries for checking connections.
			 * We replace the empty query with SELECT command not
			 * to affect load balance.
			 * [Pgpool-general] Confused about JDBC and load balancing
			 */
			parse_tree_list = raw_parser(POOL_DUMMY_READ_QUERY);
		}
		else
		{
			/*
			 * Unable to parse the query. Probably syntax error or the
			 * query is too new and our parser cannot understand. Treat as
			 * if it were an DELETE command. Note that the DELETE command
			 * does not execute, instead the original query will be sent
			 * to backends, which may or may not cause an actual syntax errors.
			 * The command will be sent to all backends in replication mode
			 * or master/primary in master/slave mode.
			 */
			if (!strcmp(remote_host, "[local]"))
			{
				pool_log("Parse: Unable to parse the query: \"%s\" from local client", stmt);
			}
			else
			{
				pool_log("Parse: Unable to parse the query: \"%s\" from client %s(%s)", stmt, remote_host, remote_port);
			}
			parse_tree_list = raw_parser(POOL_DUMMY_WRITE_QUERY);
			query_context->is_parse_error = true;
		}
	}

	if (parse_tree_list != NIL)
	{
		/* Save last query string for logging purpose */
		snprintf(query_string_buffer, sizeof(query_string_buffer), "Parse: %s", stmt);

		node = (Node *) lfirst(list_head(parse_tree_list));

		/* If replication mode, check to see what kind of insert lock is
		 * neccessary.
		 */
		if (REPLICATION)
			insert_stmt_with_lock = need_insert_lock(backend, stmt, node);

		/*
		 * Start query context
		 */
		pool_start_query(query_context, pstrdup(stmt), strlen(stmt) + 1, node);

		msg = pool_create_sent_message('P', len, contents, 0, name, query_context);
		if (!msg)
		{
			pool_error("Parse: cannot create parse message: %s", strerror(errno));
			return POOL_END;
		}

		session_context->uncompleted_message = msg;

		/*
		 * If the table is to be cached, set is_cache_safe TRUE and register table oids.
		 */
		if (pool_config->memory_cache_enabled)
		{
			bool is_likely_select = false;
			bool is_select_query = false;
			int num_oids;
			int *oids;
			int i;

			/* Check if the query is actually SELECT */
			is_likely_select = pool_is_likely_select(query_context->original_query);
			if (is_likely_select && IsA(node, SelectStmt))
			{
				is_select_query = true;
			}

			/* Switch the flag of is_cache_safe in session_context */
			if (is_select_query && !query_context->is_parse_error &&
				pool_is_allow_to_cache(query_context->parse_tree,
									   query_context->original_query))
			{
				pool_set_cache_safe();
			}
			else
			{
				pool_unset_cache_safe();
			}

			/* If table is to be cached and the query is DML, save the table oid */
			if (!is_select_query && !query_context->is_parse_error)
			{
				num_oids = pool_extract_table_oids(node, &oids);

				if (num_oids > 0)
				{
					/* Save to oid buffer */
					for (i=0;i<num_oids;i++)
					{
						pool_add_dml_table_oid(oids[i]);
					}
				}
			}
		}

		/*
		 * Decide where to send query
		 */
		pool_where_to_send(query_context, query_context->original_query,
						   query_context->parse_tree);

		if (REPLICATION)
		{
			char *rewrite_query;
			bool rewrite_to_params = true;

			/*
			 * rewrite `now()'.
			 * if stmt is unnamed, we rewrite `now()' to timestamp constant.
			 * else we rewrite `now()' to params and expand that at Bind
			 * message.
			 */
			if (*name == '\0')
				rewrite_to_params = false;
			msg->num_tsparams = 0;
			rewrite_query = rewrite_timestamp(backend, node, rewrite_to_params, msg);
			if (rewrite_query != NULL)
			{
				int alloc_len = len - strlen(stmt) + strlen(rewrite_query);
				contents = pool_memory_realloc(session_context->memory_context,
                                                                      msg->contents, alloc_len);
				strcpy(contents, name);
				strcpy(contents + strlen(name) + 1, rewrite_query);
				memcpy(contents + strlen(name) + strlen(rewrite_query) + 2,
					   stmt + strlen(stmt) + 1,
					   len - (strlen(name) + strlen(stmt) + 2));

				len = alloc_len;
				name = contents;
				stmt = contents + strlen(name) + 1;
				pool_debug("Parse: rewrite query  %s %s len=%d", name, stmt, len);

				msg->len = len;
				msg->contents = contents;

				query_context->rewritten_query = rewrite_query;
			}
		}

		/*
		 * If the query is BEGIN READ WRITE in master/slave mode,
		 * we send BEGIN instead of it to slaves/standbys.
		 * original_query which is BEGIN READ WRITE is sent to primary.
		 * rewritten_query which is BEGIN is sent to standbys.
		 */
		if (is_start_transaction_query(query_context->parse_tree) &&
			is_read_write((TransactionStmt *)query_context->parse_tree) &&
			MASTER_SLAVE)
		{
			query_context->rewritten_query = pstrdup("BEGIN");
		}
	}

	pool_memory_context_switch_to(old_context);

	/*
	 * If in replication mode, send "SYNC" message if not in a transaction.
	 */
	if (REPLICATION)
	{
		char kind;

		if (TSTATE(backend, MASTER_NODE_ID) != 'T')
		{
			int i;

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
				pool_query_context_destroy(query_context);
				return POOL_END;
			}

			/*
			 * SYNC message returns "Ready for Query" message.
			 */
			if (ReadyForQuery(frontend, backend, 0, false) != POOL_CONTINUE)
			{
				pool_query_context_destroy(query_context);
				return POOL_END;
			}

			/*
			 * set in_progress flag, because ReadyForQuery unset it.
			 * in_progress flag influences VALID_BACKEND.
			 */
			if (!pool_is_query_in_progress())
				pool_set_query_in_progress();
		}

		if (is_strict_query(query_context->parse_tree))
		{
			start_internal_transaction(frontend, backend, query_context->parse_tree);
			allow_close_transaction = 1;
		}

		if (insert_stmt_with_lock)
		{
			/* start a transaction if needed and lock the table */
			status = insert_lock(frontend, backend, stmt, (InsertStmt *)query_context->parse_tree, insert_stmt_with_lock);
			if (status != POOL_CONTINUE)
			{
				pool_query_context_destroy(query_context);
				return status;
			}
		}
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
		if (pool_extended_send_and_wait(query_context, "P", len, contents, 1, MASTER_NODE_ID) != POOL_CONTINUE)
		{
			pool_query_context_destroy(query_context);
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
			pool_query_context_destroy(query_context);
			return POOL_END;
		}
		else
		{
			/*
			 * Check if other than deadlock error detected.  If so, emit
			 * log. This is useful when invalid encoding error occurs. In
			 * this case, PostgreSQL does not report what statement caused
			 * that error and make users confused.
			 */
			per_node_error_log(backend, MASTER_NODE_ID, stmt, "Parse: Error or notice message from backend: ", true);
		}

		if (deadlock_detected)
		{
			POOL_QUERY_CONTEXT *error_qc;

			error_qc = pool_init_query_context();
			pool_start_query(error_qc, POOL_ERROR_QUERY, strlen(POOL_ERROR_QUERY) + 1, node);
			pool_copy_prep_where(query_context->where_to_send, error_qc->where_to_send);
			
			pool_log("Parse: received deadlock error message from master node");

			if (pool_send_and_wait(error_qc, -1, MASTER_NODE_ID) != POOL_CONTINUE)
			{
				pool_query_context_destroy(query_context);
				return POOL_END;
			}

			pool_query_context_destroy(error_qc);
			pool_set_query_in_progress();
			session_context->query_context = query_context;
		}
		else
		{
			if (pool_extended_send_and_wait(query_context, "P", len, contents, -1, MASTER_NODE_ID) != POOL_CONTINUE)
			{
				pool_query_context_destroy(query_context);
				return POOL_END;
			}
		}
	}
	else
	{
		if (pool_extended_send_and_wait(query_context, "P", len, contents, 1, MASTER_NODE_ID) != POOL_CONTINUE)
		{
			pool_query_context_destroy(query_context);
			return POOL_END;
		}
	}

	/*
	 * Ok. we are safe to call free_parser();
	 */
	/* free_parser(); */

	return POOL_CONTINUE;

}

POOL_STATUS Bind(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend,
				 int len, char *contents)
{
	char *pstmt_name;
	char *portal_name;
	char *rewrite_msg = NULL;
	POOL_SENT_MESSAGE *parse_msg;
	POOL_SENT_MESSAGE *bind_msg;
	POOL_SESSION_CONTEXT *session_context;
	POOL_QUERY_CONTEXT *query_context;
	int insert_stmt_with_lock = 0;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("Bind: cannot get session context");
		return POOL_END;
	}

	/*
	 * Rewrite message
	 */
	portal_name = contents;
	pstmt_name = contents + strlen(portal_name) + 1;

	parse_msg = pool_get_sent_message('Q', pstmt_name);
	if (!parse_msg)
		parse_msg = pool_get_sent_message('P', pstmt_name);
	if (!parse_msg)
	{
		pool_error("Bind: cannot get parse message \"%s\"", pstmt_name);
		return POOL_END;
	}

	bind_msg = pool_create_sent_message('B', len, contents,
										parse_msg->num_tsparams, portal_name,
										parse_msg->query_context);
	if (!bind_msg)
	{
		pool_error("Bind: cannot create bind message: %s", strerror(errno));
		return POOL_END;
	}

	query_context = parse_msg->query_context;
	if (!query_context)
	{
		pool_error("Bind: cannot get query context");
		return POOL_END;
	}

	/*
	 * If the query can be cached, save its offset of query text in bind message's content.
	 */
	if (query_context->is_cache_safe)
	{
		bind_msg->param_offset = sizeof(char) * (strlen(portal_name) + strlen(pstmt_name) + 2);
	}

	session_context->uncompleted_message = bind_msg;

	/* rewrite bind message */
	if (REPLICATION && bind_msg->num_tsparams > 0)
	{
		rewrite_msg = bind_rewrite_timestamp(backend, bind_msg, contents, &len);
		if (rewrite_msg != NULL)
			contents = rewrite_msg;
	}

	session_context->query_context = query_context;

	if (pool_config->load_balance_mode && pool_is_writing_transaction())
	{
		pool_where_to_send(query_context, query_context->original_query,
						   query_context->parse_tree);

		if (parse_before_bind(frontend, backend, parse_msg) != POOL_CONTINUE)
			return POOL_END;
	}

	/*
	 * Start a transaction if necessary in replication mode
	 */
	if (REPLICATION)
	{
		pool_debug("Bind: checking strict query");
		if (is_strict_query(query_context->parse_tree))
		{
			pool_debug("Bind: strict query");
			start_internal_transaction(frontend, backend, query_context->parse_tree);
			allow_close_transaction = 1;
		}

		pool_debug("Bind: checking insert lock");
		insert_stmt_with_lock = need_insert_lock(backend, query_context->original_query, query_context->parse_tree);
		if (insert_stmt_with_lock)
		{
			pool_debug("Bind: issuing insert lock");
			/* issue a LOCK command to keep consistency */
			if (insert_lock(frontend, backend, query_context->original_query, (InsertStmt *)query_context->parse_tree, insert_stmt_with_lock) != POOL_CONTINUE)
			{
				pool_query_context_destroy(query_context);
				return POOL_END;
			}
		}
	}

	pool_debug("Bind: waiting for master completing the query");
	if (pool_extended_send_and_wait(query_context, "B", len, contents, 1, MASTER_NODE_ID)
		!= POOL_CONTINUE)
	{
		return POOL_END;
	}

	if (pool_extended_send_and_wait(query_context, "B", len, contents, -1, MASTER_NODE_ID)
		!= POOL_CONTINUE)
	{
		return POOL_END;
	}
    if(rewrite_msg)
        free(rewrite_msg);

	return POOL_CONTINUE;
}

POOL_STATUS Describe(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend,
							int len, char *contents)
{
	POOL_SENT_MESSAGE *msg;
	POOL_SESSION_CONTEXT *session_context;
	POOL_QUERY_CONTEXT *query_context;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("Describe: cannot get session context");
		return POOL_END;
	}

	/* Prepared Statement */
	if (*contents == 'S')
	{
		msg = pool_get_sent_message('Q', contents+1);
		if (!msg)
			msg = pool_get_sent_message('P', contents+1);
		if (!msg)
		{
			pool_error("Describe: cannot get parse message");
			return POOL_END;
		}
	}
	/* Portal */
	else
	{
		msg = pool_get_sent_message('B', contents+1);
		if (!msg)
		{
			pool_error("Describe: cannot get bind message");
			return POOL_END;
		}
	}

	query_context = msg->query_context;

	if (query_context == NULL)
	{
		pool_error("Describe: cannot get query context");
		return POOL_END;
	}

	session_context->query_context = query_context;

	/*
	 * Calling pool_where_to_send here is dangerous because the node
	 * parse/bind has been sent could be change by
	 * pool_where_to_send() and it leads to "portal not found"
	 * etc. errors.
	 */
	pool_debug("Describe: waiting for master completing the query");
	if (pool_extended_send_and_wait(query_context, "D", len, contents, 1, MASTER_NODE_ID)
		!= POOL_CONTINUE)
		return POOL_END;

	if (pool_extended_send_and_wait(query_context, "D", len, contents, -1, MASTER_NODE_ID)
		!= POOL_CONTINUE)
		return POOL_END;

	return POOL_CONTINUE;
}


POOL_STATUS Close(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend,
				  int len, char *contents)
{
	POOL_SENT_MESSAGE *msg;
	POOL_SESSION_CONTEXT *session_context;
	POOL_QUERY_CONTEXT *query_context;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("Close: cannot get session context");
		return POOL_END;
	}

	/* Prepared Statement */
	if (*contents == 'S')
	{
		msg = pool_get_sent_message('Q', contents+1);
		if (!msg)
			msg = pool_get_sent_message('P', contents+1);

		if (!msg)
			pool_debug("Close: cannot get parse message");
	}
	/* Portal */
	else if (*contents == 'P')
	{
		msg = pool_get_sent_message('B', contents+1);
		if (!msg)
			pool_debug("Close: cannot get bind message");
	}
	else
	{
		pool_error("Close: invalid message");
		return POOL_END;
	}

	/*
	 * As per the postgresql, calling close on non existing portals is not
	 * an error. So on the same footings we will ignore all such calls and
	 * return the close complete message to clients with out going to backend
	 */
	if (!msg)
	{
		int len = htonl(sizeof(len));
		pool_set_command_success();
		pool_unset_query_in_progress();

		pool_write(frontend, "3", 1);
		pool_write_and_flush(frontend, &len, sizeof(len));

		return POOL_CONTINUE;
	}

	session_context->uncompleted_message = msg;
	query_context = msg->query_context;

	if (!query_context)
	{
		pool_error("Close: cannot get query context");
		return POOL_END;
	}

	session_context->query_context = query_context;
	/* pool_where_to_send(query_context, query_context->original_query, query_context->parse_tree); */

	pool_debug("Close: waiting for master completing the query");
	if (pool_extended_send_and_wait(query_context, "C", len, contents, 1, MASTER_NODE_ID)
		!= POOL_CONTINUE)
		return POOL_END;

	if (pool_extended_send_and_wait(query_context, "C", len, contents, -1, MASTER_NODE_ID)
		!= POOL_CONTINUE)
		return POOL_END;

	return POOL_CONTINUE;
}


POOL_STATUS FunctionCall3(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend,
						  int len, char *contents)
{
	/*
	 * If Function call message for lo_creat, rewrite it
	 */
	char *rewrite_lo;
	int rewrite_len;

	rewrite_lo = pool_rewrite_lo_creat('F', contents, len, frontend,
									   backend, &rewrite_len);

	if (rewrite_lo != NULL)
	{
		contents = rewrite_lo;
		len = rewrite_len;
	}
	return  SimpleForwardToBackend('F', frontend, backend, len, contents);
}

/*
 * Process ReadyForQuery('Z') message.
 * If send_ready is true, send 'Z' message to frontend.
 * If cache_commit is true, commit or discard query cache according to
 * transaction state.
 *
 * - if the error status "mismatch_ntuples" is set, send an error query
 *	 to all DB nodes to abort transaction or do failover.
 * - internal transaction is closed
 */
POOL_STATUS ReadyForQuery(POOL_CONNECTION *frontend,
						  POOL_CONNECTION_POOL *backend, bool send_ready, bool cache_commit)
{
	int i;
	int len;
	signed char kind;
	signed char state = 0;
	POOL_SESSION_CONTEXT *session_context;
	Node *node = NULL;
	char *query = NULL;
	bool got_estate = false;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("ReadyForQuery: cannot get session context");
		return POOL_END;
	}

	/*
	 * If the numbers of update tuples are differ and
	 * failover_if_affected_tuples_mismatch is false, we abort
	 * transactions by using do_error_command.  If
	 * failover_if_affected_tuples_mismatch is true, trigger failover.
	 * This only works with PROTO_MAJOR_V3.
	 */
	if (session_context->mismatch_ntuples && MAJOR(backend) == PROTO_MAJOR_V3)
	{
		int i;
		char kind;

		/*
		 * If failover_if_affected_tuples_mismatch, is true, then
		 * decide victim nodes by using find_victim_nodes and
		 * degenerate them.
		 */
		if (pool_config->failover_if_affected_tuples_mismatch)
		{
			int *victim_nodes;
			int number_of_nodes;
			char msgbuf[128];

			victim_nodes = find_victim_nodes(session_context->ntuples, NUM_BACKENDS,
											 MASTER_NODE_ID, &number_of_nodes);
			if (victim_nodes)
			{
				int i;
				String *msg;
				POOL_MEMORY_POOL *old_context;

				if (session_context->query_context)
					old_context = pool_memory_context_switch_to(session_context->query_context->memory_context);
				else
					old_context = pool_memory_context_switch_to(session_context->memory_context);

				msg = init_string("ReadyForQuery: Degenerate backends:");

				for (i=0;i<number_of_nodes;i++)
				{
					snprintf(msgbuf, sizeof(msgbuf), " %d", victim_nodes[i]);
					string_append_char(msg, msgbuf);
				}
				pool_log("%s", msg->data);
				free_string(msg);

				msg = init_string("ReadyForQuery: Number of affected tuples are:");

				for (i=0;i<NUM_BACKENDS;i++)
				{
					snprintf(msgbuf, sizeof(msgbuf), " %d", session_context->ntuples[i]);
					string_append_char(msg, msgbuf);
				}
				pool_log("%s", msg->data);
				free_string(msg);

				pool_memory_context_switch_to(old_context);

				degenerate_backend_set(victim_nodes, number_of_nodes);
				child_exit(1);
			}
			else
			{
				pool_error("ReadyForQuery: find_victim_nodes returned no victim node");
			}
		}

		/*
		 * XXX: discard rest of ReadyForQuery packet
		 */
		if (pool_read_message_length(backend) < 0)
			return POOL_END;

		state = pool_read_kind(backend);
		if (state < 0)
			return POOL_END;

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
		session_context->mismatch_ntuples = false;
	}

	/*
	 * if a transaction is started for insert lock, we need to close
	 * the transaction.
	 */
	/* if (pool_is_query_in_progress() && allow_close_transaction) */
	if (allow_close_transaction)
	{
		if (end_internal_transaction(frontend, backend) != POOL_CONTINUE)
			return POOL_END;
	}

	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		if ((len = pool_read_message_length(backend)) < 0)
			return POOL_END;

		/*
		 * Set transaction state for each node
		 */
		state = TSTATE(backend,
					   MASTER_SLAVE ? PRIMARY_NODE_ID : REAL_MASTER_NODE_ID);

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i))
				continue;

			if (pool_read(CONNECTION(backend, i), &kind, sizeof(kind)))
				return POOL_END;

			TSTATE(backend, i) = kind;

			pool_debug("ReadyForQuery: transaction state:%c", state);

			/*
			 * The transaction state to be returned to frontend is
			 * master's.
			 */
			if (i == (MASTER_SLAVE ? PRIMARY_NODE_ID : REAL_MASTER_NODE_ID))
			{
				state = kind;
			}
			/*
			 * However, if the state is 'E', then frontend should had been
			 * already reported an ERROR. So, to match with that, let be the
			 * state to be returned to frontend.
			 */
			if (kind == 'E')
				got_estate = true;
		}
	}

	if (send_ready)
	{
		pool_write(frontend, "Z", 1);

		if (MAJOR(backend) == PROTO_MAJOR_V3)
		{
			len = htonl(len);
			pool_write(frontend, &len, sizeof(len));
			if (got_estate)
				state = 'E';
			pool_write(frontend, &state, 1);
		}
		pool_flush(frontend);
	}

	if (pool_is_query_in_progress())
	{
		node = pool_get_parse_tree();

		if (pool_is_command_success())
		{
			query = pool_get_query_string();

			if (node)
			{
				/*
				 * If the query was BEGIN/START TRANSACTION, clear the
				 * history that we had a writing command in the transaction
				 * and forget the transaction isolation level.
				 *
				 * XXX If BEGIN is received while we are already in an
				 * explicit transaction, the command *successes*
				 * (just with a NOTICE message). In this case we lose
				 * "writing_transaction" etc. info.
				 */
				if (is_start_transaction_query(node))
				{
					pool_unset_writing_transaction();
					pool_unset_failed_transaction();
					pool_unset_transaction_isolation();
				}

				/*
				 * If the query was COMMIT/ABORT, clear the history
				 * that we had a writing command in the transaction
				 * and forget the transaction isolation level.  This
				 * is necessary if succeeding transaction is not an
				 * explicit one.
				 */
				else if (is_commit_or_rollback_query(node))
				{
					pool_unset_writing_transaction();
					pool_unset_failed_transaction();
					pool_unset_transaction_isolation();
				}

				/*
				 * SET TRANSACTION ISOLATION LEVEL SERIALIZABLE or SET
				 * SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL
				 * SERIALIZABLE, remember it.
				 */
				else if (is_set_transaction_serializable(node))
				{
					pool_set_transaction_isolation(POOL_SERIALIZABLE);
				}

				/*
				 * If 2PC commands has been executed, automatically close
				 * transactions on standbys if there is any open
				 * transaction since 2PC commands close transaction on
				 * primary.
				 */
				else if (is_2pc_transaction_query(node))
				{
					if (close_standby_transactions(frontend, backend) != POOL_CONTINUE)
						return POOL_END;
				}

				else if (!is_select_query(node, query))
				{
					/*
					 * If the query was not READ SELECT, and we are in an
					 * explicit transaction, remember that we had a write
					 * query in this transaction.
					 */
					if (TSTATE(backend, MASTER_SLAVE ? PRIMARY_NODE_ID : REAL_MASTER_NODE_ID) == 'T')
					{
						/* However, if the query is "SET TRANSACTION READ ONLY" or its variant,
						 * don't set it.
						 */
						if (!pool_is_transaction_read_only(node))
						{
							pool_debug("not SET TRANSACTION READ ONLY");
							pool_set_writing_transaction();
						}
					}

					/*
					 * If the query was CREATE TEMP TABLE, discard
					 * temp table relcache because we might have had
					 * persistent table relation cache which has table
					 * name as the temp table.
					 */
					if (IsA(node, CreateStmt))
					{
						CreateStmt *create_table_stmt = (CreateStmt *)node;
						if (create_table_stmt->relation->relpersistence)
							discard_temp_table_relcache();
					}
				}
			}

			/* Memory cache enabled? */
			if (cache_commit && pool_config->memory_cache_enabled)
			{
				/* If we are doing extended query and the state is after EXECUTE,
				 * then we can commit cache.
				 * We check latter condition by looking at query_context->query_w_hex.
				 * This check is necessary for certain frame work such as PHP PDO.
				 * It sends Sync message right after PARSE and it produces
				 * "Ready for query" message from backend.
				 */
				if (pool_is_doing_extended_query_message())
				{
					if (session_context->query_context &&
						session_context->query_context->query_state[MASTER_NODE_ID] == POOL_EXECUTE_COMPLETE)
					{
						pool_handle_query_cache(backend, session_context->query_context->query_w_hex, node, state);
						free(session_context->query_context->query_w_hex);
						session_context->query_context->query_w_hex = NULL;
					}
				}
				else
				{
					if (MAJOR(backend) != PROTO_MAJOR_V3)
					{
						state = 'I';	/* XXX I don't think query cache works with PROTO2 protocol */
					}
					pool_handle_query_cache(backend, query, node, state);
				}
			}
		}
		/*
		 * If PREPARE or extended query protocol commands caused error,
		 * remove the temporary saved message.
		 * (except when ReadyForQuery() is called during Parse() of extended queries)
		 */
		else
		{
			if ((pool_is_doing_extended_query_message() &&
				 session_context->query_context->query_state[MASTER_NODE_ID] != POOL_UNPARSED &&
			     session_context->uncompleted_message) ||
			    (!pool_is_doing_extended_query_message() && session_context->uncompleted_message))
			{
				pool_add_sent_message(session_context->uncompleted_message);
				pool_remove_sent_message(session_context->uncompleted_message->kind,
										 session_context->uncompleted_message->name);
				session_context->uncompleted_message = NULL;
			}
		}

		pool_unset_query_in_progress();
	}

	if (!pool_is_doing_extended_query_message())
	{
		if (!(node && IsA(node, PrepareStmt)))
			pool_query_context_destroy(pool_get_session_context()->query_context);
	}

	/*
	 * Show ps idle status
	 */
	pool_ps_idle_display(backend);

	return POOL_CONTINUE;
}

/*
 * Close running transactions on standbys.
 */
static POOL_STATUS close_standby_transactions(POOL_CONNECTION *frontend,
											  POOL_CONNECTION_POOL *backend)
{
	int i;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (CONNECTION_SLOT(backend, i) &&
			TSTATE(backend, i) == 'T' &&
			BACKEND_INFO(i).backend_status == CON_UP &&
			(MASTER_SLAVE ? PRIMARY_NODE_ID : REAL_MASTER_NODE_ID) != i)
		{
			per_node_statement_log(backend, i, "COMMIT");
			if (do_command(frontend, CONNECTION(backend, i), "COMMIT", MAJOR(backend), 
						   MASTER_CONNECTION(backend)->pid,
						   MASTER_CONNECTION(backend)->key, 0) != POOL_CONTINUE)
			{
				return POOL_END;
			}
		}
	}
	return POOL_CONTINUE;
}

POOL_STATUS ParseComplete(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	POOL_SESSION_CONTEXT *session_context;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("ParseComplete: cannot get session context");
		return POOL_END;
	}

	if (session_context->uncompleted_message)
	{
		POOL_QUERY_CONTEXT *qc;

		pool_add_sent_message(session_context->uncompleted_message);

		qc = session_context->uncompleted_message->query_context;
		if (qc)
			pool_set_query_state(qc, POOL_PARSE_COMPLETE);

		session_context->uncompleted_message = NULL;
	}

	return SimpleForwardToFrontend('1', frontend, backend);
}

POOL_STATUS BindComplete(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	POOL_SESSION_CONTEXT *session_context;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("BindComplete: cannot get session context");
		return POOL_END;
	}

	if (session_context->uncompleted_message)
	{
		POOL_QUERY_CONTEXT *qc;

		pool_add_sent_message(session_context->uncompleted_message);

		qc = session_context->uncompleted_message->query_context;
		if (qc)
			pool_set_query_state(qc, POOL_BIND_COMPLETE);

		session_context->uncompleted_message = NULL;
	}

	return SimpleForwardToFrontend('2', frontend, backend);
}

POOL_STATUS CloseComplete(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	POOL_SESSION_CONTEXT *session_context;
	POOL_STATUS status;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("CloseComplete: cannot get session context");
		return POOL_END;
	}

	/* Send CloseComplete(3) to frontend before removing the target message */
	status = SimpleForwardToFrontend('3', frontend, backend);

	/* Remove the target message */
	if (session_context->uncompleted_message)
	{
		pool_remove_sent_message(session_context->uncompleted_message->kind,
								 session_context->uncompleted_message->name);
		session_context->uncompleted_message = NULL;
	}
	else
	{
		pool_error("CloseComplete: uncompleted message not found");
		return POOL_END;
	}

	return status;
}

POOL_STATUS CommandComplete(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	int i;
	int len, len1, sendlen;
	int status;
	int rows;
	char *p, *p1, *p2;
	bool update_or_delete = false;
	POOL_SESSION_CONTEXT *session_context;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("CommandComplete: cannot get session context");
		return POOL_END;
	}

	if (session_context->query_context != NULL)
	{
		Node *node = session_context->query_context->parse_tree;

		if (IsA(node, PrepareStmt))
		{
			if (session_context->uncompleted_message)
			{
				pool_add_sent_message(session_context->uncompleted_message);
				session_context->uncompleted_message = NULL;
			}
		}
		else if (IsA(node, DeallocateStmt))
		{
			char *name;
			
			name = ((DeallocateStmt *)node)->name;
			if (name == NULL)
			{
				pool_remove_sent_messages('Q');
				pool_remove_sent_messages('P');
			}
			else
			{
				pool_remove_sent_message('Q', name);
				pool_remove_sent_message('P', name);
			}
		}
		else if (IsA(node, DiscardStmt))
		{
			DiscardStmt *stmt = (DiscardStmt *)node;

			if (stmt->target == DISCARD_PLANS)
			{
				pool_remove_sent_messages('Q');
				pool_remove_sent_messages('P');
			}
			else if (stmt->target == DISCARD_ALL)
			{
				pool_clear_sent_message_list();
			}
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
		else if (IsA(node, TransactionStmt))
		{
			TransactionStmt *stmt = (TransactionStmt *) node;

			if (stmt->kind == TRANS_STMT_BEGIN || stmt->kind == TRANS_STMT_START)
			{
				int i;

				for (i = 0; i < NUM_BACKENDS; i++)
				{
					if (!VALID_BACKEND(i))
						continue;

					TSTATE(backend, i) = 'T';
				}
				pool_unset_writing_transaction();
				pool_unset_failed_transaction();
				pool_unset_transaction_isolation();
			}
		}

	}

	status = pool_read(MASTER(backend), &len, sizeof(len));
	if (status < 0)
	{
		pool_error("CommandComplete: error while reading message length");
		return POOL_END;
	}

	len = ntohl(len);
	len -= 4;
	len1 = len;

	p = pool_read2(MASTER(backend), len);
	if (p == NULL)
		return POOL_END;
	p1 = malloc(len);
	if (p1 == NULL)
	{
		pool_error("CommandComplete: malloc failed");
		return POOL_ERROR;
	}
	memcpy(p1, p, len);

	rows = extract_ntuples(p);

	/*
	 * Save number of affected tuples of master node.
	 */
	session_context->ntuples[MASTER_NODE_ID] = rows;

	if (strstr(p, "UPDATE") || strstr(p, "DELETE"))
		update_or_delete = true;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!IS_MASTER_NODE_ID(i))
		{
			if (!VALID_BACKEND(i))
			{
				session_context->ntuples[i] = -1;
				continue;
			}

			status = pool_read(CONNECTION(backend, i), &len, sizeof(len));
			if (status < 0)
			{
				pool_error("CommandComplete: error while reading message length");
				return POOL_END;
			}

			len = ntohl(len);
			len -= 4;

			p = pool_read2(CONNECTION(backend, i), len);
			if (p == NULL)
				return POOL_END;

			if (len != len1)
			{
				pool_debug("CommandComplete: length does not match between backends master(%d) %d th backend(%d)",
 						   len, i, len1);
			}

			int n = extract_ntuples(p);

			/*
			 * Save number of affected tuples.
			 */
			session_context->ntuples[i] = n;

			/*
			 * if we are in the parallel mode, we have to sum up the number
			 * of affected rows
			 */
			if (PARALLEL_MODE && is_parallel_table && update_or_delete)
			{
				rows += n;
			}
			else
			{
				if (rows != n)
				{
					/*
					 * Remember that we have different number of UPDATE/DELETE
					 * affected tuples in backends.
					 */
					session_context->mismatch_ntuples = true;
				}
			}
		}
	}

	if (session_context->mismatch_ntuples)
	{
		char msgbuf[128];
		POOL_MEMORY_POOL *old_context;

		if (session_context->query_context)
			old_context = pool_memory_context_switch_to(session_context->query_context->memory_context);
		else
			old_context = pool_memory_context_switch_to(session_context->memory_context);

		String *msg = init_string("pgpool detected difference of the number of inserted, updated or deleted tuples. Possible last query was: \"");
		string_append_char(msg, query_string_buffer);
		string_append_char(msg, "\"");
		pool_send_error_message(frontend, MAJOR(backend),
								"XX001", msg->data, "",
								"check data consistency between master and other db node",  __FILE__, __LINE__);
		pool_error("%s", msg->data);
		free_string(msg);

		msg = init_string("CommandComplete: Number of affected tuples are:");

		for (i=0;i<NUM_BACKENDS;i++)
		{
			snprintf(msgbuf, sizeof(msgbuf), " %d", session_context->ntuples[i]);
			string_append_char(msg, msgbuf);
		}
		pool_log("%s", msg->data);
		free_string(msg);

		pool_memory_context_switch_to(old_context);
	}
	else
	{
		if (PARALLEL_MODE && is_parallel_table && update_or_delete)
		{
			char tmp[32];

			strncpy(tmp, p1, 7);
			sprintf(tmp+7, "%d", rows);

			p2 = strdup(tmp);
			if (p2 == NULL)
			{
				pool_error("CommandComplete: malloc failed");
				free(p1);
				return POOL_ERROR;
			}

			free(p1);
			p1 = p2;
			len1 = strlen(p2) + 1;
		}

		pool_write(frontend, "C", 1);
		sendlen = htonl(len1+4);
		pool_write(frontend, &sendlen, sizeof(sendlen));
		pool_write_and_flush(frontend, p1, len1);
	}

	/* save the received result for each kind */
	if (pool_config->enable_query_cache && SYSDB_STATUS == CON_UP)
	{
		query_cache_register('C', frontend, backend->info->database, p1, len1);
	}

	/* Save the received result to buffer for each kind */
	if (pool_config->memory_cache_enabled)
	{
		if (pool_is_cache_safe() && !pool_is_cache_exceeded())
		{
			memqcache_register('C', frontend, p1, len1);
		}
	}

	free(p1);

	if (pool_is_doing_extended_query_message())
	{
		pool_set_query_state(session_context->query_context, POOL_EXECUTE_COMPLETE);
	}

	return POOL_CONTINUE;
}

POOL_STATUS ParameterDescription(POOL_CONNECTION *frontend,
								 POOL_CONNECTION_POOL *backend)
{
	int len, len1 = 0;
	char *p = NULL;
	char *p1 = NULL;
	int status;
	int sendlen;
	int i;

	POOL_SESSION_CONTEXT *session_context;
	int num_params, send_num_params, num_dmy;
	char kind = 't';

	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("ParameterDescription: cannot get session context");
		return POOL_END;
	}

	/* only in replication mode and rewritten query */
	if (!REPLICATION || !session_context->query_context->rewritten_query)
		return SimpleForwardToFrontend('t', frontend, backend);

	/* get number of parameters in original query */
	num_params = session_context->query_context->num_original_params;

	status = pool_read(MASTER(backend), &len, sizeof(len));
	if (status < 0)
	{
		pool_error("ParameterDescription: error while reading message length");
		return POOL_END;
	}

	len = ntohl(len);
	len -= sizeof(int32);
	len1 = len;

	/* number of parameters in rewritten query is just discarded */
	status = pool_read(MASTER(backend), &num_dmy, sizeof(int16));
	len -= sizeof(int16);

	p = pool_read2(MASTER(backend), len);
	if (p == NULL)
		return POOL_END;

	p1 = malloc(len);
	if (p1 == NULL)
	{
		pool_error("ParameterDescription: malloc failed");
		return POOL_ERROR;
	}
	memcpy(p1, p, len);

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
		{
			status = pool_read(CONNECTION(backend, i), &len, sizeof(len));
			if (status < 0)
			{
				pool_error("ParameterDescription: error while reading message length");
                free(p1);
				return POOL_END;
			}

			len = ntohl(len);
			len -= sizeof(int32);

			p = pool_read2(CONNECTION(backend, i), len);
			if (p == NULL)
            {
                free(p1);
				return POOL_END;
            }
			if (len != len1)
			{
				pool_debug("ParameterDescription: length does not match between backends master(%d) %d th backend(%d) kind:(%c)",
						   len, i, len1, kind);
			}
		}
	}

	pool_write(frontend, &kind, 1);

	/* send back OIDs of parameters in original query and left are discarded */
	len = sizeof(int16) + num_params * sizeof(int32);
	sendlen = htonl(len + sizeof(int32));
	pool_write(frontend, &sendlen, sizeof(int32));

	send_num_params = htons(num_params);
	pool_write(frontend, &send_num_params, sizeof(int16));

	if (pool_write_and_flush(frontend, p1, num_params * sizeof(int32)) < 0)
	{
		pool_error("ParameterDescription: pool_write_and_flush failed");
		free(p1);
		return POOL_END;
	}

	free(p1);
	return POOL_CONTINUE;
}

POOL_STATUS ErrorResponse3(POOL_CONNECTION *frontend,
						   POOL_CONNECTION_POOL *backend)
{
	POOL_STATUS ret;

	ret = SimpleForwardToFrontend('E', frontend, backend);
	if (ret != POOL_CONTINUE)
		return ret;

	ret = raise_intentional_error_if_need(backend);
	if (ret != POOL_CONTINUE)
		return ret;
	
#ifdef NOT_USED
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
#endif

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
	char *bufp = NULL;
	char *contents;
	POOL_STATUS status;
	int len;
	POOL_SESSION_CONTEXT *session_context;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("ProcessFrontendResponse: cannot get session context");
		return POOL_END;
	}

	if (pool_read_buffer_is_empty(frontend) && frontend->no_forward != 0)
		return POOL_CONTINUE;

	if (pool_read(frontend, &fkind, 1) < 0)
	{
		pool_log("ProcessFrontendResponse: failed to read kind from frontend. frontend abnormally exited");
		return POOL_END_WITH_FRONTEND_ERROR;
	}

	pool_debug("ProcessFrontendResponse: kind from frontend %c(%02x)", fkind, fkind);

	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		if (pool_read(frontend, &len, sizeof(len)) < 0)
			return POOL_END;
		len = ntohl(len) - 4;
		if (len > 0)
			bufp = pool_read2(frontend, len);
	}
	else
	{
		if (fkind != 'F')
			bufp = pool_read_string(frontend, &len, 0);
	}

	if (len > 0 && bufp == NULL)
		return POOL_END;

	if (fkind != 'S' && pool_is_ignore_till_sync())
	{
		/*
		 * Flag setting for calling ProcessBackendResponse()
		 * in pool_process_query().
		 */
		if (!pool_is_query_in_progress())
			pool_set_query_in_progress();
		return POOL_CONTINUE;
	}

	pool_unset_doing_extended_query_message();

	/*
	 * Allocate buffer and copy the packet contents.  Because inside
	 * these protocol modules, pool_read2 maybe called and modify its
	 * buffer contents.
	 */
	contents = malloc(len);
	if (contents == NULL)
	{
		pool_error("ProcessFrontendResponse: cannot allocate memory. request size:%d", len);
		return POOL_ERROR;
	}
	memcpy(contents, bufp, len);

	switch (fkind)
	{
		POOL_QUERY_CONTEXT *query_context;
		char *query;
		Node *node;
		List *parse_tree_list;
		POOL_MEMORY_POOL *old_context;

		case 'X':	/* Terminate */
			free(contents);
			return POOL_END;

		case 'Q':	/* Query */
			allow_close_transaction = 1;
			status = SimpleQuery(frontend, backend, len, contents);
			break;

		case 'E':	/* Execute */
			allow_close_transaction = 1;
			pool_set_doing_extended_query_message();
			if (!pool_is_query_in_progress() && !pool_is_ignore_till_sync())
				pool_set_query_in_progress();
			status = Execute(frontend, backend, len, contents);
			break;

		case 'P':	/* Parse */
			allow_close_transaction = 0;
			pool_set_doing_extended_query_message();
			if (!pool_is_query_in_progress() && !pool_is_ignore_till_sync())
				pool_set_query_in_progress();
			status = Parse(frontend, backend, len, contents);
			break;

		case 'B':	/* Bind */
			pool_set_doing_extended_query_message();
			if (!pool_is_query_in_progress() && !pool_is_ignore_till_sync())
				pool_set_query_in_progress();
			status = Bind(frontend, backend, len, contents);
			break;

		case 'C':	/* Close */
			if (!pool_is_query_in_progress() && !pool_is_ignore_till_sync())
				pool_set_query_in_progress();
			status = Close(frontend, backend, len, contents);
			break;

		case 'D':	/* Describe */
			pool_set_doing_extended_query_message();
			if (!pool_is_query_in_progress() && !pool_is_ignore_till_sync())
				pool_set_query_in_progress();
			status = Describe(frontend, backend, len, contents);
			break;

		case 'S':  /* Sync */
			pool_set_doing_extended_query_message();
			if (pool_is_ignore_till_sync())
				pool_unset_ignore_till_sync();
			if (!pool_is_query_in_progress())
				pool_set_query_in_progress();
			status = SimpleForwardToBackend(fkind, frontend, backend, len, contents);
			break;

		case 'F':	/* FunctionCall */
			/*
			 * Create dummy query context as if it were an INSERT.
			 */
			query_context = pool_init_query_context();
			if (!query_context)
			{
				pool_error("ProcessFrontendResponse: pool_init_query_context failed");
				free(contents);
				return POOL_END;
			}
			old_context = pool_memory_context_switch_to(query_context->memory_context);
			query = "INSERT INTO foo VALUES(1)";
			parse_tree_list = raw_parser(query);
			node = (Node *) lfirst(list_head(parse_tree_list));
			pool_start_query(query_context, query, strlen(query) + 1, node);
			pool_where_to_send(query_context, query_context->original_query,
							   query_context->parse_tree);

			if (MAJOR(backend) == PROTO_MAJOR_V3)
				status = FunctionCall3(frontend, backend, len, contents);
			else
				status = FunctionCall(frontend, backend);

			pool_memory_context_switch_to(old_context);
			break;

		case 'c':	/* CopyDone */
		case 'd':	/* CopyData */
		case 'f':	/* CopyFail */
		case 'H':	/* Flush */
			if (MAJOR(backend) == PROTO_MAJOR_V3)
			{
				status = SimpleForwardToBackend(fkind, frontend, backend, len, contents);
				break;
			}

		default:
			pool_error("ProcessFrontendResponse: unknown message type %c(%02x)", fkind, fkind);
			status = POOL_ERROR;
	}
	free(contents);

	if (status != POOL_CONTINUE)
		status = POOL_ERROR;
	return status;
}

POOL_STATUS ProcessBackendResponse(POOL_CONNECTION *frontend,
								   POOL_CONNECTION_POOL *backend,
								   int *state, short *num_fields)
{
	int status;
	char kind;
	POOL_SESSION_CONTEXT *session_context;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("ProcessBackendResponse: cannot get session context");
		return POOL_END;
	}

	if (pool_is_ignore_till_sync())
	{
		if (pool_is_query_in_progress())
			pool_unset_query_in_progress();
		return POOL_CONTINUE;
	}

	if (pool_is_skip_reading_from_backends())
	{
		pool_unset_skip_reading_from_backends();
		return POOL_CONTINUE;
	}

    status = read_kind_from_backend(frontend, backend, &kind);
    if (status != POOL_CONTINUE)
        return status;

	/*
	 * Sanity check
	 */
	if (kind == 0)
	{
		pool_error("ProcessBackendResponse: kind is 0!");
		return POOL_ERROR;
	}

	pool_debug("ProcessBackendResponse: kind from backend: %c", kind);

	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		switch (kind)
		{
			case 'G':	/* CopyInResponse */
				status = CopyInResponse(frontend, backend);
				break;

			case 'S':	/* ParameterStatus */
				status = ParameterStatus(frontend, backend);
				break;

			case 'Z':	/* ReadyForQuery */
				status = ReadyForQuery(frontend, backend, true, true);
				pool_debug("ProcessBackendResponse: Ready For Query");
				break;

			case '1':	/* ParseComplete */
				status = ParseComplete(frontend, backend);
				pool_set_command_success();
				pool_unset_query_in_progress();
				break;

			case '2':	/* BindComplete */
				status = BindComplete(frontend, backend);
				pool_set_command_success();
				pool_unset_query_in_progress();
				break;

			case '3':	/* CloseComplete */
				status = CloseComplete(frontend, backend);
				pool_set_command_success();
				pool_unset_query_in_progress();
				break;

			case 'E':	/* ErrorResponse */
				status = ErrorResponse3(frontend, backend);
				pool_unset_command_success();
				if (TSTATE(backend, MASTER_SLAVE ? PRIMARY_NODE_ID :
						   REAL_MASTER_NODE_ID) != 'I')
					pool_set_failed_transaction();
				if (pool_is_doing_extended_query_message())
				{
					pool_set_ignore_till_sync();
					pool_unset_query_in_progress();
				}
				break;

			case 'C':	/* CommandComplete */				
				status = CommandComplete(frontend, backend);
				pool_set_command_success();
				if (pool_is_doing_extended_query_message())
					pool_unset_query_in_progress();
				break;

			case 't':	/* ParameterDescription */
				status = ParameterDescription(frontend, backend);
				break;

			case 'I':	/* EmptyQueryResponse */
				status = SimpleForwardToFrontend(kind, frontend, backend);
				if (pool_is_doing_extended_query_message())
					pool_unset_query_in_progress();
				break;

			case 'T':	/* RowDescription */
				status = SimpleForwardToFrontend(kind, frontend, backend);
				if (pool_is_doing_extended_query_message())
					pool_unset_query_in_progress();
				break;

			case 'n':	/* NoData */
				status = SimpleForwardToFrontend(kind, frontend, backend);
				if (pool_is_doing_extended_query_message())
					pool_unset_query_in_progress();
				break;

			case 's':	/* PortalSuspended */
				status = SimpleForwardToFrontend(kind, frontend, backend);
				if (pool_is_doing_extended_query_message())
					pool_unset_query_in_progress();
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
			case 'A':	/* NotificationResponse */
				status = NotificationResponse(frontend, backend);
				break;

			case 'B':	/* BinaryRow */
				status = BinaryRow(frontend, backend, *num_fields);
				break;

			case 'C':	/* CompletedResponse */
				status = CompletedResponse(frontend, backend);
				break;

			case 'D':	/* AsciiRow */
				status = AsciiRow(frontend, backend, *num_fields);
				break;

			case 'E':	/* ErrorResponse */
				status = ErrorResponse(frontend, backend);
				if (TSTATE(backend, MASTER_SLAVE ? PRIMARY_NODE_ID :
						   REAL_MASTER_NODE_ID) != 'I')
					pool_set_failed_transaction();
				break;

			case 'G':	/* CopyInResponse */
				status = CopyInResponse(frontend, backend);
				break;

			case 'H':	/* CopyOutResponse */
				status = CopyOutResponse(frontend, backend);
				break;

			case 'I':	/* EmptyQueryResponse */
				status = EmptyQueryResponse(frontend, backend);
				break;

			case 'N':	/* NoticeResponse */
				status = NoticeResponse(frontend, backend);
				break;

			case 'P':	/* CursorResponse */
				status = CursorResponse(frontend, backend);
				break;

			case 'T':	/* RowDescription */
				status = RowDescription(frontend, backend, num_fields);
				break;

			case 'V':	/* FunctionResultResponse and FunctionVoidResponse */
				status = FunctionResultResponse(frontend, backend);
				break;

			case 'Z':	/* ReadyForQuery */
				status = ReadyForQuery(frontend, backend, true, true);
				break;

			default:
				pool_error("Unknown message type %c(%02x)", kind, kind);
				exit(1);
		}
	}

	/* Do we receive ready for query while processing reset
	 * request?
	 */
	if (kind == 'Z' && frontend->no_forward && *state == 1)
	{
		*state = 0;
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
						if (id < 0 || !VALID_BACKEND(id))
						{
							pool_error("CopyDataRow: pool_get_id returns invalid id: %d", id);
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
					char *contents = NULL;

					pool_debug("CopyDataRows: read kind from frontend %c(%02x)", kind, kind);

					if (pool_read(frontend, &len, sizeof(len)) < 0)
						return POOL_END;
					len = ntohl(len) - 4;
					if (len > 0)
						contents = pool_read2(frontend, len);

					SimpleForwardToBackend(kind, frontend, backend, len, contents);
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
		strlcpy(buf, string, sizeof(buf));
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
 * This function raises intentional error to make backends the same 
 * transaction state.
 */
POOL_STATUS raise_intentional_error_if_need(POOL_CONNECTION_POOL *backend)
{
	int i;
	POOL_STATUS ret;
	POOL_SESSION_CONTEXT *session_context;
	POOL_QUERY_CONTEXT *query_context;

	/* Get session context */
	session_context = pool_get_session_context();
	if (!session_context)
	{
		pool_error("raise_intentional_error_if_need: cannot get session context");
		return POOL_END;
	}

	query_context = session_context->query_context;

	if (MASTER_SLAVE &&
		TSTATE(backend, PRIMARY_NODE_ID) == 'T' &&
		PRIMARY_NODE_ID != MASTER_NODE_ID &&
		query_context &&
		is_select_query(query_context->parse_tree, query_context->original_query))
	{
		pool_set_node_to_be_sent(query_context, PRIMARY_NODE_ID);
		if (pool_is_doing_extended_query_message())
		{
			ret = do_error_execute_command(backend, PRIMARY_NODE_ID, PROTO_MAJOR_V3);
			if (ret != POOL_CONTINUE)
				return ret;
		}
		else
		{
			ret = do_error_command(CONNECTION(backend, PRIMARY_NODE_ID), MAJOR(backend));
			if (ret != POOL_CONTINUE)
				return ret;
		}
		pool_debug("raise_intentional_error: intentional error occurred");
	}

	if (REPLICATION &&
		TSTATE(backend, REAL_MASTER_NODE_ID) == 'T' &&
		!pool_config->replicate_select &&
		query_context &&
		is_select_query(query_context->parse_tree, query_context->original_query))
	{
		for (i = 0; i < NUM_BACKENDS; i++)
		{
			/*
			 * Send a syntax error query to all backends except the node
			 * which the original query was sent.
			 */
			if (pool_is_node_to_be_sent(query_context, i))
				continue;
			else
				pool_set_node_to_be_sent(query_context, i);

			if (VALID_BACKEND(i))
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
				if (pool_is_doing_extended_query_message())
				{
					ret = do_error_execute_command(backend, i, PROTO_MAJOR_V3);
					if (ret != POOL_CONTINUE)
						return ret;
				}
				else
				{
					ret = do_error_command(CONNECTION(backend, i), MAJOR(backend));
					if (ret != POOL_CONTINUE)
						return ret;
				}
			}
		}
	}

	return POOL_CONTINUE;
}

void
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
	if (detect_serialization_error(CONNECTION(backend, backend_id), MAJOR(backend), true) == SPECIFIED_ERROR)
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
	POOL_SESSION_CONTEXT *session_context;
	POOL_MEMORY_POOL *old_context;

	session_context = pool_get_session_context();
	if (!session_context)
		return;

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

	if (session_context->query_context)
		old_context = pool_memory_context_switch_to(session_context->query_context->memory_context);
	else
		old_context = pool_memory_context_switch_to(session_context->memory_context);

	msg = init_string(prefix);
	string_append_char(msg, error_messages[specific_error]);
	pool_error(msg->data, query);
	free_string(msg);

	pool_memory_context_switch_to(old_context);
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

static POOL_STATUS parse_before_bind(POOL_CONNECTION *frontend,
									 POOL_CONNECTION_POOL *backend,
									 POOL_SENT_MESSAGE *message)
{
	int i;
	int len = message->len;
	char kind = '\0';
	char *contents = message->contents;
	bool parse_was_sent = false;
	bool backup[MAX_NUM_BACKENDS];
	POOL_STATUS status;
	POOL_QUERY_CONTEXT *qc = message->query_context;

	memcpy(backup, qc->where_to_send, sizeof(qc->where_to_send));

	/* expect to send to master node only */
	for (i = 0; i < NUM_BACKENDS; i++)
	{
		if (qc->where_to_send[i] && statecmp(qc->query_state[i], POOL_PARSE_COMPLETE) < 0)
		{
			pool_debug("parse_before_bind: waiting for backend %d completing parse", i);
			if (pool_extended_send_and_wait(qc, "P", len, contents, 1, i) != POOL_CONTINUE)
				return POOL_END;
		}
		else
		{
			qc->where_to_send[i] = 0;
		}
	}

	for (i = 0; i < NUM_BACKENDS; i++)
	{
		if (qc->where_to_send[i])
		{
			parse_was_sent = true;
			break;
		}
	}
	
	if (parse_was_sent)
	{
		while (kind != '1')
		{
			status = read_kind_from_backend(frontend, backend, &kind);
			if (status != POOL_CONTINUE)
			{
				memcpy(qc->where_to_send, backup, sizeof(backup));
				return status;
			}
			status = pool_discard_packet_contents(backend);
			if (status != POOL_CONTINUE)
			{
				memcpy(qc->where_to_send, backup, sizeof(backup));
				return status;
			}
		}
	}

	memcpy(qc->where_to_send, backup, sizeof(backup));
	return POOL_CONTINUE;
}

/*
 * Find victim nodes by "decide by majority" rule and returns array
 * of victim node ids. If no victim is found, return NULL.
 *
 * Arguments:
 * ntuples: Array of number of affected tuples. -1 represents down node.
 * nmembers: Number of elements in ntuples.
 * master_node: The master node id. Less than 0 means ignore this parameter.
 * number_of_nodes: Number of elements in victim nodes array.
 *
 * Note: If no one wins and master_node >= 0, winner would be the
 * master and other nodes who has same number of tuples as the master.
 *
 * Caution: Returned victim node array is allocated in static memory
 * of this function. Subsequent calls to this function will overwrite
 * the memory.
 */
static int* find_victim_nodes(int *ntuples, int nmembers, int master_node, int *number_of_nodes)
{
	static int victim_nodes[MAX_NUM_BACKENDS];
	static int votes[MAX_NUM_BACKENDS];
	int maxvotes;
	int majority_ntuples;
	int me;
	int cnt;
	int healthy_nodes;
	int i, j;

	healthy_nodes = 0;
	*number_of_nodes = 0;
	maxvotes = 0;
	majority_ntuples = 0;

	for (i=0;i<nmembers;i++)
	{
		me = ntuples[i];

		/* Health node? */
		if (me < 0)
		{
			votes[i] = -1;
			continue;
		}

		healthy_nodes++;
		votes[i] = 1;

		for (j=0;j<nmembers;j++)
		{
			if (i != j && me == ntuples[j])
			{
				votes[i]++;

				if (votes[i] > maxvotes)
				{
					maxvotes = votes[i];
					majority_ntuples = me;
				}
			}
		}
	}

	/* Everyone is different */
	if (maxvotes == 1)
	{
		/* Master node is specified? */
		if (master_node < 0)
			return NULL;

		/*
		 * If master node is specified, let it and others who has same
		 * ntuples win.
		 */
		majority_ntuples = ntuples[master_node];
	}
	else
	{
		/* Find number of majority */
		cnt = 0;
		for (i=0;i<nmembers;i++)
		{
			if (votes[i] == maxvotes)
			{
				cnt++;
			}
		}

		if (cnt <= healthy_nodes / 2.0)
		{
			/* No one wins */

			/* Master node is specified? */
			if (master_node < 0)
				return NULL;

			/*
			 * If master node is specified, let it and others who has same
			 * ntuples win.
			 */
			majority_ntuples = ntuples[master_node];
		}
	}

	/* Make victim nodes list */
	for (i=0;i<nmembers;i++)
	{
		if (ntuples[i] >= 0 && ntuples[i] != majority_ntuples)
		{
			victim_nodes[(*number_of_nodes)++] = i;
		}
	}

	return victim_nodes;
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
