/* -*-pgsql-c-*- */
/*
 *
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
 */
#include "pool.h"
#include "pool_config.h"
#include "pool_proto_modules.h"
#include "pool_session_context.h"
#include "pool_query_context.h"
#include "parser/nodes.h"

#include <string.h>
#include <netinet/in.h>

/*
 * Create and initialize per query session context
 */
POOL_QUERY_CONTEXT *pool_init_query_context(void)
{
	POOL_QUERY_CONTEXT *qc;

	qc = calloc(1, sizeof(*qc));
	if (!qc)
	{
		pool_error("pool_init_query_context: cannot allocate memory");
		return NULL;
	}

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
		session_context = pool_get_session_context();
		session_context->in_progress = false;
		session_context->query_context = NULL;
		free(query_context);
	}
}

/*
 * Start query
 */
void pool_start_query(POOL_QUERY_CONTEXT *query_context, char *query, Node *node)
{
	POOL_SESSION_CONTEXT *session_context;

	if (query_context)
	{
		session_context = pool_get_session_context();
		query_context->original_query = query;
		query_context->parse_tree = node;
		session_context->in_progress = true;
		session_context->query_context = query_context;
	}
}

/*
 * Specify DB node to send query
 */
void pool_set_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id)
{
	if (!query_context)
	{
		pool_error("pool_set_node_to_be_sent: no query context");
		return;
	}

	if (node_id < 0 || node_id > MAX_NUM_BACKENDS)
	{
		pool_error("pool_set_node_to_be_sent: invalid ndoe id:%d", node_id);
		return;
	}

	query_context->where_to_send[node_id] = true;
	
	return;
}

/*
 * Unspecify DB node to send query
 */
void pool_unset_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id)
{
	if (!query_context)
	{
		pool_error("pool_unset_node_to_be_sent: no query context");
		return;
	}

	if (node_id < 0 || node_id > MAX_NUM_BACKENDS)
	{
		pool_error("pool_unset_node_to_be_sent: invalid ndoe id:%d", node_id);
		return;
	}

	query_context->where_to_send[node_id] = false;
	
	return;
}

/*
 * Clear DB node map
 */
void pool_clear_node_to_be_sent(POOL_QUERY_CONTEXT *query_context)
{
	if (!query_context)
	{
		pool_error("pool_clear_node_to_be_sent: no query context");
		return;
	}
	memset(query_context->where_to_send, false, sizeof(query_context->where_to_send));
	return;
}

/*
 * Set all DB node map entry
 */
void pool_setall_node_to_be_sent(POOL_QUERY_CONTEXT *query_context)
{
	if (!query_context)
	{
		pool_error("pool_setall_node_to_be_sent: no query context");
		return;
	}
	memset(query_context->where_to_send, true, sizeof(query_context->where_to_send));
	return;
}

/*
 * Return if the DB node is needed to send query
 */
bool pool_is_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id)
{
	if (!query_context)
	{
		pool_error("pool_is_node_to_be_sent: no query context");
		return false;
	}

	if (node_id < 0 || node_id > MAX_NUM_BACKENDS)
	{
		pool_error("pool_is_node_to_be_sent: invalid ndoe id:%d", node_id);
		return false;
	}

	return query_context->where_to_send[node_id];
}

/*
 * Return if the DB node is needed to send query.
 * Intended to be called from VALID_BACKEND
 */
bool pool_is_node_to_be_sent_in_current_query(int node_id)
{
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context();
	if (!sc)
		return true;

	if (sc->in_progress && sc->query_context)
	{
		return pool_is_node_to_be_sent(sc->query_context, node_id);
	}
	return true;
}

/*
 * Decide where to send queries(thus expecting response)
 */
void pool_where_to_send(POOL_QUERY_CONTEXT *query_context, char *query, Node *node)
{
	POOL_SESSION_CONTEXT *session_context;
	POOL_CONNECTION_POOL *backend;

	if (!query_context)
	{
		pool_error("pool_where_to_send: no query context");
		return;
	}

	session_context = pool_get_session_context();
	backend = session_context->backend;

	/*
	 * Zap out DB node map
	 */
	pool_clear_node_to_be_sent(query_context);

	/*
	 * In raw mode, we send only to master node. Simple enough.
	 */
	if (RAW_MODE)
	{
		pool_set_node_to_be_sent(query_context, MASTER_NODE_ID);
	}
	else if (MASTER_SLAVE)
	{
		/*
		 * PREPARE, SET, DEALLOCATE and DISCARD statements must be
		 * replicated even if we are in master/slave mode.
		 */
		if (IsA(node, PrepareStmt) || IsA(node, DeallocateStmt) ||
			IsA(node, VariableSetStmt) || IsA(node, DiscardStmt))
		{
			pool_setall_node_to_be_sent(query_context);
		}
		else
		{
			if (pool_config->load_balance_mode &&
				MAJOR(backend) == PROTO_MAJOR_V3 &&
				TSTATE(backend) == 'I' &&
				is_select_query(node, query) &&
				!is_sequence_query(node))
			{
				/* load balance */
				pool_set_node_to_be_sent(query_context,
										 session_context->load_balance_node_id);
			}
			else
			{
				/* only send to master node */
				pool_set_node_to_be_sent(query_context, MASTER_NODE_ID);
			}
		}
	}
	else if (REPLICATION|PARALLEL_MODE)
	{
		if (pool_config->load_balance_mode &&
			MAJOR(backend) == PROTO_MAJOR_V3 &&
			TSTATE(backend) == 'I' &&
			is_select_query(node, query) &&
			!is_sequence_query(node))
		{
			/* load balance */
			pool_set_node_to_be_sent(query_context,
									 session_context->load_balance_node_id);
		}
		else if (REPLICATION &&
				 !pool_config->replicate_select &&
				 is_select_query(node, query) &&
				 !is_sequence_query(node))
		{
			/* only send to master node */
			pool_set_node_to_be_sent(query_context, MASTER_NODE_ID);
		}
		else
		{
			/* send to all nodes */
			pool_setall_node_to_be_sent(query_context);
		}
	}
	else
	{
		pool_error("pool_where_to_send: unknown mode");
	}
	return;
}


/*
 * Send query and wait for response
 * send_type:
 *  -1: do not send this node_id
 *	0: send to all nodes
 *  >0: send to this node_id
 */
POOL_STATUS pool_send_and_wait(POOL_QUERY_CONTEXT *query_context, char *query, int len,
							   int send_type, int node_id)
{
	POOL_SESSION_CONTEXT *session_context;
	POOL_CONNECTION *frontend;
	POOL_CONNECTION_POOL *backend;

	int i;

	session_context = pool_get_session_context();
	frontend = session_context->frontend;
	backend = session_context->backend;

	/* Send query */
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;
		else if (send_type < 0 && i == node_id)
			continue;
		else if (send_type > 0 && i != node_id)
			continue;

		per_node_statement_log(backend, i, query);

		if (send_simplequery_message(CONNECTION(backend, i), len, query, MAJOR(backend)) != POOL_CONTINUE)
		{
			return POOL_END;
		}
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

		if (wait_for_query_response(frontend, CONNECTION(backend, i), query, MAJOR(backend)) != POOL_CONTINUE)
		{
			/* Cancel current transaction */
			CancelPacket cancel_packet;

			cancel_packet.protoVersion = htonl(PROTO_CANCEL);
			cancel_packet.pid = MASTER_CONNECTION(backend)->pid;
			cancel_packet.key= MASTER_CONNECTION(backend)->key;
			cancel_request(&cancel_packet);

			return POOL_END;
		}

		/*
		 * Check if some error detected.  If so, emit
		 * log. This is usefull when invalid encoding error
		 * occurs. In this case, PostgreSQL does not report
		 1* what statement caused that error and make users
		 * confused.
		 */
		per_node_error_log(backend, i, query, "pool_send_and_wait: Error or notice message from backend: ", true);
	}
	return POOL_CONTINUE;
}
