/* -*-pgsql-c-*- */
/*
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2026	PgPool Global Development Group
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
 * child.c: worker child process main
 *
 */
#include "config.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include <signal.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#include "pool.h"
#include "pool_config.h"

#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/elog.h"
#include "utils/pool_ip.h"
#include "utils/ps_status.h"
#include "utils/pool_stream.h"

#include "context/pool_process_context.h"
#include "context/pool_session_context.h"
#include "protocol/pool_process_query.h"
#include "protocol/pool_pg_utils.h"
#include "main/pool_internal_comms.h"
#include "auth/md5.h"
#include "auth/pool_hba.h"

#include "watchdog/wd_internal_commands.h"
#include "watchdog/watchdog.h"

static POOL_CONNECTION_POOL_SLOT *slots[MAX_NUM_BACKENDS];
static volatile sig_atomic_t reload_config_request = 0;
static volatile sig_atomic_t restart_request = 0;

static void establish_persistent_connection(void);
static void discard_persistent_connection(void);
static void check_replication_time_lag(void);
static void check_replication_time_lag_with_cmd(void);
static char *build_instance_identifier_for_node(int node_id);
static void CheckReplicationTimeLagErrorCb(void *arg);
static unsigned long long int text_to_lsn(char *text);
static RETSIGTYPE my_signal_handler(int sig);
static RETSIGTYPE reload_config_handler(int sig);
static void reload_config(void);
static void sr_check_will_die(int code, Datum arg);

#define CHECK_REQUEST \
	do { \
		if (reload_config_request) \
		{ \
			reload_config(); \
			reload_config_request = 0; \
		} else if (restart_request) \
		{ \
		  ereport(LOG,(errmsg("worker process received restart request"))); \
		  exit(1); \
		} \
    } while (0)


#define PG10_SERVER_VERSION	100000	/* PostgreSQL 10 server version num */
#define PG91_SERVER_VERSION	90100	/* PostgreSQL 9.1 server version num */

static volatile bool follow_primary_lock_acquired;

/*
* worker child main loop
*/
void
do_worker_child(void *params)
{
	sigjmp_buf	local_sigjmp_buf;
	MemoryContext WorkerMemoryContext;

	ereport(DEBUG1,
			(errmsg("I am %d", getpid())));

	/* Identify myself via ps */
	init_ps_display("", "", "", "");
	set_ps_display("worker process", false);

	/*
	 * install the call back for preparation of exit
	 */
	on_system_exit(sr_check_will_die, (Datum) NULL);

	/* set up signal handlers */
	signal(SIGALRM, SIG_DFL);
	signal(SIGTERM, my_signal_handler);
	signal(SIGINT, my_signal_handler);
	signal(SIGHUP, reload_config_handler);
	signal(SIGQUIT, my_signal_handler);
	signal(SIGCHLD, my_signal_handler);
	signal(SIGUSR1, my_signal_handler);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

	/* Create per loop iteration memory context */
	WorkerMemoryContext = AllocSetContextCreate(TopMemoryContext,
												"Worker_main_loop",
												ALLOCSET_DEFAULT_MINSIZE,
												ALLOCSET_DEFAULT_INITSIZE,
												ALLOCSET_DEFAULT_MAXSIZE);

	MemoryContextSwitchTo(TopMemoryContext);

	/* Initialize per process context */
	pool_init_process_context();

	/*
	 * Open pool_passwd.
	 */
	if (strcmp("", pool_config->pool_passwd))
	{
		pool_reopen_passwd_file();
	}

	if (sigsetjmp(local_sigjmp_buf, 1) != 0)
	{
		pool_signal(SIGALRM, SIG_IGN);
		error_context_stack = NULL;
		EmitErrorReport();
		MemoryContextSwitchTo(TopMemoryContext);
		FlushErrorState();
	}
	/* We can now handle ereport(ERROR) */
	PG_exception_stack = &local_sigjmp_buf;


	for (;;)
	{
		MemoryContextSwitchTo(WorkerMemoryContext);
		MemoryContextResetAndDeleteChildren(WorkerMemoryContext);

		/*
		 * Since WorkerMemoryContext is used for "slots", we need to clear it
		 * so that new slots are allocated later on.
		 */
		memset(slots, 0, sizeof(slots));

		bool		watchdog_leader;	/* true if I am the watchdog leader */


		CHECK_REQUEST;

		if (pool_config->sr_check_period <= 0)
		{
			sleep(30);
		}

		/*
		 * Get watchdog status if watchdog is enabled.
		 */
		watchdog_leader = false;
		if (pool_config->use_watchdog)
		{
			WD_STATES	wd_status;
			WDPGBackendStatus *backendStatus;

			wd_status = wd_internal_get_watchdog_local_node_state();
			ereport(DEBUG1,
					(errmsg("watchdog status: %d", wd_status)));

			/*
			 * Ask the watchdog to get all the backend states from the
			 * Leader/Coordinator Pgpool-II node.
			 */
			watchdog_leader = false;
			backendStatus = get_pg_backend_status_from_leader_wd_node();

			if (!backendStatus)

				/*
				 * Couldn't get leader status.
				 */
				watchdog_leader = false;
			else
			{
				int			quorum = wd_internal_get_watchdog_quorum_state();
				int			node_count = backendStatus->node_count;

				ereport(DEBUG1,
						(errmsg("quorum: %d node_count: %d",
								quorum, node_count)));
				if (quorum >= 0 && backendStatus->node_count <= 0)
				{
					/*
					 * Quorum exists and node_count <= 0. Definitely I am the
					 * leader.
					 */
					watchdog_leader = true;
				}
				else
					watchdog_leader = false;

				pfree(backendStatus);
			}
		}

		/*
		 * If streaming replication mode, do time lag checking Also skip if
		 * failover/failback is ongoing.
		 */
		if (pool_config->sr_check_period > 0 && STREAM &&
			Req_info->switching == false)
		{
			/*
			 * Acquire follow primary lock. If fail to acquire lock, try
			 * again.
			 */
			follow_primary_lock_acquired = false;

			if (pool_acquire_follow_primary_lock(false, false) == true)
			{
				follow_primary_lock_acquired = true;

				establish_persistent_connection();
				PG_TRY();
				{
					POOL_NODE_STATUS *node_status;
					int			i;

					/* Do replication time lag checking */

					/*
					 * Use external command if replication_delay_source_cmd is
					 * configured
					 */
					if (pool_config->replication_delay_source_cmd &&
						strlen(pool_config->replication_delay_source_cmd) > 0)
						check_replication_time_lag_with_cmd();
					else
						check_replication_time_lag();

					/* Check node status */
					node_status = verify_backend_node_status(slots);


					for (i = 0; i < NUM_BACKENDS; i++)
					{
						ereport(DEBUG1,
								(errmsg("node status[%d]: %d", i, node_status[i])));

						if (node_status[i] == POOL_NODE_STATUS_INVALID)
						{
							int			n;

							ereport(LOG,
									(errmsg("pgpool_worker_child: invalid node found %d", i)));

							/*
							 * If detach_false_primary is enabled, send
							 * degenerate request to detach invalid node.
							 */
							if (pool_config->detach_false_primary)
							{
								/*
								 * However if watchdog is enabled and I am not
								 * the leader, do not detach the invalid node
								 * because the information to determine the
								 * false primary might be outdated or
								 * temporarily inconsistent.  See
								 * [pgpool-hackers: 4431] for more details.
								 */
								if (!pool_config->use_watchdog ||
									(pool_config->use_watchdog && watchdog_leader))
								{
									n = i;

									/*
									 * In the case watchdog enabled, we need
									 * to add REQ_DETAIL_CONFIRMED, which
									 * means no quorum consensus is required.
									 * If we do not add this, the target node
									 * will remain quarantine state since
									 * other node does not request failover.
									 */
									degenerate_backend_set(&n, 1,
														   REQ_DETAIL_SWITCHOVER | REQ_DETAIL_CONFIRMED);
								}
								else if (pool_config->use_watchdog)
									ereport(LOG,
											(errmsg("do not detach invalid node %d because I am not the leader or quorum does not exist", i)));
							}
						}
					}
				}
				PG_CATCH();
				{
					discard_persistent_connection();
					pool_release_follow_primary_lock(false);
					follow_primary_lock_acquired = false;
					sleep(pool_config->sr_check_period);
					PG_RE_THROW();
				}
				PG_END_TRY();

				/* Discard persistent connections */
				discard_persistent_connection();
				if (follow_primary_lock_acquired)
				{
					pool_release_follow_primary_lock(false);
					follow_primary_lock_acquired = false;
				}
			}
		}
		sleep(pool_config->sr_check_period);
	}
	exit(0);
}

/*
 * Establish persistent connection to backend
 */
static void
establish_persistent_connection(void)
{
	int			i;
	BackendInfo *bkinfo;

	char	   *password = get_pgpool_config_user_password(pool_config->sr_check_user,
														   pool_config->sr_check_password);

	for (i = 0; i < NUM_BACKENDS; i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		if (slots[i] == NULL)
		{
			bkinfo = pool_get_node_info(i);
			slots[i] = make_persistent_db_connection_noerror(i, bkinfo->backend_hostname,
															 bkinfo->backend_port,
															 pool_config->sr_check_database,
															 pool_config->sr_check_user,
															 password ? password : "", false);
		}
	}

	if (password)
		pfree(password);
}

/*
 * Discard persistent connection to backend
 */
static void
discard_persistent_connection(void)
{
	int			i;

	for (i = 0; i < NUM_BACKENDS; i++)
	{
		if (slots[i])
		{
			discard_persistent_db_connection(slots[i]);
			slots[i] = NULL;
		}
	}
}

/*
 * Check replication time lag
 */
static void
check_replication_time_lag(void)
{
	/* backend server version cache */
	static int	server_version[MAX_NUM_BACKENDS];

	int			i;
	POOL_SELECT_RESULT *res;
	POOL_SELECT_RESULT *res_rep;	/* query results of pg_stat_replication */
	uint64		lsn[MAX_NUM_BACKENDS];
	char	   *query;
	char	   *stat_rep_query;
	BackendInfo *bkinfo;
	uint64		lag;
	uint64		delay_threshold_by_time;
	ErrorContextCallback callback;
	int			active_standby_node;
	bool		replication_delay_by_time;

	/* clear replication state */
	for (i = 0; i < NUM_BACKENDS; i++)
	{
		bkinfo = pool_get_node_info(i);

		*bkinfo->replication_state = '\0';
		*bkinfo->replication_sync_state = '\0';
	}

	if (NUM_BACKENDS <= 1)
	{
		/* If there's only one node, there's no point to do checking */
		return;
	}

	if (REAL_PRIMARY_NODE_ID < 0)
	{
		/* No need to check if there's no primary */
		return;
	}

	if (!VALID_BACKEND(REAL_PRIMARY_NODE_ID))
	{
		/*
		 * No need to check replication delay if primary is down. This could
		 * happen if ALWAYS_PRIMARY flag is on because REAL_PRIMARY_NODE_ID
		 * macro returns the node id which ALWAYS_PRIMARY flag is set to.  If
		 * we do not check this, subsequent test (i == PRIMARY_NODE_ID) in the
		 * for loop below will return unexpected result because
		 * PRIMARY_NODE_ID macro returns MAIN_NODE_ID, which could be a
		 * standby server.
		 */
		return;
	}

	/*
	 * Register a error context callback to throw proper context message
	 */
	callback.callback = CheckReplicationTimeLagErrorCb;
	callback.arg = NULL;
	callback.previous = error_context_stack;
	error_context_stack = &callback;
	stat_rep_query = NULL;
	active_standby_node = 0;
	replication_delay_by_time = false;

	for (i = 0; i < NUM_BACKENDS; i++)
	{
		lsn[i] = 0;

		if (!VALID_BACKEND(i))
			continue;

		if (!slots[i])
		{
			ereport(ERROR,
					(errmsg("Failed to check replication time lag"),
					 errdetail("No persistent db connection for the node %d", i),
					 errhint("check sr_check_user and sr_check_password")));

		}

		if (server_version[i] == 0)
		{
			query = "SELECT pg_catalog.current_setting('server_version_num')";

			/*
			 * Get backend server version. If the query fails, keep previous
			 * info.
			 */
			if (get_query_result(slots, i, query, &res) == 0)
			{
				server_version[i] = atoi(res->data[0]);
				ereport(DEBUG1,
						(errmsg("backend %d server version: %d", i, server_version[i])));
			}

		}

		if (PRIMARY_NODE_ID == i)
		{
			if (server_version[i] >= PG10_SERVER_VERSION)
				query = "SELECT pg_catalog.pg_current_wal_lsn()";
			else
				query = "SELECT pg_catalog.pg_current_xlog_location()";

			if (server_version[i] == PG91_SERVER_VERSION)
				stat_rep_query = "SELECT application_name, state, '' AS sync_state, '' AS replay_lag FROM pg_catalog.pg_stat_replication";
			else if (server_version[i] >= PG10_SERVER_VERSION)
			{
				stat_rep_query = "SELECT application_name, state, sync_state,(EXTRACT(EPOCH FROM replay_lag)*1000000)::BIGINT FROM pg_catalog.pg_stat_replication";
				if (pool_config->delay_threshold_by_time > 0)
					replication_delay_by_time = true;
			}
			else if (server_version[i] > PG91_SERVER_VERSION)
				stat_rep_query = "SELECT application_name, state, sync_state, '' AS replay_lag FROM pg_catalog.pg_stat_replication";
		}
		else
		{
			if (server_version[i] >= PG10_SERVER_VERSION)
				query = "SELECT pg_catalog.pg_last_wal_replay_lsn()";
			else
				query = "SELECT pg_catalog.pg_last_xlog_replay_location()";

			active_standby_node++;
		}

		if (get_query_result(slots, i, query, &res) == 0 && res->nullflags[0] != -1)
		{
			lsn[i] = text_to_lsn(res->data[0]);
			free_select_result(res);
		}
	}

	/*
	 * Call pg_stat_replication and fill the replication status.
	 */
	if (slots[PRIMARY_NODE_ID] && stat_rep_query != NULL)
	{
		int			status;

		status = get_query_result(slots, PRIMARY_NODE_ID, stat_rep_query, &res_rep);

		if (status == -1 || (status == -2 && active_standby_node > 0))
		{
			ereport(LOG,
					(errmsg("get_query_result failed: status: %d", status)));
		}

		for (i = 0; i < NUM_BACKENDS; i++)
		{
			bkinfo = pool_get_node_info(i);

			*bkinfo->replication_state = '\0';
			*bkinfo->replication_sync_state = '\0';

			if (i == PRIMARY_NODE_ID)
				continue;

			if (status == 0)
			{
				int			j;
				char	   *s;
#define	NUM_COLS 4

				for (j = 0; j < res_rep->numrows; j++)
				{
					if (strcmp(res_rep->data[j * NUM_COLS], bkinfo->backend_application_name) == 0)
					{
						/*
						 * If sr_check_user has enough privilege, it should
						 * return some string. If not, NULL pointer will be
						 * returned for res_rep->data[1] and [2]. So we need
						 * to prepare for the latter case.
						 */
						s = res_rep->data[j * NUM_COLS + 1] ? res_rep->data[j * NUM_COLS + 1] : "";
						strlcpy(bkinfo->replication_state, s, NAMEDATALEN);

						s = res_rep->data[j * NUM_COLS + 2] ? res_rep->data[j * NUM_COLS + 2] : "";
						strlcpy(bkinfo->replication_sync_state, s, NAMEDATALEN);

						s = res_rep->data[j * NUM_COLS + 3];
						if (s)
						{
							bkinfo->standby_delay = atol(s);
							ereport(DEBUG1,
									(errmsg("standby delay in milli seconds * 1000: " UINT64_FORMAT "", bkinfo->standby_delay)));
						}
						else
							bkinfo->standby_delay = 0;
					}
				}
			}
		}
		if (status == 0)
			free_select_result(res_rep);
	}

	for (i = 0; i < NUM_BACKENDS; i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		/* Set standby delay value */
		bkinfo = pool_get_node_info(i);
		lag = (lsn[PRIMARY_NODE_ID] > lsn[i]) ? lsn[PRIMARY_NODE_ID] - lsn[i] : 0;

		if (PRIMARY_NODE_ID == i)
		{
			bkinfo->standby_delay = 0;
		}
		else
		{
			if (replication_delay_by_time)
			{
				/*
				 * If replication delay is measured by time, indicate it in
				 * shared memory area.
				 */
				bkinfo->standby_delay_by_time = true;
			}
			else
			{
				/*
				 * If replication delay is not measured by time, set the LSN
				 * lag to shared memory area.
				 */
				bkinfo->standby_delay = lag;
				bkinfo->standby_delay_by_time = false;
			}

			/* Log delay if necessary */
			if (replication_delay_by_time)
			{
				lag = bkinfo->standby_delay;
				delay_threshold_by_time = pool_config->delay_threshold_by_time;
				delay_threshold_by_time *= 1000;	/* convert from milli
													 * seconds to micro
													 * seconds */

				/* Log delay if necessary */
				if ((pool_config->log_standby_delay == LSD_ALWAYS && lag > 0) ||
					(pool_config->log_standby_delay == LSD_OVER_THRESHOLD &&
					 lag > delay_threshold_by_time))
				{
					ereport(LOG,
							(errmsg("Replication of node: %d is behind %.6f second(s) from the primary server (node: %d)",
									i, ((float) lag) / 1000000, PRIMARY_NODE_ID)));
				}
			}
			else
			{
				if ((pool_config->log_standby_delay == LSD_ALWAYS && lag > 0) ||
					(pool_config->delay_threshold &&
					 pool_config->log_standby_delay == LSD_OVER_THRESHOLD &&
					 lag > pool_config->delay_threshold))
				{
					ereport(LOG,
							(errmsg("Replication of node: %d is behind " UINT64_FORMAT " bytes from the primary server (node: %d)",
									i, (uint64) (lsn[PRIMARY_NODE_ID] - lsn[i]), PRIMARY_NODE_ID)));
				}
			}
		}
	}

	error_context_stack = callback.previous;
}

#define MAX_CMD_OUTPUT 4096
#define MAX_REASONABLE_DELAY_MS 3600000.0	/* 1 hour in milliseconds */

/*
 * Check replication time lag using external command
 *
 * The external command receives only replica (standby) node identifiers as arguments,
 * omitting the primary node. It returns delay values in milliseconds for each replica.
 * A value of -1 indicates a node that is down but not yet detected by pgpool's health checks.
 */
static void
check_replication_time_lag_with_cmd(void)
{
	char	   *command = NULL;
	char	   *line;
	char	   *token;
	char	   *saveptr;
	char	   *line_copy;
	char	   *temp_token;
	char	   *endptr;
	char	   *ident;
	const char *base_command;
	double		delay_ms;
	uint64		delay;
	uint64		delay_threshold_by_time;
	int			token_count = 0;
	int			primary_node_id;
	int			save_errno;
	int			i;
	size_t		total_len;
	size_t		current_len;
	BackendInfo *bkinfo;
	ErrorContextCallback callback;
	int			pipefd[2] = {-1, -1};
	pid_t		pid = -1;
	int			ret;
	struct timeval timeout;
	fd_set		readfds;
	ssize_t		bytes_read;
	int			status;
	int			num_replicas;

	if (NUM_BACKENDS <= 1)
	{
		/* If there's only one node, there's no point to do checking */
		return;
	}

	if (REAL_PRIMARY_NODE_ID < 0)
	{
		/* No need to check if there's no primary */
		return;
	}

	if (!VALID_BACKEND(REAL_PRIMARY_NODE_ID))
	{
		/* No need to check replication delay if primary is down */
		return;
	}

	/* Capture primary node ID to avoid race conditions during execution */
	primary_node_id = REAL_PRIMARY_NODE_ID;

	if (!pool_config->replication_delay_source_cmd ||
		strlen(pool_config->replication_delay_source_cmd) == 0)
	{
		ereport(WARNING,
				(errmsg("replication_delay_source_cmd is not configured"),
				 errhint("Set replication_delay_source_cmd to use external command mode")));
		/* Fall back to builtin method */
		check_replication_time_lag();
		return;
	}

	/* Allocate buffer for command output */
	line = palloc(MAX_CMD_OUTPUT);
	memset(line, 0, MAX_CMD_OUTPUT);

	/*
	 * Register a error context callback to throw proper context message
	 */
	callback.callback = CheckReplicationTimeLagErrorCb;
	callback.arg = NULL;
	callback.previous = error_context_stack;
	error_context_stack = &callback;

	/* Execute command as current process user */
	PG_TRY();
	{
		base_command = pool_config->replication_delay_source_cmd;
		total_len = strlen(base_command) + 1;	/* +1 for NUL */

		/* Build command with replica-only arguments (omit primary) */

		/*
		 * Calculate total command length including space-separated replica
		 * identifiers
		 */
		for (i = 0; i < NUM_BACKENDS; i++)
		{
			if (i == primary_node_id)
				continue;		/* Skip primary node */

			ident = build_instance_identifier_for_node(i);

			total_len += 1 /* space */ + strlen(ident);
			pfree(ident);
		}

		command = palloc(total_len);
		strlcpy(command, base_command, total_len);

		/* Append replica identifiers */
		current_len = strlen(command);

		for (i = 0; i < NUM_BACKENDS; i++)
		{
			if (i == primary_node_id)
				continue;		/* Skip primary node */

			ident = build_instance_identifier_for_node(i);

			/* Append space and identifier */
			snprintf(command + current_len, total_len - current_len, " %s", ident);
			current_len += strlen(command + current_len);

			pfree(ident);
		}

		ereport(DEBUG1,
				(errmsg("executing replication delay command: %s", command)));

		if (pipe(pipefd) == -1)
		{
			ereport(ERROR,
					(errmsg("pipe failed: %m")));
		}

		pid = fork();
		if (pid == -1)
		{
			close(pipefd[0]);
			close(pipefd[1]);
			ereport(ERROR,
					(errmsg("fork failed: %m")));
		}

		if (pid == 0)
		{
			/* Child process */
			close(pipefd[0]);	/* Close read end */
			if (dup2(pipefd[1], STDOUT_FILENO) == -1)
			{
				fprintf(stderr, "dup2 failed: %s\n", strerror(errno));
				exit(1);
			}
			close(pipefd[1]);	/* Close write end (duplicated to stdout) */

			/* Execute command using shell */
			execl("/bin/sh", "sh", "-c", command, (char *) NULL);

			/* If execl fails */
			fprintf(stderr, "execl failed: %s\n", strerror(errno));
			_exit(127);
		}

		/* Parent process */
		close(pipefd[1]);		/* Close write end */
		pipefd[1] = -1;

		/* Set up timeout for select */
		timeout.tv_sec = pool_config->replication_delay_source_timeout;
		timeout.tv_usec = 0;

		FD_ZERO(&readfds);
		FD_SET(pipefd[0], &readfds);

		/* Wait for output or timeout */
		ret = select(pipefd[0] + 1, &readfds, NULL, NULL, &timeout);

		if (ret == -1)
		{
			save_errno = errno;

			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			pid = -1;
			close(pipefd[0]);
			pipefd[0] = -1;
			if (save_errno == EINTR)
			{
				/* Interrupted */
				ereport(ERROR,
						(errmsg("select interrupted during replication delay command execution")));
			}
			else
			{
				ereport(ERROR,
						(errmsg("select failed: %m")));
			}
		}
		else if (ret == 0)
		{
			/* Timeout */
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			pid = -1;
			close(pipefd[0]);
			pipefd[0] = -1;
			ereport(ERROR,
					(errmsg("replication delay command timed out after %d seconds: %s",
							pool_config->replication_delay_source_timeout, command),
					 errhint("Consider increasing replication_delay_source_timeout or optimizing the command")));
		}

		/* Data is available */
		bytes_read = read(pipefd[0], line, MAX_CMD_OUTPUT - 1);
		close(pipefd[0]);
		pipefd[0] = -1;

		/* Wait for child to finish */
		waitpid(pid, &status, 0);
		pid = -1;

		if (bytes_read < 0)
		{
			ereport(ERROR,
					(errmsg("failed to read output from replication delay command: %s", command),
					 errdetail("read failed: %m")));
		}

		/* Check exit status */
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		{
			ereport(ERROR,
					(errmsg("replication delay command failed with exit code %d: %s",
							WEXITSTATUS(status), command)));
		}
		else if (WIFSIGNALED(status))
		{
			ereport(ERROR,
					(errmsg("replication delay command terminated by signal %d: %s",
							WTERMSIG(status), command)));
		}

		/* Check if output was truncated */
		if (bytes_read == MAX_CMD_OUTPUT - 1 && line[MAX_CMD_OUTPUT - 2] != '\n')
		{
			ereport(WARNING,
					(errmsg("replication delay command output may have been truncated")));
		}

		/* Null-terminate the string */
		line[bytes_read] = '\0';

		pfree(command);
		command = NULL;

		/* Set primary node delay to 0 */
		bkinfo = pool_get_node_info(primary_node_id);
		bkinfo->standby_delay = 0;
		bkinfo->standby_delay_by_time = true;

		/* Count expected replicas */
		num_replicas = NUM_BACKENDS - 1;	/* Total nodes minus primary */

		/* Count tokens in output for validation */
		line_copy = pstrdup(line);
		temp_token = strtok(line_copy, " \t\n");

		while (temp_token != NULL)
		{
			token_count++;
			temp_token = strtok(NULL, " \t\n");
		}
		pfree(line_copy);

		/* Validate output format */
		if (token_count == 0)
		{
			ereport(WARNING,
					(errmsg("replication delay command produced no output"),
					 errhint("Command should output delay values separated by spaces, one per replica node")));
		}
		else if (token_count < num_replicas)
		{
			ereport(WARNING,
					(errmsg("replication delay command returned %d values, expected %d (one per replica, excluding primary)",
							token_count, num_replicas),
					 errhint("Command should output one delay value per replica node. Missing values will be treated as 0.")));
		}
		else if (token_count > num_replicas)
		{
			ereport(WARNING,
					(errmsg("replication delay command returned %d values, expected %d (one per replica, excluding primary)",
							token_count, num_replicas),
					 errhint("Command should output exactly one delay value per replica node. Extra values will be ignored.")));
		}

		/* Parse the output - one delay value per replica in order */
		token = strtok_r(line, " \t\n", &saveptr);

		for (i = 0; i < NUM_BACKENDS && token != NULL; i++)
		{
			if (i == primary_node_id)
				continue;		/* Skip primary - it's not in the output */

			if (!VALID_BACKEND(i))
			{
				/* Skip invalid backend but consume token */
				token = strtok_r(NULL, " \t\n", &saveptr);
				continue;
			}

			delay_ms = strtod(token, &endptr);

			/* Validate the conversion */
			if (*endptr != '\0')
			{
				ereport(WARNING,
						(errmsg("invalid delay value '%s' for node %d, treating as 0",
								token, i)));
				delay_ms = 0;
			}

			bkinfo = pool_get_node_info(i);

			/* Handle -1 for down nodes */
			if (delay_ms == -1.0)
			{
				ereport(LOG,
						(errmsg("node %d reported as down by external command (delay -1), relying on health check for failover decision",
								i)));
				/* Keep previous delay value, don't trigger failover */
				token = strtok_r(NULL, " \t\n", &saveptr);
				continue;
			}

			/* Validate delay value range */
			if (delay_ms < 0)
			{
				ereport(WARNING,
						(errmsg("negative delay value %.3f for node %d (other than -1), treating as 0",
								delay_ms, i)));
				delay_ms = 0;
			}
			else if (delay_ms > MAX_REASONABLE_DELAY_MS)
			{
				ereport(WARNING,
						(errmsg("extremely large delay value %.3f for node %d",
								delay_ms, i)));
			}

			/*
			 * Convert delay from milliseconds to microseconds for internal
			 * storage
			 */
			delay = (uint64) (delay_ms * 1000);
			bkinfo->standby_delay = delay;
			bkinfo->standby_delay_by_time = true;

			/*
			 * Log delay if necessary. threshold is in milliseconds, convert
			 * to microseconds.
			 */
			delay_threshold_by_time = pool_config->delay_threshold_by_time * 1000LL;

			if ((pool_config->log_standby_delay == LSD_ALWAYS && delay_ms > 0) ||
				(pool_config->log_standby_delay == LSD_OVER_THRESHOLD &&
				 bkinfo->standby_delay > delay_threshold_by_time))
			{
				ereport(LOG,
						(errmsg("Replication of node: %d is behind %.3f second(s) from the primary server (node: %d) [external command]",
								i, delay_ms / 1000, primary_node_id)));
			}

			token = strtok_r(NULL, " \t\n", &saveptr);
		}

	}
	PG_CATCH();
	{
		/* Cleanup in case of error */
		if (pid > 0)
		{
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
		}
		if (pipefd[0] != -1)
			close(pipefd[0]);
		if (pipefd[1] != -1)
			close(pipefd[1]);

		if (line)
			pfree(line);
		if (command)
			pfree(command);
		error_context_stack = callback.previous;
		PG_RE_THROW();
	}
	PG_END_TRY();

	/* Normal cleanup */
	pfree(line);

	error_context_stack = callback.previous;
}

/*
 * build_instance_identifier_for_node
 *  Build an identifier string for a backend node for passing to external commands.
 *  Format: "<hostname>:<port>"
 */
static char *
build_instance_identifier_for_node(int node_id)
{
	BackendInfo *bi = pool_get_node_info(node_id);
	const char *hostname;

	if (!bi || bi->backend_hostname[0] == '\0' || bi->backend_port <= 0)
	{
		/* Fallback if hostname or port is not set */
		return psprintf("unknown_node_%d", node_id);
	}

	hostname = bi->backend_hostname;

	/* Validate hostname for security - check for shell metacharacters */
	if (strpbrk(hostname, "$`\\|;&<>()[]{}\"\'\n\r\t") != NULL)
	{
		ereport(LOG,
				(errmsg("hostname for node %d contains potentially dangerous characters: %s",
						node_id, hostname),
				 errhint("Hostnames with shell metacharacters may pose security risks when used with external commands. Consider using IP addresses or sanitized hostnames.")));
	}

	/* Use hostname:port format */
	return psprintf("%s:%d", hostname, bi->backend_port);
}

static void
CheckReplicationTimeLagErrorCb(void *arg)
{
	errcontext("while checking replication time lag");
}

/*
 * Convert logid/recoff style text to 64bit log location (LSN)
 */
static unsigned long long int
text_to_lsn(char *text)
{
/*
 * WAL segment size in bytes.  XXX We should fetch this from
 * PostgreSQL, rather than having fixed value.
 */
#define WALSEGMENTSIZE 16 * 1024 * 1024

	unsigned int xlogid;
	unsigned int xrecoff;
	unsigned long long int lsn;

	if (sscanf(text, "%X/%X", &xlogid, &xrecoff) != 2)
	{
		ereport(ERROR,
				(errmsg("invalid LSN format"),
				 errdetail("wrong log location format: %s", text)));

	}
	lsn = xlogid * ((unsigned long long int) 0xffffffff - WALSEGMENTSIZE) + xrecoff;
#ifdef DEBUG
	ereport(LOG,
			(errmsg("lsn: %X %X %llX", xlogid, xrecoff, lsn)));
#endif
	return lsn;
}

static RETSIGTYPE my_signal_handler(int sig)
{
	int			save_errno = errno;

	POOL_SETMASK(&BlockSig);

	switch (sig)
	{
		case SIGTERM:
		case SIGINT:
		case SIGQUIT:
			exit(0);
			break;

			/* Failback or new node added */
		case SIGUSR1:
			restart_request = 1;
			break;

		case SIGCHLD:
			break;

		default:
			exit(1);
			break;
	}

	POOL_SETMASK(&UnBlockSig);

	errno = save_errno;
}

static RETSIGTYPE reload_config_handler(int sig)
{
	int			save_errno = errno;

	POOL_SETMASK(&BlockSig);
	reload_config_request = 1;
	POOL_SETMASK(&UnBlockSig);
	errno = save_errno;
}

static void
reload_config(void)
{
	ereport(LOG,
			(errmsg("reloading config file")));
	MemoryContext oldContext = MemoryContextSwitchTo(TopMemoryContext);

	pool_get_config(get_config_file_name(), CFGCXT_RELOAD);
	MemoryContextSwitchTo(oldContext);
	if (pool_config->enable_pool_hba)
		load_hba(get_hba_file_name());

	if (strcmp("", pool_config->pool_passwd))
		pool_reopen_passwd_file();

	reload_config_request = 0;
}

/*
 * Execute query against specified backend using an established connection to
 * backend.  Return -1 on failure, -2 on no rows returned, or 0 otherwise.
 * Caller must prepare memory for POOL_SELECT_RESULT and pass it as "res". It
 * is guaranteed that no exception occurs within this function.
 */
int
get_query_result(POOL_CONNECTION_POOL_SLOT **slots, int backend_id, char *query, POOL_SELECT_RESULT **res)
{
	int			sts = -1;
	MemoryContext oldContext = CurrentMemoryContext;

	PG_TRY();
	{
		do_query(slots[backend_id]->con, query, res, PROTO_MAJOR_V3);
	}
	PG_CATCH();
	{
		/* ignore the error message */
		res = NULL;
		MemoryContextSwitchTo(oldContext);
		FlushErrorState();
		ereport(LOG,
				(errmsg("get_query_result: do_query failed")));
	}
	PG_END_TRY();

	if (!res)
	{
		ereport(LOG,
				(errmsg("get_query_result: no result returned"),
				 errdetail("node id (%d)", backend_id)));
		return sts;
	}

	if ((*res)->numrows <= 0)
	{
		free_select_result(*res);
		ereport(DEBUG1,
				(errmsg("get_query_result: no rows returned"),
				 errdetail("node id (%d)", backend_id)));
		return -2;
	}

	sts = 0;
	return sts;
}

static void
sr_check_will_die(int code, Datum arg)
{
	if (follow_primary_lock_acquired)
		pool_release_follow_primary_lock(false);

}
