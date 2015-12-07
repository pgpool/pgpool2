
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

#ifndef WD_IPC_COMMANDS_H
#define WD_IPC_COMMANDS_H

#include "watchdog/wd_ipc_defines.h"

typedef enum WdCommandResult
{
	CLUSTER_IN_TRANSATIONING,
	COMMAND_OK,
	COMMAND_FAILED
}WdCommandResult;


typedef struct WDIPCCmdResult
{
	char	type;
	int		length;
	char*	data;
}WDIPCCmdResult;

extern void wd_ipc_initialize_data(void);
extern char* get_watchdog_ipc_address(void);

extern int wd_set_node_mask_for_failback_req(int *node_id_set, int count);
extern int wd_set_node_mask_for_degenerate_req(int *node_id_set, int count);
extern int wd_set_node_mask_for_promote_req(int *node_id_set, int count);
extern int wd_chk_node_mask_for_failback_req(int *node_id_set, int count);
extern int wd_chk_node_mask_for_degenerate_req(int *node_id_set, int count);
extern int wd_chk_node_mask_for_promote_req(int *node_id_set, int count);


extern WdCommandResult wd_start_recovery(void);
extern WdCommandResult wd_end_recovery(void);
extern WdCommandResult wd_send_failback_request(int node_id);
extern WdCommandResult wd_degenerate_backend_set(int *node_id_set, int count);
extern WdCommandResult wd_promote_backend(int node_id);
extern WDFailoverCMDResults wd_send_failover_sync_command(WDFailoverCMDTypes cmdType, char* syncReqType);


extern char* wd_get_watchdog_nodes(int nodeID);

extern WDIPCCmdResult* issue_command_to_watchdog(char type, int timeout_sec, char* data, int data_len, bool blocking);


/* wd_interlock.c */

extern WDFailoverCMDResults wd_release_failover_command_lock(WDFailoverCMDTypes cmdType);
extern WDFailoverCMDResults wd_failover_command_check_lock(WDFailoverCMDTypes cmdType);
extern WDFailoverCMDResults wd_failover_command_end(WDFailoverCMDTypes cmdType);
extern WDFailoverCMDResults wd_failover_command_start(WDFailoverCMDTypes cmdType);
extern void wd_wati_until_lock_or_timeout(WDFailoverCMDTypes cmdType);



#endif /* WD_IPC_COMMANDS_H */
