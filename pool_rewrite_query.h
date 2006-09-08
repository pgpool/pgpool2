/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2006	PgPool Global Development Group
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
 * pool_rewrite_query.h: rewrite_query
 *
 */

#include "parser/nodes.h"
#include "parser/parser.h"
#include "parser/pg_list.h"
#include "parser/parsenodes.h"
#include "parser/pool_memory.h"
#include "parser/pool_string.h"

/* return code set */
#define INSERT_SQL_RESTRICTION 1
#define SELECT_INIT 2
#define SELECT_BACKEND_CONNECT 3
#define SELECT_NOT_BACKEND_CONNECT 4
#define SELECT_PGCATALOG 5
#define SELECT_CHECK_PGCATALOG 6
#define SELECT_DEFAULT 7
#define SELECT_DEFAULT_INSIDE_DBLINK 8
#define SELECT_AEXPR 9
#define SELECT_AEXPR_FALSE 10
#define SELECT_ONETABLE 11
#define SELECT_ONETABLE_FALSE 12
#define SELECT_RELATION_ERROR 13

typedef struct {
	int r_code; 
	int r_node;
	char *table_relname;
	char *table_alias;
	char *schemaname;
	char *dbname;
	char *rewrite_query;
	POOL_STATUS status;
	NodeTag type;
} RewriteQuery;

typedef struct {
	char *hostaddr;
	char *dbname;
	char *user;
	int   port;
	char *password;
} ConInfoTodblink;

extern RewriteQuery *rewrite_query_stmt(Node *node, POOL_CONNECTION *frontend,POOL_CONNECTION_POOL *backend);
extern void nodeToRewriteString(RewriteQuery *message, ConInfoTodblink *dblink,void *obj);
char *pool_error_message(char *message);
extern int IsSelectpgcatalog(Node *node,POOL_CONNECTION_POOL *backend);
extern char *is_parallel_query(Node *node,POOL_CONNECTION_POOL *backend);

extern POOL_STATUS pool_parallel_exec(POOL_CONNECTION *frontend,POOL_CONNECTION_POOL *backend, char *string,Node *node,bool send_to_frontend);

