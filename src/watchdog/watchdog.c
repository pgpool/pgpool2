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
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include "pool.h"
#include "utils/elog.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "pool_config.h"
#include "watchdog/watchdog.h"
#include "watchdog/wd_ext.h"

WdInfo * WD_List = NULL;					/* watchdog server list */
unsigned char * WD_Node_List = NULL;		/* node list */

pid_t wd_ppid = 0;
static pid_t lifecheck_pid;
static pid_t child_pid;
static pid_t hb_receiver_pid[WD_MAX_IF_NUM];
static pid_t hb_sender_pid[WD_MAX_IF_NUM];

static void wd_check_config(void);
static int has_setuid_bit(char * path);
static void *exec_func(void *arg);

/* send signal specified by sig to watchdog processes */
void
wd_kill_watchdog(int sig)
{
	int i;
    if(lifecheck_pid > 0)
        kill (lifecheck_pid, sig);
    lifecheck_pid = 0;
    
    if(child_pid > 0)
        kill (child_pid, sig);
    child_pid = 0;

	if (!strcmp(pool_config->wd_lifecheck_method, MODE_HEARTBEAT))
	{
		for (i = 0; i < pool_config->num_hb_if; i++)
		{
            if(hb_receiver_pid[i] > 0)
                kill(hb_receiver_pid[i], sig);
            hb_receiver_pid[i] = 0;
            
            if(hb_sender_pid[i] > 0)
                kill (hb_sender_pid[i], sig);
		}
	}
}

static void
wd_check_config(void)
{
	int wd_authkey_len = strlen(pool_config->wd_authkey);

	if (pool_config->other_wd->num_wd == 0)
		ereport(ERROR,
			(errmsg("invalid watchdog configuration. other pgpools setting is not defined")));

	if (wd_authkey_len > MAX_PASSWORD_SIZE)
		ereport(ERROR,
				(errmsg("invalid watchdog configuration. wd_authkey length can't be larger than %d",
						MAX_PASSWORD_SIZE)));
#ifndef USE_SSL
	if (wd_authkey_len > 0)
		ereport(LOG,
			(errmsg("watchdog is configured to use authentication, but pgpool-II is built without SSL support"),
				errdetail("The authentication method used by pgpool-II without the SSL support is known to be weak")));
#endif
}

pid_t
wd_main(int fork_wait_time)
{
	int i;

	if (!pool_config->use_watchdog)
		return 0;

	/* check pool_config data */
	wd_check_config();

	/* initialize */
	wd_init();

	wd_ppid = getpid();

	/* launch child process */
	child_pid = wd_child(1);

	if (!strcmp(pool_config->wd_lifecheck_method, MODE_HEARTBEAT))
	{
		for (i = 0; i < pool_config->num_hb_if; i++)
		{
			/* heartbeat receiver process */
			hb_receiver_pid[i] = wd_hb_receiver(1, &(pool_config->hb_if[i]));

			/* heartbeat sender process */
			hb_sender_pid[i] = wd_hb_sender(1, &(pool_config->hb_if[i]));
		}
	}

	/* fork lifecheck process*/
	lifecheck_pid = fork_a_lifecheck(fork_wait_time);

	return lifecheck_pid;
}


/* if pid is for one of watchdog processes return 1, othewize return 0 */
int
wd_is_watchdog_pid(pid_t pid)
{
	int i;

	if (pid == lifecheck_pid || pid == child_pid)
	{
		return 1;
	}

	for (i = 0; i < pool_config->num_hb_if; i++)
	{
		if (pid == hb_receiver_pid[i] || pid == hb_sender_pid[i])
		{
			return 1;
		}
	}

	return 0;
}

char* wd_process_name_from_pid(pid_t pid)
{
	int i;
	if (pid == lifecheck_pid)
		return "watchdog lifecheck";
	if(pid == child_pid)
		return "watchdog child";
	for (i = 0; i < pool_config->num_hb_if; i++)
	{
		if (pid == hb_receiver_pid[i])
			return "watchdog heartbeat receiver";

		if (pid == hb_receiver_pid[i] || pid == hb_sender_pid[i])
			return "watchdog heartbeat sender";
	}
	return "unknown watchdog process"; /* should never happen */
}

/*
 * restart watchdog process specified by pid if restart_child is true
 * return the new pid of the forked process or 0 if restart_child was false.
 */
pid_t
wd_reaper_watchdog(pid_t pid, bool restart_child)
{
	int i;

	/* watchdog lifecheck process exits */
	if (pid == lifecheck_pid)
	{
		if(restart_child)
			lifecheck_pid = fork_a_lifecheck(1);
		else
			lifecheck_pid = 0;

		return lifecheck_pid;
	}

	/* watchdog child process exits */
	else if (pid == child_pid)
	{
		if(restart_child)
			child_pid = wd_child(1);
		else
			child_pid = 0;

		return child_pid;
	}

	/* receiver/sender process exits */
	else
	{
		for (i = 0; i < pool_config->num_hb_if; i++)
		{
			if (pid == hb_receiver_pid[i])
			{
				if(restart_child)
					hb_receiver_pid[i] = wd_hb_receiver(1, &(pool_config->hb_if[i]));
				else
					hb_receiver_pid[i] = 0;

				return hb_receiver_pid[i];
			}

			else if (pid == hb_sender_pid[i])
			{
				if(restart_child)
					hb_sender_pid[i] = wd_hb_sender(1, &(pool_config->hb_if[i]));
				else
					hb_sender_pid[i] = 0;

				return hb_sender_pid[i];
			}
		}
	}
	return -1;
}

void wd_check_network_command_configurations(void)
{
	char path[128];
	char cmd[128];

	if (pool_config->use_watchdog == 0)
		return;
	/*
	 * If delegate IP is not assigned to the node
	 * the configuration is not used
	 */
	if (strlen(pool_config->delegate_IP) == 0)
		return;

	/* check setuid bit of ifup command */
	wd_get_cmd(cmd, pool_config->if_up_cmd);
	snprintf(path, sizeof(path), "%s/%s", pool_config->ifconfig_path, cmd);
	if (! has_setuid_bit(path))
	{
		ereport(WARNING,
				(errmsg("checking setuid bit of if_up_cmd"),
				 errdetail("ifup[%s] doesn't have setuid bit", path)));
	}
	/* check setuid bit of ifdown command */
	wd_get_cmd(cmd, pool_config->if_down_cmd);
	snprintf(path, sizeof(path), "%s/%s", pool_config->ifconfig_path, cmd);
	if (! has_setuid_bit(path))
	{
		ereport(WARNING,
				(errmsg("checking setuid bit of if_down_cmd"),
				 errdetail("ifdown[%s] doesn't have setuid bit", path)));
	}

	/* check setuid bit of arping command */
	wd_get_cmd(cmd, pool_config->arping_cmd);
	snprintf(path, sizeof(path), "%s/%s", pool_config->arping_path, cmd);
	if (! has_setuid_bit(path))
	{
		ereport(WARNING,
				(errmsg("checking setuid bit of arping command"),
				 errdetail("arping[%s] doesn't have setuid bit", path)));

	}
}

/* 
 * if the file has setuid bit and the owner is root, it returns 1, otherwise returns 0 
 */
static int
has_setuid_bit(char * path)
{
	struct stat buf;
	if (stat(path,&buf) < 0)
	{
		ereport(FATAL,
			(return_code(1),
			 errmsg("has_setuid_bit: command '%s' not found", path)));
	}
	return ((buf.st_uid == 0) && (S_ISREG(buf.st_mode)) && (buf.st_mode & S_ISUID))?1:0;
}


/*
 * The function is wrapper over pthread_create.
 */
int watchdog_thread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
	WdThreadInfo* thread_arg = palloc(sizeof(WdThreadInfo));
	thread_arg->arg = arg;
	thread_arg->start_routine = start_routine;
	return pthread_create(thread, attr, exec_func, thread_arg);
}

static void *
exec_func(void *arg)
{
	WdThreadInfo* thread_arg = (WdThreadInfo*) arg;
	Assert(thread_arg != NULL);
	return thread_arg->start_routine(thread_arg->arg);
}
