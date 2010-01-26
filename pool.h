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
 * pool.h.: master definition header file
 *
 */

#ifndef POOL_H
#define POOL_H

#include "config.h"
#include "pool_type.h"
#include "pool_signal.h"
#include "libpq-fe.h"
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <limits.h>

#ifdef USE_SSL
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

/* undef this if you have problems with non blocking accept() */
#define NONE_BLOCK

#define POOLMAXPATHLEN 8192

/* configuration file name */
#define POOL_CONF_FILE_NAME "pgpool.conf"

/* PCP user/password file name */
#define PCP_PASSWD_FILE_NAME "pcp.conf"

/* HBA configuration file name */
#define HBA_CONF_FILE_NAME "pool_hba.conf"

/* pid file directory */
#define DEFAULT_LOGDIR "/tmp"

/* Unix domain socket directory */
#define DEFAULT_SOCKET_DIR "/tmp"

/* pid file name */
#define DEFAULT_PID_FILE_NAME "/var/run/pgpool/pgpool.pid"

/* status file name */
#define STATUS_FILE_NAME "pgpool_status"

typedef enum {
	POOL_CONTINUE = 0,
	POOL_IDLE,
	POOL_END,
	POOL_ERROR,
	POOL_FATAL,
	POOL_DEADLOCK
} POOL_STATUS;


typedef enum {
	INIT_CONFIG = 1,   /* 0x01 */
	RELOAD_CONFIG = 2  /* 0x02 */
} POOL_CONFIG_CONTEXT;


/* protocol major version numbers */
#define PROTO_MAJOR_V2	2
#define PROTO_MAJOR_V3	3

/* Cancel packet proto major */
#define PROTO_CANCEL	80877102

/*
 * In protocol 3.0 and later, the startup packet length is not fixed, but
 * we set an arbitrary limit on it anyway.	This is just to prevent simple
 * denial-of-service attacks via sending enough data to run the server
 * out of memory.
 */
#define MAX_STARTUP_PACKET_LENGTH 10000


typedef struct StartupPacket_v2
{
	int			protoVersion;		/* Protocol version */
	char		database[SM_DATABASE];	/* Database name */
	char		user[SM_USER];	/* User name */
	char		options[SM_OPTIONS];	/* Optional additional args */
	char		unused[SM_UNUSED];		/* Unused */
	char		tty[SM_TTY];	/* Tty for debug output */
} StartupPacket_v2;

/* startup packet info */
typedef struct
{
	char *startup_packet;		/* raw startup packet without packet length (malloced area) */
	int len;					/* raw startup packet length */
	int major;	/* protocol major version */
	int minor;	/* protocol minor version */
	char *database;	/* database name in startup_packet (malloced area) */
	char *user;	/* user name in startup_packet (malloced area) */
} StartupPacket;

typedef struct CancelPacket
{
	int			protoVersion;		/* Protocol version */
	int			pid;	/* bcckend process id */
	int			key;	/* cancel key */
} CancelPacket;

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
    char *pid_file_name;		/* pid file name */
    char *backend_socket_dir;	/* Unix domain socket directory for the PostgreSQL server */
	int replication_mode;		/* replication mode */

	int log_connections;		/* 0:false, 1:true - logs incoming connections */
	int log_hostname;		/* 0:false, 1:true - resolve hostname */
	int enable_pool_hba;		/* 0:false, 1:true - enables pool_hba.conf file authentication */

	int load_balance_mode;		/* load balance mode */

	int replication_stop_on_mismatch;		/* if there's a data mismatch between master and secondary
											 * start degenration to stop replication mode
											 */
	int replicate_select; /* if non 0, replicate SELECT statement when load balancing is disabled. */
	char **reset_query_list;		/* comma separated list of quries to be issued at the end of session */

	int print_timestamp;		/* if non 0, print time stamp to each log line */
	int master_slave_mode;		/* if non 0, operate in master/slave mode */
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

	BackendDesc *backend_desc;	/* PostgreSQL Server description. Placed on shared memory */

	LOAD_BALANCE_STATUS	load_balance_status[MAX_NUM_BACKENDS];	/* to remember which DB node is selected for load balancing */

	/* followings do not exist in the configuration file */
    int	current_slot;	/* current backend slot # */
	int replication_enabled;		/* replication mode enabled */
	int master_slave_enabled;		/* master/slave mode enabled */
	int num_reset_queries;		/* number of queries in reset_query_list */

	/* ssl configuration */
	int ssl;	/* if non 0, activate ssl support (frontend+backend) */
	char *ssl_cert;	/* path to ssl certificate (frontend only) */
	char *ssl_key;	/* path to ssl key (frontend only) */
} POOL_CONFIG;

#define MAX_PASSWORD_SIZE		1024

typedef struct {
	int num;	/* number of entries */
	char **names;		/* parameter names */
	char **values;		/* values */
} ParamStatus;

/*
 * stream connection structure
 */
typedef struct {
	int fd;		/* fd for connection */

	char *wbuf;	/* write buffer for the connection */
	int wbufsz;	/* write buffer size */
	int wbufpo;	/* buffer offset */

#ifdef USE_SSL
	SSL_CTX *ssl_ctx; /* SSL connection context */
	SSL *ssl;	/* SSL connection */
#endif
	int ssl_active; /* SSL is failed if < 0, off if 0, on if > 0 */

	char *hp;	/* pending data buffer head address */
	int po;		/* pending data offset */
	int bufsz;	/* pending data buffer size */
	int len;	/* pending data length */

	char *sbuf;	/* buffer for pool_read_string */
	int sbufsz;	/* its size in bytes */

	char *buf2;	/* buffer for pool_read2 */
	int bufsz2;	/* its size in bytes */

	int isbackend;		/* this connection is for backend if non 0 */
	int db_node_id;		/* DB node id for this connection */

	char tstate;		/* transaction state (V3 only) */

	/*
	 * following are used to remember when re-use the authenticated connection
	 */
	int auth_kind;		/* 3: clear text password, 4: crypt password, 5: md5 password */
	int pwd_size;		/* password (sent back from frontend) size in host order */
	char password[MAX_PASSWORD_SIZE];		/* password (sent back from frontend) */
	char salt[4];		/* password salt */

	/*
	 * following are used to remember current session paramter status.
	 * re-used connection will need them (V3 only)
	 */
	ParamStatus params;

	int no_forward;		/* if non 0, do not write to frontend */

	char kind;	/* kind cache */

	/*
	 * frontend info needed for hba
	 */
	int protoVersion;
	SockAddr raddr;
	UserAuth auth_method;
	char *auth_arg;
	char *database;
	char *username;
} POOL_CONNECTION;

/*
 * connection pool structure
 */
typedef struct {
	StartupPacket *sp;	/* startup packet info */
    int pid;	/* backend pid */
    int key;	/* cancel key */
    POOL_CONNECTION	*con;
	time_t closetime;	/* absolute time in second when the connection closed
						 * if 0, that means the connection is under use.
						 */
} POOL_CONNECTION_POOL_SLOT;

typedef struct {
	ConnectionInfo *info;		/* connection info on shmem */
    POOL_CONNECTION_POOL_SLOT	*slots[MAX_NUM_BACKENDS];
} POOL_CONNECTION_POOL;

typedef struct {
	SystemDBInfo *info;
	PGconn *pgconn;
	/* persistent connection to the system DB */
	POOL_CONNECTION_POOL_SLOT *connection;
	BACKEND_STATUS *system_db_status;
} POOL_SYSTEMDB_CONNECTION_POOL;

/* 
 * for pool_clear_cache() in pool_query_cache.c 
 *
 * used to specify the time which cached data created before it to be deleted.
 */
typedef enum {
	second, seconds,
	minute, minutes,
	hour, hours,
	day, days,
	week, weeks,
	month, months,
	year, years,
	decade, decades,
	century, centuries,
	millennium, millenniums
} UNIT;

typedef struct {
	int quantity;
	UNIT unit;
} Interval;

/*
 * Relation cache structure
 */
#define MAX_ITEM_LENGTH	1024

/* Relation lookup cache structure */

typedef void *(*func_ptr) ();

typedef struct {
	char dbname[MAX_ITEM_LENGTH];	/* database name */
	char relname[MAX_ITEM_LENGTH];	/* table name */
	void *data;	/* user data */
	int refcnt;		/* reference count */
	int session_id;		/* LocalSessionId */
} PoolRelCache;

typedef struct {
	int num;		/* number of cache items */
	char sql[MAX_ITEM_LENGTH];	/* Query to relation */
	/*
	 * User defined function to be called at data register.
	 * Argument is POOL_SELECT_RESULT *.
	 * This function must return a pointer to be
	 * saved in cache->data.
	 */
	func_ptr	register_func;
	/*
	 * User defined function to be called at data unregister.
	 * Argument cache->data.
	 */
	func_ptr	unregister_func;
	bool cache_is_session_local;		/* True if cache life time is session local */
	PoolRelCache *cache;	/* cache data */
} POOL_RELCACHE;


#ifdef NOT_USED
#define NUM_BACKENDS (in_load_balance? (selected_slot+1) : \
					  (((!REPLICATION && !PARALLEL_MODE)||master_slave_dml)? Req_info->master_node_id+1: \
					   pool_config->backend_desc->num_backends))
#endif
/* NUM_BACKENDS now always returns actual number of backends if not in_load_balance */
#define NUM_BACKENDS (in_load_balance ? (selected_slot+1) : pool_config->backend_desc->num_backends)
#define BACKEND_INFO(backend_id) (pool_config->backend_desc->backend_info[(backend_id)])
#define LOAD_BALANCE_STATUS(backend_id) (pool_config->load_balance_status[(backend_id)])
/* if RAW_MODE, VALID_BACKEND returns the selected node only */
#define VALID_BACKEND(backend_id) \
	(RAW_MODE ? (backend_id) == MASTER_NODE_ID : \
	(in_load_balance ? LOAD_BALANCE_STATUS(backend_id) == LOAD_SELECTED : \
    ((BACKEND_INFO(backend_id).backend_status == CON_UP) || \
	 (BACKEND_INFO(backend_id).backend_status == CON_CONNECT_WAIT))))
#define CONNECTION_SLOT(p, slot) ((p)->slots[(slot)])
#define CONNECTION(p, slot) (CONNECTION_SLOT(p, slot)->con)
#define MASTER_CONNECTION(p) ((p)->slots[MASTER_NODE_ID])
#define MASTER_NODE_ID (in_load_balance? selected_slot : Req_info->master_node_id)
#define IS_MASTER_NODE_ID(node_id) (MASTER_NODE_ID == (node_id))
//#define SECONDARY_CONNECTION(p) ((p)->slots[1])
#define REPLICATION (pool_config->replication_enabled)
#define MASTER_SLAVE (pool_config->master_slave_enabled)
#define DUAL_MODE (REPLICATION || MASTER_SLAVE)
#define PARALLEL_MODE (pool_config->parallel_mode)
#define RAW_MODE (!REPLICATION && !PARALLEL_MODE && !MASTER_SLAVE)
#define MASTER(p) MASTER_CONNECTION(p)->con
//#define SECONDARY(p) SECONDARY_CONNECTION(p)->con
#define MAJOR(p) MASTER_CONNECTION(p)->sp->major
#define TSTATE(p) MASTER(p)->tstate
#define SYSDB_INFO (system_db_info->info)
#define SYSDB_CONNECTION (system_db_info->connection)
#define SYSDB_STATUS (*system_db_info->system_db_status)

#define Max(x, y)		((x) > (y) ? (x) : (y))
#define Min(x, y)		((x) < (y) ? (x) : (y))

#define LOCK_COMMENT "/*INSERT LOCK*/"
#define LOCK_COMMENT_SZ (sizeof(LOCK_COMMENT)-1)
#define NO_LOCK_COMMENT "/*NO INSERT LOCK*/"
#define NO_LOCK_COMMENT_SZ (sizeof(NO_LOCK_COMMENT)-1)
#define NO_LOAD_BALANCE "/*NO LOAD BALANCE*/"
#define NO_LOAD_BALANCE_COMMENT_SZ (sizeof(NO_LOAD_BALANCE)-1)

#define MAX_NUM_SEMAPHORES		3
#define CONN_COUNTER_SEM 0
#define REQUEST_INFO_SEM 1

#define MY_PROCESS_INFO (pids[my_proc_id])

/*
 * number specified when semaphore is locked/unlocked
 */
typedef enum SemNum
{
	SEMNUM_CONFIG,
	SEMNUM_NODES,
	SEMNUM_PROCESSES
} SemNum;

/*
 * up/down request info area in shared memory
 */
typedef enum {
	NODE_UP_REQUEST = 0,
	NODE_DOWN_REQUEST,
	NODE_RECOVERY_REQUEST,
	CLOSE_IDLE_REQUEST
} POOL_REQUEST_KIND;

typedef struct {
	POOL_REQUEST_KIND	kind;	/* request kind */
	int node_id[MAX_NUM_BACKENDS];		/* request node id */
	int master_node_id;	/* the youngest node id which is not in down status */
	int conn_counter;
} POOL_REQUEST_INFO;

/* description of row. corresponding to RowDescription message */
typedef struct {
	char *attrname;		/* attribute name */
	int oid;	/* 0 or non 0 if it's a table oblect */
	int attrnumber;		/* attribute number starting with 1. 0 if it's not a table */
	int typeoid;		/* data type oid */
	int	size;	/* data length minus means variable data type */
	int mod;	/* data type modifier */
} AttrInfo;

typedef struct {
	int num_attrs;		/* number of attributes */
	AttrInfo *attrinfo;
} RowDesc;

typedef struct {
	RowDesc *rowdesc;	/* attribute info */
	int numrows;		/* number of rows */
	int *nullflags;	/* if NULL, -1 or length of the string excluding termination null */
	char **data;		/* actual row character data terminated with null */
} POOL_SELECT_RESULT;

/*
 * global variables
 */
extern pid_t mypid; /* parent pid */
extern POOL_CONFIG *pool_config;	/* configuration values */
extern POOL_CONNECTION_POOL *pool_connection_pool;	/* connection pool */
extern volatile sig_atomic_t backend_timer_expired; /* flag for connection closed timer is expired */
extern long int weight_master;	/* normalized weight of master (0-RAND_MAX range) */
extern int my_proc_id;  /* process table id (!= UNIX's PID) */
extern POOL_SYSTEMDB_CONNECTION_POOL *system_db_info; /* systemdb */
extern ProcessInfo *pids; /* shmem process information table */
extern ConnectionInfo *con_info; /* shmem connection info table */
extern int in_load_balance;		/* non 0 if in load balance mode */
extern int selected_slot;		/* selected DB node for load balance */
extern int master_slave_dml;	/* non 0 if master/slave mode is specified in config file */
extern POOL_REQUEST_INFO *Req_info;
extern volatile sig_atomic_t *InRecovery;
extern char remote_ps_data[];		/* used for set_ps_display */
extern volatile sig_atomic_t got_sighup;

#define QUERY_STRING_BUFFER_LEN 1024
extern char query_string_buffer[];		/* last query string sent to simpleQuery() */

extern int LocalSessionId;	/* Local session id. incremented when new frontend connected */

/*
 * public functions
 */
extern char *get_config_file_name(void);
extern char *get_hba_file_name(void);
#ifdef __GNUC__
extern void pool_error(const char *fmt,...)
   	__attribute__((format (printf, 1, 2)));
extern void pool_debug(const char *fmt,...)
   	__attribute__((format (printf, 1, 2)));
extern void pool_log(const char *fmt,...)
   	__attribute__((format (printf, 1, 2)));
#else
extern void pool_error(const char *fmt,...);
extern void pool_debug(const char *fmt,...);
extern void pool_log(const char *fmt,...);
#endif
extern int pool_init_config(void);
extern int pool_get_config(char *confpath, POOL_CONFIG_CONTEXT context);
extern void do_child(int unix_fd, int inet_fd);
extern void pcp_do_child(int unix_fd, int inet_fd, char *pcp_conf_file);
extern int select_load_balancing_node(void);
extern int pool_init_cp(void);
extern POOL_STATUS pool_process_query(POOL_CONNECTION *frontend,
									  POOL_CONNECTION_POOL *backend,
									  int reset_request);

extern POOL_CONNECTION *pool_open(int fd);
extern void pool_close(POOL_CONNECTION *cp);
extern int pool_read(POOL_CONNECTION *cp, void *buf, int len);
extern char *pool_read2(POOL_CONNECTION *cp, int len);
extern int pool_write(POOL_CONNECTION *cp, void *buf, int len);
extern int pool_flush(POOL_CONNECTION *cp);
extern int pool_flush_it(POOL_CONNECTION *cp);
extern int pool_write_and_flush(POOL_CONNECTION *cp, void *buf, int len);
extern char *pool_read_string(POOL_CONNECTION *cp, int *len, int line);
extern int pool_unread(POOL_CONNECTION *cp, void *data, int len);

extern int pool_do_auth(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
extern int pool_do_reauth(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *cp);

extern int pool_init_cp(void);
extern POOL_CONNECTION_POOL *pool_create_cp(void);
extern POOL_CONNECTION_POOL *pool_get_cp(char *user, char *database, int protoMajor, int check_socket);
extern void pool_discard_cp(char *user, char *database, int protoMajor);
extern void pool_backend_timer(void);

/* SSL functionality */
extern void pool_ssl_negotiate_serverclient(POOL_CONNECTION *cp);
extern void pool_ssl_negotiate_clientserver(POOL_CONNECTION *cp);
extern void pool_ssl_close(POOL_CONNECTION *cp);
extern int pool_ssl_read(POOL_CONNECTION *cp, void *buf, int size);
extern int pool_ssl_write(POOL_CONNECTION *cp, const void *buf, int size);

extern POOL_STATUS ErrorResponse(POOL_CONNECTION *frontend, 
								  POOL_CONNECTION_POOL *backend);

extern POOL_STATUS NoticeResponse(POOL_CONNECTION *frontend, 
								  POOL_CONNECTION_POOL *backend);

extern void notice_backend_error(int node_id);
extern void degenerate_backend_set(int *node_id_set, int count);
extern void send_failback_request(int node_id);

extern void pool_connection_pool_timer(POOL_CONNECTION_POOL *backend);
extern RETSIGTYPE pool_backend_timer_handler(int sig);

extern int connect_inet_domain_socket(int secondary_backend);
extern int connect_unix_domain_socket(int secondary_backend);
extern int connect_inet_domain_socket_by_port(char *host, int port);
extern int connect_unix_domain_socket_by_port(int port, char *socket_dir);

extern void pool_set_timeout(int timeoutval);
extern int pool_check_fd(POOL_CONNECTION *cp);

extern void pool_send_frontend_exits(POOL_CONNECTION_POOL *backend);

extern int pool_read_message_length(POOL_CONNECTION_POOL *cp);
extern int *pool_read_message_length2(POOL_CONNECTION_POOL *cp);
extern signed char pool_read_kind(POOL_CONNECTION_POOL *cp);
extern int pool_read_int(POOL_CONNECTION_POOL *cp);

extern POOL_STATUS SimpleForwardToFrontend(char kind, POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
extern POOL_STATUS SimpleForwardToBackend(char kind, POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
extern POOL_STATUS ParameterStatus(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);

extern int pool_init_params(ParamStatus *params);
extern void pool_discard_params(ParamStatus *params);
extern char *pool_find_name(ParamStatus *params, char *name, int *pos);
extern int pool_get_param(ParamStatus *params, int index, char **name, char **value);
extern int pool_add_param(ParamStatus *params, char *name, char *value);
extern void pool_param_debug_print(ParamStatus *params);

extern void pool_send_error_message(POOL_CONNECTION *frontend, int protoMajor,
							 char *code,
							 char *message,
							 char *detail,
							 char *hint,
							 char *file,
							 int line);
extern void pool_send_fatal_message(POOL_CONNECTION *frontend, int protoMajor,
							 char *code,
							 char *message,
							 char *detail,
							 char *hint,
							 char *file,
							 int line);
extern void pool_send_severity_message(POOL_CONNECTION *frontend, int protoMajor,
							 char *code,
							 char *message,
							 char *detail,
							 char *hint,
							 char *file,
							 char *severity,
							 int line);
extern void pool_send_readyforquery(POOL_CONNECTION *frontend);
extern int send_startup_packet(POOL_CONNECTION_POOL_SLOT *cp);
extern void pool_free_startup_packet(StartupPacket *sp);
extern void child_exit(int code);

extern int health_check(void);
extern int system_db_health_check(void);

extern void init_prepared_list(void);

extern void *pool_shared_memory_create(size_t size);
extern void pool_shmem_exit(int code);

extern int pool_semaphore_create(int numSems);
extern void pool_semaphore_lock(int semNum);
extern void pool_semaphore_unlock(int semNum);

extern BackendInfo *pool_get_node_info(int node_number);
extern int pool_get_node_count(void);
extern int *pool_get_process_list(int *array_size);
extern ProcessInfo *pool_get_process_info(pid_t pid);
extern SystemDBInfo *pool_get_system_db_info(void);
extern POOL_STATUS OneNode_do_command(POOL_CONNECTION *frontend, POOL_CONNECTION *backend, char *query, char *database);

extern POOL_CONNECTION_POOL_SLOT *make_persistent_db_connection(
	char *hostname, int port, char *dbname, char *user, char *password);

/* define pool_system.c */
extern POOL_CONNECTION_POOL_SLOT *pool_system_db_connection(void);
extern DistDefInfo *pool_get_dist_def_info (char * dbname, char * schema_name, char * table_name);
extern RepliDefInfo *pool_get_repli_def_info (char * dbname, char * schema_name, char * table_name);
extern int pool_get_id (DistDefInfo *info, const char * value);
extern int system_db_connect (void);
extern int pool_memset_system_db_info (SystemDBInfo *info);
extern void pool_close_libpq_connection(void);

/* pool_query_cache.c */
extern POOL_STATUS pool_query_cache_lookup(POOL_CONNECTION *frontend, char *query, char *database, char tstate);
extern int pool_query_cache_register(char kind, POOL_CONNECTION *frontend, char *database, char *data, int data_len, char *query);
extern int pool_query_cache_table_exists(void);
extern int pool_clear_cache_by_time(Interval *interval, int size);

/* pool_hba.c */
extern void load_hba(char *hbapath);
extern void ClientAuthentication(POOL_CONNECTION *frontend);

/* pool_ip.c */
extern void pool_getnameinfo_all(SockAddr *saddr, char *remote_host, char *remote_port);

/* strlcpy.c */
extern size_t strlcpy(char *dst, const char *src, size_t siz);

/* ps_status.c */
extern bool update_process_title;
extern char **save_ps_display_args(int argc, char **argv);
extern void init_ps_display(const char *username, const char *dbname,
							const char *host_info, const char *initial_str);
extern void set_ps_display(const char *activity, bool force);
extern const char *get_ps_display(int *displen);

/* recovery.c */
extern int start_recovery(int recovery_node);
extern void finish_recovery(void);

/* child.c */
extern void pool_set_nonblock(int fd);
extern void pool_unset_nonblock(int fd);
extern void cancel_request(CancelPacket *sp);
extern void check_stop_request(void);

/* pool_process_query.c */
extern void reset_variables(void);
extern void per_node_statement_log(POOL_CONNECTION_POOL *backend, int node_id, char *query);
extern POOL_STATUS pool_extract_error_message(POOL_CONNECTION *backend, int major, bool unread, char **message);
extern POOL_STATUS do_command(POOL_CONNECTION *frontend, POOL_CONNECTION *backend,
					   char *query, int protoMajor, int pid, int key, int no_ready_for_query);
extern POOL_STATUS do_query(POOL_CONNECTION *backend, char *query, POOL_SELECT_RESULT **result, int major);
extern void free_select_result(POOL_SELECT_RESULT *result);

/* pool_relcache.c */
extern POOL_RELCACHE *pool_create_relcache(int cachesize, char *sql,
									func_ptr register_func, func_ptr unregister_func,
									bool issessionlocal);
extern void pool_discard_relcache(POOL_RELCACHE *relcache);
extern void *pool_search_relcache(POOL_RELCACHE *relcache, POOL_CONNECTION_POOL *backend, char *table);
extern void *int_register_func(POOL_SELECT_RESULT *res);
extern void *int_unregister_func(void *data);

/* pool_lobj.c */
extern char *pool_rewrite_lo_creat(char kind, char *packet, int packet_len, POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, int* len);

#endif /* POOL_H */
