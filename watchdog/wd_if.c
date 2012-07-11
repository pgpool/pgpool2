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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include "pool.h"
#include "pool_config.h"
#include "watchdog.h"

int wd_IP_up(void);
int wd_IP_down(void);
static int exec_ifconfig(char * path,char * command);

int
wd_IP_up(void)
{
	int rtn = WD_OK;
	char path[128];
	if (WD_List->delegate_ip == 0)
	{
		WD_List->delegate_ip = 1;
		sprintf(path,"%s/ifconfig",pool_config->ifconfig_path);
		rtn = exec_ifconfig(path,pool_config->if_up_cmd);
		sprintf(path,"%s/arping",pool_config->arping_path);
		rtn = exec_ifconfig(path,pool_config->arping_cmd);
	}
	return rtn;
}
int
wd_IP_down(void)
{
	int rtn = WD_OK;
	char path[128];
	if (WD_List->delegate_ip == 1)
	{
		WD_List->delegate_ip = 0;
		sprintf(path,"%s/ifconfig",pool_config->ifconfig_path);
		rtn = exec_ifconfig(path,pool_config->if_down_cmd);
	}

	pool_log("wd_IP_down: ifconfig down %s", (rtn == WD_OK) ? "succeeded" : "failed");
	return rtn;
}

static int
exec_ifconfig(char * path,char * command)
{
	int pfd[2];
	int status;
	char * args[24];
	int pid, i = 0;
	char buf[256];
	char *bp, *ep;

	if (pipe(pfd) == -1)
	{
		pool_error("exec_ifconfig: pipe open error:%s",strerror(errno));
		return WD_NG;
	}
	memset(buf,0,sizeof(buf));
	strncpy(buf,command,sizeof(buf));
	bp = buf;
	while (*bp == ' ')
	{
		bp ++;
	}
	while (*bp != '\0')
	{
		ep = strchr(bp,' ');
		if (ep != NULL)
		{
			*ep = '\0';
		}
		if (!strncmp(bp,"$_IP_$",5))
		{
			args[i++] = pool_config->delegate_IP;
		}
		else
		{
			args[i++] = bp;
		}
		if (ep != NULL)
		{
			bp = ep +1;
			while (*bp == ' ')
			{
				bp ++;
			}
		}
		else
		{
			break;
		}
	}
	args[i++] = NULL;

	pid = fork();
	if (pid == 0)
	{
		close(STDOUT_FILENO);
		dup2(pfd[1], STDOUT_FILENO);
		close(pfd[0]);
		status = execv(path,args);
		exit(0);
	}
	else
	{
		close(pfd[1]);
		for (;;)
		{
			int result;
			result = wait(&status);
			if (result < 0)
			{
				if (errno == EINTR)
					continue;
				return WD_NG;
			}

			if (WIFEXITED(status) == 0 || WEXITSTATUS(status) != 0)
				return WD_NG;
			else
				break;
		}
		close(pfd[0]);
	}
	return WD_OK;
}


