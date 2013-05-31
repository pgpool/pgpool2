/*
 * $Header$
 *
 * Handles watchdog connection, and protocol communication with pgpool-II
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
 */
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include "pool.h"
#include "pool_config.h"
#include "watchdog.h"
#include "wd_ext.h"

#include "libpq-fe.h"

int is_wd_lifecheck_ready(void);
int wd_lifecheck(void);
int wd_ping_pgpool(WdInfo * pgpool);
static void * thread_ping_pgpool(void * arg);
static PGconn * create_conn(char * hostname, int port);
static int pgpool_down(WdInfo * pool);

static void check_pgpool_status(void);
static void check_pgpool_status_by_query(void);
static void check_pgpool_status_by_hb(void);
static int ping_pgpool(PGconn * conn);
static int is_parent_alive(void);

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
				pool_debug("is_wd_lifecheck_ready: pgpool %d (%s:%d) has not started yet",
				           i, p->hostname, p->pgpool_port);
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
				pool_debug("is_wd_lifecheck_ready: pgpool %d (%s:%d) has not send the heartbeat signal yet",
				           i, p->hostname, p->pgpool_port);
				rtn = WD_NG;
			}
		}
		/* otherwise */
		else
		{
			pool_error("is_wd_lifecheck_ready: unkown watchdog mode %s",
			           pool_config->wd_lifecheck_method);
			return WD_NG;
		}

		p ++;
		i ++;
	}

	return rtn;
}

/*
 * Check if pgpool is living
 */
int
wd_lifecheck(void)
{
	struct timeval tv;

	/* I'm in down.... */
	if (WD_MYSELF->status == WD_DOWN)
	{
		pool_error("wd_lifecheck: watchdog status is DOWN. You need to restart pgpool");
		return WD_NG;
	}

	/* set startup time */
	gettimeofday(&tv, NULL);

	/* check upper connection */
	if (strlen(pool_config->trusted_servers) &&
		wd_is_upper_ok(pool_config->trusted_servers) != WD_OK)
	{
		pool_error("wd_lifecheck: failed to connect to any trusted servers");

		if (WD_MYSELF->status == WD_MASTER)
		{
			wd_IP_down();
		}

		wd_set_myself(&tv, WD_DOWN);
		wd_notice_server_down();

		return WD_NG;
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
	WdInfo * p = WD_List;
	struct timeval tv;

	gettimeofday(&tv, NULL);

	cnt = 0;
	while (p->status != WD_END)
	{
		pool_debug("check_pgpool_status_by_hb: checking pgpool %d (%s:%d)",
		           cnt, p->hostname, p->pgpool_port);

		/* about myself */
		if (p == WD_MYSELF)
		{
			/* parent is dead so it's orphan.... */
			if (is_parent_alive() == WD_NG && WD_MYSELF->status != WD_DOWN)
			{
				pool_debug("check_pgpool_status_by_hb: NG; the main pgpool process does't exist.");
				pool_log("check_pgpool_status_by_hb: lifecheck failed. pgpool %d (%s:%d) seems not to be working",
		                 cnt, p->hostname, p->pgpool_port);
				wd_set_myself(&tv, WD_DOWN);
				wd_notice_server_down();
			}
			/* otherwise, the parent would take care of children. */
			else
			{
				pool_debug("check_pgpool_status_by_hb: OK; status %d", p->status);
			}
		}

		/*  about other pgpools, check the latest heartbeat. */
		else
		{
			if (wd_check_heartbeat(p) == WD_NG)
			{
				pool_debug("check_pgpool_status_by_hb: NG; status %d", p->status);

				pool_log("check_pgpool_status_by_hb: lifecheck failed. pgpool %d (%s:%d) seems not to be working",
		                 cnt, p->hostname, p->pgpool_port);

				if (p->status != WD_DOWN)
					pgpool_down(p);
			}
			else
			{
				pool_debug("check_pgpool_status_by_hb: OK; status %d", p->status);
			}
		}

		p++;
		cnt++;
		if (cnt >= MAX_WATCHDOG_NUM)
		{
			pool_error("check_pgpool_status_by_hb: pgpool num is out of range(%d)",cnt);
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
		thread_arg[cnt].conn = create_conn(p->hostname, p->pgpool_port);
		rc = pthread_create(&thread[cnt], &attr, thread_ping_pgpool, (void*)&thread_arg[cnt]);
		p ++;
		cnt ++;
		if (cnt >= MAX_WATCHDOG_NUM)
		{
			pool_error("check_pgpool_status_by_query: pgpool num is out of range(%d)",cnt);
			break;
		}
	}
	pthread_attr_destroy(&attr);

	/* check results of queries */
	p = WD_List;
	for (i = 0; i < cnt; )
	{
		int result;

		pool_debug("check_pgpool_status_by_qury: checking pgpool %d (%s:%d)",
		           i, p->hostname, p->pgpool_port);

		rc = pthread_join(thread[i], (void **)&result);
		if ((rc != 0) && (errno == EINTR))
		{
			usleep(100);
			continue;
		}

		if (result == WD_OK)
		{
			pool_debug("check_pgpool_status_by_query: OK; status: %d", p->status);

			/* life point init */
			p->life = pool_config->wd_life_point;
		}
		else
		{
			pool_debug("check_pgpool_status_by_query: NG; status: %d life:%d", p->status, p->life);

			if (p->life > 0)
			{
				p->life --;
			}

			/* pgpool goes down */
			if (p->life <= 0)
			{
				pool_log("check_pgpool_status_by_query: lifecheck failed %d times. pgpool %d (%s:%d) seems not to be working",
				         pool_config->wd_life_point, i, p->hostname, p->pgpool_port);

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
		pool_error("create_conn: wd_lifecheck_dbname is empty");
		return NULL;
	}

	if (strlen(pool_config->wd_lifecheck_user) == 0)
	{
		pool_error("create_conn: wd_lifecheck_user is empty");
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
		pool_debug("create_conn: Connection to database failed: %s", PQerrorMessage(conn));
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

	pool_log("pgpool_down: %s:%d is going down",
	         pool->hostname, pool->pgpool_port);

	prev_status = pool->status;
	pool->status = WD_DOWN;

	/* the active pgpool goes down and I'm sandby pgpool */
	if (prev_status == WD_MASTER && WD_MYSELF->status == WD_NORMAL)
	{
		if (wd_am_I_oldest() == WD_OK)
		{
			pool_log("pgpool_down: I'm oldest so standing for master");
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

	gettimeofday(&tv, NULL);

	interval = WD_TIME_DIFF_SEC(tv, pgpool->hb_last_recv_time);
	pool_debug("wd_check_heartbeat: the latest heartbeat from %s:%d received %d seconds ago",
	           pgpool->hostname, pgpool->pgpool_port, interval);

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
