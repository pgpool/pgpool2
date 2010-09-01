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
#include "pool_select_walker.h"
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

static POOL_DEST send_to_where(Node *node, char *query);
static void where_to_send_deallocate(POOL_QUERY_CONTEXT *query_context, Node *node);

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

	/* Create memory context */
	qc->memory_context = pool_memory_create(PARSER_BLOCK_SIZE);

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
		pool_unset_query_in_progress();
		session_context->query_context = NULL;
		pool_memory_delete(query_context->memory_context, 0);
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
		query_context->rewritten_query = NULL;
		query_context->parse_tree = node;
		query_context->virtual_master_node_id = REAL_MASTER_NODE_ID;
		pool_set_query_in_progress();
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
	int i;

	if (!query_context)
	{
		pool_error("pool_setall_node_to_be_sent: no query context");
		return;
	}

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if ((BACKEND_INFO(i)).backend_status == CON_UP ||
			(BACKEND_INFO((i)).backend_status == CON_CONNECT_WAIT))
			query_context->where_to_send[i] = true;
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

	if (!query_context)
	{
		pool_error("pool_multi_node_to_be_sent: no query context");
		return false;
	}

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
 * Returns true if the DB node is needed to send query.
 * Intended to be called from VALID_BACKEND
 */
bool pool_is_node_to_be_sent_in_current_query(int node_id)
{
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context();
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

	sc = pool_get_session_context();
	if (!sc)
	{
		return REAL_MASTER_NODE_ID;
	}

	if (sc->query_context)
	{
		return sc->query_context->virtual_master_node_id;
	}
	return REAL_MASTER_NODE_ID;
}

/*
 * Decide where to send queries(thus expecting response)
 */
void pool_where_to_send(POOL_QUERY_CONTEXT *query_context, char *query, Node *node)
{
	POOL_SESSION_CONTEXT *session_context;
	POOL_CONNECTION_POOL *backend;
	int i;

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
	 * If there is "NO LOAD BALANCE" comment, we send only to master node.
	 */
	if (!strncasecmp(query, NO_LOAD_BALANCE, NO_LOAD_BALANCE_COMMENT_SZ))
	{
		pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
		return;
	}

	/*
	 * In raw mode, we send only to master node. Simple enough.
	 */
	if (RAW_MODE)
	{
		pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
	}
	else if (MASTER_SLAVE)
	{
		POOL_DEST dest;
		POOL_MEMORY_POOL *old_context = pool_memory;

		pool_memory = query_context->memory_context;
		dest = send_to_where(node, query);
		pool_memory = old_context;

		pool_debug("send_to_where: %d query: %s", dest, query);

		/* Should be sent to primary only? */
		if (dest == POOL_PRIMARY)
		{
			pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
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
				if (TSTATE(backend, MASTER_NODE_ID) == 'I' ||
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
						pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
					}

					/*
					 * If a writing function call is used, 
					 * we prefer to send to the primary.
					 */
					else if (pool_has_function_call(node))
					{
						pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
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
						pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
					}

					/*
					 * If temporary table is used in the SELECT,
					 * we prefer to send to the primary.
					 */
					else if (pool_has_temp_table(node))
					{
						pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
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
					pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
				}
			}
			else
			{
				/* Send to the primary only */
				pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
			}
		}

		/* PREPARE? */
		if (IsA(node, PrepareStmt))
		{
			/* Make sure that same prepared statement does not exist */
			if (pool_get_prep_where(((PrepareStmt *)node)->name) == NULL)
			{
				/* Save the send map */
				pool_add_prep_where(((PrepareStmt *)node)->name, query_context->where_to_send);
			}
		}

		/*
		 * EXECUTE?
		 */
		else if (IsA(node, ExecuteStmt))
		{
			bool *wts;

			wts = pool_get_prep_where(((ExecuteStmt *)node)->name);
			if (wts)
			{
				/* Inherit same map from PREPARE */
				pool_copy_prep_where(wts, query_context->where_to_send);
			}
		}

		/*
		 * DEALLOCATE?
		 */
		else if (IsA(node, DeallocateStmt))
		{
			where_to_send_deallocate(query_context, node);
		}
	}
	else if (REPLICATION || PARALLEL_MODE)
	{
		if (is_select_query(node, query) && !is_sequence_query(node))
		{
			/*
			 * If a writing function call is used or replicate_select is true,
			 * we prefer to send to all nodes.
			 */
			if (pool_has_function_call(node) || pool_config->replicate_select)
			{
				pool_setall_node_to_be_sent(query_context);
			}
			else if (pool_config->load_balance_mode &&
					 MAJOR(backend) == PROTO_MAJOR_V3 &&
					 TSTATE(backend, MASTER_NODE_ID) == 'I')
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
		/*
		 * DEALLOCATE?
		 */
		else if (IsA(node, DeallocateStmt))
		{
			where_to_send_deallocate(query_context, node);
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
		return;
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
 * Send query and wait for response
 * string:
 *  simple query protocol: a query
 *  extended query protocol: contents of the message
 * send_type:
 *  -1: do not send this node_id
 *   0: send to all nodes
 *  >0: send to this node_id
 * kind:
 *  simple query protocol: ""
 *  extended query protocol: a kind
 */
POOL_STATUS pool_send_and_wait(POOL_QUERY_CONTEXT *query_context, char *string,
							   int len, int send_type, int node_id, char *kind)
{
	POOL_SESSION_CONTEXT *session_context;
	POOL_CONNECTION *frontend;
	POOL_CONNECTION_POOL *backend;
	bool is_commit;
	int i;

	is_commit = is_commit_query(query_context->parse_tree);

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

		/*
		 * If in master/slave mode, we do not send COMMIT/ABORT to
		 * slaves/standbys if it's not in I(idle in transaction)
		 * state.
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

		per_node_statement_log(backend, i, string);

		if (*kind == '\0')
		{
			if (send_simplequery_message(CONNECTION(backend, i), len, string, MAJOR(backend)) != POOL_CONTINUE)
				return POOL_END;
		}			
		else
		{
			if (send_extended_protocol_message(backend, i, kind, len, string) != POOL_CONTINUE)
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

		/*
		 * If in master/slave mode, we do not send COMMIT/ABORT to
		 * slaves/standbys if it's in I(idle) state.
		 */
		if (is_commit && MASTER_SLAVE && !IS_MASTER_NODE_ID(i) && TSTATE(backend, i) == 'I')
		{
			continue;
		}

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

		/*
		 * Check if some error detected.  If so, emit
		 * log. This is usefull when invalid encoding error
		 * occurs. In this case, PostgreSQL does not report
		 * what statement caused that error and make users
		 * confused.
		 */
		per_node_error_log(backend, i, string, "pool_send_and_wait: Error or notice message from backend: ", true);
	}
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
		T_DropPropertyStmt,
		T_CreatePLangStmt,
		T_DropPLangStmt,
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
		T_DropCastStmt,
		T_CreateOpClassStmt,
		T_CreateOpFamilyStmt,
		T_AlterOpFamilyStmt,
		T_RemoveOpClassStmt,
		T_RemoveOpFamilyStmt,
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
		T_DropFdwStmt,
		T_CreateForeignServerStmt,
		T_AlterForeignServerStmt,
		T_DropForeignServerStmt,
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
			/* SELECT INTO? */
			if (((SelectStmt *)node)->intoClause)
				return POOL_PRIMARY;

			/* SELECT FOR SHARE or UPDATE */
			else if (((SelectStmt *)node)->lockingClause)
				return POOL_PRIMARY;

			/*
			 * SELECT nextval(), setval()
			 * XXX: We do not search in subquery.
			 */
			else if (is_sequence_query(node))
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
			ListCell   *list_item;

			/*
			 * Check "BEGIN READ WRITE" "START TRANSACTION READ WRITE"
			 */
			if (((TransactionStmt *)node)->kind == TRANS_STMT_BEGIN ||
				((TransactionStmt *)node)->kind == TRANS_STMT_START)
			{
				List *options = ((TransactionStmt *)node)->options;
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
				/* Other TRANSACTION start commands are sent to the primary */
				return POOL_PRIMARY;
			}
			else if (((TransactionStmt *)node)->kind == TRANS_STMT_SAVEPOINT ||
					 ((TransactionStmt *)node)->kind == TRANS_STMT_ROLLBACK_TO ||
					 ((TransactionStmt *)node)->kind == TRANS_STMT_RELEASE)
			{
				/* SAVEPOINT related commands are sent to the primary */
				return POOL_PRIMARY;
			}

			/*
			 * 2PC commands
			 */
			else if (is_2pc_transaction_query(node, query))
				return POOL_PRIMARY;
			else
				/* COMMIT etc. */
				return POOL_PRIMARY;
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
	bool *wts;

	/* DELLOCATE ALL? */
	if (d->name == NULL)
	{
		pool_setall_node_to_be_sent(query_context);
		return;
	}
	else
	{
		wts = pool_get_prep_where(d->name);
		if (wts)
		{
			/* Inherit same map from PREPARE */
			pool_copy_prep_where(wts, query_context->where_to_send);
			return;
		}
		else
		{
			PreparedStatement *ps;

			ps = pool_get_prepared_statement_by_pstmt_name(d->name);
			if (ps && ps->qctxt)
			{
				pool_copy_prep_where(ps->qctxt->where_to_send, query_context->where_to_send);
				return;
			}
		}
	}
	/* prepared statement was not found */
	pool_setall_node_to_be_sent(query_context);
}

/*
 * Returns parse tree for current query.
 * Preconition: the query is in progress state.
 */
Node *pool_get_parse_tree(void)
{
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context();
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
 * Preconition: the query is in progress state.
 */
char *pool_get_query_string(void)
{
	POOL_SESSION_CONTEXT *sc;

	sc = pool_get_session_context();
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
bool is_set_transaction_serializable(Node *node, char *query)
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
 * Return true if the query is 2PC transaction query.
 */
bool is_2pc_transaction_query(Node *node, char *query)
{
	if (((TransactionStmt *)node)->kind == TRANS_STMT_PREPARE ||
		((TransactionStmt *)node)->kind == TRANS_STMT_COMMIT_PREPARED ||
		((TransactionStmt *)node)->kind == TRANS_STMT_ROLLBACK_PREPARED)
		return true;

	return false;
}

/*
 * Set query state, if specified state less than current state
 * state:
 *  0: before parse   1: parse done     2: bind done
 *  3: describe done  4: execute done  -1: in error
 */
void pool_set_query_state(POOL_QUERY_CONTEXT *query_context, short state)
{
	int i;

	if (!query_context)
	{
		pool_error("pool_set_query_state: no query context");
		return;
	}

	if (state < -1 || state > 4)
	{
		pool_error("pool_set_query_state: invalid query state: %d", state);
		return;
	}

	for (i = 0; i < NUM_BACKENDS; i++)
	{
		if (query_context->where_to_send[i] &&
			query_context->query_state[i] < state)
			query_context->query_state[i] = state;
	}
}
