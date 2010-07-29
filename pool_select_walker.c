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
 */
#include <stdlib.h>
#include <string.h>

#include "pool.h"
#include "pool_select_walker.h"
#include "pool_relcache.h"
#include "parser/parsenodes.h"
#include "pool_session_context.h"

typedef struct {
	bool		has_temp_table;		/* True if temporary table is used */
} SelectContext;

static bool temp_table_walker(Node *node, void *context);
static bool is_temp_table(char *table_name);

/*
 * Return true if this SELECT has temporary table.
 */
bool pool_has_temp_table(Node *node)
{

	SelectContext	ctx;

	if (!IsA(node, SelectStmt))
		return false;

	ctx.has_temp_table = false;

	raw_expression_tree_walker(node, temp_table_walker, &ctx);

	return ctx.has_temp_table;
}

/*
 * Walker function to find a temp table
 */
static bool
temp_table_walker(Node *node, void *context)
{
	SelectContext	*ctx = (SelectContext *) context;

	if (node == NULL)
		return false;

	if (IsA(node, RangeVar))
	{
		RangeVar *rgv = (RangeVar *)node;

		pool_debug("temp_table_walker: relname: %s", rgv->relname);

		if (is_temp_table(rgv->relname))
		{
			ctx->has_temp_table = true;
			return false;
		}
	}
	return raw_expression_tree_walker(node, temp_table_walker, context);
}

/*
 * Judge the table used in a query represented by node is a temporary
 * table or not.
 */
static bool is_temp_table(char *table_name)
{
/*
 * Query to know if pg_class has relistemp column or not.
 * PostgreSQL 8.4 or later has this.
 */
#define HASRELITEMPPQUERY "SELECT count(*) FROM pg_catalog.pg_class AS c, pg_attribute AS a WHERE c.relname = 'pg_class' AND a.attrelid = c.oid AND a.attname = 'relistemp'"

/*
 * Query to know if the target table is a temporary one.
 * This query is valid through PostgreSQL 7.3 to 8.3.
 */
#define ISTEMPQUERY83 "SELECT count(*) FROM pg_class AS c, pg_namespace AS n WHERE c.relname = '%s' AND c.relnamespace = n.oid AND n.nspname ~ '^pg_temp_'"

/*
 * Query to know if the target table is a temporary one.
 * This query is valid PostgreSQL 8.4 or later.
 */
#define ISTEMPQUERY84 "SELECT count(*) FROM pg_catalog.pg_class AS c WHERE c.relname = '%s' AND c.relistemp"

	int hasrelistemp;
	bool result;
	static POOL_RELCACHE *hasrelistemp_cache;
	static POOL_RELCACHE *relcache;
	char *query;
	POOL_CONNECTION_POOL *backend;

	if (table_name == NULL)
	{
			return false;
	}

	backend = pool_get_session_context()->backend;

	/*
	 * Check backend version
	 */
	if (!hasrelistemp_cache)
	{
		hasrelistemp_cache = pool_create_relcache(32, HASRELITEMPPQUERY,
										int_register_func, int_unregister_func,
										false);
		if (hasrelistemp_cache == NULL)
		{
			pool_error("is_temp_table: pool_create_relcache error");
			return false;
		}
	}

	hasrelistemp = pool_search_relcache(hasrelistemp_cache, backend, "pg_class")==0?0:1;
	if (hasrelistemp)
		query = ISTEMPQUERY84;
	else
		query = ISTEMPQUERY83;

	/*
	 * If relcache does not exist, create it.
	 */
	if (!relcache)
	{
		relcache = pool_create_relcache(32, query,
										int_register_func, int_unregister_func,
										true);
		if (relcache == NULL)
		{
			pool_error("is_temp_table: pool_create_relcache error");
			return false;
		}
	}

	/*
	 * Search relcache.
	 */
	result = pool_search_relcache(relcache, backend, table_name)==0?false:true;
	return result;
}
