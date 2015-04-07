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
 * pool.h.: master definition header file
 *
 */

#ifndef POOL_H
#define POOL_H

#include "config.h"
#include "pool_type.h"
#include "utils/pool_signal.h"
#include "parser/nodes.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <limits.h>

#ifdef USE_SSL
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include <syslog.h>

/* undef this if you have problems with non blocking accept() */
#define NONE_BLOCK

#define POOLMAXPATHLEN 8192

/*
 * Brought from PostgreSQL's pg_config_manual.h.
 *
 * Maximum length for identifiers (e.g. table names, column names,
 * function names).  Names actually are limited to one less byte than this,
 * because the length must include a trailing zero byte.
 *
 * Please note that in version 2 protocol, maximum user name length is
 * SM_USER, which is 32.
 */
#define NAMEDATALEN 64

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

/* default string used to identify pgpool on syslog output */
#define DEFAULT_SYSLOG_IDENT "pgpool"

/* function return codes */
#define GENERAL_ERROR		(-1)
#define RETRY				(-2)
#define OPERATION_TIMEOUT	(-3)


typedef enum {
	POOL_CONTINUE = 0,
	POOL_IDLE,
	POOL_END,
	POOL_ERROR,
	POOL_FATAL,
	POOL_DEADLOCK
} POOL_STATUS;

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
	char *application_name;		/* not malloced. pointing to in startup_packet */
    bool system_db;     /* true if database name is one of default that comes with server e.g postgres or template.. */
} StartupPacket;

typedef struct CancelPacket
{
	int			protoVersion;		/* Protocol version */
	int			pid;	/* backend process id */
	int			key;	/* cancel key */
} CancelPacket;

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

	char *buf3;	/* buffer for pool_push/pop */
	int bufsz3;	/* its size in bytes */

	int isbackend;		/* this connection is for backend if non 0 */
	int db_node_id;		/* DB node id for this connection */

	char tstate;		/* Transaction state (V3 only) 'I' if idle
						 * (not in a transaction block); 'T' if in a
						 * transaction block; or 'E' if in a failed
						 * transaction block
						 */

	/* True if an internal transaction has already started */
	bool is_internal_transaction_started;

	/*
	 * following are used to remember when re-use the authenticated connection
	 */
	int auth_kind;		/* 3: clear text password, 4: crypt password, 5: md5 password */
	int pwd_size;		/* password (sent back from frontend) size in host order */
	char password[MAX_PASSWORD_SIZE];		/* password (sent back from frontend) */
	char salt[4];		/* password salt */

	/*
	 * following are used to remember current session parameter status.
	 * re-used connection will need them (V3 only)
	 */
	ParamStatus params;

	int no_forward;		/* if non 0, do not write to frontend */

	char kind;	/* kind cache */

	/* true if remote end closed the connection */
	bool EOF_on_socket;
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


/* NUM_BACKENDS now always returns actual number of backends */
#define NUM_BACKENDS (pool_config->backend_desc->num_backends)
#define BACKEND_INFO(backend_id) (pool_config->backend_desc->backend_info[(backend_id)])
#define LOAD_BALANCE_STATUS(backend_id) (pool_config->load_balance_status[(backend_id)])

/*
 * This macro returns true if:
 *   current query is in progress and the DB node is healthy OR
 *   no query is in progress and the DB node is healthy
 */
extern bool pool_is_node_to_be_sent_in_current_query(int node_id);
extern int pool_virtual_master_db_node_id(void);
extern BACKEND_STATUS* my_backend_status[];
extern int my_master_node_id;

#define VALID_BACKEND(backend_id) \
	((RAW_MODE && (backend_id) == REAL_MASTER_NODE_ID) ||		\
	(pool_is_node_to_be_sent_in_current_query((backend_id)) &&	\
	 ((*(my_backend_status[(backend_id)]) == CON_UP) ||			\
	  (*(my_backend_status[(backend_id)]) == CON_CONNECT_WAIT))))

/*
 * For raw mode failover control
 */
#define VALID_BACKEND_RAW(backend_id) \
	((*(my_backend_status[(backend_id)]) == CON_UP) ||			\
	 (*(my_backend_status[(backend_id)]) == CON_CONNECT_WAIT))

#define CONNECTION_SLOT(p, slot) ((p)->slots[(slot)])
#define CONNECTION(p, slot) (CONNECTION_SLOT(p, slot)->con)

/*
 * The first DB node id appears in pgpool.conf or the first "live" DB
 * node otherwise.
 */
#define REAL_MASTER_NODE_ID (Req_info->master_node_id)

/*
 * The primary node id in streaming replication mode. If not in the
 * mode or there's no primary node, this macro returns
 * REAL_MASTER_NODE_ID.
 */
#define PRIMARY_NODE_ID (Req_info->primary_node_id >=0?\
						 Req_info->primary_node_id:REAL_MASTER_NODE_ID)

/*
 * Real primary node id. If not in the mode or there's no primary
 * node, this macro returns -1.
 */
#define REAL_PRIMARY_NODE_ID (Req_info->primary_node_id)

/*
 * "Virtual" master node id. It's same as REAL_MASTER_NODE_ID if not
 * in load balance mode. If in load balance, it's the first load
 * balance node.
 */
#define MASTER_NODE_ID (pool_virtual_master_db_node_id())
#define IS_MASTER_NODE_ID(node_id) (MASTER_NODE_ID == (node_id))
#define MASTER_CONNECTION(p) ((p)->slots[MASTER_NODE_ID])
#define MASTER(p) MASTER_CONNECTION(p)->con

#define REPLICATION (pool_config->replication_mode)
#define MASTER_SLAVE (pool_config->master_slave_mode)
#define STREAM (MASTER_SLAVE && !strcmp("stream", pool_config->master_slave_sub_mode))
#define DUAL_MODE (REPLICATION || MASTER_SLAVE)
#define PARALLEL_MODE (pool_config->parallel_mode)
#define RAW_MODE (!REPLICATION && !PARALLEL_MODE && !MASTER_SLAVE)
#define MAJOR(p) MASTER_CONNECTION(p)->sp->major
#define TSTATE(p, i) (CONNECTION(p, i)->tstate)
#define INTERNAL_TRANSACTION_STARTED(p, i) (CONNECTION(p, i)->is_internal_transaction_started)
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

#define MAX_NUM_SEMAPHORES		4
#define CONN_COUNTER_SEM 0
#define REQUEST_INFO_SEM 1
#define SHM_CACHE_SEM	2
#define QUERY_CACHE_STATS_SEM	3
#define MAX_REQUEST_QUEUE_SIZE	10

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
	CLOSE_IDLE_REQUEST,
	PROMOTE_NODE_REQUEST
} POOL_REQUEST_KIND;

typedef struct {
	POOL_REQUEST_KIND	kind;		/* request kind */
	int node_id[MAX_NUM_BACKENDS];	/* request node id */
	int count;						/* request node ids count */
}POOL_REQUEST_NODE;

typedef struct {
	POOL_REQUEST_NODE request[MAX_REQUEST_QUEUE_SIZE];
	int request_queue_head;
	int request_queue_tail;
	int master_node_id;		/* the youngest node id which is not in down status */
	int primary_node_id;	/* the primary node id in streaming replication mode */
	int conn_counter;
	bool switching;	/* it true, failover or failback is in progress */
} POOL_REQUEST_INFO;

/* description of row. corresponding to RowDescription message */
typedef struct {
	char *attrname;		/* attribute name */
	int oid;	/* 0 or non 0 if it's a table object */
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
 * recovery mode
 */
typedef enum {
	RECOVERY_INIT = 0,
	RECOVERY_ONLINE,
	RECOVERY_DETACH,
	RECOVERY_PROMOTE
} POOL_RECOVERY_MODE;

/*
 * global variables
 */
extern pid_t mypid; /* parent pid */
extern bool run_as_pcp_child;

typedef enum
{
	PT_MAIN,
	PT_CHILD,
	PT_WORKER,
	PT_HB_SENDER,
	PT_HB_RECEIVER,
	PT_WATCHDOG,
	PT_LIFECHECK,
	PT_FOLLOWCHILD,
	PT_WATCHDOG_UTILITY,
	PT_PCP
} ProcessType;

extern ProcessType processType;

typedef enum
{
	INITIALIZING,
	PERFORMING_HEALTH_CHECK,
	PERFORMING_SYSDB_CHECK,
	SLEEPING,
	WAITIG_FOR_CONNECTION,
	BACKEND_CONNECTING,
	PROCESSING,
    EXITING
} ProcessState;

extern ProcessState processState;

extern POOL_CONNECTION_POOL *pool_connection_pool;	/* connection pool */
extern volatile sig_atomic_t backend_timer_expired; /* flag for connection closed timer is expired */
extern volatile sig_atomic_t health_check_timer_expired;		/* non 0 if health check timer expired */
extern long int weight_master;	/* normalized weight of master (0-RAND_MAX range) */
extern int my_proc_id;  /* process table id (!= UNIX's PID) */
extern POOL_SYSTEMDB_CONNECTION_POOL *system_db_info; /* systemdb */
extern ProcessInfo *process_info; /* shmem process information table */
extern ConnectionInfo *con_info; /* shmem connection info table */
extern POOL_REQUEST_INFO *Req_info;
extern volatile sig_atomic_t *InRecovery;
extern char remote_ps_data[];		/* used for set_ps_display */
extern volatile sig_atomic_t got_sighup;
extern volatile sig_atomic_t exit_request;

#define QUERY_STRING_BUFFER_LEN 1024
extern char query_string_buffer[];		/* last query string sent to simpleQuery() */

extern BACKEND_STATUS private_backend_status[MAX_NUM_BACKENDS];

extern char remote_host[];	/* client host */
extern char remote_port[];	/* client port */

/*
 * public functions
 */
extern bool register_node_operation_request(POOL_REQUEST_KIND kind, int* node_id_set, int count);
extern char *get_config_file_name(void);
extern char *get_hba_file_name(void);
extern void do_child(int *fds);
extern void pcp_do_child(int unix_fd, int inet_fd, char *pcp_conf_file);
extern int select_load_balancing_node(void);
extern int pool_init_cp(void);
extern POOL_STATUS pool_process_query(POOL_CONNECTION *frontend,
									  POOL_CONNECTION_POOL *backend,
									  int reset_request);

extern int pool_do_auth(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
extern int pool_do_reauth(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *cp);

/* SSL functionality */
extern void pool_ssl_negotiate_serverclient(POOL_CONNECTION *cp);
extern void pool_ssl_negotiate_clientserver(POOL_CONNECTION *cp);
extern void pool_ssl_close(POOL_CONNECTION *cp);
extern int pool_ssl_read(POOL_CONNECTION *cp, void *buf, int size);
extern int pool_ssl_write(POOL_CONNECTION *cp, const void *buf, int size);
extern bool pool_ssl_pending(POOL_CONNECTION *cp);

extern POOL_STATUS ErrorResponse(POOL_CONNECTION *frontend, 
								  POOL_CONNECTION_POOL *backend);

extern void NoticeResponse(POOL_CONNECTION *frontend,
								  POOL_CONNECTION_POOL *backend);

extern void notice_backend_error(int node_id);
extern void degenerate_backend_set(int *node_id_set, int count);
extern bool degenerate_backend_set_ex(int *node_id_set, int count, bool error, bool test_only);
extern void promote_backend(int node_id);
extern void send_failback_request(int node_id);


extern void pool_set_timeout(int timeoutval);
extern int pool_check_fd(POOL_CONNECTION *cp);

extern void pool_send_frontend_exits(POOL_CONNECTION_POOL *backend);

extern int pool_read_message_length(POOL_CONNECTION_POOL *cp);
extern int *pool_read_message_length2(POOL_CONNECTION_POOL *cp);
extern signed char pool_read_kind(POOL_CONNECTION_POOL *cp);
extern int pool_read_int(POOL_CONNECTION_POOL *cp);

extern POOL_STATUS SimpleForwardToFrontend(char kind, POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
extern POOL_STATUS SimpleForwardToBackend(char kind, POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, int len, char *contents);
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
extern void send_startup_packet(POOL_CONNECTION_POOL_SLOT *cp);
extern void pool_free_startup_packet(StartupPacket *sp);
extern void child_exit(int code);

extern void init_prepared_list(void);
extern void proc_exit(int);

extern void *pool_shared_memory_create(size_t size);
extern void pool_shmem_exit(int code);

extern void pool_semaphore_create(int numSems);
extern void pool_semaphore_lock(int semNum);
extern void pool_semaphore_unlock(int semNum);

extern BackendInfo *pool_get_node_info(int node_number);
extern int pool_get_node_count(void);
extern int *pool_get_process_list(int *array_size);
extern ProcessInfo *pool_get_process_info(pid_t pid);
extern POOL_STATUS OneNode_do_command(POOL_CONNECTION *frontend, POOL_CONNECTION *backend, char *query, char *database);

/* child.c */
extern POOL_CONNECTION_POOL_SLOT *make_persistent_db_connection(
	char *hostname, int port, char *dbname, char *user, char *password, bool retry);
extern POOL_CONNECTION_POOL_SLOT *make_persistent_db_connection_noerror(
    char *hostname, int port, char *dbname, char *user, char *password, bool retry);
extern void discard_persistent_db_connection(POOL_CONNECTION_POOL_SLOT *cp);

/* define pool_system.c */
extern POOL_CONNECTION_POOL_SLOT *pool_system_db_connection(void);
extern DistDefInfo *pool_get_dist_def_info (char * dbname, char * schema_name, char * table_name);
extern RepliDefInfo *pool_get_repli_def_info (char * dbname, char * schema_name, char * table_name);
extern int pool_get_id (DistDefInfo *info, const char * value);
extern int system_db_connect (void);
extern int pool_memset_system_db_info (SystemDBInfo *info);
extern void pool_close_libpq_connection(void);
extern int system_db_health_check(void);
extern SystemDBInfo *pool_get_system_db_info(void);


/* pool_hba.c */
extern int load_hba(char *hbapath);
extern void ClientAuthentication(POOL_CONNECTION *frontend);

/* pool_ip.c */
extern void pool_getnameinfo_all(SockAddr *saddr, char *remote_host, char *remote_port);

/* strlcpy.c */
#ifndef HAVE_STRLCPY
extern size_t strlcpy(char *dst, const char *src, size_t siz);
#endif

/* ps_status.c */
extern bool update_process_title;
extern char **save_ps_display_args(int argc, char **argv);
extern void init_ps_display(const char *username, const char *dbname,
							const char *host_info, const char *initial_str);
extern void set_ps_display(const char *activity, bool force);
extern const char *get_ps_display(int *displen);
extern void pool_ps_idle_display(POOL_CONNECTION_POOL *backend);

/* recovery.c */
extern void start_recovery(int recovery_node);
extern void finish_recovery(void);
extern int wait_connection_closed(void);

/* child.c */
extern void cancel_request(CancelPacket *sp);
extern void check_stop_request(void);
extern void pool_initialize_private_backend_status(void);
extern bool is_session_connected(void);
extern int send_to_pg_frontend(char* data, int len, bool flush);
extern int pg_frontend_exists(void);
extern int set_pg_frontend_blocking(bool blocking);
extern int get_frontend_protocol_version(void);

/* pool_process_query.c */
extern void reset_variables(void);
extern void reset_connection(void);
extern void per_node_statement_log(POOL_CONNECTION_POOL *backend, int node_id, char *query);
extern void per_node_error_log(POOL_CONNECTION_POOL *backend, int node_id, char *query, char *prefix, bool unread);
extern int pool_extract_error_message(bool read_kind, POOL_CONNECTION *backend, int major, bool unread, char **message);
extern POOL_STATUS do_command(POOL_CONNECTION *frontend, POOL_CONNECTION *backend,
					   char *query, int protoMajor, int pid, int key, int no_ready_for_query);
extern void do_query(POOL_CONNECTION *backend, char *query, POOL_SELECT_RESULT **result, int major);
extern void free_select_result(POOL_SELECT_RESULT *result);
extern int compare(const void *p1, const void *p2);
extern void do_error_execute_command(POOL_CONNECTION_POOL *backend, int node_id, int major);
extern POOL_STATUS pool_discard_packet_contents(POOL_CONNECTION_POOL *cp);
extern void pool_dump_valid_backend(int backend_id);

/* pool_auth.c */
extern void pool_random_salt(char *md5Salt);

/* main.c */
extern void pool_sleep(unsigned int second);

/* pool_worker_child.c */
extern void do_worker_child(void);

/* md5.c */
extern bool pg_md5_encrypt(const char *passwd, const char *salt, size_t salt_len, char *buf);

/* pool_connection_pool.c */
extern int pool_init_cp(void);
extern POOL_CONNECTION_POOL *pool_create_cp(void);
extern POOL_CONNECTION_POOL *pool_get_cp(char *user, char *database, int protoMajor, int check_socket);
extern void pool_discard_cp(char *user, char *database, int protoMajor);
extern void pool_backend_timer(void);
extern void pool_connection_pool_timer(POOL_CONNECTION_POOL *backend);
extern RETSIGTYPE pool_backend_timer_handler(int sig);
extern int connect_inet_domain_socket(int slot, bool retry);
extern int connect_unix_domain_socket(int slot, bool retry);
extern int connect_inet_domain_socket_by_port(char *host, int port, bool retry);
extern int connect_unix_domain_socket_by_port(int port, char *socket_dir, bool retry);
extern int pool_pool_index(void);

extern int PgpoolMain(bool discard_status, bool clear_memcache_oidmaps);
#endif /* POOL_H */
