/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2025	PgPool Global Development Group
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
#include "protocol/pool_process_query.h"
#include "parser/pg_config_manual.h"
#include "pool_config.h"
#include "context/pool_session_context.h"
#include "context/pool_query_context.h"
#include "utils/elog.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/pool_stream.h"

static int	extract_ntuples(char *message);
static POOL_STATUS handle_mismatch_tuples(POOL_CONNECTION * frontend, POOL_CONNECTION_POOL * backend, char *packet, int packetlen, bool command_complete);
static int	forward_command_complete(POOL_CONNECTION * frontend, char *packet, int packetlen);
static int	forward_empty_query(POOL_CONNECTION * frontend, char *packet, int packetlen);
static int	forward_packet_to_frontend(POOL_CONNECTION * frontend, char kind, char *packet, int packetlen);
static void process_clear_cache(POOL_CONNECTION_POOL * backend);
static void clear_query_cache(void);
static bool check_alter_role_statement(AlterRoleStmt *stmt);

POOL_STATUS
CommandComplete(POOL_CONNECTION * frontend, POOL_CONNECTION_POOL * backend, bool command_complete)
{
	int			len,
				len1;
	char	   *p,
			   *p1;
	int			i;
	POOL_SESSION_CONTEXT *session_context;
	POOL_CONNECTION *con;

	p1 = NULL;
	len1 = 0;

	/* Get session context */
	session_context = pool_get_session_context(false);

	/*
	 * Handle misc process which is necessary when query context exists.
	 */
	if (session_context->query_context != NULL && (!SL_MODE || (SL_MODE && !pool_is_doing_extended_query_message())))
		handle_query_context(backend);

	/*
	 * If operated in streaming replication mode and doing an extended query,
	 * read backend message according to the query context. Also we set the
	 * transaction state at this point.
	 */
	if (SL_MODE && pool_is_doing_extended_query_message())
	{
		p1 = NULL;

		for (i = 0; i < NUM_BACKENDS; i++)
		{
			if (VALID_BACKEND(i))
			{
				con = CONNECTION(backend, i);

				if (pool_read(con, &len, sizeof(len)) < 0)
				{
					if (p1 != NULL)
						pfree(p1);
					return POOL_END;
				}

				len = ntohl(len);
				len -= 4;
				len1 = len;

				p = pool_read2(con, len);
				if (p == NULL)
				{
					if (p1 != NULL)
						pfree(p1);
					return POOL_END;
				}
				if (p1 != NULL)
					pfree(p1);
				p1 = palloc(len);
				memcpy(p1, p, len);

				if (session_context->query_context &&
					session_context->query_context->parse_tree &&
					is_start_transaction_query(session_context->query_context->parse_tree))
				{
					TSTATE(backend, i) = 'T';	/* we are inside a transaction */
					ereport(DEBUG1,
							(errmsg("processing command complete"),
							 errdetail("set transaction state to T")));
				}
			}
		}
	}

	/*
	 * Otherwise just read from main node.
	 */
	else
	{
		con = MAIN(backend);

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
	 * Check if a transaction start command was issued by a multi statement
	 * query by looking at the command tag. We cannot look into the parse tree
	 * because our parse tree is only the first query in the multi statement.
	 */
	if (SL_MODE && session_context->query_context &&
		session_context->query_context->is_multi_statement)
	{
		if (!strcmp(p1, "BEGIN"))
		{
			/*
			 * If the query was a transaction starting command, remember it
			 * until it gets committed or roll backed.
			 */
			elog(DEBUG1, "Call set_tx_started_by_multi_statement_query() in CommandComplete");
			set_tx_started_by_multi_statement_query();
		}
		else if (!strcmp(p1, "COMMIT") || !strcmp(p1, "ROLLBACK"))
		{
			/*
			 * It is possible that the multi statement query included both
			 * BEGIN and COMMIT/ROLLBACK, or just COMMIT/ROLLBACK command.  In
			 * this case we forget that a transaction was started by multi
			 * statement query.
			 */
			elog(DEBUG1, "Call unset_tx_started_by_multi_statement_query() in CommandComplete");
			unset_tx_started_by_multi_statement_query();
		}
	}

	/*
	 * If operated in streaming replication mode and extended query mode, just
	 * forward the packet to frontend and we are done. Otherwise, we need to
	 * do mismatch tuples process (forwarding to frontend is done in
	 * handle_mismatch_tuples().
	 */
	if (SL_MODE && pool_is_doing_extended_query_message())
	{
		int			status;

		if (p1 == NULL)
		{
			elog(WARNING, "CommandComplete: expected p1 is not NULL");
			return POOL_END;
		}

		if (command_complete)
			status = forward_command_complete(frontend, p1, len1);
		else
			status = forward_empty_query(frontend, p1, len1);

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
		 * If we are doing extended query, register the SELECT result to temp
		 * query cache now.
		 */
		if (pool_is_doing_extended_query_message())
		{
			char	   *query;
			Node	   *node;
			char		state;

			if (session_context->query_context == NULL)
			{
				elog(WARNING, "expected query_contex is NULL");
				return POOL_END;
			}
			query = session_context->query_context->query_w_hex;
			node = pool_get_parse_tree();
			state = TSTATE(backend, MAIN_NODE_ID);

			/*
			 * If some rows have been fetched by an execute with non 0 row option,
			 * we do not create cache.
			 */ 
			pool_handle_query_cache(backend, query, node, state,
									session_context->query_context->partial_fetch);

			/*
			 * CommandComplete guarantees that all rows have been fetched.  We
			 * can unconditionally set atEnd flag.
			 */
			session_context->query_context->atEnd = true;
		}
	}

	pfree(p1);

	if (pool_is_doing_extended_query_message() && pool_is_query_in_progress())
	{
		pool_set_query_state(session_context->query_context, POOL_EXECUTE_COMPLETE);
	}

	/*
	 * If we are in streaming replication mode and we are doing extended
	 * query, reset query in progress flag and previous pending message.
	 */
	if (SL_MODE && pool_is_doing_extended_query_message())
	{
		pool_at_command_success(frontend, backend);
		pool_unset_query_in_progress();
		pool_pending_message_reset_previous_message();

		if (session_context->query_context == NULL)
		{
			elog(WARNING, "At command complete there's no query context");
		}
		else
		{
			/*
			 * Destroy query context if it is not referenced from sent
			 * messages and pending messages except bound to named statements
			 * or portals.  Named statements and portals should remain until
			 * they are explicitly closed.
			 */
			if (can_query_context_destroy(session_context->query_context))

			{
				POOL_SENT_MESSAGE * msg = pool_get_sent_message_by_query_context(session_context->query_context);

				if (!msg || (msg && *msg->name == '\0'))
				{
					pool_zap_query_context_in_sent_messages(session_context->query_context);
					pool_query_context_destroy(session_context->query_context);
				}
			}
		}
	}

	return POOL_CONTINUE;
}

/*
 * Handle misc process which is necessary when query context exists.
 */
void
handle_query_context(POOL_CONNECTION_POOL * backend)
{
	POOL_SESSION_CONTEXT *session_context;
	Node	   *node;

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
		char	   *name;

		name = ((DeallocateStmt *) node)->name;
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
		DiscardStmt *stmt = (DiscardStmt *) node;

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
	 * JDBC driver sends "BEGIN" query internally if setAutoCommit(false).
	 * But it does not send Sync message after "BEGIN" query.  In extended
	 * query protocol, PostgreSQL returns ReadyForQuery when a client sends
	 * Sync message.  Problem is, pgpool can't know the transaction state
	 * without receiving ReadyForQuery. So we remember that we need to send
	 * Sync message internally afterward, whenever we receive BEGIN in
	 * extended protocol.
	 */
	else if (IsA(node, TransactionStmt))
	{
		TransactionStmt *stmt = (TransactionStmt *) node;

		if (stmt->kind == TRANS_STMT_BEGIN || stmt->kind == TRANS_STMT_START)
		{
			int			i;

			for (i = 0; i < NUM_BACKENDS; i++)
			{
				if (!VALID_BACKEND(i))
					continue;

				TSTATE(backend, i) = 'T';
			}

			if (pool_config->disable_load_balance_on_write != DLBOW_TRANS_TRANSACTION)
				pool_unset_writing_transaction();

			pool_unset_failed_transaction();
			pool_unset_transaction_isolation();
		}
		else if (stmt->kind == 	TRANS_STMT_COMMIT)
		{
			/* Commit ongoing CREATE/DROP temp table status */
			pool_temp_tables_commit_pending();			

			/* Forget a transaction was started by multi statement query */
			unset_tx_started_by_multi_statement_query();

			/*
			 * Query cache enabled and REVOKE was issued in this transaction?
			 */
			if (pool_config->memory_cache_enabled &&
				query_cache_disabled_tx())
			{
				/*
				 * Clear all the query cache.
				 */
				clear_query_cache();

				/* Unset query cache disabled in this transaction */
				unset_query_cache_disabled_tx();
			}
		}
		else if (stmt->kind == TRANS_STMT_ROLLBACK)
		{
			/* Remove ongoing CREATE/DROP temp table status */
			pool_temp_tables_remove_pending();

			/* Forget a transaction was started by multi statement query */
			unset_tx_started_by_multi_statement_query();

			/* Unset query cache disabled in this transaction */
			unset_query_cache_disabled_tx();
		}
	}
	else if (IsA(node, CreateStmt))
	{
		CreateStmt *stmt = (CreateStmt *) node;
		POOL_TEMP_TABLE_STATE	state;

		/* Is this a temporary table? */
		if (stmt->relation->relpersistence == 't')
		{
			if (TSTATE(backend, MAIN_NODE_ID ) == 'T')	/* Are we inside a transaction? */
			{
				state = TEMP_TABLE_CREATING;
			}
			else
			{
				state = TEMP_TABLE_CREATE_COMMITTED;
			}
			ereport(DEBUG1,
					(errmsg("Creating temp table: %s. commit status: %d",
							stmt->relation->relname, state)));
			pool_temp_tables_add(stmt->relation->relname, state);
		}
	}
	else if (IsA(node, DropStmt))
	{
		DropStmt *stmt = (DropStmt *) node;
		POOL_TEMP_TABLE_STATE	state;

		if (stmt->removeType == OBJECT_TABLE)
		{
			/* Loop through stmt->objects */
			ListCell   *cell;
			ListCell   *next;

			if (TSTATE(backend, MAIN_NODE_ID ) == 'T')	/* Are we inside a transaction? */
			{
				state = TEMP_TABLE_DROPPING;
			}
			else
			{
				state = TEMP_TABLE_DROP_COMMITTED;
			}

			for (cell = list_head(session_context->temp_tables); cell; cell = next)
			{
				char *tablename = (char *)lfirst(cell);
				ereport(DEBUG1,
						(errmsg("Dropping temp table: %s", tablename)));
				pool_temp_tables_delete(tablename, state);

				if (!session_context->temp_tables)
					return;

				next = lnext(session_context->temp_tables, cell);
			}
		}
	}
	else if (IsA(node, VariableSetStmt))
	{
		VariableSetStmt *stmt = (VariableSetStmt *) node;

		/* SET ROLE or SESSION AUTHORIZATION command? */
		if (stmt->kind == VAR_SET_VALUE &&
			(!strcmp(stmt->name, "role") ||
			 !strcmp(stmt->name, "session_authorization")))
			/* disable query cache in this session */
			set_query_cache_disabled();
	}
	else if (IsA(node, GrantStmt))
	{
		GrantStmt *stmt = (GrantStmt *) node;

		/* REVOKE? */
		if (stmt->is_grant)
			return;

		/* Clear query cache */
		process_clear_cache(backend);
	}
	else if (IsA(node, AlterTableStmt) || IsA(node, AlterDatabaseStmt) ||
			 IsA(node, AlterDatabaseSetStmt))
	{
		/* Clear query cache */
		process_clear_cache(backend);
	}
	else if (IsA(node, AlterRoleStmt))
	{
		if (check_alter_role_statement(castNode(AlterRoleStmt, node)))
		{
			/* Clear query cache */
			process_clear_cache(backend);
		}
	}
}

/*
 * Check whether the ALTER ROLE statement needs query cache invalidation.
 * stmt must be AlterRoleStmt.
 */
static bool
check_alter_role_statement(AlterRoleStmt *stmt)
{
	ListCell	*l;

	foreach(l, stmt->options)
	{
		DefElem	*elm = (DefElem *) lfirst(l);

		/*
		 * We want to detect other than ALTER ROLE foo WITH PASSWORD or
		 * WITH CONNECTION LIMIT case.  It does not change any privilege of the
		 * role.
		 */
		if (strcmp(elm->defname, "password") &&
			strcmp(elm->defname, "connectionlimit"))
			return true;
	}
	return false;
}

/*
 * Extract the number of tuples from CommandComplete message
 */
static int
extract_ntuples(char *message)
{
	char	   *rows;

	if ((rows = strstr(message, "UPDATE")) || (rows = strstr(message, "DELETE")))
		rows += 7;
	else if ((rows = strstr(message, "INSERT")))
	{
		rows += 7;
		while (*rows && *rows != ' ')
			rows++;
	}
	else
		return 0;

	return atoi(rows);
}

/*
 * Handle mismatch tuples
 */
static POOL_STATUS handle_mismatch_tuples(POOL_CONNECTION * frontend, POOL_CONNECTION_POOL * backend, char *packet, int packetlen, bool command_complete)
{
	POOL_SESSION_CONTEXT *session_context;

	int			rows;
	int			i;
	int			len;
	char	   *p;

	/* Get session context */
	session_context = pool_get_session_context(false);

	rows = extract_ntuples(packet);

	/*
	 * Save number of affected tuples of main node.
	 */
	session_context->ntuples[MAIN_NODE_ID] = rows;


	for (i = 0; i < NUM_BACKENDS; i++)
	{
		if (!IS_MAIN_NODE_ID(i))
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
						 errdetail("length does not match between backends main(%d) %d th backend(%d)",
								   len, i, packetlen)));
			}

			int			n = extract_ntuples(p);

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
		StringInfoData	msg;

		initStringInfo(&msg);
		appendStringInfoString(&msg, "pgpool detected difference of the number of inserted, updated or deleted tuples. Possible last query was: \"");
		appendStringInfoString(&msg, query_string_buffer);
		appendStringInfoString(&msg, "\"");
		pool_send_error_message(frontend, MAJOR(backend),
								"XX001", msg.data, "",
								"check data consistency between main and other db node", __FILE__, __LINE__);
		ereport(LOG,
				(errmsg("%s", msg.data)));

		pfree(msg.data);

		initStringInfo(&msg);
		appendStringInfoString(&msg, "CommandComplete: Number of affected tuples are:");

		for (i = 0; i < NUM_BACKENDS; i++)
			appendStringInfo(&msg, " %d", session_context->ntuples[i]);

		ereport(LOG,
				(errmsg("processing command complete"),
				 errdetail("%s", msg.data)));

		pfree(msg.data);
	}
	else
	{
		if (command_complete)
		{
			if (forward_command_complete(frontend, packet, packetlen) < 0)
				return POOL_END;
		}
		else
		{
			if (forward_empty_query(frontend, packet, packetlen) < 0)
				return POOL_END;
		}
	}

	return POOL_CONTINUE;
}

/*
 * Forward Command complete packet to frontend
 */
static int
forward_command_complete(POOL_CONNECTION * frontend, char *packet, int packetlen)
{
	return forward_packet_to_frontend(frontend, 'C', packet, packetlen);
}

/*
 * Forward Empty query response to frontend
 */
static int
forward_empty_query(POOL_CONNECTION * frontend, char *packet, int packetlen)
{
	return forward_packet_to_frontend(frontend, 'I', packet, packetlen);
}

/*
 * Forward packet to frontend
 */
static int
forward_packet_to_frontend(POOL_CONNECTION * frontend, char kind, char *packet, int packetlen)
{
	int			sendlen;

	if (pool_write(frontend, &kind, 1) < 0)
		return -1;

	sendlen = htonl(packetlen + 4);
	if (pool_write(frontend, &sendlen, sizeof(sendlen)) < 0)
		return -1;

	pool_write_and_flush(frontend, packet, packetlen);

	return 0;
}

/*
 * Process statements that need clearing query cache
 */
static void
process_clear_cache(POOL_CONNECTION_POOL * backend)
{
	/* Query cache enabled? */
	if (!pool_config->memory_cache_enabled)
		return;

	/* Streaming replication mode? */
	if (SL_MODE)
	{
		/*
		 * Are we inside a transaction?
		 */
		if (TSTATE(backend, MAIN_NODE_ID ) == 'T')
		{
			/*
			 * Disable query cache in this transaction.
			 * All query cache will be cleared at commit.
			 */
			set_query_cache_disabled_tx();
		}
		else if (TSTATE(backend, MAIN_NODE_ID ) == 'I')	/* outside transaction */
		{
			/*
			 * Clear all the query cache.
			 */
			clear_query_cache();
		}
	}
	else
	{
		/*
		 * Are we inside a transaction?
		 */
		if (TSTATE(backend, MAIN_NODE_ID ) == 'T')
		{
			/* Inside user started transaction? */
			if (!INTERNAL_TRANSACTION_STARTED(backend, MAIN_NODE_ID))
			{
				/*
				 * Disable query cache in this transaction.
				 * All query cache will be cleared at commit.
				 */
				set_query_cache_disabled_tx();
			}
			else
			{
				/*
				 * Clear all the query cache.
				 */
				clear_query_cache();
			}
		}
		else if (TSTATE(backend, MAIN_NODE_ID ) == 'I')	/* outside transaction */
		{
			/*
			 * Clear all the query cache.
			 */
			clear_query_cache();
		}
	}
}

/*
 * Clear query cache on shmem or memcached
 */
static
void clear_query_cache(void)
{
	/*
	 * Clear all the shared memory cache and oid maps.
	 */
	if (pool_is_shmem_cache())
	{
		pool_clear_memory_cache();
		ereport(LOG,
				(errmsg("all query cache in shared memory deleted")));
	}
	else
#ifdef USE_MEMCACHED
	{
		/*
		 * Clear all the memcached cache and oid maps.
		 */
		delete_all_cache_on_memcached();
		pool_discard_oid_maps();
		ereport(LOG,
				(errmsg("all query cache in memcached deleted")));
	}
#else
	{
		ereport(WARNING,
				(errmsg("failed to clear cache on memcached, memcached support is not enabled")));
	}
#endif
}
