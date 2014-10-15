/* -*-pgsql-c-*- */
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
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/elog.h"

#include "context/pool_process_context.h"
#include "context/pool_session_context.h"
#include "pool_config.h"
#include "utils/pool_ip.h"
#include "auth/md5.h"
#include "utils/pool_stream.h"

char remote_ps_data[NI_MAXHOST];		/* used for set_ps_display */
static POOL_CONNECTION_POOL_SLOT	*slots[MAX_NUM_BACKENDS];
static volatile sig_atomic_t reload_config_request = 0;
static volatile sig_atomic_t restart_request = 0;

static void establish_persistent_connection(void);
static void discard_persistent_connection(void);
static void check_replication_time_lag(void);
static void CheckReplicationTimeLagErrorCb(void *arg);
static unsigned long long int text_to_lsn(char *text);
static RETSIGTYPE my_signal_handler(int sig);
static RETSIGTYPE reload_config_handler(int sig);
static void reload_config(void);
#define CHECK_REQUEST \
	do { \
		if (reload_config_request) \
		{ \
			reload_config(); \
			reload_config_request = 0; \
		} else if (restart_request) \
		{ \
		  ereport(LOG,(errmsg("worker process received restart request"))); \
		  exit(1); \
		} \
    } while (0)

/*
* worker child main loop
*/
void do_worker_child(void)
{
    sigjmp_buf	local_sigjmp_buf;
	MemoryContext WorkerMemoryContext;
	
	ereport(DEBUG1,
		(errmsg("I am %d", getpid())));

	/* Identify myself via ps */
	init_ps_display("", "", "", "");
	set_ps_display("worker process", false);

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
	WorkerMemoryContext = AllocSetContextCreate(TopMemoryContext,
                                             "Worker_main_loop",
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
        MemoryContextSwitchTo(WorkerMemoryContext);
		MemoryContextResetAndDeleteChildren(WorkerMemoryContext);

		CHECK_REQUEST;

		if (pool_config->sr_check_period <= 0)
		{
			sleep(30);
		}

		/*
		 * If streaming replication mode, do time lag checking
		 */

		if (pool_config->sr_check_period > 0 && MASTER_SLAVE && !strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP))
		{
			establish_persistent_connection();
            PG_TRY();
            {

            	/* Do replication time lag checking */
            	check_replication_time_lag();
            }
            PG_CATCH();
            {
	    		discard_persistent_connection();
	    		sleep(pool_config->sr_check_period);
	    		PG_RE_THROW();
            }
            PG_END_TRY();
            
			/* Discard persistent connections */
			discard_persistent_connection();
		}
		sleep(pool_config->sr_check_period);
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
	
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		if (slots[i] == NULL)
		{
                bkinfo = pool_get_node_info(i);
                slots[i] = make_persistent_db_connection_noerror(bkinfo->backend_hostname,
											  bkinfo->backend_port,
											  "postgres",
											  pool_config->sr_check_user,
											  pool_config->sr_check_password, true);
		}
	}
}

/*
 * Discard persistent connection to backend
 */
static void discard_persistent_connection(void)
{
	int i;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (slots[i])
		{
			discard_persistent_db_connection(slots[i]);
			slots[i] = NULL;
		}
	}
}

/*
 * Check replication time lag
 */
static void check_replication_time_lag(void)
{
	int i;
	int active_nodes = 0;
	POOL_SELECT_RESULT *res;
	unsigned long long int lsn[MAX_NUM_BACKENDS];
	char *query;
	BackendInfo *bkinfo;
	unsigned long long int lag;
	ErrorContextCallback callback;

	if (NUM_BACKENDS <= 1)
	{
		/* If there's only one node, there's no point to do checking */
		return;
	}

	if (REAL_PRIMARY_NODE_ID < 0)
	{
		/* No need to check if there's no primary */
		return;
	}

	/* Count healthy nodes */
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
			active_nodes++;
	}

	if (active_nodes <= 1)
	{
		/* If there's only one or less active node, there's no point
		 * to do checking */
		return;
	}

	/*
	 * Register a error context callback to throw proper context message
	 */
	callback.callback = CheckReplicationTimeLagErrorCb;
	callback.arg = NULL;
	callback.previous = error_context_stack;
	error_context_stack = &callback;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		if (!slots[i])
		{
            ereport(ERROR,
                    (errmsg("Failed to check replication time lag"),
                     errdetail("No persistent db connection for the node %d",i),
                        errhint("check sr_check_user and sr_check_password")));

		}

		if (PRIMARY_NODE_ID == i)
		{
			query = "SELECT pg_current_xlog_location()";
		}
		else
		{
			query = "SELECT pg_last_xlog_replay_location()";
		}

		do_query(slots[i]->con, query, &res, PROTO_MAJOR_V3);

		if (!res)
		{
            ereport(ERROR,
                (errmsg("Failed to check replication time lag"),
                     errdetail("Query to node (%d) returned no result for node",i)));
		}
		if (res->numrows <= 0)
		{
			free_select_result(res);
            ereport(ERROR,
                (errmsg("Failed to check replication time lag"),
                     errdetail("Query to node (%d) returned result with no rows",i)));
		}
		if (res->data[0] == NULL)
		{
			free_select_result(res);
            ereport(ERROR,
                (errmsg("Failed to check replication time lag"),
                     errdetail("Query to node (%d) returned no data",i)));
		}

		if (res->nullflags[0] == -1)
		{
			free_select_result(res);
			lsn[i] = 0;
            ereport(ERROR,
                (errmsg("Failed to check replication time lag"),
                     errdetail("Query to node (%d) returned NULL data",i)));
		}
		else
		{
			lsn[i] = text_to_lsn(res->data[0]);
			free_select_result(res);
		}
	}

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		/* Set standby delay value */
		bkinfo = pool_get_node_info(i);
		lag = (lsn[PRIMARY_NODE_ID] > lsn[i]) ? lsn[PRIMARY_NODE_ID] - lsn[i] : 0;

		if (PRIMARY_NODE_ID == i)
		{
			bkinfo->standby_delay = 0;
		}
		else
		{
			bkinfo->standby_delay = lag;

			/* Log delay if necessary */
			if ((!strcmp(pool_config->log_standby_delay, "always") && lag > 0) ||
				(pool_config->delay_threshold &&
				 !strcmp(pool_config->log_standby_delay, "if_over_threshold") &&
				 lag > pool_config->delay_threshold))
			{
                ereport(LOG,
                        (errmsg("Replication of node:%d is behind %llu bytes from the primary server (node:%d)",
                                i, lsn[PRIMARY_NODE_ID] - lsn[i], PRIMARY_NODE_ID)));
			}
		}
	}

	error_context_stack = callback.previous;
}

static void CheckReplicationTimeLagErrorCb(void *arg)
{
	errcontext("while checking replication time lag");
}
/*
 * Convert logid/recoff style text to 64bit log location (LSN)
 */
static unsigned long long int text_to_lsn(char *text)
{
/*
 * WAL segment size in bytes.  XXX We should fetch this from
 * PostgreSQL, rather than having fixed value.
 */
#define WALSEGMENTSIZE 16 * 1024 * 1024

	unsigned int xlogid;
	unsigned int xrecoff;
	unsigned long long int lsn;

	if (sscanf(text, "%X/%X", &xlogid, &xrecoff) != 2)
	{
        ereport(ERROR,
            (errmsg("invalid LSN format"),
                 errdetail("wrong log location format: %s", text)));

	}
	lsn = xlogid * ((unsigned long long int)0xffffffff - WALSEGMENTSIZE) + xrecoff;
#ifdef DEBUG
	ereport(LOG,
			(errmsg("lsn: %X %X %llX", xlogid, xrecoff, lsn)));
#endif
	return lsn;
}

static RETSIGTYPE my_signal_handler(int sig)
{
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
}

static RETSIGTYPE reload_config_handler(int sig)
{
	POOL_SETMASK(&BlockSig);
	reload_config_request = 1;
	POOL_SETMASK(&UnBlockSig);
}

static void reload_config(void)
{
	ereport(LOG,
			(errmsg("reloading config file")));
    MemoryContext oldContext = MemoryContextSwitchTo(TopMemoryContext);
	pool_get_config(get_config_file_name(), RELOAD_CONFIG);
    MemoryContextSwitchTo(oldContext);
	if (pool_config->enable_pool_hba)
		load_hba(get_hba_file_name());
	reload_config_request = 0;
}
