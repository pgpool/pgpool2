/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2009	PgPool Global Development Group
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
 * pool_relcache.c: Per process relation cache modules
 */
#include "config.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "pool.h"
#include "pool_relcache.h"
#include "pool_session_context.h"

/*
 * Create relation cache
 */
POOL_RELCACHE *pool_create_relcache(int cachesize, char *sql,
									func_ptr register_func, func_ptr unregister_func,
									bool issessionlocal)
{
	POOL_RELCACHE *p;
	PoolRelCache *ip;

	if (cachesize < 0)
	{
		pool_error("pool_create_relcache: wrong cache size: %d", cachesize);
		return NULL;
	}

	ip = (PoolRelCache *)malloc(sizeof(PoolRelCache)*cachesize);
	if (ip == NULL)
	{
		pool_error("pool_create_relcache: cannot allocate memory %zd", sizeof(PoolRelCache)*cachesize);
		return NULL;
	}
	memset(ip, 0, sizeof(PoolRelCache)*cachesize);

	p = (POOL_RELCACHE *)malloc(sizeof(POOL_RELCACHE));
	if (p == NULL)
	{
		pool_error("pool_create_relcache: cannot allocate memory %zd", sizeof(POOL_RELCACHE));
		free(ip);
		return NULL;
	}

	p->num = cachesize;
	strncpy(p->sql, sql, sizeof(p->sql)-1);
	p->register_func = register_func;
	p->unregister_func = unregister_func;
	p->cache_is_session_local = issessionlocal;
	p->cache = ip;
	
	return p;
}
/*
 * Discard relation cache.
 */
void pool_discard_relcache(POOL_RELCACHE *relcache)
{
	int i;

	for (i=0;i<relcache->num;i++)
	{
		(*relcache->unregister_func)(relcache->cache[i].data);
	}
	free(relcache->cache);
	free(relcache);
}

/*
 * Search relcache. If found, return user data. Otherwise return 0.
 * If not found in cache, do the query and store the result into cache and return it.
 */
void *pool_search_relcache(POOL_RELCACHE *relcache, POOL_CONNECTION_POOL *backend, char *table)
{
	char *rel;
	char *dbname;
	int i;
	int maxrefcnt = INT_MAX;
	char query[1024];
	POOL_SELECT_RESULT *res = NULL;
	int index = 0;
	int local_session_id;

	/* Eliminate double quotes */
	rel = malloc(strlen(table)+1);
	if (!rel)
	{
		pool_error("pool_search_relcache: malloc failed");
		return NULL;
	}

	local_session_id = pool_get_local_session_id();
	if (local_session_id < 0)
	{
		pool_error("pool_search_relcache: pool_get_local_session_id failed");
		return NULL;
	}

	for(i=0;*table;table++)
	{
		if (*table != '"')
			rel[i++] = *table;
	}
	rel[i] = '\0';

	/* Obtain database name */
	dbname = MASTER_CONNECTION(backend)->sp->database;

	/* Look for cache first */
	for (i=0;i<relcache->num;i++)
	{
		/*
		 * If cache is session local, we need to check session id
		 */
		if (relcache->cache_is_session_local)
		{
			if (relcache->cache[i].session_id != local_session_id)
				continue;
		}

		if (strcasecmp(relcache->cache[i].dbname, dbname) == 0 &&
			strcasecmp(relcache->cache[i].relname, rel) == 0)
		{
			/* Found */
			if (relcache->cache[i].refcnt < INT_MAX)
				relcache->cache[i].refcnt++;
			free(rel);
			return relcache->cache[i].data;
		}
	}

	/* Not in cache. Check the system catalog */
	snprintf(query, sizeof(query), relcache->sql, rel);

	per_node_statement_log(backend, REAL_MASTER_NODE_ID, query);

	if (do_query(backend->slots[REAL_MASTER_NODE_ID]->con, query, &res, MAJOR(backend)) != POOL_CONTINUE)
	{
		pool_error("pool_search_relcache: do_query failed");
		if (res)
			free_select_result(res);
		free(rel);
		return NULL;
	}

	/*
	 * Look for replacement in cache
	 */
	for (i=0;i<relcache->num;i++)
	{
		/*
		 * If cache is session local, we can discard old cache immediately
		 */
		if (relcache->cache_is_session_local)
		{
			if (relcache->cache[i].session_id != local_session_id)
			{
				index = i;
				break;
			}
		}

		if (relcache->cache[i].refcnt == 0)
		{
			/* Found empty slot */
			index = i;
			break;
		}
		else if (relcache->cache[i].refcnt < maxrefcnt)
		{
			maxrefcnt = relcache->cache[i].refcnt;
			index = i;
		}
	}

	/* Register cache */
	strncpy(relcache->cache[index].dbname, dbname, MAX_ITEM_LENGTH);
	strncpy(relcache->cache[index].relname, rel, MAX_ITEM_LENGTH);
	relcache->cache[index].refcnt = 1;
	relcache->cache[index].session_id = local_session_id;
	free(rel);

	/*
	 * Call user defined unregister/register fuction.
	 */
	(*relcache->unregister_func)(relcache->cache[index].data);
	relcache->cache[index].data = (*relcache->register_func)(res);
	free_select_result(res);

	return 	relcache->cache[index].data;
}

/*
 * Standard register/unregister function for "SELECT count(*)" type
 * query. Returns row count.
 */
void *int_register_func(POOL_SELECT_RESULT *res)
{
	if (res->numrows >= 1)
		return (void *)atol(res->data[0]);
	return (void *)0;
}

void *int_unregister_func(void *data)
{
	/* Nothing to do since no memory was allocated */
	return NULL;
}

void *string_register_func(POOL_SELECT_RESULT *res)
{
	char	*ret = NULL;
	int		 i;

	if (res->numrows == 0)
		return NULL;

	for (i = 0; i < res->numrows; i++)
	{
		ret = strdup(res->data[i]);
		break;
	}

	return (void *)ret;
}

void *string_unregister_func(void *data)
{
	free(data);
	return data;
}
