/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2012	PgPool Global Development Group
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
 */
#include "pool.h"
#include "pool_config.h"
#include "pool_process_context.h"

#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include <sys/stat.h>
#include <fcntl.h>

#include <sys/wait.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include "getopt_long.h"
#endif

#include <libgen.h>

#include "version.h"
#include "parser/pool_memory.h"
#include "parser/pool_string.h"
#include "pool_passwd.h"

/*
 * Process pending signal actions.
 */
#define CHECK_REQUEST \
	do { \
		if (wakeup_request) \
		{ \
			wakeup_children(); \
			wakeup_request = 0; \
		} \
		if (failover_request) \
		{ \
			failover(); \
			failover_request = 0; \
		} \
		if (sigchld_request) \
		{ \
			reaper(); \
		} \
		if (reload_config_request) \
		{ \
			reload_config(); \
			reload_config_request = 0; \
		} \
    } while (0)


#define PGPOOLMAXLITSENQUEUELENGTH 10000
static void daemonize(void);
static int read_pid_file(void);
static void write_pid_file(void);
static int read_status_file(bool discard_status);
static int write_status_file(void);
static pid_t pcp_fork_a_child(int unix_fd, int inet_fd, char *pcp_conf_file);
static pid_t fork_a_child(int unix_fd, int inet_fd, int id);
static pid_t worker_fork_a_child(void);
static int create_unix_domain_socket(struct sockaddr_un un_addr_tmp);
static int create_inet_domain_socket(const char *hostname, const int port);
static void myexit(int code);
static void failover(void);
static void reaper(void);
static void wakeup_children(void);
static void reload_config(void);
static int pool_pause(struct timeval *timeout);
static void kill_all_children(int sig);
static int get_next_master_node(void);
static pid_t fork_follow_child(int old_master, int new_master, int old_primary);

static RETSIGTYPE exit_handler(int sig);
static RETSIGTYPE reap_handler(int sig);
static RETSIGTYPE failover_handler(int sig);
static RETSIGTYPE reload_config_handler(int sig);
static RETSIGTYPE health_check_timer_handler(int sig);
static RETSIGTYPE wakeup_handler(int sig);

static void usage(void);
static void show_version(void);
static void stop_me(void);

static int trigger_failover_command(int node, const char *command_line,
									int old_master, int new_master, int old_primary);

static int find_primary_node(void);
static int find_primary_node_repeatedly(void);

static struct sockaddr_un un_addr;		/* unix domain socket path */
static struct sockaddr_un pcp_un_addr;  /* unix domain socket path for PCP */

ProcessInfo *process_info;	/* Per child info table on shmem */

/*
 * Private copy of backend status
 */
BACKEND_STATUS private_backend_status[MAX_NUM_BACKENDS];

/*
 * shmem connection info table
 * this is a three dimension array. i.e.:
 * con_info[pool_config->num_init_children][pool_config->max_pool][MAX_NUM_BACKENDS]
 */
ConnectionInfo *con_info;

static int unix_fd;	/* unix domain socket fd */
static int inet_fd;	/* inet domain socket fd */

static int follow_pid; /* pid for child process handling follow command */
static int pcp_pid; /* pid for child process handling PCP */
static int pcp_unix_fd; /* unix domain socket fd for PCP (not used) */
static int pcp_inet_fd; /* inet domain socket fd for PCP */
static char pcp_conf_file[POOLMAXPATHLEN+1]; /* path for pcp.conf */
static char conf_file[POOLMAXPATHLEN+1];
static char hba_file[POOLMAXPATHLEN+1];

static int exiting = 0;		/* non 0 if I'm exiting */
static int switching = 0;		/* non 0 if I'm fail overing or degenerating */

static int clear_cache = 0;		/* non 0 if clear chache option (-c) is given */
static int not_detach = 0;		/* non 0 if non detach option (-n) is given */

static int stop_sig = SIGTERM;	/* stopping signal default value */

static volatile sig_atomic_t health_check_timer_expired;		/* non 0 if health check timer expired */

POOL_REQUEST_INFO *Req_info;		/* request info area in shared memory */
volatile sig_atomic_t *InRecovery; /* non 0 if recovery is started */
volatile sig_atomic_t reload_config_request = 0;
static volatile sig_atomic_t failover_request = 0;
static volatile sig_atomic_t sigchld_request = 0;
static volatile sig_atomic_t wakeup_request = 0;

static int pipe_fds[2]; /* for delivering signals */

int my_proc_id;

static BackendStatusRecord backend_rec;	/* Backend status record */

static pid_t worker_pid; /* pid of worker process */

BACKEND_STATUS* my_backend_status[MAX_NUM_BACKENDS];		/* Backend status buffer */
int my_master_node_id;		/* Master node id buffer */

int myargc;
char **myargv;

/*
* pgpool main program
*/
int main(int argc, char **argv)
{
	int opt;
	int i;
	int pid;
	int size;
	int retrycnt;
	int sys_retrycnt;
	int debug_level = 0;
	int	optindex;
	bool discard_status = false;

	static struct option long_options[] = {
		{"hba-file", required_argument, NULL, 'a'},
		{"clear", no_argument, NULL, 'c'},
		{"debug", no_argument, NULL, 'd'},
		{"config-file", required_argument, NULL, 'f'},
		{"pcp-file", required_argument, NULL, 'F'},
		{"help", no_argument, NULL, 'h'},
		{"mode", required_argument, NULL, 'm'},
		{"dont-detach", no_argument, NULL, 'n'},
		{"discard-status", no_argument, NULL, 'D'},
		{"version", no_argument, NULL, 'v'},
		{NULL, 0, NULL, 0}
	};
	
	myargc = argc;
	myargv = argv;

	snprintf(conf_file, sizeof(conf_file), "%s/%s", DEFAULT_CONFIGDIR, POOL_CONF_FILE_NAME);
	snprintf(pcp_conf_file, sizeof(pcp_conf_file), "%s/%s", DEFAULT_CONFIGDIR, PCP_PASSWD_FILE_NAME);
	snprintf(hba_file, sizeof(hba_file), "%s/%s", DEFAULT_CONFIGDIR, HBA_CONF_FILE_NAME);

    while ((opt = getopt_long(argc, argv, "a:cdf:F:hm:nDv", long_options, &optindex)) != -1)
	{
		switch (opt)
		{
			case 'a':    /* specify hba configuration file */
				if (!optarg)
				{
					usage();
					exit(1);
				}
				strlcpy(hba_file, optarg, sizeof(hba_file));
				break;

			case 'c':			/* clear cache option */
				clear_cache = 1;
				break;

			case 'd':	/* debug option */
				debug_level = 1;
				break;

			case 'f':	/* specify configuration file */
				if (!optarg)
				{
					usage();
					exit(1);
				}
				strlcpy(conf_file, optarg, sizeof(conf_file));
				break;

			case 'F':   /* specify PCP password file */
				if (!optarg)
				{
					usage();
					exit(1);
				}
				strlcpy(pcp_conf_file, optarg, sizeof(pcp_conf_file));
				break;

			case 'h':
				usage();
				exit(0);
				break;

			case 'm':	/* stop mode */
				if (!optarg)
				{
					usage();
					exit(1);
				}
				if (*optarg == 's' || !strcmp("smart", optarg))
					stop_sig = SIGTERM;		/* smart shutdown */
				else if (*optarg == 'f' || !strcmp("fast", optarg))
					stop_sig = SIGINT;		/* fast shutdown */
				else if (*optarg == 'i' || !strcmp("immediate", optarg))
					stop_sig = SIGQUIT;		/* immediate shutdown */
				else
				{
					usage();
					exit(1);
				}
				break;

			case 'n':	/* no detaching control ttys */
				not_detach = 1;
				break;

			case 'D':	/* discard pgpool_status */
				discard_status = true;
				break;

			case 'v':
				show_version();
				exit(0);

			default:
				usage();
				exit(1);
		}
	}

#ifdef USE_SSL
	/* global ssl init */
	SSL_library_init();
	SSL_load_error_strings();
#endif /* USE_SSL */

	mypid = getpid();

	if (pool_init_config())
		exit(1);

	/*
	 * Override debug level
	 */
	pool_config->debug_level = debug_level;

	if (pool_get_config(conf_file, INIT_CONFIG))
	{
		pool_error("Unable to get configuration. Exiting...");
		exit(1);
	}

	/*
	 * Open syslog connection if required
	 */
	if (!strcmp(pool_config->log_destination, "syslog")) {
		openlog(pool_config->syslog_ident, LOG_PID|LOG_NDELAY|LOG_NOWAIT, pool_config->syslog_facility);
		/* set a flag to allow pool_error.c to begin writing to syslog
		   instead of stdout now that pool_get_config() is done */
		pool_config->logsyslog = 1;
	}

	/*
	 * Override debug level
	 */
	if (pool_config->debug_level == 0)
		pool_config->debug_level = debug_level;

	if (pool_config->enable_pool_hba)
		load_hba(hba_file);

	/*
	 * If a non-switch argument remains, then it should be either "reload" or "stop".
	 */
	if (optind == (argc - 1))
	{
		if (!strcmp(argv[optind], "reload"))
		{
				pid_t pid;

				pid = read_pid_file();
				if (pid < 0)
				{
					pool_error("could not read pid file");
					pool_shmem_exit(1);
					exit(1);
				}

				if (kill(pid, SIGHUP) == -1)
				{
					pool_error("could not reload configuration file pid: %d. reason: %s", pid, strerror(errno));
					pool_shmem_exit(1);
					exit(1);
				}
				pool_shmem_exit(0);
				exit(0);
		}
		if (!strcmp(argv[optind], "stop"))
		{
			stop_me();
			pool_shmem_exit(0);
			exit(0);
		}
		else
		{
			usage();
			pool_shmem_exit(1);
			exit(1);
		}
	}
	/*
	 * else if no non-switch argument remains, then it should be a start request
	 */
	else if (optind == argc)
	{
		pid = read_pid_file();
		if (pid > 0)
		{
			if (kill(pid, 0) == 0)
			{
				fprintf(stderr, "pid file found. is another pgpool(%d) is running?\n", pid);
				exit(1);
			}
			else
				fprintf(stderr, "pid file found but it seems bogus. Trying to start pgpool anyway...\n");
		}
	}
	/*
	 * otherwise an error...
	 */
	else
	{
		usage();
		exit(1);
	}

	/* set signal masks */
	poolinitmask();

	if (not_detach)
		write_pid_file();
	else
		daemonize();

	/*
	 * Locate pool_passwd
	 */
	if (strcmp("", pool_config->pool_passwd))
	{
		char pool_passwd[POOLMAXPATHLEN+1];
		char dirnamebuf[POOLMAXPATHLEN+1];
		char *dirp;

		strlcpy(dirnamebuf, conf_file, sizeof(dirnamebuf));
		dirp = dirname(dirnamebuf);
		snprintf(pool_passwd, sizeof(pool_passwd), "%s/%s",
				 dirp, pool_config->pool_passwd);
		pool_init_pool_passwd(pool_passwd);
	}

	if (pool_semaphore_create(MAX_NUM_SEMAPHORES))
	{
		pool_error("Unable to create semaphores. Exiting...");
		pool_shmem_exit(1);
		exit(1);
	}

	/*
	 * Restore previous backend status if possible
	 */
	read_status_file(discard_status);

	/* clear cache */
	if (clear_cache && pool_config->enable_query_cache && SYSDB_STATUS == CON_UP)
	{
		Interval interval[1];

		interval[0].quantity = 0;
		interval[0].unit = second;

		pool_clear_cache_by_time(interval, 1);
	}

	/* set unix domain socket path */
	snprintf(un_addr.sun_path, sizeof(un_addr.sun_path), "%s/.s.PGSQL.%d",
			 pool_config->socket_dir,
			 pool_config->port);

	/* set up signal handlers */
	pool_signal(SIGPIPE, SIG_IGN);

	/* create unix domain socket */
	unix_fd = create_unix_domain_socket(un_addr);

	/* create inet domain socket if any */
	if (pool_config->listen_addresses[0])
	{
		inet_fd = create_inet_domain_socket(pool_config->listen_addresses, pool_config->port);
	}

	/*
	 * con_info is a 3 dimension array: i corresponds to pgpool child
	 * process, j corresponds to connection pool in each process and k
	 * corresponds to backends in each connection pool.
	 *
	 * XXX: Before 2010/4/12 this was a 2 dimension array: i
	 * corresponds to pgpool child process, j corresponds to
	 * connection pool in each process. Of course this was wrong.
	 */
	size = pool_coninfo_size();
	con_info = pool_shared_memory_create(size);
	if (con_info == NULL)
	{
		pool_error("failed to allocate connection informations");
		myexit(1);
	}
	memset(con_info, 0, size);

	size = pool_config->num_init_children * (sizeof(ProcessInfo));
	process_info = pool_shared_memory_create(size);
	if (process_info == NULL)
	{
		pool_error("failed to allocate process_info");
		myexit(1);
	}
	memset(process_info, 0, size);
	for (i = 0; i < pool_config->num_init_children; i++)
	{
		process_info[i].connection_info = pool_coninfo(i,0,0);
	}

	/* create fail over/switch over event area */
	Req_info = pool_shared_memory_create(sizeof(POOL_REQUEST_INFO));
	if (Req_info == NULL)
	{
		pool_error("failed to allocate Req_info");
		myexit(1);
	}

	/*
	 * Initialize backend status area.
	 * From now on, VALID_BACKEND macro can be used.
	 * (get_next_master_node() uses VALID_BACKEND)
	 */

	for (i=0;i<MAX_NUM_BACKENDS;i++)
	{
		my_backend_status[i] = &(BACKEND_INFO(i).backend_status);
	}

	/* initialize Req_info */
	Req_info->kind = NODE_UP_REQUEST;
	memset(Req_info->node_id, -1, sizeof(int) * MAX_NUM_BACKENDS);
	Req_info->master_node_id = get_next_master_node();
	Req_info->conn_counter = 0;
	Req_info->switching = false;

	InRecovery = pool_shared_memory_create(sizeof(int));
	if (InRecovery == NULL)
	{
		pool_error("failed to allocate InRecovery");
		myexit(1);
	}
	*InRecovery = 0;

	/*
	 * We need to block signal here. Otherwise child might send some
	 * signals, for example SIGUSR1(fail over).  Children will inherit
	 * signal blocking but they do unblock signals at the very beginning
	 * of process.  So this is harmless.
	 */
	POOL_SETMASK(&BlockSig);

	/* fork the children */
	for (i=0;i<pool_config->num_init_children;i++)
	{
		process_info[i].pid = fork_a_child(unix_fd, inet_fd, i);
		process_info[i].start_time = time(NULL);
	}

	/* set up signal handlers */

	pool_signal(SIGTERM, exit_handler);
	pool_signal(SIGINT, exit_handler);
	pool_signal(SIGQUIT, exit_handler);
	pool_signal(SIGCHLD, reap_handler);
	pool_signal(SIGUSR1, failover_handler);
	pool_signal(SIGUSR2, wakeup_handler);
	pool_signal(SIGHUP, reload_config_handler);

	/* create pipe for delivering event */
	if (pipe(pipe_fds) < 0)
	{
		pool_error("failed to create pipe");
		myexit(1);
	}

	pool_log("%s successfully started. version %s (%s)", PACKAGE, VERSION, PGPOOLVERSION);

	/* fork a child for PCP handling */
	snprintf(pcp_un_addr.sun_path, sizeof(pcp_un_addr.sun_path), "%s/.s.PGSQL.%d",
			 pool_config->pcp_socket_dir,
			 pool_config->pcp_port);
	pcp_unix_fd = create_unix_domain_socket(pcp_un_addr);
    /* maybe change "*" to pool_config->pcp_listen_addresses */
	pcp_inet_fd = create_inet_domain_socket("*", pool_config->pcp_port);
	pcp_pid = pcp_fork_a_child(pcp_unix_fd, pcp_inet_fd, pcp_conf_file);

	/* Fork worker process */
	worker_pid = worker_fork_a_child();

	retrycnt = 0;		/* reset health check retry counter */
	sys_retrycnt = 0;	/* reset SystemDB health check retry counter */

	/* Save primary node id */
	Req_info->primary_node_id = find_primary_node();

	/*
	 * This is the main loop
	 */
	for (;;)
	{
		CHECK_REQUEST;

		/* do we need health checking for PostgreSQL? */
		if (pool_config->health_check_period > 0)
		{
			int sts;
			int sys_sts = 0;
			unsigned int sleep_time;

			if (retrycnt == 0)
			{
				pool_debug("starting health checking");
			}
			else
			{
				pool_debug("retrying %d th health checking", retrycnt);
			}

			if (pool_config->health_check_timeout > 0)
			{
				/*
				 * set health checker timeout. we want to detect
				 * communication path failure much earlier before
				 * TCP/IP stack detects it.
				 */
				pool_signal(SIGALRM, health_check_timer_handler);
				alarm(pool_config->health_check_timeout);
			}

			/*
			 * do actual health check. trying to connect to the backend
			 */
			errno = 0;
			health_check_timer_expired = 0;
			POOL_SETMASK(&UnBlockSig);
			sts = health_check();
			POOL_SETMASK(&BlockSig);
			if (pool_config->parallel_mode || pool_config->enable_query_cache)
				sys_sts = system_db_health_check();

			if ((sts > 0 || sys_sts < 0) && (errno != EINTR || (errno == EINTR && health_check_timer_expired)))
			{
				if (sts > 0)
				{
					sts--;

					if (!pool_config->parallel_mode)
					{
						if (POOL_DISALLOW_TO_FAILOVER(BACKEND_INFO(sts).flag))
						{
							pool_log("health_check: %d failover is canceld because failover is disallowed", sts);
						}
						else
						{
							pool_log("set %d th backend down status", sts);
							Req_info->kind = NODE_DOWN_REQUEST;
							Req_info->node_id[0] = sts;
							failover();
							/* need to distribute this info to children */
						}
					}
					else
					{
						retrycnt++;
						pool_signal(SIGALRM, SIG_IGN);	/* Cancel timer */

						if (retrycnt > NUM_BACKENDS)
						{
							/* retry count over */
							pool_log("set %d th backend down status", sts);
							Req_info->kind = NODE_DOWN_REQUEST;
							Req_info->node_id[0] = sts;
							failover();
							retrycnt = 0;
						}
						else
						{
							/* continue to retry */
							sleep_time = pool_config->health_check_period/NUM_BACKENDS;
							pool_debug("retry sleep time: %d seconds", sleep_time);
							pool_sleep(sleep_time);
							continue;
						}
					}
				}
				if (sys_sts < 0)
				{
					sys_retrycnt++;
					pool_signal(SIGALRM, SIG_IGN);

					if (sys_retrycnt > NUM_BACKENDS)
					{
						pool_log("set SystemDB down status");
						SYSDB_STATUS = CON_DOWN;
						sys_retrycnt = 0;
					}
					else if (sts == 0) /* goes to sleep only when SystemDB alone was down */
					{
						sleep_time = pool_config->health_check_period/NUM_BACKENDS;
						pool_debug("retry sleep time: %d seconds", sleep_time);
						pool_sleep(sleep_time);
						continue;
					}
				}
			}

			if (pool_config->health_check_timeout > 0)
			{
				/* seems ok. cancel health check timer */
				pool_signal(SIGALRM, SIG_IGN);
			}

			sleep_time = pool_config->health_check_period;
			pool_sleep(sleep_time);
		}
		else
		{
			for (;;)
			{
				int r;
				struct timeval t = {3, 0};

				POOL_SETMASK(&UnBlockSig);
				r = pool_pause(&t);
				POOL_SETMASK(&BlockSig);
				if (r > 0)
					break;
			}
		}
	}

	pool_shmem_exit(0);
}

static void show_version(void)
{
	fprintf(stderr, "%s version %s (%s)\n",	PACKAGE, VERSION, PGPOOLVERSION);
}

static void usage(void)
{
	fprintf(stderr, "%s version %s (%s),\n",	PACKAGE, VERSION, PGPOOLVERSION);
	fprintf(stderr, "  A generic connection pool/replication/load balance server for PostgreSQL\n\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  pgpool [ -c] [ -f CONFIG_FILE ] [ -F PCP_CONFIG_FILE ] [ -a HBA_CONFIG_FILE ]\n");
	fprintf(stderr, "         [ -n ] [ -D ] [ -d ]\n");
	fprintf(stderr, "  pgpool [ -f CONFIG_FILE ] [ -F PCP_CONFIG_FILE ] [ -a HBA_CONFIG_FILE ]\n");
	fprintf(stderr, "         [ -m SHUTDOWN-MODE ] stop\n");
	fprintf(stderr, "  pgpool [ -f CONFIG_FILE ] [ -F PCP_CONFIG_FILE ] [ -a HBA_CONFIG_FILE ] reload\n\n");
	fprintf(stderr, "Common options:\n");
	fprintf(stderr, "  -a, --hba-file=HBA_CONFIG_FILE\n");
	fprintf(stderr, "                      Sets the path to the pool_hba.conf configuration file\n");
	fprintf(stderr, "                      (default: %s/%s)\n",DEFAULT_CONFIGDIR, HBA_CONF_FILE_NAME);
	fprintf(stderr, "  -f, --config-file=CONFIG_FILE\n");
	fprintf(stderr, "                      Sets the path to the pgpool.conf configuration file\n");
	fprintf(stderr, "                      (default: %s/%s)\n",DEFAULT_CONFIGDIR, POOL_CONF_FILE_NAME);
	fprintf(stderr, "  -F, --pcp-file=PCP_CONFIG_FILE\n");
	fprintf(stderr, "                      Sets the path to the pcp.conf configuration file\n");
	fprintf(stderr, "                      (default: %s/%s)\n",DEFAULT_CONFIGDIR, PCP_PASSWD_FILE_NAME);
	fprintf(stderr, "  -h, --help          Prints this help\n\n");
	fprintf(stderr, "Start options:\n");
	fprintf(stderr, "  -c, --clear         Clears query cache (enable_query_cache must be on)\n");
	fprintf(stderr, "  -n, --dont-detach   Don't run in daemon mode, does not detach control tty\n");
	fprintf(stderr, "  -D, --discard-status Discard pgpool_status file and do not restore previous status\n");
	fprintf(stderr, "  -d, --debug         Debug mode\n\n");
	fprintf(stderr, "Stop options:\n");
	fprintf(stderr, "  -m, --mode=SHUTDOWN-MODE\n");
	fprintf(stderr, "                      Can be \"smart\", \"fast\", or \"immediate\"\n\n");
	fprintf(stderr, "Shutdown modes are:\n");
	fprintf(stderr, "  smart       quit after all clients have disconnected\n");
	fprintf(stderr, "  fast        quit directly, with proper shutdown\n");
	fprintf(stderr, "  immediate   the same mode as fast\n");
}

/*
* detach control ttys
*/
static void daemonize(void)
{
	int			i;
	pid_t		pid;
	int			fdlimit;
    int         rc_chdir;

	pid = fork();
	if (pid == (pid_t) -1)
	{
		pool_error("fork() failed. reason: %s", strerror(errno));
		pool_shmem_exit(1);
		exit(1);
		return;					/* not reached */
	}
	else if (pid > 0)
	{			/* parent */
		pool_shmem_exit(0);
		exit(0);
	}

#ifdef HAVE_SETSID
	if (setsid() < 0)
	{
		pool_error("setsid() failed. reason:%s", strerror(errno));
		pool_shmem_exit(1);
		exit(1);
	}
#endif

	mypid = getpid();

	rc_chdir = chdir("/");

	i = open("/dev/null", O_RDWR);
	dup2(i, 0);
	dup2(i, 1);
	dup2(i, 2);

	/* close syslog connection for daemonizing */
	if (pool_config->logsyslog) {
		closelog();
	}

    fdlimit = sysconf(_SC_OPEN_MAX);
    for (i = 3; i < fdlimit; i++)
		close(i);

	/* reopen syslog connection after daemonizing */
	if (pool_config->logsyslog) {
		openlog(pool_config->syslog_ident, LOG_PID|LOG_NDELAY|LOG_NOWAIT, pool_config->syslog_facility);
	}

	write_pid_file();
}


/*
* stop myself
*/
static void stop_me(void)
{
	pid_t pid;

	pid = read_pid_file();
	if (pid < 0)
	{
		pool_error("could not read pid file");
		pool_shmem_exit(1);
		exit(1);
	}

	if (kill(pid, stop_sig) == -1)
	{
		pool_error("could not stop pid: %d. reason: %s", pid, strerror(errno));
		pool_shmem_exit(1);
		exit(1);
	}

	fprintf(stderr, "stop request sent to pgpool. waiting for termination...");

	while (kill(pid, 0) == 0)
	{
		fprintf(stderr, ".");
		sleep(1);
	}
	fprintf(stderr, "done.\n");
}

/*
* read the pid file
*/
static int read_pid_file(void)
{
	FILE *fd;
	char pidbuf[128];

	fd = fopen(pool_config->pid_file_name, "r");
	if (!fd)
	{
		return -1;
	}
	if (fread(pidbuf, 1, sizeof(pidbuf), fd) <= 0)
	{
		pool_error("could not read pid file as %s. reason: %s",
				   pool_config->pid_file_name, strerror(errno));
		fclose(fd);
		return -1;
	}
	fclose(fd);
	return(atoi(pidbuf));
}

/*
* write the pid file
*/
static void write_pid_file(void)
{
	FILE *fd;
	char pidbuf[128];

	fd = fopen(pool_config->pid_file_name, "w");
	if (!fd)
	{
		pool_error("could not open pid file as %s. reason: %s",
				   pool_config->pid_file_name, strerror(errno));
		pool_shmem_exit(1);
		exit(1);
	}
	snprintf(pidbuf, sizeof(pidbuf), "%d", (int)getpid());
	fwrite(pidbuf, strlen(pidbuf)+1, 1, fd);
	if (fclose(fd))
	{
		pool_error("could not write pid file as %s. reason: %s",
				   pool_config->pid_file_name, strerror(errno));
		pool_shmem_exit(1);
		exit(1);
	}
}

/*
* Read the status file
*/
static int read_status_file(bool discard_status)
{
	FILE *fd;
	char fnamebuf[POOLMAXPATHLEN];
	int i;
	bool someone_wakeup = false;

	snprintf(fnamebuf, sizeof(fnamebuf), "%s/%s", pool_config->logdir, STATUS_FILE_NAME);
	fd = fopen(fnamebuf, "r");
	if (!fd)
	{
		pool_log("Backend status file %s does not exist", fnamebuf);
		return -1;
	}

	/*
	 * If discard_status is true, unlink pgpool_status and
	 * do not restore previous status.
	 */
	if (discard_status)
	{
		fclose(fd);
		if (unlink(fnamebuf) == 0)
		{
			pool_log("Backend status file %s discarded", fnamebuf);
		}
		else
		{
			pool_error("Failed to discard backend status file %s reason:%s", fnamebuf, strerror(errno));
		}
		return 0;
	}

	if (fread(&backend_rec, 1, sizeof(backend_rec), fd) != sizeof(backend_rec))
	{
		pool_error("Could not read backend status file as %s. reason: %s",
				   fnamebuf, strerror(errno));
		fclose(fd);
		return -1;
	}
	fclose(fd);

	for (i=0;i< pool_config->backend_desc->num_backends;i++)
	{
		if (backend_rec.status[i] == CON_DOWN)
		{
			BACKEND_INFO(i).backend_status = CON_DOWN;
			pool_log("read_status_file: %d th backend is set to down status", i);
		}
		else
		{
			BACKEND_INFO(i).backend_status = CON_CONNECT_WAIT;
			someone_wakeup = true;
		}
	}

	/*
	 * If no one woke up, we regard the status file bogus
	 */
	if (someone_wakeup == false)
	{
		for (i=0;i< pool_config->backend_desc->num_backends;i++)
		{
			BACKEND_INFO(i).backend_status = CON_CONNECT_WAIT;
		}
	}

	return 0;
}

/*
* Write the pid file
*/
static int write_status_file(void)
{
	FILE *fd;
	char fnamebuf[POOLMAXPATHLEN];
	int i;

	snprintf(fnamebuf, sizeof(fnamebuf), "%s/%s", pool_config->logdir, STATUS_FILE_NAME);
	fd = fopen(fnamebuf, "w");
	if (!fd)
	{
		pool_error("Could not open status file %s", fnamebuf);
		return -1;
	}

	memset(&backend_rec, 0, sizeof(backend_rec));

	for (i=0;i< pool_config->backend_desc->num_backends;i++)
	{
		backend_rec.status[i] = BACKEND_INFO(i).backend_status;
	}

	if (fwrite(&backend_rec, 1, sizeof(backend_rec), fd) != sizeof(backend_rec))
	{
		pool_error("Could not write backend status file as %s. reason: %s",
				   fnamebuf, strerror(errno));
		fclose(fd);
		return -1;
	}
	fclose(fd);
	return 0;
}

/*
 * fork a child for PCP
 */
pid_t pcp_fork_a_child(int unix_fd, int inet_fd, char *pcp_conf_file)
{
	pid_t pid;

	pid = fork();

	if (pid == 0)
	{
		close(pipe_fds[0]);
		close(pipe_fds[1]);

		myargv = save_ps_display_args(myargc, myargv);

		/* call PCP child main */
		POOL_SETMASK(&UnBlockSig);
		reload_config_request = 0;
		run_as_pcp_child = true;
		pcp_do_child(unix_fd, inet_fd, pcp_conf_file);
	}
	else if (pid == -1)
	{
		pool_error("fork() failed. reason: %s", strerror(errno));
		myexit(1);
	}
	return pid;
}

/*
* fork a child
*/
pid_t fork_a_child(int unix_fd, int inet_fd, int id)
{
	pid_t pid;

	pid = fork();

	if (pid == 0)
	{
		/* Before we unconditionally closed pipe_fds[0] and pipe_fds[1]
		 * here, which is apparently wrong since in the start up of
		 * pgpool, pipe(2) is not called yet and it mistakenly closes
		 * fd 0. Now we check the fd > 0 before close(), expecting
		 * pipe returns fds greater than 0.  Note that we cannot
		 * unconditionally remove close(2) calls since fork_a_child()
		 * may be called *after* pgpool starting up.
		 */
		if (pipe_fds[0] > 0)
		{
			close(pipe_fds[0]);
			close(pipe_fds[1]);
		}

		myargv = save_ps_display_args(myargc, myargv);

		/* call child main */
		POOL_SETMASK(&UnBlockSig);
		reload_config_request = 0;
		my_proc_id = id;
		run_as_pcp_child = false;
		do_child(unix_fd, inet_fd);
	}
	else if (pid == -1)
	{
		pool_error("fork() failed. reason: %s", strerror(errno));
		myexit(1);
	}
	return pid;
}

/*
* fork worker child process
*/
pid_t worker_fork_a_child()
{
	pid_t pid;

	pid = fork();

	if (pid == 0)
	{
		/* Before we unconditionally closed pipe_fds[0] and pipe_fds[1]
		 * here, which is apparently wrong since in the start up of
		 * pgpool, pipe(2) is not called yet and it mistakenly closes
		 * fd 0. Now we check the fd > 0 before close(), expecting
		 * pipe returns fds greater than 0.  Note that we cannot
		 * unconditionally remove close(2) calls since fork_a_child()
		 * may be called *after* pgpool starting up.
		 */
		if (pipe_fds[0] > 0)
		{
			close(pipe_fds[0]);
			close(pipe_fds[1]);
		}

		myargv = save_ps_display_args(myargc, myargv);

		/* call child main */
		POOL_SETMASK(&UnBlockSig);
		reload_config_request = 0;
		do_worker_child();
	}
	else if (pid == -1)
	{
		pool_error("fork() failed. reason: %s", strerror(errno));
		myexit(1);
	}
	return pid;
}

/*
* create inet domain socket
*/
static int create_inet_domain_socket(const char *hostname, const int port)
{
	struct sockaddr_in addr;
	int fd;
	int status;
	int one = 1;
	int len;
	int backlog;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		pool_error("Failed to create INET domain socket. reason: %s", strerror(errno));
		myexit(1);
	}
	if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &one,
					sizeof(one))) == -1)
	{
		pool_error("setsockopt() failed. reason: %s", strerror(errno));
		myexit(1);
	}

	memset((char *) &addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;

	if (strcmp(hostname, "*")==0)
	{
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		struct hostent *hostinfo;

		hostinfo = gethostbyname(hostname);
		if (!hostinfo)
		{
			pool_error("could not resolve host name \"%s\": %s", hostname, hstrerror(h_errno));
			myexit(1);
		}
		addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;
	}

	addr.sin_port = htons(port);
	len = sizeof(struct sockaddr_in);
	status = bind(fd, (struct sockaddr *)&addr, len);
	if (status == -1)
	{
		char *host = "", *serv = "";
		char hostname[NI_MAXHOST], servname[NI_MAXSERV];
		if (getnameinfo((struct sockaddr *) &addr, len, hostname, sizeof(hostname), servname, sizeof(servname), 0) == 0) {
			host = hostname;
			serv = servname;
		}
		pool_error("bind(%s:%s) failed. reason: %s", host, serv, strerror(errno));
		myexit(1);
	}

	backlog = pool_config->num_init_children * 2;
	if (backlog > PGPOOLMAXLITSENQUEUELENGTH)
		backlog = PGPOOLMAXLITSENQUEUELENGTH;

	status = listen(fd, backlog);
	if (status < 0)
	{
		pool_error("listen() failed. reason: %s", strerror(errno));
		myexit(1);
	}
	return fd;
}

/*
* create UNIX domain socket
*/
static int create_unix_domain_socket(struct sockaddr_un un_addr_tmp)
{
	struct sockaddr_un addr;
	int fd;
	int status;
	int len;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1)
	{
		pool_error("Failed to create UNIX domain socket. reason: %s", strerror(errno));
		myexit(1);
	}
	memset((char *) &addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", un_addr_tmp.sun_path);
	len = sizeof(struct sockaddr_un);
	status = bind(fd, (struct sockaddr *)&addr, len);
	if (status == -1)
	{
		pool_error("bind(%s) failed. reason: %s", addr.sun_path, strerror(errno));
		myexit(1);
	}

	if (chmod(un_addr_tmp.sun_path, 0777) == -1)
	{
		pool_error("chmod() failed. reason: %s", strerror(errno));
		myexit(1);
	}

	status = listen(fd, PGPOOLMAXLITSENQUEUELENGTH);
	if (status < 0)
	{
		pool_error("listen() failed. reason: %s", strerror(errno));
		myexit(1);
	}
	return fd;
}

static void myunlink(const char* path)
{
	if (unlink(path) == 0) return;
	pool_error("unlink(%s) failed: %s", path, strerror(errno));
}

static void myexit(int code)
{
	int i;

	if (getpid() != mypid)
		return;

	if (process_info != NULL) {
		POOL_SETMASK(&AuthBlockSig);
		exiting = 1;
		for (i = 0; i < pool_config->num_init_children; i++)
		{
			pid_t pid = process_info[i].pid;
			if (pid)
			{
				kill(pid, SIGTERM);
			}
		}
		while (wait(NULL) > 0)
			;
		if (errno != ECHILD)
			pool_error("wait() failed. reason:%s", strerror(errno));
		POOL_SETMASK(&UnBlockSig);
	}

	myunlink(un_addr.sun_path);
	myunlink(pcp_un_addr.sun_path);
	myunlink(pool_config->pid_file_name);

	write_status_file();

	pool_shmem_exit(code);
	exit(code);
}

void notice_backend_error(int node_id)
{
	int n = node_id;

	degenerate_backend_set(&n, 1);
}

/* notice backend connection error using SIGUSR1 */
void degenerate_backend_set(int *node_id_set, int count)
{
	pid_t parent = getppid();
	int i;
	bool need_signal = false;

	if (pool_config->parallel_mode)
	{
		return;
	}

	pool_semaphore_lock(REQUEST_INFO_SEM);
	Req_info->kind = NODE_DOWN_REQUEST;
	for (i = 0; i < count; i++)
	{
		if (node_id_set[i] < 0 || node_id_set[i] >= MAX_NUM_BACKENDS ||
			!VALID_BACKEND(node_id_set[i]))
		{
			pool_log("degenerate_backend_set: node %d is not valid backend.", i);
			continue;
		}

		if (POOL_DISALLOW_TO_FAILOVER(BACKEND_INFO(node_id_set[i]).flag))
		{
			pool_log("degenerate_backend_set: %d failover request from pid %d is canceld because failover is disallowed", node_id_set[i], getpid());
			continue;
		}

		pool_log("degenerate_backend_set: %d fail over request from pid %d", node_id_set[i], getpid());
		Req_info->node_id[i] = node_id_set[i];
		need_signal = true;
	}

	if (need_signal)
		kill(parent, SIGUSR1);

	pool_semaphore_unlock(REQUEST_INFO_SEM);
}

/* send promote node request using SIGUSR1 */
void promote_backend(int node_id)
{
	pid_t parent = getppid();

	if (!MASTER_SLAVE || strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP))
	{
		return;
	}

	if (node_id < 0 || node_id >= MAX_NUM_BACKENDS || !VALID_BACKEND(node_id))
	{
		pool_error("promote_backend: node %d is not valid backend.", node_id);
		return;
	}

	pool_semaphore_lock(REQUEST_INFO_SEM);
	Req_info->kind = PROMOTE_NODE_REQUEST;
	Req_info->node_id[0] = node_id;
	pool_log("promote_backend: %d promote node request from pid %d", node_id, getpid());
	kill(parent, SIGUSR1);
	pool_semaphore_unlock(REQUEST_INFO_SEM);
}

/* send failback request using SIGUSR1 */
void send_failback_request(int node_id)
{
	pid_t parent = getppid();

	pool_log("send_failback_request: fail back %d th node request from pid %d", node_id, getpid());
	Req_info->kind = NODE_UP_REQUEST;
	Req_info->node_id[0] = node_id;

    if (node_id < 0 || node_id >= MAX_NUM_BACKENDS || 
		(RAW_MODE && BACKEND_INFO(node_id).backend_status != CON_DOWN && VALID_BACKEND(node_id)))
	{
		pool_error("send_failback_request: node %d is alive.", node_id);
		return;
	}

	kill(parent, SIGUSR1);
}

static RETSIGTYPE exit_handler(int sig)
{
	int i;

	POOL_SETMASK(&AuthBlockSig);

	/*
	 * this could happen in a child process if a signal has been sent
	 * before resetting signal handler
	 */
	if (getpid() != mypid)
	{
		pool_debug("exit_handler: I am not parent");
		POOL_SETMASK(&UnBlockSig);
		pool_shmem_exit(0);
		exit(0);
	}

	if (sig == SIGTERM)
		pool_log("received smart shutdown request");
	else if (sig == SIGINT)
		pool_log("received fast shutdown request");
	else if (sig == SIGQUIT)
		pool_log("received immediate shutdown request");
	else
	{
		pool_error("exit_handler: unknown signal received %d", sig);
		POOL_SETMASK(&UnBlockSig);
		return;
	}

	exiting = 1;

	for (i = 0; i < pool_config->num_init_children; i++)
	{
		pid_t pid = process_info[i].pid;
		if (pid)
		{
			kill(pid, sig);
		}
	}

	kill(pcp_pid, sig);
	kill(worker_pid, sig);

	POOL_SETMASK(&UnBlockSig);

	while (wait(NULL) > 0)
		;

	if (errno != ECHILD)
		pool_error("wait() failed. reason:%s", strerror(errno));

	process_info = NULL;
	myexit(0);
}

/*
 * Calculate next valid master node id.
 * If no valid node found, returns -1.
 */
static int get_next_master_node(void)
{
	int i;

	for (i=0;i<pool_config->backend_desc->num_backends;i++)
	{
		/*
		 * Do not use VALID_BACKEND macro in raw mode.
		 * VALID_BACKEND return true only if the argument is master
		 * node id. In other words, standby nodes are false. So need
		 * to check backend status without VALID_BACKEND.
		 */
		if (RAW_MODE)
		{
			/* if in raw mode, we can switch to a standby node */
			if (BACKEND_INFO(i).backend_status == CON_CONNECT_WAIT)
				break;
		}
		else if (VALID_BACKEND(i))
			break;
	}

	if (i == pool_config->backend_desc->num_backends)
		i = -1;

	return i;
}

/*
 * handle SIGUSR1
 *
 */
static RETSIGTYPE failover_handler(int sig)
{
	POOL_SETMASK(&BlockSig);
	failover_request = 1;
	write(pipe_fds[1], "\0", 1);
	POOL_SETMASK(&UnBlockSig);
}

/*
 * backend connection error, failover/failback request, if possible
 * failover() must be called under protecting signals.
 */
static void failover(void)
{
	int i;
	int node_id;
	int new_master;
	int new_primary;
	int nodes[MAX_NUM_BACKENDS];
	bool need_to_restart_children;

	pool_debug("failover_handler called");

	memset(nodes, 0, sizeof(int) * MAX_NUM_BACKENDS);

	/*
	 * this could happen in a child process if a signal has been sent
	 * before resetting signal handler
	 */
	if (getpid() != mypid)
	{
		pool_debug("failover_handler: I am not parent");
		kill(pcp_pid, SIGUSR2);
		return;
	}

	/*
	 * processing SIGTERM, SIGINT or SIGQUIT
	 */
	if (exiting)
	{
		pool_debug("failover_handler called while exiting");
		kill(pcp_pid, SIGUSR2);
		return;
	}

	/*
	 * processing fail over or switch over
	 */
	if (switching)
	{
		pool_debug("failover_handler called while switching");
		kill(pcp_pid, SIGUSR2);
		return;
	}

	pool_semaphore_lock(REQUEST_INFO_SEM);

	if (Req_info->kind == CLOSE_IDLE_REQUEST)
	{
		pool_semaphore_unlock(REQUEST_INFO_SEM);
		kill_all_children(SIGUSR1);
		kill(pcp_pid, SIGUSR2);
		return;
	}

	/*
	 * if not in replication mode/master slave mode, we treat this a restart request.
	 * otherwise we need to check if we have already failovered.
	 */
	pool_debug("failover_handler: starting to select new master node");
	switching = 1;
	Req_info->switching = true;
	node_id = Req_info->node_id[0];

	/* failback request? */
	if (Req_info->kind == NODE_UP_REQUEST)
	{
		if (node_id >= MAX_NUM_BACKENDS ||
			(Req_info->kind == NODE_UP_REQUEST && !(RAW_MODE &&
            BACKEND_INFO(node_id).backend_status == CON_DOWN) && VALID_BACKEND(node_id)) ||
			(Req_info->kind == NODE_DOWN_REQUEST && !VALID_BACKEND(node_id)))
		{
			pool_semaphore_unlock(REQUEST_INFO_SEM);
			pool_error("failover_handler: invalid node_id %d status:%d MAX_NUM_BACKENDS: %d", node_id,
					   BACKEND_INFO(node_id).backend_status, MAX_NUM_BACKENDS);
			kill(pcp_pid, SIGUSR2);
			switching = 0;
			Req_info->switching = false;
			return;
		}

		pool_log("starting fail back. reconnect host %s(%d)",
				 BACKEND_INFO(node_id).backend_hostname,
				 BACKEND_INFO(node_id).backend_port);
		BACKEND_INFO(node_id).backend_status = CON_CONNECT_WAIT;	/* unset down status */
		trigger_failover_command(node_id, pool_config->failback_command,
								 MASTER_NODE_ID, get_next_master_node(), PRIMARY_NODE_ID);
	}
	else if (Req_info->kind == PROMOTE_NODE_REQUEST)
	{
		if (node_id != -1 && VALID_BACKEND(node_id))
		{
			pool_log("starting promotion. promote host %s(%d)",
					 BACKEND_INFO(node_id).backend_hostname,
					 BACKEND_INFO(node_id).backend_port);
		}
		else
		{
			pool_log("failover: no backends are promoted");
			pool_semaphore_unlock(REQUEST_INFO_SEM);
			kill(pcp_pid, SIGUSR2);
			switching = 0;
			Req_info->switching = false;
			return;
		}
	}
	else
	{
		int cnt = 0;

		for (i = 0; i < MAX_NUM_BACKENDS; i++)
		{
			if (Req_info->node_id[i] != -1 &&
				VALID_BACKEND(Req_info->node_id[i]))
			{
				pool_log("starting degeneration. shutdown host %s(%d)",
						 BACKEND_INFO(Req_info->node_id[i]).backend_hostname,
						 BACKEND_INFO(Req_info->node_id[i]).backend_port);


				BACKEND_INFO(Req_info->node_id[i]).backend_status = CON_DOWN;	/* set down status */
				/* save down node */
				nodes[Req_info->node_id[i]] = 1;
				cnt++;
			}
		}

		if (cnt == 0)
		{
			pool_log("failover: no backends are degenerated");
			pool_semaphore_unlock(REQUEST_INFO_SEM);
			kill(pcp_pid, SIGUSR2);
			switching = 0;
			Req_info->switching = false;
			return;
		}
	}

	new_master = get_next_master_node();

	if (new_master < 0)
	{
		pool_error("failover_handler: no valid DB node found");
	}

/*
 * Before we tried to minimize restarting pgpool to protect existing
 * connections from clients to pgpool children. What we did here was,
 * if children other than master went down, we did not fail over.
 * This is wrong. Think about following scenario. If someone
 * accidentally plugs out the network cable, the TCP/IP stack keeps
 * retrying for long time (typically 2 hours). The only way to stop
 * the retry is restarting the process.  Bottom line is, we need to
 * restart all children in any case.  See pgpool-general list posting
 * "TCP connections are *not* closed when a backend timeout" on Jul 13
 * 2008 for more details.
 */
#ifdef NOT_USED
	else
	{
		if (Req_info->master_node_id == new_master && *InRecovery == 0)
		{
			pool_log("failover_handler: do not restart pgpool. same master node %d was selected", new_master);
			if (Req_info->kind == NODE_UP_REQUEST)
			{
				pool_log("failback done. reconnect host %s(%d)",
						 BACKEND_INFO(node_id).backend_hostname,
						 BACKEND_INFO(node_id).backend_port);
			}
			else
			{
				pool_log("failover done. shutdown host %s(%d)",
						 BACKEND_INFO(node_id).backend_hostname,
						 BACKEND_INFO(node_id).backend_port);
			}

			/* exec failover_command */
			for (i = 0; i < pool_config->backend_desc->num_backends; i++)
			{
				if (nodes[i])
					trigger_failover_command(i, pool_config->failover_command);
			}

			pool_semaphore_unlock(REQUEST_INFO_SEM);
			switching = 0;
			Req_info->switching = false;
			kill(pcp_pid, SIGUSR2);
			switching = 0;
			Req_info->switching = false;
			return;
		}
	}
#endif


   /* On 2011/5/2 Tatsuo Ishii says: if mode is streaming replication
	* and request is NODE_UP_REQUEST(failback case) we don't need to
	* restart all children. Existing session will not use newly
	* attached node, but load balanced node is not changed util this
	* session ends, so it's not harmless anyway.
	*/
	if (MASTER_SLAVE && !strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP)	&&
		Req_info->kind == NODE_UP_REQUEST)
	{
		pool_log("Do not restart children because we are failbacking node id %d host%s port:%d and we are in streaming replication mode", node_id, 
				 BACKEND_INFO(node_id).backend_hostname,
				 BACKEND_INFO(node_id).backend_port);

		need_to_restart_children = false;
	}
	else
	{
		pool_log("Restart all children");

		/* kill all children */
		for (i = 0; i < pool_config->num_init_children; i++)
		{
			pid_t pid = process_info[i].pid;
			if (pid)
			{
				kill(pid, SIGQUIT);
				pool_debug("failover_handler: kill %d", pid);
			}
		}

		need_to_restart_children = true;
	}

	/* Exec failover_command if needed */
	for (i = 0; i < pool_config->backend_desc->num_backends; i++)
	{
		if (nodes[i])
			trigger_failover_command(i, pool_config->failover_command,
									 MASTER_NODE_ID, new_master, PRIMARY_NODE_ID);
	}

/* no need to wait since it will be done in reap_handler */
#ifdef NOT_USED
	while (wait(NULL) > 0)
		;

	if (errno != ECHILD)
		pool_error("failover_handler: wait() failed. reason:%s", strerror(errno));
#endif

	if (Req_info->kind == PROMOTE_NODE_REQUEST && VALID_BACKEND(node_id))
		new_primary = node_id;
	else
		new_primary =  find_primary_node_repeatedly();

	/* 
	 * If follow_master_command is provided and in master/slave
	 * streaming replication mode, we start degenerating all backends
	 * as they are not replicated anymore.
	 */
	int follow_cnt = 0;
	if (MASTER_SLAVE && !strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP))
	{
		if (*pool_config->follow_master_command != '\0' ||
			Req_info->kind == PROMOTE_NODE_REQUEST)
		{
			/* only if the failover is against the current primary */
			if (((Req_info->kind == NODE_DOWN_REQUEST) &&
				 (nodes[Req_info->primary_node_id])) ||
				((Req_info->kind == PROMOTE_NODE_REQUEST) &&
				 (VALID_BACKEND(node_id)))) {

				for (i = 0; i < pool_config->backend_desc->num_backends; i++)
				{
					/* do not degenerate the new primary */
					if ((new_primary >= 0) && (i != new_primary)) {
						BackendInfo *bkinfo;
						bkinfo = pool_get_node_info(i);
						pool_log("starting follow degeneration. shutdown host %s(%d)",
								 bkinfo->backend_hostname,
								 bkinfo->backend_port);
						bkinfo->backend_status = CON_DOWN;	/* set down status */
						follow_cnt++;
					}
				}

				if (follow_cnt == 0)
				{
					pool_log("failover: no follow backends are degenerated");
				}
				else
				{
					/* update new master node */
					new_master = get_next_master_node();
					pool_log("failover: %d follow backends have been degenerated", follow_cnt);
				}
			}
		}
	}

	memset(Req_info->node_id, -1, sizeof(int) * MAX_NUM_BACKENDS);
	pool_semaphore_unlock(REQUEST_INFO_SEM);

	/* exec follow_master_command */
	if ((follow_cnt > 0) && (*pool_config->follow_master_command != '\0'))
	{
		follow_pid = fork_follow_child(Req_info->master_node_id, new_master,
									   Req_info->primary_node_id);
	}

	/* Save primary node id */
	Req_info->primary_node_id = new_primary;
	pool_log("failover: set new primary node: %d", Req_info->primary_node_id);

	if (new_master >= 0)
	{
		Req_info->master_node_id = new_master;
		pool_log("failover: set new master node: %d", Req_info->master_node_id);
	}


	/* Fork the children if needed */
	if (need_to_restart_children)
	{
		for (i=0;i<pool_config->num_init_children;i++)
		{
			process_info[i].pid = fork_a_child(unix_fd, inet_fd, i);
			process_info[i].start_time = time(NULL);
		}
	}
	else
	{
		/* Set restart request to each child. Children will exit(1)
		 * whenever they are idle to restart.
		 */
		for (i=0;i<pool_config->num_init_children;i++)
		{
			process_info[i].need_to_restart = 1;
		}
	}

	/*
	 * Send restart request to worker child.
	 */
	kill(worker_pid, SIGUSR1);

	if (Req_info->kind == NODE_UP_REQUEST)
	{
		pool_log("failback done. reconnect host %s(%d)",
				 BACKEND_INFO(node_id).backend_hostname,
				 BACKEND_INFO(node_id).backend_port);
	}
	else if (Req_info->kind == PROMOTE_NODE_REQUEST)
	{
		pool_log("promotion done. promoted host %s(%d)",
				 BACKEND_INFO(node_id).backend_hostname,
				 BACKEND_INFO(node_id).backend_port);
	}
	else
	{
		pool_log("failover done. shutdown host %s(%d)",
				 BACKEND_INFO(node_id).backend_hostname,
				 BACKEND_INFO(node_id).backend_port);
	}

	switching = 0;
	Req_info->switching = false;

	/* kick wakeup_handler in pcp_child to notice that
	 * failover/failback done
	 */
	kill(pcp_pid, SIGUSR2);

	sleep(1);

	/*
	 * Send restart request to pcp child.
	 */
	kill(pcp_pid, SIGUSR1);
}

/*
 * health check timer handler
 */
static RETSIGTYPE health_check_timer_handler(int sig)
{
	POOL_SETMASK(&BlockSig);
	health_check_timer_expired = 1;
	POOL_SETMASK(&UnBlockSig);
}


/*
 * check if we can connect to the backend
 * returns 0 for ok. otherwise returns backend id + 1
 */
int health_check(void)
{
	int fd;
	int sts;
	static bool is_first = true;
	static char *dbname;

	/* V2 startup packet */
	typedef struct {
		int len;		/* startup packet length */
		StartupPacket_v2 sp;
	} MySp;
	MySp mysp;
	char kind;
	int i;

	/* Do not execute health check during recovery */
	if (*InRecovery)
		return 0;

 Retry:
	/*
	 * First we try with "postgres" database.
	 */
	if (is_first)
		dbname = "postgres";

	memset(&mysp, 0, sizeof(mysp));
	mysp.len = htonl(296);
	mysp.sp.protoVersion = htonl(PROTO_MAJOR_V2 << 16);
	strcpy(mysp.sp.database, dbname);
	strncpy(mysp.sp.user, pool_config->health_check_user, sizeof(mysp.sp.user) - 1);
	*mysp.sp.options = '\0';
	*mysp.sp.unused = '\0';
	*mysp.sp.tty = '\0';

	for (i=0;i<pool_config->backend_desc->num_backends;i++)
	{
		pool_debug("health_check: %d th DB node status: %d", i, BACKEND_INFO(i).backend_status);

		if (BACKEND_INFO(i).backend_status == CON_UNUSED ||
			BACKEND_INFO(i).backend_status == CON_DOWN)
			continue;

		if (*(BACKEND_INFO(i).backend_hostname) == '/')
			fd = connect_unix_domain_socket(i, FALSE);
		else
			fd = connect_inet_domain_socket(i, FALSE);

		if (fd < 0)
		{
			pool_error("health check failed. %d th host %s at port %d is down",
					   i,
					   BACKEND_INFO(i).backend_hostname,
					   BACKEND_INFO(i).backend_port);

			return i+1;
		}

		if (write(fd, &mysp, sizeof(mysp)) < 0)
		{
			pool_error("health check failed during write. host %s at port %d is down. reason: %s",
					   BACKEND_INFO(i).backend_hostname,
					   BACKEND_INFO(i).backend_port,
					   strerror(errno));
			close(fd);
			return i+1;
		}

		/*
		 * Don't bother to be blocked by read(2). It will be
		 * interrupted by ALRAM anyway.
		 */
		sts = read(fd, &kind, 1);
		if (sts == -1)
		{
			pool_error("health check failed during read. host %s at port %d is down. reason: %s",
					   BACKEND_INFO(i).backend_hostname,
					   BACKEND_INFO(i).backend_port,
					   strerror(errno));
			close(fd);
			return i+1;
		}
		else if (sts == 0)
		{
			pool_error("health check failed. EOF encountered. host %s at port %d is down",
					   BACKEND_INFO(i).backend_hostname,
					   BACKEND_INFO(i).backend_port);
			close(fd);
			return i+1;
		}

		if (is_first)
			is_first = false;

		/*
		 * If a backend raised a FATAL error(max connections error or
		 * starting up error?), do not send a Terminate message.
		 */
		if ((kind != 'E') && (write(fd, "X", 1) < 0))
		{
			if (!strcmp(dbname, "postgres"))
			{
				/*
				 * Retry with template1
				 */
				dbname = "template1";
				goto Retry;
			}

			pool_error("health check failed during write. host %s at port %d is down. reason: %s. Perhaps wrong health check user?",
					   BACKEND_INFO(i).backend_hostname,
					   BACKEND_INFO(i).backend_port,
					   strerror(errno));
			close(fd);
			return i+1;
		}

		close(fd);
	}

	return 0;
}

/*
 * check if we can connect to the SystemDB
 * returns 0 for ok. otherwise returns -1
 */
int
system_db_health_check(void)
{
	int fd;

	/* V2 startup packet */
	typedef struct {
		int len;		/* startup packet length */
		StartupPacket_v2 sp;
	} MySp;
	MySp mysp;
	char kind;

	memset(&mysp, 0, sizeof(mysp));
	mysp.len = htonl(296);
	mysp.sp.protoVersion = htonl(PROTO_MAJOR_V2 << 16);
	strcpy(mysp.sp.database, "template1");
	strncpy(mysp.sp.user, SYSDB_INFO->user, sizeof(mysp.sp.user) - 1);
	*mysp.sp.options = '\0';
	*mysp.sp.unused = '\0';
	*mysp.sp.tty = '\0';

	pool_debug("health_check: SystemDB status: %d", SYSDB_STATUS);

	/* if SystemDB is already down, ignore */
	if (SYSDB_STATUS == CON_UNUSED || SYSDB_STATUS == CON_DOWN)
		return 0;

	if (*SYSDB_INFO->hostname == '/')
		fd = connect_unix_domain_socket_by_port(SYSDB_INFO->port, SYSDB_INFO->hostname, FALSE);
	else
		fd = connect_inet_domain_socket_by_port(SYSDB_INFO->hostname, SYSDB_INFO->port, FALSE);

	if (fd < 0)
	{
		pool_error("health check failed. SystemDB host %s at port %d is down",
				   SYSDB_INFO->hostname,
				   SYSDB_INFO->port);

		return -1;
	}

	if (write(fd, &mysp, sizeof(mysp)) < 0)
	{
		pool_error("health check failed during write. SystemDB host %s at port %d is down",
				   SYSDB_INFO->hostname,
				   SYSDB_INFO->port);
		close(fd);
		return -1;
	}

	read(fd, &kind, 1);

	if (write(fd, "X", 1) < 0)
	{
		pool_error("health check failed during write. SystemDB host %s at port %d is down",
				   SYSDB_INFO->hostname,
				   SYSDB_INFO->port);
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

/*
 * handle SIGCHLD
 */
static RETSIGTYPE reap_handler(int sig)
{
	POOL_SETMASK(&BlockSig);
	sigchld_request = 1;
	write(pipe_fds[1], "\0", 1);
	POOL_SETMASK(&UnBlockSig);
}

/*
 * Attach zombie processes and restart child processes.
 * reaper() must be called protected from signals.
 */
static void reaper(void)
{
	pid_t pid;
	int status;
	int i;

	pool_debug("reap_handler called");

	if (exiting)
	{
		pool_debug("reap_handler: exited due to exiting");
		return;
	}

	if (switching)
	{
		pool_debug("reap_handler: exited due to switching");
		return;
	}

	/* clear SIGCHLD request */
	sigchld_request = 0;

#ifdef HAVE_WAITPID
	pool_debug("reap_handler: call waitpid");
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
#else
	pool_debug("reap_handler: call wait3");
	while ((pid = wait3(&status, WNOHANG, NULL)) > 0)
#endif
	{
		if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
		{
			/* Child terminated by segmentation fault. Report it */
			pool_error("Child process %d was terminated by segmentation fault", pid);
		}
			
		/* if exiting child process was PCP handler */
		if (pid == pcp_pid)
		{
			if (WIFSIGNALED(status))
				pool_log("PCP child %d exits with status %d by signal %d", pid, status, WTERMSIG(status));
			else
				pool_log("PCP child %d exits with status %d", pid, status);

			pcp_pid = pcp_fork_a_child(pcp_unix_fd, pcp_inet_fd, pcp_conf_file);
			pool_log("fork a new PCP child pid %d", pcp_pid);
			break;
		}

		/* exiting process was worker process */
		else if (pid == worker_pid)
		{
			if (WIFSIGNALED(status))
				pool_log("worker child %d exits with status %d by signal %d", pid, status, WTERMSIG(status));
			else
				pool_log("worker child %d exits with status %d", pid, status);

			if (status)
				worker_pid = worker_fork_a_child();

			pool_log("fork a new worker child pid %d", worker_pid);
			break;
		} else
		{
			if (WIFSIGNALED(status))
				pool_debug("child %d exits with status %d by signal %d", pid, status, WTERMSIG(status));
			else
				pool_debug("child %d exits with status %d", pid, status);

			/* look for exiting child's pid */
			for (i=0;i<pool_config->num_init_children;i++)
			{
				if (pid == process_info[i].pid)
				{
					/* if found, fork a new child */
					if (!switching && !exiting && status)
					{
						process_info[i].pid = fork_a_child(unix_fd, inet_fd, i);
						process_info[i].start_time = time(NULL);
						pool_debug("fork a new child pid %d", process_info[i].pid);
						break;
					}
				}
			}
		}
	}
	pool_debug("reap_handler: normally exited");
}

/*
 * get node information specified by node_number
 */
BackendInfo *
pool_get_node_info(int node_number)
{
	if (node_number < 0 || node_number >= NUM_BACKENDS)
		return NULL;

	return &BACKEND_INFO(node_number);
}

/*
 * get number of nodes
 */
int
pool_get_node_count(void)
{
	return NUM_BACKENDS;
}

/*
 * get process ids
 */
int *
pool_get_process_list(int *array_size)
{
	int	   *array;
	int		i;

	*array_size = pool_config->num_init_children;
	array = calloc(*array_size, sizeof(int));
	for (i = 0; i < *array_size; i++)
		array[i] = process_info[i].pid;

	return array;
}

/*
 * get process information specified by pid
 */
ProcessInfo *
pool_get_process_info(pid_t pid)
{
	int		i;

	for (i = 0; i < pool_config->num_init_children; i++)
		if (process_info[i].pid == pid)
			return &process_info[i];

	return NULL;
}

/*
 * get System DB information
 */
SystemDBInfo *
pool_get_system_db_info(void)
{
	if (system_db_info == NULL)
		return NULL;

	return system_db_info->info;
}


/*
 * handle SIGUSR2
 * Wakeup all processes
 */
static void wakeup_children(void)
{
	kill_all_children(SIGUSR2);
}


static RETSIGTYPE wakeup_handler(int sig)
{
	POOL_SETMASK(&BlockSig);
	wakeup_request = 1;
	write(pipe_fds[1], "\0", 1);
	POOL_SETMASK(&UnBlockSig);
}

/*
 * handle SIGHUP
 *
 */
static RETSIGTYPE reload_config_handler(int sig)
{
	POOL_SETMASK(&BlockSig);
	reload_config_request = 1;
	write(pipe_fds[1], "\0", 1);
	POOL_SETMASK(&UnBlockSig);
}

static void reload_config(void)
{
	pool_log("reload config files.");
	pool_get_config(conf_file, RELOAD_CONFIG);
	if (pool_config->enable_pool_hba)
		load_hba(hba_file);
	if (pool_config->parallel_mode)
		pool_memset_system_db_info(system_db_info->info);
	kill_all_children(SIGHUP);

	if (worker_pid)
		kill(worker_pid, SIGHUP);
}

static void kill_all_children(int sig)
{
	int i;

	/* kill all children */
	for (i = 0; i < pool_config->num_init_children; i++)
	{
		pid_t pid = process_info[i].pid;
		if (pid)
		{
			kill(pid, sig);
		}
	}

	/* make PCP process reload as well */
	if (sig == SIGHUP)
		kill(pcp_pid, sig);
}

/*
 * pause in a period specified by timeout. If any data is coming
 * through pipe_fds[0], that means one of: failover request(SIGUSR1),
 * SIGCHLD received, children wake up request(SIGUSR2 used in on line
 * recovery processing) or config file reload request(SIGHUP) has been
 * occurred.  In this case this function returns 1.
 * otherwise 0: (no signal event occurred), -1: (error)
 * XXX: is it ok that select(2) error is ignored here?
 */
static int pool_pause(struct timeval *timeout)
{
	fd_set rfds;
	int n;
	char dummy;

	FD_ZERO(&rfds);
	FD_SET(pipe_fds[0], &rfds);
	n = select(pipe_fds[0]+1, &rfds, NULL, NULL, timeout);
	if (n == 1)
		read(pipe_fds[0], &dummy, 1);
	return n;
}

/*
 * sleep for seconds specified by "second".  Unlike pool_pause(), this
 * function guarantees that it will sleep for specified seconds.  This
 * function uses pool_pause() internally. If it informs that there is
 * a pending signal event, they are processed using CHECK_REQUEST
 * macro. Note that most of these processes are done while all signals
 * are blocked.
 */
void pool_sleep(unsigned int second)
{
	struct timeval current_time, sleep_time;

	gettimeofday(&current_time, NULL);
	sleep_time.tv_sec = second + current_time.tv_sec;
	sleep_time.tv_usec = current_time.tv_usec;

	POOL_SETMASK(&UnBlockSig);
	while (sleep_time.tv_sec > current_time.tv_sec)
	{
		struct timeval timeout;
		int r;

		timeout.tv_sec = sleep_time.tv_sec - current_time.tv_sec;
		timeout.tv_usec = sleep_time.tv_usec - current_time.tv_usec;
		if (timeout.tv_usec < 0)
		{
			timeout.tv_sec--;
			timeout.tv_usec += 1000000;
		}

		r = pool_pause(&timeout);
		POOL_SETMASK(&BlockSig);
		if (r > 0)
			CHECK_REQUEST;
		POOL_SETMASK(&UnBlockSig);
		gettimeofday(&current_time, NULL);
	}
	POOL_SETMASK(&BlockSig);
}

/*
 * get_config_file_name: return full path of pgpool.conf.
 */
char *get_config_file_name(void)
{
	return conf_file;
}

/*
 * get_config_file_name: return full path of pool_hba.conf.
 */
char *get_hba_file_name(void)
{
	return hba_file;
}

/*
 * trigger_failover_command: execute specified command at failover.
 *                           command_line is null-terminated string.
 */
static int trigger_failover_command(int node, const char *command_line,
									int old_master, int new_master, int old_primary)
{
	int r = 0;
	String *exec_cmd;
	char port_buf[6];
	char buf[2];
	BackendInfo *info;
	BackendInfo *newmaster;

	if (command_line == NULL || (strlen(command_line) == 0))
		return 0;

	/* check nodeID */
	if (node < 0 || node > NUM_BACKENDS)
		return -1;

	info = pool_get_node_info(node);
	if (!info)
		return -1;

	buf[1] = '\0';
	pool_memory = pool_memory_create(PREPARE_BLOCK_SIZE);
	if (!pool_memory)
	{
		pool_error("trigger_failover_command: pool_memory_create() failed");
		return -1;
	}
	exec_cmd = init_string("");

	while (*command_line)
	{
		if (*command_line == '%')
		{
			if (*(command_line + 1))
			{
				char val = *(command_line + 1);
				switch (val)
				{
					case 'p': /* failed node port */
						snprintf(port_buf, sizeof(port_buf), "%d", info->backend_port);
						string_append_char(exec_cmd, port_buf);
						break;

					case 'D': /* failed node database directory */
						string_append_char(exec_cmd, info->backend_data_directory);
						break;

					case 'd': /* failed node id */
						snprintf(port_buf, sizeof(port_buf), "%d", node);
						string_append_char(exec_cmd, port_buf);
						break;

					case 'h': /* failed host name */
						string_append_char(exec_cmd, info->backend_hostname);
						break;

					case 'H': /* new master host name */
						newmaster = pool_get_node_info(get_next_master_node());
						if (newmaster)
							string_append_char(exec_cmd, newmaster->backend_hostname);
						else
							/* no vaid new master */
							string_append_char(exec_cmd, "");
						break;

					case 'm': /* new master node id */
						snprintf(port_buf, sizeof(port_buf), "%d", new_master);
						string_append_char(exec_cmd, port_buf);
						break;

					case 'M': /* old master node id */
						snprintf(port_buf, sizeof(port_buf), "%d", old_master);
						string_append_char(exec_cmd, port_buf);
						break;

					case 'P': /* old primary node id */
						snprintf(port_buf, sizeof(port_buf), "%d", old_primary);
						string_append_char(exec_cmd, port_buf);
						break;

					case '%': /* escape */
						string_append_char(exec_cmd, "%");
						break;

					default: /* ignore */
						break;
				}
				command_line++;
			}
		} else {
			buf[0] = *command_line;
			string_append_char(exec_cmd, buf);
		}
		command_line++;
	}

	if (strlen(exec_cmd->data) != 0)
	{
		pool_log("execute command: %s", exec_cmd->data);
		r = system(exec_cmd->data);
	}

	pool_memory_delete(pool_memory, 0);
	pool_memory = NULL;

	return r;
}

/*
 * Find the primary node (i.e. not standby node) and returns its node
 * id. If no primary node is found, returns -1.
 */
static int find_primary_node(void)
{
	BackendInfo *bkinfo;
	POOL_CONNECTION_POOL_SLOT *s;
	POOL_CONNECTION *con; 
	POOL_STATUS status;
	POOL_SELECT_RESULT *res;
	bool is_standby;
	int i;

	/* Streaming replication mode? */
	if (pool_config->master_slave_mode == 0 ||
		strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP))
	{
		/* No point to look for primary node if not in streaming
		 * replication mode.
		 */
		pool_debug("find_primary_node: not in streaming replication mode");
		return -1;
	}

	for(i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		/*
		 * Check to see if this is a standby node or not.
		 */
		is_standby = false;

		bkinfo = pool_get_node_info(i);
		s = make_persistent_db_connection(bkinfo->backend_hostname, 
										  bkinfo->backend_port,
										  "postgres",
										  pool_config->sr_check_user,
										  pool_config->sr_check_password);
		if (!s)
		{
			pool_error("find_primary_node: make_persistent_connection failed");
			return -1;
		}
		con = s->con;
#ifdef NOT_USED
		status = do_query(con, "SELECT count(*) FROM pg_catalog.pg_proc AS p WHERE p.proname = 'pgpool_walrecrunning'",
						  &res, PROTO_MAJOR_V3);
		if (res->numrows <= 0)
		{
			pool_log("find_primary_node: do_query returns no rows");
		}
		if (res->data[0] == NULL)
		{
			pool_log("find_primary_node: do_query returns no data");
		}
		if (res->nullflags[0] == -1)
		{
			pool_log("find_primary_node: do_query returns NULL");
		}
		if (res->data[0] && !strcmp(res->data[0], "0"))
		{
			pool_log("find_primary_node: pgpool_walrecrunning does not exist");
			free_select_result(res);
			discard_persistent_db_connection(s);
			return -1;
		}
#endif
		status = do_query(con, "SELECT pg_is_in_recovery()",
						  &res, PROTO_MAJOR_V3);
		if (res->numrows <= 0)
		{
			pool_log("find_primary_node: do_query returns no rows");
		}
		if (res->data[0] == NULL)
		{
			pool_log("find_primary_node: do_query returns no data");
		}
		if (res->nullflags[0] == -1)
		{
			pool_log("find_primary_node: do_query returns NULL");
		}
		if (res->data[0] && !strcmp(res->data[0], "t"))
		{
			is_standby = true;
		}   
		free_select_result(res);
		discard_persistent_db_connection(s);

		/*
		 * If this is a standby, we continue to look for primary node.
		 */
		if (is_standby)
		{
			pool_debug("find_primary_node: %d node is standby", i);
		}
		else
		{
			break;
		}
	}

	if (i == NUM_BACKENDS)
	{
		pool_debug("find_primary_node: no primary node found");
		return -1;
	}

	pool_log("find_primary_node: primary node id is %d", i);
	return i;
}

static int find_primary_node_repeatedly(void)
{
	int sec;
	int node_id = -1;

	/* Streaming replication mode? */
	if (pool_config->master_slave_mode == 0 ||
		strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP))
	{
		/* No point to look for primary node if not in streaming
		 * replication mode.
		 */
		pool_debug("find_primary_node: not in streaming replication mode");
		return -1;
	}

	pool_log("find_primary_node_repeatedly: waiting for finding a primary node");
	for (sec = 0; sec < pool_config->recovery_timeout; sec++)
	{
		node_id = find_primary_node();
		if (node_id != -1)
			break;
		pool_sleep(1);
	}
	return node_id;
}

/*
* fork a follow child
*/
pid_t fork_follow_child(int old_master, int new_master, int old_primary)
{
	pid_t pid;
	int i;

	pid = fork();

	if (pid == 0)
	{
		pool_log("start triggering follow command.");
		for (i = 0; i < pool_config->backend_desc->num_backends; i++)
		{
			BackendInfo *bkinfo;
			bkinfo = pool_get_node_info(i);
			if (bkinfo->backend_status == CON_DOWN)
				trigger_failover_command(i, pool_config->follow_master_command,
										 old_master, new_master, old_primary);
		}
		exit(0);
	}
	else if (pid == -1)
	{
		pool_error("follow fork() failed. reason: %s", strerror(errno));
		exit(1);
	}
	return pid;
}
