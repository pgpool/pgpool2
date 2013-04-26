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
#include "pool_config.h"
#include "watchdog.h"
#include "wd_ext.h"

static volatile bool * WD_Locks;

int wd_init_interlock(void);
void wd_start_interlock(void);
void wd_end_interlock(void);
void wd_leave_interlock(void);
void wd_wait_for_lock(WD_LOCK_ID lock_id);
bool wd_am_I_lock_holder(void);
bool wd_is_locked(WD_LOCK_ID lock_id);
void wd_set_lock(WD_LOCK_ID lock_id, bool value);
int wd_unlock(WD_LOCK_ID lock_id);

static void wd_update_lock_holder(void);
static int wd_assume_lock_holder(void);
static void wd_resign_lock_holder(void);
static void wd_lock_all(void);

/* initialize interlock */
int
wd_init_interlock(void)
{
	/* allocate WD_lock_holder */
	if (WD_Locks == NULL)
	{
		WD_Locks = pool_shared_memory_create(sizeof(WD_Locks));
		if (WD_Locks == NULL)
		{
			pool_error("wd_init_interlock: failed to allocate WD_Locks");
			return WD_NG;
		}
		memset((void *)WD_Locks, 0, sizeof(WD_Locks) * WD_MAX_LOCK_NUM);
	}

	return WD_OK;
}

/* notify to start interlocking */
void wd_start_interlock(void)
{
	pool_log("wd_end_interlock: start interlocking");

	wd_set_interlocking(WD_MYSELF, true);
	wd_send_packet_no(WD_START_INTERLOCK);

	/* try to assume lock holder */
	wd_assume_lock_holder();

	/* lock all the resource */
	wd_lock_all();
}

/* notify to end intelocking */
void wd_end_interlock(void)
{
	pool_log("wd_end_interlock: end interlocking");

	wd_set_interlocking(WD_MYSELF, false);
	wd_send_packet_no(WD_END_INTERLOCK);

	/* wait for all pgpools ending interlock */
	while (wd_is_locked(WD_FAILOVER_END_LOCK))
	{
		struct timeval t = {1, 0};
		select(0, NULL, NULL, NULL, &t);

		if (wd_am_I_lock_holder() && !wd_get_interlocking())
		{
			wd_unlock(WD_FAILOVER_END_LOCK);
		}
	}

	wd_clear_interlocking_info();
}

/* leave from interlocking by the wayside */
void wd_leave_interlock(void)
{
	pool_log("wd_leave_interlock: leaving from interlocking");

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
	while (wd_is_locked(lock_id))
	{
		struct timeval t = {1, 0};
		select(0, NULL, NULL, NULL, &t);

		if (wd_am_I_lock_holder())
			break;
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

	/* (master and lock holder) or (succeeded to become lock holder) */
	if ((WD_MYSELF->status == WD_MASTER && wd_get_lock_holder() == NULL) ||
	    (wd_send_packet_no(WD_STAND_FOR_LOCK_HOLDER) == WD_OK))
	{
		wd_set_lock_holder(WD_MYSELF, true);
		wd_send_packet_no(WD_DECLARE_LOCK_HOLDER);
		rtn = WD_OK;
	}

	return rtn;
}

/*
 * update information of who's loock holder
 * Note that the lock holder pgpool can go down accidentally.
 */
static void
wd_update_lock_holder(void)
{
	/* assume lock holder if not exist */
	if (wd_get_lock_holder() == NULL)
		wd_assume_lock_holder();
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
	pool_debug("wd_unlock: send unlock request: %d", lock_id);

	return rtn;
}
