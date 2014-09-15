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
 * watchdog.h.: watchdog definition header file
 *
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <sys/time.h>
#include "libpq-fe.h"

#include "auth/md5.h"

#define WD_MAX_HOST_NAMELEN (128)
#define WD_MAX_PATH_LEN (128)
#define MAX_WATCHDOG_NUM (128)
#define WD_SEND_TIMEOUT (1)
#define WD_MAX_IF_NUM (256)
#define WD_MAX_IF_NAME_LEN (16)

#define WD_INFO(wd_id) (pool_config->other_wd->wd_info[(wd_id)])
#define WD_HB_IF(if_id) (pool_config->hb_if[(if_id)])

#define WD_MYSELF (WD_List)

#define WD_NG (0)
#define WD_OK (1)

#define WD_MAX_PACKET_STRING (256)

#define WD_TIME_INIT(tv)      ((tv).tv_sec = (tv).tv_usec = 0)
#define WD_TIME_ISSET(tv)     ((tv).tv_sec || (tv).tv_usec)
#define WD_TIME_BEFORE(a,b)   (((a).tv_sec == (b).tv_sec) ? \
                               ((a).tv_usec < (b).tv_usec) : \
                               ((a).tv_sec < (b).tv_sec))
#define WD_TIME_DIFF_SEC(a,b) (int)(((a).tv_sec - (b).tv_sec) + \
                                    ((a).tv_usec - (b).tv_usec) / 1000000.0)

/*
 * packet number of watchdog negotiation
 */
typedef enum {

	/* normal packet */
	WD_INVALID = 0,				/* invalid packet no */
	WD_INFO_REQ,				/* information request */
	WD_ADD_REQ,					/* add request into the watchdog list */
	WD_ADD_ACCEPT,				/* accept the add request */
	WD_ADD_REJECT,				/* reject the add request */
	WD_STAND_FOR_MASTER,		/* announce candidacy */
	WD_VOTE_YOU,				/* agree to the candidacy */
	WD_MASTER_EXIST,			/* disagree to the candidacy */
	WD_DECLARE_NEW_MASTER,		/* announce assumption */
	WD_STAND_FOR_LOCK_HOLDER,	/* announce candidacy for lock holder */
	WD_LOCK_HOLDER_EXIST,		/* reject the assumption for lock holder */
	WD_DECLARE_LOCK_HOLDER,		/* announce to assume lock holder */
	WD_RESIGN_LOCK_HOLDER,		/* announce to resign lock holder */
	WD_START_INTERLOCK,			/* announce to start interlocking */
	WD_END_INTERLOCK,			/* announce to end interlocking */
	WD_SERVER_DOWN,				/* announce server down */
	WD_AUTH_FAILED,				/* fail answer to authentication */
	WD_READY,					/* answer to the announce */

	/* node packet */
	WD_START_RECOVERY,		/* announce start online recovery */
	WD_END_RECOVERY,		/* announce end online recovery */
	WD_FAILBACK_REQUEST,	/* announce failback request */
	WD_DEGENERATE_BACKEND,	/* announce degenerate backend */
	WD_PROMOTE_BACKEND,		/* announce promote backend */
	WD_NODE_READY,			/* answer to the node announce */
	WD_NODE_FAILED,			/* fail answer to the node announce */

	/* lock packet */
	WD_UNLOCK_REQUEST,		/* announce to unlock command */
	WD_LOCK_READY,			/* answer to the lock announce */
	WD_LOCK_FAILED			/* fail answer to the lock announce */

} WD_PACKET_NO;

/*
 * watchdog status
 */
typedef enum {
	WD_END = 0,
	WD_INIT,
	WD_NORMAL,
	WD_MASTER,
	WD_DOWN
} WD_STATUS;

/*
 * watchdog locks
 */
typedef enum {
	WD_FAILOVER_START_LOCK = 0,
	WD_FAILOVER_END_LOCK,
	WD_FAILOVER_COMMAND_LOCK,
	WD_FAILBACK_COMMAND_LOCK,
	WD_FOLLOW_MASTER_COMMAND_LOCK,
	WD_MAX_LOCK_NUM
} WD_LOCK_ID;

/*
 * watchdog list
 */
typedef struct WdInfo {
	WD_STATUS status;						/* status */
	struct timeval tv;						/* startup time value */
	char hostname[WD_MAX_HOST_NAMELEN];		/* host name */
	int pgpool_port;						/* pgpool port */
	int wd_port;							/* watchdog port */
	int life;								/* life point */
	char delegate_ip[WD_MAX_HOST_NAMELEN];	/* delegate IP */
	int delegate_ip_flag;					/* delegate IP flag */
	struct timeval hb_send_time; 			/* send time */
	struct timeval hb_last_recv_time; 		/* recv time */
	bool is_lock_holder;					/* lock holder flag */
	bool in_interlocking;					/* interlocking is in progress */
	bool is_contactable;					/* able to create socket and connection */
} WdInfo;

typedef struct {
	int node_id_set[MAX_NUM_BACKENDS];	/* node sets */
	int node_num;						/* node number */
} WdNodeInfo;

typedef struct {
	WD_LOCK_ID lock_id;
} WdLockInfo;

typedef union {
	WdInfo wd_info;
	WdNodeInfo wd_node_info;
	WdLockInfo wd_lock_info;
} WD_PACKET_BODY;

typedef struct {
	char addr[WD_MAX_HOST_NAMELEN];
	char if_name[WD_MAX_IF_NAME_LEN];
	int dest_port;
} WdHbIf;

typedef struct {
	int num_wd;		/* number of watchdogs */
	WdInfo wd_info[MAX_WATCHDOG_NUM];
} WdDesc;

/*
 * negotiation packet
 */
typedef struct {
	WD_PACKET_NO packet_no;	/* packet number */
	WD_PACKET_BODY wd_body;			/* watchdog information */
	struct timeval send_time;
	char hash[(MD5_PASSWD_LEN+1)*2];
} WdPacket;

/*
 * thread argument for watchdog negotiation
 */
typedef struct {
	int sock;			/* socket */
	WdInfo * target;	/* target watchdog information */
	WdPacket * packet;	/* packet data */
} WdPacketThreadArg;

/*
 * heartbeat packet
 */
typedef struct {
	char from[WD_MAX_HOST_NAMELEN];
	int from_pgpool_port;
	struct timeval send_time;
	WD_STATUS status;
	char hash[(MD5_PASSWD_LEN+1)*2];
} WdHbPacket;

/*
 * thread argument for lifecheck of pgpool
 */
typedef struct {
	PGconn * conn;	/* PGconn */
	int retry;		/* retry times (not used?)*/
} WdPgpoolThreadArg;

/*
 * thread information for pool_thread
 */
typedef struct {
	void *(*start_routine)(void *);
	void *arg;
} WdThreadInfo;

extern WdInfo * WD_List;
extern unsigned char * WD_Node_List;

#endif /* WATCHDOG_H */
