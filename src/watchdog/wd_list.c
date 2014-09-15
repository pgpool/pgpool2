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

#include "pool.h"
#include "utils/elog.h"
#include "pool_config.h"
#include "watchdog/watchdog.h"
#include "watchdog/wd_ext.h"

static void wd_reset_wd_info(WdInfo *info);

/* add or modify watchdog information list */
int
wd_set_wd_list(char * hostname, int pgpool_port, int wd_port, char * delegate_ip, struct timeval * tv, int status)
{
	int i = 0;
	WdInfo * p = NULL;

	if ((WD_List == NULL) || (hostname == NULL))
		ereport(ERROR,
			(errmsg("adding watchdog information list. memory allocation error")));

	if (strcmp(pool_config->delegate_IP, delegate_ip))
		ereport(ERROR,
			(errmsg("adding watchdog information list. delegate IP mismatch error"),
				 errdetail("delegate IP defined in config \"%s\" does not match with \"%s\"",pool_config->delegate_IP, delegate_ip)));

	for ( i = 0 ; i < MAX_WATCHDOG_NUM ; i ++)
	{
		p = (WD_List+i);

		if( p->status != WD_END)
		{
			/* found; modify the pgpool. */
			if ((!strncmp(p->hostname, hostname, sizeof(p->hostname))) &&
				(p->pgpool_port == pgpool_port)	&&
				(p->wd_port == wd_port))
			{
				if (p->status == WD_DOWN || p->status == WD_INIT)
				{
					wd_reset_wd_info(p);
				}

				p->status = status;

				if (tv != NULL)
				{
					memcpy(&(p->tv), tv, sizeof(struct timeval));
				}

				return i;
			}
		}

		/* not found; add as a new pgpool */
		else
		{
			wd_reset_wd_info(p);

			p->status = status;
			p->pgpool_port = pgpool_port;
			p->wd_port = wd_port;

			strlcpy(p->hostname, hostname, sizeof(p->hostname));
			strlcpy(p->delegate_ip, delegate_ip, sizeof(p->delegate_ip));

			if (tv != NULL)
			{
				memcpy(&(p->tv), tv, sizeof(struct timeval));
			}

			return i;
		}
	}
	ereport(WARNING,
			(errmsg("failed adding watchdog information list. list is full")));
	return -1;
}

/** reset watchdog info*/
static void
wd_reset_wd_info(WdInfo * info)
{
	WD_TIME_INIT(info->hb_send_time);
	WD_TIME_INIT(info->hb_last_recv_time);

	info->in_interlocking = false;
	info->is_lock_holder = false;
}

/* add watchdog information to list using config description */
int
wd_add_wd_list(WdDesc * other_wd)
{
	WdInfo * p = NULL;
	int i = 0;

	if (other_wd == NULL)
	{
		ereport(ERROR,
				(errmsg("adding watchdog information list. memory allocation error")));
	}
	for ( i = 0 ; i < other_wd->num_wd ; i ++)
	{
		p = &(other_wd->wd_info[i]);
		strlcpy(p->delegate_ip, pool_config->delegate_IP, sizeof(p->delegate_ip));
		wd_set_wd_info(p);
	}

	return i;
}

/* set watchdog information to list */
int
wd_set_wd_info(WdInfo * info)
{
	int rtn;
	rtn = wd_set_wd_list(info->hostname, info->pgpool_port, info->wd_port,
	                     info->delegate_ip, &(info->tv), info->status);
	return rtn;
}

/* return master if exist, NULL if not found */
WdInfo *
wd_is_exist_master(void)
{
	WdInfo * p = WD_List;

	while (p->status != WD_END)
	{
		/* find master pgpool in the other servers */
		if (p->status == WD_MASTER)
		{
			/* master found */
			return p;
		}
		p++;
	}
	/* not found */
	return NULL;
}

/* set or unset in_interlocking flag */
void
wd_set_interlocking(WdInfo *info, bool value)
{
	WdInfo * p = WD_List;

	while (p->status != WD_END)
	{
		if ((!strncmp(p->hostname, info->hostname, sizeof(p->hostname))) &&
			(p->pgpool_port == info->pgpool_port) &&
			(p->wd_port == info->wd_port))
		{
			p->in_interlocking = value;

			return;
		}
		p++;
	}
}

/* set or unset lock holder flag */
void
wd_set_lock_holder(WdInfo *info, bool value)
{
	WdInfo * p = WD_List;

	while (p->status != WD_END)
	{
		if ((!strncmp(p->hostname, info->hostname, sizeof(p->hostname))) &&
			(p->pgpool_port == info->pgpool_port) &&
			(p->wd_port == info->wd_port))
		{
			p->is_lock_holder = value;

			return;
		}
		p++;
	}
}

/* return the lock holder if exist, NULL if not found */
WdInfo *
wd_get_lock_holder(void)
{
	WdInfo * p = WD_List;

	while (p->status != WD_END)
	{
		/* find failover lock holder */
		if ((p->status == WD_NORMAL || p->status == WD_MASTER) &&
			p->is_lock_holder)
		{
			/* found */
			return p;
		}
		p++;
	}

	/* not found */
	return NULL;
}

/* return the pgpool in interlocking found in first, NULL if not found */
WdInfo *
wd_get_interlocking(void)
{
	WdInfo * p = WD_List;

	/* for updating contactable flag */
	wd_update_info();

	while (p->status != WD_END)
	{
		/* skip if not contactable */
		if (!p->is_contactable)
		{
			p++;
			continue;
		}

		/* find pgpool in interlocking */
		if ((p->status == WD_NORMAL || p->status == WD_MASTER) &&
		    p->in_interlocking)
		{
			/* found */
			return p;
		}
		p++;
	}

	/* not found */
	return NULL;
}

/* if all pgpool are in interlocking return true, otherwise false */
bool
wd_are_interlocking_all(void)
{
	WdInfo * p = WD_List;
	bool rtn = true;

	/* for updating contactable flag */
	wd_update_info();

	while (p->status != WD_END)
	{
		/* skip if not contactable */
		if (!p->is_contactable)
		{
			p++;
			continue;
		}

		/* find pgpool not in interlocking */
		if ((p->status == WD_NORMAL || p->status == WD_MASTER) &&
		    !p->in_interlocking)
		{
			rtn = false;
		}
		p++;
	}

	return rtn;
}

/* clear flags for interlocking */
void
wd_clear_interlocking_info(void)
{
	WdInfo * p = WD_List;

	while (p->status != WD_END)
	{
		wd_set_lock_holder(p, false);
		wd_set_interlocking(p, false);
		p++;
	}
}

int
wd_am_I_oldest(void)
{
	WdInfo * p = WD_List;

	p++;
	while (p->status != WD_END)
	{
		if ((p->status == WD_NORMAL) ||
			(p->status == WD_MASTER))
		{
			if (WD_TIME_BEFORE(p->tv, WD_MYSELF->tv))
			{
				return WD_NG;
			}
		}
		p++;
	}
	return WD_OK;
}

int
wd_set_myself(struct timeval * tv, int status)
{
	if (WD_MYSELF == NULL)
	{
		return WD_NG;
	}

	if (tv != NULL)
	{
		memcpy(&(WD_MYSELF->tv),tv,sizeof(struct timeval));
	}

	WD_MYSELF->status = status;

	return WD_OK;
}

/*
 * if master exists and it is alive actually return the master,
 * otherwise return NULL
 */
WdInfo *
wd_is_alive_master(void)
{
	WdInfo * master = NULL;

	if (WD_MYSELF->status == WD_MASTER)
		return WD_MYSELF;

	master = wd_is_exist_master();
	if (master != NULL)
	{
		if ((!strcmp(pool_config->wd_lifecheck_method, MODE_HEARTBEAT)) ||
		    (!strcmp(pool_config->wd_lifecheck_method, MODE_QUERY)
			     && wd_ping_pgpool(master) == WD_OK))
		{
			return master;
		}
	}

	ereport(DEBUG1,
		(errmsg("watchdog checking master"),
			 errdetail("alive master not found")));

	return NULL;
}

/*
 * if master exists and it is contactable return true,
 * otherwise return false
 */
bool
wd_is_contactable_master(void)
{
	WdInfo * master = NULL;

	if (WD_MYSELF->status == WD_MASTER)
		return true;

	/* for updating contactable flag */
	wd_update_info();

	master = wd_is_exist_master();
	if (master != NULL)
	{
		return master->is_contactable;
	}

	return false;
}

bool
wd_are_contactable_all(void)
{
	WdInfo * p = WD_List;

	/* for updating contactable flag */
	wd_update_info();

	while (p->status != WD_END)
	{
		if ((p->status == WD_NORMAL || p->status == WD_MASTER) &&
		    !p->is_contactable)
		{
			return false;
		}
		p++;
	}

	return true;
}

/*
 * get watchdog information of specified index
 */
WdInfo *
wd_get_watchdog_info(int wd_index)
{
	if (wd_index < 0 || wd_index > pool_config->other_wd->num_wd)
		return NULL;

	return &WD_List[wd_index];
}
