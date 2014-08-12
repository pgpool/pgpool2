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
 * pool_rewrite_query.c: rewrite_query
 *
 */

#include "pool.h"
#include "pool_config.h"
#include "parallel_query/pool_rewrite_query.h"
#include "protocol/pool_proto_modules.h"
#include "context/pool_session_context.h"
#include "utils/elog.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>

static int getInsertRule(ListCell *lc,List *list_t ,DistDefInfo *info, int div_key_num);
static void examInsertStmt(Node *node,POOL_CONNECTION_POOL *backend,RewriteQuery *message);
static void examSelectStmt(Node *node,POOL_CONNECTION_POOL *backend,RewriteQuery *message);
static char *delimistr(char *str);
static int direct_parallel_query(RewriteQuery *message);
static void initMessage(RewriteQuery *message);
static void initdblink(ConInfoTodblink *dblink, POOL_CONNECTION_POOL *backend);
static void analyze_debug(RewriteQuery *message);


/* create error message  */
char *pool_error_message(char *message)
{
	String *str;

	str = init_string("");
	string_append_char(str,message);
	return str->data;
}

/*
 *  search DistDefInfo(this info is build in starting process
 *  and get node id where a query send.
 */
static int getInsertRule(ListCell *lc,List *list_t ,DistDefInfo *info,int div_key_num)
{
	int loop_counter = 0;
	int node_number = -1;
	ListCell *cell;

	if(list_t->length != 1)
		return -1;

	cell = list_head(list_t);

	if(!cell && !IsA(cell,List))
		return 1;

	foreach(lc,lfirst(cell))
	{
		A_Const *constant;
		Value value;
		void *obj = NULL;

		obj = lfirst(lc);

		/* it supports casting syntax such as "A::B::C" */
		while (obj && IsA(obj, TypeCast))
		{
			TypeCast *type = (TypeCast *) obj;
			obj = type->arg;
		}

		if(!obj || !IsA(obj, A_Const))
			return -1;

		if (loop_counter == div_key_num)
		{
			constant = (A_Const *) obj;
			value = constant->val;
			if (value.type == T_Integer)
			{
				char temp[16];
				sprintf(temp,"%ld",value.val.ival);
				node_number = pool_get_id(info,temp);
				break;
			}
			else
			{
				if(value.val.str)
					node_number = pool_get_id(info, value.val.str);
				else
					return -1;
				break;
			}
		}
		loop_counter++;
	}
	/* if node_number is -1, cannot get return value from pool_get_id() */
	return node_number;
}

/*
 * This function processes the decision whether to
 * distribute the insert sentence to the node.
 */
static void examInsertStmt(Node *node,POOL_CONNECTION_POOL *backend, RewriteQuery *message)
{
	RangeVar *table;
	int cell_num;
	int node_number;
	DistDefInfo *info = NULL;
	ListCell *lc = NULL;
	List *list_t = NULL;
	int div_key_num = 0;
	int dist_def_flag = 0;
	InsertStmt *insert = (InsertStmt *) node;

  message->type = node->type;


	/* insert target table */
	table = insert->relation;
	if (!table)
	{
		/* send  error message to frontend */
		message->r_code = INSERT_SQL_RESTRICTION;
		message->r_node = -1;
		message->rewrite_query = pool_error_message("cannot find table name");
		return;
	}

	info = pool_get_dist_def_info(MASTER_CONNECTION(backend)->sp->database,
								  table->schemaname,
								  table->relname);

	if (!info)
	{
		/* send  error message to frontend */
		message->r_code = INSERT_DIST_NO_RULE;
		return;
	}

	/* the source SELECT ? */
	if (insert->selectStmt && ((SelectStmt *)insert->selectStmt)->targetList)
	{
		/* send  error message to frontend */
		message->r_code = INSERT_SQL_RESTRICTION;
		message->r_node = -1;
		message->rewrite_query = pool_error_message("cannot use SelectStmt in InsertStmt");
		return;
	}

	list_t = insert->selectStmt ? (List *)(((SelectStmt *)insert->selectStmt)->valuesLists) : NULL;

	if (!list_t)
	{
		/* send  error message to frontend */
		message->r_code = INSERT_SQL_RESTRICTION;
		message->r_node = -1;
		message->rewrite_query = pool_error_message("cannot find target List");
		return;
	}

	/* number of target list */

	if(list_t->length == 1 && IsA(lfirst(list_head(list_t)),List))
	{
		cell_num = ((List *) lfirst(list_head(list_t)))->length;
	}
	else
	{
			/* send  error message to frontend */
			message->r_code = INSERT_SQL_RESTRICTION;
			message->r_node = -1;
			message->rewrite_query = pool_error_message("cannot analyze this InsertStmt");
			return;
  }


	/* Is the target columns ?*/
	if (!insert->cols)
	{
		div_key_num = info->dist_key_col_id;
		dist_def_flag = 1;

		ereport(DEBUG2,
				(errmsg("cell number %d, div key num %d, div_key columname %s",cell_num,div_key_num,info->col_list[div_key_num])));

		if (cell_num < div_key_num)
		{
			/* send  error message to frontend */
			message->r_code = INSERT_SQL_RESTRICTION;
			message->r_node = -1;
			message->rewrite_query = pool_error_message("cannot find dividing key in InsertStmt");
			return;
		}

  }
	else
	{
		List *list_cols = (List *) insert->cols;

		foreach(lc, list_cols)
		{
			Node *n;
			ResTarget *target;
 			n = lfirst(lc);
			target = (ResTarget *) n;
			if (strcmp(target->name,info->dist_key_col_name) == 0)
			{
				dist_def_flag = 1;
				break;
			}
			div_key_num++;
		}

		if (cell_num < div_key_num)
		{
			/* send  error message to frontend */
			message->r_code = INSERT_SQL_RESTRICTION;
			message->r_node = -1;
			message->rewrite_query = pool_error_message("cannot find dividing key in InsertStmt");
			return;
		}
	}

	if (dist_def_flag != 1)
	{
		/* send  error message to frontend */
		message->r_code = INSERT_SQL_RESTRICTION;
		message->r_node = -1;
		message->rewrite_query = pool_error_message("cannot find dividing key in InsertStmt");
		return;
	}

	/* this loop get insert one args of divide rule */
	node_number = getInsertRule(lc, list_t, info, div_key_num);

	if (node_number < 0)
	{
		/* send  error message to frontend */
		message->r_code = INSERT_SQL_RESTRICTION;
		message->r_node = -1;
		message->rewrite_query = pool_error_message("cannot get node_id from system db");
		return;
	}

	ereport(DEBUG1,
		(errmsg("insert node_number = %d",node_number)));

	message->r_code = 0;
	message->r_node = node_number;
	message->rewrite_query = nodeToString(node);
}

/* start of rewriting query */
static void examSelectStmt(Node *node,POOL_CONNECTION_POOL *backend,RewriteQuery *message)
{
	static ConInfoTodblink dblink;

	/* initialize dblink info */
	initdblink(&dblink,backend);

	/* initialize  message */
	initMessage(message);
	message->type = node->type;
	message->r_code = SELECT_DEFAULT;

  /* do rewrite query */
	nodeToRewriteString(message,&dblink,node);
}

/* initialize Message */
static void initMessage(RewriteQuery *message)
{
	message->r_code = 0;
	message->r_node = 0;
	message->column = 0;
	message->virtual_num = 0;
	message->is_pg_catalog = false;
	message->is_loadbalance = false;
	message->is_parallel = false;
	message->table_relname = NULL;
	message->table_alias = NULL;
	message->dbname = NULL;
	message->schemaname = NULL;
	message->rewrite_query = NULL;
	message->rewritelock = -1;
	message->ignore_rewrite = -1;
	message->ret_num = 0;
}

/* set dblink info */
static void initdblink(ConInfoTodblink *dblink,POOL_CONNECTION_POOL *backend)
{
	dblink->dbname =  MASTER_CONNECTION(backend)->sp->database;
	dblink->hostaddr = pool_config->pgpool2_hostname;
	dblink->user = MASTER_CONNECTION(backend)->sp->user;
	dblink->port = pool_config->port;
	dblink->password = MASTER_CONNECTION(backend)->con->password;
}

/* reference of pg_catalog or not */
int IsSelectpgcatalog(Node *node,POOL_CONNECTION_POOL *backend)
{
	static ConInfoTodblink dblink;
	static RewriteQuery message;

	/* initialize dblink info */
	initdblink(&dblink,backend);

	/* initialize  message */
	initMessage(&message);

	message.type = node->type;

	initdblink(&dblink,backend);

	if(message.is_pg_catalog)
		return 1;
	return 0;
}

/*
 *  SELECT statement or INSERT statement is special,
 *  peculiar process is needed in parallel mode.
 */
RewriteQuery *rewrite_query_stmt(Node *node,POOL_CONNECTION *frontend,POOL_CONNECTION_POOL *backend,RewriteQuery *message)
{
	MemoryContext oldContext = CurrentMemoryContext;
	PG_TRY();
	{
		switch(node->type)
		{
            case T_SelectStmt:
            {
                SelectStmt *stmt = (SelectStmt *)node;

                 /* Because "SELECT INTO" cannot be used in a parallel mode,
                  * the error message is generated and send "ready for query" to frontend.
                  */
                if(stmt->intoClause)
                {
                    pool_send_error_message(frontend, MAJOR(backend), "XX000",
                                            "pgpool2 sql restriction",
                                            "cannot use select into ...", "", __FILE__,
                                            __LINE__);


                    pool_send_readyforquery(frontend);
                    message->status=POOL_CONTINUE;
                    break;
                }

                /*
                 * The Query is actually rewritten based on analytical information on the Query.
                 */
                examSelectStmt(node,backend,message);

                if (message->r_code != SELECT_PGCATALOG &&
                    message->r_code != SELECT_RELATION_ERROR)
                {
                    /*
                     * The rewritten Query is transmitted to system db,
                     * and execution status is received.
                     */
                    POOL_CONNECTION_POOL_SLOT *system_db = pool_system_db_connection();
                    message->status = OneNode_do_command(frontend,
                                                        system_db->con,
                                                        message->rewrite_query,
                                                        backend->info->database);
                }
                else
                {
                    if(TSTATE(backend, MASTER_NODE_ID) == 'T' &&
                       message->r_code == SELECT_RELATION_ERROR)
                    {
                        /*
                         * In the case of message->r_code == SELECT_RELATION_ERROR and in the transaction,
                         * Transmit the Query to all back ends, and to abort transaction.
                         */
						ereport(DEBUG1,
							(errmsg("rewriting query statement(INSERT)"),
								 errdetail("Inside transaction. Abort transaction")));
                        message->rewrite_query = nodeToString(node);
                        message->status = pool_parallel_exec(frontend,backend,message->rewrite_query,node,true);
                    }
                    else
                    {
                        /*
                         * Other cases of message->r_code == SELECT_RELATION_ERROR
                         * or SELECT_PG_CATALOG,
                         * Transmit the Query to Master node and receive status.
                         */
						ereport(DEBUG1,
							(errmsg("rewriting query statement"),
								 errdetail("executed by Master")));

                        message->rewrite_query = nodeToString(node);
                        message->status = OneNode_do_command(frontend,
                                                            MASTER(backend),
                                                            message->rewrite_query,
                                                            backend->info->database);
                    }
                }
				ereport(DEBUG1,
					(errmsg("rewriting query statement"),
						 errdetail("select message_code %d",message->r_code)));

            }
            break;

            case T_InsertStmt:

              /* The distribution of the INSERT sentence. */
                examInsertStmt(node,backend,message);

                if(message->r_code == 0 )
                {
                    /* send the INSERT sentence */
                    message->status = OneNode_do_command(frontend,
                                                        CONNECTION(backend,message->r_node),
                                                        message->rewrite_query,
                                                        backend->info->database);
                }
                else if (message->r_code == INSERT_SQL_RESTRICTION)
                {
                    /* Restriction case of INSERT sentence */
                    pool_send_error_message(frontend, MAJOR(backend), "XX000",
                                            "pgpool2 sql restriction",
                                            message->rewrite_query, "", __FILE__,
                                            __LINE__);

                    if(TSTATE(backend, MASTER_NODE_ID) == 'T')
                    {
                        /* In Transaction, send the invalid message to backend to abort this transaction */
						ereport(DEBUG1,
							(errmsg("rewriting query statement(INSERT)"),
								 errdetail("Inside transaction. Abort transaction")));

                        message->status = pool_parallel_exec(frontend,backend, "POOL_RESET_TSTATE",node,false);
                    }
                    else
                    {
                        /* return "ready for query" to frontend */
                        pool_send_readyforquery(frontend);
                        message->status=POOL_CONTINUE;
                    }
                }
                break;
#if 0
            case T_UpdateStmt:
                /* Improve UpdateStmt for complex query */
                break;
#endif
            default:
                message->type = node->type;
                message->status = POOL_CONTINUE;
                break;
        }
    }
	PG_CATCH();
	{
		message->status= POOL_END;
		MemoryContextSwitchTo(oldContext);
		FlushErrorState();
	}
    PG_END_TRY();

	ereport(DEBUG2,
		(errmsg("rewriting query statement"),
			 errdetail("query rule %d",node->type)));

	return message;
}

#define POOL_PARALLEL "pool_parallel"
#define POOL_LOADBALANCE "pool_loadbalance"

/*
 * After analyzing query, check the analyze[0]->state.
 * if the analyze[0]->state ==`P`, this query can be executed
 * on parallel engine.
 */
static int direct_parallel_query(RewriteQuery *message)
{
	if(message && message->analyze[0] && message->analyze[0]->state == 'P')
		return 1;
	else
		return 0;
}


/* escape delimiter character */
static char *delimistr(char *str)
{
	char *result;
	int i,j = 0;
	int len = strlen(str);

	result = palloc(len -1);

	for(i = 0; i < len; i++)
	{
		char c = (unsigned char) str[i];
		if((i != 0) && (i != len -1))
		{
			if(c=='\'' && (char) str[i+1]=='\'')
				i++;
			result[j] = c;
			j++;
		}
	}

	result[j] = '\0';

	return result;
}

/* for debug */
void analyze_debug(RewriteQuery *message)
{
	int analyze_num,i;
	analyze_num = message->analyze_num;

	for(i = 0; i< analyze_num; i++)
	{
		AnalyzeSelect *analyze = message->analyze[i];
		ereport(DEBUG1,
				(errmsg("analyze_debug :select no(%d), last select(%d), last_part(%d), state(%c)",
						analyze->now_select,analyze->last_select,analyze->call_part,analyze->state)));
	}
}

/*
 * This function checks the KEYWORD(POOL_PARALLEL,POOL_LOADBALANCE)
 * if the special function(like pool_parallel() or pool_loadbalance())
 * is used, mark the r_code,is_parallel and is_loadbalance.
 * In other cases, It is necessary to analyze the Query.
 */
RewriteQuery *is_parallel_query(Node *node, POOL_CONNECTION_POOL *backend)
{
	static RewriteQuery message;
	static ConInfoTodblink dblink;

	initMessage(&message);

	if (IsA(node, SelectStmt))
	{
		SelectStmt *stmt;
		Node *n;
		int direct_ok;

		stmt = (SelectStmt *) node;

		/* Check the special function is used in this query*/
		if (!(stmt->distinctClause || stmt->intoClause ||
			stmt->fromClause || stmt->groupClause || stmt->havingClause ||
			stmt->sortClause || stmt->limitOffset || stmt->limitCount ||
			stmt->lockingClause || stmt->larg || stmt->rarg) &&
			(n = lfirst(list_head(stmt->targetList))) && IsA(n, ResTarget))
		{
			ResTarget *target = (ResTarget *) n;

			if (target->val && IsA(target->val, FuncCall))
			{
				FuncCall *func = (FuncCall *) target->val;
				if (list_length(func->funcname) == 1 && func->args)
				{
					Node *func_args = (Node *) lfirst(list_head(func->args));
					message.rewrite_query = delimistr(nodeToString(func_args));

					/* pool_parallel() is used in this query */
					if(strcmp(strVal(lfirst(list_head(func->funcname))),
						   POOL_PARALLEL) == 0)
					{
						message.r_code = SEND_PARALLEL_ENGINE;
						message.is_parallel = true;
						message.is_loadbalance = false;
						ereport(DEBUG1,
							(errmsg("checking if query can be executed in parallel mode"),
								 errdetail("pool_parallel_exec \"%s\"",message.rewrite_query)));

						return &message;
					}
					else /* pool_loadbalance() is used in this query */
					if(strcmp(strVal(lfirst(list_head(func->funcname))),
						   						POOL_LOADBALANCE) == 0)
					{
						message.r_code = SEND_LOADBALANCE_ENGINE;
						message.is_loadbalance = true;
						message.is_parallel = false;
						ereport(DEBUG1,
							(errmsg("checking if query can be executed in parallel mode"),
								 errdetail("loadbalance_mode \"%s\"",message.rewrite_query)));

						return &message;
					}
				}
			}
		}

    /* ANALYZE QUERY */
		message.r_code = SELECT_ANALYZE;
		message.is_loadbalance = true;

		initdblink(&dblink,backend);
		nodeToRewriteString(&message,&dblink,node);

		if(message.is_pg_catalog)
		{
			message.is_loadbalance = false;
			message.is_parallel = false;
			ereport(DEBUG1,
				(errmsg("checking if query can be executed in parallel mode"),
					 errdetail("query is load balanced (pgcatalog)")));
			return &message;
		}

		if(message.is_loadbalance)
		{
			message.is_parallel = false;
			ereport(DEBUG1,
				(errmsg("checking if query can be executed in parallel mode"),
					 errdetail("query is load balanced")));
			return &message;
		}

		/* Analyzing Query Start */
		analyze_debug(&message);

		/* After the analyzing query,
		 * this query can be executed as parallel exec, is_parallel flag is turned on
		 */
		direct_ok = direct_parallel_query(&message);
		if(direct_ok == 1)
		{
			message.rewrite_query = nodeToString(node);
			message.is_parallel = true;
			message.is_loadbalance = false;
			ereport(DEBUG1,
				(errmsg("checking if query can be executed in parallel mode"),
					 errdetail("query: \"%s\"",message.rewrite_query)));

			return &message;
		}
	}
	else if (IsA(node, CopyStmt))
	{
		/* For Copy Statement, check the table name, mark the is_parallel flag. */
		CopyStmt *stmt = (CopyStmt *)node;

		if (stmt->is_from == FALSE && stmt->filename == NULL)
		{
			RangeVar *relation = (RangeVar *)stmt->relation;

			/* check on distribution table or replicate table */

			if(pool_get_dist_def_info (MASTER_CONNECTION(backend)->sp->database, relation->schemaname, relation->relname))
			{
				message.rewrite_query = nodeToString(stmt);
				message.is_parallel = true;
				message.is_loadbalance = false;
				message.r_code = SEND_PARALLEL_ENGINE;
			}
		}
	}

	return &message;
}

POOL_STATUS pool_do_parallel_query(POOL_CONNECTION *frontend,
								   POOL_CONNECTION_POOL *backend,
								   Node *node, bool *parallel, char **string, int *len)
{
	/* The Query is analyzed first in a parallel mode(in_parallel_query),
	 * and, next, the Query is rewritten(rewrite_query_stmt).
	 */

	/* analyze the query */
	RewriteQuery *r_query = is_parallel_query(node,backend);

	if(r_query->is_loadbalance)
	{
        /* Usual processing of pgpool is done by using the rewritten Query
         * if judged a possible load-balancing as a result of analyzing
         * the Query.
         * Of course, the load is distributed only for load_balance_mode=true.
         */
		if(r_query->r_code ==  SEND_LOADBALANCE_ENGINE)
		{
			/* use rewritten query */
			*string = r_query->rewrite_query;
			/* change query length */
			*len = strlen(*string)+1;
		}
		ereport(DEBUG1,
			(errmsg("doing parallel query"),
				 errdetail("load balancing query: \"%s\"",*string)));
	}
	else if (r_query->is_parallel)
	{
		/*
		 * For the Query that the parallel processing is possible.
		 * Call parallel exe engine and return status to the upper layer.
		 */
		POOL_STATUS stats = pool_parallel_exec(frontend,backend,r_query->rewrite_query, node,true);
		pool_unset_query_in_progress();
		return stats;
	}
	else if(!r_query->is_pg_catalog)
	{
		/* rewrite query and execute */
		r_query = rewrite_query_stmt(node,frontend,backend,r_query);
		if(r_query->type == T_InsertStmt)
		{
			/* free_parser(); */

			if(r_query->r_code != INSERT_DIST_NO_RULE) {
				pool_unset_query_in_progress();
				pool_set_skip_reading_from_backends();
				return r_query->status;
			}
		}
		else if(r_query->type == T_SelectStmt)
		{
			pool_unset_query_in_progress();
			pool_set_skip_reading_from_backends();
			return r_query->status;
		}
	}

	*parallel = false;
	return POOL_CONTINUE;
}
