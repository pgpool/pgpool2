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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "pool.h"
#include "pool_config.h"
#include "watchdog.h"

#define WD_MAX_PING_RESULT 256

int wd_is_upper_ok(char * server_list);
int wd_is_unused_ip(char * ip);

static void * exec_ping(void * arg);
static double get_result (char * ping_data);

/**
 * Try to conect to trusted servers.
 */
int
wd_is_upper_ok(char * server_list)
{
	pthread_attr_t attr;
	char * buf;
	int rc = 0;
	int i,cnt;
	int len;
	pthread_t thread[MAX_WATCHDOG_NUM];
	WdInfo thread_arg[MAX_WATCHDOG_NUM];

	char * bp, *ep;
	int rtn = WD_NG;

	if (server_list == NULL)
	{
		return WD_NG;
	}
	len = strlen(server_list)+2;
	buf = malloc(len);
	memset(buf,0,len);
	strlcpy(buf,server_list,len);
	/* thread init */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	/* set hostname as a thread_arg */
	bp = buf;
	cnt = 0;
	while (*bp != '\0')
	{
		ep = strchr(bp,',');
		if (ep != NULL)
		{
			*ep = '\0';
		}
		strlcpy(thread_arg[cnt].hostname,bp,sizeof(thread_arg[cnt].hostname));
		rc = pthread_create(&thread[cnt], &attr, exec_ping, (void*)&thread_arg[cnt]);

		cnt ++;
		if (ep != NULL)
		{
			bp = ep + 1;
		}
		else
		{
			break;
		}
		if (cnt >= MAX_WATCHDOG_NUM)
		{
			pool_debug("wd_is_upper_ok: trusted server num is out of range(%d)",cnt);
			break;
		}
	}
	pthread_attr_destroy(&attr);
	for (i=0; i <cnt; )
	{
		void * result;
		rc = pthread_join(thread[i], &result);
		if ((rc != 0) && (errno == EINTR))
		{
			usleep(100);
			continue;
		}
		if (result == (void *)WD_OK)
		{
			rtn = WD_OK;
		}
		i++;
	}
	free(buf);
	return rtn;
}

/**
 * check if IP addres is unused.
 */
int
wd_is_unused_ip(char * ip)
{
	pthread_attr_t attr;
	int rc = 0;
	pthread_t thread;
	WdInfo thread_arg;

	int rtn = WD_NG;
	void * result;

	if (ip == NULL)
	{
		return WD_NG;
	}

	/* thread init */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	/* set hostname as a thread_arg */
	strlcpy(thread_arg.hostname,ip,sizeof(thread_arg.hostname));

	rc = pthread_create(&thread, &attr, exec_ping, (void*)&thread_arg);
	pthread_attr_destroy(&attr);

	rc = pthread_join(thread, &result);
	if ((rc != 0) && (errno == EINTR))
	{
		return WD_NG;
	}
	if (result == (void *)WD_NG)
	{
		rtn = WD_OK;
	}

	return rtn;
}


static void *
exec_ping(void * arg)
{
	WdInfo * thread_arg;
	uintptr_t rtn = (uintptr_t)WD_NG;
	int pfd[2];
	int status;
	char * args[8];
	int pid, i = 0;
	int r_size = 0;
	char result[WD_MAX_PING_RESULT];
	char ping_path[WD_MAX_PATH_LEN];

	snprintf(ping_path,sizeof(ping_path),"%s/ping",pool_config->ping_path);
	thread_arg = (WdInfo *)arg;
	memset(result,0,sizeof(result));
	if (pipe(pfd) == -1)
	{
		fprintf(stderr,"pipe open error:%s\n",strerror(errno));
		return NULL;
	}

	args[i++] = "ping";
	args[i++] = "-q";
	args[i++] = "-c3";
	args[i++] = thread_arg->hostname;
	args[i++] = NULL;

	pid = fork();
	if (pid == 0)
	{
		close(STDOUT_FILENO);
		dup2(pfd[1], STDOUT_FILENO);
		close(pfd[0]);
		status = execv(ping_path,args);
		exit(0);
	}
	else
	{
		close(pfd[1]);
		for (;;)
		{
			int r;
			r = waitpid(pid, &status, 0);
			if (r < 0)
			{
				if (errno == EINTR)
					continue;
				return NULL;
			}

			if (WIFEXITED(status) == 0 || WEXITSTATUS(status) != 0)
				return NULL;
			else
				break;
		}
		i = 0;
		while  (( (r_size = read (pfd[0], &result[i], sizeof(result)-i-1)) > 0) && (errno == EINTR))
		{
			i += r_size;
		}
		result[sizeof(result)-1] = '\0';

		close(pfd[0]);
	}

	/* Check whether average RTT >= 0 */
	rtn = (get_result (result) >= 0) ? WD_OK : WD_NG;

	pthread_exit((void *)rtn);
}

static double
get_result (char * ping_data)
{
	char * sp = NULL;
	char * ep = NULL;
	int i;
	double msec = 0;

	if (ping_data == NULL)
	{
		return -1;
	}
	/*
	 skip result until average data
	 tipical result of ping is as follows,
	 "rtt min/avg/max/mdev = 0.045/0.045/0.046/0.006 ms"
	 we can find the average data beyond the 4th '/'.
	 */
	sp = ping_data;
	for ( i = 0 ; i < 4 ; i ++)
	{
		sp = strchr(sp,'/');
		if (sp == NULL)
		{
			return -1;
		}
		sp ++;
	}
	ep = strchr (sp,'/');
	if (ep == NULL)
	{
		return -1;
	}
	*ep = '\0';
	errno = 0;
	/* convert to numeric data from text */
	msec = strtod(sp,(char **)NULL);
	if (errno != 0)
	{
		return -1;
	}
	return msec;
}
