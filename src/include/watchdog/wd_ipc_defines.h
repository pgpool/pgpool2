/* -*-pgsql-c-*- */
/*
 *
 * $Header$
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
 *
 */

#ifndef WD_IPC_DEFINES_H
#define WD_IPC_DEFINES_H

typedef enum WDFailoverCMDTypes
{
	NODE_FAILED_CMD = 0,
	NODE_FAILBACK_CMD,
	NODE_PROMOTE_CMD,
	MAX_FAILOVER_CMDS
}WDFailoverCMDTypes;

typedef enum WDFailoverCMDResults
{
	FAILOVER_RES_ERROR = 0,
	FAILOVER_RES_TRANSITION,
	FAILOVER_RES_PROCEED_LOCK_HOLDER,
	FAILOVER_RES_PROCEED_UNLOCKED,
	FAILOVER_RES_BLOCKED
}WDFailoverCMDResults;

typedef enum {
	WD_COMMAND_ACTION_DEFAULT = 0,
	WD_COMMAND_ACTION_SEND_ALL,
	WD_COMMAND_ACTION_SEND_MASTER,
	WD_COMMAND_ACTION_LOCAL
} WD_COMMAND_ACTIONS;


/* IPC MESSAGES TYPES */
#define WD_REGISTER_FOR_NOTIFICATION		'0'
#define WD_NODE_STATUS_CHANGE_COMMAND		'2'
#define WD_GET_NODES_LIST_COMMAND			'3'
#define WD_NODES_LIST_DATA					'4'

#define WD_TRY_COMMAND_LOCK					'5'
#define WD_COMMAND_UNLOCK					'6'

#define WD_IPC_CMD_CLUSTER_IN_TRAN			'7'
#define WD_IPC_CMD_RESULT_BAD				'8'
#define WD_IPC_CMD_RESULT_OK				'9'

#define WD_FUNCTION_COMMAND					'f'
#define WD_FAILOVER_CMD_SYNC_REQUEST		's'


#define WD_FUNCTION_START_RECOVERY		"START_RECOVERY"
#define WD_FUNCTION_END_RECOVERY		"END_RECOVERY"
#define WD_FUNCTION_FAILBACK_REQUEST	"FAILBACK_REQUEST"
#define WD_FUNCTION_DEGENERATE_REQUEST	"DEGENERATE_BACKEND_REQUEST"
#define WD_FUNCTION_PROMOTE_REQUEST		"PROMOTE_BACKEND_REQUEST"

/* Use to inform node new node status by lifecheck */
#define WD_LIFECHECK_NODE_STATUS_DEAD	1
#define WD_LIFECHECK_NODE_STATUS_ALIVE	2



#endif
