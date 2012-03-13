/*
 * $Header$
 *
 * Handles watchdog connection, and protocol communication with pgpool-II
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2011	PgPool Global Development Group
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
#include <wait.h>
#include <ctype.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include "pool.h"
#include "pool_config.h"
#include "watchdog.h"
#include "wd_ext.h"

WdInfo * WD_List = NULL;		/* watchdog server list */

int wd_main(int fork_wait_time);
static void child_wait(int signo);
static void wd_exit(int exit_status);

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

	kill (0, exit_signo);

	child_wait(0);

	exit(0);
}

int
wd_main(int fork_wait_time)
{
	int status = WD_INIT;
	pid_t pgid = 0;
	pid_t pid = 0;

	/* initialize */
	status = wd_init();
	if (status < 0)
	{
		return WD_NG;
	}

	/* launch child process */
	status = wd_child(1);
	if (status != WD_OK)
	{
		return WD_NG;
	}

	pgid = getpgid(0);
	pid = fork();
	if (pid != 0)
	{
		return WD_OK;
	}

	signal(SIGCHLD, SIG_DFL);
	signal(SIGHUP, SIG_IGN);	
	signal(SIGINT, wd_exit);	
	signal(SIGQUIT, wd_exit);	
	signal(SIGTERM, wd_exit);	
	signal(SIGPIPE, SIG_IGN);	
	setpgid(0,pgid);

	if (fork_wait_time > 0) {
		sleep(fork_wait_time);
	}

	/* watchdog loop */
	for (;;)
	{
		/* pgpool life check */
		wd_lifecheck();
		sleep(pool_config->wd_interval);
	}
}
