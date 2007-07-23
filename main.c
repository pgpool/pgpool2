/* -*-pgsql-c-*- */
/*
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

static RETSIGTYPE exit_handler(int sig);
static RETSIGTYPE reap_handler(int sig);
static RETSIGTYPE failover_handler(int sig);
static RETSIGTYPE reload_config_handler(int sig);
static RETSIGTYPE health_check_timer_handler(int sig);
static RETSIGTYPE wakeup_handler(int sig);

static void usage(void);
static void stop_me(void);

static struct sockaddr_un un_addr;		/* unix domain socket path */
static struct sockaddr_un pcp_un_addr;  /* unix domain socket path for PCP */

ProcessInfo *pids;	/* child pid table */

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
static int degenerated = 0;	/* set non 0 if already degerated */
#endif

static int clear_cache = 0;		/* non 0 if clear chache option (-c) is given */
static int not_detach = 0;		/* non 0 if non detach option (-n) is given */
int debug = 0;	/* non 0 if debug option is given (-d) */

pid_t mypid;	/* pgpool parent process id */

long int weight_master;	/* normalized weight of master (0-RAND_MAX range) */

static int stop_sig = SIGTERM;	/* stopping signal default value */

static int health_check_timer_expired;		/* non 0 if health check timer expired */

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
	ConnectionInfo *con_info = NULL;
	int retrycnt;
	int sys_retrycnt;

	myargc = argc;
	myargv = argv;

	snprintf(conf_file, sizeof(conf_file), "%s/%s", DEFAULT_CONFIGDIR, POOL_CONF_FILE_NAME);
	snprintf(pcp_conf_file, sizeof(pcp_conf_file), "%s/%s", DEFAULT_CONFIGDIR, PCP_PASSWD_FILE_NAME);
	snprintf(hba_file, sizeof(hba_file), "%s/%s", DEFAULT_CONFIGDIR, HBA_CONF_FILE_NAME);

	while ((opt = getopt(argc, argv, "a:cdf:F:hm:n")) != -1)
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
					pool_error("could not stop pid: %d. reason: %s", pid, strerror(errno));
					pool_shmem_exit(1);
					exit(1);
				}
				pool_log("xxx %d", pid);
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
		pool_error("Unable to create semaphoes. Exiting...");
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
	Req_info->node_id = -1;
	Req_info->master_node_id = 0;
	Req_info->conn_counter = 0;

	InRecovery = pool_shared_memory_create(sizeof(int));
	if (InRecovery == NULL)
	{
		pool_error("failed to allocate InRecovery");
		myexit(1);
	}
	*InRecovery = 0;

	/* fork the children */
	for (i=0;i<pool_config->num_init_children;i++)
	{
		pids[i].pid = fork_a_child(unix_fd, inet_fd, i);
		pids[i].start_time = time(NULL);
	}

	/* set up signal handlers */
	POOL_SETMASK(&BlockSig);
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
				pool_log("starting health checking");
			}
			else
			{
				pool_log("retrying %d th health checking", retrycnt);
			}

			if (pool_config->health_check_timeout > 0)
			{
				/*
				 * set health checker timeout. we want to detect
				 * commnuication path failure much earlier before
				 * TCP/IP statck detects it.
				 */
				pool_signal(SIGALRM, health_check_timer_handler);
				alarm(pool_config->health_check_timeout);
			}

			/*
			 * do actual health check. trying to connect to the backend
			 */
			errno = 0;
			health_check_timer_expired = 0;
			sts = health_check();
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
						Req_info->node_id = sts;
						failover();
						/* need to distribute this info to children */
					}
					else
					{
						retrycnt++;
						pool_signal(SIGALRM, SIG_IGN);

						if (retrycnt > NUM_BACKENDS)
						{
							/* retry count over */
							pool_log("set %d th backend down status", sts);
							Req_info->kind = NODE_DOWN_REQUEST;
							Req_info->node_id = sts;
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

				POOL_SETMASK(&UnBlockSig);
				r = pool_pause(NULL);
				POOL_SETMASK(&BlockSig);
				if (r > 0)
					break;
			}
		}
	}

	pool_shmem_exit(0);
}

static void usage(void)
{
	fprintf(stderr, "%s version %s(%s),\n",	PACKAGE, VERSION, PGPOOLVERSION);
	fprintf(stderr, "  a generic connection pool/replication/load balance server for PostgreSQL\n\n");
	fprintf(stderr, "usage: pgpool [-c][-f config_file][-F pcp_config_file][-a hba_file][-n][-d]\n");
	fprintf(stderr, "usage: pgpool [-f config_file][-F pcp_config_file][-a hba_file] [-m {s[mart]|f[ast]|i[mmediate]}] stop\n");
	fprintf(stderr, "usage: pgpool -h\n");
	fprintf(stderr, "  config_file default path: %s/%s\n",DEFAULT_CONFIGDIR, POOL_CONF_FILE_NAME);
	fprintf(stderr, "  pcp_config_file default path: %s/%s\n", DEFAULT_CONFIGDIR, PCP_PASSWD_FILE_NAME);
	fprintf(stderr, "  hba_file default path:    %s/%s\n",DEFAULT_CONFIGDIR, HBA_CONF_FILE_NAME);
	fprintf(stderr, "  -c: clears query cache. enable_query_cache must be on\n");
	fprintf(stderr, "  -n: don't run in daemon mode. does not detatch control tty\n");
	fprintf(stderr, "  -d: debug mode. lots of debug information will be printed\n");
	fprintf(stderr, "  stop: stop pgpool\n");
	fprintf(stderr, "  -h: print this help\n");
}

/*
* detatch control ttys
*/
static void daemonize(void)
{
	int			i;
	pid_t		pid;

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

	i = open("/dev/null", O_RDWR);
	dup2(i, 0);
	dup2(i, 1);
	dup2(i, 2);
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
	char path[POOLMAXPATHLEN];
	char pidbuf[128];

	snprintf(path, sizeof(path), "%s/%s", pool_config->logdir, PID_FILE_NAME);
	fd = fopen(path, "r");
	if (!fd)
	{
		return -1;
	}
	if (fread(pidbuf, 1, sizeof(pidbuf), fd) <= 0)
	{
		pool_error("could not read pid file as %s. reason: %s",
				   path, strerror(errno));
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
	char path[POOLMAXPATHLEN];
	char pidbuf[128];

	snprintf(path, sizeof(path), "%s/%s", pool_config->logdir, PID_FILE_NAME);
	fd = fopen(path, "w");
	if (!fd)
	{
		pool_error("could not open pid file as %s. reason: %s",
				   path, strerror(errno));
		pool_shmem_exit(1);
		exit(1);
	}
	snprintf(pidbuf, sizeof(pidbuf), "%d", (int)getpid());
	fwrite(pidbuf, strlen(pidbuf), 1, fd);
	if (fclose(fd))
	{
		pool_error("could not write pid file as %s. reason: %s",
				   path, strerror(errno));
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
		close(pipe_fds[0]);
		close(pipe_fds[1]);

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
		pool_error("bind() failed. reason: %s", strerror(errno));
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
		pool_error("bind() failed. reason: %s", strerror(errno));
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

static void myexit(int code)
{
	char path[POOLMAXPATHLEN];
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
	
	unlink(un_addr.sun_path);
	unlink(pcp_un_addr.sun_path);
	snprintf(path, sizeof(path), "%s/%s", pool_config->logdir, PID_FILE_NAME);
	unlink(path);

	pool_shmem_exit(code);
	exit(code);
}

/* notice backend connection error using SIGUSR1 */
void notice_backend_error(int node_id)
{
	pid_t parent = getppid();

	if (pool_config->parallel_mode)
	{
		return;
	}

	pool_log("notice_backend_error: %d fail over request from pid %d", node_id, getpid());

	Req_info->kind = NODE_DOWN_REQUEST;
	Req_info->node_id = node_id;

	if (node_id < 0 || node_id >= MAX_NUM_BACKENDS || !VALID_BACKEND(node_id))
	{
		pool_error("notice_backend_error: node %d is not valid backend.");
		return;
	}

	kill(parent, SIGUSR1);
}

/* send failback request using SIGUSR1 */
void send_failback_request(int node_id)
{
	pid_t parent = getppid();

	pool_log("send_failback_request: fail back %d th node request from pid %d", node_id, getpid());
	Req_info->kind = NODE_UP_REQUEST;
	Req_info->node_id = node_id;

	if (node_id < 0 || node_id >= MAX_NUM_BACKENDS || VALID_BACKEND(node_id))
	{
		pool_error("send_failback_request: node %d is alive.");
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
	int node_id;
	int i;

	pool_debug("failover_handler called");

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

	if (Req_info->kind == CLOSE_IDLE_REQUEST)
	{
		kill_all_children(SIGUSR1);
		kill(pcp_pid, SIGUSR2);
		return;
	}

	node_id = Req_info->node_id;

	if (node_id < 0)
	{
		pool_error("failover_handler: invalid node_id %d", node_id);
		kill(pcp_pid, SIGUSR2);
		return;
	}

	if (node_id >= MAX_NUM_BACKENDS ||
		(Req_info->kind == NODE_UP_REQUEST && VALID_BACKEND(node_id)) ||
		(Req_info->kind == NODE_DOWN_REQUEST && !VALID_BACKEND(node_id)))
	{
		pool_error("failover_handler: invalid node_id %d status:%d MAX_NUM_BACKENDS: %d", node_id,
					BACKEND_INFO(node_id).backend_status, MAX_NUM_BACKENDS);
		kill(pcp_pid, SIGUSR2);
		return;
	}

	if (Req_info->kind == NODE_UP_REQUEST)
	{
		pool_log("starting fail back. reconnect host %s(%d)",
				 BACKEND_INFO(node_id).backend_hostname,
				 BACKEND_INFO(node_id).backend_port);
	}
	else
	{
		pool_log("starting degeneration. shutdown host %s(%d)",
				 BACKEND_INFO(node_id).backend_hostname,
				 BACKEND_INFO(node_id).backend_port);
	}

	/* 
	 * if not in replication mode/master slave mode, we treat this a restart request.
	 * otherwise we need to check if we have already failovered.
	 */
	pool_debug("failover_handler: starting to selec new master node");
	switching = 1;

	/* failback request? */
	if (Req_info->kind == NODE_UP_REQUEST)
	{
		BACKEND_INFO(node_id).backend_status = CON_CONNECT_WAIT;	/* unset down status */
	}
	else
	{
		BACKEND_INFO(node_id).backend_status = CON_DOWN;	/* set down status */
	}

	for (i=0;i<pool_config->backend_desc->num_backends;i++)
	{
		if (VALID_BACKEND(i))
			break;
	}

	if (i == pool_config->backend_desc->num_backends)
	{
		pool_error("failover_handler: no valid DB node found");
	}
	else
	{
		if (Req_info->master_node_id == i && *InRecovery == 0)
		{
			pool_log("failover_handler: do not restart pgpool. same master node %d was selected", i);
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
			kill(pcp_pid, SIGUSR2);
			return;
		}

		pool_log("failover_handler: set new master node: %d", i);
		Req_info->master_node_id = i;
	}

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

/* no need to wait since it will be done in reap_handler */
#ifdef NOT_USED
	while (wait(NULL) > 0)
		;

	if (errno != ECHILD)
		pool_error("failover_handler: wait() failed. reason:%s", strerror(errno));
#endif

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

	Req_info->node_id = -1;
	switching = 0;
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
		pool_debug("health_check: %d the DB node status: %d", i, BACKEND_INFO(i).backend_status);

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

		read(fd, &kind, 1);

		if (write(fd, "X", 1) < 0)
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
	sigchld_request = 0;

	if (exiting)
	{
		pool_debug("reap_handler: exited due to exiting");
		return;
	}

	if (switching)
	{
		pool_debug("reap_handler: exited due to swicting");
		return;
	}

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
}

/*
 * pool_pause: A process pauses by select().
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
 * pool_pause: A process sleep using pool_pause().
 *             If a signal event occurs, it raises signal handler.
 */
static void pool_sleep(unsigned int second)
{
	struct timeval current_time, sleep_time;

	gettimeofday(&current_time, NULL);
	sleep_time.tv_sec = second + current_time.tv_sec;
	sleep_time.tv_usec = current_time.tv_usec;

	POOL_SETMASK(&UnBlockSig);
	while (sleep_time.tv_sec > current_time.tv_sec ||
		   sleep_time.tv_usec > current_time.tv_usec)
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
