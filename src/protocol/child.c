/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2013	PgPool Global Development Group
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
 * child.c: child process main
 *
 */
#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include <signal.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#include "pool.h"
#include "context/pool_process_context.h"
#include "context/pool_session_context.h"
#include "pool_config.h"
#include "utils/pool_ip.h"
#include "utils/pool_stream.h"
#include "auth/md5.h"
#include "auth/pool_passwd.h"

static POOL_CONNECTION *do_accept(int unix_fd, int inet_fd, struct timeval *timeout);
static StartupPacket *read_startup_packet(POOL_CONNECTION *cp);
static POOL_CONNECTION_POOL *connect_backend(StartupPacket *sp, POOL_CONNECTION *frontend);
static RETSIGTYPE die(int sig);
static RETSIGTYPE close_idle_connection(int sig);
static RETSIGTYPE wakeup_handler(int sig);
static RETSIGTYPE reload_config_handler(int sig);
static RETSIGTYPE authentication_timeout(int sig);
static int send_params(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
static void send_frontend_exits(void);
static int s_do_auth(POOL_CONNECTION_POOL_SLOT *cp, char *password);
static void connection_count_up(void);
static void connection_count_down(void);
static void init_system_db_connection(void);
static bool connect_using_existing_connection(POOL_CONNECTION *frontend,
											  POOL_CONNECTION_POOL *backend,
											  StartupPacket *sp);
/*
 * non 0 means SIGTERM(smart shutdown) or SIGINT(fast shutdown) has arrived
 */
volatile sig_atomic_t exit_request = 0;

static int idle;		/* non 0 means this child is in idle state */
static int accepted = 0;

extern int myargc;
extern char **myargv;

char remote_ps_data[NI_MAXHOST];		/* used for set_ps_display */

volatile sig_atomic_t got_sighup = 0;

char remote_host[NI_MAXHOST];	/* client host */
char remote_port[NI_MAXSERV];	/* client port */

/*
* child main loop
*/
void do_child(int unix_fd, int inet_fd)
{
	POOL_CONNECTION *frontend;
	POOL_CONNECTION_POOL *backend;
	struct timeval now;
	struct timezone tz;
	struct timeval timeout;
	static int connected;		/* non 0 if has been accepted connections from frontend */
	int connections_count = 0;	/* used if child_max_connections > 0 */
	int found;
	char psbuf[NI_MAXHOST + 128];

	pool_debug("I am %d", getpid());

	/* Identify myself via ps */
	init_ps_display("", "", "", "");

	/* set up signal handlers */
	signal(SIGALRM, SIG_DFL);
	signal(SIGTERM, die);
	signal(SIGINT, die);
	signal(SIGHUP, reload_config_handler);
	signal(SIGQUIT, die);
	signal(SIGCHLD, SIG_DFL);
	signal(SIGUSR1, close_idle_connection);
	signal(SIGUSR2, wakeup_handler);
	signal(SIGPIPE, SIG_IGN);

#ifdef NONE_BLOCK
	/* set listen fds to none-blocking */
	pool_set_nonblock(unix_fd);
	if (inet_fd)
	{
		pool_set_nonblock(inet_fd);
	}
#endif

	/* Initialize my backend status */
	pool_initialize_private_backend_status();

	/* Initialize per process context */
	pool_init_process_context();

	/* initialize random seed */
	gettimeofday(&now, &tz);
#if defined(sun) || defined(__sun)
	srand((unsigned int) now.tv_usec);
#else
	srandom((unsigned int) now.tv_usec);
#endif

	/* initialize system db connection */
	init_system_db_connection();

	/* initialize connection pool */
	if (pool_init_cp())
	{
		child_exit(1);
	}

	/*
	 * Open pool_passwd in child process.  This is necessary to avoid the
	 * file descriptor race condition reported in [pgpool-general: 1141].
	 */
	if (strcmp("", pool_config->pool_passwd))
	{
		pool_reopen_passwd_file();
	}

	timeout.tv_sec = pool_config->child_life_time;
	timeout.tv_usec = 0;

	for (;;)
	{
		StartupPacket *sp;

		idle = 1;

		/* pgpool stop request already sent? */
		check_stop_request();

		/* Check if restart request is set because of failback event
		 * happend.  If so, exit myself with exit code 1 to be
		 * restarted by pgpool parent.
		 */
		if (pool_get_my_process_info()->need_to_restart)
		{
			pool_log("do_child: failback event found. restart myself.");
			pool_get_my_process_info()->need_to_restart = 0;
			child_exit(1);
		}

		accepted = 0;

		/* perform accept() */
		frontend = do_accept(unix_fd, inet_fd, &timeout);

		if (frontend == NULL)	/* connection request from frontend timed out */
		{
			/* check select() timeout */
			if (connected && pool_config->child_life_time > 0 &&
				timeout.tv_sec == 0 && timeout.tv_usec == 0)
			{
				pool_debug("child life %d seconds expired", pool_config->child_life_time);
				/*
				 * Doesn't need to call this. child_exit() calls it.
				 * send_frontend_exits();
				 */
				child_exit(2);
			}
			continue;
		}

		/* set frontend fd to blocking */
		pool_unset_nonblock(frontend->fd);

		/* reset busy flag */
		idle = 0;

		/* check backend timer is expired */
		if (backend_timer_expired)
		{
			pool_backend_timer();
			backend_timer_expired = 0;
		}

		/* read the startup packet */
	retry_startup:
		sp = read_startup_packet(frontend);
		if (sp == NULL)
		{
			/* failed to read the startup packet. return to the accept() loop */
			pool_close(frontend);
			connection_count_down();
			continue;
		}

		/* cancel request? */
		if (sp->major == 1234 && sp->minor == 5678)
		{
			cancel_request((CancelPacket *)sp->startup_packet);

			pool_close(frontend);
			pool_free_startup_packet(sp);
			connection_count_down();
			continue;
		}

		/* SSL? */
		if (sp->major == 1234 && sp->minor == 5679 && !frontend->ssl_active)
		{
			pool_debug("SSLRequest from client");
			pool_ssl_negotiate_serverclient(frontend);
			goto retry_startup;
		}

		if (pool_config->enable_pool_hba)
		{
			/*
			 * do client authentication.
			 * Note that ClientAuthentication does not return if frontend
			 * was rejected; it simply terminates this process.
			 */
			frontend->protoVersion = sp->major;
			frontend->database = strdup(sp->database);
			if (frontend->database == NULL)
			{
				pool_error("do_child: strdup failed: %s\n", strerror(errno));
				child_exit(1);
			}
			frontend->username = strdup(sp->user);
			if (frontend->username == NULL)
			{
				pool_error("do_child: strdup failed: %s\n", strerror(errno));
				child_exit(1);
			}
			ClientAuthentication(frontend);
		}

		/*
		 * Ok, negotiation with frontend has been done. Let's go to the
		 * next step.  Connect to backend if there's no existing
		 * connection which can be reused by this frontend.
		 * Authentication is also done in this step.
		 */

		/* Check if restart request is set because of failback event
		 * happend.  If so, close idle connections to backend and make
		 * a new copy of backend status.
		 */
		if (pool_get_my_process_info()->need_to_restart)
		{
			pool_log("do_child: failback event found. discard existing connections");
			pool_get_my_process_info()->need_to_restart = 0;
			close_idle_connection(0);
			pool_initialize_private_backend_status();
		}

		/*
		 * if there's no connection associated with user and database,
		 * we need to connect to the backend and send the startup packet.
		 */

		/* look for existing connection */
		found = 0;
		backend = pool_get_cp(sp->user, sp->database, sp->major, 1);

		if (backend != NULL)
		{
			found = 1;

			/* existing connection associated with same user/database/major found.
			 * however we should make sure that the startup packet contents are identical.
			 * OPTION data and others might be different.
			 */
			if (sp->len != MASTER_CONNECTION(backend)->sp->len)
			{
				pool_debug("do_child: connection exists but startup packet length is not identical");
				found = 0;
			}
			else if(memcmp(sp->startup_packet, MASTER_CONNECTION(backend)->sp->startup_packet, sp->len) != 0)
			{
				pool_debug("do_child: connection exists but startup packet contents is not identical");
				found = 0;
			}

			if (found == 0)
			{
				/* we need to discard existing connection since startup packet is different */
				pool_discard_cp(sp->user, sp->database, sp->major);
				backend = NULL;
			}
		}

		if (backend == NULL)
		{
			/* create a new connection to backend */
			if ((backend = connect_backend(sp, frontend)) == NULL)
			{
				connection_count_down();
				continue;
			}
		}

		else
		{
			/* reuse existing connection */
			if (!connect_using_existing_connection(frontend, backend, sp))
				continue;
		}

		connected = 1;

 		/* show ps status */
		sp = MASTER_CONNECTION(backend)->sp;
		snprintf(psbuf, sizeof(psbuf), "%s %s %s idle",
				 sp->user, sp->database, remote_ps_data);
		set_ps_display(psbuf, false);

		/*
		 * Initialize per session context
		 */
		pool_init_session_context(frontend, backend);

		/* Mark this connection pool is connected from frontend */
		pool_coninfo_set_frontend_connected(pool_get_process_context()->proc_id, pool_pool_index());

		/* query process loop */
		for (;;)
		{
			POOL_STATUS status;

			status = pool_process_query(frontend, backend, 0);

			sp = MASTER_CONNECTION(backend)->sp;

			switch (status)
			{
				/* client exits */
				case POOL_END:
					/*
					 * do not cache connection if:
					 * pool_config->connection_cahe == 0 or
					 * database name is template0, template1, postgres or regression
					 */
					if (pool_config->connection_cache == 0 ||
						!strcmp(sp->database, "template0") ||
						!strcmp(sp->database, "template1") ||
						!strcmp(sp->database, "postgres") ||
						!strcmp(sp->database, "regression"))
					{
						reset_connection();
						pool_close(frontend);
						pool_send_frontend_exits(backend);
						pool_discard_cp(sp->user, sp->database, sp->major);
					}
					else
					{
						POOL_STATUS status1;

						/* send reset request to backend */
						status1 = pool_process_query(frontend, backend, 1);
						pool_close(frontend);

						/* if we detect errors on resetting connection, we need to discard
						 * this connection since it might be in unknown status
						 */
						if (status1 != POOL_CONTINUE)
						{
							pool_debug("error in resetting connections. discarding connection pools...");
							pool_send_frontend_exits(backend);
							pool_discard_cp(sp->user, sp->database, sp->major);
						}
						else
							pool_connection_pool_timer(backend);
					}
					break;

				/* error occurred. discard backend connection pool
                   and disconnect connection to the frontend */
				case POOL_ERROR:
					pool_log("do_child: exits with status 1 due to error");
					child_exit(1);
					break;

				/* fatal error occurred. just exit myself... */
				case POOL_FATAL:
					notice_backend_error(1);
					child_exit(1);
					break;

				/* not implemented yet */
				case POOL_IDLE:
					do_accept(unix_fd, inet_fd, &timeout);
					pool_debug("accept while idle");
					break;

				default:
					break;
			}

			if (status != POOL_CONTINUE)
				break;
		}

		/* Destroy session context */
		pool_session_context_destroy();

		/* Mark this connection pool is not connected from frontend */
		pool_coninfo_unset_frontend_connected(pool_get_process_context()->proc_id, pool_pool_index());

		accepted = 0;
		connection_count_down();

		timeout.tv_sec = pool_config->child_life_time;
		timeout.tv_usec = 0;

		/* increment queries counter if necessary */
		if ( pool_config->child_max_connections > 0 )
			connections_count++;

		/* check if maximum connections count for this child reached */
		if ( ( pool_config->child_max_connections > 0 ) &&
			( connections_count >= pool_config->child_max_connections ) )
		{
			pool_log("child exiting, %d connections reached", pool_config->child_max_connections);
			send_frontend_exits();
			child_exit(2);
		}
	}
	child_exit(0);
}

/* -------------------------------------------------------------------
 * private functions
 * -------------------------------------------------------------------
 */

/*
* perform accept() and return new fd
*/
static POOL_CONNECTION *do_accept(int unix_fd, int inet_fd, struct timeval *timeout)
{
    fd_set	readmask;
    int fds;
	int save_errno;

	SockAddr saddr;
	int fd = 0;
	int afd;
	int inet = 0;
	POOL_CONNECTION *cp;
#ifdef ACCEPT_PERFORMANCE
	struct timeval now1, now2;
	static long atime;
	static int cnt;
#endif
	struct timeval *timeoutval;
	struct timeval tv1, tv2, tmback = {0, 0};

	set_ps_display("wait for connection request", false);

	/* Destroy session context for just in case... */
	pool_session_context_destroy();

	FD_ZERO(&readmask);
	FD_SET(unix_fd, &readmask);
	if (inet_fd)
		FD_SET(inet_fd, &readmask);

	if (timeout->tv_sec == 0 && timeout->tv_usec == 0)
		timeoutval = NULL;
	else
	{
		timeoutval = timeout;
		tmback.tv_sec = timeout->tv_sec;
		tmback.tv_usec = timeout->tv_usec;
		gettimeofday(&tv1, NULL);

#ifdef DEBUG
		pool_log("before select = {%d, %d}", timeoutval->tv_sec, timeoutval->tv_usec);
		pool_log("g:before select = {%d, %d}", tv1.tv_sec, tv1.tv_usec);
#endif
	}

	fds = select(Max(unix_fd, inet_fd)+1, &readmask, NULL, NULL, timeoutval);

	save_errno = errno;
	/* check backend timer is expired */
	if (backend_timer_expired)
	{
		pool_backend_timer();
		backend_timer_expired = 0;
	}

	/*
	 * following code fragment computes remaining timeout val in a
	 * portable way. Linux does this automatically but other platforms do not.
	 */
	if (timeoutval)
	{
		gettimeofday(&tv2, NULL);

		tmback.tv_usec -= tv2.tv_usec - tv1.tv_usec;
		tmback.tv_sec -= tv2.tv_sec - tv1.tv_sec;

		if (tmback.tv_usec < 0)
		{
			tmback.tv_sec--;
			if (tmback.tv_sec < 0)
			{
				timeout->tv_sec = 0;
				timeout->tv_usec = 0;
			}
			else
			{
				tmback.tv_usec += 1000000;
				timeout->tv_sec = tmback.tv_sec;
				timeout->tv_usec = tmback.tv_usec;
			}
		}
#ifdef DEBUG
		pool_log("g:after select = {%d, %d}", tv2.tv_sec, tv2.tv_usec);
		pool_log("after select = {%d, %d}", timeout->tv_sec, timeout->tv_usec);
#endif
	}

	errno = save_errno;

	if (fds == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
			return NULL;

		pool_error("select() failed. reason %s", strerror(errno));
		return NULL;
	}

	/* timeout */
	if (fds == 0)
	{
		return NULL;
	}

	if (FD_ISSET(unix_fd, &readmask))
	{
		fd = unix_fd;
	}

	if (FD_ISSET(inet_fd, &readmask))
	{
		fd = inet_fd;
		inet++;
	}

	/*
	 * Note that some SysV systems do not work here. For those
	 * systems, we need some locking mechanism for the fd.
	 */
	memset(&saddr, 0, sizeof(saddr));
	saddr.salen = sizeof(saddr.addr);

#ifdef ACCEPT_PERFORMANCE
	gettimeofday(&now1,0);
#endif

 retry_accept:

	/* wait if recovery is started */
	while (*InRecovery == 1)
	{
		pause();
	}

	afd = accept(fd, (struct sockaddr *)&saddr.addr, &saddr.salen);

	save_errno = errno;
	/* check backend timer is expired */
	if (backend_timer_expired)
	{
		pool_backend_timer();
		backend_timer_expired = 0;
	}
	errno = save_errno;
	if (afd < 0)
	{
		if (errno == EINTR && *InRecovery)
			goto retry_accept;

		/*
		 * "Resource temporarily unavailable" (EAGAIN or EWOULDBLOCK)
		 * can be silently ignored. And EINTR can be ignored.
		 */
		if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
			pool_error("accept() failed. reason: %s", strerror(errno));
		return NULL;
	}
#ifdef ACCEPT_PERFORMANCE
	gettimeofday(&now2,0);
	atime += (now2.tv_sec - now1.tv_sec)*1000000 + (now2.tv_usec - now1.tv_usec);
	cnt++;
	if (cnt % 100 == 0)
	{
		pool_log("cnt: %d atime: %ld", cnt, atime);
	}
#endif

	/* reload config file */
	if (got_sighup)
	{
		pool_get_config(get_config_file_name(), RELOAD_CONFIG);
		if (pool_config->enable_pool_hba)
		{
			load_hba(get_hba_file_name());
			if (strcmp("", pool_config->pool_passwd))
				pool_reopen_passwd_file();
		}
		if (pool_config->parallel_mode)
			pool_memset_system_db_info(system_db_info->info);
		got_sighup = 0;
	}

	connection_count_up();
	accepted = 1;

	if (pool_config->parallel_mode)
	{
		/*
		 * do not accept new connection if any of DB node or SystemDB is down when operating in
		 * parallel mode
		 */
		int i;

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (BACKEND_INFO(i).backend_status == CON_DOWN || SYSDB_STATUS == CON_DOWN)
			{
				StartupPacket *sp;
				char *msg = "pgpool is not available in parallel query mode";

				if (SYSDB_STATUS == CON_DOWN)
					pool_log("Cannot accept() new connection. SystemDB is down");
				else
					pool_log("Cannot accept() new connection. %d th backend is down", i);

				if ((cp = pool_open(afd)) == NULL)
				{
					close(afd);
					child_exit(1);
				}

				sp = read_startup_packet(cp);
				if (sp == NULL)
				{
					/* failed to read the startup packet. return to the accept() loop */
					pool_close(cp);
					child_exit(1);
				}

				pool_debug("do_accept: send error message to frontend");

				if (sp->major == PROTO_MAJOR_V3)
				{
					char buf[256];

					if (SYSDB_STATUS == CON_DOWN)
						snprintf(buf, sizeof(buf), "SystemDB is down");
					else
						snprintf(buf, sizeof(buf), "%d th backend is down", i);

					pool_send_error_message(cp, sp->major, "08S01",
											msg,
											buf,
											((SYSDB_STATUS == CON_DOWN) ? "repair the SystemDB and restart pgpool"
											                           : "repair the backend and restart pgpool"),
											__FILE__,
											__LINE__);
				}
				else
				{
					pool_send_error_message(cp, sp->major,
											0,
											msg,
											"",
											"",
											"",
											0);
				}
				pool_close(cp);
				child_exit(1);
			}
		}
	}
	else
	{
		/*
		 * do not accept new connection if all DB nodes are down when operating in
		 * non parallel mode
		 */
		int i;
		int found = 0;

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (VALID_BACKEND(i))
			{
				found = 1;
			}
		}
		if (found == 0)
		{
			pool_log("Cannot accept() new connection. all backends are down");
			child_exit(1);
		}
	}

	pool_debug("I am %d accept fd %d", getpid(), afd);

	pool_getnameinfo_all(&saddr, remote_host, remote_port);
	snprintf(remote_ps_data, sizeof(remote_ps_data),
			 remote_port[0] == '\0' ? "%s" : "%s(%s)",
			 remote_host, remote_port);

	set_ps_display("accept connection", false);

	/* log who is connecting */
	if (pool_config->log_connections)
	{
		pool_log("connection received: host=%s%s%s",
				 remote_host, remote_port[0] ? " port=" : "", remote_port);
	}

	/* set NODELAY and KEEPALIVE options if INET connection */
	if (inet)
	{
		int on = 1;

		if (setsockopt(afd, IPPROTO_TCP, TCP_NODELAY,
					   (char *) &on,
					   sizeof(on)) < 0)
		{
			pool_error("do_accept: setsockopt() failed: %s", strerror(errno));
			close(afd);
			return NULL;
		}
		if (setsockopt(afd, SOL_SOCKET, SO_KEEPALIVE,
					   (char *) &on,
					   sizeof(on)) < 0)
		{
			pool_error("do_accept: setsockopt() failed: %s", strerror(errno));
			close(afd);
			return NULL;
		}
	}

	if ((cp = pool_open(afd)) == NULL)
	{
		close(afd);
		return NULL;
	}

	/* save ip address for hba */
	memcpy(&cp->raddr, &saddr, sizeof(SockAddr));
	if (cp->raddr.addr.ss_family == 0)
		cp->raddr.addr.ss_family = AF_UNIX;

	return cp;
}

/*
* Read startup packet
*
* Read the startup packet and parse the contents.
*/
static StartupPacket *read_startup_packet(POOL_CONNECTION *cp)
{
	StartupPacket *sp;
	StartupPacket_v2 *sp2;
	int protov;
	int len;
	char *p;

	sp = (StartupPacket *)calloc(sizeof(*sp), 1);
	if (!sp)
	{
		pool_error("read_startup_packet: out of memory");
		return NULL;
	}

	if (pool_config->authentication_timeout > 0)
	{
		pool_signal(SIGALRM, authentication_timeout);
		alarm(pool_config->authentication_timeout);
	}

	/* read startup packet length */
	if (pool_read(cp, &len, sizeof(len)))
	{
		pool_error("read_startup_packet: incorrect packet length (%d)", len);
		pool_free_startup_packet(sp);
		alarm(0);
		pool_signal(SIGALRM, SIG_IGN);
		return NULL;
	}
	len = ntohl(len);
	len -= sizeof(len);

	if (len <= 0 || len >= MAX_STARTUP_PACKET_LENGTH)
	{
		pool_error("read_startup_packet: incorrect packet length (%d)", len);
		pool_free_startup_packet(sp);
		alarm(0);
		pool_signal(SIGALRM, SIG_IGN);
		return NULL;
	}

	sp->startup_packet = calloc(len, 1);
	if (!sp->startup_packet)
	{
		pool_error("read_startup_packet: out of memory");
		pool_free_startup_packet(sp);
		alarm(0);
		pool_signal(SIGALRM, SIG_IGN);
		return NULL;
	}

	/* read startup packet */
	if (pool_read(cp, sp->startup_packet, len))
	{
		pool_free_startup_packet(sp);
		alarm(0);
		pool_signal(SIGALRM, SIG_IGN);
		return NULL;
	}

	sp->len = len;
	memcpy(&protov, sp->startup_packet, sizeof(protov));
	sp->major = ntohl(protov)>>16;
	sp->minor = ntohl(protov) & 0x0000ffff;
	p = sp->startup_packet;

	switch(sp->major)
	{
		case PROTO_MAJOR_V2: /* V2 */
			sp2 = (StartupPacket_v2 *)(sp->startup_packet);

			sp->database = calloc(SM_DATABASE+1, 1);
			if (!sp->database)
			{
				pool_error("read_startup_packet: out of memory");
				pool_free_startup_packet(sp);
				alarm(0);
				pool_signal(SIGALRM, SIG_IGN);
				return NULL;
			}
			strncpy(sp->database, sp2->database, SM_DATABASE);

			sp->user = calloc(SM_USER+1, 1);
			if (!sp->user)
			{
				pool_error("read_startup_packet: out of memory");
				pool_free_startup_packet(sp);
				alarm(0);
				pool_signal(SIGALRM, SIG_IGN);
				return NULL;
			}
			strncpy(sp->user, sp2->user, SM_USER);

			break;

		case PROTO_MAJOR_V3: /* V3 */
			p += sizeof(int);	/* skip protocol version info */

			while(*p)
			{
				if (!strcmp("user", p))
				{
					p += (strlen(p) + 1);
					sp->user = strdup(p);
					if (!sp->user)
					{
						pool_error("read_startup_packet: out of memory");
						pool_free_startup_packet(sp);
						alarm(0);
						pool_signal(SIGALRM, SIG_IGN);
						return NULL;
					}
				}
				else if (!strcmp("database", p))
				{
					p += (strlen(p) + 1);
					sp->database = strdup(p);
					if (!sp->database)
					{
						pool_error("read_startup_packet: out of memory");
						pool_free_startup_packet(sp);
						alarm(0);
						pool_signal(SIGALRM, SIG_IGN);
						return NULL;
					}
				}

				/*
				 * From 9.0, the start up packet may include
				 * application name. After receiving such that packet,
				 * backend sends parameter status of application_name.
				 * Upon reusing connection to backend, we need to
				 * emulate this behavior of backend. So we remember
				 * this and send parameter status packet to frontend
				 * instead of backend in
				 * connect_using_existing_connection().
				 */
				else if (!strcmp("application_name", p))
				{
					p += (strlen(p) + 1);
					sp->application_name = p;
					pool_debug("read_startup_packet: application_name: %s", p);
				}

				p += (strlen(p) + 1);
			}
			break;

		case 1234:		/* cancel or SSL request */
			/* set dummy database, user info */
			sp->database = calloc(1, 1);
			if (!sp->database)
			{
				pool_error("read_startup_packet: out of memory");
				pool_free_startup_packet(sp);
				alarm(0);
				pool_signal(SIGALRM, SIG_IGN);
				return NULL;
			}
			sp->user = calloc(1, 1);
			if (!sp->user)
			{
				pool_error("read_startup_packet: out of memory");
				pool_free_startup_packet(sp);
				alarm(0);
				pool_signal(SIGALRM, SIG_IGN);
				return NULL;
			}
			break;

		default:
			pool_error("read_startup_packet: invalid major no: %d", sp->major);
			pool_free_startup_packet(sp);
			alarm(0);
			pool_signal(SIGALRM, SIG_IGN);
			return NULL;
	}

	/* Check a user name was given. */
	if (sp->major != 1234 &&
	    (sp->user == NULL || sp->user[0] == '\0'))
	{
		pool_send_fatal_message(cp, sp->major, "28000",
		                        "no PostgreSQL user name specified in startup packet",
								"",
								"",
								__FILE__, __LINE__);
		pool_error("read_startup_packet: no PostgreSQL user name specified in startup packet");
		pool_free_startup_packet(sp);
		alarm(0);
		pool_signal(SIGALRM, SIG_IGN);
		return NULL;
	}

	/* The database defaults to ther user name. */
	if (sp->database == NULL || sp->database[0] == '\0')
	{
		sp->database = strdup(sp->user);
	}

	pool_debug("Protocol Major: %d Minor: %d database: %s user: %s",
			   sp->major, sp->minor, sp->database, sp->user);
	alarm(0);
	pool_signal(SIGALRM, SIG_IGN);
	return sp;
}

/*
 * send startup packet
 */
int send_startup_packet(POOL_CONNECTION_POOL_SLOT *cp)
{
	int len;

	len = htonl(cp->sp->len + sizeof(len));
	pool_write(cp->con, &len, sizeof(len));
	return pool_write_and_flush(cp->con, cp->sp->startup_packet, cp->sp->len);
}

/*
 * Reuse existing connection
 */
static bool connect_using_existing_connection(POOL_CONNECTION *frontend,
											  POOL_CONNECTION_POOL *backend,
											  StartupPacket *sp)
{
	int i, freed = 0;
	/*
	 * Save startup packet info
	 */
	for (i = 0; i < NUM_BACKENDS; i++)
	{
		if (VALID_BACKEND(i))
		{
			if (!freed)
			{
				pool_free_startup_packet(backend->slots[i]->sp);
				freed = 1;
			}
			backend->slots[i]->sp = sp;
		}
	}

	/* Reuse existing connection to backend */

	if (pool_do_reauth(frontend, backend))
	{
		pool_close(frontend);
		connection_count_down();
		return false;
	}

	if (MAJOR(backend) == 3)
	{
		char command_buf[1024];

		/* If we have received application_name in the start up
		 * packet, we send SET command to backend. Also we add or
		 * replace existing application_name data.
		 */
		if (sp->application_name)
		{
			snprintf(command_buf, sizeof(command_buf), "SET application_name TO '%s'", sp->application_name);

			for (i=0;i<NUM_BACKENDS;i++)
			{
				if (VALID_BACKEND(i))
					if (do_command(frontend, CONNECTION(backend, i),
							   command_buf, MAJOR(backend),
								   MASTER_CONNECTION(backend)->pid,
								   MASTER_CONNECTION(backend)->key, 0) != POOL_CONTINUE)
					{
						pool_error("connect_using_existing_connection: do_command failed. command: %s", command_buf);
						return false;
					}
			}

			pool_add_param(&MASTER(backend)->params, "application_name", sp->application_name);
		}

		if (send_params(frontend, backend))
		{
			pool_close(frontend);
			connection_count_down();
			return false;
		}
	}

	/* Send ReadyForQuery to frontend */
	pool_write(frontend, "Z", 1);

	if (MAJOR(backend) == 3)
	{
		int len;
		char tstate;

		len = htonl(5);
		pool_write(frontend, &len, sizeof(len));
		tstate = TSTATE(backend, MASTER_NODE_ID);
		pool_write(frontend, &tstate, 1);
	}

	if (pool_flush(frontend) < 0)
	{
		pool_close(frontend);
		connection_count_down();
		return false;
	}
	return true;
}

/*
 * process cancel request
 */
void cancel_request(CancelPacket *sp)
{
	int	len;
	int fd;
	POOL_CONNECTION *con;
	int i,j,k;
	ConnectionInfo *c = NULL;
	CancelPacket cp;
	bool found = false;

	pool_debug("Cancel request received");

	/* look for cancel key from shmem info */
	for (i=0;i<pool_config->num_init_children;i++)
	{
		for (j=0;j<pool_config->max_pool;j++)
		{
			for (k=0;k<NUM_BACKENDS;k++)
			{
				c = pool_coninfo(i, j, k);
				pool_debug("con_info: address:%p database:%s user:%s pid:%d key:%d i:%d",
						   c, c->database, c->user, ntohl(c->pid), ntohl(c->key),i);

				if (c->pid == sp->pid && c->key == sp->key)
				{
					pool_debug("found pid:%d key:%d i:%d",ntohl(c->pid), ntohl(c->key),i);
					c = pool_coninfo(i, j, 0);
					found = true;
					goto found;
				}
			}
		}
	}

 found:
	if (!found)
	{
		pool_error("cancel_request: invalid cancel key: pid:%d key:%d",ntohl(sp->pid), ntohl(sp->key));
		return;	/* invalid key */
	}

	for (i=0;i<NUM_BACKENDS;i++,c++)
	{
		if (!VALID_BACKEND(i))
			continue;

		if (*(BACKEND_INFO(i).backend_hostname) == '/')
			fd = connect_unix_domain_socket(i, TRUE);
		else
			fd = connect_inet_domain_socket(i, TRUE);

		if (fd < 0)
		{
			pool_error("Could not create socket for sending cancel request for backend %d", i);
			return;
		}

		con = pool_open(fd);
		if (con == NULL)
			return;

		len = htonl(sizeof(len) + sizeof(CancelPacket));
		pool_write(con, &len, sizeof(len));

		cp.protoVersion = sp->protoVersion;
		cp.pid = c->pid;
		cp.key = c->key;

		pool_log("cancel_request: canceling backend pid:%d key: %d", ntohl(cp.pid),ntohl(cp.key));

		if (pool_write_and_flush(con, &cp, sizeof(CancelPacket)) < 0)
			pool_error("Could not send cancel request packet for backend %d", i);

		pool_close(con);

		/*
		 * this is needed to ensure that the next DB node executes the
		 * query supposed to be canceled.
		 */
		sleep(1);
	}
}

static POOL_CONNECTION_POOL *connect_backend(StartupPacket *sp, POOL_CONNECTION *frontend)
{
	POOL_CONNECTION_POOL *backend;
	int i;

	/* connect to the backend */
	backend = pool_create_cp();
	if (backend == NULL)
	{
		pool_send_error_message(frontend, sp->major, "XX000", "connection cache is full", "",
								"increase max_pool", __FILE__, __LINE__);
		pool_close(frontend);
		pool_free_startup_packet(sp);
		return NULL;
	}

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			/* set DB node id */
			CONNECTION(backend, i)->db_node_id = i;

			/* mark this is a backend connection */
			CONNECTION(backend, i)->isbackend = 1;
			pool_ssl_negotiate_clientserver(CONNECTION(backend, i));

			/*
			 * save startup packet info
			 */
			CONNECTION_SLOT(backend, i)->sp = sp;

			/* send startup packet */
			if (send_startup_packet(CONNECTION_SLOT(backend, i)) < 0)
			{
				pool_error("do_child: fails to send startup packet to the %d th backend", i);
				pool_discard_cp(sp->user, sp->database, sp->major);
				pool_close(frontend);
				return NULL;
			}
		}
	}

	/*
	 * do authentication stuff
	 */
	if (pool_do_auth(frontend, backend))
	{
		pool_close(frontend);
		pool_discard_cp(sp->user, sp->database, sp->major);
		return NULL;
	}

	return backend;
}

/*
 * signal handler for SIGTERM, SIGINT and SIGQUUT
 */
static RETSIGTYPE die(int sig)
{
	pool_debug("child received shutdown request signal %d", sig);

	exit_request = sig;

	switch (sig)
	{
		case SIGTERM:	/* smart shutdown */
			if (idle == 0)
			{
				pool_debug("child receives smart shutdown request but it's not in idle state");
			}
			break;

		case SIGINT:	/* fast shutdown */
		case SIGQUIT:	/* immediate shutdown */
			child_exit(0);
			break;
		default:
			pool_error("die() received unknown signal: %d", sig);
			break;
	}
}

/*
 * signal handler for SIGUSR1
 * close all idle connections
 */
static RETSIGTYPE close_idle_connection(int sig)
{
	int i, j;
	POOL_CONNECTION_POOL *p = pool_connection_pool;
	ConnectionInfo *info;

	pool_debug("child receives close connection request");

	for (j=0;j<pool_config->max_pool;j++, p++)
	{
		if (!MASTER_CONNECTION(p))
			continue;
 		if (!MASTER_CONNECTION(p)->sp)
			continue;
		if (MASTER_CONNECTION(p)->sp->user == NULL)
			continue;

		if (MASTER_CONNECTION(p)->closetime > 0)		/* idle connection? */
		{
			pool_debug("close_idle_connection: close idle connection: user %s database %s", MASTER_CONNECTION(p)->sp->user, MASTER_CONNECTION(p)->sp->database);
			pool_send_frontend_exits(p);

			for (i=0;i<NUM_BACKENDS;i++)
			{
				if (!VALID_BACKEND(i))
					continue;

				if (i == 0)
				{
					/* only first backend allocated the memory for the start up packet */
					pool_free_startup_packet(CONNECTION_SLOT(p, i)->sp);
				}
				pool_close(CONNECTION(p, i));
			}
			info = p->info;
			memset(p, 0, sizeof(POOL_CONNECTION_POOL));
			p->info = info;
			memset(p->info, 0, sizeof(ConnectionInfo));
		}
	}
}

/*
 * signal handler for SIGALRM
 *
 */
static RETSIGTYPE authentication_timeout(int sig)
{
	pool_log("authentication is timeout");
	child_exit(1);
}

/*
 * send frontend exiting messages to all connections.  this is called
 * in any case when child process exits, for example failover, child
 * life time expires or child max connections expires.
 */
static void send_frontend_exits(void)
{
	int i;
	POOL_CONNECTION_POOL *p = pool_connection_pool;

#ifdef HAVE_SIGPROCMASK
	sigset_t oldmask;
#else
	int	oldmask;
#endif

	POOL_SETMASK2(&BlockSig, &oldmask);

	for (i=0;i<pool_config->max_pool;i++, p++)
	{
		if (!MASTER_CONNECTION(p))
			continue;
		if (!MASTER_CONNECTION(p)->sp)
			continue;
		if (MASTER_CONNECTION(p)->sp->user == NULL)
			continue;
		pool_send_frontend_exits(p);
	}

	POOL_SETMASK(&oldmask);
}

static int send_params(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	int index;
	char *name, *value;
	int len, sendlen;

	index = 0;
	while (pool_get_param(&MASTER(backend)->params, index++, &name, &value) == 0)
	{
		pool_write(frontend, "S", 1);
		len = sizeof(sendlen) + strlen(name) + 1 + strlen(value) + 1;
		sendlen = htonl(len);
		pool_write(frontend, &sendlen, sizeof(sendlen));
		pool_write(frontend, name, strlen(name) + 1);
		pool_write(frontend, value, strlen(value) + 1);
	}

	if (pool_flush(frontend))
	{
		pool_error("pool_send_params: pool_flush() failed");
		return -1;
	}
	return 0;
}

void pool_free_startup_packet(StartupPacket *sp)
{
	if (sp)
	{
		if (sp->startup_packet)
			free(sp->startup_packet);
		if (sp->database)
			free(sp->database);
		if (sp->user)
			free(sp->user);
		free(sp);
	}
	sp = NULL;
}

/*
 * Do house keeping works when pgpool child process exits
 */
void child_exit(int code)
{
	if (getpid() == mypid)
	{
		pool_log("child_exit: called from pgpool main. ignored.");
		return;
	}

	/* count down global connection counter */
	if (accepted)
		connection_count_down();

	/* prepare to shutdown connections to system db */
	if(pool_config->parallel_mode)
	{
		if (system_db_info->pgconn)
			pool_close_libpq_connection();
		if (pool_system_db_connection())
			pool_close(pool_system_db_connection()->con);
	}

	if (pool_config->memory_cache_enabled && !pool_is_shmem_cache())
	{
		memcached_disconnect();
	}

	/* let backend know now we are exiting */
	if (pool_connection_pool)
		send_frontend_exits();

	exit(code);
}

/*
 * create a persistent connection
 */
POOL_CONNECTION_POOL_SLOT *make_persistent_db_connection(
	char *hostname, int port, char *dbname, char *user, char *password, bool retry)
{
	POOL_CONNECTION_POOL_SLOT *cp;
	int fd;

#define MAX_USER_AND_DATABASE	1024

	/* V3 startup packet */
	typedef struct {
		int protoVersion;
		char data[MAX_USER_AND_DATABASE];
	} StartupPacket_v3;

	static StartupPacket_v3 *startup_packet;
	int len, len1;
	int status;

	cp = malloc(sizeof(POOL_CONNECTION_POOL_SLOT));
	if (cp == NULL)
	{
		pool_error("make_persistent_db_connection: could not allocate memory");
		return NULL;
	}
	memset(cp, 0, sizeof(POOL_CONNECTION_POOL_SLOT));

	startup_packet = malloc(sizeof(*startup_packet));
	if (startup_packet == NULL)
	{
		pool_error("make_persistent_db_connection: could not allocate memory");
		return NULL;
	}
	memset(startup_packet, 0, sizeof(*startup_packet));
	startup_packet->protoVersion = htonl(0x00030000);	/* set V3 proto major/minor */

	/*
	 * create socket
	 */
	if (*hostname == '/')
	{
		fd = connect_unix_domain_socket_by_port(port, hostname, retry);
	}
	else
	{
		fd = connect_inet_domain_socket_by_port(hostname, port, retry);
	}

	if (fd < 0)
	{
		pool_error("make_persistent_db_connection: connection to %s(%d) failed", hostname, port);
		return NULL;
	}

	cp->con = pool_open(fd);
	cp->closetime = 0;
	cp->con->isbackend = 1;
	pool_ssl_negotiate_clientserver(cp->con);

	/*
	 * build V3 startup packet
	 */
	len = snprintf(startup_packet->data, sizeof(startup_packet->data), "user") + 1;
	len1 = snprintf(&startup_packet->data[len], sizeof(startup_packet->data)-len, "%s", user) + 1;
	if (len1 >= (sizeof(startup_packet->data)-len))
	{
		pool_error("make_persistent_db_connection: too long user name");
		return NULL;
	}

	len += len1;
	len1 = snprintf(&startup_packet->data[len], sizeof(startup_packet->data)-len, "database") + 1;
	if (len1 >= (sizeof(startup_packet->data)-len))
	{
		pool_error("make_persistent_db_connection: too long user name");
		return NULL;
	}

	len += len1;
	len1 = snprintf(&startup_packet->data[len], sizeof(startup_packet->data)-len, "%s", dbname) + 1;
	if (len1 >= (sizeof(startup_packet->data)-len))
	{
		pool_error("make_persistent_db_connection: too long database name");
		return NULL;
	}
	len += len1;
	startup_packet->data[len++] = '\0';

	cp->sp = malloc(sizeof(StartupPacket));
	if (cp->sp == NULL)
	{
		pool_error("make_persistent_db_connection: could not allocate memory");
		return NULL;
	}

	cp->sp->startup_packet = (char *)startup_packet;
	cp->sp->len = len + 4;
	cp->sp->major = 3;
	cp->sp->minor = 0;
	cp->sp->database = strdup(dbname);
	if (cp->sp->database == NULL)
	{
		pool_error("make_persistent_db_connection: could not allocate memory");
		return NULL;
	}
	cp->sp->user = strdup(user);
	if (cp->sp->user == NULL)
	{
		pool_error("make_persistent_db_connection: could not allocate memory");
		return NULL;
	}

	/*
	 * send startup packet
	 */
	status = send_startup_packet(cp);
	if (status)
	{
		pool_error("make_persistent_db_connection: send_startup_packet failed");
		return NULL;
	}

	/*
	 * do authentication
	 */
	if (s_do_auth(cp, password))
	{
		pool_error("make_persistent_db_connection: s_do_auth failed");
		return NULL;
	}

	return cp;
}

/*
 * Discard connection and memory allocated by
 * make_persistent_db_connection().
 */
void discard_persistent_db_connection(POOL_CONNECTION_POOL_SLOT *cp)
{
	int len;

	if(cp == NULL)
		return;

	pool_write(cp->con, "X", 1);
	len = htonl(4);
	pool_write(cp->con, &len, sizeof(len));

	/*
	 * XXX we cannot call pool_flush() here since backend may already
	 * close the socket and pool_flush() automatically invokes fail
	 * over handler. This could happen in copy command (remember the
	 * famous "lost synchronization with server, resetting
	 * connection" message)
	 */
	pool_set_nonblock(cp->con->fd);
	pool_flush_it(cp->con);
	pool_unset_nonblock(cp->con->fd);

	pool_close(cp->con);
	free(cp->sp->startup_packet);
	free(cp->sp->database);
	free(cp->sp->user);
	free(cp->sp);
	free(cp);
}

/*
 * Do authentication. Assuming the only caller is
 * *make_persistent_db_connection().
 */
static int s_do_auth(POOL_CONNECTION_POOL_SLOT *cp, char *password)
{
	char kind;
	int status;
	int length;
	int auth_kind;
	char state;
	char *p;
	int pid, key;
	bool keydata_done;

	/*
	 * read kind expecting 'R' packet (authentication response)
	 */
	status = pool_read(cp->con, &kind, sizeof(kind));
	if (status < 0)
	{
		pool_error("s_do_auth: error while reading message kind");
		return -1;
	}

	if (kind != 'R')
	{
		pool_error("s_do_auth: expecting R got %c", kind);
		return -1;
	}

	/* read message length */
	status = pool_read(cp->con, &length, sizeof(length));
	if (status < 0)
	{
		pool_error("s_do_auth: error while reading message length");
		return -1;
	}
	length = ntohl(length);

	/* read auth kind */
	status = pool_read(cp->con, &auth_kind, sizeof(auth_kind));
	if (status < 0)
	{
		pool_error("s_do_auth: error while reading auth kind");
		return -1;
	}
	auth_kind = ntohl(auth_kind);
	pool_debug("s_do_auth: auth kind: %d", auth_kind);

	if (auth_kind == 0)	/* trust authentication? */
	{
		cp->con->auth_kind = 0;
	}
	else if (auth_kind == 3) /* clear text password? */
	{
		int size = htonl(strlen(password) + 5);

		pool_write(cp->con, "p", 1);
		pool_write(cp->con, &size, sizeof(size));
		pool_write_and_flush(cp->con, password, strlen(password) + 1);
		status = pool_flush(cp->con);
		if (status > 0)
		{
			pool_error("s_do_auth: error while sending clear text password");
			return -1;
		}
		return s_do_auth(cp, password);
	}
	else if (auth_kind == 4) /* crypt password? */
	{
		int size;
		char salt[3];
		char *crypt_password;

		status = pool_read(cp->con, &salt, 2);
		if (status > 0)
		{
			pool_error("s_do_auth: error while reading crypt salt");
			return -1;
		}
		salt[2] = '\0';

		crypt_password = crypt(password, salt);
		size = htonl(strlen(crypt_password) + 5);
		pool_write(cp->con, "p", 1);
		pool_write(cp->con, &size, sizeof(size));
		pool_write_and_flush(cp->con, crypt_password, strlen(crypt_password) + 1);
		status = pool_flush(cp->con);
		if (status > 0)
		{
			pool_error("s_do_auth: error while sending crypt password");
			return -1;
		}
		return s_do_auth(cp, password);
	}
	else if (auth_kind == 5) /* md5 password? */
	{
		char salt[4];
		char *buf, *buf1;
		int size;

		status = pool_read(cp->con, &salt, 4);
		if (status > 0)
		{
			pool_error("s_do_auth: error while reading md5 salt");
			return -1;
		}

		buf = malloc(2 * (MD5_PASSWD_LEN + 4)); /* hash + "md5" + '\0' */
		if (buf == NULL)
		{
			pool_error("s_do_auth(): malloc failed: %s", strerror(errno));
			return -1;
		}
		memset(buf, 0, 2 * (MD5_PASSWD_LEN + 4));

		/* build md5 password */
		buf1 = buf + MD5_PASSWD_LEN + 4;
		pool_md5_encrypt(password, cp->sp->user, strlen(cp->sp->user), buf1);
		pool_md5_encrypt(buf1, salt, 4, buf + 3);
		memcpy(buf, "md5", 3);

		size = htonl(strlen(buf) + 5);
		pool_write(cp->con, "p", 1);
		pool_write(cp->con, &size, sizeof(size));
		pool_write_and_flush(cp->con, buf, strlen(buf) + 1);
		status = pool_flush(cp->con);
		if (status > 0)
		{
			pool_error("s_do_auth: error while sending md5 password");
			return -1;
		}

		status = s_do_auth(cp, password);
		free(buf);
		return status;
	}
	else
	{
		pool_error("s_do_auth: auth kind %d not supported yet", auth_kind);
		return -1;
	}

	/*
	 * Read backend key data and wait until Ready for query arriving or
	 * error happens.
	 */

	keydata_done = false;

	for (;;)
	{
		status = pool_read(cp->con, &kind, sizeof(kind));
		if (status < 0)
		{
			pool_error("s_do_auth: error while reading message kind");
			return -1;
		}

		switch (kind)
		{
			case 'K':	/* backend key data */
				keydata_done = true;
				pool_debug("s_do_auth: backend key data received");

				/* read message length */
				status = pool_read(cp->con, &length, sizeof(length));
				if (status < 0)
				{
					pool_error("s_do_auth: error while reading message length");
					return -1;
				}
				if (ntohl(length) != 12)
				{
					pool_error("s_do_auth: backend key data length is not 12 (%d)", ntohl(length));
				}

				/* read pid */
				if (pool_read(cp->con, &pid, sizeof(pid)) < 0)
				{
					pool_error("s_do_auth: failed to read pid");
					return -1;
				}
				cp->pid = pid;

				/* read key */
				if (pool_read(cp->con, &key, sizeof(key)) < 0)
				{
					pool_error("s_do_auth: failed to read key");
					return -1;
				}
				cp->key = key;
				break;

			case 'Z':	/* Ready for query */
				/* read message length */
				status = pool_read(cp->con, &length, sizeof(length));
				if (status < 0)
				{
					pool_error("s_do_auth: error while reading message length");
					return -1;
				}
				length = ntohl(length);

				/* read transaction state */
				status = pool_read(cp->con, &state, sizeof(state));
				if (status < 0)
				{
					pool_error("s_do_auth: error while reading transaction state");
					return -1;
				}

				pool_debug("s_do_auth: transaction state: %c", state);
				cp->con->tstate = state;

				if (!keydata_done)
				{
					pool_error("s_do_auth: ready for query arrived before receiving keydata");
				}
				return 0;
				break;

			case 'S':	/* parameter status */
			case 'N':	/* notice response */
			case 'E':	/* error response */
				/* Just throw away data */
				status = pool_read(cp->con, &length, sizeof(length));
				if (status < 0)
				{
					pool_error("s_do_auth: error while reading message length. kind:%c", kind);
					return -1;
				}

				length = ntohl(length);
				length -= 4;

				p = pool_read2(cp->con, length);
				if (p == NULL)
					return -1;
				break;

			default:
				pool_error("s_do_auth: unknown response \"%c\" while processing BackendKeyData",
						   kind);
				break;
		}
	}
	return -1;
}

/*
 * Count up connection counter (from frontend to pgpool)
 * in shared memory
 */
static void connection_count_up(void)
{
#ifdef HAVE_SIGPROCMASK
	sigset_t oldmask;
#else
	int	oldmask;
#endif

	POOL_SETMASK2(&BlockSig, &oldmask);
	pool_semaphore_lock(CONN_COUNTER_SEM);
	Req_info->conn_counter++;
	pool_semaphore_unlock(CONN_COUNTER_SEM);
	POOL_SETMASK(&oldmask);
}

/*
 * Count down connection counter (from frontend to pgpool)
 * in shared memory
 */
static void connection_count_down(void)
{
#ifdef HAVE_SIGPROCMASK
	sigset_t oldmask;
#else
	int	oldmask;
#endif

	POOL_SETMASK2(&BlockSig, &oldmask);
	pool_semaphore_lock(CONN_COUNTER_SEM);
	/*
	 * Make sure that we do not decrement too much.  If failed to read
	 * a start up packet, or receive cancel request etc.,
	 * connection_count_down() is called and goes back to the
	 * connection accept loop. Problem is, at the very beginning of
	 * the connection accept loop, if we have received a signal, we
	 * call child_exit() which calls connection_count_down() again.
	 */
	if (Req_info->conn_counter > 0)
		Req_info->conn_counter--;
	pool_semaphore_unlock(CONN_COUNTER_SEM);
	POOL_SETMASK(&oldmask);
}

/*
 * handle SIGUSR2
 * Wakeup all process
 */
static RETSIGTYPE wakeup_handler(int sig)
{
}


/*
 * Select load balancing node
 */
int select_load_balancing_node(void)
{
	int selected_slot;
	double total_weight,r;
	int i;

	/* choose a backend in random manner with weight */
	selected_slot = MASTER_NODE_ID;
	total_weight = 0.0;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			total_weight += BACKEND_INFO(i).backend_weight;
		}
	}

#if defined(sun) || defined(__sun)
	r = (((double)rand())/RAND_MAX) * total_weight;
#else
	r = (((double)random())/RAND_MAX) * total_weight;
#endif

	total_weight = 0.0;
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i) && BACKEND_INFO(i).backend_weight > 0.0)
		{
			if(r >= total_weight)
				selected_slot = i;
			else
				break;
			total_weight += BACKEND_INFO(i).backend_weight;
		}
	}

	pool_debug("select_load_balancing_node: selected backend id is %d", selected_slot);
	return selected_slot;
}

/* SIGHUP handler */
static RETSIGTYPE reload_config_handler(int sig)
{
	got_sighup = 1;
}

/*
 * Exit myself if SIGTERM, SIGINT or SIGQUIT has been sent
 */
void check_stop_request(void)
{
    /*
	 * If smart shutdown was requested but we are not in idle state,
	 * do not exit
	 */
	if (exit_request == SIGTERM && idle == 0)
		return;

	if (exit_request)
	{
		reset_variables();
		child_exit(0);
	}
}

/*
 * Initialize system DB connection
 */
static void init_system_db_connection(void)
{	
	if (pool_config->parallel_mode)
	{
		system_db_connect();
		if (PQstatus(system_db_info->pgconn) != CONNECTION_OK)
		{
			pool_error("Could not make persistent libpq system DB connection");
		}

		system_db_info->connection = make_persistent_db_connection(pool_config->system_db_hostname,
																   pool_config->system_db_port,
																   pool_config->system_db_dbname,
																   pool_config->system_db_user,
																   pool_config->system_db_password, false);
		if (system_db_info->connection == NULL)
		{
			pool_error("Could not make persistent system DB connection");
		}
	}
}

/*
 * Initialize my backend status and master node id.
 * We copy the backend status to private area so that
 * they are not changed while I am alive.
 */
void pool_initialize_private_backend_status(void)
{
	int i;

	pool_debug("pool_initialize_private_backend_status: initialize backend status");

	for (i=0;i<MAX_NUM_BACKENDS;i++)
	{
		private_backend_status[i] = BACKEND_INFO(i).backend_status;
		/* my_backend_status is referred to by VALID_BACKEND macro. */
		my_backend_status[i] = &private_backend_status[i];
	}

	my_master_node_id = REAL_MASTER_NODE_ID;
}
