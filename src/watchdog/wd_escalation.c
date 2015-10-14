/*
 *
 * wd_escalation
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2015	PgPool Global Development Group
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
#include <unistd.h>
#include <errno.h>

#include "pool.h"
#include "utils/elog.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "pool_config.h"
#include "watchdog/wd_utils.h"

#include "query_cache/pool_memqcache.h"
extern int *escalation_status;

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

	exit(0);
}

/*
 * fork escalation process
 */
pid_t
fork_escalation_process(void)
{
	pid_t pid;
	int has_vip = 0;

	pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			ereport(NOTICE,
					(errmsg("failed to fork a escalation process")));
		return pid;
	}
	on_exit_reset();
	processType = PT_WATCHDOG_UTILITY;

	POOL_SETMASK(&UnBlockSig);

	init_ps_display("", "", "", "");
	
	pool_signal(SIGTERM, wd_exit);
	pool_signal(SIGINT, wd_exit);
	pool_signal(SIGQUIT, wd_exit);
	pool_signal(SIGCHLD, SIG_DFL);
	pool_signal(SIGHUP, SIG_IGN);
	pool_signal(SIGPIPE, SIG_IGN);

	MemoryContextSwitchTo(TopMemoryContext);
	
	set_ps_display("escalation",false);

	ereport(LOG,
			(errmsg("watchdog: escalation started")));

	*escalation_status = 1;

	/*
	 * STEP 1
	 * clear shared memory cache
	 */
	if (pool_config->memory_cache_enabled && pool_is_shmem_cache() &&
		pool_config->clear_memqcache_on_escalation)
	{
		ereport(LOG,
			(errmsg("watchdog escalation"),
				 errdetail("clearing all the query cache on shared memory")));
		
		pool_clear_memory_cache();
	}

	/*
	 * STEP 2
	 * execute escalation command
	 */
	if (strlen(pool_config->wd_escalation_command))
	{
		int r = system(pool_config->wd_escalation_command);
		if (WIFEXITED(r))
		{
			if (WEXITSTATUS(r) == EXIT_SUCCESS)
				ereport(LOG,
						(errmsg("watchdog escalation successful")));
			else
			{
				ereport(WARNING,
						(errmsg("watchdog escalation command failed with exit status: %d", WEXITSTATUS(r))));
			}
		}
		else
		{
			ereport(WARNING,
					(errmsg("watchdog escalation command exit abnormally")));
		}
	}

	/*
	 * STEP 3
	 * interface up as delegate IP
	 */

	if (strlen(pool_config->delegate_IP) != 0)
	{
		has_vip = wd_IP_up();
	}

	if (has_vip == WD_OK)
		*escalation_status = 2;

	return pid;
}

/*
 * fork plunge process
 */
pid_t
fork_plunging_process(void)
{
	pid_t pid;
	int has_vip = 0;
	
	pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			ereport(NOTICE,
					(errmsg("failed to fork a escalation process")));
		return pid;
	}
	on_exit_reset();
	processType = PT_WATCHDOG_UTILITY;
	
	POOL_SETMASK(&UnBlockSig);
	
	init_ps_display("", "", "", "");
	
	pool_signal(SIGTERM, wd_exit);
	pool_signal(SIGINT, wd_exit);
	pool_signal(SIGQUIT, wd_exit);
	pool_signal(SIGCHLD, SIG_DFL);
	pool_signal(SIGHUP, SIG_IGN);
	pool_signal(SIGPIPE, SIG_IGN);
	
	MemoryContextSwitchTo(TopMemoryContext);
	
	set_ps_display("plunging",false);
	
	ereport(LOG,
			(errmsg("watchdog: plunging started")));
	
	*escalation_status = 1;
	
	/*
	 * STEP 2
	 * execute escalation command
	 */
	if (strlen(pool_config->wd_plunge_command))
	{
		int r = system(pool_config->wd_plunge_command);
		if (WIFEXITED(r))
		{
			if (WEXITSTATUS(r) == EXIT_SUCCESS)
				ereport(LOG,
						(errmsg("watchdog plunge successful")));
			else
			{
				ereport(WARNING,
						(errmsg("watchdog plunge command failed with exit status: %d", WEXITSTATUS(r))));
			}
		}
		else
		{
			ereport(WARNING,
					(errmsg("watchdog plunge command exit abnormally")));
		}
	}
	
	/*
	 * STEP 3
	 * interface up as delegate IP
	 */
	
	if (strlen(pool_config->delegate_IP) != 0)
	{
		has_vip = wd_IP_down();
	}
	
	if (has_vip == WD_OK)
		*escalation_status = 2;
	
	return pid;
}
