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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

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

#define WD_INTERLOCK_WAIT_MSEC 500
#define WD_INTERLOCK_TIMEOUT_SEC 10
#define WD_INTERLOCK_WAIT_COUNT ((int) ((WD_INTERLOCK_TIMEOUT_SEC * 1000)/WD_INTERLOCK_WAIT_MSEC))

/* initialize interlock */
int
wd_init_interlock(void)
{
	int alloc_size;

	/* allocate WD_lock_holder */
	if (WD_Locks == NULL)
	{
		alloc_size = sizeof(bool) * WD_MAX_LOCK_NUM;

		WD_Locks = pool_shared_memory_create(alloc_size);
		memset((void *)WD_Locks, 0, alloc_size);
	}

	return WD_OK;
}

/* notify to start interlocking */
void wd_start_interlock(bool by_health_check, int node_id)
{
	int count;

	ereport(LOG,
		(errmsg("watchdog notifying to start interlocking")));

	/* confirm other pgpools are contactable */
	wd_confirm_contactable();

	/* lock all the resource */
	wd_lock_all();

	wd_set_interlocking(WD_MYSELF, true);
	wd_send_packet_no(WD_START_INTERLOCK);

	/* try to assume lock holder */
	wd_assume_lock_holder();

	/*
	 * If it is due to DB down detection by healthcheck, send failover request
	 * to other pgpools because detection of DB down on the others may be late.
	 */
	if (by_health_check && wd_am_I_lock_holder())
		wd_degenerate_backend_set(&node_id, 1);

	/* wait for all pgpools starting interlock */
	count = WD_INTERLOCK_WAIT_COUNT;
	while (wd_is_locked(WD_FAILOVER_START_LOCK))
	{
		if (WD_MYSELF->status == WD_DOWN)
		{
			wd_set_lock_holder(WD_MYSELF, false);
			return;
		}

		if (wd_am_I_lock_holder() && wd_are_interlocking_all())
		{
			wd_unlock(WD_FAILOVER_START_LOCK);
		}

		sleep_in_waiting();
		if (--count < 0)
		{
			ereport(WARNING,
					(errmsg("watchdog start interlocking, timed out")));
			break;
		}
	}
}

/* notify to end interlocking */
void wd_end_interlock(void)
{
	int count;

	ereport(LOG,
			(errmsg("watchdog notifying to end interlocking")));

	wd_set_interlocking(WD_MYSELF, false);
	wd_send_packet_no(WD_END_INTERLOCK);

	/* wait for all pgpools ending interlock */
	count = WD_INTERLOCK_WAIT_COUNT;
	while (wd_is_locked(WD_FAILOVER_END_LOCK))
	{
		if (WD_MYSELF->status == WD_DOWN)
		{
			wd_set_lock_holder(WD_MYSELF, false);
			break;
		}

		if (wd_am_I_lock_holder() && !wd_get_interlocking())
		{
			wd_unlock(WD_FAILOVER_END_LOCK);
		}

		sleep_in_waiting();
		if (--count < 0)
		{
			ereport(WARNING,
					(errmsg("watchdog end interlocking, timed out")));
			break;
		}
	}

	if (wd_am_I_lock_holder())
		wd_resign_lock_holder();

	wd_clear_interlocking_info();
}

/* leave from interlocking by the wayside */
void wd_leave_interlock(void)
{
	ereport(LOG,
		(errmsg("watchdog leaving from interlocking")));

	if (wd_am_I_lock_holder())
		wd_resign_lock_holder();

	wd_end_interlock();
}

/* if lock holder return true otherwise false */
bool wd_am_I_lock_holder(void)
{
	wd_update_lock_holder();
	return WD_MYSELF->is_lock_holder;
}

/* waiting for the lock or to be lock holder */
void wd_wait_for_lock(WD_LOCK_ID lock_id)
{
	int count;

	count = WD_INTERLOCK_WAIT_COUNT;
	while (wd_is_locked(lock_id))
	{
		if (WD_MYSELF->status == WD_DOWN)
		{
			wd_set_lock_holder(WD_MYSELF, false);
			return;
		}

		if (wd_am_I_lock_holder())
			break;

		sleep_in_waiting();
		if (--count < 0)
		{
			ereport(WARNING,
					(errmsg("watchdog waiting for lock, timed out")));
			break;
		}
	}
}

/*
 * assume lock holder
 * return WD_OK if success.
 */
static int
wd_assume_lock_holder(void)
{
	int rtn = WD_NG;

	wd_set_lock_holder(WD_MYSELF, false);

	if (WD_MYSELF->status == WD_DOWN)
		return WD_NG;

	/*
	 * confirm contatable master exists;
	 * If contactable master doesn't exists, WD_STAND_FOR_LOCK_HOLDER always
	 * returns WD_OK, eventually multiple pgpools can become lock_holder.
	 * Down of the host on which master pgpool was working is the case.
	 */
	while (!wd_is_contactable_master())
	{
		if (WD_MYSELF->status == WD_DOWN)
			return WD_NG;
	}

	/* I'm master and not lock holder, or I succeeded to become lock holder */
	if ((WD_MYSELF->status == WD_MASTER && wd_get_lock_holder() == NULL) ||
	    (wd_send_packet_no(WD_STAND_FOR_LOCK_HOLDER) == WD_OK))
	{
		if (wd_send_packet_no(WD_DECLARE_LOCK_HOLDER) == WD_OK)
		{
			wd_set_lock_holder(WD_MYSELF, true);
			ereport(LOG,
				(errmsg("watchdog became a new lock holder")));

			rtn = WD_OK;
		}
	}

	return rtn;
}

/*
 * update information of who's lock holder
 * Note that a lock holder pgpool may go down accidentally during interlocking.
 */
static void
wd_update_lock_holder(void)
{
	/* assume lock holder if not exist */
	while (wd_get_lock_holder() == NULL)
	{
		if (WD_MYSELF->status == WD_DOWN)
			return;

		wd_assume_lock_holder();
	}
}

/* resign lock holder */
static void
wd_resign_lock_holder(void)
{
	wd_set_lock_holder(WD_MYSELF, false);
	wd_send_packet_no(WD_RESIGN_LOCK_HOLDER);
}

/* returns true if lock_id is locked */
bool
wd_is_locked(WD_LOCK_ID lock_id)
{
	return WD_Locks[lock_id];
}

/* lock of unlock a resource */
void
wd_set_lock(WD_LOCK_ID lock_id, bool value)
{
	WD_Locks[lock_id] = value;
}

/* lock all the resource */
static void
wd_lock_all(void)
{
	int i;

	for (i = 0; i < WD_MAX_LOCK_NUM; i++)
		wd_set_lock(i, true);
}

/* unlock a resource */
int
wd_unlock(WD_LOCK_ID lock_id)
{
	int rtn;

	wd_set_lock(lock_id, false);
	rtn = wd_send_lock_packet(WD_UNLOCK_REQUEST, lock_id);
	ereport(DEBUG1,
		(errmsg("watchdog unlocking"),
			 errdetail("sent unlock request: %d", lock_id)));

	return rtn;
}

/* confirm other pgpools are contactable */
static void
wd_confirm_contactable(void)
{
	int count;

	count = WD_INTERLOCK_WAIT_COUNT;
	while (!wd_are_contactable_all())
	{
		if (WD_MYSELF->status == WD_DOWN)
			return;

		sleep_in_waiting();
		if (--count < 0)
		{
			ereport(WARNING,
					(errmsg("watchdog confirming contactable, timed out")));
			break;
		}
	}
}

static void
sleep_in_waiting(void)
{
	struct timeval t = {0, WD_INTERLOCK_WAIT_MSEC * 1000};
	select(0, NULL, NULL, NULL, &t);
}
