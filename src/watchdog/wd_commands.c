/*
 * $Header$
 *
 * Handles watchdog connection, and protocol communication with pgpool-II
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2016	PgPool Global Development Group
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
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "pool.h"
#include "utils/elog.h"
#include "utils/json_writer.h"
#include "utils/json.h"
#include "utils/pool_stream.h"
#include "pool_config.h"
#include "watchdog/wd_json_data.h"
#include "watchdog/wd_ipc_commands.h"
#include "watchdog/wd_ipc_defines.h"

#define WD_DEFAULT_IPC_COMMAND_TIMEOUT	8	/* default number of seconds to
											 * wait for IPC command results */
#define WD_INTERLOCK_WAIT_MSEC		500
#define WD_INTERLOCK_TIMEOUT_SEC	10
#define WD_INTERLOCK_WAIT_COUNT ((int) ((WD_INTERLOCK_TIMEOUT_SEC * 1000)/WD_INTERLOCK_WAIT_MSEC))

static void FreeCmdResult(WDIPCCmdResult * res);
static char *get_wd_failover_state_json(bool start);

static int	open_wd_command_sock(bool throw_error);
static WDFailoverCMDResults wd_get_failover_result_from_data(WDIPCCmdResult * result, unsigned int *wd_failover_id);
static WDFailoverCMDResults wd_issue_failover_command(char *func_name, int *node_id_set, int count, unsigned char flags);
/* shared memory variables */
char	   *watchdog_ipc_address = NULL;
bool	   *watchdog_require_cleanup = NULL;	/* shared memory variable set
												 * to true when watchdog
												 * process terminates
												 * abnormally */
bool	   *watchdog_node_escalated = NULL; /* shared memory variable set to
											 * true when watchdog process has
											 * performed escalation */
unsigned int *ipc_shared_key = NULL;	/* key lives in shared memory used to
										 * identify the ipc internal clients */
void
wd_ipc_initialize_data(void)
{
	if (watchdog_ipc_address == NULL)
	{
		char		wd_ipc_sock_addr[255];

		snprintf(wd_ipc_sock_addr, sizeof(wd_ipc_sock_addr), "%s/.s.PGPOOLWD_CMD.%d",
				 pool_config->wd_ipc_socket_dir,
				 pool_config->wd_port);

		watchdog_ipc_address = pool_shared_memory_create(strlen(wd_ipc_sock_addr) + 1);
		strcpy(watchdog_ipc_address, wd_ipc_sock_addr);
	}

	if (ipc_shared_key == NULL)
	{
		ipc_shared_key = pool_shared_memory_create(sizeof(unsigned int));
		*ipc_shared_key = 0;
		while (*ipc_shared_key == 0)
		{
			pool_random_salt((char *) ipc_shared_key);
		}
	}

	if (watchdog_require_cleanup == NULL)
	{
		watchdog_require_cleanup = pool_shared_memory_create(sizeof(bool));
		*watchdog_require_cleanup = false;
	}

	if (watchdog_node_escalated == NULL)
	{
		watchdog_node_escalated = pool_shared_memory_create(sizeof(bool));
		*watchdog_node_escalated = false;
	}
}

WD_STATES
get_watchdog_local_node_state(void)
{
	WD_STATES	ret = WD_DEAD;
	WDGenericData *state = get_wd_runtime_variable_value(WD_RUNTIME_VAR_WD_STATE);

	if (state == NULL)
	{
		ereport(LOG,
				(errmsg("failed to get current state of local watchdog node"),
				 errdetail("get runtime variable value from watchdog returned no data")));
		return WD_DEAD;
	}
	if (state->valueType != VALUE_DATA_TYPE_INT)
	{
		ereport(LOG,
				(errmsg("failed to get current state of local watchdog node"),
				 errdetail("get runtime variable value from watchdog returned invalid value type")));
		pfree(state);
		return WD_DEAD;
	}
	ret = (WD_STATES) state->data.intVal;
	pfree(state);
	return ret;
}

int
get_watchdog_quorum_state(void)
{
	WD_STATES	ret = WD_DEAD;
	WDGenericData *state = get_wd_runtime_variable_value(WD_RUNTIME_VAR_QUORUM_STATE);

	if (state == NULL)
	{
		ereport(LOG,
				(errmsg("failed to get quorum state of watchdog cluster"),
				 errdetail("get runtime variable value from watchdog returned no data")));
		return WD_DEAD;
	}
	if (state->valueType != VALUE_DATA_TYPE_INT)
	{
		ereport(LOG,
				(errmsg("failed to get quorum state of watchdog cluster"),
				 errdetail("get runtime variable value from watchdog returned invalid value type")));
		pfree(state);
		return WD_DEAD;
	}
	ret = (WD_STATES) state->data.intVal;
	pfree(state);
	return ret;
}

char *
get_watchdog_ipc_address(void)
{
	return watchdog_ipc_address;
}

unsigned int *
get_ipc_shared_key(void)
{
	return ipc_shared_key;
}

void
set_watchdog_process_needs_cleanup(void)
{
	*watchdog_require_cleanup = true;
}

void
reset_watchdog_process_needs_cleanup(void)
{
	*watchdog_require_cleanup = false;
}

bool
get_watchdog_process_needs_cleanup(void)
{
	return *watchdog_require_cleanup;
}


void
set_watchdog_node_escalated(void)
{
	*watchdog_node_escalated = true;
}

void
reset_watchdog_node_escalated(void)
{
	*watchdog_node_escalated = false;
}

bool
get_watchdog_node_escalation_state(void)
{
	return *watchdog_node_escalated;
}

/*
 * function issues the command to watchdog process over the watchdog
 * IPC command socket.
 * type:            command type to send. valid command
 *                  types are defined in wd_ipc_defines.h
 * timeout_sec:     number of seconds to wait for the command response
 *                  from watchdog
 * data:            command data
 * data_len:        length of data
 * blocking:        send true if caller wants to wait for the results
 *                  when blocking is false the timeout_sec is ignored
 */
WDIPCCmdResult *
issue_command_to_watchdog(char type, int timeout_sec, char *data, int data_len, bool blocking)
{
	struct timeval start_time,
				tv;
	int			sock;
	WDIPCCmdResult *result = NULL;
	char		res_type = 'P';
	int			res_length,
				len;

	gettimeofday(&start_time, NULL);

	/* open the watchdog command socket for IPC */
	sock = open_wd_command_sock(false);
	if (sock < 0)
		return NULL;

	len = htonl(data_len);

	if (socket_write(sock, &type, sizeof(char)) <= 0)
	{
		close(sock);
		return NULL;
	}

	if (socket_write(sock, &len, sizeof(int)) <= 0)
	{
		close(sock);
		return NULL;
	}
	if (data && data_len > 0)
	{
		if (socket_write(sock, data, data_len) <= 0)
		{
			close(sock);
			return NULL;
		}
	}

	if (blocking)
	{
		/* if we are asked to wait for results */
		fd_set		fds;
		struct timeval *timeout_st = NULL;

		if (timeout_sec > 0)
		{
			tv.tv_sec = timeout_sec;
			tv.tv_usec = 0;
			timeout_st = &tv;
		}
		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		for (;;)
		{
			int			select_res;

			select_res = select(sock + 1, &fds, NULL, NULL, timeout_st);
			if (select_res == 0)
			{
				close(sock);
				result = palloc(sizeof(WDIPCCmdResult));
				result->type = WD_IPC_CMD_TIMEOUT;
				result->length = 0;
				result->data = NULL;
				return result;
			}
			if (select_res < 0)
			{
				if (errno == EAGAIN || errno == EINTR)
					continue;
				ereport(WARNING,
						(errmsg("error reading from IPC command socket for ipc command %c", type),
						 errdetail("select system call failed with error \"%s\"", strerror(errno))));
				close(sock);
				return NULL;
			}
			if (select_res > 0)
			{
				/* read the result type char */
				if (socket_read(sock, &res_type, 1, 0) <= 0)
				{
					ereport(WARNING,
							(errmsg("error reading from IPC command socket for ipc command %c", type),
							 errdetail("read from socket failed with error \"%s\"", strerror(errno))));
					close(sock);
					return result;
				}
				/* read the result data length */
				if (socket_read(sock, &res_length, sizeof(int), 0) <= 0)
				{
					ereport(WARNING,
							(errmsg("error reading from IPC command socket for ipc command %c", type),
							 errdetail("read from socket failed with error \"%s\"", strerror(errno))));
					close(sock);
					return result;
				}

				result = palloc(sizeof(WDIPCCmdResult));
				result->type = res_type;
				result->length = ntohl(res_length);
				result->data = NULL;

				if (result->length > 0)
				{
					result->data = palloc(result->length);
					if (socket_read(sock, result->data, result->length, 0) <= 0)
					{
						pfree(result->data);
						pfree(result);
						ereport(DEBUG1,
								(errmsg("error reading from IPC command socket for ipc command %c", type),
								 errdetail("read from socket failed with error \"%s\"", strerror(errno))));
						close(sock);
						return NULL;
					}
				}
				break;
			}
		}
	}
	else
	{
		/*
		 * For non blocking mode if we are sucessful in sending the command
		 * that means the command is success
		 */
		result = palloc0(sizeof(WDIPCCmdResult));
		result->type = WD_IPC_CMD_RESULT_OK;
	}
	close(sock);
	return result;
}

/*
 * Function gets the runtime value of watchdog varibale using the
 * watchdog IPC
 */
WDGenericData *
get_wd_runtime_variable_value(char *varName)
{
	unsigned int *shared_key = get_ipc_shared_key();
	char	   *data = get_simple_request_json(WD_JSON_KEY_VARIABLE_NAME, varName,
											   shared_key ? *shared_key : 0, pool_config->wd_authkey);

	WDIPCCmdResult *result = issue_command_to_watchdog(WD_GET_RUNTIME_VARIABLE_VALUE,
													   WD_DEFAULT_IPC_COMMAND_TIMEOUT,
													   data, strlen(data), true);

	pfree(data);

	if (result == NULL)
	{
		ereport(WARNING,
				(errmsg("get runtime variable value from watchdog failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return NULL;
	}
	if (result->type == WD_IPC_CMD_CLUSTER_IN_TRAN)
	{
		ereport(WARNING,
				(errmsg("get runtime variable value from watchdog failed"),
				 errdetail("watchdog cluster is not in stable state"),
				 errhint("try again when the cluster is fully initialized")));
		FreeCmdResult(result);
		return NULL;
	}
	else if (result->type == WD_IPC_CMD_TIMEOUT)
	{
		ereport(WARNING,
				(errmsg("get runtime variable value from watchdog failed"),
				 errdetail("ipc command timeout")));
		FreeCmdResult(result);
		return NULL;
	}
	else if (result->type == WD_IPC_CMD_RESULT_OK)
	{
		json_value *root = NULL;
		WDGenericData *genData = NULL;
		WDValueDataType dayaType;

		root = json_parse(result->data, result->length);
		/* The root node must be object */
		if (root == NULL || root->type != json_object)
		{
			FreeCmdResult(result);
			return NULL;
		}

		if (json_get_int_value_for_key(root, WD_JSON_KEY_VALUE_DATA_TYPE, (int *) &dayaType))
		{
			FreeCmdResult(result);
			json_value_free(root);
			return NULL;
		}

		switch (dayaType)
		{
			case VALUE_DATA_TYPE_INT:
				{
					int			intVal;

					if (json_get_int_value_for_key(root, WD_JSON_KEY_VALUE_DATA, &intVal))
					{
						ereport(WARNING,
								(errmsg("get runtime variable value from watchdog failed"),
								 errdetail("unable to get INT value from JSON data returned by watchdog")));
					}
					else
					{
						genData = palloc(sizeof(WDGenericData));
						genData->valueType = dayaType;
						genData->data.intVal = intVal;
					}
				}
				break;

			case VALUE_DATA_TYPE_LONG:
				{
					long		longVal;

					if (json_get_long_value_for_key(root, WD_JSON_KEY_VALUE_DATA, &longVal))
					{
						ereport(WARNING,
								(errmsg("get runtime variable value from watchdog failed"),
								 errdetail("unable to get LONG value from JSON data returned by watchdog")));
					}
					else
					{
						genData = palloc(sizeof(WDGenericData));
						genData->valueType = dayaType;
						genData->data.longVal = longVal;
					}
				}
				break;

			case VALUE_DATA_TYPE_BOOL:
				{
					bool		boolVal;

					if (json_get_bool_value_for_key(root, WD_JSON_KEY_VALUE_DATA, &boolVal))
					{
						ereport(WARNING,
								(errmsg("get runtime variable value from watchdog failed"),
								 errdetail("unable to get BOOL value from JSON data returned by watchdog")));
					}
					else
					{
						genData = palloc(sizeof(WDGenericData));
						genData->valueType = dayaType;
						genData->data.boolVal = boolVal;
					}
				}
				break;

			case VALUE_DATA_TYPE_STRING:
				{
					char	   *ptr = json_get_string_value_for_key(root, WD_JSON_KEY_VALUE_DATA);

					if (ptr == NULL)
					{
						ereport(WARNING,
								(errmsg("get runtime variable value from watchdog failed"),
								 errdetail("unable to get STRING value from JSON data returned by watchdog")));
					}
					else
					{
						genData = palloc(sizeof(WDGenericData));
						genData->valueType = dayaType;
						genData->data.stringVal = pstrdup(ptr);
					}
				}
				break;

			default:
				ereport(WARNING,
						(errmsg("get runtime variable value from watchdog failed, unknown value data type")));
				break;
		}

		json_value_free(root);
		FreeCmdResult(result);
		return genData;
	}

	ereport(WARNING,
			(errmsg("get runtime variable value from watchdog failed")));
	FreeCmdResult(result);
	return NULL;

}

/*
 * function gets the PG backend status of all attached nodes from
 * the master watchdog node.
 */
WDPGBackendStatus *
get_pg_backend_status_from_master_wd_node(void)
{
	unsigned int *shared_key = get_ipc_shared_key();
	char	   *data = get_data_request_json(WD_DATE_REQ_PG_BACKEND_DATA,
											 shared_key ? *shared_key : 0, pool_config->wd_authkey);

	WDIPCCmdResult *result = issue_command_to_watchdog(WD_GET_MASTER_DATA_REQUEST,
													   WD_DEFAULT_IPC_COMMAND_TIMEOUT,
													   data, strlen(data), true);

	pfree(data);

	if (result == NULL)
	{
		ereport(WARNING,
				(errmsg("get backend node status from master watchdog failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return NULL;
	}
	if (result->type == WD_IPC_CMD_CLUSTER_IN_TRAN)
	{
		ereport(WARNING,
				(errmsg("get backend node status from master watchdog failed"),
				 errdetail("watchdog cluster is not in stable state"),
				 errhint("try again when the cluster is fully initialized")));
		FreeCmdResult(result);
		return NULL;
	}
	else if (result->type == WD_IPC_CMD_TIMEOUT)
	{
		ereport(WARNING,
				(errmsg("get backend node status from master watchdog failed"),
				 errdetail("ipc command timeout")));
		FreeCmdResult(result);
		return NULL;
	}
	else if (result->type == WD_IPC_CMD_RESULT_OK)
	{
		WDPGBackendStatus *backendStatus = get_pg_backend_node_status_from_json(result->data, result->length);

		/*
		 * Watchdog returns the zero length data when the node itself is a
		 * master watchdog node
		 */
		if (result->length <= 0)
		{
			backendStatus = palloc0(sizeof(WDPGBackendStatus));
			backendStatus->node_count = -1;
		}
		else
		{
			backendStatus = get_pg_backend_node_status_from_json(result->data, result->length);
		}
		FreeCmdResult(result);
		return backendStatus;
	}

	ereport(WARNING,
			(errmsg("get backend node status from master watchdog failed")));
	FreeCmdResult(result);
	return NULL;
}

WdCommandResult
wd_start_recovery(void)
{
	char		type;
	unsigned int *shared_key = get_ipc_shared_key();

	char	   *func = get_wd_node_function_json(WD_FUNCTION_START_RECOVERY, NULL, 0, 0,
												 shared_key ? *shared_key : 0, pool_config->wd_authkey);

	WDIPCCmdResult *result = issue_command_to_watchdog(WD_IPC_ONLINE_RECOVERY_COMMAND,
													   pool_config->recovery_timeout,
													   func, strlen(func), true);

	pfree(func);

	if (result == NULL)
	{
		ereport(WARNING,
				(errmsg("start recovery command lock failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return COMMAND_FAILED;
	}

	type = result->type;
	FreeCmdResult(result);
	if (type == WD_IPC_CMD_CLUSTER_IN_TRAN)
	{
		ereport(WARNING,
				(errmsg("start recovery command lock failed"),
				 errdetail("watchdog cluster is not in stable state"),
				 errhint("try again when the cluster is fully initialized")));
		return CLUSTER_IN_TRANSATIONING;
	}
	else if (type == WD_IPC_CMD_TIMEOUT)
	{
		ereport(WARNING,
				(errmsg("start recovery command lock failed"),
				 errdetail("ipc command timeout")));
		return COMMAND_TIMEOUT;
	}
	else if (type == WD_IPC_CMD_RESULT_OK)
	{
		return COMMAND_OK;
	}
	return COMMAND_FAILED;
}

WdCommandResult
wd_end_recovery(void)
{
	char		type;
	unsigned int *shared_key = get_ipc_shared_key();

	char	   *func = get_wd_node_function_json(WD_FUNCTION_END_RECOVERY, NULL, 0, 0,
												 shared_key ? *shared_key : 0, pool_config->wd_authkey);


	WDIPCCmdResult *result = issue_command_to_watchdog(WD_IPC_ONLINE_RECOVERY_COMMAND,
													   WD_DEFAULT_IPC_COMMAND_TIMEOUT,
													   func, strlen(func), true);

	pfree(func);

	if (result == NULL)
	{
		ereport(WARNING,
				(errmsg("end recovery command lock failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return COMMAND_FAILED;
	}

	type = result->type;
	FreeCmdResult(result);

	if (type == WD_IPC_CMD_CLUSTER_IN_TRAN)
	{
		ereport(WARNING,
				(errmsg("end recovery command lock failed"),
				 errdetail("watchdog cluster is not in stable state"),
				 errhint("try again when the cluster is fully initialized")));
		return CLUSTER_IN_TRANSATIONING;
	}
	else if (type == WD_IPC_CMD_TIMEOUT)
	{
		ereport(WARNING,
				(errmsg("end recovery command lock failed"),
				 errdetail("ipc command timeout")));
		return COMMAND_TIMEOUT;
	}
	else if (type == WD_IPC_CMD_RESULT_OK)
	{
		return COMMAND_OK;
	}
	return COMMAND_FAILED;
}

static char *
get_wd_failover_state_json(bool start)
{
	char	   *json_str;
	JsonNode   *jNode = jw_create_with_object(true);
	unsigned int *shared_key = get_ipc_shared_key();

	jw_put_int(jNode, WD_IPC_SHARED_KEY, shared_key ? *shared_key : 0); /* put the shared key */
	if (pool_config->wd_authkey != NULL && strlen(pool_config->wd_authkey) > 0)
		jw_put_string(jNode, WD_IPC_AUTH_KEY, pool_config->wd_authkey); /* put the auth key */

	jw_put_int(jNode, "FailoverFuncState", start ? 0 : 1);
	jw_finish_document(jNode);
	json_str = pstrdup(jw_get_json_string(jNode));
	jw_destroy(jNode);
	return json_str;
}

static WDFailoverCMDResults
wd_send_failover_func_status_command(bool start)
{
	WDFailoverCMDResults res;
	unsigned int failover_id;

	char	   *json_data = get_wd_failover_state_json(start);

	WDIPCCmdResult *result = issue_command_to_watchdog(WD_FAILOVER_INDICATION
													   ,pool_config->recovery_timeout,
													   json_data, strlen(json_data), true);

	pfree(json_data);

	res = wd_get_failover_result_from_data(result, &failover_id);

	FreeCmdResult(result);
	return res;
}

static WDFailoverCMDResults wd_get_failover_result_from_data(WDIPCCmdResult * result, unsigned int *wd_failover_id)
{
	if (result == NULL)
		return FAILOVER_RES_ERROR;

	if (result == NULL)
	{
		ereport(WARNING,
				(errmsg("failover command on watchdog failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return FAILOVER_RES_ERROR;
	}
	if (result->type == WD_IPC_CMD_CLUSTER_IN_TRAN)
	{
		ereport(WARNING,
				(errmsg("failover command on watchdog failed"),
				 errdetail("watchdog cluster is not in stable state"),
				 errhint("try again when the cluster is fully initialized")));
		return FAILOVER_RES_TRANSITION;
	}
	else if (result->type == WD_IPC_CMD_TIMEOUT)
	{
		ereport(WARNING,
				(errmsg("failover command on watchdog failed"),
				 errdetail("ipc command timeout")));
		return FAILOVER_RES_TIMEOUT;
	}
	else if (result->type == WD_IPC_CMD_RESULT_OK)
	{
		WDFailoverCMDResults res = FAILOVER_RES_ERROR;
		json_value *root;

		root = json_parse(result->data, result->length);
		/* The root node must be object */
		if (root == NULL || root->type != json_object)
		{
			ereport(NOTICE,
					(errmsg("unable to parse json data from failover command result")));
			return res;
		}
		if (root && json_get_int_value_for_key(root, WD_FAILOVER_RESULT_KEY, (int *) &res))
		{
			json_value_free(root);
			return FAILOVER_RES_ERROR;
		}
		if (root && json_get_int_value_for_key(root, WD_FAILOVER_ID_KEY, (int *) wd_failover_id))
		{
			json_value_free(root);
			return FAILOVER_RES_ERROR;
		}
		return res;
	}
	return FAILOVER_RES_ERROR;
}

static WDFailoverCMDResults
wd_issue_failover_command(char *func_name, int *node_id_set, int count, unsigned char flags)
{
	WDFailoverCMDResults res;
	char	   *func;
	unsigned int *shared_key = get_ipc_shared_key();
	unsigned int wd_failover_id;

	func = get_wd_node_function_json(func_name, node_id_set, count, flags,
									 shared_key ? *shared_key : 0, pool_config->wd_authkey);

	WDIPCCmdResult *result = issue_command_to_watchdog(WD_IPC_FAILOVER_COMMAND,
													   WD_DEFAULT_IPC_COMMAND_TIMEOUT,
													   func, strlen(func), true);

	pfree(func);
	res = wd_get_failover_result_from_data(result, &wd_failover_id);
	FreeCmdResult(result);
	return res;
}

/*
 * send the degenerate backend request to watchdog.
 * now watchdog can respond to the request in following ways.
 *
 * 1 - It can tell the caller to procees with failover. This
 * happens when the current node is the master watchdog node.
 *
 * 2 - It can tell the caller to failover not allowed
 * this happens when either cluster does not have the quorum
 *
 */
WDFailoverCMDResults
wd_degenerate_backend_set(int *node_id_set, int count, unsigned char flags)
{
	if (pool_config->use_watchdog)
		return wd_issue_failover_command(WD_FUNCTION_DEGENERATE_REQUEST, node_id_set, count, flags);
	return FAILOVER_RES_PROCEED;
}

WDFailoverCMDResults
wd_promote_backend(int node_id, unsigned char flags)
{
	if (pool_config->use_watchdog)
		return wd_issue_failover_command(WD_FUNCTION_PROMOTE_REQUEST, &node_id, 1, flags);
	return FAILOVER_RES_PROCEED;
}

WDFailoverCMDResults
wd_send_failback_request(int node_id, unsigned char flags)
{
	if (pool_config->use_watchdog)
		return wd_issue_failover_command(WD_FUNCTION_FAILBACK_REQUEST, &node_id, 1, flags);
	return FAILOVER_RES_PROCEED;
}

/*
 * Function returns the JSON of watchdog nodes
 * pass nodeID = -1 to get list of all nodes
 */
char *
wd_get_watchdog_nodes(int nodeID)
{
	WDIPCCmdResult *result;
	char	   *json_str;
	unsigned int *shared_key = get_ipc_shared_key();

	JsonNode   *jNode = jw_create_with_object(true);

	jw_put_int(jNode, "NodeID", nodeID);

	jw_put_int(jNode, WD_IPC_SHARED_KEY, shared_key ? *shared_key : 0); /* put the shared key */

	if (pool_config->wd_authkey != NULL && strlen(pool_config->wd_authkey) > 0)
		jw_put_string(jNode, WD_IPC_AUTH_KEY, pool_config->wd_authkey); /* put the auth key */

	jw_finish_document(jNode);

	json_str = jw_get_json_string(jNode);

	result = issue_command_to_watchdog(WD_GET_NODES_LIST_COMMAND
									   ,WD_DEFAULT_IPC_COMMAND_TIMEOUT,
									   json_str, strlen(json_str), true);

	jw_destroy(jNode);

	if (result == NULL)
	{
		ereport(WARNING,
				(errmsg("get watchdog nodes command failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return NULL;
	}
	else if (result->type == WD_IPC_CMD_CLUSTER_IN_TRAN)
	{
		ereport(WARNING,
				(errmsg("get watchdog nodes command failed"),
				 errdetail("watchdog cluster is not in stable state"),
				 errhint("try again when the cluster is fully initialized")));
		FreeCmdResult(result);
		return NULL;
	}
	else if (result->type == WD_IPC_CMD_TIMEOUT)
	{
		ereport(WARNING,
				(errmsg("get watchdog nodes command failed"),
				 errdetail("ipc command timeout")));
		FreeCmdResult(result);
		return NULL;
	}
	else if (result->type == WD_IPC_CMD_RESULT_OK)
	{
		char	   *data = result->data;

		/* do not free the result->data, Save the data copy */
		pfree(result);
		return data;
	}
	FreeCmdResult(result);
	return NULL;
}

static int
open_wd_command_sock(bool throw_error)
{
	size_t		len;
	struct sockaddr_un addr;
	int			sock = -1;

	/* We use unix domain stream sockets for the purpose */
	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		/* socket create failed */
		ereport(throw_error ? ERROR : LOG,
				(errmsg("failed to connect to watchdog command server socket"),
				 errdetail("connect on \"%s\" failed with reason: \"%s\"", addr.sun_path, strerror(errno))));
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", watchdog_ipc_address);
	len = sizeof(struct sockaddr_un);

	if (connect(sock, (struct sockaddr *) &addr, len) == -1)
	{
		close(sock);
		ereport(throw_error ? ERROR : LOG,
				(errmsg("failed to connect to watchdog command server socket"),
				 errdetail("connect on \"%s\" failed with reason: \"%s\"", addr.sun_path, strerror(errno))));
		return -1;
	}
	return sock;
}

WDFailoverCMDResults
wd_failover_start(void)
{
	if (pool_config->use_watchdog)
		return wd_send_failover_func_status_command(true);
	return FAILOVER_RES_PROCEED;
}

WDFailoverCMDResults
wd_failover_end(void)
{
	if (pool_config->use_watchdog)
		return wd_send_failover_func_status_command(false);
	return FAILOVER_RES_PROCEED;
}


static void
FreeCmdResult(WDIPCCmdResult * res)
{
	if (res == NULL)
		return;

	if (res->data)
		pfree(res->data);
	pfree(res);
}
