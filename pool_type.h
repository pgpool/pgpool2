/* -*-pgsql-c-*- */
/*
 *
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
 * pool_type.h.: type definition header file
 *
 */

#ifndef POOL_TYPE_H
#define POOL_TYPE_H

#include <sys/types.h>

/*
 * startup packet definitions (v2) stolen from PostgreSQL
 */
#define SM_DATABASE		64
#define SM_USER			32
#define SM_OPTIONS		64
#define SM_UNUSED		64
#define SM_TTY			64

#define MAX_NUM_BACKENDS 128
#define MAX_CONNECTION_SLOTS 128
#define MAX_DB_HOST_NAMELEN	 128

typedef enum {
	CON_UNUSED,		/* unused slot */
    CON_CONNECT_WAIT,		/* waiting for connection starting */
	CON_UP,	/* up and running */
	CON_DOWN		/* down, disconnected */
} BACKEND_STATUS;

typedef enum {
	LOAD_UNSELECTED = 0,
	LOAD_SELECTED
} LOAD_BALANCE_STATUS;

/*
 * PostgreSQL backend descriptor. Placed on shared memory area.
 */
typedef struct {
	char backend_hostname[MAX_DB_HOST_NAMELEN];	/* backend host name */
	int backend_port;	/* backend port numbers */
	BACKEND_STATUS backend_status;	/* backend status */
	double backend_weight;	/* normalized backend load balance ratio */
} BackendInfo;

typedef struct {
	int num_backends;		/* number of used PostgreSQL backends */
	BackendInfo backend_info[MAX_NUM_BACKENDS];
} BackendDesc;

/*
 * Connection pool information. Placed on shared memory area.
 */
typedef struct {
	char		database[SM_DATABASE];	/* Database name */
	char		user[SM_USER];	/* User name */
	int			major;	/* protocol major version */
	int			minor;	/* protocol minor version */
	int			counter; /* used counter */
	time_t 		create_time; /* connection creation time */
} ConnectionInfo;


/*
 * process information
 * This object put on shared memory.
 */
typedef struct {
	pid_t pid; /* OS's process id */
	time_t start_time; /* fork() time */
	ConnectionInfo *connection_info; /* connection information */
} ProcessInfo;

/*
 * 
 * system db structure
 */
typedef struct {
	char *dbname;			/* database name */
	char *schema_name;		/* schema name */
	char *table_name;		/* table name */
	char *dist_key_col_name;/* column name for dist key */
	int  dist_key_col_id;	/* column index id for dist key */
	int  col_num;			/* number of clumn*/
	char **col_list;		/* column list */
	char **type_list;		/* type list */
	char *dist_def_func;	/* function name of distribution rule */
	char *prepare_name;		/* prepared statement name */
	int is_created_prepare;	/* is prepare statement created? */
} DistDefInfo;

typedef struct {
	int has_prepared_statement;	/* true if the current session has prepared statement created */
	char *register_prepared_statement; /* prepared statement name for cache register */
} QueryCacheTableInfo;

typedef struct {
	char *hostname;			/* host name */
	int port;				/* port number */
	char *user;				/* login user name */
	char *password;			/* login password */
	char *schema_name;		/* schema name */
	char *database_name;	/* database name */
	int dist_def_num;		/* number of distribution table */
	DistDefInfo *dist_def_slot; /* distribution rule list */
	QueryCacheTableInfo query_cache_table_info; /* query cache db session info */
	BACKEND_STATUS system_db_status;
} SystemDBInfo;


#endif /* POOL_TYPE_H */
