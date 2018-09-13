/* -*-pgsql-c-*- */
/*
 * $Header$
 * 
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2017	PgPool Global Development Group
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
 * This file contains modules which process the "Command Complete" and "Empty
 * query response" message sent from backend. The main function is
 * "CommandComplete".
 *---------------------------------------------------------------------
 */
#include <string.h>
#include <arpa/inet.h>

#include "pool.h"
#include "protocol/pool_proto_modules.h"
#include "parser/pg_config_manual.h"
#include "parser/pool_string.h"
#include "pool_config.h"
#include "context/pool_session_context.h"
#include "context/pool_query_context.h"
#include "utils/elog.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/pool_stream.h"

static int extract_ntuples(char *message);
static POOL_STATUS handle_mismatch_tuples(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char *packet, int packetlen, bool command_complete);
static int foward_command_complete(POOL_CONNECTION *frontend, char *packet, int packetlen);
static int foward_empty_query(POOL_CONNECTION *frontend, char *packet, int packetlen);
static int foward_packet_to_frontend(POOL_CONNECTION *frontend, char kind, char *packet, int packetlen);

POOL_STATUS CommandComplete(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, bool command_complete)
{
	int len, len1;
	char *p, *p1;
	int i;
	POOL_SESSION_CONTEXT *session_context;
	POOL_CONNECTION	*con;

	p1 = NULL;
	len1 = 0;

	/* Get session context */
	session_context = pool_get_session_context(false);

	/*
	 * Handle misc process which is neccessary when query context exists.
	 */
	if (session_context->query_context != NULL && (!STREAM || (SL_MODE && !pool_is_doing_extended_query_message())))
		handle_query_context(backend);

	/*
	 * If operated in streaming replication mode and doing an extended query,
	 * read backend message according to the query context.
	 * Also we set the transaction state at this point.
	 */
	if (STREAM && pool_is_doing_extended_query_message())
	{
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i))
			{
				con = CONNECTION(backend, i);

				if (pool_read(con, &len, sizeof(len)) < 0)
					return POOL_END;

				len = ntohl(len);
				len -= 4;
				len1 = len;

				p = pool_read2(con, len);
				if (p == NULL)
					return POOL_END;
				p1 = palloc(len);
				memcpy(p1, p, len);

				if (session_context->query_context &&
					session_context->query_context->parse_tree &&
					is_start_transaction_query(session_context->query_context->parse_tree))
					TSTATE(backend, i) ='T';		/* we are inside a transaction */
				{
					ereport(DEBUG1,
							(errmsg("processing command complete"),
							 errdetail("set transaction state to T")));
				}
			}
		}
	}
	/*
	 * Otherwise just read from master node.
	 */
	else
	{
		con = MASTER(backend);

		if (pool_read(con, &len, sizeof(len)) < 0)
			return POOL_END;

		len = ntohl(len);
		len -= 4;
		len1 = len;

		p = pool_read2(con, len);
		if (p == NULL)
			return POOL_END;
		p1 = palloc(len);
		memcpy(p1, p, len);
	}

	/*
	 * If operated in streaming replication mode and extended query mode, just
	 * forward the packet to frontend and we are done. Otherwise, we need to
	 * do mismatch tuples process (forwarding to frontend is done in
	 * handle_mismatch_tuples().
	 */
	if (STREAM && pool_is_doing_extended_query_message())
	{
		int status;

		if (command_complete)
			status = foward_command_complete(frontend, p1, len1);
		else
			status = foward_empty_query(frontend, p1, len1);

		if (status < 0)
			return POOL_END;
	}
	else
	{
		if (handle_mismatch_tuples(frontend, backend, p1, len1, command_complete) != POOL_CONTINUE)
			return POOL_END;
	}

	/* Save the received result to buffer for each kind */
	if (pool_config->memory_cache_enabled)
	{
		if (pool_is_cache_safe() && !pool_is_cache_exceeded())
		{
			memqcache_register('C', frontend, p1, len1);
		}

		/*
		 * If we are in streaming replication mode and we are doing extended
		 * query, register query cache now.
		 */
		if (STREAM && pool_is_doing_extended_query_message())
		{
			char *query;
			Node *node;
			char state;

			query = session_context->query_context->query_w_hex;
			node = pool_get_parse_tree();
			state = TSTATE(backend, MASTER_NODE_ID);
			pool_handle_query_cache(backend, query, node, state);
		}
	}

	pfree(p1);

	if (pool_is_doing_extended_query_message() && pool_is_query_in_progress())
	{
		pool_set_query_state(session_context->query_context, POOL_EXECUTE_COMPLETE);
	}

	/*
	 * If we are in streaming replication mode and we are doing extended
	 * query, reset query in progress flag and prevoius pending message.
	*/
	if (STREAM && pool_is_doing_extended_query_message())
	{
		pool_at_command_success(frontend, backend);
		pool_unset_query_in_progress();
		pool_pending_message_reset_previous_message();
	}

	return POOL_CONTINUE;
}

/*
 * Handle misc process which is neccessary when query context exists.
 */
void handle_query_context(POOL_CONNECTION_POOL *backend)
{
	POOL_SESSION_CONTEXT *session_context;
	Node *node;

	/* Get session context */
	session_context = pool_get_session_context(false);

	node = session_context->query_context->parse_tree;

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

/*
 * Handle mismatch tuples
 */
static POOL_STATUS handle_mismatch_tuples(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, char *packet, int packetlen, bool command_complete)
{
	POOL_SESSION_CONTEXT *session_context;

	int rows;
	int i;
	int len;
	char *p;

	/* Get session context */
	session_context = pool_get_session_context(false);

	rows = extract_ntuples(packet);

	/*
	 * Save number of affected tuples of master node.
	 */
	session_context->ntuples[MASTER_NODE_ID] = rows;


	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!IS_MASTER_NODE_ID(i))
		{
			if (!VALID_BACKEND(i))
			{
				session_context->ntuples[i] = -1;
				continue;
			}

			pool_read(CONNECTION(backend, i), &len, sizeof(len));

			len = ntohl(len);
			len -= 4;

			p = pool_read2(CONNECTION(backend, i), len);
			if (p == NULL)
				return POOL_END;

			if (len != packetlen)
			{
				ereport(DEBUG1,
					(errmsg("processing command complete"),
						errdetail("length does not match between backends master(%d) %d th backend(%d)",
							   len, i, packetlen)));
			}

			int n = extract_ntuples(p);

			/*
			 * Save number of affected tuples.
			 */
			session_context->ntuples[i] = n;

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

	if (session_context->mismatch_ntuples)
	{
		char msgbuf[128];

		String *msg = init_string("pgpool detected difference of the number of inserted, updated or deleted tuples. Possible last query was: \"");
		string_append_char(msg, query_string_buffer);
		string_append_char(msg, "\"");
		pool_send_error_message(frontend, MAJOR(backend),
								"XX001", msg->data, "",
								"check data consistency between master and other db node",  __FILE__, __LINE__);
		ereport(LOG,
			(errmsg("%s", msg->data)));
		free_string(msg);

		msg = init_string("CommandComplete: Number of affected tuples are:");

		for (i=0;i<NUM_BACKENDS;i++)
		{
			snprintf(msgbuf, sizeof(msgbuf), " %d", session_context->ntuples[i]);
			string_append_char(msg, msgbuf);
		}
		ereport(LOG,
			(errmsg("processing command complete"),
				 errdetail("%s", msg->data)));

		free_string(msg);
	}
	else
	{
		if (command_complete)
		{
			if (foward_command_complete(frontend, packet, packetlen) < 0)
				return POOL_END;
		}
		else
		{
			if (foward_empty_query(frontend, packet, packetlen) < 0)
				return POOL_END;
		}
	}

	return POOL_CONTINUE;
}

/*
 * Forward Command complete packet to frontend
 */
static int foward_command_complete(POOL_CONNECTION *frontend, char *packet, int packetlen)
{
	return foward_packet_to_frontend(frontend, 'C', packet, packetlen);
}

/*
 * Forward Empty query response to frontend
 */
static int foward_empty_query(POOL_CONNECTION *frontend, char *packet, int packetlen)
{
	return foward_packet_to_frontend(frontend, 'I', packet, packetlen);
}

/*
 * Forward packet to frontend
 */
static int foward_packet_to_frontend(POOL_CONNECTION *frontend, char kind, char *packet, int packetlen)
{
	int sendlen;

	if (pool_write(frontend, &kind, 1) < 0)
		return -1;

	sendlen = htonl(packetlen+4);
	if (pool_write(frontend, &sendlen, sizeof(sendlen)) < 0)
		return -1;

	pool_write_and_flush(frontend, packet, packetlen);

	return 0;
}
