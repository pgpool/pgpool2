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
 * pool_rewrite_query.c: rewrite_query
 *
 */

#include "pool.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "pool_rewrite_query.h"


static int getInsertRule(ListCell *lc,List *list_t ,DistDefInfo *info, int div_key_num);
static void examInsertStmt(Node *node,POOL_CONNECTION_POOL *backend,RewriteQuery *message);
static void examSelectStmt(Node *node,POOL_CONNECTION_POOL *backend,RewriteQuery *message);
static char *delimistr(char *str);
static int direct_parallel_query(SelectStmt *stmt,POOL_CONNECTION_POOL *backend);
static int check_whereClause(Node *where);
static void initMessage(RewriteQuery *message,bool analyze);
static void initdblink(ConInfoTodblink *dblink, POOL_CONNECTION_POOL *backend);

char *pool_error_message(char *message)
{
	String *str;

	str = init_string("");
	string_append_char(str,message);
	return str->data;
}

static int getInsertRule(ListCell *lc,List *list_t ,DistDefInfo *info,int div_key_num)
{
	int loop_counter = 0;
	int node_number = -1;

	foreach(lc,list_t)
	{
		Node *n;
		ResTarget *target;
		A_Const *constant;
		Value value;
		void *obj = NULL;

		n = lfirst(lc);
		target = (ResTarget *) n;

		if(target->val && IsA(target->val, TypeCast))
		{
			TypeCast *type = (TypeCast *) target->val;
			obj = type->arg;
			if(!obj)
			{
				return -1;
			}
		} else {
			obj = target->val;
		}

		if(obj && !IsA(obj, A_Const))
		{
			return -1;
		}

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

	/* the source SELECT ? */
	if (insert->selectStmt)
	{
		/* send  error message to frontend */
		message->r_code = INSERT_SQL_RESTRICTION;
		message->r_node = -1;
		message->rewrite_query = pool_error_message("cannot use SelectStmt in InsertStmt");
		return;
	}

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

	list_t = (List *) insert->targetList;
	if (!list_t)
	{
		/* send  error message to frontend */
		message->r_code = INSERT_SQL_RESTRICTION;
		message->r_node = -1;
		message->rewrite_query = pool_error_message("cannot find target List");
		return;
	}
	
	/* number of target list */
	cell_num = list_t->length;

	/* pool_debug("exam_InsertStmt insert table_name %s:",table->relname); */

	info = pool_get_dist_def_info(MASTER_CONNECTION(backend)->sp->database,
								  table->schemaname,
								  table->relname);

	if (!info)
	{
		/* send  error message to frontend */
		message->r_code = INSERT_DIST_NO_RULE;
		return;
	}

	/* Is the target columns ?*/
	if (!insert->cols)
	{
		div_key_num = info->dist_key_col_id;
		dist_def_flag = 1;

		pool_debug("div key num %d, div_key columname %s",div_key_num,info->col_list[div_key_num]);

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
	
	pool_debug("insert node_number =%d",node_number);
	message->r_code = 0;
	message->r_node = node_number;
	message->rewrite_query = nodeToString(node);
}

static void examSelectStmt(Node *node,POOL_CONNECTION_POOL *backend,RewriteQuery *message)
{
	static ConInfoTodblink dblink;

	/* initialize dblink info */
	initdblink(&dblink,backend);

	/* initialize  message */
	initMessage(message,true);
	message->type = node->type;
	message->r_code = SELECT_DEFAULT;

	nodeToRewriteString(message,&dblink,node);
}


static void initMessage(RewriteQuery *message, bool analyze)
{
	message->r_code = 0;
	message->r_node = 0;
	message->is_pg_catalog = false;
	message->is_loadbalance = false;
	message->is_parallel = false;
	message->table_relname = NULL;
	message->table_alias = NULL;
	message->dbname = NULL;
	message->schemaname = NULL;
	message->rewrite_query = NULL;

	if(analyze)
	{
		message->analyze_num = 0;
	}
}

static void initdblink(ConInfoTodblink *dblink,POOL_CONNECTION_POOL *backend)
{
	dblink->dbname =  MASTER_CONNECTION(backend)->sp->database;
	dblink->hostaddr = pool_config->pgpool2_hostname;
	dblink->user = MASTER_CONNECTION(backend)->sp->user;
	dblink->port = pool_config->port;
	dblink->password = MASTER_CONNECTION(backend)->con->password;
}

int IsSelectpgcatalog(Node *node,POOL_CONNECTION_POOL *backend)
{
	static ConInfoTodblink dblink;
	static RewriteQuery message;

	/* initialize dblink info */
	initdblink(&dblink,backend);

	/* initialize  message */
	initMessage(&message,false);

	message.type = node->type;

	initdblink(&dblink,backend);

	if(message.is_pg_catalog) 
	{
		pool_debug("Isselectpgcatalog %d",message.is_pg_catalog);
		return 1;
	}
	else
	{
		return 0;
	}
}

RewriteQuery *rewrite_query_stmt(Node *node,POOL_CONNECTION *frontend,POOL_CONNECTION_POOL *backend,RewriteQuery *message)
{
	switch(node->type)
	{
		case T_SelectStmt:
		{
			SelectStmt *stmt = (SelectStmt *)node;
			if(stmt->into)
			{
				pool_send_error_message(frontend, MAJOR(backend), "XX000",
										"pgpool2 sql restriction",
										"cannot use select into ...", "", __FILE__,
										__LINE__);


				pool_send_readyforquery(frontend);
				message->status=POOL_CONTINUE;
				break;
			}

			examSelectStmt(node,backend,message);

			if (message->r_code != SELECT_PGCATALOG &&
				message->r_code != SELECT_RELATION_ERROR)
			{
				POOL_CONNECTION_POOL_SLOT *system_db = pool_system_db_connection();
				message->status = OneNode_do_command(frontend, 
													system_db->con, 
													message->rewrite_query,
													backend->info->database);
			}
			else
			{
				if(TSTATE(backend) == 'T' &&
				   message->r_code == SELECT_RELATION_ERROR)
				{
					pool_debug("pool_rewrite_stmt(select): inside transaction");
					message->rewrite_query = nodeToString(node);
					message->status = pool_parallel_exec(frontend,backend,message->rewrite_query,node,true);
				} 
				else
				{ 
					pool_debug("pool_rewrite_stmt: executed by Master");
					message->rewrite_query = nodeToString(node);
					message->status = OneNode_do_command(frontend, 
														MASTER(backend),
														message->rewrite_query,
														backend->info->database);
				}
			}
			pool_debug("pool_rewrite_stmt: XXX message_code %d",message->r_code);
		}
		break;
			
		case T_InsertStmt:
			examInsertStmt(node,backend,message);

			if(message->r_code == 0 )
			{
				message->status = OneNode_do_command(frontend, 
													CONNECTION(backend,message->r_node),
													message->rewrite_query,
													backend->info->database);
			}
			else if (message->r_code == INSERT_SQL_RESTRICTION)
			{
				pool_send_error_message(frontend, MAJOR(backend), "XX000",
										"pgpool2 sql restriction",
										message->rewrite_query, "", __FILE__,
										__LINE__);

				if(TSTATE(backend) == 'T')
				{
					pool_debug("rewrite_query_stmt(insert): inside transaction");
					message->status = pool_parallel_exec(frontend,backend, "POOL_RESET_TSTATE",node,false);
				}
				else
				{
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

	pool_debug("pool_rewrite_stmt: XXX rule %d",node->type);
	
	return message;
}

#define POOL_PARALLEL "pool_parallel"
#define POOL_LOADBALANCE "pool_loadbalance"

static int check_whereClause(Node *where)
{
	if(IsA(where,SubLink) || IsA(where,RangeSubselect)
		|| IsA(where,FuncCall) || IsA(where, RowExpr))
	{
		return 1;
	}
	else if(IsA(where, A_Expr))
	{
		A_Expr *expr = (A_Expr *) where;
		int count = 0;

		if(expr->lexpr)
		{
			count = check_whereClause(expr->lexpr);
		}

		if(expr->rexpr)
		{
			count += check_whereClause(expr->rexpr);
		}
		return count;
	}
	else
	{	
		return 0;
	}
}

static int direct_parallel_query(SelectStmt *stmt,POOL_CONNECTION_POOL *backend)
{

	if(stmt->lockingClause)
	{
		pool_debug("lockingClasue is exist");	
		return 1;
	}

	if (!stmt->distinctClause && !stmt->into && !stmt->intoColNames &&
		!stmt->groupClause && !stmt->havingClause && !stmt->sortClause &&
		!stmt->limitOffset && !stmt->limitCount &&!stmt->larg && !stmt->rarg)
	{
		if(stmt->fromClause && (list_length(stmt->fromClause) == 1)
			&& list_head(stmt->fromClause) && lfirst(list_head(stmt->fromClause))
			&& IsA(lfirst(list_head(stmt->fromClause)),RangeVar))
		{
			ListCell *lc;
			DistDefInfo *info = NULL;
			RangeVar *var = NULL;

			var = (RangeVar *) lfirst(list_head(stmt->fromClause));
			info = pool_get_dist_def_info (MASTER_CONNECTION(backend)->sp->database, var->schemaname, var->relname);

			if(!info)
				return 0;

			if(stmt->whereClause && 
				(check_whereClause(stmt->whereClause)))
			{
				return 0;
			}

			foreach (lc, stmt->targetList)
       		{
           		Node *n = lfirst(lc);
           		if (IsA(n, ResTarget))
           		{
					ResTarget *target = (ResTarget *) n;
					if (target->val && (IsA(target->val, FuncCall) || 
						IsA(target->val,CaseExpr) || IsA(target->val,SubLink)))
					{
						return 0;
					}
					else if(target->val && (IsA(target->val, TypeCast)))
					{
						TypeCast *type = (TypeCast *)target->val;
						if(type->arg && (IsA(type->arg,FuncCall) ||
						   IsA(type->arg,CaseExpr) || IsA(type->arg,SubLink)))
							return 0;
					}
				}
			}

			return 1;				
		}
	}
	return 0;
}

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

RewriteQuery *is_parallel_query(Node *node, POOL_CONNECTION_POOL *backend)
{
	static RewriteQuery message;
	static ConInfoTodblink dblink;


	if (IsA(node, SelectStmt))
	{
		SelectStmt *stmt;
		Node *n;
		int direct_ok;

		stmt = (SelectStmt *) node;

#if 0
    /* ANALYZE QUERY */
		initMessage(&message,true);
		message.r_code = SELECT_ANALYZE;
		message.is_loadbalance = true;

		initdblink(&dblink,backend);
		nodeToRewriteString(&message,&dblink,node);

		if(message.is_pg_catalog)
		{
			message.is_loadbalance = false;
			message.is_parallel = false;
			pool_debug("is_parallel_query: query is done by loadbalance(pgcatalog)");
			return &message;
		}

		if(message.is_loadbalance)
		{
			message.is_parallel = false;
			pool_debug("is_parallel_query: query is done by loadbalance");
			return &message;
		}
#endif

		/* PARALLEL*/
		direct_ok = direct_parallel_query(stmt,backend);
		if(direct_ok == 1)
		{
			message.rewrite_query = nodeToString(node);
			message.is_parallel = true;
			message.is_loadbalance = false;
			pool_debug("can pool_parallel_exec %s",message.rewrite_query);	
			return &message;
		}

		/* LOADBALANCE OR PARALLEL */
		if (!(stmt->distinctClause || stmt->into || stmt->intoColNames ||
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

					/* PARALLEL */
					if(strcmp(strVal(lfirst(list_head(func->funcname))),
						   POOL_PARALLEL) == 0)
					{
						message.r_code = SEND_PARALLEL_ENGINE;
						message.is_parallel = true;
						message.is_loadbalance = false;
						pool_debug("can pool_parallel_exec %s",message.rewrite_query);	
						return &message;
					} 
					else /* LOADBALANCE */ 
					if(strcmp(strVal(lfirst(list_head(func->funcname))),
						   						POOL_LOADBALANCE) == 0)
					{
						message.r_code = SEND_LOADBALANCE_ENGINE;
						message.is_loadbalance = true;
						message.is_parallel = false;
						pool_debug("can loadbalance_mode %s",message.rewrite_query);	
						return &message;
					}
				} 
			}
		}
		
		message.r_code = SELECT_REWRITE;
    /* ANALYZE QUERY */
		initMessage(&message,true);
		message.r_code = SELECT_ANALYZE;
		message.is_loadbalance = true;

		initdblink(&dblink,backend);
		nodeToRewriteString(&message,&dblink,node);

		if(message.is_pg_catalog)
		{
			message.is_loadbalance = false;
			message.is_parallel = false;
			pool_debug("is_parallel_query: query is done by loadbalance(pgcatalog)");
			return &message;
		}

		if(message.is_loadbalance)
		{
			message.is_parallel = false;
			pool_debug("is_parallel_query: query is done by loadbalance");
			return &message;
		}
	}
	else if (IsA(node, CopyStmt))
	{
		CopyStmt *stmt = (CopyStmt *)node;

		initMessage(&message,false);

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
