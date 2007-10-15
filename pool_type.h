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

#include "config.h"
#include <sys/types.h>
#include <sys/socket.h>

/* Define common boolean type. C++ and BEOS already has it so exclude them. */
#ifdef c_plusplus
#ifndef __cplusplus
#define __cplusplus
#endif /* __cplusplus */
#endif /* c_plusplus */

#ifndef __BEOS__
#ifndef __cplusplus
#ifndef bool
typedef char bool;
#endif
#ifndef true
#define true ((bool) 1)
#endif
#ifndef TRUE
#define TRUE ((bool) 1)
#endif
#ifndef false
#define false ((bool) 0)
#endif
#ifndef FALSE
#define FALSE ((bool) 0)
#endif
#endif /* not C++ */
#endif /* __BEOS__ */

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
#define MAX_PATH_LENGTH 256

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
	double unnormalized_weight; /* descripted parameter */
	char backend_data_directory[MAX_PATH_LENGTH];
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
	int load_balancing_node; /* load balancing node */
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
	char *dbname;     /* database name */
	char *schema_name;    /* schema name */
	char *table_name;   /* table name */
	int  col_num;     /* number of clumn*/
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
 *  It seems that sockaddr_storage is now commonly used in place of sockaddr.
 *  So, define it if it is not define yet, and create new SockAddr structure
 *  that uses sockaddr_storage.
 */
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE

#ifndef HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY
#define ss_family __ss_family
#else
#error struct sockaddr_storage does not provide an ss_family member
#endif /* HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY */
#endif /* HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY */

#ifdef HAVE_STRUCT_SOCKADDR_STORAGE___SS_LEN
#define ss_len __ss_len
#define HAVE_STRUCT_SOCKADDR_STORAGE_SS_LEN 1
#endif /* HAVE_STRUCT_SOCKADDR_STORAGE___SS_LEN */

#else /* !HAVE_STRUCT_SOCKADDR_STORAGE */

/* Define a struct sockaddr_storage if we don't have one. */
struct sockaddr_storage
{
	union
	{
		struct sockaddr sa;		/* get the system-dependent fields */
		long int ss_align; /* ensures struct is properly aligned. original uses int64 */
		char ss_pad[128];		/* ensures struct has desired size */
	}
	ss_stuff;
};

#define ss_family   ss_stuff.sa.sa_family
/* It should have an ss_len field if sockaddr has sa_len. */
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
#define ss_len      ss_stuff.sa.sa_len
#define HAVE_STRUCT_SOCKADDR_STORAGE_SS_LEN 1
#endif
#endif /* HAVE_STRUCT_SOCKADDR_STORAGE */

typedef struct
{
	struct sockaddr_storage addr;
	/* ACCEPT_TYPE_ARG3 - Third argument type of accept().
	 * It is defined in ac_func_accept_argtypes.m4
	 */
	ACCEPT_TYPE_ARG3 salen;
}
SockAddr;

/* UserAuth type used for HBA which indicates the authentication method */
typedef enum UserAuth
{
	uaReject,
	/*  uaKrb4, */
	/*  uaKrb5, */
	uaTrust
	/*  uaIdent, */
	/*  uaPassword, */
	/*  uaCrypt, */
	/*  uaMD5 */
#ifdef USE_PAM
	,uaPAM
#endif /* USE_PAM */
}
UserAuth;

#define AUTH_REQ_OK         0   /* User is authenticated  */
#define AUTH_REQ_KRB4       1   /* Kerberos V4 */
#define AUTH_REQ_KRB5       2   /* Kerberos V5 */
#define AUTH_REQ_PASSWORD   3   /* Password */
#define AUTH_REQ_CRYPT      4   /* crypt password */
#define AUTH_REQ_MD5        5   /* md5 password */
#define AUTH_REQ_SCM_CREDS  6   /* transfer SCM credentials */

typedef unsigned int AuthRequest;

#endif /* POOL_TYPE_H */
