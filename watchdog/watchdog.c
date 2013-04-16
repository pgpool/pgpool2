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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include "pool.h"
#include "pool_config.h"
#include "watchdog.h"
#include "wd_ext.h"

WdInfo * WD_List = NULL;					/* watchdog server list */
unsigned char * WD_Node_List = NULL;		/* node list */

pid_t wd_ppid = 0;
static pid_t lifecheck_pid;
static pid_t child_pid;
static pid_t reader_pid[WD_MAX_IF_NUM];
static pid_t writer_pid[WD_MAX_IF_NUM];

pid_t wd_main(int fork_wait_time);
void wd_kill_watchdog(int sig);
int wd_reaper_watchdog(pid_t pid, int status);
static pid_t fork_a_lifecheck(int fork_wait_time);
static void wd_exit(int exit_status);
static int wd_check_config(void);
static int has_sticky_bit(char * path);

static void
wd_exit(int exit_signo)
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

void
wd_kill_watchdog(int sig)
{
	int i;

	kill (lifecheck_pid, sig);
	kill (child_pid, sig);

	if (!strcmp(pool_config->watchdog_mode, "udp"))
	{
		for (i = 0; i < pool_config->other_wd->num_udp_if; i++)
		{
			kill (reader_pid[i], sig);
			kill (writer_pid[i], sig);
		}
	}
}

static int
wd_check_config(void)
{
	int status = WD_OK;

	if (pool_config->other_wd->num_wd == 0)
	{
		pool_error("wd_check_config: there is no other pgpools setting.");
		status = WD_NG;
	}
	if (strlen(pool_config->delegate_IP) == 0)
	{
		pool_error("wd_check_config: delegate_IP is empty");
		status = WD_NG;
	}
	if (strlen(pool_config->wd_udp_authkey) > MAX_PASSWORD_SIZE)
	{
		pool_error("wd_check_config: wd_udp_authkey length can't be larger than %d",
		           MAX_PASSWORD_SIZE);
		status = WD_NG;
	}

	return status;
}

pid_t
wd_main(int fork_wait_time)
{
	int status = WD_INIT;
	int i;

	if (!pool_config->use_watchdog)
	{
		return 0;
	}

	/* check pool_config data */
	status = wd_check_config();
	if (status != WD_OK)
	{
		pool_error("watchdog: wd_check_config failed");
		return 0;
	}

	/* initialize */
	status = wd_init();
	if (status != WD_OK)
	{
		pool_error("watchdog: wd_init failed");
		return 0;
	}

	wd_ppid = getpid();

	/* launch child process */
	child_pid = wd_child(1);
	if (child_pid < 0 )
	{
		pool_error("launch wd_child failed");
		return 0;
	}

	if (!strcmp(pool_config->watchdog_mode, "udp"))
	{
		for (i = 0; i < pool_config->other_wd->num_udp_if; i++)
		{
			/* reader process */
			reader_pid[i] = wd_reader(1, pool_config->other_wd->udp_if[i]);
			if (reader_pid[i] < 0 )
			{
				pool_error("launch wd_reader failed");
				return reader_pid[i];
			}

			/* writer process */
			writer_pid[i] = wd_writer(1, pool_config->other_wd->udp_if[i]);
			if (writer_pid[i] < 0 )
			{
				pool_error("launch wd_writer failed");
				return writer_pid[i];
			}
		}
	}

	/* fork lifecheck process*/
	lifecheck_pid = fork_a_lifecheck(fork_wait_time);
	if (lifecheck_pid < 0 )
	{
		pool_error("launch lifecheck process failed");
		return 0;
	}

	return lifecheck_pid;
}


/* fork lifecheck process*/
static pid_t
fork_a_lifecheck(int fork_wait_time)
{
	pid_t pid;

	pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			pool_error("fork_a_lifecheck: fork() failed.");

		return pid;
	}

	if (fork_wait_time > 0) {
		sleep(fork_wait_time);
	}

	myargv = save_ps_display_args(myargc, myargv);

	POOL_SETMASK(&UnBlockSig);

	init_ps_display("", "", "", "");

	signal(SIGTERM, wd_exit);	
	signal(SIGINT, wd_exit);	
	signal(SIGQUIT, wd_exit);	
	signal(SIGCHLD, SIG_DFL);
	signal(SIGHUP, SIG_IGN);	
	signal(SIGPIPE, SIG_IGN);	

	set_ps_display("lifecheck",false);

	/* wait until ready to go */
	while (WD_OK != is_wd_lifecheck_ready())
	{
		sleep(pool_config->wd_interval * 10);
	}

	pool_log("watchdog: lifecheck started");

	/* watchdog loop */
	for (;;)
	{
		/* pgpool life check */
		wd_lifecheck();
		sleep(pool_config->wd_interval);
	}

	return pid;
}

int
wd_is_watchdog_pid(pid_t pid)
{
	int i;

	if (pid == lifecheck_pid || pid == child_pid)
	{
		return 1;
	}

	for (i = 0; i < pool_config->other_wd->num_udp_if; i++)
	{
		if (pid == reader_pid[i] || pid == writer_pid[i])
		{
			return 1;
		}
	}

	return 0;
}

int
wd_reaper_watchdog(pid_t pid, int status)
{
	int i;

	/* exiting process was watchdog lifecheck process */
	if (pid == lifecheck_pid)
	{
		if (WIFSIGNALED(status))
			pool_debug("watchdog lifecheck process %d exits with status %d by signal %d",
			           pid, status, WTERMSIG(status));
		else
			pool_debug("watchdog lifecheck process %d exits with status %d", pid, status);

		lifecheck_pid = fork_a_lifecheck(1);

		if (lifecheck_pid < 0)
		{
			pool_error("wd_reaper: fork a watchdog lifecheck process failed");
			return 0;
		}

		pool_log("fork a new watchdog lifecheck pid %d", lifecheck_pid);
	}

	/* exiting process was watchdog child process */
	else if (pid == child_pid)
	{
		if (WIFSIGNALED(status))
			pool_debug("watchdog child process %d exits with status %d by signal %d",
			           pid, status, WTERMSIG(status));
		else
			pool_debug("watchdog child process %d exits with status %d", pid, status);

		child_pid = wd_child(1);

		if (child_pid < 0)
		{
			pool_error("wd_reaper: fork a watchdog child process failed");
			return 0;
		}

		pool_log("fork a new watchdog child pid %d", child_pid);
	}

	/* exiting process was reader/writer process */
	else
	{
		for (i = 0; i < pool_config->other_wd->num_udp_if; i++)
		{
			if (pid == reader_pid[i])
			{
				if (WIFSIGNALED(status))
					pool_debug("watchdog reader process %d exits with status %d by signal %d",
					           pid, status, WTERMSIG(status));
				else
					pool_debug("watchdog reader process %d exits with status %d", pid, status);

				reader_pid[i] = wd_reader(1, pool_config->other_wd->udp_if[i]);

				if (reader_pid[i] < 0)
				{
					pool_error("wd_reaper: fork a watchdog reader process failed");
					return 0;
				}
		
				pool_log("fork a new watchdog reader pid %d", reader_pid[i]);
				break;
			}

			else if (pid == writer_pid[i])
			{
				if (WIFSIGNALED(status))
					pool_debug("watchdog writer process %d exits with status %d by signal %d",
					           pid, status, WTERMSIG(status));
				else
					pool_debug("watchdog writer process %d exits with status %d", pid, status);

				writer_pid[i] = wd_writer(1, pool_config->other_wd->udp_if[i]);

				if (writer_pid[i] < 0)
				{
					pool_error("wd_reaper: fork a watchdog writer process failed");
					return 0;
				}
		
				pool_log("fork a new watchdog writer pid %d", writer_pid[i]);
				break;
			}
		}
	}
	
	return 1;
}

int
wd_chk_sticky(void)
{
	char path[128];
	char cmd[128];
	
	/* check sticky bit of ifup command */
	wd_get_cmd(cmd, pool_config->if_up_cmd);
	snprintf(path, sizeof(path), "%s/%s", pool_config->ifconfig_path, cmd);
	if (! has_sticky_bit(path))
	{
		pool_log("wd_chk_sticky: ifup[%s] doesn't have sticky bit", path);
		return 0;
	}

	/* check sticky bit of ifdown command */
	wd_get_cmd(cmd, pool_config->if_down_cmd);
	snprintf(path, sizeof(path), "%s/%s", pool_config->ifconfig_path, cmd);
	if (! has_sticky_bit(path))
	{
		pool_log("wd_chk_sticky: ifdown[%s] doesn't have sticky bit", path);
		return 0;
	}

	/* check sticky bit of arping command */
	wd_get_cmd(cmd, pool_config->arping_cmd);
	snprintf(path, sizeof(path), "%s/%s", pool_config->arping_path, cmd);
	if (! has_sticky_bit(path))
	{
		pool_log("wd_chk_sticky: arping[%s] doesn't have sticky bit", path);
		return 0;
	}

	pool_log("wd_chk_sticy: all commands have sticky bit");
	return 1;
}

/* if the file has sticky bit and the owner is root, it returns 1, otherwise returns 0 */
static int
has_sticky_bit(char * path)
{
	struct stat buf;
	if (stat(path,&buf) < 0)
	{
		pool_error("has_stickey_bit: %s: no such a command", path);
		pool_shmem_exit(1);
		exit(1);
	}
	return ((buf.st_uid == 0) && (S_ISREG(buf.st_mode)) && (buf.st_mode & S_ISUID))?1:0;
}
