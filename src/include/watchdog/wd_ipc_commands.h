
/* -*-pgsql-c-*- */
/*
 *
 * $Header$
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
 *
 */

#ifndef WD_IPC_COMMANDS_H
#define WD_IPC_COMMANDS_H

#include "watchdog/wd_ipc_defines.h"
#include "watchdog/wd_json_data.h"

typedef enum WdCommandResult
{
	CLUSTER_IN_TRANSATIONING,
	COMMAND_OK,
	COMMAND_FAILED,
	COMMAND_TIMEOUT
}WdCommandResult;


typedef struct WDIPCCmdResult
{
	char	type;
	int		length;
	char*	data;
}WDIPCCmdResult;

typedef struct WDGenericData
{
	WDValueDataType	valueType;
	union data
	{
		char	*stringVal;
		int		intVal;
		bool	boolVal;
		long	longVal;
	}data;
}WDGenericData;


extern void wd_ipc_initialize_data(void);
extern char* get_watchdog_ipc_address(void);
extern unsigned int* get_ipc_shared_key(void);
extern void set_watchdog_process_needs_cleanup(void);
extern void reset_watchdog_process_needs_cleanup(void);
extern bool get_watchdog_process_needs_cleanup(void);
extern void set_watchdog_node_escalated(void);
extern void reset_watchdog_node_escalated(void);
extern bool get_watchdog_node_escalation_state(void);

extern WdCommandResult wd_start_recovery(void);
extern WdCommandResult wd_end_recovery(void);
extern WDFailoverCMDResults wd_send_failback_request(int node_id, unsigned int *wd_failover_id);
extern WDFailoverCMDResults wd_degenerate_backend_set(int *node_id_set, int count, unsigned int *wd_failover_id);
extern WDFailoverCMDResults wd_promote_backend(int node_id, unsigned int *wd_failover_id);

extern WDPGBackendStatus* get_pg_backend_status_from_master_wd_node(void);
extern WDGenericData *get_wd_runtime_variable_value(char *varName);
extern WD_STATES get_watchdog_local_node_state(void);

extern char* wd_get_watchdog_nodes(int nodeID);

extern WDIPCCmdResult* issue_command_to_watchdog(char type, int timeout_sec, char* data, int data_len, bool blocking);


/* functions for failover commands interlocking */
extern WDFailoverCMDResults wd_end_failover_interlocking(unsigned int wd_failover_id);
extern WDFailoverCMDResults wd_start_failover_interlocking(unsigned int wd_failover_id);
extern WDFailoverCMDResults wd_failover_lock_release(enum WDFailoverLocks lock, unsigned int wd_failover_id);
extern WDFailoverCMDResults wd_failover_lock_status(enum WDFailoverLocks lock, unsigned int wd_failover_id);
extern void wd_wait_until_command_complete_or_timeout(enum WDFailoverLocks lock, unsigned int wd_failover_id);



#endif /* WD_IPC_COMMANDS_H */
