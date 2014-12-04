/* -*-pgsql-c-*- */
/*
 *
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2014	PgPool Global Development Group
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
 */
#include "pool.h"
#include "pool_config.h"
#include "protocol/pool_proto_modules.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/elog.h"
#include "utils/pool_select_walker.h"
#include "context/pool_session_context.h"
#include "context/pool_query_context.h"
#include "parser/nodes.h"

#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>

/*
 * Where to send query
 */
typedef enum {
	POOL_PRIMARY,
	POOL_STANDBY,
	POOL_EITHER,
	POOL_BOTH
} POOL_DEST;

#define CHECK_QUERY_CONTEXT_IS_VALID \
						do { \
							if (!query_context) \
								ereport(ERROR, \
									(errmsg("setting db node for query to be sent, no query context")));\
						} while (0)

static POOL_DEST send_to_where(Node *node, char *query);
static void where_to_send_deallocate(POOL_QUERY_CONTEXT *query_context, Node *node);
static char* remove_read_write(int len, const char *contents, int *rewritten_len);

/*
 * Create and initialize per query session context
 */
POOL_QUERY_CONTEXT *pool_init_query_context(void)
{
	MemoryContext memory_context = AllocSetContextCreate(QueryContext,
															"QueryContextMemoryContext",
															ALLOCSET_SMALL_MINSIZE,
															ALLOCSET_SMALL_INITSIZE,
															ALLOCSET_SMALL_MAXSIZE);

	MemoryContext oldcontext = MemoryContextSwitchTo(memory_context);
	POOL_QUERY_CONTEXT *qc;
	qc = palloc0(sizeof(*qc));
	qc->memory_context = memory_context;
	MemoryContextSwitchTo(oldcontext);
	return qc;
}

/*
 * Destroy query context
 */
void pool_query_context_destroy(POOL_QUERY_CONTEXT *query_context)
{
	POOL_SESSION_CONTEXT *session_context;

	if (query_context)
	{
		MemoryContext memory_context = query_context->memory_context;
		session_context = pool_get_session_context(false);
		pool_unset_query_in_progress();
		query_context->original_query = NULL;
		session_context->query_context = NULL;
		pfree(query_context);
		MemoryContextDelete(memory_context);
	}
}

/*
 * Start query
 */
void pool_start_query(POOL_QUERY_CONTEXT *query_context, char *query, int len, Node *node)
{
	POOL_SESSION_CONTEXT *session_context;

	if (query_context)
	{
		MemoryContext old_context;
		session_context = pool_get_session_context(false);
		old_context = MemoryContextSwitchTo(query_context->memory_context);
		query_context->original_length = len;
		query_context->rewritten_length = -1;
		query_context->original_query = pstrdup(query);
		query_context->rewritten_query = NULL;
		query_context->parse_tree = node;
		query_context->virtual_master_node_id = my_master_node_id;
		query_context->is_cache_safe = false;
		query_context->num_original_params = -1;
		if (pool_config->memory_cache_enabled)
			query_context->temp_cache = pool_create_temp_query_cache(query);
		pool_set_query_in_progress();
		session_context->query_context = query_context;
		MemoryContextSwitchTo(old_context);
	}
}

/*
 * Specify DB node to send query
 */
void pool_set_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id)
{
	CHECK_QUERY_CONTEXT_IS_VALID;

	if (node_id < 0 || node_id >= MAX_NUM_BACKENDS)
		ereport(ERROR,
			(errmsg("setting db node for query to be sent, invalid node id:%d",node_id),
				 errdetail("backend node id: %d out of range, node id can be between 0 and %d",node_id,MAX_NUM_BACKENDS)));

	query_context->where_to_send[node_id] = true;
	
	return;
}

/*
 * Unspecify DB node to send query
 */
void pool_unset_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id)
{
	CHECK_QUERY_CONTEXT_IS_VALID;

	if (node_id < 0 || node_id >= MAX_NUM_BACKENDS)
		ereport(ERROR,
			(errmsg("un setting db node for query to be sent, invalid node id:%d",node_id),
				 errdetail("backend node id: %d out of range, node id can be between 0 and %d",node_id,MAX_NUM_BACKENDS)));

	query_context->where_to_send[node_id] = false;
	
	return;
}

/*
 * Clear DB node map
 */
void pool_clear_node_to_be_sent(POOL_QUERY_CONTEXT *query_context)
{
	CHECK_QUERY_CONTEXT_IS_VALID;

	memset(query_context->where_to_send, false, sizeof(query_context->where_to_send));
	return;
}

/*
 * Set all DB node map entry
 */
void pool_setall_node_to_be_sent(POOL_QUERY_CONTEXT *query_context)
{
	int i;
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context(false);

	CHECK_QUERY_CONTEXT_IS_VALID;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (private_backend_status[i] == CON_UP ||
			(private_backend_status[i] == CON_CONNECT_WAIT))
		{
			/*
			 * In streaming replication mode, if the node is not
			 * primary node nor load balance node, there's no point to
			 * send query.
			 */
			if (pool_config->master_slave_mode &&
				!strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP) &&
				i != PRIMARY_NODE_ID && i != sc->load_balance_node_id)
			{
				continue;
			}
			query_context->where_to_send[i] = true;
		}
	}
	return;
}

/*
 * Return true if multiple nodes are targets
 */
bool pool_multi_node_to_be_sent(POOL_QUERY_CONTEXT *query_context)
{
	int i;
	int cnt = 0;

	CHECK_QUERY_CONTEXT_IS_VALID;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (((BACKEND_INFO(i)).backend_status == CON_UP ||
			 BACKEND_INFO((i)).backend_status == CON_CONNECT_WAIT) &&
			query_context->where_to_send[i])
		{
			cnt++;
			if (cnt > 1)
			{
				return true;
			}
		}
	}
	return false;
}

/*
 * Return if the DB node is needed to send query
 */
bool pool_is_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id)
{
	CHECK_QUERY_CONTEXT_IS_VALID;

	if (node_id < 0 || node_id >= MAX_NUM_BACKENDS)
		ereport(ERROR,
			(errmsg("checking if db node is needed to be sent, invalid node id:%d",node_id),
				 errdetail("backend node id: %d out of range, node id can be between 0 and %d",node_id,MAX_NUM_BACKENDS)));

	return query_context->where_to_send[node_id];
}

/*
 * Returns true if the DB node is needed to send query.
 * Intended to be called from VALID_BACKEND
 */
bool pool_is_node_to_be_sent_in_current_query(int node_id)
{
	POOL_SESSION_CONTEXT *sc;

	if (RAW_MODE)
		return node_id == REAL_MASTER_NODE_ID;

	sc = pool_get_session_context(true);
	if (!sc)
		return true;

	if (pool_is_query_in_progress() && sc->query_context)
	{
		return pool_is_node_to_be_sent(sc->query_context, node_id);
	}
	return true;
}

/*
 * Returns virtual master DB node id,
 */
int pool_virtual_master_db_node_id(void)
{
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context(true);
	if (!sc)
	{
		return REAL_MASTER_NODE_ID;
	}

	if (sc->query_context)
	{
		return sc->query_context->virtual_master_node_id;
	}

	/*
	 * No query context exists.  If in master/slave mode, returns
	 * primary node if exists.  Otherwise returns my_master_node_id,
	 * which represents the last REAL_MASTER_NODE_ID.
	 */
	if (MASTER_SLAVE)
	{
		return PRIMARY_NODE_ID;
	}
	return my_master_node_id;
}

/*
 * Decide where to send queries(thus expecting response)
 */
void pool_where_to_send(POOL_QUERY_CONTEXT *query_context, char *query, Node *node)
{
	POOL_SESSION_CONTEXT *session_context;
	POOL_CONNECTION_POOL *backend;
	int i;

	CHECK_QUERY_CONTEXT_IS_VALID;

	session_context = pool_get_session_context(false);
	backend = session_context->backend;

	/*
	 * Zap out DB node map
	 */
	pool_clear_node_to_be_sent(query_context);

	/*
	 * If there is "NO LOAD BALANCE" comment, we send only to master node.
	 */
	if (!strncasecmp(query, NO_LOAD_BALANCE, NO_LOAD_BALANCE_COMMENT_SZ))
	{
		pool_set_node_to_be_sent(query_context,
								 MASTER_SLAVE ? PRIMARY_NODE_ID : REAL_MASTER_NODE_ID);
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (query_context->where_to_send[i])
			{
				query_context->virtual_master_node_id = i;
				break;
			}
		}
		return;
	}

	/*
	 * In raw mode, we send only to master node. Simple enough.
	 */
	if (RAW_MODE)
	{
		pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
	}
	else if (MASTER_SLAVE && query_context->is_multi_statement)
	{
		/*
		 * If we are in master/slave mode and we have multi statement
		 * query, we should send it to primary server only. Otherwise
		 * it is possible to send a write query to standby servers
		 * because we only use the first element of the multi
		 * statement query and don't care about the rest.  Typical
		 * situation where we are bugged by this is, "BEGIN;DELETE
		 * FROM table;END". Note that from pgpool-II 3.1.0
		 * transactional statements such as "BEGIN" is unconditionally
		 * sent to all nodes(see send_to_where() for more details).
		 * Someday we might be able to understand all part of multi
		 * statement queries, but until that day we need this band
		 * aid.
		 */
		if (query_context->is_multi_statement)
		{
			pool_set_node_to_be_sent(query_context, PRIMARY_NODE_ID);
		}
	}
	else if (MASTER_SLAVE)
	{
		POOL_DEST dest;

		dest = send_to_where(node, query);

		ereport(DEBUG1,
			(errmsg("decide where to send the queries"),
				 errdetail("destination = %d for query= \"%s\"", dest, query)));

		/* Should be sent to primary only? */
		if (dest == POOL_PRIMARY)
		{
			pool_set_node_to_be_sent(query_context, PRIMARY_NODE_ID);
		}
		/* Should be sent to both primary and standby? */
		else if (dest == POOL_BOTH)
		{
			pool_setall_node_to_be_sent(query_context);
		}

		/*
		 * Ok, we might be able to load balance the SELECT query.
		 */
		else
		{
			if (pool_config->load_balance_mode &&
				is_select_query(node, query) &&
				MAJOR(backend) == PROTO_MAJOR_V3)
			{
				/* 
				 * If (we are outside of an explicit transaction) OR
				 * (the transaction has not issued a write query yet, AND
				 *	transaction isolation level is not SERIALIZABLE)
				 * we might be able to load balance.
				 */
				if (TSTATE(backend, PRIMARY_NODE_ID) == 'I' ||
					(!pool_is_writing_transaction() &&
					 !pool_is_failed_transaction() &&
					 pool_get_transaction_isolation() != POOL_SERIALIZABLE))
				{
					BackendInfo *bkinfo = pool_get_node_info(session_context->load_balance_node_id);

					/*
					 * Load balance if possible
					 */

					/*
					 * If replication delay is too much, we prefer to send to the primary.
					 */
					if (!strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP) &&
						pool_config->delay_threshold &&
						bkinfo->standby_delay > pool_config->delay_threshold)
					{
						pool_set_node_to_be_sent(query_context, PRIMARY_NODE_ID);
					}

					/*
					 * If a writing function call is used, 
					 * we prefer to send to the primary.
					 */
					else if (pool_has_function_call(node))
					{
						pool_set_node_to_be_sent(query_context, PRIMARY_NODE_ID);
					}

					/*
					 * If system catalog is used in the SELECT, we
					 * prefer to send to the primary. Example: SELECT
					 * * FROM pg_class WHERE relname = 't1'; Because
					 * 't1' is a constant, it's hard to recognize as
					 * table name.  Most use case such query is
					 * against system catalog, and the table name can
					 * be a temporary table, it's best to query
					 * against primary system catalog.
					 * Please note that this test must be done *before*
					 * test using pool_has_temp_table.
					 */
					else if (pool_has_system_catalog(node))
					{
						pool_set_node_to_be_sent(query_context, PRIMARY_NODE_ID);
					}

					/*
					 * If temporary table is used in the SELECT,
					 * we prefer to send to the primary.
					 */
					else if (pool_config->check_temp_table && pool_has_temp_table(node))
					{
						pool_set_node_to_be_sent(query_context, PRIMARY_NODE_ID);
					}

					/*
					 * If unlogged table is used in the SELECT,
					 * we prefer to send to the primary.
					 */
					else if (pool_config->check_unlogged_table && pool_has_unlogged_table(node))
					{
						pool_set_node_to_be_sent(query_context, PRIMARY_NODE_ID);
					}

					else
					{
						pool_set_node_to_be_sent(query_context,
												 session_context->load_balance_node_id);
					}
				}
				else
				{
					/* Send to the primary only */
					pool_set_node_to_be_sent(query_context, PRIMARY_NODE_ID);
				}
			}
			else
			{
				/* Send to the primary only */
				pool_set_node_to_be_sent(query_context, PRIMARY_NODE_ID);
			}
		}
	}
	else if (REPLICATION || PARALLEL_MODE)
	{
		if (pool_config->load_balance_mode &&
			is_select_query(node, query) &&
			MAJOR(backend) == PROTO_MAJOR_V3)
		{
			/*
			 * If a writing function call is used or replicate_select is true,
			 * we prefer to send to all nodes.
			 */
			if (pool_has_function_call(node) || pool_config->replicate_select)
			{
				pool_setall_node_to_be_sent(query_context);
			}
			/* 
			 * If (we are outside of an explicit transaction) OR
			 * (the transaction has not issued a write query yet, AND
			 *	transaction isolation level is not SERIALIZABLE)
			 * we might be able to load balance.
			 */
			else if (TSTATE(backend, MASTER_NODE_ID) == 'I' ||
					 (!pool_is_writing_transaction() &&
					  !pool_is_failed_transaction() &&
					  pool_get_transaction_isolation() != POOL_SERIALIZABLE))
			{
				/* load balance */
				pool_set_node_to_be_sent(query_context,
										 session_context->load_balance_node_id);
			}
			else
			{
				/* only send to master node */
				pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
			}
		}
		else
		{
			if (is_select_query(node, query) && !pool_config->replicate_select &&
				!pool_has_function_call(node))
			{
				/* only send to master node */
				pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
			}
			else
			{
				/* send to all nodes */
				pool_setall_node_to_be_sent(query_context);
			}
		}
	}
	else
	{
		ereport(WARNING,
				(errmsg("unknown pgpool-II mode while deciding for where to send query")));
		return;
	}

	/*
	 * EXECUTE?
	 */
	if (IsA(node, ExecuteStmt))
	{
		POOL_SENT_MESSAGE *msg;

		msg = pool_get_sent_message('Q', ((ExecuteStmt *)node)->name);
		if (!msg)
			msg = pool_get_sent_message('P', ((ExecuteStmt *)node)->name);
		if (msg)
			pool_copy_prep_where(msg->query_context->where_to_send,
								 query_context->where_to_send);
	}

	/*
	 * DEALLOCATE?
	 */
	else if (IsA(node, DeallocateStmt))
	{
		where_to_send_deallocate(query_context, node);
	}

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (query_context->where_to_send[i])
		{
			query_context->virtual_master_node_id = i;
			break;
		}
	}

	return;
}

/*
 * Send simple query and wait for response
 * send_type:
 *  -1: do not send this node_id
 *   0: send to all nodes
 *  >0: send to this node_id
 */
POOL_STATUS pool_send_and_wait(POOL_QUERY_CONTEXT *query_context,
							   int send_type, int node_id)
{
	POOL_SESSION_CONTEXT *session_context;
	POOL_CONNECTION *frontend;
	POOL_CONNECTION_POOL *backend;
	bool is_commit;
	bool is_begin_read_write;
	int i;
	int len;
	char *string;

	session_context = pool_get_session_context(false);
	frontend = session_context->frontend;
	backend = session_context->backend;
	is_commit = is_commit_or_rollback_query(query_context->parse_tree);
	is_begin_read_write = false;
	len = 0;
	string = NULL;

	/*
	 * If the query is BEGIN READ WRITE or
	 * BEGIN ... SERIALIZABLE in master/slave mode,
	 * we send BEGIN to slaves/standbys instead.
	 * original_query which is BEGIN READ WRITE is sent to primary.
	 * rewritten_query which is BEGIN is sent to standbys.
	 */
	if (pool_need_to_treat_as_if_default_transaction(query_context))
	{
		is_begin_read_write = true;
	}
	else
	{
		if (query_context->rewritten_query)
		{
			len = query_context->rewritten_length;
			string = query_context->rewritten_query;
		}
		else
		{
			len = query_context->original_length;
			string = query_context->original_query;
		}
	}

	/* Send query */
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;
		else if (send_type < 0 && i == node_id)
			continue;
		else if (send_type > 0 && i != node_id)
			continue;

		/*
		 * If in master/slave mode, we do not send COMMIT/ABORT to
		 * slaves/standbys if it's in I(idle) state.
		 */
		if (is_commit && MASTER_SLAVE && !IS_MASTER_NODE_ID(i) && TSTATE(backend, i) == 'I')
		{
			pool_unset_node_to_be_sent(query_context, i);
			continue;
		}

		/*
		 * If in reset context, we send COMMIT/ABORT to nodes those
		 * are not in I(idle) state.  This will ensure that
		 * transactions are closed.
		 */
		if (is_commit && session_context->reset_context && TSTATE(backend, i) == 'I')
		{
			pool_unset_node_to_be_sent(query_context, i);
			continue;
		}

		if (is_begin_read_write)
		{
			if (REAL_PRIMARY_NODE_ID == i)
			{
				len = query_context->original_length;
				string = query_context->original_query;
			}
			else
			{
				len = query_context->rewritten_length;
				string = query_context->rewritten_query;
			}
		}

		per_node_statement_log(backend, i, string);

		send_simplequery_message(CONNECTION(backend, i), len, string, MAJOR(backend));
	}

	/* Wait for response */
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;
		else if (send_type < 0 && i == node_id)
			continue;
		else if (send_type > 0 && i != node_id)
			continue;

#ifdef NOT_USED
		/*
		 * If in master/slave mode, we do not send COMMIT/ABORT to
		 * slaves/standbys if it's in I(idle) state.
		 */
		if (is_commit && MASTER_SLAVE && !IS_MASTER_NODE_ID(i) && TSTATE(backend, i) == 'I')
		{
			continue;
		}
#endif

		if (is_begin_read_write)
		{
			if(REAL_PRIMARY_NODE_ID == i)
				string = query_context->original_query;
			else
				string = query_context->rewritten_query;
		}
        
        wait_for_query_response_with_trans_cleanup(frontend,
                                                   CONNECTION(backend, i),
                                                   MAJOR(backend),
                                                   MASTER_CONNECTION(backend)->pid,
                                                   MASTER_CONNECTION(backend)->key);
        
		/*
		 * Check if some error detected.  If so, emit
		 * log. This is useful when invalid encoding error
		 * occurs. In this case, PostgreSQL does not report
		 * what statement caused that error and make users
		 * confused.
		 */		
		per_node_error_log(backend, i, string, "pool_send_and_wait: Error or notice message from backend: ", true);
	}

	return POOL_CONTINUE;
}

/*
 * Send extended query and wait for response
 * send_type:
 *  -1: do not send this node_id
 *   0: send to all nodes
 *  >0: send to this node_id
 */
POOL_STATUS pool_extended_send_and_wait(POOL_QUERY_CONTEXT *query_context,
										char *kind, int len, char *contents,
										int send_type, int node_id)
{
	POOL_SESSION_CONTEXT *session_context;
	POOL_CONNECTION *frontend;
	POOL_CONNECTION_POOL *backend;
	bool is_commit;
	bool is_begin_read_write;
	int i;
	int str_len;
	int rewritten_len;
	char *str;
	char *rewritten_begin;

	session_context = pool_get_session_context(false);
	frontend = session_context->frontend;
	backend = session_context->backend;
	is_commit = is_commit_or_rollback_query(query_context->parse_tree);
	is_begin_read_write = false;
	str_len = 0;
	rewritten_len = 0;
	str = NULL;
	rewritten_begin = NULL;

	/*
	 * If the query is BEGIN READ WRITE or
	 * BEGIN ... SERIALIZABLE in master/slave mode,
	 * we send BEGIN to slaves/standbys instead.
	 * original_query which is BEGIN READ WRITE is sent to primary.
	 * rewritten_query which is BEGIN is sent to standbys.
	 */
	if (pool_need_to_treat_as_if_default_transaction(query_context))
	{
		is_begin_read_write = true;

		if (*kind == 'P')
			rewritten_begin = remove_read_write(len, contents, &rewritten_len);
	}

	if (!rewritten_begin)
	{	
		str_len = len;
		str = contents;
	}

	/* Send query */
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;
		else if (send_type < 0 && i == node_id)
			continue;
		else if (send_type > 0 && i != node_id)
			continue;

		/*
		 * If in reset context, we send COMMIT/ABORT to nodes those
		 * are not in I(idle) state.  This will ensure that
		 * transactions are closed.
		 */
		if (is_commit && session_context->reset_context && TSTATE(backend, i) == 'I')
		{
			pool_unset_node_to_be_sent(query_context, i);
			continue;
		}

		if (rewritten_begin)
		{
			if (REAL_PRIMARY_NODE_ID == i)
			{
				str = contents;
				str_len = len;
			}
			else
			{
				str = rewritten_begin;
				str_len = rewritten_len;
			}
		}

		if (pool_config->log_per_node_statement)
		{
			char msgbuf[QUERY_STRING_BUFFER_LEN];
			char *stmt;

			if (*kind == 'P' || *kind == 'E')
			{
				if (query_context->rewritten_query)
				{
					if (is_begin_read_write)
					{
						if (REAL_PRIMARY_NODE_ID == i)
							stmt = query_context->original_query;
						else
							stmt = query_context->rewritten_query;
					}
					else
					{
						stmt = query_context->rewritten_query;
					}
				}
				else
				{
					stmt = query_context->original_query;
				}

				if (*kind == 'P')
					snprintf(msgbuf, sizeof(msgbuf), "Parse: %s", stmt);
				else
					snprintf(msgbuf, sizeof(msgbuf), "Execute: %s", stmt);
			}
			else
			{
				snprintf(msgbuf, sizeof(msgbuf), "%c message", *kind);
			}

			per_node_statement_log(backend, i, msgbuf);
		}

		send_extended_protocol_message(backend, i, kind, str_len, str);
	}

	if (!is_begin_read_write)
	{
		if (query_context->rewritten_query)
			str = query_context->rewritten_query;
		else
			str = query_context->original_query;
	}

	/* Wait for response */
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;
		else if (send_type < 0 && i == node_id)
			continue;
		else if (send_type > 0 && i != node_id)
			continue;

		/*
		 * If in master/slave mode, we do not send COMMIT/ABORT to
		 * slaves/standbys if it's in I(idle) state.
		 */
		if (is_commit && MASTER_SLAVE && !IS_MASTER_NODE_ID(i) && TSTATE(backend, i) == 'I')
		{
			continue;
		}

		if (is_begin_read_write)
		{
			if (REAL_PRIMARY_NODE_ID == i)
				str = query_context->original_query;
			else
				str = query_context->rewritten_query;
		}

        wait_for_query_response_with_trans_cleanup(frontend,
                                                   CONNECTION(backend, i),
                                                   MAJOR(backend),
                                                   MASTER_CONNECTION(backend)->pid,
                                                   MASTER_CONNECTION(backend)->key);

		/*
		 * Check if some error detected.  If so, emit
		 * log. This is useful when invalid encoding error
		 * occurs. In this case, PostgreSQL does not report
		 * what statement caused that error and make users
		 * confused.
		 */		
		per_node_error_log(backend, i, str, "pool_send_and_wait: Error or notice message from backend: ", true);
	}

	if(rewritten_begin)
        pfree(rewritten_begin);
	return POOL_CONTINUE;
}

/*
 * From syntactically analysis decide the statement to be sent to the
 * primary, the standby or either or both in master/slave+HR/SR mode.
 */
static POOL_DEST send_to_where(Node *node, char *query)

{
/* From storage/lock.h */
#define NoLock					0
#define AccessShareLock			1		/* SELECT */
#define RowShareLock			2		/* SELECT FOR UPDATE/FOR SHARE */
#define RowExclusiveLock		3		/* INSERT, UPDATE, DELETE */
#define ShareUpdateExclusiveLock 4		/* VACUUM (non-FULL),ANALYZE, CREATE
										 * INDEX CONCURRENTLY */
#define ShareLock				5		/* CREATE INDEX (WITHOUT CONCURRENTLY) */
#define ShareRowExclusiveLock	6		/* like EXCLUSIVE MODE, but allows ROW
										 * SHARE */
#define ExclusiveLock			7		/* blocks ROW SHARE/SELECT...FOR
										 * UPDATE */
#define AccessExclusiveLock		8		/* ALTER TABLE, DROP TABLE, VACUUM
										 * FULL, and unqualified LOCK TABLE */

/* From 9.0 include/nodes/node.h */
	static NodeTag nodemap[] = {
		T_PlannedStmt,
		T_InsertStmt,
		T_DeleteStmt,
		T_UpdateStmt,
		T_SelectStmt,
		T_AlterTableStmt,
		T_AlterTableCmd,
		T_AlterDomainStmt,
		T_SetOperationStmt,
		T_GrantStmt,
		T_GrantRoleStmt,
		/*
		T_AlterDefaultPrivilegesStmt,	Our parser does not support yet
		*/
		T_ClosePortalStmt,
		T_ClusterStmt,
		T_CopyStmt,
		T_CreateStmt,	/* CREATE TABLE */
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
		T_TransactionStmt,
		T_ViewStmt,		/* CREATE VIEW */
		T_LoadStmt,
		T_CreateDomainStmt,
		T_CreatedbStmt,
		T_DropdbStmt,
		T_VacuumStmt,
		T_ExplainStmt,
		T_CreateSeqStmt,
		T_AlterSeqStmt,
		T_VariableSetStmt,		/* SET */
		T_VariableShowStmt,
		T_DiscardStmt,
		T_CreateTrigStmt,
		T_CreatePLangStmt,
		T_CreateRoleStmt,
		T_AlterRoleStmt,
		T_DropRoleStmt,
		T_LockStmt,
		T_ConstraintsSetStmt,
		T_ReindexStmt,
		T_CheckPointStmt,
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
		T_DeallocateStmt,		/* DEALLOCATE */
		T_DeclareCursorStmt,	/* DECLARE */
		T_CreateTableSpaceStmt,
		T_DropTableSpaceStmt,
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

	if (bsearch(&nodeTag(node), nodemap, sizeof(nodemap)/sizeof(nodemap[0]),
				sizeof(NodeTag), compare) != NULL)
	{
		/*
		 * SELECT INTO
		 * SELECT FOR SHARE or UPDATE
		 */
		if (IsA(node, SelectStmt))
		{
			/* SELECT INTO or SELECT FOR SHARE or UPDATE ? */
			if (pool_has_insertinto_or_locking_clause(node))
				return POOL_PRIMARY;

			return POOL_EITHER;
		}

		/*
		 * COPY FROM
		 */
		else if (IsA(node, CopyStmt))
		{
			return (((CopyStmt *)node)->is_from)?POOL_PRIMARY:POOL_EITHER;
		}

		/*
		 * LOCK
		 */
		else if (IsA(node, LockStmt))
		{
			return (((LockStmt *)node)->mode >= RowExclusiveLock)?POOL_PRIMARY:POOL_BOTH;
		}

		/*
		 * Transaction commands
		 */
		else if (IsA(node, TransactionStmt))
		{
			/*
			 * Check "BEGIN READ WRITE" "START TRANSACTION READ WRITE"
			 */
			if (is_start_transaction_query(node))
			{
				/* But actually, we send BEGIN to standby if it's
				   BEGIN READ WRITE or START TRANSACTION READ WRITE */
				if (is_read_write((TransactionStmt *)node))
					return POOL_BOTH;
				/* Other TRANSACTION start commands are sent to both primary
				   and standby */
				else
					return POOL_BOTH;
			}
			/* SAVEPOINT related commands are sent to both primary and standby */
			else if (is_savepoint_query(node))
				return POOL_BOTH;
			/*
			 * 2PC commands
			 */
			else if (is_2pc_transaction_query(node))
				return POOL_PRIMARY;
			else
				/* COMMIT etc. */
				return POOL_BOTH;
		}

		/*
		 * SET
		 */
		else if (IsA(node, VariableSetStmt))
		{
			ListCell   *list_item;
			bool ret = POOL_BOTH;

			/*
			 * SET transaction_read_only TO off
			 */
			if (((VariableSetStmt *)node)->kind == VAR_SET_VALUE &&
				!strcmp(((VariableSetStmt *)node)->name, "transaction_read_only"))
			{
				List *options = ((VariableSetStmt *)node)->args;
				foreach(list_item, options)
				{
					A_Const *v = (A_Const *)lfirst(list_item);

					switch (v->val.type)
					{
						case T_String:
							if (!strcasecmp(v->val.val.str, "off") ||
								!strcasecmp(v->val.val.str, "f") ||
								!strcasecmp(v->val.val.str, "false"))
								ret = POOL_PRIMARY;
							break;
						case T_Integer:
							if (v->val.val.ival)
								ret = POOL_PRIMARY;
						default:
							break;
					}
				}
				return ret;
			}

			/* SET TRANSACTION ISOLATION LEVEL SERIALIZABLE or
			 * SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL SERIALIZABLE or
			 * SET transaction_isolation TO 'serializable'
			 * SET default_transaction_isolation TO 'serializable'
			 */
			else if (is_set_transaction_serializable(node))
			{
				return POOL_PRIMARY;
			}

			/*
			 * Check "SET TRANSACTION READ WRITE" "SET SESSION
			 * CHARACTERISTICS AS TRANSACTION READ WRITE"
			 */
			else if (((VariableSetStmt *)node)->kind == VAR_SET_MULTI &&
				(!strcmp(((VariableSetStmt *)node)->name, "TRANSACTION") ||
				 !strcmp(((VariableSetStmt *)node)->name, "SESSION CHARACTERISTICS")))
			{
				List *options = ((VariableSetStmt *)node)->args;
				foreach(list_item, options)
				{
					DefElem *opt = (DefElem *) lfirst(list_item);

					if (!strcmp("transaction_read_only", opt->defname))
					{
						bool read_only;

						read_only = ((A_Const *)opt->arg)->val.val.ival;
						if (!read_only)
							return POOL_PRIMARY;
					}
				}
				return POOL_BOTH;
			}
			else
			{
				/*
				 * All other SET command sent to both primary and
				 * standby
				 */
				return POOL_BOTH;
			}
		}

		/*
		 * DISCARD
		 */
		else if (IsA(node, DiscardStmt))
		{
			return POOL_BOTH;
		}

		/*
		 * PREPARE
		 */
		else if (IsA(node, PrepareStmt))
		{
			PrepareStmt *prepare_statement = (PrepareStmt *)node;

			char *string = nodeToString(prepare_statement->query);

			/* Note that this is a recursive call */
			return send_to_where((Node *)(prepare_statement->query), string);
		}

		/*
		 * EXECUTE
		 */
		else if (IsA(node, ExecuteStmt))
		{
			/* This is temporary decision. where_to_send will inherit
			 *  same destination AS PREPARE.
			 */
			return POOL_PRIMARY; 
		}

		/*
		 * DEALLOCATE
		 */
		else if (IsA(node, DeallocateStmt))
		{
			/* This is temporary decision. where_to_send will inherit
			 *  same destination AS PREPARE.
			 */
			return POOL_PRIMARY; 
		}

		/*
		 * Other statements are sent to primary
		 */
		return POOL_PRIMARY;
	}

	/*
	 * All unknown statements are sent to primary
	 */
	return POOL_PRIMARY;
}

static
void where_to_send_deallocate(POOL_QUERY_CONTEXT *query_context, Node *node)
{
	DeallocateStmt *d = (DeallocateStmt *)node;
	POOL_SENT_MESSAGE *msg;

	/* DEALLOCATE ALL? */
	if (d->name == NULL)
	{
		pool_setall_node_to_be_sent(query_context);
	}
	else
	{
		msg = pool_get_sent_message('Q', d->name);
		if (!msg)
			msg = pool_get_sent_message('P', d->name);
		if (msg)
		{
			/* Inherit same map from PREPARE or PARSE */
			pool_copy_prep_where(msg->query_context->where_to_send,
								 query_context->where_to_send);
			return;
		}
		/* prepared statement was not found */
		pool_setall_node_to_be_sent(query_context);
	}
}

/*
 * Returns parse tree for current query.
 * Precondition: the query is in progress state.
 */
Node *pool_get_parse_tree(void)
{
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context(true);
	if (!sc)
		return NULL;

	if (pool_is_query_in_progress() && sc->query_context)
	{
		return sc->query_context->parse_tree;
	}
	return NULL;
}

/*
 * Returns raw query string for current query.
 * Precondition: the query is in progress state.
 */
char *pool_get_query_string(void)
{
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context(true);
	if (!sc)
		return NULL;

	if (pool_is_query_in_progress() && sc->query_context)
	{
		return sc->query_context->original_query;
	}
	return NULL;
}

/*
 * Return true if the query is:
 * SET TRANSACTION ISOLATION LEVEL SERIALIZABLE or
 * SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL SERIALIZABLE or
 * SET transaction_isolation TO 'serializable'
 * SET default_transaction_isolation TO 'serializable'
 */
bool is_set_transaction_serializable(Node *node)
{
	ListCell   *list_item;

	if (!IsA(node, VariableSetStmt))
		return false;

	if (((VariableSetStmt *)node)->kind == VAR_SET_VALUE &&
		!strcmp(((VariableSetStmt *)node)->name, "transaction_isolation"))
	{
		List *options = ((VariableSetStmt *)node)->args;
		foreach(list_item, options)
		{
			A_Const *v = (A_Const *)lfirst(list_item);

			switch (v->val.type)
			{
				case T_String:
					if (!strcasecmp(v->val.val.str, "serializable"))
						return true;
					break;
				default:
					break;
			}
		}
		return false;
	}

	else if (((VariableSetStmt *)node)->kind == VAR_SET_MULTI &&
			 (!strcmp(((VariableSetStmt *)node)->name, "TRANSACTION") ||
			  !strcmp(((VariableSetStmt *)node)->name, "SESSION CHARACTERISTICS")))
	{
		List *options = ((VariableSetStmt *)node)->args;
		foreach(list_item, options)
		{
			DefElem *opt = (DefElem *) lfirst(list_item);
			if (!strcmp("transaction_isolation", opt->defname) ||
				!strcmp("default_transaction_isolation", opt->defname))
			{
				A_Const *v = (A_Const *)opt->arg;
 
				if (!strcasecmp(v->val.val.str, "serializable"))
					return true;
			}
		}
	}
	return false;
}

/*
 * Returns true if SQL is transaction starting command (START
 * TRANSACTION or BEGIN)
 */
bool is_start_transaction_query(Node *node)
{
	TransactionStmt *stmt;

	if (node == NULL || !IsA(node, TransactionStmt))
		return false;

	stmt = (TransactionStmt *)node;
	return stmt->kind == TRANS_STMT_START || stmt->kind == TRANS_STMT_BEGIN;
}

/*
 * Return true if start transaction query with "READ WRITE" option.
 */
bool is_read_write(TransactionStmt *node)
{
	ListCell   *list_item;

	List *options = node->options;
	foreach(list_item, options)
	{
		DefElem *opt = (DefElem *) lfirst(list_item);

		if (!strcmp("transaction_read_only", opt->defname))
		{
			bool read_only;

			read_only = ((A_Const *)opt->arg)->val.val.ival;
			if (read_only)
				return false;	/* TRANSACTION READ ONLY */
			else
				/*
				 * TRANSACTION READ WRITE specified. This sounds a little bit strange,
				 * but actually the parse code works in the way.
				 */
				return true;
		}
	}

	/*
	 * No TRANSACTION READ ONLY/READ WRITE clause specified.
	 */
	return false;
}

/*
 * Return true if start transaction query with "SERIALIZABLE" option.
 */
bool is_serializable(TransactionStmt *node)
{
	ListCell   *list_item;

	List *options = node->options;
	foreach(list_item, options)
	{
		DefElem *opt = (DefElem *) lfirst(list_item);

		if (!strcmp("transaction_isolation", opt->defname) &&
			IsA(opt->arg, A_Const) &&
			((A_Const *)opt->arg)->val.type == T_String &&
			!strcmp("serializable", ((A_Const *)opt->arg)->val.val.str))
				return true;
	}
	return false;
}

/*
 * If the query is BEGIN READ WRITE or
 * BEGIN ... SERIALIZABLE in master/slave mode,
 * we send BEGIN to slaves/standbys instead.
 * original_query which is BEGIN READ WRITE is sent to primary.
 * rewritten_query which is BEGIN is sent to standbys.
 */
bool pool_need_to_treat_as_if_default_transaction(POOL_QUERY_CONTEXT *query_context)
{
	return (MASTER_SLAVE &&
			is_start_transaction_query(query_context->parse_tree) &&
			(is_read_write((TransactionStmt *)query_context->parse_tree) ||
			 is_serializable((TransactionStmt *)query_context->parse_tree)));
}

/*
 * Return true if the query is SAVEPOINT related query.
 */
bool is_savepoint_query(Node *node)
{
	if (((TransactionStmt *)node)->kind == TRANS_STMT_SAVEPOINT ||
		((TransactionStmt *)node)->kind == TRANS_STMT_ROLLBACK_TO ||
		((TransactionStmt *)node)->kind == TRANS_STMT_RELEASE)
		return true;

	return false;
}

/*
 * Return true if the query is 2PC transaction query.
 */
bool is_2pc_transaction_query(Node *node)
{
	if (((TransactionStmt *)node)->kind == TRANS_STMT_PREPARE ||
		((TransactionStmt *)node)->kind == TRANS_STMT_COMMIT_PREPARED ||
		((TransactionStmt *)node)->kind == TRANS_STMT_ROLLBACK_PREPARED)
		return true;

	return false;
}

/*
 * Set query state, if a current state is before it than the specified state.
 */
void pool_set_query_state(POOL_QUERY_CONTEXT *query_context, POOL_QUERY_STATE state)
{
	int i;

	CHECK_QUERY_CONTEXT_IS_VALID;

	for (i = 0; i < NUM_BACKENDS; i++)
	{
		if (query_context->where_to_send[i] &&
			statecmp(query_context->query_state[i], state) < 0)
			query_context->query_state[i] = state;
	}
}

/*
 * Return -1, 0 or 1 according to s1 is "before, equal or after" s2 in terms of state
 * transition order. 
 * The State transition order is defined as: UNPARSED < PARSE_COMPLETE < BIND_COMPLETE < EXECUTE_COMPLETE
 */
int statecmp(POOL_QUERY_STATE s1, POOL_QUERY_STATE s2)
{
	int ret;

	switch (s2) {
		case POOL_UNPARSED:
			ret = (s1 == s2) ? 0 : 1;
			break;
		case POOL_PARSE_COMPLETE:
			if (s1 == POOL_UNPARSED)
				ret = -1;
			else
				ret = (s1 == s2) ? 0 : 1;
			break;
		case POOL_BIND_COMPLETE:
			if (s1 == POOL_UNPARSED || s1 == POOL_PARSE_COMPLETE)
				ret = -1;
			else
				ret = (s1 == s2) ? 0 : 1;
			break;
		case POOL_EXECUTE_COMPLETE:
			ret = (s1 == s2) ? 0 : -1;
			break;
		default:
			ret = -2;
			break;
	}

	return ret;
}

/*
 * Remove READ WRITE option from the packet of START TRANSACTION command.
 * To free the return value is required. 
 */
static
char* remove_read_write(int len, const char* contents, int *rewritten_len)
{
	char *rewritten_query;
	char *rewritten_contents;
	const char *name;
	const char *stmt;

	rewritten_query = "BEGIN";
	name = contents;
	stmt = contents + strlen(name) + 1;

	*rewritten_len = len - strlen(stmt) + strlen(rewritten_query);
	if (len < *rewritten_len)
	{
        ereport(ERROR,
            (errmsg("invalid message length of transaction packet")));
	}

	rewritten_contents = palloc(*rewritten_len);

	strcpy(rewritten_contents, name);
	strcpy(rewritten_contents + strlen(name) + 1, rewritten_query);
	memcpy(rewritten_contents + strlen(name) + strlen(rewritten_query) + 2,
		   stmt + strlen(stmt) + 1,
		   len - (strlen(name) + strlen(stmt) + 2));

	return rewritten_contents;
}

/*
 * Return true if current query is safe to cache.
 */
bool pool_is_cache_safe(void)
{
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context(true);
	if (!sc)
		return false;

	if (pool_is_query_in_progress() && sc->query_context)
	{
		return sc->query_context->is_cache_safe;
	}
	return false;
}

/*
 * Set safe to cache.
 */
void pool_set_cache_safe(void)
{
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context(true);
	if (!sc)
		return;

	if (sc->query_context)
	{
		sc->query_context->is_cache_safe = true;
	}
}

/*
 * Unset safe to cache.
 */
void pool_unset_cache_safe(void)
{
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context(true);
	if (!sc)
		return;

	if (sc->query_context)
	{
		sc->query_context->is_cache_safe = false;
	}
}

/*
 * Return true if current temporary query cache is exceeded
 */
bool pool_is_cache_exceeded(void)
{
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context(true);
	if (!sc)
		return false;

	if (pool_is_query_in_progress() && sc->query_context)
	{
		if (sc->query_context->temp_cache)
			return sc->query_context->temp_cache->is_exceeded;
		return true;
	}
	return false;
}

/*
 * Set current temporary query cache is exceeded
 */
void pool_set_cache_exceeded(void)
{
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context(true);
	if (!sc)
		return;

	if (sc->query_context && sc->query_context->temp_cache)
	{
		sc->query_context->temp_cache->is_exceeded = true;
	}
}

/*
 * Unset current temporary query cache is exceeded
 */
void pool_unset_cache_exceeded(void)
{
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context(true);
	if (!sc)
		return;

	if (sc->query_context && sc->query_context->temp_cache)
	{
		sc->query_context->temp_cache->is_exceeded = false;
	}
}
