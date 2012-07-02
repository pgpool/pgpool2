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
#include "pool_config.h"
#include "watchdog.h"
#include "wd_ext.h"

int wd_set_wd_list(char * hostname, int pgpool_port, int wd_port, struct timeval * tv, int status);
int wd_add_wd_list(WdDesc * other_wd);
int wd_set_wd_info(WdInfo * info);
WdInfo * wd_is_exist_master(void);
int wd_am_I_oldest(void);
int wd_set_myself(struct timeval * tv, int status);
WdInfo * wd_is_alive_master(void);

int
wd_set_wd_list(char * hostname, int pgpool_port, int wd_port, struct timeval * tv, int status)
{
	int i = 0;
	WdInfo * p = NULL;

	if ((WD_List == NULL) || (hostname == NULL))
	{
		pool_error("memory allocate error");
		return -1;
	}
	for ( i = 0 ; i < MAX_WATCHDOG_NUM ; i ++)
	{
		p = (WD_List+i);	
		if( p->status != WD_END)
		{
			if ((!strncmp(p->hostname, hostname, sizeof(p->hostname))) &&
				(p->pgpool_port == pgpool_port)	&&
				(p->wd_port == wd_port))
			{
				if (tv != NULL)
				{
					memcpy(&(p->tv),tv,sizeof(struct timeval));
				}
				p->status = status;
				return i;
			}
		}
		else
		{
			strncpy(p->hostname, hostname, sizeof(p->hostname));
			p->pgpool_port = pgpool_port;
			p->wd_port = wd_port;
			if (tv != NULL)
			{
				memcpy(&(p->tv),tv,sizeof(struct timeval));
			}
			p->status = status;
			return i;
		}
	}
	pool_error("Can not add new watchdog information cause the WD_List is full.");
	return -1;
}

int
wd_add_wd_list(WdDesc * other_wd)
{
	WdInfo * p = NULL;
	int i = 0;

	if (other_wd == NULL)
	{
		pool_error("memory allocate error");
		return -1;
	}
	for ( i = 0 ; i < other_wd->num_wd ; i ++)
	{
		p = &(other_wd->wd_info[i]);
		wd_set_wd_info(p);
	}
	return i;
}

int
wd_set_wd_info(WdInfo * info)
{
	int rtn;
	rtn = wd_set_wd_list(info->hostname, info->pgpool_port, info->wd_port, &(info->tv), info->status);
	return rtn;
}

WdInfo *
wd_is_exist_master(void)
{
	WdInfo * p = WD_List;

	p++;
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
			if ((p->tv.tv_sec < WD_List->tv.tv_sec) ||
				((p->tv.tv_sec == WD_List->tv.tv_sec) &&
				(p->tv.tv_usec < WD_List->tv.tv_usec)))
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
	if (WD_List == NULL)
	{
		return WD_NG;
	}
	if (tv != NULL)
	{
		memcpy(&(WD_List->tv),tv,sizeof(struct timeval));
	}
	WD_List->status = status;
	return WD_OK;
}

WdInfo *
wd_is_alive_master(void)
{
	WdInfo * master = NULL;

	master = wd_is_exist_master();
	if (master != NULL)
	{
		if (wd_ping_pgpool(master) != WD_OK)
		{
			master = NULL;
		}
	}
	return master;
}

