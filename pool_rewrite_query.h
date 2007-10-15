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
#define SELECT_CHECK_PGCATALOG_REPLICATION 6
#define SELECT_DEFAULT 7
#define SELECT_DEFAULT_INSIDE_DBLINK 8
#define SELECT_AEXPR 9
#define SELECT_AEXPR_FALSE 10
#define SELECT_ONETABLE 11
#define SELECT_ONETABLE_FALSE 12
#define SELECT_RELATION_ERROR 13
#define INSERT_DIST_NO_RULE 14
#define SEND_PARALLEL_ENGINE 15
#define SEND_LOADBALANCE_ENGINE 16
#define SELECT_NOT_REPLICATION 17
#define SELECT_REWRITE 18
#define SELECT_ANALYZE 19
#define SELECT_DEFAULT_PREP 20

typedef struct {
	char **col_list;		/* column list */
	char **type_list;		/* type list */
	int  *return_list;
	int col_num;
	bool valid;
}SelectDefInfo;

typedef struct {
	DistDefInfo *distinfo;
	RepliDefInfo *repliinfo;
	SelectDefInfo *selectinfo;
	char *alias;
	char state;
	int ret_num;
} RangeInfo;

typedef struct {
	char **col_list;		/* column list */
	char **type_list;		/* type list */
	char **table_list;		/* table list */
	char *state_list;
	int *column_no;
	int *valid;
	int col_num;	
} VirtualTable;

typedef struct {
	char **col_list;		/* column list */
	char **type_list;		/* type list */
	char **table_list;		/* table list */
	char state;
	int *valid;
	int col_num;
	char **using_list;	
	int using_length;
} JoinTable;

typedef struct {
  int now_select;
	int part;
  int last_select;
	int call_part;
  int from_num;
	int larg_count;
	int rarg_count;
	int ret_count;
  char state;
  char *table_name;
	char partstate[8];
	bool select_union;
	bool select_range;
	bool aggregate;
	RangeInfo **range;
	int rangeinfo_num;
	VirtualTable *virtual;
	JoinTable *join;
	SelectDefInfo *select_ret;
} AnalyzeSelect;

typedef struct {
	int r_code; 
	int r_node;
	int part;
	int rewritelock;
	int analyze_num;
	int current_select;
	int ignore_rewrite;
	int column;
	int virtual_num;
	int ret_num;
	bool is_pg_catalog;
	bool is_loadbalance;
	bool is_parallel;
	bool fromClause;
	char *table_relname;
	char *table_alias;
	char *schemaname;
	char *dbname;
	char *rewrite_query;
	char table_state;
	POOL_STATUS status;
	NodeTag type;
	AnalyzeSelect **analyze;
} RewriteQuery;

typedef struct {
	char *hostaddr;
	char *dbname;
	char *user;
	int   port;
	char *password;
} ConInfoTodblink;

extern RewriteQuery *rewrite_query_stmt(Node *node, POOL_CONNECTION *frontend,POOL_CONNECTION_POOL *backend,RewriteQuery *message);
extern void nodeToRewriteString(RewriteQuery *message, ConInfoTodblink *dblink,void *obj);
char *pool_error_message(char *message);
extern int IsSelectpgcatalog(Node *node,POOL_CONNECTION_POOL *backend);
extern RewriteQuery *is_parallel_query(Node *node,POOL_CONNECTION_POOL *backend);

extern POOL_STATUS pool_parallel_exec(POOL_CONNECTION *frontend,POOL_CONNECTION_POOL *backend, char *string,Node *node,bool send_to_frontend);

