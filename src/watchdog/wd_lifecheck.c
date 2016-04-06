/*
 * $Header$
 *
 * Handles watchdog connection, and protocol communication with pgpool-II
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2016	PgPool Global Development Group
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
 */
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include "pool.h"
#include "utils/elog.h"
#include "pool_config.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "parser/pg_list.h"
#include "watchdog/watchdog.h"
#include "watchdog/wd_ext.h"

#include "libpq-fe.h"

typedef struct WdUpstreamConnectionData
{
	char*		hostname;	/* host name of server */
	pid_t		pid;		/* pid of ping process */
	bool		reachable;	/* true if last ping was successful */
	int			outputfd;	/* pipe fd linked to output of ping process */
}WdUpstreamConnectionData;

List* g_trusted_server_list = NIL;

static void wd_initialize_trusted_servers_list(void);
static bool wd_ping_all_server(void);
static WdUpstreamConnectionData* wd_get_server_from_pid(pid_t pid);

static void * thread_ping_pgpool(void * arg);
static PGconn * create_conn(char * hostname, int port);
static int pgpool_down(WdInfo * pool);

static void check_pgpool_status(void);
static void check_pgpool_status_by_query(void);
static void check_pgpool_status_by_hb(void);
static int ping_pgpool(PGconn * conn);
static int is_parent_alive(void);
static int wd_lifecheck(void);

static void wd_lifecheck_exit(int exit_status);

static void
wd_lifecheck_exit(int exit_signo)
{
	sigset_t mask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	wd_notice_server_down();

	exit(0);
}


/* fork lifecheck process*/
pid_t
fork_a_lifecheck(int fork_wait_time)
{
	pid_t pid;
	sigjmp_buf	local_sigjmp_buf;

	pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			ereport(ERROR,
					(errmsg("failed to fork a lifecheck process")));
		return pid;
	}
	on_exit_reset();
	processType = PT_LIFECHECK;

	if (fork_wait_time > 0) {
		sleep(fork_wait_time);
	}

	POOL_SETMASK(&UnBlockSig);

	init_ps_display("", "", "", "");

	pool_signal(SIGTERM, wd_lifecheck_exit);
	pool_signal(SIGINT, wd_lifecheck_exit);
	pool_signal(SIGQUIT, wd_lifecheck_exit);
	pool_signal(SIGCHLD, SIG_DFL);
	pool_signal(SIGHUP, SIG_IGN);
	pool_signal(SIGPIPE, SIG_IGN);
	
	/* Create per loop iteration memory context */
	ProcessLoopContext = AllocSetContextCreate(TopMemoryContext,
											   "wd_lifecheck_main_loop",
											   ALLOCSET_DEFAULT_MINSIZE,
											   ALLOCSET_DEFAULT_INITSIZE,
											   ALLOCSET_DEFAULT_MAXSIZE);

	MemoryContextSwitchTo(TopMemoryContext);

	set_ps_display("lifecheck",false);

	wd_initialize_trusted_servers_list();

	/* wait until ready to go */
	while (WD_OK != is_wd_lifecheck_ready())
	{
		sleep(pool_config->wd_interval * 10);
	}
	ereport(LOG,
			(errmsg("watchdog: lifecheck started")));

	if (sigsetjmp(local_sigjmp_buf, 1) != 0)
	{
		/* Since not using PG_TRY, must reset error stack by hand */
		error_context_stack = NULL;
	
		EmitErrorReport();
		MemoryContextSwitchTo(TopMemoryContext);
		FlushErrorState();
		sleep(pool_config->wd_heartbeat_keepalive);
	}

	/* We can now handle ereport(ERROR) */
	PG_exception_stack = &local_sigjmp_buf;

	/* watchdog loop */
	for (;;)
	{
		MemoryContextSwitchTo(ProcessLoopContext);
		MemoryContextResetAndDeleteChildren(ProcessLoopContext);

		/* pgpool life check */
		wd_lifecheck();
		sleep(pool_config->wd_interval);
	}

	return pid;
}

int
is_wd_lifecheck_ready(void)
{
	int rtn = WD_OK;
	WdInfo * p = WD_List;
	int i = 0;

	while (p->status != WD_END)
	{
		/* query mode */
		if (!strcmp(pool_config->wd_lifecheck_method, MODE_QUERY))
		{
			if (wd_ping_pgpool(p) == WD_NG)
			{
				ereport(DEBUG1,
					(errmsg("watchdog checking life check is ready"),
						errdetail("pgpool:%d at \"%s:%d\" has not started yet",
							   i, p->hostname, p->pgpool_port)));
				rtn = WD_NG;
			}
		}
		/* heartbeat mode */
		else if (!strcmp(pool_config->wd_lifecheck_method, MODE_HEARTBEAT))
		{
			if (p == WD_List)
			{
				p++;
				i++;
				continue;
			}

			if (!WD_TIME_ISSET(p->hb_last_recv_time) ||
			    !WD_TIME_ISSET(p->hb_send_time))
			{
				ereport(DEBUG1,
					(errmsg("watchdog checking life check is ready"),
						errdetail("pgpool:%d at \"%s:%d\" has not send the heartbeat signal yet",
							   i, p->hostname, p->pgpool_port)));
				rtn = WD_NG;
			}
		}
		/* otherwise */
		else
		{
			ereport(ERROR,
				(errmsg("checking if watchdog is ready, unkown watchdog mode \"%s\"",
							pool_config->wd_lifecheck_method)));
		}

		p ++;
		i ++;
	}

	return rtn;
}

/*
 * Check if pgpool is living
 */
static int
wd_lifecheck(void)
{
	struct timeval tv;

	/* I'm in down.... */
	if (WD_MYSELF->status == WD_DOWN)
	{
		ereport(NOTICE,
				(errmsg("watchdog lifecheck, watchdog status is DOWN. You need to restart pgpool")));
		return WD_NG;
	}

	/* set startup time */
	gettimeofday(&tv, NULL);

	/* check upper connection */
	if (strlen(pool_config->trusted_servers))
	{
		if(wd_ping_all_server() == false)
		{
			ereport(WARNING,
					(errmsg("watchdog lifecheck, failed to connect to any trusted servers")));

			if (WD_MYSELF->status == WD_MASTER &&
				strlen(pool_config->delegate_IP) != 0)
			{
				wd_IP_down();
			}
			wd_set_myself(&tv, WD_DOWN);
			wd_notice_server_down();

			return WD_NG;
		}
	}

	/* skip lifecheck during recovery execution */
	if (*InRecovery != RECOVERY_INIT)
	{
		return WD_OK;
	}

	/* check and update pgpool status */
	check_pgpool_status();

	return WD_OK;
}

/*
 * check and update pgpool status
 */
static void
check_pgpool_status()
{
	/* query mode */
	if (!strcmp(pool_config->wd_lifecheck_method, MODE_QUERY))
	{
		check_pgpool_status_by_query();
	}
	/* heartbeat mode */
	else if (!strcmp(pool_config->wd_lifecheck_method, MODE_HEARTBEAT))
	{
		check_pgpool_status_by_hb();
	}
}

static void
check_pgpool_status_by_hb(void)
{
	int cnt;
	WdInfo *p = WD_List;
	struct timeval tv;

	gettimeofday(&tv, NULL);

	cnt = 0;
	while (p->status != WD_END)
	{
		ereport(DEBUG1,
			(errmsg("watchdog life checking by heartbeat"),
				errdetail("checking pgpool %d (%s:%d)",
					   cnt, p->hostname, p->pgpool_port)));

		/* about myself */
		if (p == WD_MYSELF)
		{
			/* parent is dead so it's orphan.... */
			if (is_parent_alive() == WD_NG && WD_MYSELF->status != WD_DOWN)
			{
				ereport(LOG,
					(errmsg("checking pgpool status by heartbeat"),
						errdetail("lifecheck failed. pgpool %d (%s:%d) seems not to be working",
							   cnt, p->hostname, p->pgpool_port)));

				wd_set_myself(&tv, WD_DOWN);
				wd_notice_server_down();
			}
			/* otherwise, the parent would take care of children. */
			else
			{
				ereport(DEBUG1,
					(errmsg("watchdog life checking by heartbeat"),
						 errdetail("OK; status %d", p->status)));
			}
		}

		/*  about other pgpools, check the latest heartbeat. */
		else
		{
			if (p->status == WD_DOWN)
			{
				ereport(LOG,
					(errmsg("checking pgpool status by heartbeat"),
						 errdetail("pgpool: %d at \"%s:%d\" status is down",
								   cnt, p->hostname, p->pgpool_port)));

			}
			else if (wd_check_heartbeat(p) == WD_NG)
			{
				ereport(DEBUG1,
						(errmsg("checking pgpool status by heartbeat"),
						 errdetail("NG; status %d", p->status)));

				ereport(LOG,
					(errmsg("checking pgpool status by heartbeat"),
						 errdetail("lifecheck failed. pgpool: %d at \"%s:%d\" seems not to be working",
								   cnt, p->hostname, p->pgpool_port)));

				if (p->status != WD_DOWN)
					pgpool_down(p);
			}
			else
			{
				ereport(DEBUG1,
					(errmsg("checking pgpool status by heartbeat"),
						 errdetail("OK; status %d", p->status)));
			}
		}

		p++;
		cnt++;
		if (cnt >= MAX_WATCHDOG_NUM)
		{
			ereport(WARNING,
					(errmsg("checking pgpool status by heartbeat, pgpool num is out of range:%d",cnt)));
			break;
		}
	}
}

static void
check_pgpool_status_by_query(void)
{
	WdInfo * p = WD_List;
	struct timeval tv;
	pthread_attr_t attr;
	pthread_t thread[MAX_WATCHDOG_NUM];
	WdPgpoolThreadArg thread_arg[MAX_WATCHDOG_NUM];
	int rc;
	int i,cnt;

	/* set startup time */
	gettimeofday(&tv, NULL);

	/* thread init */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	/* send queries to all pgpools using threads */
	cnt = 0;
	while (p->status != WD_END)
	{
		if (p->status != WD_DOWN)
		{
			thread_arg[cnt].conn = create_conn(p->hostname, p->pgpool_port);
			rc = watchdog_thread_create(&thread[cnt], &attr, thread_ping_pgpool, (void*)&thread_arg[cnt]);
		}
		p ++;
		cnt ++;
		if (cnt >= MAX_WATCHDOG_NUM)
		{
			ereport(WARNING,
					(errmsg("checking pgpool status by query, pgpool num is out of range:%d",cnt)));
			break;
		}
	}
	pthread_attr_destroy(&attr);

	/* check results of queries */
	p = WD_List;
	for (i = 0; i < cnt; )
	{
		int result;

		ereport(DEBUG1,
				(errmsg("checking pgpool status by query"),
					errdetail("checking pgpool %d (%s:%d)",
						   i, p->hostname, p->pgpool_port)));

		if (p->status == WD_DOWN)
		{
			ereport(LOG,
				(errmsg("checking pgpool status by query"),
					errdetail("pgpool %d (%s:%d) is in down status",
						   i, p->hostname, p->pgpool_port)));
			i++;
			p++;
			continue;
		}
		else
		{
			rc = pthread_join(thread[i], (void **)&result);
			if ((rc != 0) && (errno == EINTR))
			{
				usleep(100);
				continue;
			}
		}

		if (result == WD_OK)
		{
			ereport(DEBUG1,
				(errmsg("checking pgpool status by query"),
					 errdetail("WD_OK: status: %d", p->status)));

			/* life point init */
			p->life = pool_config->wd_life_point;
		}
		else
		{
			ereport(DEBUG1,
				(errmsg("checking pgpool status by query"),
					 errdetail("NG; status: %d life:%d", p->status, p->life)));
			if (p->life > 0)
			{
				p->life --;
			}

			/* pgpool goes down */
			if (p->life <= 0)
			{
				ereport(LOG,
					(errmsg("checking pgpool status by query"),
						errdetail("lifecheck failed %d times. pgpool %d (%s:%d) seems not to be working",
								   pool_config->wd_life_point, i, p->hostname, p->pgpool_port)));

				/* It's me! */
				if ((i == 0) &&
					(WD_MYSELF->status != WD_DOWN))
				{
					wd_set_myself(&tv, WD_DOWN);
					wd_notice_server_down();
				}

				/* It's other pgpool */
				else if (p->status != WD_DOWN)
					pgpool_down(p);
			}
		}
		i++;
		p++;
	}
}

/*
 * Thread function to send lifecheck query to pgpool
 * Used in wd_lifecheck.
 */
static void *
thread_ping_pgpool(void * arg)
{
	uintptr_t rtn;
	WdPgpoolThreadArg * thread_arg;
	PGconn * conn;

	thread_arg = (WdPgpoolThreadArg *)arg;
	conn = thread_arg->conn;
	rtn = (uintptr_t)ping_pgpool(conn);

	pthread_exit((void *)rtn);
}

/*
 * Create connection to pgpool
 */
static PGconn *
create_conn(char * hostname, int port)
{
	static char conninfo[1024];
	PGconn *conn;

	if (strlen(pool_config->wd_lifecheck_dbname) == 0)
	{
		ereport(WARNING,
				(errmsg("watchdog life checking, wd_lifecheck_dbname is empty")));
		return NULL;
	}

	if (strlen(pool_config->wd_lifecheck_user) == 0)
	{
		ereport(WARNING,
				(errmsg("watchdog life checking, wd_lifecheck_user is empty")));
		return NULL;
	}

	snprintf(conninfo,sizeof(conninfo),
		"host='%s' port='%d' dbname='%s' user='%s' password='%s' connect_timeout='%d'",
		hostname,
		port,
		pool_config->wd_lifecheck_dbname,
		pool_config->wd_lifecheck_user,
		pool_config->wd_lifecheck_password,
		pool_config->wd_interval / 2 + 1);
	conn = PQconnectdb(conninfo);

	if (PQstatus(conn) != CONNECTION_OK)
	{
		ereport(DEBUG1,
			(errmsg("watchdog life checking"),
				 errdetail("Connection to database failed: %s", PQerrorMessage(conn))));
		PQfinish(conn);
		return NULL;
	}
	return conn;
}

/* handle other pgpool's down */
static int
pgpool_down(WdInfo * pool)
{
	int rtn = WD_OK;
	WD_STATUS prev_status;

	ereport(LOG,
		(errmsg("active pgpool goes down"),
			 errdetail("pgpool on %s:%d down",
					   pool->hostname, pool->pgpool_port)));

	prev_status = pool->status;
	pool->status = WD_DOWN;

	/* the active pgpool goes down and I'm sandby pgpool */
	if (prev_status == WD_MASTER && WD_MYSELF->status == WD_NORMAL)
	{
		if (wd_am_I_oldest() == WD_OK)
		{
			ereport(LOG,
				(errmsg("active pgpool goes down"),
					 errdetail("I am the oldest, so standing for master")));

			/* stand for master */
			rtn = wd_stand_for_master();
			if (rtn == WD_OK)
			{
				/* win */
				wd_escalation();
			}
			else
			{
				/* rejected by others */
				pool->status = prev_status;
			}
		}
	}

	return rtn;
}

/*
 * Check if pgpool is alive using heartbeat signal.
 */
int
wd_check_heartbeat(WdInfo * pgpool)
{
	int interval;
	struct timeval tv;

	if (!WD_TIME_ISSET(pgpool->hb_last_recv_time) ||
	    !WD_TIME_ISSET(pgpool->hb_send_time))
	{
		ereport(DEBUG1,
			(errmsg("watchdog checking if pgpool is alive using heartbeat"),
				errdetail("pgpool (%s:%d) was restarted and has not send the heartbeat signal yet",
					   pgpool->hostname, pgpool->pgpool_port)));
		return WD_OK;
	}

	gettimeofday(&tv, NULL);

	interval = WD_TIME_DIFF_SEC(tv, pgpool->hb_last_recv_time);
	ereport(DEBUG1,
		(errmsg("watchdog checking if pgpool is alive using heartbeat"),
			errdetail("the last heartbeat from \"%s:%d\" received %d seconds ago",
				   pgpool->hostname, pgpool->pgpool_port, interval)));

	if (interval > pool_config->wd_heartbeat_deadtime)
		return WD_NG;
	else
		return WD_OK;
}

/*
 * Check if pgpool can accept the lifecheck query.
 */
int
wd_ping_pgpool(WdInfo * pgpool)
{
	int rtn;
	PGconn * conn;

	conn = create_conn(pgpool->hostname, pgpool->pgpool_port);
	rtn = ping_pgpool(conn);

	return rtn;
}

/* inner function for issueing lifecheck query */
static int
ping_pgpool(PGconn * conn)
{
	int rtn = WD_NG;
	int status = PGRES_FATAL_ERROR;
	PGresult * res = (PGresult *)NULL;

	if (!conn)
	{
		return WD_NG;
	}

	res = PQexec(conn, pool_config->wd_lifecheck_query );

	status = PQresultStatus(res);
	if (res != NULL)
	{
		PQclear(res);
	}

	if ((status != PGRES_NONFATAL_ERROR ) &&
		(status != PGRES_FATAL_ERROR ))
	{
		rtn = WD_OK;
	}
	PQfinish(conn);

	return rtn;
}

static int
is_parent_alive()
{
	if (mypid == getppid())
		return WD_OK;
	else
		return WD_NG;
}

static void wd_initialize_trusted_servers_list(void)
{
	char* token;
	char* tmpString;
	const char* delimi = ",";
	if (g_trusted_server_list)
		return;

	if (strlen(pool_config->trusted_servers) <= 0)
		return;

	/* This has to be created in TopMemoryContext */
	MemoryContext oldCxt = MemoryContextSwitchTo(TopMemoryContext);

	tmpString = pstrdup(pool_config->trusted_servers);
	for (token = strtok(tmpString, delimi); token != NULL; token = strtok(NULL, delimi))
	{
		WdUpstreamConnectionData* server = palloc(sizeof(WdUpstreamConnectionData));
		server->pid = 0;
		server->reachable = false;
		server->hostname = token;
		g_trusted_server_list = lappend(g_trusted_server_list,server);

		ereport(LOG,
				(errmsg("watchdog lifecheck trusted server \"%s\" added for the availability check",token)));
	}
	MemoryContextSwitchTo(oldCxt);
}

static bool wd_ping_all_server(void)
{
	ListCell *lc;
	pid_t pid;
	int status;
	int ping_process = 0;

	foreach(lc, g_trusted_server_list)
	{
		WdUpstreamConnectionData* server = (WdUpstreamConnectionData*)lfirst(lc);
		if (server->pid <= 0)
			server->pid = wd_issue_ping_command(server->hostname,&server->outputfd);
		
		if (server->pid > 0)
			ping_process++;
	}

	while(ping_process > 0)
	{
		pid = waitpid(0, &status, 0);
		if (pid > 0)
		{
			/* find the server object associated with this pid */
			WdUpstreamConnectionData* server = wd_get_server_from_pid(pid);
			if (server)
			{
				ping_process--;
				server->reachable = wd_get_ping_result(server->hostname, status, server->outputfd);
				server->pid = 0;
				close(server->outputfd);
				if (server->reachable)
				{
					/* one reachable server is all we need */
					return true;
				}
			}
		}
		if (pid == -1) /* wait pid error */
		{
			if (errno == EINTR)
				continue;
			ereport(WARNING,
				(errmsg("failed to check the ping status of trusted servers"),
					 errdetail("waitpid failed with reason: %s", strerror(errno))));
			break;
		}
	}
	return false;
}

static WdUpstreamConnectionData* wd_get_server_from_pid(pid_t pid)
{
	ListCell *lc;

	foreach(lc, g_trusted_server_list)
	{
		WdUpstreamConnectionData* server = (WdUpstreamConnectionData*)lfirst(lc);
		if (server->pid == pid)
		{
			return server;
		}
	}
	return NULL;
}

