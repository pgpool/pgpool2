/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2015	PgPool Global Development Group
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
 *
 * libpcpcp_ext.h - 
 *	  This file contains definitions for structures and
 *	  externs for functions used by frontend libpcp applications.
 */

#ifndef LIBPCP_EXT_H
#define LIBPCP_EXT_H

/*
 * startup packet definitions (v2) stolen from PostgreSQL
 */
#define SM_DATABASE		64
#define SM_USER			32
#define SM_OPTIONS		64
#define SM_UNUSED		64
#define SM_TTY			64

#define MAX_NUM_BACKENDS 128
#define MAX_CONNECTION_SLOTS MAX_NUM_BACKENDS
#define MAX_DB_HOST_NAMELEN	 128
#define MAX_PATH_LENGTH 256

typedef enum {
	CON_UNUSED,		/* unused slot */
    CON_CONNECT_WAIT,		/* waiting for connection starting */
	CON_UP,	/* up and running */
	CON_DOWN		/* down, disconnected */
} BACKEND_STATUS;

/*
 * PostgreSQL backend descriptor. Placed on shared memory area.
 */
typedef struct {
	char backend_hostname[MAX_DB_HOST_NAMELEN];	/* backend host name */
	int backend_port;	/* backend port numbers */
	BACKEND_STATUS backend_status;	/* backend status */
	double backend_weight;	/* normalized backend load balance ratio */
	double unnormalized_weight; /* descripted parameter */
	char backend_data_directory[MAX_PATH_LENGTH];
	unsigned short flag;		/* various flags */
	unsigned long long int standby_delay;		/* The replication delay against the primary */
} BackendInfo;

typedef struct {
	int num_backends;		/* number of used PostgreSQL backends */
	BackendInfo backend_info[MAX_NUM_BACKENDS];
} BackendDesc;
/*
 * Connection pool information. Placed on shared memory area.
 */
typedef struct {
	int			backend_id;	/* backend id */
	char		database[SM_DATABASE];	/* Database name */
	char		user[SM_USER];	/* User name */
	int			major;	/* protocol major version */
	int			minor;	/* protocol minor version */
	int			pid;	/* backend process id */
	int			key;	/* cancel key */
	int			counter; /* used counter */
	time_t 		create_time; /* connection creation time */
	int load_balancing_node; /* load balancing node */
	char		connected;	/* True if frontend connected. Please note
							 * that we use "char" instead of "bool".
							 * Since 3.1, we need to export this
							 * structure so that PostgreSQL C
							 * functions can use. Problem is,
							 * PostgreSQL defines bool itself, and if
							 * we use bool, the size of the structure
							 * might be out of control of
							 * pgpool-II. So we use "char" here.
							 */
} ConnectionInfo;

/*
 * process information
 * This object put on shared memory.
 */
typedef struct {
	pid_t pid; /* OS's process id */
	time_t start_time; /* fork() time */
	ConnectionInfo *connection_info; /* head of the connection info for this process */
	char need_to_restart;		/* If non 0, exit this child process
								 * as soon as current session ends.
								 * Typical case this flag being set is
								 * failback a node in streaming
								 * replication mode.
								 */
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
	int  col_num;			/* number of column*/
	char **col_list;		/* column list */
	char **type_list;		/* type list */
	char *dist_def_func;	/* function name of distribution rule */
	char *prepare_name;		/* prepared statement name */
	int is_created_prepare;	/* is prepare statement created? */
} DistDefInfo;

typedef struct {
	char *dbname;     /* database name */
	char *schema_name;    /* schema name */
	char *table_name;   /* table name */
	int  col_num;     /* number of column*/
	char **col_list;    /* column list */
	char **type_list;   /* type list */
	char *prepare_name;   /* prepared statement name */
	int is_created_prepare; /* is prepare statement created? */
} RepliDefInfo;

typedef struct {
	int has_prepared_statement;	/* true if the current session has prepared statement created */
	char *register_prepared_statement; /* prepared statement name for cache register */
} QueryCacheTableInfo;

typedef struct {
	char *hostname;     /* host name */
	int port;       /* port number */
	char *user;       /* login user name */
	char *password;     /* login password */
	char *schema_name;    /* schema name */
	char *database_name;  /* database name */
	int repli_def_num;    /* number of replication table */
	int dist_def_num;   /* number of distribution table */
	RepliDefInfo *repli_def_slot; /* replication rule list */
	DistDefInfo *dist_def_slot; /* distribution rule list */
	QueryCacheTableInfo query_cache_table_info; /* query cache db session info */
	BACKEND_STATUS system_db_status;
} SystemDBInfo;
/*
 * reporting types
 */
/* some length definitions */
#define POOLCONFIG_MAXIDLEN 4
#define POOLCONFIG_MAXNAMELEN 64
#define POOLCONFIG_MAXVALLEN 512
#define POOLCONFIG_MAXDESCLEN 80
#define POOLCONFIG_MAXIDENTLEN 63
#define POOLCONFIG_MAXPORTLEN 6
#define POOLCONFIG_MAXSTATLEN 2
#define POOLCONFIG_MAXWEIGHTLEN 20
#define POOLCONFIG_MAXDATELEN 128
#define POOLCONFIG_MAXCOUNTLEN 16

/* config report struct*/
typedef struct {
	char name[POOLCONFIG_MAXNAMELEN+1];
	char value[POOLCONFIG_MAXVALLEN+1];
	char desc[POOLCONFIG_MAXDESCLEN+1];
} POOL_REPORT_CONFIG;

/* nodes report struct */
typedef struct {
	char node_id[POOLCONFIG_MAXIDLEN+1];
	char hostname[POOLCONFIG_MAXIDENTLEN+1];
	char port[POOLCONFIG_MAXPORTLEN+1];
	char status[POOLCONFIG_MAXSTATLEN+1];
	char lb_weight[POOLCONFIG_MAXWEIGHTLEN+1];
	char role[POOLCONFIG_MAXWEIGHTLEN+1];
} POOL_REPORT_NODES;

/* processes report struct */
typedef struct {
	char pool_pid[POOLCONFIG_MAXCOUNTLEN+1];
	char start_time[POOLCONFIG_MAXDATELEN+1];
	char database[POOLCONFIG_MAXIDENTLEN+1];
	char username[POOLCONFIG_MAXIDENTLEN+1];
	char create_time[POOLCONFIG_MAXDATELEN+1];
	char pool_counter[POOLCONFIG_MAXCOUNTLEN+1];
} POOL_REPORT_PROCESSES;

/* pools reporting struct */
typedef struct {
	int pool_pid;
	time_t start_time;
	int pool_id;
	int backend_id;
	char database[POOLCONFIG_MAXIDENTLEN+1];
	char username[POOLCONFIG_MAXIDENTLEN+1];
	time_t create_time;
	int pool_majorversion;
	int pool_minorversion;
	int pool_counter;
	int pool_backendpid;
	int pool_connected;
} POOL_REPORT_POOLS;

/* version struct */
typedef struct {
	char version[POOLCONFIG_MAXVALLEN+1];
} POOL_REPORT_VERSION;

struct WdInfo;

extern int pcp_connect(char *hostname, int port, char *username, char *password);
extern void pcp_disconnect(void);
extern int pcp_terminate_pgpool(char mode);
extern int pcp_node_count(void);
extern BackendInfo *pcp_node_info(int nid);
extern int *pcp_process_count(int *process_count);
extern ProcessInfo *pcp_process_info(int pid, int *array_size);
extern SystemDBInfo *pcp_systemdb_info(void);
extern void free_systemdb_info(SystemDBInfo * si);
extern int pcp_detach_node(int nid);
extern int pcp_detach_node_gracefully(int nid);
extern int pcp_attach_node(int nid);
extern POOL_REPORT_CONFIG* pcp_pool_status(int *array_size);
extern void pcp_set_timeout(long sec);
extern int pcp_recovery_node(int nid);
extern void pcp_enable_debug(void);
extern void pcp_disable_debug(void);
extern int pcp_promote_node(int nid);
extern int pcp_promote_node_gracefully(int nid);
extern struct WdInfo *pcp_watchdog_info(int nid);

/* ------------------------------
 * pcp_error.c
 * ------------------------------
 */
//extern ErrorCode errorcode;
//extern void pcp_errorstr(ErrorCode e);

#endif /* LIBPCP_EXT_H */
