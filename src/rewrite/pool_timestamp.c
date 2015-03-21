/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2013	PgPool Global Development Group
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
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "pool.h"
#include "rewrite/pool_timestamp.h"
#include "utils/elog.h"
#include "utils/pool_relcache.h"
#include "utils/pool_select_walker.h"
#include "pool_config.h"
#include "parser/parsenodes.h"
#include "parser/parser.h"
#include "utils/palloc.h"

typedef struct {
	char	*attrname;	/* attribute name */
	char	*adsrc;		/* default value expression */
	int		 use_timestamp;
} TSAttr;

typedef struct {
	int		relnatts;
	TSAttr	attr[1];
} TSRel;

typedef struct {
	A_Const					*ts_const;
	POOL_CONNECTION_POOL	*backend;
	char					*relname;
	int						 num_params;		/* num of original params (for Parse) */
	bool		 			 rewrite_to_params;	/* true if rewritten to param insread of const */
	bool		 			 rewrite;			/* has rewritten? */
	List					*params;			/* list of additional params */
} TSRewriteContext;

static void *ts_register_func(POOL_SELECT_RESULT *res);
static void *ts_unregister_func(void *data);
static TSRel *relcache_lookup(TSRewriteContext *ctx);
static bool isStringConst(Node *node, const char *str);
static bool rewrite_timestamp_walker(Node *node, void *context);
static bool rewrite_timestamp_insert(InsertStmt *i_stmt, TSRewriteContext *ctx);
static bool rewrite_timestamp_update(UpdateStmt *u_stmt, TSRewriteContext *ctx);
static char *get_current_timestamp(POOL_CONNECTION_POOL *backend);
static Node *makeTsExpr(TSRewriteContext *ctx);
static A_Const *makeStringConstFromQuery(POOL_CONNECTION_POOL *backend, char *expression);
bool raw_expression_tree_walker(Node *node, bool (*walker) (), void *context);

POOL_RELCACHE	*ts_relcache;


static void *
ts_register_func(POOL_SELECT_RESULT *res)
{
/* Number of result columns included in res */
#define NUM_COLS		3

	TSRel	*rel;
	int		 i;

	if (res->numrows == 0)
		return NULL;

	rel = (TSRel *) malloc(sizeof(TSRel) + sizeof(TSAttr) * (res->numrows - 1));

	for (i = 0; i < res->numrows; i++)
	{
		int index = 0;

		rel->attr[i].attrname = strdup(res->data[i * NUM_COLS + index]);
		index++;

		if (res->data[i * NUM_COLS + index])
			rel->attr[i].adsrc = strdup(res->data[i * NUM_COLS + index]);
		else
			rel->attr[i].adsrc = NULL;

		index++;

		rel->attr[i].use_timestamp = *(res->data[i * NUM_COLS + index]) == 't';
		ereport(DEBUG1,
			(errmsg("timestamp register function"),
				errdetail("attrname %s adsrc %s use_timestamp = %d",
					   rel->attr[i].attrname, (rel->attr[i].adsrc? rel->attr[i].adsrc:"NULL"),
					   rel->attr[i].use_timestamp)));
	}

	rel->relnatts = res->numrows;
	return (void *) rel;
}


static void *
ts_unregister_func(void *data)
{
	TSRel	*rel = (TSRel *) data;
	if (rel)
		free(rel);
	return (void *)0;
}


static TSRel*
relcache_lookup(TSRewriteContext *ctx)
{
#define ATTRDEFQUERY "SELECT attname, d.adsrc, coalesce((d.adsrc LIKE '%%now()%%' OR d.adsrc LIKE '%%''now''::text%%')" \
	" AND (a.atttypid = 'timestamp'::regtype::oid OR" \
	" a.atttypid = 'timestamp with time zone'::regtype::oid OR" \
	" a.atttypid = 'date'::regtype::oid OR" \
	" a.atttypid = 'time'::regtype::oid OR" \
	" a.atttypid = 'time with time zone'::regtype::oid)" \
    " , false)" \
	" FROM pg_catalog.pg_class c, pg_catalog.pg_attribute a " \
	" LEFT JOIN pg_catalog.pg_attrdef d ON (a.attrelid = d.adrelid AND a.attnum = d.adnum)" \
	" WHERE c.oid = a.attrelid AND a.attnum >= 1 AND a.attisdropped = 'f' AND c.relname = '%s'" \
	" ORDER BY a.attnum"

#define ATTRDEFQUERY2 "SELECT attname, d.adsrc, coalesce((d.adsrc LIKE '%%now()%%' OR d.adsrc LIKE '%%''now''::text%%')" \
	" AND (a.atttypid = 'timestamp'::regtype::oid OR" \
	" a.atttypid = 'timestamp with time zone'::regtype::oid OR" \
	" a.atttypid = 'date'::regtype::oid OR" \
	" a.atttypid = 'time'::regtype::oid OR" \
	" a.atttypid = 'time with time zone'::regtype::oid)" \
    " , false)" \
	" FROM pg_catalog.pg_class c, pg_catalog.pg_attribute a " \
	" LEFT JOIN pg_catalog.pg_attrdef d ON (a.attrelid = d.adrelid AND a.attnum = d.adnum)" \
	" WHERE c.oid = a.attrelid AND a.attnum >= 1 AND a.attisdropped = 'f' AND c.oid = pgpool_regclass('%s')" \
	" ORDER BY a.attnum"

#define ATTRDEFQUERY3 "SELECT attname, d.adsrc, coalesce((d.adsrc LIKE '%%now()%%' OR d.adsrc LIKE '%%''now''::text%%')" \
	" AND (a.atttypid = 'timestamp'::regtype::oid OR" \
	" a.atttypid = 'timestamp with time zone'::regtype::oid OR" \
	" a.atttypid = 'date'::regtype::oid OR" \
	" a.atttypid = 'time'::regtype::oid OR" \
	" a.atttypid = 'time with time zone'::regtype::oid)" \
    " , false)" \
	" FROM pg_catalog.pg_class c, pg_catalog.pg_attribute a " \
	" LEFT JOIN pg_catalog.pg_attrdef d ON (a.attrelid = d.adrelid AND a.attnum = d.adnum)" \
	" WHERE c.oid = a.attrelid AND a.attnum >= 1 AND a.attisdropped = 'f' AND c.oid = to_regclass('%s')" \
	" ORDER BY a.attnum"

	char *query;

	if (pool_has_to_regclass())
	{
		query = ATTRDEFQUERY3;		
	}
	else if (pool_has_pgpool_regclass())
	{
		query = ATTRDEFQUERY2;
	}
	else
	{
		query = ATTRDEFQUERY;
	}

	if (!ts_relcache)
	{
		ts_relcache = pool_create_relcache(pool_config->relcache_size, query, ts_register_func, ts_unregister_func, false);

		if (ts_relcache == NULL)
		{
			ereport(WARNING,
				(errmsg("unable to create relcache")));
			return NULL;
		}
	}

	return (TSRel *) pool_search_relcache(ts_relcache, ctx->backend, ctx->relname);
}

static Node *
makeTsExpr(TSRewriteContext *ctx)
{
	ParamRef		*param;

	if (!ctx->rewrite_to_params)
		return (Node *) ctx->ts_const;

	param = makeNode(ParamRef);
	param->number = 0;
	ctx->params = lappend(ctx->params, param);
	return (Node *) param;
}


static bool
isStringConst(Node *node, const char *str)
{
	A_Const		*a_const;
	Value		val;

	if (!IsA(node, A_Const))
		return false;

	a_const = (A_Const *) node;
	val = a_const->val;

	if (val.type == T_String && val.val.str && strcmp(str, val.val.str) == 0)
		return true;

	return false;
}


bool
isSystemType(Node *node, const char *name)
{
	TypeName	*typename;

	if (!IsA(node, TypeName))
		return false;

	typename = (TypeName *) node;
	if (list_length(typename->names) == 2 &&
		strcmp("pg_catalog", strVal(linitial(typename->names))) == 0 &&
		strcmp(name, strVal(lsecond(typename->names))) == 0)
		return true;

	return false;
}


static bool
isSystemTypeCast(Node *node, const char *name)
{
	TypeCast	*typecast;

	if (!IsA(node, TypeCast))
		return false;

	typecast = (TypeCast *) node;
	return isSystemType((Node *) typecast->typeName, name);
}

/*
 * walker function for raw_expression_tree_walker
 */
static bool
rewrite_timestamp_walker(Node *node, void *context)
{
	TSRewriteContext	*ctx = (TSRewriteContext *) context;

	if (node == NULL)
		return false;

	if (nodeTag(node) == T_FuncCall)
	{
		/* `now()' FuncCall */
		FuncCall	*fcall = (FuncCall *) node;

		if ((list_length(fcall->funcname) == 1 &&
			 strcmp("now", strVal(linitial(fcall->funcname))) == 0) ||
			(list_length(fcall->funcname) == 2 &&
			 strcmp("pg_catalog", strVal(linitial(fcall->funcname))) == 0 &&
			 strcmp("now", strVal(lsecond(fcall->funcname))) == 0))
		{
			TypeCast	*tc = makeNode(TypeCast);
			tc->arg = makeTsExpr(ctx);
			tc->typeName = SystemTypeName("text");

			fcall->funcname = SystemFuncName("timestamptz");
			fcall->args = list_make1(tc);
			ctx->rewrite = true;
		}
	}
	else if (IsA(node, TypeCast))
	{
		/* CURRENT_DATE, CURRENT_TIME, LOCALTIMESTAMP, LOCALTIME etc.*/
		TypeCast	*tc = (TypeCast *) node;

		if ((isSystemType((Node *) tc->typeName, "date") ||
			 isSystemType((Node *) tc->typeName, "timestamp") ||
			 isSystemType((Node *) tc->typeName, "timestamptz") ||
			 isSystemType((Node *) tc->typeName, "time") ||
			 isSystemType((Node *) tc->typeName, "timetz")))
		{
			/* rewrite `'now'::timestamp' and `'now'::text::timestamp' both */
			if (isSystemTypeCast(tc->arg, "text"))
				tc = (TypeCast *) tc->arg;

			if (isStringConst(tc->arg, "now"))
			{
				tc->arg = (Node *) makeTsExpr(ctx);
				ctx->rewrite = true;
			}
		}
	}
	else if (IsA(node, ParamRef))
	{
		ParamRef	*param = (ParamRef *) node;

		/* count how many params in original query */
		if (ctx->num_params < param->number)
			ctx->num_params = param->number;
	}

	return raw_expression_tree_walker(node, rewrite_timestamp_walker, context);
}


/*
 * Get `now()' from MASTER node
 */
static char *
get_current_timestamp(POOL_CONNECTION_POOL *backend)
{
	POOL_SELECT_RESULT *res;
	static char		timestamp[64];

	do_query(MASTER(backend), "SELECT now()", &res, MAJOR(backend));

	if (res->numrows != 1)
	{
		free_select_result(res);
		return NULL;
	}

	strlcpy(timestamp, res->data[0], sizeof(timestamp));

	free_select_result(res);
	return timestamp;
}


/*
 * rewrite InsertStmt
 */
static bool
rewrite_timestamp_insert(InsertStmt *i_stmt, TSRewriteContext *ctx)
{
	int		 i;
	bool	 rewrite = false;
	TSRel	*relcache;


	if (i_stmt->selectStmt == NULL)
	{
		List		*values = NIL;
		SelectStmt	*selectStmt = makeNode(SelectStmt);

		relcache = relcache_lookup(ctx);
		if (relcache == NULL)
			return false;

		/*
		 * INSERT INTO rel DEFAULT VALUES
		 * rewrite to:
		 * INSERT INTO rel VALUES (DEFAULT, '2009-..',...)
		 */

		for (i = 0; i < relcache->relnatts; i++)
		{
			if (relcache->attr[i].use_timestamp)
			{
				rewrite = true;
				if (ctx->rewrite_to_params)
					values = lappend(values, makeTsExpr(ctx));
				else
					values = lappend(values,
									 makeStringConstFromQuery(ctx->backend, relcache->attr[i].adsrc));
			}
			else
				values = lappend(values, makeNode(SetToDefault));
		}
		if (rewrite)
		{
			selectStmt->valuesLists = list_make1(values);
			i_stmt->selectStmt = (Node *) selectStmt;
		}

		return rewrite;
	}
	else if (IsA(i_stmt->selectStmt, SelectStmt))
	{
		SelectStmt	*selectStmt = (SelectStmt *) i_stmt->selectStmt;
		ListCell	*lc_row, *lc_val, *lc_col;

		/*
		 * Rewrite `now()' call to timestamp literal.
		 */
		raw_expression_tree_walker(
				(Node *) selectStmt,
				rewrite_timestamp_walker, (void *) ctx);

		rewrite = ctx->rewrite;

		/*
		 * if `INSERT INTO rel SELECT ...'
		 */
		if (selectStmt->valuesLists == NIL)
			return rewrite;

		relcache = relcache_lookup(ctx);
		if (relcache == NULL)
			return false;

		if (i_stmt->cols == NIL)
		{
			/*
			 * INSERT INTO rel VALUES (...)
			 *
			 * CREATE TABLE r1 (
			 *   i1 int,
			 *   t1 timestamp default now(),
			 *   i2 int,
			 *   t2 timestamp default now(),
			 * )
			 *
			 * INSERT INTO r1 VALUES (1, DEFAULT);
			 * rewrite to:
			 * INSERT INTO r1 VALUES (1, '20xx-xx-xx...', DEFAULT, '2..')
			 */
			foreach (lc_row, selectStmt->valuesLists)
			{
				List		*values = lfirst(lc_row);

				i = 0;
				foreach (lc_val, values)
				{
					if (relcache->attr[i].use_timestamp == true && IsA(lfirst(lc_val), SetToDefault))
					{
						rewrite = true;
						if (ctx->rewrite_to_params)
							lfirst(lc_val) = makeTsExpr(ctx);
						else
							lfirst(lc_val) = makeStringConstFromQuery(ctx->backend, relcache->attr[i].adsrc);
					}
					i++;
				}

				/* fill rest columns */
				for (; i < relcache->relnatts; i++)
				{
					if (relcache->attr[i].use_timestamp == true)
					{
						rewrite = true;
						if (ctx->rewrite_to_params)
							values = lappend(values, makeTsExpr(ctx));
						else
							values = lappend(values,
											 makeStringConstFromQuery(ctx->backend, relcache->attr[i].adsrc));
					}
					else
						values = lappend(values, makeNode(SetToDefault));
				}
			}
		}
		else
		{
			/*
			 * INSERT INTO rel(col1, col2) VALUES (val, val2)
			 *
			 * if timestamp column is not given by column list
			 * add colname to column list and add timestamp to values list.
			 */
			int			append_columns = 0;
			int			*append_columns_list;
			ResTarget	*col;

			append_columns_list = (int *)malloc(sizeof(int)*relcache->relnatts);

			for (i = 0; i < relcache->relnatts; i++)
			{
				if (relcache->attr[i].use_timestamp == false)
					continue;

				foreach (lc_col, i_stmt->cols)
				{
					col = lfirst(lc_col);

					if (strcmp(relcache->attr[i].attrname, col->name) == 0)
						break;
				}

				if (lc_col == NULL)
				{
					/* column not found in query, append it.*/
					rewrite = true;
					col = makeNode(ResTarget);
					col->name = relcache->attr[i].attrname;
					col->indirection = NIL;
					col->val = NULL;
					i_stmt->cols = lappend(i_stmt->cols, col);
					append_columns_list[append_columns++] = i;
				}
			}

			foreach (lc_row, selectStmt->valuesLists)
			{
				List		*values = lfirst(lc_row);

				/* replace DEFAULT to literal */
				forboth (lc_col, i_stmt->cols, lc_val, values)
				{
					col = lfirst(lc_col);
					for (i = 0; i < relcache->relnatts; i++)
					{
						if (strcmp(relcache->attr[i].attrname, col->name) == 0)
							break;
					}

					if (relcache->attr[i].use_timestamp == true && IsA(lfirst(lc_val), SetToDefault))
					{
						rewrite = true;
						if (ctx->rewrite_to_params)
							lfirst(lc_val) = makeTsExpr(ctx);
						else
							lfirst(lc_val) = makeStringConstFromQuery(ctx->backend, relcache->attr[i].adsrc);
					}
				}

				/* add ts_const to values list */
				for (i = 0; i < append_columns; i++)
				{
					if (ctx->rewrite_to_params)
						values = lappend(values, makeTsExpr(ctx));
					else
						values = lappend(values,
										 makeStringConstFromQuery(ctx->backend, relcache->attr[append_columns_list[i]].adsrc));
				}
			}
			free(append_columns_list);
		}
	}

	return rewrite;
}


/*
 * rewrite UpdateStmt
 */
static bool
rewrite_timestamp_update(UpdateStmt *u_stmt, TSRewriteContext *ctx)
{
	TSRel			*relcache = NULL;
	ListCell		*lc;
	bool			 rewrite;

	/* rewrite "update ... set col1 = now()" */
	raw_expression_tree_walker(
			(Node *) u_stmt->targetList,
			rewrite_timestamp_walker, (void *) ctx);

	raw_expression_tree_walker(
			(Node *) u_stmt->whereClause,
			rewrite_timestamp_walker, (void *) ctx);

	raw_expression_tree_walker(
			(Node *) u_stmt->fromClause,
			rewrite_timestamp_walker, (void *) ctx);

	rewrite = ctx->rewrite;

	/* rewrite "update ... set col1 = default" */
	foreach (lc, u_stmt->targetList)
	{
		ResTarget	*res = (ResTarget *) lfirst(lc);
		int			 i;

		if (IsA(res->val, SetToDefault))
		{
			relcache = relcache_lookup(ctx);
			if (relcache == NULL)
				return false;

			for (i = 0; i < relcache->relnatts; i++)
			{
				if (strcmp(relcache->attr[i].attrname, res->name) == 0)
				{
					if (relcache->attr[i].use_timestamp)
					{
						if (ctx->rewrite_to_params)
							res->val = (Node *) makeTsExpr(ctx);
						else
							res->val = (Node *)makeStringConstFromQuery(ctx->backend, relcache->attr[i].adsrc);
						rewrite = true;
					}
					break;
				}
			}
		}
	}
	return rewrite;
}

/*
 * Rewrite `now()' to timestamp literal.
 * returns query string as palloced string, or NULL if not to need rewrite.
 */
char *
rewrite_timestamp(POOL_CONNECTION_POOL *backend, Node *node,
				  bool rewrite_to_params, POOL_SENT_MESSAGE *message)
{
	TSRewriteContext	ctx;
	Node			*stmt;
	bool			 rewrite = false;
	char			*timestamp;
	char			*rewrite_query;

	if (node == NULL)
		return NULL;

	if (!REPLICATION)
		return NULL;

	/* init context */
	ctx.ts_const = makeNode(A_Const);
	ctx.ts_const->val.type = T_String;
	ctx.rewrite_to_params = rewrite_to_params;
	ctx.backend = backend;
	ctx.num_params = 0;
	ctx.rewrite = false;
	ctx.params = NIL;

	/*
	 * Prepare?
	 */
	if (IsA(node, PrepareStmt))
	{
		stmt = ((PrepareStmt *) node)->query;
		ctx.rewrite_to_params = true;
	}
	else
		stmt = node;

	if (IsA(stmt, InsertStmt))
	{
		InsertStmt *i_stmt = (InsertStmt *) stmt;
		ctx.relname = nodeToString(i_stmt->relation);
		rewrite = rewrite_timestamp_insert(i_stmt, &ctx);
	}
	else if (IsA(stmt, UpdateStmt))
	{
		UpdateStmt *u_stmt = (UpdateStmt *) stmt;
		ctx.relname = nodeToString(u_stmt->relation);
		rewrite = rewrite_timestamp_update(u_stmt, &ctx);
	}
	else if (IsA(stmt, DeleteStmt))
	{
		DeleteStmt *d_stmt = (DeleteStmt *) stmt;
		raw_expression_tree_walker(
				(Node *) d_stmt->usingClause,
				rewrite_timestamp_walker, (void *) &ctx);

		raw_expression_tree_walker(
				(Node *) d_stmt->whereClause,
				rewrite_timestamp_walker, (void *) &ctx);
		rewrite = ctx.rewrite;
	}
	else if (IsA(stmt, ExecuteStmt))
	{
		ExecuteStmt *e_stmt = (ExecuteStmt *) stmt;

		/* rewrite params */
		raw_expression_tree_walker(
				(Node *) e_stmt->params,
				rewrite_timestamp_walker, (void *) &ctx);

		rewrite = ctx.rewrite;

		/* add params */
		if (message)
		{
			int		i;

			for (i = 0; i < message->num_tsparams; i++)
			{
				e_stmt->params = lappend(e_stmt->params, ctx.ts_const);
				rewrite = true;
			}
		}
	}
	else
		;

	/* save number of parameters in original query */
	if (message)
		message->query_context->num_original_params = ctx.num_params;

	if (!rewrite)
		return NULL;

	if (ctx.rewrite_to_params && message)
	{
		ListCell	*lc;
		int			 num = ctx.num_params + 1;

		/* renumber params */
		foreach (lc, ctx.params)
		{
			ParamRef		*param = (ParamRef *) lfirst(lc);
			param->number = num++;
		}

		/* save to portal */
		message->num_tsparams = list_length(ctx.params);

		/* add param type */
		if (IsA(node, PrepareStmt))
		{
			int				 i;
			PrepareStmt		*p_stmt = (PrepareStmt *) node;

			for (i = 0; i < message->num_tsparams; i++)
				p_stmt->argtypes =
				   	lappend(p_stmt->argtypes, SystemTypeName("timestamptz"));
		}
	}
	else
	{
		timestamp = get_current_timestamp(backend);
		if (timestamp == NULL)
		{
			ereport(WARNING,
					(errmsg("rewrite timestamp failed, unable to get current timestamp")));
			return NULL;
		}

		ctx.ts_const->val.val.str = timestamp;
	}
	rewrite_query = nodeToString(node);

	return rewrite_query;
}


/*
 * rewrite Bind message to add parameter
 */
char *
bind_rewrite_timestamp(POOL_CONNECTION_POOL *backend,
					   POOL_SENT_MESSAGE *message,
					   const char *orig_msg, int *len)
{
	int16		 tmp2,
				 num_params,
				 num_formats;
	int32		 tmp4;
	int			 i,
				 ts_len,
				 copy_len,
				 num_org_params;
	const char	*copy_from;
	char		*ts,
				*copy_to,
				*new_msg;

#ifdef TIMESTAMPDEBUG
	fprintf(stderr, "message length:%d\n", *len);
	for(i=0;i<*len;i++)
	{
		fprintf(stderr, "%02x ", orig_msg[i]);
	}
#endif

	ts = get_current_timestamp(backend);
	if (ts == NULL)
	{
		ereport(WARNING,
			(errmsg("bind rewrite timestamp failed, unable to get current timestamp")));
		return NULL;
	}

	ts_len = strlen(ts);

	/* enlarge length for timestamp parameters */
	*len += (ts_len + sizeof(int32)) * message->num_tsparams;
	/* allocate extra memory for parameter formats */
	num_org_params = message->query_context->num_original_params;
	new_msg = copy_to = (char *) palloc(*len + sizeof(int16) * (message->num_tsparams + num_org_params));
	copy_from = orig_msg;

	/* portal_name */
	copy_len = strlen(copy_from) + 1;
	memcpy(copy_to, copy_from, copy_len);
	copy_to += copy_len; copy_from += copy_len;

	/* stmt_name */
	copy_len = strlen(copy_from) + 1;
	memcpy(copy_to, copy_from, copy_len);
	copy_to += copy_len; copy_from += copy_len;

	/* format code */
	memcpy(&tmp2, copy_from, sizeof(int16));
	copy_len = sizeof(int16);
	tmp2 = num_formats = ntohs(tmp2);

	if (num_formats >= 1)
	{
		/* one means the specified format code is applied all original parameters */
		if (num_formats == 1)
		{
			*len += (num_org_params - 1) * sizeof(int16);
			tmp2 += num_org_params - 1;
		}

		/* enlarge message length for timestamp parameter's formats */
		*len += message->num_tsparams * sizeof(int16);
		tmp2 += message->num_tsparams;
	}

	tmp2 = htons(tmp2);
	memcpy(copy_to, &tmp2, copy_len);	/* copy number of format codes */
	copy_to += copy_len; copy_from += copy_len;

	copy_len = num_formats * sizeof(int16);

	memcpy(copy_to, copy_from, copy_len);		/* copy format codes */
	copy_to += copy_len; copy_from += copy_len;

	if (num_formats >= 1)
	{
		/* copy the specified format code as numbers of original parameters */
		if (num_formats == 1)
		{
			memcpy(copy_to, copy_from, (num_org_params - 1) * sizeof(int16));
			copy_to += (num_org_params - 1) * sizeof(int16);
		}

		/* set additional format codes to zero(text) */
		memset(copy_to, 0, message->num_tsparams * sizeof(int16));
		copy_to += sizeof(int16) * message->num_tsparams;
	}

	/* num params */
	memcpy(&tmp2, copy_from, sizeof(int16));
	copy_len = sizeof(int16);
	num_params = ntohs(tmp2);
	tmp2 = htons(num_params + message->num_tsparams);
	memcpy(copy_to, &tmp2, sizeof(int16));
	copy_to += copy_len; copy_from += copy_len;

	/* params */
	copy_len = 0;
	for (i = 0; i < num_params; i++)
	{
		memcpy(&tmp4, copy_from + copy_len, sizeof(int32));
		tmp4 = ntohl(tmp4);		/* param length */
		copy_len += sizeof(int32);

		/* If param length is -1, it indicates that the value is NULL
		 * and we don't have value slot. So we don't add up copy_len.
		 */
		if (tmp4 > 0)
		{
			copy_len += tmp4;
		}
	}
	memcpy(copy_to, copy_from, copy_len);
	copy_to += copy_len; copy_from += copy_len;

	tmp4 = htonl(ts_len);
	for (i = 0; i < message->num_tsparams; i++)
	{
		memcpy(copy_to, &tmp4, sizeof(int32));
		copy_to += sizeof(int32);
		memcpy(copy_to, ts, ts_len);
		copy_to += ts_len;
	}

	/* result format code */
	memcpy(&tmp2, copy_from, sizeof(int16));
	copy_len = sizeof(int16);
	copy_len += sizeof(int16) * ntohs(tmp2);
	memcpy(copy_to, copy_from, copy_len);

	return new_msg;
}

static A_Const *makeStringConstFromQuery(POOL_CONNECTION_POOL *backend, char *expression)
{
	A_Const *con;
	POOL_SELECT_RESULT *res;
	char query[1024];
	int len;
	char *str;

	snprintf(query, sizeof(query), "SELECT %s", expression);
	do_query(MASTER(backend), query, &res, MAJOR(backend));

	if (res->numrows != 1)
	{
		free_select_result(res);
		return NULL;
	}

	len = strlen(res->data[0]) + 1;
	str = palloc(len);
	strcpy(str, res->data[0]);
	free_select_result(res);

	con = makeNode(A_Const);
	con->val.type = T_String;
	con->val.val.str = str;
	return con;
}

/* from nodeFuncs.c start */

/*
 * raw_expression_tree_walker --- walk raw parse trees
 *
 * This has exactly the same API as expression_tree_walker, but instead of
 * walking post-analysis parse trees, it knows how to walk the node types
 * found in raw grammar output.  (There is not currently any need for a
 * combined walker, so we keep them separate in the name of efficiency.)
 * Unlike expression_tree_walker, there is no special rule about query
 * boundaries: we descend to everything that's possibly interesting.
 *
 * Currently, the node type coverage extends to SelectStmt and everything
 * that could appear under it, but not other statement types.
 */
bool
			raw_expression_tree_walker(Node *node, bool (*walker) (), void *context)
{
	ListCell   *temp;

	/*
	 * The walker has already visited the current node, and so we need only
	 * recurse into any sub-nodes it has.
	 */
	if (node == NULL)
		return false;

	/* Guard against stack overflow due to overly complex expressions */
	/*
	check_stack_depth();
	*/

	switch (nodeTag(node))
	{
		case T_SetToDefault:
		case T_CurrentOfExpr:
		case T_Integer:
		case T_Float:
		case T_String:
		case T_BitString:
		case T_Null:
		case T_ParamRef:
		case T_A_Const:
		case T_A_Star:
			/* primitive node types with no subnodes */
			break;
		case T_Alias:
			/* we assume the colnames list isn't interesting */
			break;
		case T_RangeVar:
			return walker(((RangeVar *) node)->alias, context);
		case T_SubLink:
			{
				SubLink    *sublink = (SubLink *) node;

				if (walker(sublink->testexpr, context))
					return true;
				/* we assume the operName is not interesting */
				if (walker(sublink->subselect, context))
					return true;
			}
			break;
		case T_CaseExpr:
			{
				CaseExpr   *caseexpr = (CaseExpr *) node;

				if (walker(caseexpr->arg, context))
					return true;
				/* we assume walker doesn't care about CaseWhens, either */
				foreach(temp, caseexpr->args)
				{
					CaseWhen   *when = (CaseWhen *) lfirst(temp);

					Assert(IsA(when, CaseWhen));
					if (walker(when->expr, context))
						return true;
					if (walker(when->result, context))
						return true;
				}
				if (walker(caseexpr->defresult, context))
					return true;
			}
			break;
		case T_RowExpr:
			/* Assume colnames isn't interesting */
			return walker(((RowExpr *) node)->args, context);
		case T_CoalesceExpr:
			return walker(((CoalesceExpr *) node)->args, context);
		case T_MinMaxExpr:
			return walker(((MinMaxExpr *) node)->args, context);
		case T_XmlExpr:
			{
				XmlExpr    *xexpr = (XmlExpr *) node;

				if (walker(xexpr->named_args, context))
					return true;
				/* we assume walker doesn't care about arg_names */
				if (walker(xexpr->args, context))
					return true;
			}
			break;
		case T_NullTest:
			return walker(((NullTest *) node)->arg, context);
		case T_BooleanTest:
			return walker(((BooleanTest *) node)->arg, context);
		case T_JoinExpr:
			{
				JoinExpr   *join = (JoinExpr *) node;

				if (walker(join->larg, context))
					return true;
				if (walker(join->rarg, context))
					return true;
				if (walker(join->quals, context))
					return true;
				if (walker(join->alias, context))
					return true;
				/* using list is deemed uninteresting */
			}
			break;
		case T_IntoClause:
			{
				IntoClause *into = (IntoClause *) node;

				if (walker(into->rel, context))
					return true;
				/* colNames, options are deemed uninteresting */
			}
			break;
		case T_List:
			foreach(temp, (List *) node)
			{
				if (walker((Node *) lfirst(temp), context))
					return true;
			}
			break;
		case T_SelectStmt:
			{
				SelectStmt *stmt = (SelectStmt *) node;

				if (walker(stmt->distinctClause, context))
					return true;
				if (walker(stmt->intoClause, context))
					return true;
				if (walker(stmt->targetList, context))
					return true;
				if (walker(stmt->fromClause, context))
					return true;
				if (walker(stmt->whereClause, context))
					return true;
				if (walker(stmt->groupClause, context))
					return true;
				if (walker(stmt->havingClause, context))
					return true;
				if (walker(stmt->windowClause, context))
					return true;
				if (walker(stmt->withClause, context))
					return true;
				if (walker(stmt->valuesLists, context))
					return true;
				if (walker(stmt->sortClause, context))
					return true;
				if (walker(stmt->limitOffset, context))
					return true;
				if (walker(stmt->limitCount, context))
					return true;
				if (walker(stmt->lockingClause, context))
					return true;
				if (walker(stmt->larg, context))
					return true;
				if (walker(stmt->rarg, context))
					return true;
			}
			break;
		case T_A_Expr:
			{
				A_Expr	   *expr = (A_Expr *) node;

				if (walker(expr->lexpr, context))
					return true;
				if (walker(expr->rexpr, context))
					return true;
				/* operator name is deemed uninteresting */
			}
			break;
		case T_ColumnRef:
			/* we assume the fields contain nothing interesting */
			break;
		case T_FuncCall:
			{
				FuncCall   *fcall = (FuncCall *) node;

				if (walker(fcall->args, context))
					return true;
				if (walker(fcall->over, context))
					return true;
				/* function name is deemed uninteresting */
			}
			break;
		case T_A_Indices:
			{
				A_Indices  *indices = (A_Indices *) node;

				if (walker(indices->lidx, context))
					return true;
				if (walker(indices->uidx, context))
					return true;
			}
			break;
		case T_A_Indirection:
			{
				A_Indirection *indir = (A_Indirection *) node;

				if (walker(indir->arg, context))
					return true;
				if (walker(indir->indirection, context))
					return true;
			}
			break;
		case T_A_ArrayExpr:
			return walker(((A_ArrayExpr *) node)->elements, context);
		case T_ResTarget:
			{
				ResTarget  *rt = (ResTarget *) node;

				if (walker(rt->indirection, context))
					return true;
				if (walker(rt->val, context))
					return true;
			}
			break;
		case T_TypeCast:
			{
				TypeCast   *tc = (TypeCast *) node;

				if (walker(tc->arg, context))
					return true;
				if (walker(tc->typeName, context))
					return true;
			}
			break;
		case T_SortBy:
			return walker(((SortBy *) node)->node, context);
		case T_WindowDef:
			{
				WindowDef  *wd = (WindowDef *) node;

				if (walker(wd->partitionClause, context))
					return true;
				if (walker(wd->orderClause, context))
					return true;
			}
			break;
		case T_RangeSubselect:
			{
				RangeSubselect *rs = (RangeSubselect *) node;

				if (walker(rs->subquery, context))
					return true;
				if (walker(rs->alias, context))
					return true;
			}
			break;
		case T_RangeFunction:
			{
				RangeFunction *rf = (RangeFunction *) node;

				if (walker(rf->functions, context))
					return true;
				if (walker(rf->alias, context))
					return true;
			}
			break;
		case T_TypeName:
			{
				TypeName   *tn = (TypeName *) node;

				if (walker(tn->typmods, context))
					return true;
				if (walker(tn->arrayBounds, context))
					return true;
				/* type name itself is deemed uninteresting */
			}
			break;
		case T_ColumnDef:
			{
				ColumnDef  *coldef = (ColumnDef *) node;

				if (walker(coldef->typeName, context))
					return true;
				if (walker(coldef->raw_default, context))
					return true;
				/* for now, constraints are ignored */
			}
			break;
		case T_LockingClause:
			return walker(((LockingClause *) node)->lockedRels, context);
		case T_XmlSerialize:
			{
				XmlSerialize *xs = (XmlSerialize *) node;

				if (walker(xs->expr, context))
					return true;
				if (walker(xs->typeName, context))
					return true;
			}
			break;
		case T_WithClause:
			return walker(((WithClause *) node)->ctes, context);
		case T_CommonTableExpr:
			return walker(((CommonTableExpr *) node)->ctequery, context);
		default:
			/*
			elog(ERROR, "unrecognized node type: %d",
				 (int) nodeTag(node));
				 */
			break;
	}
	return false;
}
/* from nodeFuncs.c end */

