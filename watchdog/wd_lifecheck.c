/*
 * $Header$
 *
 * Handles watchdog connection, and protocol communication with pgpool-II
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
static void * ping_pgpool(void * arg);
static PGconn * create_conn(char * hostname, int port);
static int pgpool_down(WdInfo * pool);

int
is_wd_lifecheck_ready(void)
{
	PGconn * conn = NULL;
	PGresult * res = (PGresult *)NULL;
	int status;
	int rtn = WD_OK;
	WdInfo * p = WD_List;
	while (p->status != WD_END)
	{
		conn = create_conn(p->hostname, p->pgpool_port);
		if (conn != NULL)
		{
			res = PQexec(conn, pool_config->wd_lifecheck_query );

			status = PQresultStatus(res);
			if (res != NULL)
			{
				PQclear(res);
			}
			if ((status == PGRES_NONFATAL_ERROR )|| 
				(status == PGRES_FATAL_ERROR ))
			{
				rtn = WD_NG;
			}
			PQfinish(conn);
		}
		else
		{
			rtn = WD_NG;
		}
		p ++;
	}
	return rtn;
}

int
wd_lifecheck(void)
{
	WdInfo * p = WD_List;
	pthread_attr_t attr;
	int rc;
	int i,cnt;
	struct timeval tv;
	pthread_t thread[MAX_WATCHDOG_NUM];
	WdPgpoolThreadArg thread_arg[MAX_WATCHDOG_NUM];

	/* set startup time */
	gettimeofday(&tv, NULL);

	/* check upper connection */
	if ((pool_config->trusted_servers != NULL) &&
		(wd_is_upper_ok(pool_config->trusted_servers) != WD_OK))
	{
		pool_error("failed to connect trusted server");
		/* This server connection may be downwd */
		if (p->status == WD_MASTER)
		{
			wd_IP_down();
		}
		wd_set_myself(&tv, WD_DOWN);
		wd_notice_server_down();
		return WD_NG;
	}

	/* thread init */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	/* send packet to all watchdogs */
	cnt = 0;
	while (p->status != WD_END)
	{
		thread_arg[cnt].conn = create_conn(p->hostname, p->pgpool_port);
		rc = pthread_create(&thread[cnt], &attr, ping_pgpool, (void*)&thread_arg[cnt]);
		p ++;
		cnt ++;
		if (cnt >= MAX_WATCHDOG_NUM)
		{
			pool_error("pgpool num is out of range(%d)",cnt);	
			break;
		}
	}
	pthread_attr_destroy(&attr);
	p = WD_List;
	for (i = 0; i < cnt; )
	{
		int result;
		rc = pthread_join(thread[i], (void **)&result);
		if ((rc != 0) && (errno == EINTR))
		{
			usleep(100);
			continue;
		}
		if (result == WD_OK)
		{
			p->life = pool_config->wd_life_point;
			if ((i == 0) &&
				(WD_List->status == WD_DOWN))
			{
				wd_set_myself(&tv, WD_NORMAL);
				wd_startup();
				/* check existence of master pgpool */
				if (wd_is_exist_master() == NULL )
				{
					/* escalate to delegate_IP holder */
					wd_escalation();
				}
			}
		}
		else
		{
			if (p->life > 0)
			{
				p->life --;
			}
			if (p->life <= 0)
			{
				if ((i == 0) &&
					(WD_List->status != WD_DOWN))
				{
					wd_set_myself(&tv, WD_DOWN);
					wd_notice_server_down();
				}
				else
				{
					pgpool_down(p);
				}
			}
		}
		pthread_detach(thread[i]);
		i++;
		p++;
	}
	return WD_OK;
}

static void *
ping_pgpool(void * arg)
{
	WdPgpoolThreadArg * thread_arg;
	PGconn * conn;
	uintptr_t rtn = (uintptr_t)WD_NG;
	int status = PGRES_FATAL_ERROR;
	PGresult * res = (PGresult *)NULL;
	thread_arg = (WdPgpoolThreadArg *)arg;
	conn = thread_arg->conn;

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
	pthread_exit((void *)rtn);
}

static PGconn *
create_conn(char * hostname, int port)
{
	char port_str[16];
	PGconn *conn;

	snprintf(port_str, sizeof(port_str),
			 "%d", port);
	conn = PQsetdbLogin(hostname,
						port_str,
						NULL,
						NULL,
						"template1",
						pool_config->recovery_user,
						pool_config->recovery_password);

	if (PQstatus(conn) != CONNECTION_OK)
	{
		PQfinish(conn);
		return NULL;
	}
	return conn;
}

static int
pgpool_down(WdInfo * pool)
{
	int rtn = WD_DOWN;
	if ((WD_List->status == WD_NORMAL) &&
		(pool->status == WD_MASTER))
	{
		pool->status = WD_DOWN;
		if (wd_am_I_oldest() == WD_OK)
		{
			/* stand for master */
			rtn = wd_stand_for_master();
			if (rtn == WD_OK)
			{
				/* win */
				wd_escalation();
			}
		}
	}
	pool->status = WD_DOWN;
	return rtn;
}
