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
 * pool_rewrite_query.h: rewrite_query
 *
 */

#ifndef POOL_REWRITE_QUERY_H
#define POOL_REWRITE_QUERY_H

#include "parser/nodes.h"
#include "parser/parser.h"
#include "parser/pg_list.h"
#include "parser/parsenodes.h"
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

/* build Sub-Select`s target List */
typedef struct {
	char **col_list;    /* column list */
	char **type_list;   /* type list */
	int  *return_list;  /* order of col_list */
	int col_num;        /* column number */
	bool valid;         /* return to frontend */
} SelectDefInfo;

/* This struct is used as each table/sub-select of FROM-CLUASE */
typedef struct {
	DistDefInfo *distinfo;     /* distribution table into */
	RepliDefInfo *repliinfo;   /* replication table info */
	SelectDefInfo *selectinfo; /* Sub-Select info */
	char *alias;               /* alias name */
	char state;                /* P = parallel, L = loadbalance S = systemdb(dblink) E = error*/
	int ret_num;               /* build column number */
} RangeInfo;

/* build Virtual Table of FROM-Clause */
typedef struct {
	char **col_list;     /* column list */
	char **type_list;    /* type list */
	char **table_list;   /* table list */
	char *state_list;    /* state of each column */
	int *column_no;      /* order of column */
	int *valid;          /* valid column is true */
	int col_num;         /* virtual table column num */
} VirtualTable;

/* this struct is used by JOIN Expr */
typedef struct {
	char **col_list;      /* column list */
	char **type_list;     /* type list */
	char **table_list;    /* table list */
	char state;           /* P = parallel, L = loadbalance S = systemdb(dblink) E = error*/
	int *valid;           /* valid column is true */
	int col_num;          /* number of column */
	char **using_list;    /* if join expr has using-list, column name is listed up */
	int using_length;     /* column number of using-list */
} JoinTable;

/* this struct is used in optimization of aggregate opr */
typedef struct {
	ColumnRef **usec_p; /* targetlist columns */
	FuncCall **tfunc_p; /* targetlist funcs   */
	ColumnRef **col_p;  /* group by columns   */
	FuncCall **hfunc_p; /* having   funcs */
	int *umapc; /* order of number */ 
	int u_num;  /* targetlist columns num */
	int t_num;  /* targetlist funcs num */
	int c_num;  /* group by column num */
	int h_num;  /* having funcs num */
	int hc_num; /* having column num */
	int s_num;  /* sort funcs num */
	int sc_num; /* sort column num */
	bool opt;   /* optimization flag */
} Aggexpr;

/* main struct of analyzing query */
typedef struct {
	int now_select;  /* rank of select */
	int part;        /* the position of analyzing select statement */
	int last_select; /* caller select rank */
	int call_part;   /* caller's potion */
	int from_num;    /* number for from-clause */
	int larg_count;  /* left arg count */
	int rarg_count;  /* right arg count */
	int ret_count;   /* return list count */
	char state;      /* final state */
	char *table_name; /* table name or virtual table name */
	char partstate[8]; /* state of analyzing part */
	bool select_union; /* if UNION is used, this flag is true */
	bool select_range; /* RangeSubSelect is used  */
	bool aggregate;    /* aggregate optimization ? */
	bool retlock;      /* this is used  */ 
	Aggexpr *aggexpr;   /* Aggexpr in this statement*/
	RangeInfo **range;  /* RangeInfo in from clause */
	int rangeinfo_num;  /* RangeInfo's number in this select statement*/
	VirtualTable *virtual; /* Virtual Table in this select statement */
	JoinTable *join;       /* summary of join table */
	SelectDefInfo *select_ret; /* build return list */
} AnalyzeSelect;

/*
 * This struct is used as Information that relates 
 * to distribution processing of parallel query
 */
typedef struct {
	int r_code;           /* analyze or rewrite */
	int r_node;           /* which node, query is sent */
	int part;             /* part of select statement */
	int rewritelock;      /* dblink start position and lock rewrite */
	int analyze_num;      /* sum of AnalyzeSelect */
	int current_select;   /* position of analyze[] */  
	int ignore_rewrite;   /* dont rewrite */
	int column;           /* column number */
	int virtual_num;     /* Virtual table column number */
	int ret_num;         /* expect return column number */
	bool is_pg_catalog;  /* reference of pg_catalog */
	bool is_loadbalance; /* load balance ? */
	bool is_parallel;    /* can parallel exec ? */
	bool fromClause;     /* having FromClause ? */
	char *table_relname; /* table name */
	char *table_alias;   /* table alias name */
	char *schemaname;    /* schema */
	char *dbname;        /* connect dbname */
	char *rewrite_query; /* execute query */
	char table_state;    /* final state */
	POOL_STATUS status;  /* return POOL_STATUS */
	NodeTag type;        /* Query Type */
	AnalyzeSelect **analyze;  /* point to analyzing result */
} RewriteQuery;

/* This info is used in dblink */
typedef struct {
	char *hostaddr; /* hostname */
	char *dbname;   /* data base name */
	char *user;     /* access user name */
	int   port;     /* access port number */
	char *password;  /* password of connection */
} ConInfoTodblink;

extern RewriteQuery *rewrite_query_stmt(Node *node, POOL_CONNECTION *frontend,POOL_CONNECTION_POOL *backend,RewriteQuery *message);
extern void nodeToRewriteString(RewriteQuery *message, ConInfoTodblink *dblink,void *obj);
char *pool_error_message(char *message);
extern int IsSelectpgcatalog(Node *node,POOL_CONNECTION_POOL *backend);
extern RewriteQuery *is_parallel_query(Node *node,POOL_CONNECTION_POOL *backend);
extern POOL_STATUS pool_parallel_exec(POOL_CONNECTION *frontend,POOL_CONNECTION_POOL *backend, char *string,Node *node,bool send_to_frontend);

POOL_STATUS pool_do_parallel_query(POOL_CONNECTION *frontend,
								   POOL_CONNECTION_POOL *backend,
								   Node *node, bool *parallel, char **string, int *len);

#endif	/* POOL_REWRITE_QUERY_H */

