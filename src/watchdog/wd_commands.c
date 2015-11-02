/*
 * $Header$
 *
 * Handles watchdog connection, and protocol communication with pgpool-II
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2015	PgPool Global Development Group
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

#define WD_INTERLOCK_WAIT_MSEC		500
#define WD_INTERLOCK_TIMEOUT_SEC	10
#define WD_INTERLOCK_WAIT_COUNT ((int) ((WD_INTERLOCK_TIMEOUT_SEC * 1000)/WD_INTERLOCK_WAIT_MSEC))

#define FAILBACK_REQUEST_NODE_MASK		0x01
#define DEGENERATE_REQUEST_NODE_MASK	0x02
#define PROMOTE_REQUEST_NODE_MASK		0x04

static void sleep_in_waiting(void);
static WDFailoverCMDResults wd_issue_failover_lock_command(WDFailoverCMDTypes cmdType, char* syncReqType);


static char* get_wd_failover_cmd_type_json(WDFailoverCMDTypes cmdType, char* reqType);
WDFailoverCMDResults wd_send_failover_sync_command(WDFailoverCMDTypes cmdType, char* syncReqType);

static int wd_set_node_mask (unsigned char req_mask, int *node_id_set, int count);
static int wd_chk_node_mask (unsigned char req_mask, int *node_id_set, int count);

static int open_wd_command_sock(bool throw_error);

unsigned char *WD_Node_List = NULL;		/* Lives in shared memory */
char* watchdog_ipc_address = NULL;

void wd_ipc_initialize_data(void)
{
	/* allocate node list */
	if (WD_Node_List == NULL)
	{
		WD_Node_List = pool_shared_memory_create(sizeof(unsigned char) * MAX_NUM_BACKENDS);
		memset(WD_Node_List, 0, sizeof(unsigned char) * MAX_NUM_BACKENDS);
		ereport(DEBUG1,
				(errmsg("WD_Node_List: sizeof(unsigned char) (%zu) * MAX_NUM_BACKENDS (%d) = %zu bytes requested for shared memory",
						sizeof(unsigned char),
						MAX_WATCHDOG_NUM,
						sizeof(unsigned char) * MAX_NUM_BACKENDS)));
	}
	if (watchdog_ipc_address == NULL)
	{
		char wd_ipc_sock_addr[255];
		snprintf(wd_ipc_sock_addr, sizeof(wd_ipc_sock_addr), "%s/.s.PGPOOLWD_CMD.%d",
				 pool_config->wd_ipc_socket_dir,
				 pool_config->wd_port);

		watchdog_ipc_address = pool_shared_memory_create(strlen(wd_ipc_sock_addr));
		strcpy(watchdog_ipc_address, wd_ipc_sock_addr);
	}
}

char* get_watchdog_ipc_address(void)
{
	return watchdog_ipc_address;
}

WDIPCCmdResult*
issue_command_to_watchdog(char type, WD_COMMAND_ACTIONS command_action,int timeout_sec, char* data, int data_len, bool blocking)
{
	struct timeval start_time,tv;
	int sock;
	WDIPCCmdResult* result = NULL;
	char res_type = 'P';
	int res_length, action;
	gettimeofday(&start_time, NULL);
	
	/* open the watchdog command socket for IPC */
	sock = open_wd_command_sock(false);
	if (sock < 0)
		return NULL;

	res_length = htonl(data_len);
	action = htonl(command_action);

	if (socket_write(sock, &type, sizeof(char)) <= 0)
	{
		close(sock);
		return NULL;
	}
	

	if (socket_write(sock, &action, sizeof(int)) <= 0)
	{
		close(sock);
		return NULL;
	}

	if (socket_write(sock, &res_length, sizeof(int)) <= 0)
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
		fd_set fds;
		struct timeval *timeout_st = NULL;
		if (timeout_sec > 0)
		{
			tv.tv_sec = timeout_sec;
			timeout_st = &tv;
		}
		FD_ZERO(&fds);
		FD_SET(sock,&fds);
		for (;;)
		{
			int select_res;
			select_res = select(sock+1,&fds,NULL,NULL,timeout_st);
			if (select_res > 0)
			{
				/* read the result type char */
				if (socket_read(sock, &res_type, 1 ,0) <=0)
				{
					ereport(LOG,
						(errmsg("error reading from IPC command socket"),
							 errdetail("read from socket failed with error \"%s\"",strerror(errno))));
					close(sock);
					return result;
				}
				/* read the result data length */
				if (socket_read(sock, &res_length, sizeof(int), 0) <= 0)
				{
					ereport(LOG,
						(errmsg("error reading from IPC command socket"),
							 errdetail("read from socket failed with error \"%s\"",strerror(errno))));
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
								(errmsg("error reading from IPC command socket"),
								 errdetail("read from socket failed with error \"%s\"",strerror(errno))));
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
		/* For non blocking mode if we are sucessful in sending the command
		 * that means the command is success
		 */
		result = palloc0(sizeof(WDIPCCmdResult));
		result->type = WD_IPC_CMD_RESULT_OK;
	}
	close(sock);
	return result;
}


WdCommandResult
wd_start_recovery(void)
{
	char type;
	char* func = get_wd_node_function_json(WD_FUNCTION_START_RECOVERY, NULL,0);
	
	WDIPCCmdResult *result = issue_command_to_watchdog(WD_FUNCTION_COMMAND, WD_COMMAND_ACTION_DEFAULT,pool_config->recovery_timeout, func, strlen(func), true);
	pfree(func);
	
	if (result == NULL)
	{
		ereport(LOG,
			(errmsg("start recovery command lock failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return COMMAND_FAILED;
	}
	
	type = result->type;
	pfree(result);
	if (type == WD_IPC_CMD_CLUSTER_IN_TRAN)
	{
		ereport(LOG,
			(errmsg("start recovery command lock failed"),
				 errdetail("watchdog cluster is not in stable state"),
					errhint("try again when the cluster is fully initialized")));
		return CLUSTER_IN_TRANSATIONING;
	}
	if (type == WD_IPC_CMD_RESULT_OK)
		return COMMAND_OK;
	
	return COMMAND_FAILED;
}

WdCommandResult
wd_end_recovery(void)
{
	char type;
	char* func = get_wd_node_function_json(WD_FUNCTION_END_RECOVERY, NULL, 0);
	
	WDIPCCmdResult *result = issue_command_to_watchdog(WD_FUNCTION_COMMAND, WD_COMMAND_ACTION_DEFAULT,2, func, strlen(func), true);
	pfree(func);
	
	if (result == NULL)
	{
		ereport(LOG,
				(errmsg("start recovery command lock failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return COMMAND_FAILED;
	}
	
	type = result->type;
	pfree(result);
	if (type == WD_IPC_CMD_CLUSTER_IN_TRAN)
	{
		ereport(LOG,
				(errmsg("start recovery command lock failed"),
				 errdetail("watchdog cluster is not in stable state"),
					errhint("try again when the cluster is fully initialized")));
		return CLUSTER_IN_TRANSATIONING;
	}
	if (type == WD_IPC_CMD_RESULT_OK)
		return COMMAND_OK;
	
	return COMMAND_FAILED;
}


WdCommandResult
wd_send_failback_request(int node_id)
{
	int n = node_id;
	char type;
	char* func;
	
	
	/* if failback packet is received already, do nothing */
	if (wd_chk_node_mask_for_failback_req(&n,1))
		return COMMAND_OK;
	
	func = get_wd_node_function_json(WD_FUNCTION_FAILBACK_REQUEST,&n, 1);
	WDIPCCmdResult *result = issue_command_to_watchdog(WD_FUNCTION_COMMAND, WD_COMMAND_ACTION_DEFAULT,2, func, strlen(func), true);
	pfree(func);
	
	if (result == NULL)
	{
		ereport(LOG,
				(errmsg("start recovery command lock failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return COMMAND_FAILED;
	}
	
	type = result->type;
	pfree(result);
	if (type == WD_IPC_CMD_CLUSTER_IN_TRAN)
	{
		ereport(LOG,
				(errmsg("start recovery command lock failed"),
				 errdetail("watchdog cluster is not in stable state"),
					errhint("try again when the cluster is fully initialized")));
		return CLUSTER_IN_TRANSATIONING;
	}
	if (type == WD_IPC_CMD_RESULT_OK)
		return COMMAND_OK;
	
	return COMMAND_FAILED;
}

static char* get_wd_failover_cmd_type_json(WDFailoverCMDTypes cmdType, char* reqType)
{
	char* json_str;
	JsonNode* jNode = jw_create_with_object(true);
	
	jw_put_int(jNode, "FailoverCMDType", cmdType);
	jw_put_string(jNode, "SyncRequestType", reqType);
	jw_finish_document(jNode);
	json_str = pstrdup(jw_get_json_string(jNode));
	jw_destroy(jNode);
	return json_str;
}


WDFailoverCMDResults
wd_send_failover_sync_command(WDFailoverCMDTypes cmdType, char* syncReqType)
{
	int failoverResCmdType;
	int interlockingResult;
	json_value *root;
	
	char* json_data = get_wd_failover_cmd_type_json(cmdType, syncReqType);
	
	WDIPCCmdResult *result = issue_command_to_watchdog(WD_FAILOVER_CMD_SYNC_REQUEST, WD_COMMAND_ACTION_DEFAULT,pool_config->recovery_timeout, json_data, strlen(json_data), true);

	pfree(json_data);

	if (result == NULL || result->length <= 0)
	{
		ereport(LOG,
				(errmsg("start recovery command lock failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return FAILOVER_RES_ERROR;
	}
	root = json_parse(result->data,result->length);
	/* The root node must be object */
	if (root == NULL || root->type != json_object)
	{
		ereport(NOTICE,
				(errmsg("unable to parse json data from replicate command")));
		return FAILOVER_RES_ERROR;
	}
	
	if (json_get_int_value_for_key(root, "FailoverCMDType", &failoverResCmdType))
	{
		json_value_free(root);
		return FAILOVER_RES_ERROR;
	}
	if (root && json_get_int_value_for_key(root, "InterlockingResult", &interlockingResult))
	{
		json_value_free(root);
		return FAILOVER_RES_ERROR;
	}
	json_value_free(root);
	pfree(result);
	
	if (failoverResCmdType != cmdType)
		return FAILOVER_RES_ERROR;
	
	if (interlockingResult < 0 || interlockingResult > FAILOVER_RES_BLOCKED)
		return FAILOVER_RES_ERROR;
	
	return interlockingResult;
}

WdCommandResult
wd_try_command_lock(void)
{
	WDIPCCmdResult* result;
	char type;
	
	result = issue_command_to_watchdog(WD_TRY_COMMAND_LOCK, WD_COMMAND_ACTION_DEFAULT,10, NULL, 0, true);
	if (result == NULL)
	{
		ereport(LOG,
				(errmsg("watchdog command lock failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return COMMAND_FAILED;
	}
	
	type = result->type;
	pfree(result);
	if (type == WD_IPC_CMD_CLUSTER_IN_TRAN)
	{
		ereport(LOG,
				(errmsg("watchdog command lock failed"),
				 errdetail("watchdog cluster is not in stable state"),
					errhint("try again when the cluster is fully initialized")));
		return CLUSTER_IN_TRANSATIONING;
	}
	
	if (type == WD_IPC_CMD_RESULT_OK)
		return COMMAND_OK;
	
	return COMMAND_FAILED;
}

void wd_command_unlock(void)
{
	/* we dont really care about results here */
	issue_command_to_watchdog(WD_COMMAND_UNLOCK, WD_COMMAND_ACTION_DEFAULT,10, NULL, 0, false);
}

WdCommandResult
wd_degenerate_backend_set(int *node_id_set, int count)
{
	char type;
	char* func;
	
	
	/* if failback packet is received already, do nothing */
	if (wd_chk_node_mask_for_degenerate_req(node_id_set,count))
		return COMMAND_OK;
	
	func = get_wd_node_function_json(WD_FUNCTION_DEGENERATE_REQUEST,node_id_set, count);
	WDIPCCmdResult *result = issue_command_to_watchdog(WD_FUNCTION_COMMAND, WD_COMMAND_ACTION_DEFAULT,2, func, strlen(func), true);
	pfree(func);
	
	if (result == NULL)
	{
		ereport(LOG,
				(errmsg("degenerate backend set command failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return COMMAND_FAILED;
	}
	
	type = result->type;
	pfree(result);
	if (type == WD_IPC_CMD_CLUSTER_IN_TRAN)
	{
		ereport(LOG,
				(errmsg("degenerate backend set command failed"),
				 errdetail("watchdog cluster is not in stable state"),
					errhint("try again when the cluster is fully initialized")));
		return CLUSTER_IN_TRANSATIONING;
	}
	if (type == WD_IPC_CMD_RESULT_OK)
		return COMMAND_OK;
	
	return COMMAND_FAILED;
}

WdCommandResult
wd_promote_backend(int node_id)
{
	int n = node_id;
	char type;
	char* func;
	WDIPCCmdResult *result;
	
	/* if promote packet is received already, do nothing */
	if (wd_chk_node_mask_for_promote_req(&n,1))
		return COMMAND_OK;
	
	func = get_wd_node_function_json(WD_FUNCTION_PROMOTE_REQUEST,&n, 1);
	result = issue_command_to_watchdog(WD_FUNCTION_COMMAND, WD_COMMAND_ACTION_DEFAULT,2, func, strlen(func), true);
	pfree(func);

	if (result == NULL)
	{
		ereport(LOG,
				(errmsg("start recovery command lock failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return COMMAND_FAILED;
	}
	
	type = result->type;
	pfree(result);
	if (type == WD_IPC_CMD_CLUSTER_IN_TRAN)
	{
		ereport(LOG,
				(errmsg("start recovery command lock failed"),
				 errdetail("watchdog cluster is not in stable state"),
					errhint("try again when the cluster is fully initialized")));
		return CLUSTER_IN_TRANSATIONING;
	}
	if (type == WD_IPC_CMD_RESULT_OK)
		return COMMAND_OK;
	
	return COMMAND_FAILED;
}

/*
 * Function returns the JSON of watchdog nodes
 * pass nodeID = -1 to get list of all nodes
 */
char* wd_get_watchdog_nodes(int nodeID)
{
	WDIPCCmdResult *result;
	char* json_str;
	JsonNode* jNode = jw_create_with_object(true);
	jw_put_int(jNode, "NodeID", nodeID);
	jw_finish_document(jNode);
	
	json_str = jw_get_json_string(jNode);
	
	result = issue_command_to_watchdog(WD_GET_NODES_LIST_COMMAND, WD_COMMAND_ACTION_DEFAULT,5, json_str, strlen(json_str), true);
	
	jw_destroy(jNode);
	
	if (result == NULL)
	{
		ereport(LOG,
				(errmsg("get watchdog nodes command failed"),
				 errdetail("issue command to watchdog returned NULL")));
		return NULL;
	}
	
	if (result->type == WD_IPC_CMD_CLUSTER_IN_TRAN)
	{
		ereport(LOG,
				(errmsg("get watchdog nodes command failed"),
				 errdetail("watchdog cluster is not in stable state"),
					errhint("try again when the cluster is fully initialized")));
		pfree(result);
		return NULL;
	}
	if (result->type == WD_IPC_CMD_RESULT_OK)
	{
		char* data = result->data;
		pfree(result);
		return data;
	}
	pfree(result);
	return NULL;
}

static int
open_wd_command_sock(bool throw_error)
{
	size_t	len;
	struct sockaddr_un addr;
	int sock = -1;
	
	/* We use unix domain stream sockets for the purpose */
	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		/* socket create failed */
		ereport(throw_error? ERROR:LOG,
				(errmsg("failed to connect to watchdog command server socket"),
				 errdetail("connect on \"%s\" failed with reason: \"%s\"", addr.sun_path, strerror(errno))));
		return -1;
	}
	
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path),"%s",watchdog_ipc_address);
	len = sizeof(struct sockaddr_un);
	
	if (connect(sock, (struct sockaddr *) &addr, len) == -1)
	{
		close(sock);
		ereport(throw_error? ERROR:LOG,
				(errmsg("failed to connect to watchdog command server socket"),
				 errdetail("connect on \"%s\" failed with reason: \"%s\"", addr.sun_path, strerror(errno))));
		return -1;
	}
	return sock;
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


/* check mask, and if maskted return 1 and clear it, otherwise return 0 */
static int
wd_chk_node_mask (unsigned char req_mask, int *node_id_set, int count)
{
	int rtn = 0;
	int i;
	int offset = 0;
	for ( i = 0 ; i < count ; i ++)
	{
		offset = *(node_id_set+i);
		if ((*(WD_Node_List + offset) & req_mask) != 0)
		{
			*(WD_Node_List + offset) ^= req_mask;
			rtn = 1;
		}
	}
	return rtn;
}

/* set mask */
static int
wd_set_node_mask (unsigned char req_mask, int *node_id_set, int count)
{
	int i;
	int offset = 0;
	for ( i = 0 ; i < count ; i ++)
	{
		offset = *(node_id_set+i);
		*(WD_Node_List + offset) |= req_mask;
	}
	return 0;
}


int
wd_set_node_mask_for_failback_req(int *node_id_set, int count)
{
	return wd_set_node_mask (FAILBACK_REQUEST_NODE_MASK, node_id_set, count);
}

int
wd_set_node_mask_for_degenerate_req(int *node_id_set, int count)
{
	return wd_set_node_mask (DEGENERATE_REQUEST_NODE_MASK, node_id_set, count);
}

int
wd_set_node_mask_for_promote_req(int *node_id_set, int count)
{
	return wd_set_node_mask (PROMOTE_REQUEST_NODE_MASK, node_id_set, count);
}

int
wd_chk_node_mask_for_failback_req(int *node_id_set, int count)
{
	return wd_chk_node_mask (FAILBACK_REQUEST_NODE_MASK, node_id_set, count);
}

int
wd_chk_node_mask_for_degenerate_req(int *node_id_set, int count)
{
	return wd_chk_node_mask (DEGENERATE_REQUEST_NODE_MASK, node_id_set, count);
}

int
wd_chk_node_mask_for_promote_req(int *node_id_set, int count)
{
	return wd_chk_node_mask (PROMOTE_REQUEST_NODE_MASK, node_id_set, count);
}

