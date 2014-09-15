/* -*-pgsql-c-*- */
/*
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pool.h"
#include "utils/elog.h"
#include "pool_config.h"
#include "utils/pool_select_walker.h"
#include "utils/pool_relcache.h"
#include "parser/parsenodes.h"
#include "context/pool_session_context.h"
#include "rewrite/pool_timestamp.h"

static bool function_call_walker(Node *node, void *context);
static bool system_catalog_walker(Node *node, void *context);
static bool is_system_catalog(char *table_name);
static bool temp_table_walker(Node *node, void *context);
static bool unlogged_table_walker(Node *node, void *context);
static bool view_walker(Node *node, void *context);
static bool is_temp_table(char *table_name);
static bool	insertinto_or_locking_clause_walker(Node *node, void *context);
static bool is_immutable_function(char *fname);
static bool select_table_walker(Node *node, void *context);
static bool non_immutable_function_call_walker(Node *node, void *context);
static char *strip_quote(char *str);
static char *make_table_name_from_rangevar(RangeVar *rangevar);

/*
 * Return true if this SELECT has function calls *and* supposed to
 * modify database.  We check black/white function list to determine
 * whether the function modifies database.
 */
bool pool_has_function_call(Node *node)
{
	SelectContext	ctx;

	if (!IsA(node, SelectStmt))
		return false;

	ctx.has_function_call = false;

	raw_expression_tree_walker(node, function_call_walker, &ctx);

	return ctx.has_function_call;
}

/*
 * Return true if this SELECT has system catalog table.
 */
bool pool_has_system_catalog(Node *node)
{

	SelectContext	ctx;

	if (!IsA(node, SelectStmt))
		return false;

	ctx.has_system_catalog = false;

	raw_expression_tree_walker(node, system_catalog_walker, &ctx);

	return ctx.has_system_catalog;
}

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
 * Return true if this SELECT has unlogged table.
 */
bool pool_has_unlogged_table(Node *node)
{

	SelectContext	ctx;

	if (!IsA(node, SelectStmt))
		return false;

	ctx.has_unlogged_table = false;

	raw_expression_tree_walker(node, unlogged_table_walker, &ctx);

	return ctx.has_unlogged_table;
}

/*
 * Return true if this SELECT has a view.
 */
bool pool_has_view(Node *node)
{

	SelectContext	ctx;

	if (!IsA(node, SelectStmt))
		return false;

	ctx.has_view = false;

	raw_expression_tree_walker(node, view_walker, &ctx);

	return ctx.has_view;
}

/*
 * Return true if this SELECT has INSERT INTO or FOR SHARE or FOR UPDATE.
 */
bool pool_has_insertinto_or_locking_clause(Node *node)
{
	SelectContext	ctx;

	if (!IsA(node, SelectStmt))
		return false;

	ctx.has_insertinto_or_locking_clause = false;

	raw_expression_tree_walker(node, insertinto_or_locking_clause_walker, &ctx);

	ereport(DEBUG1,
		(errmsg("checking if query has INSERT INTO, FOR SHARE or FOR UPDATE"),
			errdetail("result = %d",ctx.has_insertinto_or_locking_clause)));

	return ctx.has_insertinto_or_locking_clause;
}

/*
 * Search function name in whilelist or blacklist regex array
 * Return 1 on success (found in list)
 * Return 0 when not found in list
 * Return -1 if the given search type doesn't exist.
 * Search type supported are: WHITELIST and BLACKLIST 
 */
int pattern_compare(char *str, const int type, const char *param_name)
{
	int i = 0;

	RegPattern *lists_patterns;
	int *pattc;

	if (strcmp(param_name, "white_function_list") == 0 ||
	    strcmp(param_name, "black_function_list") == 0)
	{
		lists_patterns = pool_config->lists_patterns;
		pattc = &pool_config->pattc;

	} else if (strcmp(param_name, "white_memqcache_table_list") == 0 ||
	           strcmp(param_name, "black_memqcache_table_list") == 0)
	{
		lists_patterns = pool_config->lists_memqcache_table_patterns;
		pattc = &pool_config->memqcache_table_pattc;

	} else {
		ereport(WARNING,
				(errmsg("pattern_compare: unknown paramname %s", param_name)));
		return -1;
	}

	for (i = 0; i < *pattc; i++) {
		if (lists_patterns[i].type != type)
			continue;

		if (regexec(&lists_patterns[i].regexv, str, 0, 0, 0) == 0)
		{
			switch(type) {
			/* return 1 if string matches whitelist pattern */
			case WHITELIST:
				ereport(DEBUG2,
					(errmsg("comparing function name in whitelist regex array"),
						errdetail("pattern_compare: %s (%s) matched: %s",
							   param_name, lists_patterns[i].pattern, str)));
				return 1;
			/* return 1 if string matches blacklist pattern */
			case BLACKLIST:
				ereport(DEBUG2,
					(errmsg("comparing function name in blacklist regex array"),
						 errdetail("pattern_compare: %s (%s) matched: %s",
								   param_name, lists_patterns[i].pattern, str)));
				return 1;
			default:
				ereport(WARNING,
						(errmsg("pattern_compare: \"%s\" unknown pattern match type: \"%s\"", param_name, str)));
				return -1;
			}
		}
		ereport(DEBUG2,
			(errmsg("comparing function name in blacklist/whitelist regex array"),
				 errdetail("pattern_compare: %s (%s) not matched: %s",
						   param_name, lists_patterns[i].pattern, str)));
	}

	/* return 0 otherwise */
	return 0;
}

/*
 * Walker function to find a function call which is supposed to write
 * database.
 */
static bool function_call_walker(Node *node, void *context)
{
	SelectContext	*ctx = (SelectContext *) context;

	if (node == NULL)
		return false;

	if (IsA(node, FuncCall))
	{
		FuncCall *fcall = (FuncCall *)node;
		char *fname;
		int length = list_length(fcall->funcname);

		if (length > 0)
		{
			if (length == 1)	/* no schema qualification? */
			{
				fname = strVal(linitial(fcall->funcname));
			}
			else
			{
				fname = strVal(lsecond(fcall->funcname));		/* with schema qualification */
			}

			ereport(DEBUG1,
				(errmsg("function call walker, function name: \"%s\"", fname)));

			/*
			 * Check white list if any.
			 */
			if (pool_config->num_white_function_list > 0)
			{
				/* Search function in the white list regex patterns */
				if (pattern_compare(fname, WHITELIST, "white_function_list") == 1) {
					/* If the function is found in the white list, we can ignore it */
					return raw_expression_tree_walker(node, function_call_walker, context);
				}
				/*
				 * Since the function was not found in white list, we
				 * have found a writing function.
				 */
				ctx->has_function_call = true;
				return false;
			}

			/*
			 * Check black list if any.
			 */
			if (pool_config->num_black_function_list > 0)
			{
				/* Search function in the black list regex patterns */
				if (pattern_compare(fname, BLACKLIST, "black_function_list") == 1) {
					/* Found. */
					ctx->has_function_call = true;
					return false;
				}
			}
		}
	}
	return raw_expression_tree_walker(node, function_call_walker, context);
}

/*
 * Walker function to find a system catalog
 */
static bool
system_catalog_walker(Node *node, void *context)
{
	SelectContext	*ctx = (SelectContext *) context;

	if (node == NULL)
		return false;

	if (IsA(node, RangeVar))
	{
		RangeVar *rgv = (RangeVar *)node;

		ereport(DEBUG1,
			(errmsg("system catalog walker, checking relation \"%s\"",rgv->relname)));

		if (is_system_catalog(rgv->relname))
		{
			ctx->has_system_catalog = true;
			return false;
		}
	}
	return raw_expression_tree_walker(node, system_catalog_walker, context);
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

		ereport(DEBUG1,
				(errmsg("temporary table walker. checking relation \"%s\"",rgv->relname)));

		if (is_temp_table(rgv->relname))
		{
			ctx->has_temp_table = true;
			return false;
		}
	}
	return raw_expression_tree_walker(node, temp_table_walker, context);
}

/*
 * Walker function to find a unlogged table
 */
static bool
unlogged_table_walker(Node *node, void *context)
{
	SelectContext	*ctx = (SelectContext *) context;
	char *relname;

	if (node == NULL)
		return false;

	if (IsA(node, RangeVar))
	{
		RangeVar *rgv = (RangeVar *)node;
		relname = make_table_name_from_rangevar(rgv);
		
		ereport(DEBUG1,
				(errmsg("unlogged table walker. checking relation \"%s\"",relname)));

		if (is_unlogged_table(relname))
		{
			ctx->has_unlogged_table = true;
			return false;
		}
	}
	return raw_expression_tree_walker(node, unlogged_table_walker, context);
}

/*
 * Walker function to find a view
 */
static bool
view_walker(Node *node, void *context)
{
	SelectContext	*ctx = (SelectContext *) context;
	char *relname;

	if (node == NULL)
		return false;

	if (IsA(node, RangeVar))
	{
		RangeVar *rgv = (RangeVar *)node;
		relname = make_table_name_from_rangevar(rgv);

		ereport(DEBUG1,
				(errmsg("view walker. checking relation \"%s\"",relname)));

		if (is_view(relname))
		{
			ctx->has_view = true;
			return false;
		}
	}
	return raw_expression_tree_walker(node, view_walker, context);
}

/*
 * Judge the table used in a query represented by node is a system
 * catalog or not.
 */
static bool is_system_catalog(char *table_name)
{
/*
 * Query to know if pg_namespace exists. PostgreSQL 7.2 or before doesn't have.
 */
#define HASPGNAMESPACEQUERY "SELECT count(*) FROM pg_catalog.pg_class AS c WHERE c.relname = '%s'"

/*
 * Query to know if the target table belongs pg_catalog schema.
 */
#define ISBELONGTOPGCATALOGQUERY "SELECT count(*) FROM pg_class AS c, pg_namespace AS n WHERE c.relname = '%s' AND c.relnamespace = n.oid AND n.nspname = 'pg_catalog'"

#define ISBELONGTOPGCATALOGQUERY2 "SELECT count(*) FROM pg_class AS c, pg_namespace AS n WHERE c.oid = pgpool_regclass('%s') AND c.relnamespace = n.oid AND n.nspname = 'pg_catalog'"

#define ISBELONGTOPGCATALOGQUERY3 "SELECT count(*) FROM pg_class AS c, pg_namespace AS n WHERE c.oid = to_regclass('%s') AND c.relnamespace = n.oid AND n.nspname = 'pg_catalog'"

	int hasreliscatalog;
	bool result;
	static POOL_RELCACHE *hasreliscatalog_cache;
	static POOL_RELCACHE *relcache;
	POOL_CONNECTION_POOL *backend;

	if (table_name == NULL)
	{
			return false;
	}

	backend = pool_get_session_context(false)->backend;

	/*
	 * Check if pg_namespace exists
	 */
	if (!hasreliscatalog_cache)
	{
		char *query;

		query = HASPGNAMESPACEQUERY;

		hasreliscatalog_cache = pool_create_relcache(pool_config->relcache_size, query,
										int_register_func, int_unregister_func,
										false);
		if (hasreliscatalog_cache == NULL)
		{
			ereport(WARNING,
					(errmsg("unable to create relcache, while checking for system catalog")));
			return false;
		}
	}

	hasreliscatalog = pool_search_relcache(hasreliscatalog_cache, backend, "pg_namespace")==0?0:1;

	if (hasreliscatalog)
	{
		/*
		 * If relcache does not exist, create it.
		 */
		if (!relcache)
		{
			char *query;

			/* PostgreSQL 9.4 or later has to_regclass() */
			if (pool_has_to_regclass())
			{
				query = ISBELONGTOPGCATALOGQUERY3;
			}
			/* pgpool_regclass has been installed */
			else if (pool_has_pgpool_regclass())
			{
				query = ISBELONGTOPGCATALOGQUERY2;
			}
			else
			{
				query = ISBELONGTOPGCATALOGQUERY;
			}

			relcache = pool_create_relcache(pool_config->relcache_size, query,
											int_register_func, int_unregister_func,
											false);
			if (relcache == NULL)
			{
				ereport(WARNING,
						(errmsg("unable to create relcache, while checking for system catalog")));
				return false;
			}
		}
		/*
		 * Search relcache.
		 */
		result = pool_search_relcache(relcache, backend, table_name)==0?false:true;
		return result;
	}

	/*
	 * Pre 7.3. Just check whether the table starts with "pg_".
	 */
	return (strcasecmp(table_name, "pg_") == 0);
}

/*
 * Judge the table used in a query represented by node is a temporary
 * table or not.
 */
static POOL_RELCACHE *is_temp_table_relcache;

static bool is_temp_table(char *table_name)
{
/*
 * Query to know if pg_class has relistemp column or not.
 * PostgreSQL 8.4 and 9.0 have this.
 */
#define HASRELITEMPPQUERY "SELECT count(*) FROM pg_catalog.pg_class AS c, pg_attribute AS a WHERE c.relname = 'pg_class' AND a.attrelid = c.oid AND a.attname = 'relistemp'"

/*
 * Query to know if the target table is a temporary one.  This query
 * is valid in PostgreSQL 7.3 to 8.3 and 9.1 or later.  We do not use
 * regclass (or its variant) here, because temporary tables never have
 * schema qualified name.
 */
#define ISTEMPQUERY83 "SELECT count(*) FROM pg_class AS c, pg_namespace AS n WHERE c.relname = '%s' AND c.relnamespace = n.oid AND n.nspname ~ '^pg_temp_'"

/*
 * Query to know if the target table is a temporary one.  This query
 * is valid in PostgreSQL 8.4 and 9.0. We do not use regclass (or its
 * variant) here, because temporary tables never have schema qualified
 * name.
 */
#define ISTEMPQUERY84 "SELECT count(*) FROM pg_catalog.pg_class AS c WHERE c.relname = '%s' AND c.relistemp"

	int hasrelistemp;
	bool result;
	static POOL_RELCACHE *hasrelistemp_cache;
	char *query;
	POOL_CONNECTION_POOL *backend;

	if (table_name == NULL)
	{
			return false;
	}

	backend = pool_get_session_context(false)->backend;

	/*
	 * Check backend version
	 */
	if (!hasrelistemp_cache)
	{
		hasrelistemp_cache = pool_create_relcache(pool_config->relcache_size, HASRELITEMPPQUERY,
										int_register_func, int_unregister_func,
										false);
		if (hasrelistemp_cache == NULL)
		{
			ereport(WARNING,
					(errmsg("unable to create relcache, while checking for temporary table")));
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
	if (!is_temp_table_relcache)
	{
		is_temp_table_relcache = pool_create_relcache(pool_config->relcache_size, query,
													  int_register_func, int_unregister_func,
													  true);
		if (is_temp_table_relcache == NULL)
		{
			ereport(WARNING,
					(errmsg("unable to create relcache, while checking for temporary table")));
			return false;
		}
	}

	/*
	 * Search relcache.
	 */
	result = pool_search_relcache(is_temp_table_relcache, backend, table_name)==0?false:true;
	return result;
}

/*
 * Discard relcache used by is_temp_table_relcache().
 */
void discard_temp_table_relcache(void)
{
	if (is_temp_table_relcache)
	{
		pool_discard_relcache(is_temp_table_relcache);
		is_temp_table_relcache = NULL;
	}
}

/*
 * Judge the table used in a query represented by node is a unlogged
 * table or not.
 */
bool is_unlogged_table(char *table_name)
{
/*
 * Query to know if pg_class has relpersistence column or not.
 * PostgreSQL 9.1 or later has this.
 */
#define HASRELPERSISTENCEQUERY "SELECT count(*) FROM pg_catalog.pg_class AS c, pg_catalog.pg_attribute AS a WHERE c.relname = 'pg_class' AND a.attrelid = c.oid AND a.attname = 'relpersistence'"

/*
 * Query to know if the target table is a unlogged one.  This query
 * is valid in PostgreSQL 9.1 or later.
 */
#define ISUNLOGGEDQUERY "SELECT count(*) FROM pg_catalog.pg_class AS c WHERE c.relname = '%s' AND c.relpersistence = 'u'"

#define ISUNLOGGEDQUERY2 "SELECT count(*) FROM pg_catalog.pg_class AS c WHERE c.oid = pgpool_regclass('%s') AND c.relpersistence = 'u'"

#define ISUNLOGGEDQUERY3 "SELECT count(*) FROM pg_catalog.pg_class AS c WHERE c.oid = to_regclass('%s') AND c.relpersistence = 'u'"

	int hasrelpersistence;
	static POOL_RELCACHE *hasrelpersistence_cache;
	static POOL_RELCACHE *relcache;
	POOL_CONNECTION_POOL *backend;

	if (table_name == NULL)
	{
			return false;
	}

	backend = pool_get_session_context(false)->backend;

	/*
	 * Check backend version
	 */
	if (!hasrelpersistence_cache)
	{
		hasrelpersistence_cache = pool_create_relcache(pool_config->relcache_size, HASRELPERSISTENCEQUERY,
													   int_register_func, int_unregister_func,
													   false);
		if (hasrelpersistence_cache == NULL)
		{
			ereport(WARNING,
					(errmsg("unable to create relcache, while checking for unlogged table")));
			return false;
		}
	}

	hasrelpersistence = pool_search_relcache(hasrelpersistence_cache, backend, "pg_class")==0?0:1;
	if (hasrelpersistence)
	{
		bool result;
		char *query;

		/* PostgreSQL 9.4 or later has to_regclass() */
		if (pool_has_to_regclass())
		{
			query = ISUNLOGGEDQUERY3;
		}
		/* pgpool_regclass has been installed */
		else if (pool_has_pgpool_regclass())
		{
			query = ISUNLOGGEDQUERY2;
		}
		else
		{
			query = ISUNLOGGEDQUERY;
		}

		/*
		 * If relcache does not exist, create it.
		 */
		if (!relcache)
		{
			relcache = pool_create_relcache(pool_config->relcache_size, query,
											int_register_func, int_unregister_func,
											false);
			if (relcache == NULL)
			{
				ereport(WARNING,
						(errmsg("unable to create relcache, while checking for unlogged table")));
				return false;
			}
		}

		/*
		 * Search relcache.
		 */
		result = pool_search_relcache(relcache, backend, table_name)==0?false:true;
		return result;
	}
	else
	{
		return false;
	}
}

/*
 * Judge the table used in a query is a view or not.
 */
bool is_view(char *table_name)
{
/*
 * Query to know if the target table is a view (including a materialized view).
 */
#define ISVIEWQUERY "SELECT count(*) FROM pg_catalog.pg_class AS c WHERE c.relname = '%s' AND (c.relkind = 'v' OR c.relkind = 'm')"

#define ISVIEWQUERY2 "SELECT count(*) FROM pg_catalog.pg_class AS c WHERE c.oid = pgpool_regclass('%s') AND (c.relkind = 'v' OR c.relkind = 'm')"

#define ISVIEWQUERY3 "SELECT count(*) FROM pg_catalog.pg_class AS c WHERE c.oid = to_regclass('%s') AND (c.relkind = 'v' OR c.relkind = 'm')"

	static POOL_RELCACHE *relcache;
	POOL_CONNECTION_POOL *backend;
	bool result;
	char *query;

	if (table_name == NULL)
	{
			return false;
	}

	backend = pool_get_session_context(false)->backend;

	/* PostgreSQL 9.4 or later has to_regclass() */
	if (pool_has_to_regclass())
	{
		query = ISVIEWQUERY3;
	}
	/* pgpool_regclass has been installed */
	else if (pool_has_pgpool_regclass())
	{
		query = ISVIEWQUERY2;
	}
	else
	{
		query = ISVIEWQUERY;
	}

	if (!relcache)
	{
		relcache = pool_create_relcache(pool_config->relcache_size, query,
										int_register_func, int_unregister_func,
										false);
		if (relcache == NULL)
		{
			ereport(WARNING,
					(errmsg("unable to create relcache, while checking for view")));
			return false;
		}

	}

	/*
	 * Search relcache.
	 */
	result = pool_search_relcache(relcache, backend, table_name)==0?false:true;
	return result;
}

/*
 * Judge if we have pgpool_regclass or not.
 */
bool pool_has_pgpool_regclass(void)
{
/*
 * Query to know if pgpool_regclass exists.
 */
#define HASPGPOOL_REGCLASSQUERY "SELECT count(*) from (SELECT has_function_privilege('%s', 'pgpool_regclass(cstring)', 'execute') WHERE EXISTS(SELECT * FROM pg_catalog.pg_proc AS p WHERE p.proname = 'pgpool_regclass')) AS s"

	bool result;
	static POOL_RELCACHE *relcache;
	POOL_CONNECTION_POOL *backend;
	char *user;

	backend = pool_get_session_context(false)->backend;
	user = MASTER_CONNECTION(backend)->sp->user;

	if (!relcache)
	{
		relcache = pool_create_relcache(pool_config->relcache_size, HASPGPOOL_REGCLASSQUERY,
										int_register_func, int_unregister_func,
										false);
		if (relcache == NULL)
		{
			ereport(WARNING,
					(errmsg("unable to create relcache, while checking pgpool regclass presence")));
			return false;
		}
	}

	result = pool_search_relcache(relcache, backend, user)==0?0:1;
	return result;
}

/*
 * Judge if we have to_regclass or not.
 */
bool pool_has_to_regclass(void)
{
/*
 * Query to know if to_regclass exists.
 */
#define HAS_TOREGCLASSQUERY "SELECT count(*) from (SELECT has_function_privilege('%s', 'to_regclass(cstring)', 'execute') WHERE EXISTS(SELECT * FROM pg_catalog.pg_proc AS p WHERE p.proname = 'to_regclass')) AS s"

	bool result;
	static POOL_RELCACHE *relcache;
	POOL_CONNECTION_POOL *backend;
	char *user;

	backend = pool_get_session_context(false)->backend;
	user = MASTER_CONNECTION(backend)->sp->user;

	if (!relcache)
	{
		relcache = pool_create_relcache(pool_config->relcache_size, HAS_TOREGCLASSQUERY,
										int_register_func, int_unregister_func,
										false);
		if (relcache == NULL)
		{
			ereport(WARNING,
					(errmsg("unable to create relcache, while checking to_regclass presence")));
			return false;
		}
	}

	result = pool_search_relcache(relcache, backend, user)==0?0:1;
	return result;
}

/*
 * Walker function to find intoClause or lockingClause.
 */
static bool	insertinto_or_locking_clause_walker(Node *node, void *context)
{
	SelectContext	*ctx = (SelectContext *) context;

	if (node == NULL)
		return false;

	if (IsA(node, IntoClause) || IsA(node, LockingClause))
	{
		ctx->has_insertinto_or_locking_clause = true;
		return false;
	}
	return raw_expression_tree_walker(node, insertinto_or_locking_clause_walker, ctx);
}

/*
 * Return true if this SELECT has non immutable function calls.
 */
bool pool_has_non_immutable_function_call(Node *node)
{
	SelectContext	ctx;

	if (!IsA(node, SelectStmt))
		return false;

 	ctx.has_non_immutable_function_call = false;

	raw_expression_tree_walker(node, non_immutable_function_call_walker, &ctx);

	ereport(DEBUG1,
		(errmsg("checking if SELECT statement contains the IMMUTABLE function call"),
			 errdetail("result = %d", ctx.has_non_immutable_function_call)));

	return ctx.has_non_immutable_function_call;
}

/*
 * Walker function to find non immutable function call.
 */
static bool non_immutable_function_call_walker(Node *node, void *context)
{
	SelectContext	*ctx = (SelectContext *) context;

	if (node == NULL)
		return false;

	if (IsA(node, FuncCall))
	{
		FuncCall *fcall = (FuncCall *)node;
		char *fname;
		int length = list_length(fcall->funcname);

		if (length > 0)
		{
			if (length == 1)	/* no schema qualification? */
			{
				fname = strVal(linitial(fcall->funcname));
			}
			else
			{
				fname = strVal(lsecond(fcall->funcname));		/* with schema qualification */
			}

			ereport(DEBUG1,
					(errmsg("non immutable function walker. checking function \"%s\"",fname)));

			/* Check system catalog if the function is immutable */
			if (is_immutable_function(fname) == false)
			{
				/* Non immutable function call found */
				ctx->has_non_immutable_function_call = true;
				return false;
			}
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
			ctx->has_non_immutable_function_call = true;
			return false;
		}
	}

	return raw_expression_tree_walker(node, non_immutable_function_call_walker, context);
}

/*
 * Check if the function is stable.
 */
static bool is_immutable_function(char *fname)
{
/*
 * Query to know if the function is IMMUTABLE
 */
#define IS_STABLE_FUNCTION_QUERY "SELECT count(*) FROM pg_catalog.pg_proc AS p WHERE p.proname = '%s' AND p.provolatile = 'i'"
	bool result;
	static POOL_RELCACHE *relcache;
	POOL_CONNECTION_POOL *backend;

	backend = pool_get_session_context(false)->backend;

	if (!relcache)
	{
		relcache = pool_create_relcache(pool_config->relcache_size, IS_STABLE_FUNCTION_QUERY,
										int_register_func, int_unregister_func,
										false);
		if (relcache == NULL)
		{
			ereport(WARNING,
					(errmsg("unable to create relcache, while checking if the function is immutable")));
			return false;
		}
		ereport(DEBUG1,
			(errmsg("checking if the function is IMMUTABLE"),
				 errdetail("relcache created")));
	}

	result = (pool_search_relcache(relcache, backend, fname)==0) ? 0 : 1;

	ereport(DEBUG1,
		(errmsg("checking if the function is IMMUTABLE"),
			 errdetail("search result = %d", result)));
	return result;
}

/*
 * Convert table_name(possibly including schema name) to oid
 */
int pool_table_name_to_oid(char *table_name)
{
/*
 * Query to convert table name to oid
 */
#define TABLE_TO_OID_QUERY "SELECT pgpool_regclass('%s')"
#define TABLE_TO_OID_QUERY2 "SELECT oid FROM pg_class WHERE relname = '%s'"
#define TABLE_TO_OID_QUERY3 "SELECT COALESCE(to_regclass('%s')::oid, 0)"

	int oid = 0;
	static POOL_RELCACHE *relcache;
	POOL_CONNECTION_POOL *backend;
	char *query;

	if (table_name == NULL)
	{
		return oid;
	}

	backend = pool_get_session_context(false)->backend;

	if (pool_has_to_regclass())
	{
		query = TABLE_TO_OID_QUERY3;
	}
	else if (pool_has_pgpool_regclass())
	{
		query = TABLE_TO_OID_QUERY;
	}
	else
	{
		query = TABLE_TO_OID_QUERY2;
	}

	/*
	 * If relcache does not exist, create it.
	 */
	if (!relcache)
	{
		relcache = pool_create_relcache(pool_config->relcache_size, query,
										int_register_func, int_unregister_func,
										true);
		if (relcache == NULL)
		{
			ereport(WARNING,
					(errmsg("unable to create relcache, getting OID from table name")));
			return oid;
		}

		/* Se do not cache if pgpool_regclass() returns 0, which indicates
		 * there's no such a table. In this case we do not want to cache the
		 * state because the table might be created later in this session.
		 */
		relcache->no_cache_if_zero = true;	
	}

	/*
	 * Search relcache.
	 */
	oid = (int)(intptr_t)pool_search_relcache(relcache, backend, table_name);
	return oid;
}

/*
 * Extract table oids from SELECT statement. Returns number of oids.
 * Oids are returned as an int array. The contents of oid array are
 * discarded by next call to this function.
 */
int pool_extract_table_oids_from_select_stmt(Node *node, SelectContext *ctx)
{
	if(!node)
		return 0;
	if (!IsA(node, SelectStmt))
		return 0;

	ctx->num_oids = 0;
	raw_expression_tree_walker(node, select_table_walker, ctx);

	return ctx->num_oids;
}

/*
 * Walker function to extract table oids from SELECT statement.
 */
static bool
select_table_walker(Node *node, void *context)
{
	SelectContext	*ctx = (SelectContext *) context;
	int num_oids;

	if (node == NULL)
		return false;

	if (IsA(node, RangeVar))
	{
		RangeVar *rgv = (RangeVar *)node;
		char *table;
		int oid;
		char *s;

		table = make_table_name_from_rangevar(rgv);
		oid = pool_table_name_to_oid(table);

		if (oid)
		{
			if (POOL_MAX_SELECT_OIDS <= ctx->num_oids)
			{
				ereport(DEBUG1,
					(errmsg("extracting table oids from SELECT statement"),
						 errdetail("number of oids = %d exceeds the maximum limit = %d",
								   ctx->num_oids, POOL_MAX_SELECT_OIDS)));
				return false;
			}

			num_oids = ctx->num_oids++;

			ctx->table_oids[num_oids] = oid;
			s = strip_quote(table);
			strlcpy(ctx->table_names[num_oids], s, POOL_NAMEDATALEN);
			free(s);

			ereport(DEBUG1,
				(errmsg("extracting table oids from SELECT statement"),
					 errdetail("ctx->table_names[%d] = \"%s\"",
							   num_oids, ctx->table_names[num_oids])));
		}
	}

	return raw_expression_tree_walker(node, select_table_walker, context);
}

static char *strip_quote(char *str)
{
	char *after;
	int i = 0;

	after = malloc(sizeof(char) * strlen(str) + 1);

	do {
		if (*str != '"')
		{
			after[i] = *str;
			i++;
		}
		str++;
	} while (*str != '\0');

	after[i] = '\0';

	return after;
}

/*
 * makeRangeVarFromNameList
 *		Utility routine to convert a qualified-name list into RangeVar form.
 *
 * Copied from backend/catalog/namespace.c
 */
RangeVar *
makeRangeVarFromNameList(List *names)
{
	RangeVar   *rel = makeRangeVar(NULL, NULL, -1);

	switch (list_length(names))
	{
		case 1:
			rel->relname = strVal(linitial(names));
			break;
		case 2:
			rel->schemaname = strVal(linitial(names));
			rel->relname = strVal(lsecond(names));
			break;
		case 3:
			rel->catalogname = strVal(linitial(names));
			rel->schemaname = strVal(lsecond(names));
			rel->relname = strVal(lthird(names));
			break;
		default:
			ereport(WARNING,
					(errmsg("invalid relation name, too many indirections, while converting from table name to RangeVar")));
			break;
	}

	return rel;
}

/*
 * Extract table name from RageVar.  Make schema qualification name if
 * necessary.  The returned table name is in static area. So next
 * call to this function will break previous result.
 */
static char *make_table_name_from_rangevar(RangeVar *rangevar)
{
	/*
	 * Table name. Max size is calculated as follows:
	 * schema name(POOL_NAMEDATALEN byte)
	 * + single quote(1 byte)
	 * + table name (POOL_NAMEDATALEN byte)
	 * + NULL(1 byte)
	 */
	static char tablename[POOL_NAMEDATALEN*2+1+1];

	if (rangevar == NULL)
	{
		ereport(WARNING,
				(errmsg("RangeVar argument is NULL, while getting table name from RangeVar")));
		return "";
	}

	if (!IsA(rangevar, RangeVar))
	{
		ereport(WARNING,
				(errmsg("invalid argument, while getting table name from RangeVar")));

		return "";
	}

	*tablename = '\0';

	if (rangevar->schemaname)
	{
		strncpy(tablename, rangevar->schemaname, POOL_NAMEDATALEN);
		strcat(tablename, ".");
	}

	if (!rangevar->relname)
	{
		ereport(WARNING,
				(errmsg("relname is NULL in RangeVar, while getting table name from RangeVar")));

		return "";
	}

	strncat(tablename, rangevar->relname, POOL_NAMEDATALEN);
	ereport(DEBUG1,
			(errmsg("make table name from rangevar: tablename:\"%s\"", tablename)));
	return tablename;
}
