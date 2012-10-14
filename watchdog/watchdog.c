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
static pid_t child_pid;

pid_t wd_main(int fork_wait_time);
static void child_wait(int signo);
static void wd_exit(int exit_status);
static int wd_check_config(void);
static int has_sticky_bit(char * path);

static void
child_wait(int signo)
{
	pid_t pid = 0;

	do {
		int ret;
		pid = waitpid(-1,&ret,WNOHANG);
	} while(pid > 0);
}

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

	kill (child_pid, exit_signo);
	child_wait(0);

	exit(0);
}

static int
wd_check_config(void)
{
	int status = WD_OK;
	if ((pool_config->other_wd->num_wd == 0)	||
		(pool_config->delegate_IP == NULL)		||
		(strlen(pool_config->delegate_IP) == 0))
	{
		status = WD_NG;
	}
	return status;
	
}

pid_t
wd_main(int fork_wait_time)
{
	int status = WD_INIT;
	pid_t pgid = 0;
	pid_t pid = 0;

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
		return child_pid;
	}

	pgid = getpgid(0);
	pid = fork();
	if (pid != 0)
	{
		return pid;
	}

	if (fork_wait_time > 0) {
		sleep(fork_wait_time);
	}
	
	myargv = save_ps_display_args(myargc, myargv);

	init_ps_display("", "", "", "");

	signal(SIGCHLD, SIG_DFL);
	signal(SIGHUP, SIG_IGN);	
	signal(SIGINT, wd_exit);	
	signal(SIGQUIT, wd_exit);	
	signal(SIGTERM, wd_exit);	
	signal(SIGPIPE, SIG_IGN);	

	set_ps_display("lifecheck",false);
	/* wait until ready to go */
	while (WD_OK != is_wd_lifecheck_ready())
	{
		sleep(pool_config->wd_interval * 10);
	}

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
