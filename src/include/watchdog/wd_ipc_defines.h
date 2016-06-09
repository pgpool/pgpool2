/* -*-pgsql-c-*- */
/*
 *
 * $Header$
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
	FAILOVER_RES_ERROR = 0,				/* processing of command is failed */
	FAILOVER_RES_TRANSITION,			/* cluster is transitioning and is 
										 * currently not accepting any commands.
										 * retry is the best option when this result
										 * is returned by watchdog
										 */
	FAILOVER_RES_I_AM_LOCK_HOLDER,		/* node successfully becomes a lock holder */
	FAILOVER_RES_LOCK_UNLOCKED,			/* the node is not a lock holder but associated
										 * lock is unlocked */
	FAILOVER_RES_BLOCKED				/* the node is neither a lock holder and
										 * associated lock is also locked
										 */
}WDFailoverCMDResults;


/* IPC MESSAGES TYPES */
#define WD_REGISTER_FOR_NOTIFICATION		'0'
#define WD_NODE_STATUS_CHANGE_COMMAND		'2'
#define WD_GET_NODES_LIST_COMMAND			'3'
#define WD_NODES_LIST_DATA					'4'

#define WD_IPC_CMD_CLUSTER_IN_TRAN			'5'
#define WD_IPC_CMD_RESULT_BAD				'6'
#define WD_IPC_CMD_RESULT_OK				'7'
#define WD_IPC_CMD_TIMEOUT					'8'

#define WD_FUNCTION_COMMAND					'f'
#define WD_FAILOVER_CMD_SYNC_REQUEST		's'


#define WD_FUNCTION_START_RECOVERY		"START_RECOVERY"
#define WD_FUNCTION_END_RECOVERY		"END_RECOVERY"
#define WD_FUNCTION_FAILBACK_REQUEST	"FAILBACK_REQUEST"
#define WD_FUNCTION_DEGENERATE_REQUEST	"DEGENERATE_BACKEND_REQUEST"
#define WD_FUNCTION_PROMOTE_REQUEST		"PROMOTE_BACKEND_REQUEST"

#define WD_IPC_AUTH_KEY			"IPCAuthKey"	/* JSON data key for authentication.
												 * watchdog IPC server use the value for this key
												 * to authenticate the external IPC clients
												 * The valid value for this key is wd_authkey
												 * configuration parameter
												 */
#define WD_IPC_SHARED_KEY		"IPCSharedKey"	/* JSON data key for authentication.
												 * watchdog IPC server use the value of this key
												 * to authenticate the internal pgpool-II processes
												 */

/* Use to inform node new node status by lifecheck */
#define WD_LIFECHECK_NODE_STATUS_DEAD	1
#define WD_LIFECHECK_NODE_STATUS_ALIVE	2



#endif
