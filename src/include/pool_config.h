/* -*-pgsql-c-*- */
/*
 *
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
 * pool_config.h.: pool_config.l related header file
 *
 */

#ifndef POOL_CONFIG_H
#define POOL_CONFIG_H

/*
 * watchdog
 */
#include "watchdog/watchdog.h"

/*
 * Master/slave sub mode
 */
#define MODE_STREAMREP 	"stream"	/* Streaming Replication */
#define MODE_SLONY 		"slony"		/* Slony-I */

/*
 * watchdog lifecheck method
 */
#define MODE_HEARTBEAT	"heartbeat"
#define MODE_QUERY 		"query"

#include "utils/regex_array.h"
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
 * Flags for backendN_flag
 */
#define POOL_FAILOVER	0x0001	/* allow or disallow failover */
#define POOL_DISALLOW_TO_FAILOVER(x) ((unsigned short)(x) & POOL_FAILOVER)
#define POOL_ALLOW_TO_FAILOVER(x) (!(POOL_DISALLOW_TO_FAILOVER(x)))

/*
 * configuration parameters
 */
typedef struct {
	char *listen_addresses; /* hostnames/IP addresses to listen on */
    int	port;	/* port # to bind */
	char *pcp_listen_addresses;		/* PCP listen address to listen on */
	int pcp_port;				/* PCP port # to bind */
	char *socket_dir;		/* pgpool socket directory */
	char *pcp_socket_dir;		/* PCP socket directory */
	int pcp_timeout;			/* PCP timeout for an idle client */
    int	num_init_children;	/* # of children initially pre-forked */
    int	listen_backlog_multiplier; /* determines the size of the connection queue */
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
											 * start degeneration to stop replication mode
											 */

	/* If there's a disagreement with the number of affected tuples in
	 * UPDATE/DELETE, then degenerate the node which is most likely
	 * "minority".  # If false, just abort the transaction to keep the
	 * consistency.*/
	int failover_if_affected_tuples_mismatch;

	int replicate_select; /* if non 0, replicate SELECT statement when load balancing is disabled. */
	char **reset_query_list;		/* comma separated list of queries to be issued at the end of session */
	char **white_function_list;		/* list of functions with no side effects */
	char **black_function_list;		/* list of functions with side effects */
	char *log_line_prefix;		/* printf-style string to output at beginning of each log line */
    int log_error_verbosity;    /* controls how much detail about error should be emitted */
    int client_min_messages;    /*controls which message should be sent to client */
    int log_min_messages;       /*controls which message should be emitted to server log */
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
	char *health_check_password; /* password for health check username */
	int health_check_max_retries;	/* health check max retries */
	int health_check_retry_delay;	/* amount of time to wait between retries */
	int connect_timeout;		/* timeout value before giving up connecting to backend */
	int sr_check_period;		/* streaming replication check period */
	char *sr_check_user;		/* PostgreSQL user name streaming replication check */
	char *sr_check_password;	/* password for sr_check_user */
	char *failover_command;     /* execute command when failover happens */
	char *follow_master_command; /* execute command when failover is ended */
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
	int search_primary_node_timeout;	/* maximum time in seconds to search for new primary
										 * node after failover */
	int client_idle_limit_in_recovery;		/* If > 0, the client is forced to be
											 *  disconnected after n seconds idle
											 *  This parameter is only valid while in recovery 2nd stage */
	int insert_lock;	/* if non 0, automatically lock table with INSERT to keep SERIAL
						   data consistency */
	int ignore_leading_white_space;		/* ignore leading white spaces of each query */
 	int log_statement; /* 0:false, 1: true - logs all SQL statements */
 	int log_per_node_statement; /* 0:false, 1: true - logs per node detailed SQL statements */

	int parallel_mode;	/* if non 0, run in parallel query mode */

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
	int num_reset_queries;		/* number of queries in reset_query_list */
	int num_white_function_list;		/* number of functions in white_function_list */
	int num_black_function_list;		/* number of functions in black_function_list */
	int num_white_memqcache_table_list;		/* number of functions in white_memqcache_table_list */
	int num_black_memqcache_table_list;		/* number of functions in black_memqcache_table_list */
	int logsyslog;		/* flag used to start logging to syslog */

	/* ssl configuration */
	int ssl;	/* if non 0, activate ssl support (frontend+backend) */
	char *ssl_cert;	/* path to ssl certificate (frontend only) */
	char *ssl_key;	/* path to ssl key (frontend only) */
	char *ssl_ca_cert;	/* path to root (CA) certificate */
	char *ssl_ca_cert_dir;	/* path to directory containing CA certificates */

	time_t relcache_expire;		/* relation cache life time in seconds */
	int relcache_size;		/* number of relation cache life entry */
	int check_temp_table;		/* enable temporary table check */
	int check_unlogged_table;		/* enable unlogged table check */

	/* followings are for regex support and do not exist in the configuration file */
	RegPattern *lists_patterns; /* Precompiled regex patterns for black/white lists */
	int pattc; /* number of regexp pattern */
	int current_pattern_size; /* size of the regex pattern array */

	int memory_cache_enabled;   /* if true, use the memory cache functionality, false by default */
	char *memqcache_method;   /* Cache store method. Either 'shmem'(shared memory) or 'memcached'. 'shmem' by default */
	char *memqcache_memcached_host;   /* Memcached host name. Mandatory if memqcache_method=memcached. */
	int memqcache_memcached_port;   /* Memcached port number. Mandatory if memqcache_method=memcached. */
	int memqcache_total_size;   /* Total memory size in bytes for storing memory cache. Mandatory if memqcache_method=shmem. */
	int memqcache_max_num_cache;   /* Total number of cache entries. Mandatory if memqcache_method=shmem. */
	int memqcache_expire;   /* Memory cache entry life time specified in seconds. 60 by default. */
	int memqcache_auto_cache_invalidation; /* If true, invalidation of query cache is triggered by corresponding */
										   /* DDL/DML/DCL(and memqcache_expire).  If false, it is only triggered */
										   /* by memqcache_expire.  True by default. */
	int memqcache_maxcache;   /* Maximum SELECT result size in bytes. */
	int memqcache_cache_block_size;   /* Cache block size in bytes. 8192 by default */
	char *memqcache_oiddir;		/* Temporary work directory to record table oids */
	char **white_memqcache_table_list;		/* list of tables to memqcache */
	char **black_memqcache_table_list;		/* list of tables not to memqcache */

	RegPattern *lists_memqcache_table_patterns; /* Precompiled regex patterns for black/white lists */
	int memqcache_table_pattc; /* number of regexp pattern */
	int current_memqcache_table_pattern_size; /* size of the regex pattern array */

	/*
	 * database_redirect_preference_list = 'postgres:primary,mydb[0-4]:1,mydb[5-9]:2'
	 */
	char *database_redirect_preference_list;	/* raw string in pgpool.conf */
	RegArray *redirect_dbnames; /* Precompiled regex patterns for db prefrence list */
	Left_right_tokens *db_redirect_tokens; /* db redirect for dbname and node string */

	/*
	 * app_name_redirect_preference_list = 'psql:primary,myapp[0-4]:1,myapp[5-9]:standby'
	 */
	char *app_name_redirect_preference_list;	/* raw string in pgpool.conf */
	RegArray *redirect_app_names; /* Precompiled regex patterns for app name prefrence list */
	Left_right_tokens *app_name_redirect_tokens; /* app name redirect for app_name and node string */

	/*
	 * if on, ignore SQL comments when judging if load balance or query cache
	 * is possible. If off, SQL comments effectively prevent the judgment
	 * (pre 3.4 behavior). For backward compatibilty sake, default is off.
	 */
	int allow_sql_comments;

	/*
	 * add for watchdog
	 */
	int use_watchdog;					/* if non 0, use watchdog */
	char *wd_lifecheck_method;			/* method of lifecheck. 'heartbeat' or 'query' */
	int clear_memqcache_on_escalation;	/* if no 0, clear query cache on shmem when escalating */
    char *wd_escalation_command;		/* Executes this command at escalation on new active pgpool.*/
	char *wd_hostname;					/* watchdog hostname */
	int wd_port;						/* watchdog port */
	WdDesc * other_wd;					/* watchdog lists */
	char * trusted_servers;				/* icmp reachable server list (A,B,C) */
	char * delegate_IP;					/* delegate IP address */
	int  wd_interval;					/* lifecheck interval (sec) */
	char *wd_authkey;					/* Authentication key for watchdog communication */
	char * ping_path;					/* path to ping command */
	char * ifconfig_path;				/* path to ifconfig command */
	char * if_up_cmd;					/* ifup command */
	char * if_down_cmd;					/* ifdown command */
	char * arping_path;					/* path to arping command */
	char * arping_cmd;					/* arping command */
	int  wd_life_point;					/* life point (retry times at lifecheck) */
	char *wd_lifecheck_query;			/* lifecheck query */
	char *wd_lifecheck_dbname;			/* Database name connected for lifecheck */
	char *wd_lifecheck_user;			/* PostgreSQL user name for watchdog */
	char *wd_lifecheck_password;		/* password for watchdog user */
	int wd_heartbeat_port;				/* Port number for heartbeat lifecheck */
	int wd_heartbeat_keepalive;			/* Interval time of sending heartbeat signal (sec) */
	int wd_heartbeat_deadtime;			/* Deadtime interval for heartbeat signal (sec) */
	WdHbIf hb_if[WD_MAX_IF_NUM];		/* interface devices */
	int num_hb_if;						/* number of interface devices */
} POOL_CONFIG;

typedef enum {
	INIT_CONFIG = 1,   /* 0x01 */
	RELOAD_CONFIG = 2  /* 0x02 */
} POOL_CONFIG_CONTEXT;

extern POOL_CONFIG *pool_config;	/* configuration values */

extern int pool_init_config(void);
extern int pool_get_config(char *confpath, POOL_CONFIG_CONTEXT context);
extern int eval_logical(char *str);
extern char *pool_flag_to_str(unsigned short flag);

/* method use for syslog support */
extern int set_syslog_facility (char *);

/* methods used for regexp support */
extern int add_regex_pattern(char *type, char *s);
extern int growFunctionPatternArray(RegPattern item);
extern int growMemqcacheTablePatternArray(RegPattern item);

#endif /* POOL_CONFIG_H */
