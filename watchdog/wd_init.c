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
#include <sys/time.h>
#include <unistd.h>

#include "pool.h"
#include "pool_config.h"
#include "watchdog.h"
#include "wd_ext.h"

int
wd_init(void)
{
	struct timeval tv;
	WdInfo * p;

	/* set startup time */
	gettimeofday(&tv, NULL);

	/* allocate watchdog list */
	if (WD_List == NULL)
	{
		WD_List = pool_shared_memory_create(sizeof(WdInfo) * MAX_WATCHDOG_NUM);
		if (WD_List == NULL)
		{
			pool_error("wd_init: failed to allocate watchdog list");
			return WD_NG;
		}
		memset(WD_List, 0, sizeof(WdInfo) * MAX_WATCHDOG_NUM);
	}

	/* allocate node list */
	if (WD_Node_List == NULL)
	{
		WD_Node_List = pool_shared_memory_create(sizeof(unsigned char) * MAX_NUM_BACKENDS);
		if (WD_Node_List == NULL)
		{
			pool_error("wd_init: failed to allocate node list");
			return WD_NG;
		}
		memset(WD_Node_List, 0, sizeof(unsigned char) * MAX_NUM_BACKENDS);
	}

	/* initialize interlock */
	if (wd_init_interlock() != WD_OK)
	{
		pool_error("wd_init: wd_init_interlock failed");
		return WD_NG;
	}

	/* set myself to watchdog list */
	wd_set_wd_list(pool_config->wd_hostname, pool_config->port,
	               pool_config->wd_port, pool_config->delegate_IP,
				   &tv, WD_NORMAL);

	/* set other pgpools to watchdog list */
	wd_add_wd_list(pool_config->other_wd);

	/* reset time value */
	p = WD_List;
	while (p->status != WD_END)
	{
		WD_TIME_INIT(p->hb_send_time);
		WD_TIME_INIT(p->hb_last_recv_time);

		p++;
	}

	/* check upper connection */
	if (strlen(pool_config->trusted_servers) &&
		wd_is_upper_ok(pool_config->trusted_servers) != WD_OK)
	{
		pool_error("wd_init: failed to connect trusted server");
		return WD_NG;
	}

	/* send startup packet */
	if (wd_startup() == WD_NG)
	{
		pool_error("wd_init: failed to start watchdog");
		return WD_NG;
	}

	/* check existence of master pgpool */
	if (wd_is_exist_master() == NULL)
	{
		if (strlen(pool_config->delegate_IP) != 0 &&
		    !wd_is_unused_ip(pool_config->delegate_IP))
		{
			pool_error("wd_init: delegate_IP %s already exists", pool_config->delegate_IP);
			return WD_NG;
		}

		/* escalate to delegate_IP holder */
		wd_escalation();
	}

	pool_log("wd_init: start watchdog");
	return WD_OK;	
}
