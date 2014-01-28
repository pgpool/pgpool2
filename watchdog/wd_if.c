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
#include <ctype.h>
#include <errno.h>
#include "pool.h"
#include "pool_config.h"
#include "watchdog.h"
#include "wd_ext.h"

int wd_IP_up(void);
int wd_IP_down(void);
int wd_get_cmd(char * buf, char * cmd);
static int exec_ifconfig(char * path,char * command);

#define WD_TRY_PING_AT_IPUP 3
int
wd_IP_up(void)
{
	int rtn = WD_OK;
	char path[WD_MAX_PATH_LEN];
	char cmd[128];
	int i;

	if (strlen(pool_config->delegate_IP) == 0)
		return WD_NG;

	if (WD_List->delegate_ip_flag == 0)
	{
		WD_List->delegate_ip_flag = 1;

		wd_get_cmd(cmd,pool_config->if_up_cmd);
		snprintf(path,sizeof(path),"%s/%s",pool_config->ifconfig_path,cmd);
		rtn = exec_ifconfig(path,pool_config->if_up_cmd);

		if (rtn == WD_OK)
		{
			wd_get_cmd(cmd,pool_config->arping_cmd);
			snprintf(path,sizeof(path),"%s/%s",pool_config->arping_path,cmd);
			rtn = exec_ifconfig(path,pool_config->arping_cmd);
		}
		if (rtn == WD_OK)
		{
			for (i = 0; i < WD_TRY_PING_AT_IPUP; i++)
			{
				if (!wd_is_unused_ip(pool_config->delegate_IP))
					break;
			}

			if (i >= WD_TRY_PING_AT_IPUP)
				rtn = WD_NG;
		}

		if (rtn == WD_OK)
			pool_log("wd_IP_up: ifconfig up succeeded");
		else
		{
			WD_List->delegate_ip_flag = 0;
			pool_error("wd_IP_up: ifconfig up failed");
		}
	}
	else
	{
		pool_debug("wd_IP_up: already delegate IP holder");
	}

	return rtn;
}

#define WD_TRY_PING_AT_IPDOWN 3
int
wd_IP_down(void)
{
	int rtn = WD_OK;
	char path[WD_MAX_PATH_LEN];
	char cmd[128];
	int i;

	if (strlen(pool_config->delegate_IP) == 0)
		return WD_NG;

	if (WD_List->delegate_ip_flag == 1)
	{
		WD_List->delegate_ip_flag = 0;
		wd_get_cmd(cmd,pool_config->if_down_cmd);
		snprintf(path, sizeof(path), "%s/%s", pool_config->ifconfig_path, cmd);
		rtn = exec_ifconfig(path,pool_config->if_down_cmd);

		if (rtn == WD_OK)
		{
			for (i = 0; i < WD_TRY_PING_AT_IPDOWN; i++)
			{
				if (wd_is_unused_ip(pool_config->delegate_IP))
					break;
			}

			if (i >= WD_TRY_PING_AT_IPDOWN)
				rtn = WD_NG;
		}

		if (rtn == WD_OK)
			pool_log("wd_IP_down: ifconfig down succeeded");
		else
		{
			WD_List->delegate_ip_flag = 1;
			pool_error("wd_IP_down: ifconfig down failed");
		}
	}
	else
	{
		pool_debug("wd_IP_down: not delegate IP holder");
	}

	return rtn;
}

int
wd_get_cmd(char * buf, char * cmd)
{
	int i,j;
	i = 0;
	while(isspace(cmd[i]) != 0)
	{
		i++;
	}
	j = 0;
	while(isspace(cmd[i]) == 0)
	{
		buf[j++] = cmd[i++];
	}
	buf[j] = '\0';
	return strlen(buf);
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
	strlcpy(buf,command,sizeof(buf));
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


