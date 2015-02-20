/*
 * $Header$
 *
 * Handles watchdog connection, and protocol communication with pgpool-II
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
#include "utils/elog.h"
#include "pool_config.h"
#include "watchdog/watchdog.h"
#include "watchdog/wd_ext.h"

static int exec_ifconfig(char * path,char * command);
static char *string_replace(const char *string, const char *pattern, const char *replacement);

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
				ereport(DEBUG1,
					(errmsg("watchdog bringing up delegate IP"),
						 errdetail("waiting... count: %d", i+1)));
			}

			if (i >= WD_TRY_PING_AT_IPUP)
				rtn = WD_NG;
		}

		if (rtn == WD_OK)
			ereport(LOG,
				(errmsg("watchdog bringing up delegate IP, 'ifconfig up' succeeded")));
		else
			ereport(WARNING,
				(errmsg("watchdog failed to bring up delegate IP, 'ifconfig up' failed")));
	}
	else
	{
		ereport(DEBUG1,
			(errmsg("watchdog failed to bring up delegate IP"),
				 errdetail("already delegate IP holder")));
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
		{
			ereport(LOG,
				(errmsg("watchdog bringing down delegate IP"),
					 errdetail("ifconfig down succeeded")));
		}
		else
		{
			WD_List->delegate_ip_flag = 1;
			ereport(WARNING,
				(errmsg("watchdog bringing down delegate IP, ifconfig down failed")));
		}
	}
	else
	{
		ereport(DEBUG1,
			(errmsg("watchdog failed to bring down delegate IP"),
				 errdetail("not a delegate IP holder")));
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
	char* buf;
	char *bp, *ep;

	if (pipe(pfd) == -1)
	{
		ereport(WARNING,
				(errmsg("while executing ifconfig, pipe open failed with error \"%s\"",strerror(errno))));
		return WD_NG;
	}

	buf = string_replace(command,"$_IP_$",pool_config->delegate_IP);

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
		args[i++] = bp;
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
	if (pid == -1)
	{
		ereport(FATAL,
			(errmsg("failed to execute interface up/down command"),
				 errdetail("fork() failed with reason: \"%s\"", strerror(errno))));
	}
	if (pid == 0)
	{
		on_exit_reset();
		processType = PT_WATCHDOG_UTILITY;
		close(STDOUT_FILENO);
		dup2(pfd[1], STDOUT_FILENO);
		close(pfd[0]);
		status = execv(path,args);
		exit(0);
	}
	else
	{
		pfree(buf);
		close(pfd[1]);
		for (;;)
		{
			int result;
			result = waitpid(pid, &status, 0);
			if (result < 0)
			{
				if (errno == EINTR)
					continue;

				ereport(DEBUG1,
					(errmsg("watchdog exec waitpid()failed"),
						 errdetail("waitpid() system call failed with reason \"%s\"", strerror(errno))));

				return WD_NG;
			}

			if (WIFEXITED(status) == 0 || WEXITSTATUS(status) != 0)
			{
				ereport(DEBUG1,
					(errmsg("watchdog exec ifconfig failed"),
						errdetail("'%s' failed. exit status: %d",command, WEXITSTATUS(status))));

				return WD_NG;
			}
			else
				break;
		}
		close(pfd[0]);
	}
	ereport(DEBUG1,
		(errmsg("watchdog exec ifconfig: '%s' succeeded", command)));

	return WD_OK;
}

/*
 * string_replace:
 * returns the new palloced string after replacing all
 * occurances of pattern in string with replacement string
 */
static char *
string_replace(const char *string, const char *pattern, const char *replacement)
{
	char *tok = NULL;
	char *newstr = NULL;
	char *oldstr = NULL;
	char *head = NULL;
	size_t pat_len,rep_len;

	newstr = pstrdup(string);
	/* bail out if no pattern or replacement is given */
	if ( pattern == NULL || replacement == NULL )
		return newstr;

	pat_len = strlen(pattern);
	rep_len = strlen(replacement);

	head = newstr;
	while ( (tok = strstr(head,pattern)))
	{
		oldstr = newstr;
		newstr = palloc ( strlen ( oldstr ) - pat_len + rep_len + 1 );

		memcpy(newstr, oldstr, tok - oldstr );
		memcpy(newstr + (tok - oldstr), replacement, rep_len );
		memcpy(newstr + (tok - oldstr) + rep_len, tok + pat_len, strlen(oldstr) - pat_len - (tok - oldstr));
		/* put the string terminator */
		memset( newstr + strlen (oldstr) - pat_len + rep_len , 0, 1 );
		/* move back head right after the last replacement */
		head = newstr + (tok - oldstr) + rep_len;
		pfree(oldstr);
	}
	return newstr;
}


