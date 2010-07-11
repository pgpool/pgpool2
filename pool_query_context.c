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
#include <stdlib.h>

static bool is_should_be_sent_to_primary(Node *node);

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
		pool_unset_query_in_progress();
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
		return 0;

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
	 * In raw mode, we send only to master node. Simple enough.
	 */
	if (RAW_MODE)
	{
		pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
	}
	else if (MASTER_SLAVE)
	{
		/* Streaming Replication+Hot Standby? */
		if (!strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP))
		{
			/* Should be sent to primary only? */
			if (is_should_be_sent_to_primary(node))
			{
				pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
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
					 * If we are outside of an explicit transaction OR
					 * the transaction has not issued a write query
					 * yet, we might be able to load balance.
					 */
					if (TSTATE(backend, MASTER_NODE_ID) == 'I' ||
						!pool_is_writing_transaction())
					{
						BackendInfo *bkinfo = pool_get_node_info(session_context->load_balance_node_id);

						/*
						 * Load balance if possible
						 */

						/*
						 * If replication delay is too much, we prefer to send to the primary.
						 */
						if (pool_config->delay_threshold &&
							bkinfo->standby_delay > pool_config->delay_threshold)
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
					/* Send to all nodes */
					pool_setall_node_to_be_sent(query_context);
				}
			}
		}
		else	/* Slony-I case */
		{
			/*
			 * DMLs msut be sent to master
			 */
			if (IsA(node, InsertStmt) || IsA(node, DeleteStmt) ||
				IsA(node, UpdateStmt))
			{
				/* only send to master node */
				pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
			}
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
					TSTATE(backend, MASTER_NODE_ID) == 'I' &&
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
					pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
				}
			}
		}
	}
	else if (REPLICATION|PARALLEL_MODE)
	{
		if (pool_config->load_balance_mode &&
			MAJOR(backend) == PROTO_MAJOR_V3 &&
			TSTATE(backend, MASTER_NODE_ID) == 'I' &&
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
			pool_set_node_to_be_sent(query_context, REAL_MASTER_NODE_ID);
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
 * send_type:
 *  -1: do not send this node_id
 *	0: send to all nodes
 *  >0: send to this node_id
 * kind: simple query protocol is ""
 */
POOL_STATUS pool_send_and_wait(POOL_QUERY_CONTEXT *query_context, char *query, int len,
							   int send_type, int node_id, char *kind)
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

		if (*kind == '\0')
		{
			if (send_simplequery_message(CONNECTION(backend, i), len, query, MAJOR(backend)) != POOL_CONTINUE)
				return POOL_END;
		}			
		else
		{
			if (send_extended_protocol_message(backend, i, kind, len, query) != POOL_CONTINUE)
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
		 * what statement caused that error and make users
		 * confused.
		 */
		per_node_error_log(backend, i, query, "pool_send_and_wait: Error or notice message from backend: ", true);
	}
	return POOL_CONTINUE;
}

/*
 * Decide if the statement should be sent to primary only in
 * master/slave+HR/SR mode.
 */
static bool is_should_be_sent_to_primary(Node *node)
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

	static NodeTag nodemap[] = {
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
		T_ClosePortalStmt,
		T_ClusterStmt,
		T_CopyStmt,
		T_CreateStmt,	/* CREAE TABLE */
		T_DefineStmt,	/* CREATE AGGREGATE, OPERATOR, TYPE */
		T_DropStmt,		/* DROP TABLE etc. */
		T_TruncateStmt,
		T_CommentStmt,
		/*
		  T_FetchStmt,
		*/
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
		/*
		  T_LoadStmt,
		*/
		T_CreateDomainStmt,
		T_CreatedbStmt,
		T_DropdbStmt,
		T_VacuumStmt,
		/*
		  T_ExplainStmt,		XXX: explain analyze?
		*/
		T_CreateSeqStmt,
		T_AlterSeqStmt,
		T_VariableSetStmt,		/* SET */
		/*
		T_VariableShowStmt,
		T_DiscardStmt,
		*/
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
		/*
		  T_DeallocateStmt,		DEALLOCATE
		  T_DeclareCursorStmt,	DECLARE
		*/
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
				return true;

			/* SELECT FOR SHARE or UPDATE */
			else if (((SelectStmt *)node)->lockingClause)
				return true;

			/*
			 * SELECT nextval(), setval()
			 * XXX: We do not search in subquery.
			 */
			else if (is_sequence_query(node))
				return true;

			return false;
		}

		/*
		 * COPY FROM
		 */
		else if (IsA(node, CopyStmt))
		{
			return ((CopyStmt *)node)->is_from;
		}

		/*
		 * LOCK
		 */
		else if (IsA(node, LockStmt))
		{
			return (((LockStmt *)node)->mode >= RowExclusiveLock);
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
							return true;
					}
				}
				return false;
			}

			/*
			 * 2PC commands
			 */
			else if (((TransactionStmt *)node)->kind == TRANS_STMT_PREPARE ||
					 ((TransactionStmt *)node)->kind == TRANS_STMT_COMMIT_PREPARED ||
					 ((TransactionStmt *)node)->kind == TRANS_STMT_ROLLBACK_PREPARED)
				return true;

			else
				return false;
		}

		/*
		 * SET
		 */
		else if (IsA(node, VariableSetStmt))
		{
			ListCell   *list_item;
			bool ret = false;

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
								ret = true;
							break;
						case T_Integer:
							if (v->val.val.ival)
								ret = true;
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
							return true;
					}
				}
				return false;
			}
			else
				return false;
		}
		return true;
	}

	/*
	 * All unknown statements are sent to master
	 */
	return true;
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
