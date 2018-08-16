/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2017	PgPool Global Development Group
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
 * health_check.c: health check related functions.
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
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/elog.h"

#include "context/pool_process_context.h"
#include "context/pool_session_context.h"
#include "pool_config.h"
#include "utils/pool_ip.h"
#include "auth/md5.h"
#include "auth/pool_hba.h"
#include "utils/pool_stream.h"

char remote_ps_data[NI_MAXHOST];		/* used for set_ps_display */
static POOL_CONNECTION_POOL_SLOT	*slot;
static volatile sig_atomic_t reload_config_request = 0;
static volatile sig_atomic_t restart_request = 0;
static bool establish_persistent_connection(int node);
static void discard_persistent_connection(int node);
static RETSIGTYPE my_signal_handler(int sig);
static RETSIGTYPE reload_config_handler(int sig);
static void reload_config(void);
static RETSIGTYPE health_check_timer_handler(int sig);

#ifdef HEALTHCHECK_OPTS
  #if HEALTHCHECK_OPTS > 0
  #define HEALTHCHECK_DEBUG
  #endif
#endif

#ifdef HEALTHCHECK_DEBUG
static bool check_backend_down_request(int node);
#endif

#undef CHECK_REQUEST
#define CHECK_REQUEST \
	do { \
		if (reload_config_request) \
		{ \
			reload_config(); \
			reload_config_request = 0; \
		} else if (restart_request) \
		{ \
		  ereport(LOG,(errmsg("health check process received restart request"))); \
		  exit(1); \
		} \
    } while (0)

#undef CLEAR_ALARM
#define CLEAR_ALARM \
	do { \
		ereport(DEBUG1,(errmsg("health check: clearing alarm"))); \
    } while (alarm(0) > 0)

/*
* health check child main loop
*/
void do_health_check_child(int *node_id)
{
    sigjmp_buf	local_sigjmp_buf;
	MemoryContext HealthCheckMemoryContext;
	char psbuffer[NI_MAXHOST];

	ereport(DEBUG1,
			(errmsg("I am health check process pid:%d DB node id:%d", getpid(), *node_id)));

	/* Identify myself via ps */
	init_ps_display("", "", "", "");
	snprintf(psbuffer, sizeof(psbuffer), "health check process(%d)", *node_id);
	set_ps_display(psbuffer, false);

	/* set up signal handlers */
	signal(SIGALRM, SIG_DFL);
	signal(SIGTERM, my_signal_handler);
	signal(SIGINT, my_signal_handler);
	signal(SIGHUP, reload_config_handler);
	signal(SIGQUIT, my_signal_handler);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGUSR1, my_signal_handler);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

    /* Create per loop iteration memory context */
	HealthCheckMemoryContext = AllocSetContextCreate(TopMemoryContext,
                                             "health_check_main_loop",
													 ALLOCSET_DEFAULT_MINSIZE,
													 ALLOCSET_DEFAULT_INITSIZE,
													 ALLOCSET_DEFAULT_MAXSIZE);

	MemoryContextSwitchTo(TopMemoryContext);

	/* Initialize my backend status */
	pool_initialize_private_backend_status();

	/* Initialize per process context */
	pool_init_process_context();

    if (sigsetjmp(local_sigjmp_buf, 1) != 0)
	{
		pool_signal(SIGALRM, SIG_IGN);
		error_context_stack = NULL;
		EmitErrorReport();
		MemoryContextSwitchTo(TopMemoryContext);
		FlushErrorState();
	}
	/* We can now handle ereport(ERROR) */
	PG_exception_stack = &local_sigjmp_buf;


	for (;;)
	{
        MemoryContextSwitchTo(HealthCheckMemoryContext);
		MemoryContextResetAndDeleteChildren(HealthCheckMemoryContext);

		CHECK_REQUEST;

		if (pool_config->health_check_params[*node_id].health_check_period <= 0)
		{
			sleep(30);
		}

		/*
		 * If health checking is enabled and the node is not in down status,
		 * do health check.
		 */
		else if (pool_config->health_check_params[*node_id].health_check_period > 0)
		{
			bool result;

			result = establish_persistent_connection(*node_id);

#ifdef HEALTHCHECK_DEBUG
			if (check_backend_down_request(*node_id) || (result && slot == NULL))
#else
			if (result && slot == NULL)
#endif
			{
				if (POOL_DISALLOW_TO_FAILOVER(BACKEND_INFO(*node_id).flag))
				{
					ereport(LOG,
							(errmsg("health check failed on node %d but failover is disallowed for the node",
									*node_id)));
				}
				else
				{
					bool partial;

					ereport(LOG, (errmsg("health check failed on node %d (timeout:%d)",
										 *node_id, health_check_timer_expired)));

					/* trigger failover */
					partial = health_check_timer_expired?false:true;
					degenerate_backend_set(node_id, 1, partial?REQ_DETAIL_SWITCHOVER:0);
				}
			}

			/* Discard persistent connections */
			discard_persistent_connection(*node_id);
			sleep(pool_config->health_check_params[*node_id].health_check_period);
		}
	}
	exit(0);
}

/*
 * Establish persistent connection to backend.
 * Return true if connection test is done.
 */
static bool establish_persistent_connection(int node)
{
	BackendInfo *bkinfo;
	int retry_cnt;

	bkinfo = pool_get_node_info(node);

	/*
	 * If the node is already in down status or unused,
	 * do nothing.
	 */
	if (bkinfo->backend_status == CON_UNUSED ||
		bkinfo->backend_status == CON_DOWN)
		return false;

	/*
	 * If database is not specified, "postgres" database is assumed.
	 */
	if (*pool_config->health_check_params[node].health_check_database == '\0')
		pool_config->health_check_params[node].health_check_database = "postgres";

	/*
	 * Try to connect to the database.
	 */
	if (slot == NULL)
	{
		retry_cnt = pool_config->health_check_params[node].health_check_max_retries;

		char *password = get_pgpool_config_user_password(pool_config->health_check_params[node].health_check_user,
														 pool_config->health_check_params[node].health_check_password);

		do
		{
			/*
			 * Set health checker timeout. we want to detect
			 * communication path failure much earlier before
			 * TCP/IP stack detects it.
			 */
			if (pool_config->health_check_params[node].health_check_timeout > 0)
			{
				CLEAR_ALARM;
				pool_signal(SIGALRM, health_check_timer_handler);
				alarm(pool_config->health_check_params[node].health_check_timeout);
				errno = 0;
				health_check_timer_expired = 0;
			}

			slot = make_persistent_db_connection_noerror(node, bkinfo->backend_hostname,
														 bkinfo->backend_port,
														 pool_config->health_check_params[node].health_check_database,
														 pool_config->health_check_params[node].health_check_user,
														 password?password:"", false);

			if (pool_config->health_check_params[node].health_check_timeout > 0)
			{
				/* cancel health check timer */
				pool_signal(SIGALRM, SIG_IGN);
				CLEAR_ALARM;
			}

			if (slot)
			{
				if (retry_cnt != pool_config->health_check_params[node].health_check_max_retries)
				{
					ereport(LOG,
							(errmsg("health check retrying on DB node: %d succeeded",
									node)));
				}
				break;	/* Success */
			}

			retry_cnt--;

			if (retry_cnt >= 0)
			{
				ereport(LOG,
						(errmsg("health check retrying on DB node: %d (round:%d)",
								node,
								pool_config->health_check_params[node].health_check_max_retries - retry_cnt)));

				sleep(pool_config->health_check_params[node].health_check_retry_delay);
			}
		} while (retry_cnt >= 0);

		if (password)
			pfree(password);
	}
	return true;
}

/*
 * Discard persistent connection to backend
 */
static void discard_persistent_connection(int node)
{
	if (slot)
	{
		discard_persistent_db_connection(slot);
		slot = NULL;
	}
}

static RETSIGTYPE my_signal_handler(int sig)
{
	int save_errno = errno;

	POOL_SETMASK(&BlockSig);

	switch (sig)
	{
		case SIGTERM:
		case SIGINT:
		case SIGQUIT:
			exit(0);
			break;

			/* Failback or new node added */
		case SIGUSR1:
			restart_request = 1;
			break;

		default:
			exit(1);
			break;
	}

	POOL_SETMASK(&UnBlockSig);

	errno = save_errno;
}

static RETSIGTYPE reload_config_handler(int sig)
{
	int save_errno = errno;
	POOL_SETMASK(&BlockSig);
	reload_config_request = 1;
	POOL_SETMASK(&UnBlockSig);
	errno = save_errno;
}

static void reload_config(void)
{
	ereport(LOG,
			(errmsg("reloading config file")));
    MemoryContext oldContext = MemoryContextSwitchTo(TopMemoryContext);
	pool_get_config(get_config_file_name(), CFGCXT_RELOAD);
    MemoryContextSwitchTo(oldContext);
	if (pool_config->enable_pool_hba)
		load_hba(get_hba_file_name());
	reload_config_request = 0;
}
/*
 * health check timer handler
 */
static RETSIGTYPE health_check_timer_handler(int sig)
{
	int save_errno = errno;
	POOL_SETMASK(&BlockSig);
	health_check_timer_expired = 1;
	POOL_SETMASK(&UnBlockSig);
	errno = save_errno;
}

#ifdef HEALTHCHECK_DEBUG

/*
 * Node down request file. In the file, each line consists of "backend node
 * id", tab and "down".  If such a line found, check_backend_down_request()
 * will return true.
 */

# define BACKEND_DOWN_REQUEST_FILE	"backend_down_request"

/*
 * Check backend down request file with specified backend node id.  If it's
 * down ("down"), returns true and set the status to "already_down" to
 * prevent repeatable * failover. If it's other than "down", returns false.
*/

static bool check_backend_down_request(int node)
{
	static char	backend_down_request_file[POOLMAXPATHLEN];
	FILE *fd;
	int i;
#define MAXLINE 128
	char linebuf[MAXLINE];
	char readbuf[MAXLINE];
	char buf[MAXLINE];
	char *writebuf;
	char *p;
	bool found = false;
	int node_id;
	char status[MAXLINE];

	if (backend_down_request_file[0] == '\0')
	{
		snprintf(backend_down_request_file, sizeof(backend_down_request_file),
				 "%s/%s", pool_config->logdir, BACKEND_DOWN_REQUEST_FILE);
	}

	fd = fopen(backend_down_request_file, "r");
	if (!fd)
	{
		ereport(WARNING,
				(errmsg("check_backend_down_request: failed to open file %s",
						backend_down_request_file),
				 errdetail("\"%s\"",strerror(errno))));
		return false;
	}

	writebuf = NULL;

	for (i=0;;i++)
	{
		readbuf[MAXLINE-1] = '\0';
		if (fgets(readbuf, MAXLINE-1, fd) == 0)
			break;

		strncpy(buf, readbuf, sizeof(buf));
		if (strlen(readbuf) > 0 && readbuf[strlen(readbuf)-1] == '\n')
			buf[strlen(readbuf)-1] = '\0';

		p = readbuf;
		if (found == false)
		{
			sscanf(buf, "%d\t%s", &node_id, status);
			if (node_id == node && !strcmp(status, "down"))
			{
				snprintf(linebuf, sizeof(linebuf), "%d\t%s\n", node_id, "already_down");
				found = true;
				p = linebuf;
			}
		}

		if (writebuf == NULL)
		{
			writebuf = malloc(strlen(p)+1);
			memset(writebuf, 0, strlen(p)+1);
		}
		else
			writebuf = realloc(writebuf, strlen(p)+strlen(writebuf)+1);
		if (!writebuf)
		{
			fclose(fd);
			return false;
		}
		strcat(writebuf, p);
	}

	fclose(fd);

	if (!found)
		return false;

	fd = fopen(backend_down_request_file, "w");
	if (!fd)
	{
		ereport(WARNING,
                (errmsg("check_backend_down_request: failed to open file for writing %s",
						backend_down_request_file),
				 errdetail("\"%s\"",strerror(errno))));
		return false;
	}

	if (fwrite(writebuf, 1, strlen(writebuf), fd) != strlen(writebuf))
	{
		ereport(WARNING,
                (errmsg("check_backend_down_request: failed to write %s",
						backend_down_request_file),
				 errdetail("\"%s\"",strerror(errno))));
		fclose(fd);
		return false;
	}
	fclose(fd);

	return true;
}
#endif
