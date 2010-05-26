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
 * pool_relcache.h.: pool_relcache.c related header file
 *
 */

#ifndef POOL_RELCACHE_H
#define POOL_RELCACHE_H
#include "pool.h"

extern POOL_RELCACHE *pool_create_relcache(int cachesize, char *sql,
									func_ptr register_func, func_ptr unregister_func,
									bool issessionlocal);
extern void pool_discard_relcache(POOL_RELCACHE *relcache);
extern void *pool_search_relcache(POOL_RELCACHE *relcache, POOL_CONNECTION_POOL *backend, char *table);
extern void *int_register_func(POOL_SELECT_RESULT *res);
extern void *int_unregister_func(void *data);

#endif /* POOL_RELCACHE_H */
