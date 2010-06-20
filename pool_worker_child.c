/* -*-pgsql-c-*- */
/*
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
 * child.c: worker child process main
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
#include "pool_process_context.h"
#include "pool_session_context.h"
#include "pool_config.h"
#include "pool_ip.h"
#include "md5.h"
#include "pool_stream.h"

extern int myargc;
extern char **myargv;

char remote_ps_data[NI_MAXHOST];		/* used for set_ps_display */
static volatile sig_atomic_t got_sighup = 0;
static POOL_CONNECTION_POOL_SLOT	*slots[MAX_NUM_BACKENDS];

static void establish_persistent_connection(void);
static void check_replication_time_lag(void);
static long text_to_lsn(char *text);
static RETSIGTYPE my_signal_handler(int sig);

/*
* worker child main loop
*/
void do_worker_child(void)
{
	pool_debug("I am %d", getpid());

	/* Identify myself via ps */
	init_ps_display("", "", "", "");
	set_ps_display("worker process", false);

	/* set up signal handlers */
	signal(SIGALRM, SIG_DFL);
	signal(SIGTERM, my_signal_handler);
	signal(SIGINT, my_signal_handler);
	signal(SIGHUP, my_signal_handler);
	signal(SIGQUIT, my_signal_handler);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

	/* Initialize per process context */
	pool_init_process_context();

	for (;;)
	{
		/*
		 * If streaming replication mode, do time lag checking
		 */
		if (MASTER_SLAVE && !strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP))
		{
			/* Check and establish persistent connections to the backend */
			establish_persistent_connection();

			/* Do replication time lag checking */
			check_replication_time_lag();
		}
		sleep(pool_config->health_check_period);		
	}
	exit(0);
}

/*
 * Establish persistent connection to backend
 */
static void establish_persistent_connection(void)
{
	int i;
	BackendInfo *bkinfo;
	POOL_CONNECTION_POOL_SLOT *s;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		if (slots[i] == NULL)
		{
			bkinfo = pool_get_node_info(i);
			s = make_persistent_db_connection(bkinfo->backend_hostname, 
											  bkinfo->backend_port,
											  "postgres",
											  pool_config->health_check_user,
											  "");
			if (s)
				slots[i] = s;
			else
				slots[i] = NULL;
		}
	}
}

/*
 * Check replicaton time lag
 */
static void check_replication_time_lag(void)
{
	int i;
	POOL_STATUS sts;
	POOL_SELECT_RESULT *res;
	long lsn[MAX_NUM_BACKENDS];
	char *query;

	if (NUM_BACKENDS <= 1)
	{
		/* If there's only one node, there's no point to do checking */
		return;
	}

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		if (!slots[i])
		{
			pool_error("check_replication_time_lag: DB node is valid but no persistent connection");
			return;
		}

		if (REAL_MASTER_NODE_ID == i)
		{
			query = "SELECT pg_current_xlog_location()";
		}
		else
		{
			query = "SELECT pg_last_xlog_receive_location()";
		}

		sts = do_query(slots[i]->con, query, &res, PROTO_MAJOR_V3);
		if (sts != POOL_CONTINUE)
		{
			pool_error("check_replication_time_lag: %s failed", query);
			return;
		}
		if (!res)
		{
			pool_error("check_replication_time_lag: %s result is null", query);
			return;
		}
		if (res->numrows <= 0)
		{
			pool_error("check_replication_time_lag: %s returns no rows", query);
			free_select_result(res);
			return;
		}
		if (res->data[0] == NULL)
		{
			pool_error("check_replication_time_lag: %s returns no data", query);
			free_select_result(res);
			return;
		}

		if (res->nullflags[0] == -1)
		{
			pool_log("check_replication_time_lag: %s returns NULL", query);
			free_select_result(res);
			lsn[i] = 0;
		}
		else
		{
			pool_log("%d %s", i, res->data[0]);
			lsn[i] = text_to_lsn(res->data[0]);
			free_select_result(res);
		}
	}

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		if (REAL_MASTER_NODE_ID != i)
		{
			pool_log("DB node id: %d is behind %ld from primary (id: %d)", i, lsn[REAL_MASTER_NODE_ID] - lsn[i], REAL_MASTER_NODE_ID);
		}
	}
}

static long text_to_lsn(char *text)
{
	unsigned int xlogid;
	unsigned int xrecoff;
	long lsn;

	if (sscanf(text, "%X/%X", &xlogid, &xrecoff) != 2)
	{
		pool_error("text_to_lsn: wrong log location format: %s", text);
		return 0;
	}
	lsn = xlogid * 16 * 1024 * 1024 * 255 + xrecoff;
	return lsn;
}

static RETSIGTYPE my_signal_handler(int sig)
{
	switch (sig)
	{
		case SIGHUP:
		got_sighup = 1;
		break;

		case SIGTERM:
		case SIGINT:
		case SIGQUIT:
		default:
			exit(0);
			break;
	}
}
