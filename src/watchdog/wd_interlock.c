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

/*
 interlocking mechnism
 
 1-) Only master can assign locks
 2-) pgpool-II Will ask watchdog to aquire lock
	-- watchdog will return one of following
		-- CLUSTER IN TRANSATIONING
		-- LOCK AQUIRED
		-- LOCK FAILED
 
 3-) On receiving lock request, standby pgpool-II will forward
	 the lock request to master node. And wait for the reply
	 and return the result to caller based on reply
 
 4-) on receiving lock request, master pgpool-II will replicate
	 check if lock holder exists, it will decline the request
     and accept it otherwise.
	 -- Once the new lock is granted this information will be
		replicated to all connected nodes.
 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "pool.h"
#include "utils/elog.h"
#include "pool_config.h"
#include "watchdog/watchdog.h"
#include "watchdog/wd_ext.h"

static volatile bool * WD_Locks;

static void wd_update_lock_holder(void);
static int wd_assume_lock_holder(void);
static void wd_resign_lock_holder(void);
static void wd_lock_all(void);
static void wd_confirm_contactable(void);

static void sleep_in_waiting(void);
static WDFailoverCMDResults wd_issue_failover_lock_command(WDFailoverCMDTypes cmdType, char* syncReqType);

#define WD_INTERLOCK_WAIT_MSEC 500
#define WD_INTERLOCK_TIMEOUT_SEC 10
#define WD_INTERLOCK_WAIT_COUNT ((int) ((WD_INTERLOCK_TIMEOUT_SEC * 1000)/WD_INTERLOCK_WAIT_MSEC))


void wd_wati_until_lock_or_timeout(WDFailoverCMDTypes cmdType)
{
	WDFailoverCMDResults res = FAILOVER_RES_TRANSITION;
	int	count = WD_INTERLOCK_WAIT_COUNT;
	while (1)
	{
		res = wd_failover_command_check_lock(cmdType);
		if (res == FAILOVER_RES_PROCEED_LOCK_HOLDER ||
			res == FAILOVER_RES_PROCEED_UNLOCKED)
		{
			/* we have the permision */
			return;
		}
		sleep_in_waiting();
		if (--count < 0)
		{
			ereport(WARNING,
					(errmsg("timeout wating for unlock")));
			break;
		}
	}
}

WDFailoverCMDResults wd_failover_command_start(WDFailoverCMDTypes cmdType)
{
	if (pool_config->use_watchdog)
		return wd_issue_failover_lock_command(cmdType,"START_COMMAND");
	return FAILOVER_RES_PROCEED_LOCK_HOLDER;
}

WDFailoverCMDResults wd_failover_command_end(WDFailoverCMDTypes cmdType)
{
	if (pool_config->use_watchdog)
		return wd_issue_failover_lock_command(cmdType,"END_COMMAND");
	return FAILOVER_RES_PROCEED_LOCK_HOLDER;
}

WDFailoverCMDResults wd_failover_command_check_lock(WDFailoverCMDTypes cmdType)
{
	if (pool_config->use_watchdog)
		return wd_issue_failover_lock_command(cmdType,"CHECK_LOCKED");
	return FAILOVER_RES_PROCEED_LOCK_HOLDER;
}

WDFailoverCMDResults wd_release_failover_command_lock(WDFailoverCMDTypes cmdType)
{
	if (pool_config->use_watchdog)
		return wd_issue_failover_lock_command(cmdType,"UNLOCK_COMMAND");
	return FAILOVER_RES_PROCEED_LOCK_HOLDER;
}

/*
 * This is just a wrapper over wd_send_failover_sync_command()
 * but tries to wait for WD_INTERLOCK_TIMEOUT_SEC amount of time
 * if watchdog is in transition state
 */

static WDFailoverCMDResults wd_issue_failover_lock_command(WDFailoverCMDTypes cmdType, char* syncReqType)
{
	WDFailoverCMDResults res;
	int x;
	for (x=0; x < MAX_SEC_WAIT_FOR_CLUSTER_TRANSATION; x++)
	{
		res = wd_send_failover_sync_command(NODE_FAILBACK_CMD, syncReqType);
		if (res != FAILOVER_RES_TRANSITION)
			break;
		sleep(1);
	}
	return res;
}

static void
sleep_in_waiting(void)
{
	struct timeval t = {0, WD_INTERLOCK_WAIT_MSEC * 1000};
	select(0, NULL, NULL, NULL, &t);
}
