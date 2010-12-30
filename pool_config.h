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
 * pool_config.h.: pool_config.l related header file
 *
 */

#ifndef POOL_CONFIG_H
#define POOL_CONFIG_H

/*
 * Master/slave sub mode
 */
#define MODE_STREAMREP "stream"		/* Streaming Replication */
#define MODE_SLONY "slony"		/* Slony-I */

/*
 *  Regex support in white and black list function
 */
#include <regex.h>
#define BLACKLIST	0
#define WHITELIST	1
#define PATTERN_ARR_SIZE 16     /* Default length of regex array: 16 patterns */
typedef struct {
  char *pattern;
  int type;
  int flag;
  regex_t regexv;
} RegPattern;


/*
 * configuration paramters
 */
typedef struct {
	char *listen_addresses; /* hostnames/IP addresses to listen on */
    int	port;	/* port # to bind */
	int pcp_port;				/* PCP port # to bind */
	char *socket_dir;		/* pgpool socket directory */
	char *pcp_socket_dir;		/* PCP socket directory */
	int pcp_timeout;			/* PCP timeout for an idle client */
    int	num_init_children;	/* # of children initially pre-forked */
    int	child_life_time;	/* if idle for this seconds, child exits */
    int	connection_life_time;	/* if idle for this seconds, connection closes */
    int	child_max_connections;	/* if max_connections received, child exits */
	int client_idle_limit;		/* If client_idle_limit is n (n > 0), the client is forced to be
								   disconnected after n seconds idle */
	int authentication_timeout; /* maximum time in seconds to complete client authentication */
    int	max_pool;	/* max # of connection pool per child */
    char *logdir;		/* logging directory */
    char *log_destination;      /* log destination: stderr or syslog */
    int syslog_facility;        /* syslog facility: LOCAL0, LOCAL1, ... */
    char *syslog_ident;         /* syslog ident string: pgpool */
    char *pid_file_name;		/* pid file name */
    char *backend_socket_dir;	/* Unix domain socket directory for the PostgreSQL server */
	int replication_mode;		/* replication mode */

	int log_connections;		/* 0:false, 1:true - logs incoming connections */
	int log_hostname;		/* 0:false, 1:true - resolve hostname */
	int enable_pool_hba;		/* 0:false, 1:true - enables pool_hba.conf file authentication */
	char *pool_passwd;	/* pool_passwd file name. "" disables pool_passwd */

	int load_balance_mode;		/* load balance mode */

	int replication_stop_on_mismatch;		/* if there's a data mismatch between master and secondary
											 * start degenration to stop replication mode
											 */

	/* If there's a disagreement with the number of affected tuples in
	 * UPDATE/DELETE, then degenrate the node which is most likely
	 * "minority".  # If false, just abort the transaction to keep the
	 * consistency.*/
	int failover_if_affected_tuples_mismatch;

	int replicate_select; /* if non 0, replicate SELECT statement when load balancing is disabled. */
	char **reset_query_list;		/* comma separated list of quries to be issued at the end of session */
	char **white_function_list;		/* list of functions with no side effetcs */
	char **black_function_list;		/* list of functions with side effetcs */
	int print_timestamp;		/* if non 0, print time stamp to each log line */
	int master_slave_mode;		/* if non 0, operate in master/slave mode */
	char *master_slave_sub_mode;		/* either "slony" or "stream" */
	unsigned long long int delay_threshold;		/* If the standby server delays more than delay_threshold,
										 * any query goes to the primary only. The unit is in bytes.
										 * 0 disables the check. Default is 0.
										 * Note that health_check_period required to be greater than 0
										 * to enable the functionality. */
	char *log_standby_delay;		/* how to log standby lag */
	int connection_cache;		/* if non 0, cache connection pool */
	int health_check_timeout;	/* health check timeout */
	int health_check_period;	/* health check period */
	char *health_check_user;		/* PostgreSQL user name for health check */
	char *failover_command;     /* execute command when failover happens */
	char *failback_command;     /* execute command when failback happens */

	/*
	 * If true, trigger fail over when writing to the backend
	 * communication socket fails. This is the same behavior of
	 * pgpool-II 2.2.x or earlier. If set to false, pgpool will report
	 * an error and disconnect the session.
	 */
	int	fail_over_on_backend_error;

	char *recovery_user;		/* PostgreSQL user name for online recovery */
	char *recovery_password;		/* PostgreSQL user password for online recovery */
	char *recovery_1st_stage_command;   /* Online recovery command in 1st stage */
	char *recovery_2nd_stage_command;   /* Online recovery command in 2nd stage */
	int recovery_timeout;				/* maximum time in seconds to wait for remote start-up */
	int client_idle_limit_in_recovery;		/* If > 0, the client is forced to be
											 *  disconnected after n seconds idle
											 *  This parameter is only valid while in recovery 2nd statge */
	int insert_lock;	/* if non 0, automatically lock table with INSERT to keep SERIAL
						   data consistency */
	int ignore_leading_white_space;		/* ignore leading white spaces of each query */
 	int log_statement; /* 0:false, 1: true - logs all SQL statements */
 	int log_per_node_statement; /* 0:false, 1: true - logs per node detailed SQL statements */

	int parallel_mode;	/* if non 0, run in parallel query mode */

	int enable_query_cache;		/* if non 0, use query cache. 0 by default */

	char *pgpool2_hostname;		/* pgpool2 hostname */
	char *system_db_hostname;	/* system DB hostname */
	int system_db_port;			/* system DB port number */
	char *system_db_dbname;		/* system DB name */
	char *system_db_schema;		/* system DB schema name */
	char *system_db_user;		/* user name to access system DB */
	char *system_db_password;	/* password to access system DB */

	char *lobj_lock_table;		/* table name to lock for rewriting lo_creat */

	int debug_level;			/* debug message verbosity level.
								 * 0: no message, 1 <= : more verbose
								 */

	BackendDesc *backend_desc;	/* PostgreSQL Server description. Placed on shared memory */

	LOAD_BALANCE_STATUS	load_balance_status[MAX_NUM_BACKENDS];	/* to remember which DB node is selected for load balancing */

	/* followings do not exist in the configuration file */
	int replication_enabled;		/* replication mode enabled */
	int master_slave_enabled;		/* master/slave mode enabled */
	int num_reset_queries;		/* number of queries in reset_query_list */
	int num_white_function_list;		/* number of functions in white_function_list */
	int num_black_function_list;		/* number of functions in black_function_list */
	int logsyslog;		/* flag used to start logging to syslog */

	/* ssl configuration */
	int ssl;	/* if non 0, activate ssl support (frontend+backend) */
	char *ssl_cert;	/* path to ssl certificate (frontend only) */
	char *ssl_key;	/* path to ssl key (frontend only) */
	char *ssl_ca_cert;	/* path to root (CA) certificate */
	char *ssl_ca_cert_dir;	/* path to directory containing CA certificates */

	/* followings are for regex support and do not exist in the configuration file */
	RegPattern *lists_patterns; /* Precompiled regex patterns for black/white lists */
	int pattc; /* number of regexp pattern */
	int current_pattern_size; /* size of the regex pattern array */
} POOL_CONFIG;

typedef enum {
	INIT_CONFIG = 1,   /* 0x01 */
	RELOAD_CONFIG = 2  /* 0x02 */
} POOL_CONFIG_CONTEXT;

extern POOL_CONFIG *pool_config;	/* configuration values */

extern int pool_init_config(void);
extern int pool_get_config(char *confpath, POOL_CONFIG_CONTEXT context);
extern int eval_logical(char *str);

/* method use for syslog support */
extern int set_syslog_facility (char *);

/* methods used for regexp support */
extern int add_regex_pattern(char *type, char *s);
extern int growPatternArray (RegPattern item);
extern int pattern_compare(char *str, const int type);

#endif /* POOL_CONFIG_H */
