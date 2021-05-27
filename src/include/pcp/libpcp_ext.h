/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2021	PgPool Global Development Group
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
 * libpcp_ext.h -
 *	  This file contains definitions for structures and
 *	  externs for functions used by frontend libpcp applications.
 */

#ifndef LIBPCP_EXT_H
#define LIBPCP_EXT_H

#include <signal.h>
#include <stdio.h>

/*
 * startup packet definitions (v2) stolen from PostgreSQL
 */
#define SM_DATABASE		64
#define SM_USER			32
#define SM_OPTIONS		64
#define SM_UNUSED		64
#define SM_TTY			64

/*
 * Maximum hostname length including domain name and "." including NULL
 * terminate.
 * https://en.wikipedia.org/wiki/Hostname#cite_note-Raymond,_Microsoft_devblog,_2012-3
 */
#define MAX_FDQN_HOSTNAME_LEN	254

#define MAX_NUM_BACKENDS 128
#define MAX_CONNECTION_SLOTS MAX_NUM_BACKENDS
#define MAX_DB_HOST_NAMELEN	 MAX_FDQN_HOSTNAME_LEN
#define MAX_PATH_LENGTH 256

typedef enum
{
	CON_UNUSED,					/* unused slot */
	CON_CONNECT_WAIT,			/* waiting for connection starting */
	CON_UP,						/* up and running */
	CON_DOWN					/* down, disconnected */
}			BACKEND_STATUS;

/* backend status name strings */
#define BACKEND_STATUS_CON_UNUSED		"unused"
#define BACKEND_STATUS_CON_CONNECT_WAIT	"waiting"
#define BACKEND_STATUS_CON_UP			"up"
#define BACKEND_STATUS_CON_DOWN			"down"
#define BACKEND_STATUS_QUARANTINE		"quarantine"

/*
 * Backend status record file
 */
typedef struct
{
	BACKEND_STATUS status[MAX_NUM_BACKENDS];
}			BackendStatusRecord;

typedef enum
{
	ROLE_MASTER,
	ROLE_SLAVE,
	ROLE_PRIMARY,
	ROLE_STANDBY
}			SERVER_ROLE;

/*
 * PostgreSQL backend descriptor. Placed on shared memory area.
 */
typedef struct
{
	char		backend_hostname[MAX_DB_HOST_NAMELEN];	/* backend host name */
	int			backend_port;	/* backend port numbers */
	BACKEND_STATUS backend_status;	/* backend status */
	time_t		status_changed_time;	/* backend status changed time */
	double		backend_weight; /* normalized backend load balance ratio */
	double		unnormalized_weight;	/* descripted parameter */
	char		backend_data_directory[MAX_PATH_LENGTH];
	unsigned short flag;		/* various flags */
	bool		quarantine;		/* true if node is CON_DOWN because of
								 * quarantine */
	uint64		standby_delay;	/* The replication delay against the primary */
	SERVER_ROLE role;			/* Role of server. used by pcp_node_info and
								 * failover() to keep track of quarantined
								 * primary node */
}			BackendInfo;

typedef struct
{
	sig_atomic_t num_backends;	/* Number of used PostgreSQL backends. This
								 * needs to be a sig_atomic_t type since it is
								 * replaced by a local variable while
								 * reloading pgpool.conf. */

	BackendInfo backend_info[MAX_NUM_BACKENDS];
}			BackendDesc;

/*
 * Connection pool information. Placed on shared memory area.
 */
typedef struct
{
	int			backend_id;		/* backend id */
	char		database[SM_DATABASE];	/* Database name */
	char		user[SM_USER];	/* User name */
	int			major;			/* protocol major version */
	int			minor;			/* protocol minor version */
	int			pid;			/* backend process id */
	int			key;			/* cancel key */
	int			counter;		/* used counter */
	time_t		create_time;	/* connection creation time */
	int			load_balancing_node;	/* load balancing node */
	char		connected;		/* True if frontend connected. Please note
								 * that we use "char" instead of "bool". Since
								 * 3.1, we need to export this structure so
								 * that PostgreSQL C functions can use.
								 * Problem is, PostgreSQL defines bool itself,
								 * and if we use bool, the size of the
								 * structure might be out of control of
								 * pgpool-II. So we use "char" here. */
	volatile char swallow_termination;

	/*
	 * Flag to mark that if the connection will be terminated by the backend.
	 * it should not be treated as a backend node failure. This flag is used
	 * to handle pg_terminate_backend()
	 */
}			ConnectionInfo;

/*
 * process information
 * This object put on shared memory.
 */
typedef struct
{
	pid_t		pid;			/* OS's process id */
	time_t		start_time;		/* fork() time */
	ConnectionInfo *connection_info;	/* head of the connection info for
										 * this process */
	char		need_to_restart;	/* If non 0, exit this child process as
									 * soon as current session ends. Typical
									 * case this flag being set is failback a
									 * node in streaming replication mode. */
}			ProcessInfo;

/*
 * reporting types
 */
/* some length definitions */
#define POOLCONFIG_MAXIDLEN 4
#define POOLCONFIG_MAXNAMELEN 64
#define POOLCONFIG_MAXVALLEN 512
#define POOLCONFIG_MAXDESCLEN 80
#define POOLCONFIG_MAXIDENTLEN 63
#define POOLCONFIG_MAXPORTLEN 6
#define POOLCONFIG_MAXSTATLEN 12
#define POOLCONFIG_MAXWEIGHTLEN 20
#define POOLCONFIG_MAXDATELEN 128
#define POOLCONFIG_MAXCOUNTLEN 16

/* config report struct*/
typedef struct
{
	char		name[POOLCONFIG_MAXNAMELEN + 1];
	char		value[POOLCONFIG_MAXVALLEN + 1];
	char		desc[POOLCONFIG_MAXDESCLEN + 1];
}			POOL_REPORT_CONFIG;

/* nodes report struct */
typedef struct
{
	char		node_id[POOLCONFIG_MAXIDLEN + 1];
	char		hostname[MAX_DB_HOST_NAMELEN + 1];
	char		port[POOLCONFIG_MAXPORTLEN + 1];
	char		status[POOLCONFIG_MAXSTATLEN + 1];
	char		lb_weight[POOLCONFIG_MAXWEIGHTLEN + 1];
	char		role[POOLCONFIG_MAXWEIGHTLEN + 1];
	char		select[POOLCONFIG_MAXWEIGHTLEN + 1];
	char		load_balance_node[POOLCONFIG_MAXWEIGHTLEN + 1];
	char		delay[POOLCONFIG_MAXWEIGHTLEN + 1];
	char		last_status_change[POOLCONFIG_MAXDATELEN];
}			POOL_REPORT_NODES;

/* processes report struct */
typedef struct
{
	char		pool_pid[POOLCONFIG_MAXCOUNTLEN + 1];
	char		start_time[POOLCONFIG_MAXDATELEN + 1];
	char		database[POOLCONFIG_MAXIDENTLEN + 1];
	char		username[POOLCONFIG_MAXIDENTLEN + 1];
	char		create_time[POOLCONFIG_MAXDATELEN + 1];
	char		pool_counter[POOLCONFIG_MAXCOUNTLEN + 1];
}			POOL_REPORT_PROCESSES;

/* pools reporting struct */
typedef struct
{
	int			pool_pid;
	time_t		start_time;
	int			pool_id;
	int			backend_id;
	char		database[POOLCONFIG_MAXIDENTLEN + 1];
	char		username[POOLCONFIG_MAXIDENTLEN + 1];
	time_t		create_time;
	int			pool_majorversion;
	int			pool_minorversion;
	int			pool_counter;
	int			pool_backendpid;
	int			pool_connected;
}			POOL_REPORT_POOLS;

/* version struct */
typedef struct
{
	char		version[POOLCONFIG_MAXVALLEN + 1];
}			POOL_REPORT_VERSION;

typedef enum
{
	PCP_CONNECTION_OK,
	PCP_CONNECTION_CONNECTED,
	PCP_CONNECTION_NOT_CONNECTED,
	PCP_CONNECTION_BAD,
	PCP_CONNECTION_AUTH_ERROR
}			ConnStateType;

typedef enum
{
	PCP_RES_COMMAND_OK,
	PCP_RES_BAD_RESPONSE,
	PCP_RES_BACKEND_ERROR,
	PCP_RES_INCOMPLETE,
	PCP_RES_ERROR
}			ResultStateType;

struct PCPConnInfo;

typedef struct
{
	int			isint;			/* 1 if data in slot is integer, 0 otherwise */
	int			datalen;		/* Length of binary data */
	union
	{
		int		   *ptr;
		int			integer;
	}			data;
	void		(*free_func) (struct PCPConnInfo *, void *);	/* custom free function
																 * deep free of data */
}			PCPResultSlot;

typedef struct
{
	ResultStateType resultStatus;
	int			resultSlots;	/* Total number of slots contained in this
								 * result */
	int			nextFillSlot;	/* internal to keep track of last filled slot */
	PCPResultSlot resultSlot[1];	/* variable length slots */
}			PCPResultInfo;

typedef struct PCPConnInfo
{
	void	   *pcpConn;
	char	   *errMsg;			/* error message, or NULL if no error */
	ConnStateType connState;
	PCPResultInfo *pcpResInfo;
	FILE	   *Pfdebug;		/* File pointer to output debug infor */
}			PCPConnInfo;

struct WdInfo;

extern PCPConnInfo * pcp_connect(char *hostname, int port, char *username, char *password, FILE *Pfdebug);
extern void pcp_disconnect(PCPConnInfo * pcpConn);

extern PCPResultInfo * pcp_terminate_pgpool(PCPConnInfo * pcpCon, char mode);
extern PCPResultInfo * pcp_node_count(PCPConnInfo * pcpCon);
extern PCPResultInfo * pcp_node_info(PCPConnInfo * pcpCon, int nid);

extern PCPResultInfo * pcp_process_count(PCPConnInfo * pcpConn);
extern PCPResultInfo * pcp_process_info(PCPConnInfo * pcpConn, int pid);

extern PCPResultInfo * pcp_detach_node(PCPConnInfo * pcpConn, int nid);
extern PCPResultInfo * pcp_detach_node_gracefully(PCPConnInfo * pcpConn, int nid);
extern PCPResultInfo * pcp_attach_node(PCPConnInfo * pcpConn, int nid);
extern PCPResultInfo * pcp_pool_status(PCPConnInfo * pcpConn);
extern PCPResultInfo * pcp_recovery_node(PCPConnInfo * pcpConn, int nid);
extern PCPResultInfo * pcp_promote_node(PCPConnInfo * pcpConn, int nid);
extern PCPResultInfo * pcp_promote_node_gracefully(PCPConnInfo * pcpConn, int nid);
extern PCPResultInfo * pcp_watchdog_info(PCPConnInfo * pcpConn, int nid);
extern PCPResultInfo * pcp_set_backend_parameter(PCPConnInfo * pcpConn, char *parameter_name, char *value);


extern ResultStateType PCPResultStatus(const PCPResultInfo * res);
extern ConnStateType PCPConnectionStatus(const PCPConnInfo * conn);
extern void *pcp_get_binary_data(const PCPResultInfo * res, unsigned int slotno);
extern int	pcp_get_int_data(const PCPResultInfo * res, unsigned int slotno);
extern int	pcp_get_data_length(const PCPResultInfo * res, unsigned int slotno);
extern void pcp_free_result(PCPConnInfo * pcpConn);
extern void pcp_free_connection(PCPConnInfo * pcpConn);
extern int	pcp_result_slot_count(PCPResultInfo * res);
extern char *pcp_get_last_error(PCPConnInfo * pcpConn);

extern int	pcp_result_is_empty(PCPResultInfo * res);

extern char *role_to_str(SERVER_ROLE role);

/* ------------------------------
 * pcp_error.c
 * ------------------------------
 */

#endif							/* LIBPCP_EXT_H */
