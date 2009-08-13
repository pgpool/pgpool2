/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2009	PgPool Global Development Group
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
#endif

#include "version.h"
#include "parser/pool_memory.h"
#include "parser/pool_string.h"

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
static pid_t pcp_fork_a_child(int unix_fd, int inet_fd, char *pcp_conf_file);
static pid_t fork_a_child(int unix_fd, int inet_fd, int id);
static int create_unix_domain_socket(struct sockaddr_un un_addr_tmp);
static int create_inet_domain_socket(const char *hostname, const int port);
static void myexit(int code);
static void failover(void);
static void reaper(void);
static void wakeup_children(void);
static void reload_config(void);
static int pool_pause(struct timeval *timeout);
static void pool_sleep(unsigned int second);
static void kill_all_children(int sig);
static int get_next_master_node(void);

static RETSIGTYPE exit_handler(int sig);
static RETSIGTYPE reap_handler(int sig);
static RETSIGTYPE failover_handler(int sig);
static RETSIGTYPE reload_config_handler(int sig);
static RETSIGTYPE health_check_timer_handler(int sig);
static RETSIGTYPE wakeup_handler(int sig);

static void usage(void);
static void show_version(void);
static void stop_me(void);

static int trigger_failover_command(int node, const char *command_line);

static struct sockaddr_un un_addr;		/* unix domain socket path */
static struct sockaddr_un pcp_un_addr;  /* unix domain socket path for PCP */

ProcessInfo *pids;	/* shmem child pid table */

/*
 * shmem connection info table
 * this is a two dimension array. i.e.:
 * con_info[pool_config->num_init_children][pool_config->max_pool]
 */
ConnectionInfo *con_info;		

static int unix_fd;	/* unix domain socket fd */
static int inet_fd;	/* inet domain socket fd */

static int pcp_pid; /* pid for child process handling PCP */
static int pcp_unix_fd; /* unix domain socket fd for PCP (not used) */
static int pcp_inet_fd; /* inet domain socket fd for PCP */
static char pcp_conf_file[POOLMAXPATHLEN+1]; /* path for pcp.conf */
static char conf_file[POOLMAXPATHLEN+1];
static char hba_file[POOLMAXPATHLEN+1];

static int exiting = 0;		/* non 0 if I'm exiting */
static int switching = 0;		/* non 0 if I'm fail overing or degenerating */

#ifdef NOT_USED
static int degenerated = 0;	/* set non 0 if already degenerated */
#endif

static int clear_cache = 0;		/* non 0 if clear chache option (-c) is given */
static int not_detach = 0;		/* non 0 if non detach option (-n) is given */
int debug = 0;	/* non 0 if debug option is given (-d) */

pid_t mypid;	/* pgpool parent process id */

long int weight_master;	/* normalized weight of master (0-RAND_MAX range) */

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

	myargc = argc;
	myargv = argv;

	snprintf(conf_file, sizeof(conf_file), "%s/%s", DEFAULT_CONFIGDIR, POOL_CONF_FILE_NAME);
	snprintf(pcp_conf_file, sizeof(pcp_conf_file), "%s/%s", DEFAULT_CONFIGDIR, PCP_PASSWD_FILE_NAME);
	snprintf(hba_file, sizeof(hba_file), "%s/%s", DEFAULT_CONFIGDIR, HBA_CONF_FILE_NAME);

	while ((opt = getopt(argc, argv, "a:cdf:F:hm:nv")) != -1)
	{
		switch (opt)
		{
			case 'a':    /* specify hba configuration file */
				if (!optarg)
				{
					usage();
					exit(1);
				}
				strncpy(hba_file, optarg, sizeof(hba_file));
				break;

			case 'c':			/* clear cache option */
				clear_cache = 1;
				break;

			case 'd':	/* debug option */
				debug = 1;
				break;

			case 'f':	/* specify configuration file */
				if (!optarg)
				{
					usage();
					exit(1);
				}
				strncpy(conf_file, optarg, sizeof(conf_file));
				break;

			case 'F':   /* specify PCP password file */
				if (!optarg)
				{
					usage();
					exit(1);
				}
				strncpy(pcp_conf_file, optarg, sizeof(pcp_conf_file));
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

			case 'v':
				show_version();
				exit(0);

			default:
				usage();
				exit(1);
		}
	}

	mypid = getpid();

	if (pool_init_config())
		exit(1);

	if (pool_get_config(conf_file, INIT_CONFIG))
	{
		pool_error("Unable to get configuration. Exiting...");
		exit(1);
	}

	if (pool_config->enable_pool_hba)
		load_hba(hba_file);

	/*
	 * if a non-switch argument remains, then it should be either "reload", "stop" or "switch"
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

	if (pool_semaphore_create(MAX_NUM_SEMAPHORES))
	{
		pool_error("Unable to create semaphores. Exiting...");
		pool_shmem_exit(1);
		exit(1);
	}

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

	size = pool_config->num_init_children * pool_config->max_pool * sizeof(ConnectionInfo);
	con_info = pool_shared_memory_create(size);
	if (con_info == NULL)
	{
		pool_error("failed to allocate connection informations");
		myexit(1);
	}
	memset(con_info, 0, size);

	size = pool_config->num_init_children * (sizeof(ProcessInfo));
	pids = pool_shared_memory_create(size);
	if (pids == NULL)
	{
		pool_error("failed to allocate pids");
		myexit(1);
	}
	memset(pids, 0, size);
	for (i = 0; i < pool_config->num_init_children; i++)
	{
		pids[i].connection_info = &con_info[i * pool_config->max_pool];
	}

	/* create fail over/switch over event area */
	Req_info = pool_shared_memory_create(sizeof(POOL_REQUEST_INFO));
	if (Req_info == NULL)
	{
		pool_error("failed to allocate Req_info");
		myexit(1);
	}

	/* initialize Req_info */
	Req_info->kind = NODE_UP_REQUEST;
	memset(Req_info->node_id, -1, sizeof(int) * MAX_NUM_BACKENDS);
	Req_info->master_node_id = get_next_master_node();
	Req_info->conn_counter = 0;

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
		pids[i].pid = fork_a_child(unix_fd, inet_fd, i);
		pids[i].start_time = time(NULL);
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

	pool_log("pgpool successfully started");

	/* fork a child for PCP handling */
	snprintf(pcp_un_addr.sun_path, sizeof(pcp_un_addr.sun_path), "%s/.s.PGSQL.%d",
			 pool_config->pcp_socket_dir,
			 pool_config->pcp_port);
	pcp_unix_fd = create_unix_domain_socket(pcp_un_addr);
    /* maybe change "*" to pool_config->pcp_listen_addresses */
	pcp_inet_fd = create_inet_domain_socket("*", pool_config->pcp_port);
	pcp_pid = pcp_fork_a_child(pcp_unix_fd, pcp_inet_fd, pcp_conf_file);

	retrycnt = 0;		/* reset health check retry counter */
	sys_retrycnt = 0;	/* reset SystemDB health check retry counter */

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
						pool_log("set %d th backend down status", sts);
						Req_info->kind = NODE_DOWN_REQUEST;
						Req_info->node_id[0] = sts;
						failover();
						/* need to distribute this info to children */
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
	fprintf(stderr, "  a generic connection pool/replication/load balance server for PostgreSQL\n\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  pgpool [ -c] [ -f CONFIG_FILE ] [ -F PCP_CONFIG_FILE ] [ -a HBA_CONFIG_FILE ]\n");
	fprintf(stderr, "         [ -n ] [ -d ]\n");
	fprintf(stderr, "  pgpool [ -f CONFIG_FILE ] [ -F PCP_CONFIG_FILE ] [ -a HBA_CONFIG_FILE ]\n");
	fprintf(stderr, "         [ -m SHUTDOWN-MODE ] stop\n");
	fprintf(stderr, "  pgpool [ -f CONFIG_FILE ] [ -F PCP_CONFIG_FILE ] [ -a HBA_CONFIG_FILE ] reload\n\n");
	fprintf(stderr, "Common options:\n");
	fprintf(stderr, "  -a HBA_CONFIG_FILE  Sets the path to the pool_hba.conf configuration file\n");
	fprintf(stderr, "                      (default: %s/%s)\n",DEFAULT_CONFIGDIR, HBA_CONF_FILE_NAME);
	fprintf(stderr, "  -f CONFIG_FILE      Sets the path to the pgpool.conf configuration file\n");
	fprintf(stderr, "                      (default: %s/%s)\n",DEFAULT_CONFIGDIR, POOL_CONF_FILE_NAME);
	fprintf(stderr, "  -F PCP_CONFIG_FILE  Sets the path to the pcp.conf configuration file\n");
	fprintf(stderr, "                      (default: %s/%s)\n",DEFAULT_CONFIGDIR, PCP_PASSWD_FILE_NAME);
	fprintf(stderr, "  -h                  Prints this help\n\n");
	fprintf(stderr, "Start options:\n");
	fprintf(stderr, "  -c                  Clears query cache (enable_query_cache must be on)\n");
	fprintf(stderr, "  -n                  Don't run in daemon mode, does not detach control tty\n");
	fprintf(stderr, "  -d                  Debug mode\n\n");
	fprintf(stderr, "Stop options:\n");
	fprintf(stderr, "  -m SHUTDOWN-MODE    Can be \"smart\", \"fast\", or \"immediate\"\n\n");
	fprintf(stderr, "Shutdown modes are:\n");
	fprintf(stderr, "  smart       quit after all clients have disconnected\n");
	fprintf(stderr, "  fast        quit directly, with proper shutdown\n");
	fprintf(stderr, "  immediate   quit without complete shutdown; will lead to recovery on restart\n");
}

/*
* detach control ttys
*/
static void daemonize(void)
{
	int			i;
	pid_t		pid;
	int			fdlimit;

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

	chdir("/");

	i = open("/dev/null", O_RDWR);
	dup2(i, 0);
	dup2(i, 1);
	dup2(i, 2);

    fdlimit = sysconf(_SC_OPEN_MAX);
    for (i = 3; i < fdlimit; i++)
		close(i);

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
	fwrite(pidbuf, strlen(pidbuf), 1, fd);
	if (fclose(fd))
	{
		pool_error("could not write pid file as %s. reason: %s",
				   pool_config->pid_file_name, strerror(errno));
		pool_shmem_exit(1);
		exit(1);
	}
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
* create inet domain socket
*/
static int create_inet_domain_socket(const char *hostname, const int port)
{
	struct sockaddr_in addr;
	int fd;
	int status;
	int one = 1;
	int len;

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
	((struct sockaddr *)&addr)->sa_family = AF_INET;

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

	status = listen(fd, PGPOOLMAXLITSENQUEUELENGTH);
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
	((struct sockaddr *)&addr)->sa_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), un_addr_tmp.sun_path);
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

	if (pids != NULL) {
		POOL_SETMASK(&AuthBlockSig);
		exiting = 1;
		for (i = 0; i < pool_config->num_init_children; i++)
		{
			pid_t pid = pids[i].pid;
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
			pool_log("notice_backend_error: node %d is not valid backend.", i);
			continue;
		}

		pool_log("notice_backend_error: %d fail over request from pid %d", node_id_set[i], getpid());
		Req_info->node_id[i] = node_id_set[i];
	}
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

	if (node_id < 0 || node_id >= MAX_NUM_BACKENDS || VALID_BACKEND(node_id))
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
		pid_t pid = pids[i].pid;
		if (pid)
		{
			kill(pid, sig);
		}
	}

	kill(pcp_pid, sig);

	POOL_SETMASK(&UnBlockSig);

	while (wait(NULL) > 0)
		;

	if (errno != ECHILD)
		pool_error("wait() failed. reason:%s", strerror(errno));

	pids = NULL;
	myexit(0);
}

/*
 * calculate next master node id
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
			if (BACKEND_INFO(i).backend_status == CON_CONNECT_WAIT)
				break;
		}
		else if (VALID_BACKEND(i))
			break;
	}
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
	int nodes[MAX_NUM_BACKENDS];

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
	node_id = Req_info->node_id[0];

	/* failback request? */
	if (Req_info->kind == NODE_UP_REQUEST)
	{
		if (node_id >= MAX_NUM_BACKENDS ||
			(Req_info->kind == NODE_UP_REQUEST && VALID_BACKEND(node_id)) ||
			(Req_info->kind == NODE_DOWN_REQUEST && !VALID_BACKEND(node_id)))
		{
			pool_semaphore_unlock(REQUEST_INFO_SEM);
			pool_error("failover_handler: invalid node_id %d status:%d MAX_NUM_BACKENDS: %d", node_id,
					   BACKEND_INFO(node_id).backend_status, MAX_NUM_BACKENDS);
			kill(pcp_pid, SIGUSR2);
			switching = 0;
			return;
		}

		pool_log("starting fail back. reconnect host %s(%d)",
				 BACKEND_INFO(node_id).backend_hostname,
				 BACKEND_INFO(node_id).backend_port);
		BACKEND_INFO(node_id).backend_status = CON_CONNECT_WAIT;	/* unset down status */
		trigger_failover_command(node_id, pool_config->failback_command);
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
			return;
		}
	}

	new_master = get_next_master_node();

	if (new_master == pool_config->backend_desc->num_backends)
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
			kill(pcp_pid, SIGUSR2);
			switching = 0;
			return;
		}
	}
#endif
	/* kill all children */
	for (i = 0; i < pool_config->num_init_children; i++)
	{
		pid_t pid = pids[i].pid;
		if (pid)
		{
			kill(pid, SIGQUIT);
			pool_debug("failover_handler: kill %d", pid);
		}
	}

	/* exec failover_command */
	for (i = 0; i < pool_config->backend_desc->num_backends; i++)
	{
		if (nodes[i])
			trigger_failover_command(i, pool_config->failover_command);
	}

	pool_log("failover_handler: set new master node: %d", new_master);
	Req_info->master_node_id = new_master;

/* no need to wait since it will be done in reap_handler */
#ifdef NOT_USED
	while (wait(NULL) > 0)
		;

	if (errno != ECHILD)
		pool_error("failover_handler: wait() failed. reason:%s", strerror(errno));
#endif

	memset(Req_info->node_id, -1, sizeof(int) * MAX_NUM_BACKENDS);
	pool_semaphore_unlock(REQUEST_INFO_SEM);

	/* fork the children */
	for (i=0;i<pool_config->num_init_children;i++)
	{
		pids[i].pid = fork_a_child(unix_fd, inet_fd, i);
		pids[i].start_time = time(NULL);
	}

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

	switching = 0;

	/* kick wakeup_handler in pcp_child to notice that
	 * faiover/failback done
	 */
	kill(pcp_pid, SIGUSR2);
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

	/* V2 startup packet */
	typedef struct {
		int len;		/* startup packet length */
		StartupPacket_v2 sp;
	} MySp;
	MySp mysp;
	char kind;
	int i;

	if (*InRecovery)
		return 0;

	memset(&mysp, 0, sizeof(mysp));
	mysp.len = htonl(296);
	mysp.sp.protoVersion = htonl(PROTO_MAJOR_V2 << 16);
	strcpy(mysp.sp.database, "template1");
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

		if (*(BACKEND_INFO(i).backend_hostname) == '\0')
			fd = connect_unix_domain_socket(i);
		else
			fd = connect_inet_domain_socket(i);

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

		/*
		 * If a backend raised a FATAL error(max connections error or
		 * starting up error?), do not send a Terminate message.
		 */
		if ((kind != 'E') && (write(fd, "X", 1) < 0))
		{
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
	
	if (*SYSDB_INFO->hostname == '\0')
		fd = connect_unix_domain_socket_by_port(SYSDB_INFO->port, pool_config->backend_socket_dir);
	else
		fd = connect_inet_domain_socket_by_port(SYSDB_INFO->hostname, SYSDB_INFO->port);

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
 * reaper() must be called under protecting signals.
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
		/* if exiting child process was PCP handler */
		if (pid == pcp_pid)
		{
			pool_debug("PCP child %d exits with status %d by signal %d", pid, status, WTERMSIG(status));

			pcp_pid = pcp_fork_a_child(pcp_unix_fd, pcp_inet_fd, pcp_conf_file);
			pool_debug("fork a new PCP child pid %d", pcp_pid);
			break;
		} else {
			pool_debug("child %d exits with status %d by signal %d", pid, status, WTERMSIG(status));
		
			/* look for exiting child's pid */
			for (i=0;i<pool_config->num_init_children;i++)
			{
				if (pid == pids[i].pid)
				{
					/* if found, fork a new child */
					if (!switching && !exiting && status)
					{
						pids[i].pid = fork_a_child(unix_fd, inet_fd, i);
						pids[i].start_time = time(NULL);
						pool_debug("fork a new child pid %d", pids[i].pid);
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
	if (node_number >= NUM_BACKENDS)
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
		array[i] = pids[i].pid;

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
		if (pids[i].pid == pid)
			return &pids[i];

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
}

static void kill_all_children(int sig)
{
	int i;

	/* kill all children */
	for (i = 0; i < pool_config->num_init_children; i++)
	{
		pid_t pid = pids[i].pid;
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
static void pool_sleep(unsigned int second)
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
static int trigger_failover_command(int node, const char *command_line)
{
	int r = 0;
	String *exec_cmd;
	char port_buf[6];
	char buf[2];
	BackendInfo *info;

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
					case 'p': /* port */
						snprintf(port_buf, sizeof(port_buf), "%d", info->backend_port);
						string_append_char(exec_cmd, port_buf);
						break;

					case 'D': /* database directory */
						string_append_char(exec_cmd, info->backend_data_directory);
						break;

					case 'd': /* node id */
						snprintf(port_buf, sizeof(port_buf), "%d", node);
						string_append_char(exec_cmd, port_buf);
						break;

					case 'h': /* host name */
						string_append_char(exec_cmd, info->backend_hostname);
						break;

					case 'm': /* new master node id */
						snprintf(port_buf, sizeof(port_buf), "%d", get_next_master_node());
						string_append_char(exec_cmd, port_buf);
						break;

					case 'M': /* old master node id */
						snprintf(port_buf, sizeof(port_buf), "%d", MASTER_NODE_ID);
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
