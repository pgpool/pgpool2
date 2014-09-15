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
#include "utils/elog.h"
#include "pool_config.h"
#include "watchdog/watchdog.h"
#include "watchdog/wd_ext.h"

void
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
		memset(WD_List, 0, sizeof(WdInfo) * MAX_WATCHDOG_NUM);
	}

	/* allocate node list */
	if (WD_Node_List == NULL)
	{
		WD_Node_List = pool_shared_memory_create(sizeof(unsigned char) * MAX_NUM_BACKENDS);
		memset(WD_Node_List, 0, sizeof(unsigned char) * MAX_NUM_BACKENDS);
	}

	/* initialize interlock */
	wd_init_interlock();

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
		ereport(ERROR,
			(errmsg("failed to initialize watchdog, failed to connect to trusted server")));
	}
	/* send startup packet */
	if (wd_startup() == WD_NG)
	{
		ereport(ERROR,
				(errmsg("failed to initialize watchdog, startup failed")));
	}
	/* check existence of master pgpool */
	if (wd_is_exist_master() == NULL)
	{
		if (strlen(pool_config->delegate_IP) != 0 &&
		    !wd_is_unused_ip(pool_config->delegate_IP))
		{
			ereport(ERROR,
				(errmsg("failed to initialize watchdog, delegate_IP \"%s\" already exists", pool_config->delegate_IP)));
		}
		/* escalate to delegate_IP holder */
		wd_escalation();
	}
	ereport(LOG,
		(errmsg("watchdog started")));
}
