/* -*-pgpool_main-c-*- */
/*
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
 */

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
#include <sys/types.h>
#include <fcntl.h>

#include <sys/wait.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#include <libgen.h>
#include "utils/elog.h"
#include "utils/palloc.h"

#include "pool.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "pool_config.h"
#include "context/pool_process_context.h"
#include "version.h"
#include "parser/pool_string.h"
#include "auth/pool_passwd.h"
#include "query_cache/pool_memqcache.h"
#include "watchdog/wd_ext.h"

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

#define CLEAR_ALARM \
	do { \
			elog(DEBUG1,"health check: clearing alarm"); \
    } while (alarm(0) > 0)


#define PGPOOLMAXLITSENQUEUELENGTH 10000

static int process_backend_health_check_failure(int health_check_node_id, int retrycnt);
static bool do_health_check(bool use_template_db, volatile int *health_check_node_id);

static void FileUnlink(int code, Datum path);
static int write_status_file();
static pid_t pcp_fork_a_child(int unix_fd, int inet_fd, char *pcp_conf_file);
static pid_t fork_a_child(int *fds, int id);
static pid_t worker_fork_a_child(void);
static int create_unix_domain_socket(struct sockaddr_un un_addr_tmp);
static int create_inet_domain_socket(const char *hostname, const int port);
static int *create_inet_domain_sockets(const char *hostname, const int port);
static void failover(void);
static void reaper(void);
static void wakeup_children(void);
static void reload_config(void);
static int pool_pause(struct timeval *timeout);
static void kill_all_children(int sig);
static int get_next_master_node(void);
static pid_t fork_follow_child(int old_master, int new_primary, int old_primary);
static int read_status_file(bool discard_status);
static RETSIGTYPE exit_handler(int sig);
static RETSIGTYPE reap_handler(int sig);
static RETSIGTYPE failover_handler(int sig);
static RETSIGTYPE reload_config_handler(int sig);
static RETSIGTYPE health_check_timer_handler(int sig);
static RETSIGTYPE wakeup_handler(int sig);

static void initialize_shared_mem_objects(bool clear_memcache_oidmaps);
static int trigger_failover_command(int node, const char *command_line,
									int old_master, int new_master, int old_primary);
static bool verify_backend_node_status(int backend_no, bool* is_standby);
static int find_primary_node(void);
static int find_primary_node_repeatedly(void);
static void terminate_all_childrens();
static void system_will_go_down(int code, Datum arg);
static pid_t pool_waitpid(int *status);
static char* process_name_from_pid(pid_t pid);

static struct sockaddr_un un_addr;		/* unix domain socket path */
static struct sockaddr_un pcp_un_addr;  /* unix domain socket path for PCP */
ProcessInfo *process_info = NULL;	/* Per child info table on shmem */

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

static int *fds;	/* listening file descriptors (UNIX socket, inet domain sockets) */

static int pcp_unix_fd; /* unix domain socket fd for PCP (not used) */
static int pcp_inet_fd; /* inet domain socket fd for PCP */
extern char pcp_conf_file[POOLMAXPATHLEN+1]; /* path for pcp.conf */
extern char conf_file[POOLMAXPATHLEN+1];
extern char hba_file[POOLMAXPATHLEN+1];

static int exiting = 0;		/* non 0 if I'm exiting */
static int switching = 0;		/* non 0 if I'm fail overing or degenerating */

POOL_REQUEST_INFO *Req_info;		/* request info area in shared memory */
volatile sig_atomic_t *InRecovery; /* non 0 if recovery is started */
volatile sig_atomic_t reload_config_request = 0;
static volatile sig_atomic_t failover_request = 0;
static volatile sig_atomic_t sigchld_request = 0;
static volatile sig_atomic_t wakeup_request = 0;

static int pipe_fds[2]; /* for delivering signals */

int my_proc_id;

static BackendStatusRecord backend_rec;	/* Backend status record */

static pid_t worker_pid = -1; /* pid of worker process */
static pid_t follow_pid = -1; /* pid for child process handling follow command */
static pid_t pcp_pid = -1; /* pid for child process handling PCP */

BACKEND_STATUS* my_backend_status[MAX_NUM_BACKENDS];		/* Backend status buffer */
int my_master_node_id;		/* Master node id buffer */

/*
* pgpool main program
*/

int PgpoolMain(bool discard_status, bool clear_memcache_oidmaps)
{
	int i;
	volatile int health_check_node_id = 0;
	volatile bool use_template_db = false;
	volatile int retrycnt;
	volatile int sys_retrycnt;

	MemoryContext MainLoopMemoryContext;
	sigjmp_buf	local_sigjmp_buf;
	
	/* Set the process type variable */
	processType = PT_MAIN;
	processState = INITIALIZING;

	/*
	 * Restore previous backend status if possible
	 */
	read_status_file(discard_status);
    
	/*
	 * install the call back for preparation of system exit
	 */
    on_system_exit(system_will_go_down, (Datum)NULL);

	/* set unix domain socket path for connections to pgpool */
	snprintf(un_addr.sun_path, sizeof(un_addr.sun_path), "%s/.s.PGSQL.%d",
			 pool_config->socket_dir,
			 pool_config->port);
	/* set unix domain socket path for pgpool PCP communication */
	snprintf(pcp_un_addr.sun_path, sizeof(pcp_un_addr.sun_path), "%s/.s.PGSQL.%d",
			 pool_config->pcp_socket_dir,
			 pool_config->pcp_port);

	/* set up signal handlers */
	pool_signal(SIGPIPE, SIG_IGN);

	/* create unix domain socket */
	fds = malloc(sizeof(int));
	if (fds == NULL)
		ereport(FATAL,
				(errmsg("failed to allocate memory in startup process")));

	fds[0] = create_unix_domain_socket(un_addr);
	on_proc_exit(FileUnlink, (Datum) un_addr.sun_path);

	/* create inet domain socket if any */
	if (pool_config->listen_addresses[0])
	{
		int *inet_fds, *walk;
		int n = 1;

		inet_fds = create_inet_domain_sockets(pool_config->listen_addresses, pool_config->port);

		for (walk = inet_fds; *walk != -1; walk++)
			n++;

		fds = realloc(fds, sizeof(int) * (n+1));
		if (fds == NULL)
			ereport(FATAL,
					(errmsg("failed to allocate memory in startup process")));

		n = 1;
		for (walk = inet_fds; *walk != -1; walk++)
		{
			fds[n] = inet_fds[n-1];
			n++;
		}
		fds[n] = -1;
		free(inet_fds);
	}

	initialize_shared_mem_objects(clear_memcache_oidmaps);

	/* start watchdog */
	if (pool_config->use_watchdog )
	{
		if (!wd_main(1))
		{
            ereport(FATAL,
                    (errmsg("wd_main error")));
		}
	}

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
		process_info[i].pid = fork_a_child(fds, i);
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
		ereport(FATAL,
			(errmsg("failed to create pipe")));
	}
	
	MemoryContextSwitchTo(TopMemoryContext);

	/* Create per loop iteration memory context */
	MainLoopMemoryContext = AllocSetContextCreate(TopMemoryContext,
											  "pgpool_main_loop",
											  ALLOCSET_DEFAULT_MINSIZE,
											  ALLOCSET_DEFAULT_INITSIZE,
											  ALLOCSET_DEFAULT_MAXSIZE);

	/* fork a child for PCP handling */
	pcp_unix_fd = create_unix_domain_socket(pcp_un_addr);
	/* Add onproc exit to clean up the unix domain socket at exit */
	on_proc_exit(FileUnlink, (Datum)pcp_un_addr.sun_path);

    /* maybe change "*" to pool_config->pcp_listen_addresses */
	pcp_inet_fd = create_inet_domain_socket("*", pool_config->pcp_port);
	pcp_pid = pcp_fork_a_child(pcp_unix_fd, pcp_inet_fd, pcp_conf_file);

	/* Fork worker process */
	worker_pid = worker_fork_a_child();

	retrycnt = 0;		/* reset health check retry counter */
	sys_retrycnt = 0;	/* reset SystemDB health check retry counter */

	/*
	 * check for child signals to ensure child startup before reporting successfull start
	 */
	CHECK_REQUEST;

	ereport(LOG,
			(errmsg("%s successfully started. version %s (%s)", PACKAGE, VERSION, PGPOOLVERSION)));

	/* Save primary node id */
	Req_info->primary_node_id = find_primary_node();

	if (sigsetjmp(local_sigjmp_buf, 1) != 0)
	{
		/* Since not using PG_TRY, must reset error stack by hand */
		error_context_stack = NULL;
		EmitErrorReport();
		MemoryContextSwitchTo(TopMemoryContext);
		FlushErrorState();
		POOL_SETMASK(&BlockSig);

		/* Check if we are failed during health check
		 * perform the necessary actions in case
		 */
		if(processState == PERFORMING_HEALTH_CHECK)
		{
			if (errno != EINTR || health_check_timer_expired)
			{
				if (use_template_db == false)
				{
					/* Health check was performed on 'postgres' database
					 * lets try to perform health check with template1 db
					 * before marking the health check as failed
					 */
					use_template_db = true;

				}
				else
				{
					int ret;

					retrycnt++;
					ret = process_backend_health_check_failure(health_check_node_id, retrycnt);
					if (ret > 0) /* Retries are exhausted, reset the counter */
					{
						retrycnt = 0;
						if (ret == 2)
						{
							health_check_node_id = 0;
							use_template_db = false;
						}
					}
				}
			}
		}
		else if (processState == PERFORMING_SYSDB_CHECK)
		{
			if ( errno != EINTR || health_check_timer_expired)
			{
				sys_retrycnt++;
				pool_signal(SIGALRM, SIG_IGN);
				CLEAR_ALARM;

				if (sys_retrycnt > NUM_BACKENDS)
				{
					ereport(LOG,
                            (errmsg("set SystemDB down status")));

					SYSDB_STATUS = CON_DOWN;
					sys_retrycnt = 0;
				}
				int sleep_time = pool_config->health_check_period/NUM_BACKENDS;

				ereport(DEBUG2,
                        (errmsg("retry sleep time: %d seconds", sleep_time)));
				pool_sleep(sleep_time);
			}
		}
	}
	/* We can now handle ereport(ERROR) */
	PG_exception_stack = &local_sigjmp_buf;
	
	/* This is the main loop */
	for (;;)
	{
		bool all_nodes_healthy;
		CHECK_REQUEST;

		/* do we need health checking for PostgreSQL? */
		if (pool_config->health_check_period > 0)
		{
			/* rest per iteration memory context */
			MemoryContextSwitchTo(MainLoopMemoryContext);
			MemoryContextResetAndDeleteChildren(MainLoopMemoryContext);

			if (retrycnt == 0 && !use_template_db)
				ereport(DEBUG1,
					(errmsg("starting health check")));
			else if(retrycnt)
				ereport(LOG,
					(errmsg("health checking retry count %d", retrycnt)));

			if (pool_config->health_check_timeout > 0)
			{
				/*
				 * set health checker timeout. we want to detect
				 * communication path failure much earlier before
				 * TCP/IP stack detects it.
				 */
				CLEAR_ALARM;
				pool_signal(SIGALRM, health_check_timer_handler);
				alarm(pool_config->health_check_timeout);
			}
			/*
			 * do actual health check. trying to connect to the backend
			 */
			errno = 0;
			health_check_timer_expired = 0;

			if (pool_config->parallel_mode)
			{
				processState = PERFORMING_SYSDB_CHECK;
				system_db_health_check();
				sys_retrycnt = 0;
				errno = 0;
			}

			POOL_SETMASK(&UnBlockSig);

			processState = PERFORMING_HEALTH_CHECK;
			all_nodes_healthy = do_health_check(use_template_db,&health_check_node_id);
			POOL_SETMASK(&BlockSig);

			health_check_node_id = 0;
			use_template_db = false;
			retrycnt = 0;

			if (all_nodes_healthy && retrycnt)
				ereport(LOG,
					(errmsg("after retry %d, All backends are returned to healthy state",retrycnt)));


			processState = SLEEPING;

			if (pool_config->health_check_timeout > 0)
			{
				/* seems OK. cancel health check timer */
				pool_signal(SIGALRM, SIG_IGN);
				CLEAR_ALARM;
			}
			pool_sleep(pool_config->health_check_period);
		}
		else /* Health Check is not enable and we have not much to do */
		{
			processState = SLEEPING;
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
}

/*
 * Function process the backend node failure captured by the health check
 * since this function is called from the exception handler so ereport(ERROR)
 * is not allowed from this function
 * Function returns non zero if no retries are exhausted.
 * ( 1 if failover is not performed on node and 2 in case of failover)
 */
static int
process_backend_health_check_failure(int health_check_node_id, int retrycnt)
{
	/*
	 *  health check is failed on template1 database as well
	 */
	int sleep_time = pool_config->parallel_mode ?
			pool_config->health_check_period/NUM_BACKENDS : pool_config->health_check_retry_delay;

	int health_check_max_retries = pool_config->parallel_mode ?
			(NUM_BACKENDS - 1) : pool_config->health_check_max_retries;

	pool_signal(SIGALRM, SIG_IGN);	/* Cancel timer */
	CLEAR_ALARM;

	if (health_check_max_retries > 0 && retrycnt <= health_check_max_retries)
	{
		/* Keep retrying and sleep a little in between */
		ereport(DEBUG1,
			(errmsg("Sleeping for %d seconds from process backend health check failure", sleep_time),
					errdetail("health check failed retry no is %d while max retries are %d",retrycnt,health_check_max_retries) ));

		pool_sleep(sleep_time);
	}
	else
	{
		/* No more retries left, proceed to failover if allowed */
		if ((!pool_config->parallel_mode) &&
			POOL_DISALLOW_TO_FAILOVER(BACKEND_INFO(health_check_node_id).flag))
		{
			ereport(LOG,
					(errmsg("health check failed on node %d but failover is disallowed for the node", health_check_node_id)));
			return 1;
		}
		else
		{
			ereport(LOG,
					(errmsg("setting backend node %d status to NODE DOWN", health_check_node_id)));
			health_check_timer_expired = 0;
			register_node_operation_request(NODE_DOWN_REQUEST,&health_check_node_id,1);
			return 2;
			/* need to distribute this info to children ??*/
		}
	}
	return 0;
}

/*
 * register_node_operation_request()
 *
 * This function enqueues the failover/failback requests, and fires the failover() if the function
 * is not already executing
 */
bool register_node_operation_request(POOL_REQUEST_KIND kind, int* node_id_set, int count)
{
	bool failover_in_progress;
#ifdef HAVE_SIGPROCMASK
	sigset_t oldmask;
#else
	int	oldmask;
#endif

	/* 
	 * if the queue is already full
	 * what to do?
	 */
	if((Req_info->request_queue_tail - MAX_REQUEST_QUEUE_SIZE) == Req_info->request_queue_head)
	{
		return false;
	}
	POOL_SETMASK2(&BlockSig, &oldmask);
	pool_semaphore_lock(REQUEST_INFO_SEM);

	if((Req_info->request_queue_tail - MAX_REQUEST_QUEUE_SIZE) == Req_info->request_queue_head)
	{
		pool_semaphore_unlock(REQUEST_INFO_SEM);
		return false;
	}
	Req_info->request_queue_tail++;
	Req_info->request[Req_info->request_queue_tail % MAX_REQUEST_QUEUE_SIZE].kind = kind;
	if(count > 0)
		memcpy(Req_info->request[Req_info->request_queue_tail % MAX_REQUEST_QUEUE_SIZE].node_id, node_id_set, (sizeof(int) * count));
	Req_info->request[Req_info->request_queue_tail % MAX_REQUEST_QUEUE_SIZE].count = count;
	failover_in_progress = Req_info->switching;
	pool_semaphore_unlock(REQUEST_INFO_SEM);

	if (getpid() == mypid)
	{
		/* 
		 * We are invoked from main process
		 * call failover with blocked signals
		 */
		failover();
		POOL_SETMASK(&oldmask);
	}
	else
	{
		POOL_SETMASK(&oldmask);
		if(failover_in_progress == false)
		{
			pool_signal_parent(SIGUSR1);
		}
	}

	return true;
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
		on_exit_reset();

		close(pipe_fds[0]);
		close(pipe_fds[1]);

		/* Set the process type variable */
		processType = PT_PCP;

		/* call PCP child main */
		POOL_SETMASK(&UnBlockSig);
		health_check_timer_expired = 0;
		reload_config_request = 0;
		run_as_pcp_child = true;
		pcp_do_child(unix_fd, inet_fd, pcp_conf_file);
	}
	else if (pid == -1)
	{
		ereport(FATAL,
			(errmsg("fork() failed. reason: %s", strerror(errno))));
	}

	return pid;
}

/*
* fork a child
*/
pid_t fork_a_child(int *fds, int id)
{
	pid_t pid;

	pid = fork();

	if (pid == 0)
	{
		on_exit_reset();

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

		/* Set the process type variable */
		processType = PT_CHILD;

		/* call child main */
		POOL_SETMASK(&UnBlockSig);
		health_check_timer_expired = 0;
		reload_config_request = 0;
		my_proc_id = id;
		run_as_pcp_child = false;
		do_child(fds);
	}
	else if (pid == -1)
	{
        ereport(FATAL,
            (errmsg("failed to fork a child"),
                 errdetail("system call fork() failed with reason: %s", strerror(errno))));
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
		on_exit_reset();

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

		/* Set the process type variable */
		processType = PT_WORKER;

		/* call child main */
		POOL_SETMASK(&UnBlockSig);
		health_check_timer_expired = 0;
		reload_config_request = 0;
		do_worker_child();
	}
	else if (pid == -1)
	{
        ereport(FATAL,
            (errmsg("failed to fork a child"),
                 errdetail("system call fork() failed with reason: %s", strerror(errno))));
	}
	
	return pid;
}

static int *create_inet_domain_sockets(const char *hostname, const int port)
{
	int ret;
	int fd;
	int one = 1;
	int status;
	int backlog;
	int n = 0;
	int *sockfds;
	char *portstr;
	struct addrinfo *walk;
	struct addrinfo *res;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	/* getaddrinfo() requires a string because it also accepts service names, such as "http". */
	if (asprintf(&portstr, "%d", port) == -1)
	{
		ereport(FATAL,
			(errmsg("failed to create INET domain socket"),
			errdetail("asprintf() failed: %s", strerror(errno))));
	}

	if ((ret = getaddrinfo(hostname, portstr, &hints, &res)) != 0)
	{
		ereport(FATAL,
			(errmsg("failed to create INET domain socket"),
			errdetail("getaddrinfo() failed: %s", gai_strerror(ret))));
	}

	free(portstr);

	for (walk = res; walk != NULL; walk = walk->ai_next)
		n++;

	sockfds = malloc(sizeof(int) * (n+1));
	n = 0;
	for (walk = res; walk != NULL; walk = walk->ai_next)
		sockfds[n++] = -1;
	/* We always terminate the list of sockets with a -1 entry */
	sockfds[n] = -1;

	n = 0;

	for (walk = res; walk != NULL; walk = walk->ai_next)
	{
		char buf[INET6_ADDRSTRLEN+1];
		memset(buf, 0, sizeof(buf));
		if ((ret = getnameinfo((struct sockaddr*)walk->ai_addr, walk->ai_addrlen,
						buf, sizeof(buf), NULL, 0, NI_NUMERICHOST)) != 0)
		{
			ereport(FATAL,
				(errmsg("failed to create INET domain socket"),
				errdetail("getnameinfo() failed: \"%s\"", gai_strerror(ret))));
		}

		ereport(LOG,
			(errmsg("Setting up socket for %s:%d", buf, port)));

		if ((fd = socket(walk->ai_family, walk->ai_socktype, walk->ai_protocol)) == -1)
		{
			/* A single failure is not necessarily a problem (machines without
			 * proper dual stack setups), but if we cannot create any socket at
			 * all, we report a FATAL error. */
			ereport(LOG,
				(errmsg("perhaps failed to create INET domain socket"),
				errdetail("socket(%s) failed: \"%s\"", buf, strerror(errno))));
			continue;
		}

		if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &one,
						sizeof(one))) == -1)
		{
			ereport(FATAL,
				(errmsg("failed to create INET domain socket"),
				errdetail("socket error \"%s\"",strerror(errno))));
		}

		if (walk->ai_family == AF_INET6)
		{
			/* On some machines, depending on the default value in
			 * /proc/sys/net/ipv6/bindv6only, sockets will listen on both IPv6
			 * and IPv4 at the same time. Since we are creating one socket per
			 * address family, disable that option specifically to be sure it
			 * is off. */
			if ((setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one))) == -1) {
				ereport(LOG,
					(errmsg("perhaps failed to create INET domain socket"),
					errdetail("setsockopt(%s, IPV6_V6ONLY) failed: \"%s\"", buf, strerror(errno))));
			}
		}

		if (bind(fd, walk->ai_addr, walk->ai_addrlen) != 0)
		{
			ereport(FATAL,
				(errmsg("failed to create INET domain socket"),
				errdetail("bind on socket failed with error \"%s\"",strerror(errno))));
		}

		backlog = pool_config->num_init_children * pool_config->listen_backlog_multiplier;

		if (backlog > PGPOOLMAXLITSENQUEUELENGTH)
			backlog = PGPOOLMAXLITSENQUEUELENGTH;

		status = listen(fd, backlog);
		if (status < 0)
			ereport(FATAL,
				(errmsg("failed to create INET domain socket"),
				errdetail("listen on socket failed with error \"%s\"",strerror(errno))));

		sockfds[n++] = fd;
	}

	freeaddrinfo(res);

	if (n == 0) {
		ereport(FATAL,
			(errmsg("failed to create INET domain socket"),
			errdetail("Failed to create any sockets. See the earlier LOG messages.")));
	}

	return sockfds;
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
		ereport(FATAL,
			(errmsg("failed to create INET domain socket"),
			errdetail("socket error \"%s\"",strerror(errno))));
	}
	if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &one,
					sizeof(one))) == -1)
	{
		ereport(FATAL,
			(errmsg("failed to create INET domain socket"),
			errdetail("socket error \"%s\"",strerror(errno))));
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
			ereport(FATAL,
				(errmsg("failed to create INET domain socket"),
                    errdetail("could not resolve hostname \"%s\": error \"%s\"",hostname,hstrerror(h_errno))));

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
		if (getnameinfo((struct sockaddr *) &addr, len, hostname, sizeof(hostname), servname, sizeof(servname), 0) == 0) 
		{
			host = hostname;
			serv = servname;
		}
		ereport(FATAL,
			(errmsg("failed to create INET domain socket"),
             errdetail("bind on host:\"%s\" server:\"%s\" failed with error \"%s\"",host, serv,strerror(errno))));
	}

    backlog = pool_config->num_init_children * pool_config->listen_backlog_multiplier;

	if (backlog > PGPOOLMAXLITSENQUEUELENGTH)
		backlog = PGPOOLMAXLITSENQUEUELENGTH;

	status = listen(fd, backlog);
	if (status < 0)
		ereport(FATAL,
			(errmsg("failed to create INET domain socket"),
			errdetail("listen on socket failed with error \"%s\"",strerror(errno))));

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
        ereport(FATAL,
            (errmsg("failed to create a socket"),
                 errdetail("Failed to create UNIX domain socket. error: \"%s\"", strerror(errno))));
	}
	memset((char *) &addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", un_addr_tmp.sun_path);
	len = sizeof(struct sockaddr_un);
	status = bind(fd, (struct sockaddr *)&addr, len);
	if (status == -1)
	{
        ereport(FATAL,
            (errmsg("failed to bind a socket: \"%s\"",un_addr_tmp.sun_path),
                 errdetail("bind socket failed with error: \"%s\"", strerror(errno))));
	}

	if (chmod(un_addr_tmp.sun_path, 0777) == -1)
	{
        ereport(FATAL,
			(errmsg("failed to bind a socket: \"%s\"",un_addr_tmp.sun_path),
                 errdetail("system call chmod failed with error: \"%s\"", strerror(errno))));
	}

	status = listen(fd, PGPOOLMAXLITSENQUEUELENGTH);
	if (status < 0)
	{
        ereport(FATAL,
			(errmsg("failed to bind a socket: \"%s\"",un_addr_tmp.sun_path),
                 errdetail("system call listen() failed with error: \"%s\"", strerror(errno))));
	}
	return fd;
}

/*
 * function called as shared memory exit call back to kill all childrens
 */
static void terminate_all_childrens()
{
    pid_t wpid;
    /*
     * This is supposed to be called from main process
     */
	if(processType != PT_MAIN)
		return;
	POOL_SETMASK(&BlockSig);

    kill_all_children(SIGINT);
    if(pcp_pid > 0)
        kill(pcp_pid, SIGINT);
    pcp_pid = 0;
    if(worker_pid > 0)
        kill(worker_pid, SIGINT);
    worker_pid = 0;
	if (pool_config->use_watchdog)
	{
		wd_kill_watchdog(SIGINT);
	}

    /* wait for all children to exit */
    do
    {
        wpid = wait(NULL);
    }while (wpid > 0 || (wpid == -1 && errno == EINTR));
    
    if (wpid == -1 && errno != ECHILD)
        ereport(LOG,
                (errmsg("wait() failed. reason:%s", strerror(errno))));

	POOL_SETMASK(&UnBlockSig);
}

void notice_backend_error(int node_id)
{
	int n = node_id;

	if (getpid() == mypid)
	{
		ereport(LOG,
                (errmsg("notice_backend_error: called from pgpool main. ignored.")));
	}
	else
	{
		degenerate_backend_set(&n, 1);
	}
}

/* notice backend connection error using SIGUSR1 */
void degenerate_backend_set(int *node_id_set, int count)
{
	int i;

	int node_id[MAX_NUM_BACKENDS];
	int node_count = 0;

	if (pool_config->parallel_mode)
	{
		return;
	}

	for (i = 0; i < count; i++)
	{
		if (node_id_set[i] < 0 || node_id_set[i] >= MAX_NUM_BACKENDS ||
			!VALID_BACKEND(node_id_set[i]))
		{
			if (node_id_set[i] < 0 || node_id_set[i] >= MAX_NUM_BACKENDS)
				ereport(LOG,
						(errmsg("invalid degenerate backend request, node id: %d is out of range. node id must be between [0 and %d]"
								,node_id_set[i],MAX_NUM_BACKENDS)));
			else
				ereport(LOG,
						(errmsg("invalid degenerate backend request, node id : %d status: [%d] is not valid for failover"
								,node_id_set[i],BACKEND_INFO(node_id_set[i]).backend_status)));

			continue;
		}

		if (POOL_DISALLOW_TO_FAILOVER(BACKEND_INFO(node_id_set[i]).flag))
		{
			ereport(LOG,
					(errmsg("degenerate backend request for node_id: %d from pid [%d] is canceled because failover is disallowed on the node",
							node_id_set[i], getpid())));
		}
		else
		{
			ereport(LOG,
					(errmsg("received degenerate backend request for node_id: %d from pid [%d]",
							node_id_set[i], getpid())));

			node_id[node_count++] = node_id_set[i];
		}
	}

	if (node_count)
	{
		if (!pool_config->use_watchdog || WD_OK == wd_degenerate_backend_set(node_id_set, count))
		{
			register_node_operation_request(NODE_DOWN_REQUEST, node_id, node_count);
		}
		else
		{
			ereport(LOG,
					(errmsg("degenerate backend request for node_id: %d from pid [%d] is canceled  by other pgpool"
							, node_id_set[i], getpid())));
		}
	}

}

/* send promote node request using SIGUSR1 */
void promote_backend(int node_id)
{
	if (!MASTER_SLAVE || strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP))
	{
		return;
	}

	if (node_id < 0 || node_id >= MAX_NUM_BACKENDS || !VALID_BACKEND(node_id))
	{
		if (node_id < 0 || node_id >= MAX_NUM_BACKENDS)
			ereport(LOG,
					(errmsg("invalid promote backend request, node id: %d is out of range. node id must be between [0 and %d]"
							,node_id,MAX_NUM_BACKENDS)));
		else
			ereport(LOG,
					(errmsg("invalid promote backend request, node id : %d status: [%d] not valid"
							,node_id,BACKEND_INFO(node_id).backend_status)));
		return;
	}
	ereport(LOG,
			(errmsg("received promote backend request for node_id: %d from pid [%d]",
					node_id, getpid())));

	if (!pool_config->use_watchdog || WD_OK == wd_promote_backend(node_id))
	{
		register_node_operation_request(PROMOTE_NODE_REQUEST, &node_id, 1);
	}
	else
	{
		ereport(LOG,
				(errmsg("promote backend request for node_id: %d from pid [%d] is canceled  by other pgpool"
						, node_id, getpid())));
	}
}

/* send failback request using SIGUSR1 */
void send_failback_request(int node_id)
{
    if (node_id < 0 || node_id >= MAX_NUM_BACKENDS ||
		(RAW_MODE && BACKEND_INFO(node_id).backend_status != CON_DOWN && VALID_BACKEND(node_id)))
	{
		if (node_id < 0 || node_id >= MAX_NUM_BACKENDS)
			ereport(LOG,
					(errmsg("invalid failback request, node id: %d is out of range. node id must be between [0 and %d]"
							,node_id,MAX_NUM_BACKENDS)));
		else
			ereport(LOG,
					(errmsg("invalid failback request, node id : %d status: [%d] not valid for failback"
							,node_id,BACKEND_INFO(node_id).backend_status)));
		return;
	}

	ereport(LOG,
			(errmsg("received failback request for node_id: %d from pid [%d]",
					node_id, getpid())));

	if (pool_config->use_watchdog && WD_OK != wd_send_failback_request(node_id))
	{
		ereport(LOG,
				(errmsg("failback request for node_id: %d from pid [%d] is canceled  by other pgpool"
						, node_id, getpid())));
		return;
	}
	register_node_operation_request(NODE_UP_REQUEST, &node_id, 1);
}

static RETSIGTYPE exit_handler(int sig)
{
	int i;
    pid_t wpid;
	POOL_SETMASK(&AuthBlockSig);

	/*
	 * this could happen in a child process if a signal has been sent
	 * before resetting signal handler
	 */
	if (getpid() != mypid)
	{
		ereport(DEBUG1,
			(errmsg("exit_handler: I am not parent")));

		POOL_SETMASK(&UnBlockSig);
		proc_exit(0);
	}

	if (sig == SIGTERM)
		ereport(LOG,
                (errmsg("received smart shutdown request")));
	else if (sig == SIGINT)
		ereport(LOG,
                (errmsg("received fast shutdown request")));
	else if (sig == SIGQUIT)
		ereport(LOG,
                (errmsg("received immediate shutdown request")));
	else
	{
		ereport(LOG,
			(errmsg("main process received unknown signal: %d",sig),
				 errdetail("ignoring...")));

		POOL_SETMASK(&UnBlockSig);
		return;
	}
	exiting = 1;
    processState = EXITING;

    /* Close listen socket */
	ereport(LOG,
		(errmsg("shutdown request. closing listen socket")));

	int *walk;
	for (walk = fds; *walk != -1; walk++)
		close(*walk);

	for (i = 0; i < pool_config->num_init_children; i++)
	{
		pid_t pid = process_info[i].pid;
		if (pid)
		{
			kill(pid, sig);
			process_info[i].pid = 0;
		}
	}

    if(pcp_pid > 0)
        kill(pcp_pid, sig);
    pcp_pid = 0;

    if(worker_pid > 0)
        kill(worker_pid, sig);
    worker_pid = 0;

	if (pool_config->use_watchdog)
	{
		wd_kill_watchdog(sig);
	}

	POOL_SETMASK(&UnBlockSig);
    do
    {
        wpid = wait(NULL);
    }while (wpid > 0 || (wpid == -1 && errno == EINTR));
    
    if (wpid == -1 && errno != ECHILD)
        ereport(LOG,
                (errmsg("wait() failed. reason:%s", strerror(errno))));

	process_info = NULL;
	exit(0);
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
		 * to check backend status with VALID_BACKEND_RAW.
		 */
		if (RAW_MODE)
		{
			if (VALID_BACKEND_RAW(i))
				break;
		}
		else
		{
			if (VALID_BACKEND(i))
				break;
		}
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
	if(write(pipe_fds[1], "\0", 1) < 0)
        ereport(WARNING,
                (errmsg("failover_handler: write to pipe failed with error \"%s\"", strerror(errno))));

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
	bool by_health_check;
	int new_master;
	int new_primary;
	int nodes[MAX_NUM_BACKENDS];
	bool need_to_restart_children;
	int status;
	int sts;
	bool need_to_restart_pcp = false;

	ereport(DEBUG1,
		(errmsg("failover handler called")));

	memset(nodes, 0, sizeof(int) * MAX_NUM_BACKENDS);

	/*
	 * this could happen in a child process if a signal has been sent
	 * before resetting signal handler
	 */
	if (getpid() != mypid)
	{
		ereport(DEBUG1,
			(errmsg("failover handler called"),
				 errdetail("I am not parent")));
		kill(pcp_pid, SIGUSR2);
		return;
	}
	/*
	 * processing SIGTERM, SIGINT or SIGQUIT
	 */
	if (exiting)
	{
		ereport(DEBUG1,
				(errmsg("failover handler called while exiting")));
		kill(pcp_pid, SIGUSR2);
		return;
	}

	/*
	 * processing fail over or switch over
	 */
	if (switching)
	{
		ereport(DEBUG1,
				(errmsg("failover handler called while switching")));
		kill(pcp_pid, SIGUSR2);
		return;
	}
	Req_info->switching = true;
	switching = 1;
	for(;;)
	{
		POOL_REQUEST_KIND reqkind;
		int queue_index;
		int node_id_set[MAX_NUM_BACKENDS];
		int node_count;

		pool_semaphore_lock(REQUEST_INFO_SEM);

		if(Req_info->request_queue_tail == Req_info->request_queue_head) /* request queue is empty*/
		{
			switching = 0;
			Req_info->switching = false;
			pool_semaphore_unlock(REQUEST_INFO_SEM);
			break;
		}

		/* make a local copy of request */
		Req_info->request_queue_head++;
		queue_index = Req_info->request_queue_head % MAX_REQUEST_QUEUE_SIZE;
		memcpy(node_id_set, Req_info->request[queue_index].node_id , (sizeof(int) * Req_info->request[queue_index].count));
		reqkind = Req_info->request[queue_index].kind;
		node_count = Req_info->request[queue_index].count;

		pool_semaphore_unlock(REQUEST_INFO_SEM);

		if (reqkind == CLOSE_IDLE_REQUEST)
		{
			kill_all_children(SIGUSR1);
			continue;
		}

		/*
		 * if not in replication mode/master slave mode, we treat this a restart request.
		 * otherwise we need to check if we have already failovered.
		 */
		ereport(DEBUG1,
			(errmsg("failover handler"),
				 errdetail("starting to select new master node")));
		node_id = node_id_set[0];

		/* start of command inter-lock with watchdog */
		if (pool_config->use_watchdog)
		{
			by_health_check = (!failover_request && reqkind == NODE_DOWN_REQUEST);
			wd_start_interlock(by_health_check, node_id);
		}

		/* failback request? */
		if (reqkind == NODE_UP_REQUEST)
		{
			if (node_id < 0 || node_id >= MAX_NUM_BACKENDS ||
				(reqkind == NODE_UP_REQUEST && !(RAW_MODE &&
				BACKEND_INFO(node_id).backend_status == CON_DOWN) && VALID_BACKEND(node_id)) ||
				(reqkind == NODE_DOWN_REQUEST && !VALID_BACKEND(node_id)))
			{

				if (node_id < 0 || node_id >= MAX_NUM_BACKENDS)
					ereport(LOG,
						(errmsg("invalid failback request, node id: %d is invalid. node id must be between [0 and %d]",node_id,MAX_NUM_BACKENDS)));
				else
					ereport(LOG,
							(errmsg("invalid failback request, status: [%d] of node id : %d is invalid for failback",BACKEND_INFO(node_id).backend_status,node_id)));

				/* end of command inter-lock */
				if (pool_config->use_watchdog)
					wd_leave_interlock();

				continue;
			}

			ereport(LOG,
				(errmsg("starting fail back. reconnect host %s(%d)",
					 BACKEND_INFO(node_id).backend_hostname,
					 BACKEND_INFO(node_id).backend_port)));

			BACKEND_INFO(node_id).backend_status = CON_CONNECT_WAIT;	/* unset down status */

			/* wait for failback command lock or to be lock holder */
			if (pool_config->use_watchdog && !wd_am_I_lock_holder())
			{
				wd_wait_for_lock(WD_FAILBACK_COMMAND_LOCK);
			}
			/* execute failback command if lock holder */
			if (!pool_config->use_watchdog || wd_am_I_lock_holder())
			{
				trigger_failover_command(node_id, pool_config->failback_command,
										MASTER_NODE_ID, get_next_master_node(), PRIMARY_NODE_ID);

				/* unlock failback command */
				if (pool_config->use_watchdog)
					wd_unlock(WD_FAILBACK_COMMAND_LOCK);
			}
		}
		else if (reqkind == PROMOTE_NODE_REQUEST)
		{
			if (node_id != -1 && VALID_BACKEND(node_id))
			{
				ereport(LOG,
						(errmsg("starting promotion. promote host %s(%d)",
						 BACKEND_INFO(node_id).backend_hostname,
						 BACKEND_INFO(node_id).backend_port)));
			}
			else
			{
				ereport(LOG,
						(errmsg("failover: no backends are promoted")));

				/* end of command inter-lock */
				if (pool_config->use_watchdog)
					wd_leave_interlock();

				continue;
			}
		}
		else
		{
			int cnt = 0;

			for (i = 0; i < node_count; i++)
			{
				if (node_id_set[i] != -1 &&
					((RAW_MODE && VALID_BACKEND_RAW(node_id_set[i])) ||
					 VALID_BACKEND(node_id_set[i])))
				{
					ereport(LOG,
							(errmsg("starting degeneration. shutdown host %s(%d)",
							 BACKEND_INFO(node_id_set[i]).backend_hostname,
							 BACKEND_INFO(node_id_set[i]).backend_port)));

					BACKEND_INFO(node_id_set[i]).backend_status = CON_DOWN;	/* set down status */
					/* save down node */
					nodes[node_id_set[i]] = 1;
					cnt++;
				}
			}

			if (cnt == 0)
			{
				ereport(LOG,
						(errmsg("failover: no backends are degenerated")));

				/* end of command inter-lock */
				if (pool_config->use_watchdog)
					wd_leave_interlock();

				continue;
			}
		}

		new_master = get_next_master_node();

		if (new_master < 0)
		{
			ereport(LOG,
					(errmsg("failover: no valid backends node found")));
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
		if (Req_info->master_node_id == new_master && *InRecovery == RECOVERY_INIT)
		{
			ereport(LOG,
                    (errmsg("failover_handler: do not restart pgpool. same master node %d was selected", new_master)));
			if (reqkind == NODE_UP_REQUEST)
			{
				ereport(LOG,
                        (errmsg("failback done. reconnect host %s(%d)",
						 BACKEND_INFO(node_id).backend_hostname,
						 BACKEND_INFO(node_id).backend_port)));
			}
			else
			{
				ereport(LOG,
                        (errmsg("failover done. shutdown host %s(%d)",
						 BACKEND_INFO(node_id).backend_hostname,
						 BACKEND_INFO(node_id).backend_port)));
			}

			/* exec failover_command */
			for (i = 0; i < pool_config->backend_desc->num_backends; i++)
			{
				if (nodes[i])
					trigger_failover_command(i, pool_config->failover_command);
			}

			switching = 0;
			Req_info->switching = false;
			kill(pcp_pid, SIGUSR2);
			switching = 0;
			Req_info->switching = false;
			continue;
		}
	}
#endif


		/* On 2011/5/2 Tatsuo Ishii says: if mode is streaming replication
		* and request is NODE_UP_REQUEST(failback case) we don't need to
		* restart all children. Existing session will not use newly
		* attached node, but load balanced node is not changed until this
		* session ends, so it's harmless anyway.
		*/
		if (MASTER_SLAVE && !strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP)	&&
			reqkind == NODE_UP_REQUEST)
		{
			ereport(LOG,
					(errmsg("Do not restart children because we are failbacking node id %d host%s port:%d and we are in streaming replication mode", node_id,
					 BACKEND_INFO(node_id).backend_hostname,
					 BACKEND_INFO(node_id).backend_port)));

			need_to_restart_children = false;
		}
		else
		{
			ereport(LOG,
					(errmsg("Restart all children")));

			/* kill all children */
			for (i = 0; i < pool_config->num_init_children; i++)
			{
				pid_t pid = process_info[i].pid;
				if (pid)
				{
					kill(pid, SIGQUIT);
					ereport(DEBUG1,
						(errmsg("failover handler"),
							 errdetail("kill process with PID:%d", pid)));
				}
			}

			need_to_restart_children = true;
		}

		/* wait for failover command lock or to be lock holder*/
		if (pool_config->use_watchdog && !wd_am_I_lock_holder())
		{
			wd_wait_for_lock(WD_FAILOVER_COMMAND_LOCK);
		}

		/* execute failover command if lock holder */
		if (!pool_config->use_watchdog || wd_am_I_lock_holder())
		{
			/* Exec failover_command if needed */
			for (i = 0; i < pool_config->backend_desc->num_backends; i++)
			{
				if (nodes[i])
					trigger_failover_command(i, pool_config->failover_command,
												MASTER_NODE_ID, new_master, PRIMARY_NODE_ID);
			}

			/* unlock failover command */
			if (pool_config->use_watchdog)
				wd_unlock(WD_FAILOVER_COMMAND_LOCK);
		}


	/* no need to wait since it will be done in reap_handler */
#ifdef NOT_USED
		while (wait(NULL) > 0)
			;

		if (errno != ECHILD)
			ereport(LOG,
				(errmsg("failover_handler: wait() failed. reason:%s", strerror(errno))));

#endif

		if (reqkind == PROMOTE_NODE_REQUEST && VALID_BACKEND(node_id))
			new_primary = node_id;

		/*
		 * If the down node was a standby node in streaming replication
		 * mode, we can avoid calling find_primary_node_repeatedly() and
		 * recognize the former primary as the new primary node, which
		 * will reduce the time to process standby down.
		 */
		else if (MASTER_SLAVE && !strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP) &&
				 reqkind == NODE_DOWN_REQUEST)
		{
			if (Req_info->primary_node_id != node_id)
				new_primary = Req_info->primary_node_id;
			else
				new_primary =  find_primary_node_repeatedly();
		}
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
				reqkind == PROMOTE_NODE_REQUEST)
			{
				/* only if the failover is against the current primary */
				if (((reqkind == NODE_DOWN_REQUEST) &&
					 (nodes[Req_info->primary_node_id])) ||
					((reqkind == PROMOTE_NODE_REQUEST) &&
					 (VALID_BACKEND(node_id)))) {

					for (i = 0; i < pool_config->backend_desc->num_backends; i++)
					{
						/* do not degenerate the new primary */
						if ((new_primary >= 0) && (i != new_primary)) {
							BackendInfo *bkinfo;
							bkinfo = pool_get_node_info(i);
							ereport(LOG,
									(errmsg("starting follow degeneration. shutdown host %s(%d)",
									 bkinfo->backend_hostname,
									 bkinfo->backend_port)));
							bkinfo->backend_status = CON_DOWN;	/* set down status */
							follow_cnt++;
						}
					}

					if (follow_cnt == 0)
					{
						ereport(LOG,
								(errmsg("failover: no follow backends are degenerated")));
					}
					else
					{
						/* update new master node */
						new_master = get_next_master_node();
						ereport(LOG,
								(errmsg("failover: %d follow backends have been degenerated", follow_cnt)));
					}
				}
			}
		}


		/* wait for follow_master_command lock or to be lock holder */
		if (pool_config->use_watchdog && !wd_am_I_lock_holder())
		{
			wd_wait_for_lock(WD_FOLLOW_MASTER_COMMAND_LOCK);
		}

		/* execute follow_master_command */
		if (!pool_config->use_watchdog || wd_am_I_lock_holder())
		{
			if ((follow_cnt > 0) && (*pool_config->follow_master_command != '\0'))
			{
				follow_pid = fork_follow_child(Req_info->master_node_id, new_primary,
											Req_info->primary_node_id);
			}

			/* unlock follow_master_command  */
			if (pool_config->use_watchdog)
				wd_unlock(WD_FOLLOW_MASTER_COMMAND_LOCK);
		}

		/* end of command inter-lock */
		if (pool_config->use_watchdog)
			wd_end_interlock();

		/* Save primary node id */
		Req_info->primary_node_id = new_primary;
		ereport(LOG,
				(errmsg("failover: set new primary node: %d", Req_info->primary_node_id)));

		if (new_master >= 0)
		{
			Req_info->master_node_id = new_master;
			ereport(LOG,
					(errmsg("failover: set new master node: %d", Req_info->master_node_id)));
		}


		/* Fork the children if needed */
		if (need_to_restart_children)
		{
			for (i=0;i<pool_config->num_init_children;i++)
			{

				/*
				 * Try to kill pgpool child because previous kill signal
				 * may not be received by pgpool child. This could happen
				 * if multiple PostgreSQL are going down (or even starting
				 * pgpool, without starting PostgreSQL can trigger this).
				 * Child calls degenerate_backend() and it tries to aquire
				 * semaphore to write a failover request. In this case the
				 * signal mask is set as well, thus signals are never
				 * received.
				 */
				kill(process_info[i].pid, SIGQUIT);

				process_info[i].pid = fork_a_child(fds, i);
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

		if (reqkind == NODE_UP_REQUEST)
		{
			ereport(LOG,
					(errmsg("failback done. reconnect host %s(%d)",
					 BACKEND_INFO(node_id).backend_hostname,
					 BACKEND_INFO(node_id).backend_port)));
		}
		else if (reqkind == PROMOTE_NODE_REQUEST)
		{
			ereport(LOG,
					(errmsg("promotion done. promoted host %s(%d)",
					 BACKEND_INFO(node_id).backend_hostname,
					 BACKEND_INFO(node_id).backend_port)));
		}
		else
		{
			/* Temporary black magic. Without this regression 055 does not finish */
			fprintf(stderr, "failover done. shutdown host %s(%d)",
					 BACKEND_INFO(node_id).backend_hostname,
					BACKEND_INFO(node_id).backend_port);

			ereport(LOG,
					(errmsg("failover done. shutdown host %s(%d)",
					 BACKEND_INFO(node_id).backend_hostname,
					 BACKEND_INFO(node_id).backend_port)));
		}
		need_to_restart_pcp = true;
	}
	switching = 0;
	Req_info->switching = false;

	/* kick wakeup_handler in pcp_child to notice that
	 * failover/failback done
	 */
	kill(pcp_pid, SIGUSR2);

	if(need_to_restart_pcp)
	{
		sleep(1);

		/*
		 * Send restart request to pcp child.
		 */
		kill(pcp_pid, SIGUSR1);
		for (;;)
		{
			sts = waitpid(pcp_pid, &status, 0);
			if (sts != -1)
				break;
			if (sts == -1)
			{
				if (errno == EINTR)
					continue;
				else
				{
					ereport(WARNING,
							(errmsg("failover: waitpid failed. reason: %s", strerror(errno))));
					continue;
				}
			}
		}
		if (WIFSIGNALED(status))
			ereport(LOG,
					(errmsg("PCP child %d exits with status %d by signal %d in failover()", pcp_pid, status, WTERMSIG(status))));
		else
			ereport(LOG,
					(errmsg("PCP child %d exits with status %d in failover()", pcp_pid, status)));

		pcp_pid = pcp_fork_a_child(pcp_unix_fd, pcp_inet_fd, pcp_conf_file);
		ereport(LOG,
				(errmsg("fork a new PCP child pid %d in failover()", pcp_pid)));
	}
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
 * do_health_check() performs the health check on all backend nodes.
 * The inout parameter health_check_node_id is the starting backend
 * node number for health check and when the function returns or
 * exits with an error health_check_node_id contains the value
 * of last backend node number on which health check was performed.
 *
 * Function returns false if all backend nodes are down and true if all
 * backend nodes are in healthy state
 */
static bool
do_health_check(bool use_template_db, volatile int *health_check_node_id)
{
	POOL_CONNECTION_POOL_SLOT *slot;
	BackendInfo *bkinfo;
	static char *dbname;
	int i;
	bool all_nodes_healthy = false;

	/* Do not execute health check during recovery */
	if (*InRecovery)
		return false;

	dbname = use_template_db ? "template1" : "postgres";
	/*
	 * Start checking the backed nodes starting from the
	 * previously failed node
	 */
	for (i=*health_check_node_id;i<pool_config->backend_desc->num_backends;i++)
	{
		*health_check_node_id = i;
		/*
		 * Make sure that health check timer has not been expired.
		 * Before called health_check(), health_check_timer_expired is
		 * set to 0.  However it is possible that while processing DB
		 * nodes health check timer expired.
		 */
		if (health_check_timer_expired)
		{
			ereport(ERROR,
				(errmsg("health check timer has been already expired before attempting to connect backend node %d", i)));
		}

		bkinfo = pool_get_node_info(i);

		ereport(DEBUG1,
			(errmsg("Backend DB node %d status is %d", i, bkinfo->backend_status)));


		if (bkinfo->backend_status == CON_UNUSED ||
			bkinfo->backend_status == CON_DOWN)
			continue;

		all_nodes_healthy = true;
		ereport(DEBUG1,
			(errmsg("Trying to make persistent DB connection to backend node %d having status %d", i, bkinfo->backend_status)));

		slot = make_persistent_db_connection(bkinfo->backend_hostname,
											 bkinfo->backend_port,
											 dbname,
											 pool_config->health_check_user,
											 pool_config->health_check_password, false);

		ereport(DEBUG1,
			(errmsg("persistent DB connection to backend node %d having status %d is successful", i, bkinfo->backend_status)));

		discard_persistent_db_connection(slot);
	}
	return all_nodes_healthy;
}

/*
 * handle SIGCHLD
 */
static RETSIGTYPE reap_handler(int sig)
{
	POOL_SETMASK(&BlockSig);
	sigchld_request = 1;
	if(write(pipe_fds[1], "\0", 1) < 0)
        ereport(WARNING,
            (errmsg("reap_handler: write to pipe failed with error \"%s\"", strerror(errno))));

	POOL_SETMASK(&UnBlockSig);
}

/*
 * pool_waitpid:
 * nothing more than a wrapper function over NOHANG mode waitpid() or wait3()
 * depending on the existance of waitpid in the system
 */
static pid_t pool_waitpid(int *status)
{
#ifdef HAVE_WAITPID
	return waitpid(-1, status, WNOHANG);
#else
	return wait3(status, WNOHANG, NULL);
#endif
}

/*
 * process_name_from_pid()
 * helper function for reaper() to report the terminating child process type name
 */
static char* process_name_from_pid(pid_t pid)
{
	if (pid == pcp_pid)
		return "PCP child";
	if (pid == worker_pid)
		return "worker child";
	if (pool_config->use_watchdog && wd_is_watchdog_pid(pid))
		return wd_process_name_from_pid(pid);
	return "child";
}
/*
 * Attach zombie processes and restart child processes.
 * reaper() must be called protected from signals.
 * Note:
 * In pgpool child can exit in two ways, either by some signal or by
 * calling exit() system function.
 * For the case of child terminating due to a signal the reaper() function
 * always forks a new respective type of child process. But for the case when
 * child got terminated by exit() system call than the function checks the exit
 * code and if the child was exited by POOL_EXIT_FATAL than we do not restarts the
 * terminating child but shutdowns the pgpool-II. This allow
 * the child process to inform parent process of fatal failures which needs
 * to be rectified (e.g startup failure) by user for smooth running of system.
 * Also the child exits with success status POOL_EXIT_SUCCESS does not gets
 * restarted.
 */
static void reaper(void)
{
	pid_t pid;
	int status;
	int i;

	ereport(DEBUG1,
		(errmsg("reaper handler")));

	if (exiting)
	{
		ereport(DEBUG1,
				(errmsg("reaper handler: exited because already in exiting mode")));
		return;
	}

	if (switching)
	{
		ereport(DEBUG1,
				(errmsg("reaper handler: exited due to switching")));
		return;
	}

	/* clear SIGCHLD request */
	sigchld_request = 0;

	while ((pid = pool_waitpid(&status)) > 0)
	{
		pid_t new_pid = 0;
		bool shutdown_system = false;
		bool restart_child = true;
		char *exiting_process_name = process_name_from_pid(pid);

		/*
		 * Check if the terminating child wants pgpool main to go down with it
		 */
		if(WIFEXITED(status))
		{
			if(WEXITSTATUS(status) == POOL_EXIT_FATAL)
			{
				ereport(DEBUG1,
						(errmsg("%s process with pid: %d exit with FATAL ERROR. pgpool-II will be shutdown",exiting_process_name, pid)));
				shutdown_system = true;
			}
			else if(WEXITSTATUS(status) == POOL_EXIT_SUCCESS)
			{
				ereport(DEBUG1,
						(errmsg("%s process with pid: %d exit with SUCCESS. child will not be restarted",exiting_process_name, pid)));

				restart_child = false;
			}
		}
		if (WIFSIGNALED(status))
		{
			/* Child terminated by segmentation fault. Report it */
			if(WTERMSIG(status) == SIGSEGV)
				ereport(WARNING,
					(errmsg("%s process with pid: %d was terminated by segmentation fault", exiting_process_name,pid)));
			else
				ereport(LOG,
						(errmsg("%s process with pid: %d exits with status %d by signal %d", exiting_process_name, pid, status, WTERMSIG(status))));
		}
		else
			ereport(LOG,
					(errmsg("%s process with pid: %d exits with status %d", exiting_process_name,pid, status)));

		/* if exiting child process was PCP handler */
		if (pid == pcp_pid)
		{
			if(restart_child)
			{
				pcp_pid = pcp_fork_a_child(pcp_unix_fd, pcp_inet_fd, pcp_conf_file);
				new_pid = pcp_pid;
			}
			else
				pcp_pid = 0;
		}

		/* exiting process was worker process */
		else if (pid == worker_pid)
		{
			if(restart_child)
			{
				worker_pid = worker_fork_a_child();
				new_pid = worker_pid;
			}
			else
				worker_pid = 0;
		}

		/* exiting process was watchdog process */
		else if (pool_config->use_watchdog && wd_is_watchdog_pid(pid))
		{
			new_pid = wd_reaper_watchdog(pid, restart_child);
		}
		else
		{
			/* look for exiting child's pid */
			for (i=0;i<pool_config->num_init_children;i++)
			{
				if (pid == process_info[i].pid)
				{
					/* if found, fork a new child */
					if (!switching && !exiting && restart_child)
					{
						process_info[i].pid = fork_a_child(fds, i);
						process_info[i].start_time = time(NULL);
						new_pid = process_info[i].pid;
					}
					else
						process_info[i].pid = 0;
					break;
				}
			}
		}
		if(shutdown_system)
			ereport(FATAL,
				(errmsg("%s process exit with fatal error. exiting pgpool-II",exiting_process_name)));
		else if(restart_child && new_pid)
		{
			/* Report if the child was restarted */
			ereport(LOG,
					(errmsg("fork a new %s process with pid: %d",exiting_process_name, new_pid)));
		}
		else
		{
			/* And the child was not restarted */
			ereport(LOG,
					(errmsg("%s process with pid: %d exited with success and will not be restarted", exiting_process_name, pid)));
		}

	}
	ereport(DEBUG1,
			(errmsg("reaper handler: exiting normally")));
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
	array = palloc0(*array_size * sizeof(int));
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
	if(write(pipe_fds[1], "\0", 1) < 0)
        ereport(WARNING,
            (errmsg("wakeup_handler: write to pipe failed with error \"%s\"", strerror(errno))));
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
	if(write(pipe_fds[1], "\0", 1) < 0)
        ereport(WARNING,
            (errmsg("reload_config_handler: write to pipe failed with error \"%s\"", strerror(errno))));

	POOL_SETMASK(&UnBlockSig);
}

static void kill_all_children(int sig)
{
	int i;
    if(process_info)
    {
        /* kill all children */
        for (i = 0; i < pool_config->num_init_children; i++)
        {
            pid_t pid = process_info[i].pid;
            if (pid)
            {
                kill(pid, sig);
            }
        }
    }
	/* make PCP process reload as well */
	if (sig == SIGHUP && pcp_pid > 0)
		kill(pcp_pid, sig);
}

/*
 * pause in a period specified by timeout. If any data is coming
 * through pipe_fds[0], that means one of: failover request(SIGUSR1),
 * SIGCHLD received, children wake up request(SIGUSR2 used in on line
 * recovery processing) or config file reload request(SIGHUP) has been
 * occurred.  In this case this function returns 1.
 * otherwise 0: (no signal event occurred), -1: (error)
 * XXX: is it OK that select(2) error is ignored here?
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
    {
		if(read(pipe_fds[0], &dummy, 1) < 0)
            ereport(WARNING,
                (errmsg("pool_pause: read on pipe failed with error \"%s\"", strerror(errno))));
    }
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

	/* check failed nodeID */
	if (node < 0 || node >= NUM_BACKENDS)
		return -1;

	info = pool_get_node_info(node);
	if (!info)
		return -1;

	buf[1] = '\0';
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
						newmaster = pool_get_node_info(new_master);
						if (newmaster)
							string_append_char(exec_cmd, newmaster->backend_hostname);
						else
							/* no valid new master */
							string_append_char(exec_cmd, "");
						break;

					case 'm': /* new master node id */
						snprintf(port_buf, sizeof(port_buf), "%d", new_master);
						string_append_char(exec_cmd, port_buf);
						break;

					case 'r': /* new master port */
						newmaster = pool_get_node_info(get_next_master_node());
						if (newmaster)
						{
							snprintf(port_buf, sizeof(port_buf), "%d", newmaster->backend_port);
							string_append_char(exec_cmd, port_buf);
						}
						else
							/* no valid new master */
							string_append_char(exec_cmd, "");
						break;

					case 'R': /* new master database directory */
						newmaster = pool_get_node_info(get_next_master_node());
						if (newmaster)
							string_append_char(exec_cmd, newmaster->backend_data_directory);
						else
							/* no valid new master */
							string_append_char(exec_cmd, "");
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
		ereport(LOG,
                (errmsg("execute command: %s", exec_cmd->data)));
		r = system(exec_cmd->data);
	}

	return r;
}
/*
 * This function is used by find_primary_node() function and is just a wrapper
 * over make_persistent_db_connection() function and returns boolean value to 
 * inform connection status
 * This function must not throws ereport
 */
static bool
    verify_backend_node_status(int backend_no, bool* is_standby)
{
	POOL_CONNECTION_POOL_SLOT   *s = NULL;
	POOL_CONNECTION *con;
	POOL_SELECT_RESULT *res;
	BackendInfo *bkinfo = pool_get_node_info(backend_no);

	*is_standby = false;

	s = make_persistent_db_connection_noerror(bkinfo->backend_hostname,
										  bkinfo->backend_port,
										  "postgres",
										  pool_config->sr_check_user,
										  pool_config->sr_check_password, true);
	if (s)
	{
		MemoryContext oldContext = CurrentMemoryContext;
		con = s->con;

		PG_TRY();
		{
			do_query(con, "SELECT pg_is_in_recovery()",
					 &res, PROTO_MAJOR_V3);
		}
		PG_CATCH();
		{
			/* ignore the error message */
			res = NULL;
			MemoryContextSwitchTo(oldContext);
			FlushErrorState();
			ereport(LOG,
					(errmsg("verify_backend_node_status: do_query failed")));
		}
		PG_END_TRY();
		if(res)
		{
			if (res->numrows <= 0)
			{
				ereport(LOG,
						(errmsg("verify_backend_node_status: do_query returns no rows")));
			}
			if (res->data[0] == NULL)
			{
				ereport(LOG,
						(errmsg("verify_backend_node_status: do_query returns no data")));
			}
			if (res->nullflags[0] == -1)
			{
				ereport(LOG,
						(errmsg("verify_backend_node_status: do_query returns NULL")));
			}
			if (res->data[0] && !strcmp(res->data[0], "t"))
			{
				*is_standby = true;
			}
			free_select_result(res);
		}
		discard_persistent_db_connection(s);
		return true;
	}
	return false;
}

/*
 * Find the primary node (i.e. not standby node) and returns its node
 * id. If no primary node is found, returns -1.
 */
static int find_primary_node(void)
{
	int i;

	/* Streaming replication mode? */
	if (pool_config->master_slave_mode == 0 ||
		strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP))
	{
		/* No point to look for primary node if not in streaming
		 * replication mode.
		 */
		ereport(DEBUG1,
				(errmsg("find_primary_node: not in streaming replication mode")));
		return -1;
	}

	for(i=0;i<NUM_BACKENDS;i++)
	{
		bool node_status;
		bool is_standby;

		ereport(LOG,
				(errmsg("find_primary_node: checking backend no %d\n",i)));

		if (!VALID_BACKEND(i))
			continue;
		node_status = verify_backend_node_status(i,&is_standby);
        if (!node_status)
        {
            /*
             * It is possible that a node is down even if
             * VALID_BACKEND tells it's valid.  This could happen
             * before health checking detects the failure.
             * Thus we should continue to look for primary node.
             */
            continue;
        }
		if (is_standby)
			ereport(DEBUG1,
					(errmsg("find_primary_node: %d node is standby", i)));
		else
			break;
	}

	if (i == NUM_BACKENDS)
	{
		ereport(DEBUG1,
				(errmsg("find_primary_node: no primary node found")));
		return -1;
	}

	ereport(LOG,
            (errmsg("find_primary_node: primary node id is %d", i)));
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
		ereport(DEBUG1, 
			(errmsg("find_primary_node: not in streaming replication mode")));
		return -1;
	}

	/*
	 * Try to find the new primary node and keep trying for
	 * search_primary_node_timeout seconds.
	 * search_primary_node_timeout = 0 means never timeout and keep searching
	 * indefinitely
	 */
	ereport(LOG,
		(errmsg("find_primary_node_repeatedly: waiting for finding a primary node")));
	for (sec = 0; (pool_config->search_primary_node_timeout == 0 ||
				sec < pool_config->search_primary_node_timeout); sec++)
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
pid_t fork_follow_child(int old_master, int new_primary, int old_primary)
{
	pid_t pid;
	int i;

	pid = fork();

	if (pid == 0)
	{
		on_exit_reset();
		processType = PT_FOLLOWCHILD;

		ereport(LOG,
			(errmsg("start triggering follow command.")));
		for (i = 0; i < pool_config->backend_desc->num_backends; i++)
		{
			BackendInfo *bkinfo;
			bkinfo = pool_get_node_info(i);
			if (bkinfo->backend_status == CON_DOWN)
				trigger_failover_command(i, pool_config->follow_master_command,
										 old_master, new_primary, old_primary);
		}
		exit(0);
	}
	else if (pid == -1)
	{
		ereport(WARNING,
				(errmsg("follow fork() failed with reason: \"%s\"", strerror(errno))));
		exit(1);
	}
	return pid;
}


static void initialize_shared_mem_objects(bool clear_memcache_oidmaps)
{
	int size,i;
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
	memset(con_info, 0, size);

	size = pool_config->num_init_children * (sizeof(ProcessInfo));
	process_info = pool_shared_memory_create(size);
	memset(process_info, 0, size);

	for (i = 0; i < pool_config->num_init_children; i++)
	{
		process_info[i].connection_info = pool_coninfo(i,0,0);
	}

	/* create fail over/switch over event area */
	Req_info = pool_shared_memory_create(sizeof(POOL_REQUEST_INFO));

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
	Req_info->master_node_id = get_next_master_node();
	Req_info->conn_counter = 0;
	Req_info->switching = false;
	Req_info->request_queue_head = Req_info->request_queue_tail = -1;
	InRecovery = pool_shared_memory_create(sizeof(int));
	*InRecovery = RECOVERY_INIT;

	/*
	 * Initialize shared memory cache
	 */
	if (pool_config->memory_cache_enabled)
	{
		if (pool_is_shmem_cache())
		{
			size_t size;

			size = pool_shared_memory_cache_size();
			pool_init_memory_cache(size);

			size = pool_shared_memory_fsmm_size();
			if (size == 0)
				ereport(FATAL,
					(errmsg("invalid shared memory size"),
						errdetail("pool_shared_memory_fsmm_size error")));
	
			pool_init_fsmm(size);

			pool_allocate_fsmm_clock_hand();

			pool_discard_oid_maps();

			ereport(LOG,
				(errmsg("pool_discard_oid_maps: discarded memqcache oid maps")));

			pool_hash_init(pool_config->memqcache_max_num_cache);
		}

#ifdef USE_MEMCACHED
		else
		{
			if (clear_memcache_oidmaps)
			{
				pool_discard_oid_maps();
				ereport(LOG,
					(errmsg("discarded memqcache oid maps")));
			}
			else
			{
				ereport(DEBUG1,
					(errmsg("skipped discarding memqcache oid maps")));
			}
		}
#endif

		pool_init_memqcache_stats();
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
	bool is_old_format;

	snprintf(fnamebuf, sizeof(fnamebuf), "%s/%s", pool_config->logdir, STATUS_FILE_NAME);
	fd = fopen(fnamebuf, "r");
	if (!fd)
	{
		ereport(LOG,
                (errmsg("Backend status file %s does not exist", fnamebuf)));
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
			ereport(LOG,
                    (errmsg("Backend status file %s discarded", fnamebuf)));
		else
			ereport(WARNING,
					(errmsg("failed to discard backend status file: \"%s\" with reason: \"%s\"", fnamebuf, strerror(errno))));
		return 0;
	}

	/*
	 * Frist try out with old format file.
	 */
	is_old_format = true;

	if (fread(&backend_rec, 1, sizeof(backend_rec), fd) == sizeof(backend_rec))
	{
		/* It's likely old binary format status file */
		for (i=0;i< pool_config->backend_desc->num_backends;i++)
		{
			if (backend_rec.status[i] == CON_DOWN)
			{
				BACKEND_INFO(i).backend_status = CON_DOWN;
				ereport(LOG,
						(errmsg("read_status_file: %d th backend is set to down status", i)));
			}
			else if (BACKEND_INFO(i).backend_status == CON_CONNECT_WAIT ||
					 BACKEND_INFO(i).backend_status == CON_UP)
			{
				BACKEND_INFO(i).backend_status = CON_CONNECT_WAIT;
				someone_wakeup = true;
			}
			else
			{
				/* It seems it's not an old binary format status file */
				is_old_format = false;
				break;
			}
		}
	}
	else
		is_old_format = false;

	fclose(fd);

	if (!is_old_format)
	{
		/*
		 * Fall back to new ascii format file.
		 * the format looks like(case is ignored):
		 * 
		 * up|down|unused
		 * UP|down|unused
		 *   :
		 *   :
		 */
#define MAXLINE 10
		char readbuf[MAXLINE];

		fd = fopen(fnamebuf, "r");
		if (!fd)
		{
			ereport(LOG,
					(errmsg("Backend status file %s does not exist", fnamebuf)));
			return -1;
		}

		for (i=0;i<MAX_NUM_BACKENDS;i++)
		{
			BACKEND_INFO(i).backend_status = CON_UNUSED;
		}
			  
		for (i=0;;i++)
		{
			readbuf[MAXLINE-1] = '\0';
			if (fgets(readbuf, MAXLINE-1, fd) == 0)
				break;

			if (!strncasecmp("up", readbuf, 2))
			{
				BACKEND_INFO(i).backend_status = CON_UP;
				someone_wakeup = true;
			}
			else if (!strncasecmp("down", readbuf, 4))
			{
				BACKEND_INFO(i).backend_status = CON_DOWN;
				ereport(LOG,
						(errmsg("reading status file: %d th backend is set to down status", i)));
			}
			else if (!strncasecmp("unused", readbuf, 6))
			{
				BACKEND_INFO(i).backend_status = CON_UNUSED;
			}
			else
			{
				ereport(WARNING,
					(errmsg("invalid data in status file, ignoring..."),
						 errdetail("backend:%d status is invalid: \"%s\"",i,readbuf)));
			}
		}
		fclose(fd);
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
static int write_status_file()
{
	FILE *fd;
	char fnamebuf[POOLMAXPATHLEN];
	int i;

	snprintf(fnamebuf, sizeof(fnamebuf), "%s/%s", pool_config->logdir, STATUS_FILE_NAME);
	fd = fopen(fnamebuf, "w");
	if (!fd)
	{
		ereport(WARNING,
			(errmsg("failed to open status file at: \"%s\"",fnamebuf),
				 errdetail("\"%s\"",strerror(errno))));
		return -1;
	}

    if(pool_config)
    {
		char buf[10];

        for (i=0;i< pool_config->backend_desc->num_backends;i++)
        {
			char* status;

			if (BACKEND_INFO(i).backend_status == CON_UP ||
				BACKEND_INFO(i).backend_status == CON_CONNECT_WAIT)
				status = "up";
			else if (BACKEND_INFO(i).backend_status == CON_DOWN)
				status = "down";
			else
				status = "unused";

			sprintf(buf, "%s\n", status);
			if (fwrite(buf, 1, strlen(buf), fd) != strlen(buf))
			{
				ereport(WARNING,
					(errmsg("failed to write status file at: \"%s\"",fnamebuf),
						 errdetail("\"%s\"",strerror(errno))));
				fclose(fd);
				return -1;
			}
		}

        if (fflush(fd) != 0)
        {
			ereport(WARNING,
				(errmsg("failed to write status file at: \"%s\"",fnamebuf),
					 errdetail("\"%s\"",strerror(errno))));
            fclose(fd);
            return -1;
        }
        fclose(fd);
    }
	return 0;
}


static void reload_config(void)
{
	ereport(LOG,
		(errmsg("reload config files.")));
    MemoryContext oldContext = MemoryContextSwitchTo(TopMemoryContext);
	pool_get_config(conf_file, RELOAD_CONFIG);
    MemoryContextSwitchTo(oldContext);
	if (pool_config->enable_pool_hba)
		load_hba(hba_file);
	if (pool_config->parallel_mode)
		pool_memset_system_db_info(system_db_info->info);
	kill_all_children(SIGHUP);

	if (worker_pid)
		kill(worker_pid, SIGHUP);
}

/* Call back function to unlink the file */
static void FileUnlink(int code, Datum path)
{
	char* filePath = (char*)path; 
	if (unlink(filePath) == 0) return;
	/* 
	 * We are already exiting the system just produce a log entry to report an error
	 */
	ereport(LOG,
		(errmsg("unlink failed for file at path \"%s\"", filePath),
		errdetail("%s", strerror(errno))));
}

static void system_will_go_down(int code, Datum arg)
{
    if(mypid != getpid())
    {
        /* should never happen */
        ereport(LOG,(errmsg("system_will_go_down called from invalid process")));
        return;
    }
    POOL_SETMASK(&AuthBlockSig);
    /* Write status file */
    write_status_file();
    /* 
     * Terminate all childrens. But we may already have killed
     * all the childrens if we come to this function because of shutdown
     * signal.
     */
    if(processState != EXITING)
        terminate_all_childrens();
    processState = EXITING;
    POOL_SETMASK(&UnBlockSig);
    
}
