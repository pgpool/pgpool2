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

#define WD_MAX_HOST_NAMELEN (128)
#define MAX_WATCHDOG_NUM (128)
#define WD_SEND_TIMEOUT (1)

#define WD_INFO(wd_id) (pool_config->other_wd->wd_info[(wd_id)])

#define WD_NG (0)
#define WD_OK (1)

/* 
 * packet number of watchdog negotiation
 */
typedef enum {
	WD_INVALID = 0,			/* invalid packet no */
	WD_ADD_REQ,				/* add request into the watchdog list */
	WD_ADD_ACCEPT,			/* accept the add request */
	WD_ADD_REJECT,			/* reject the add request */
	WD_STAND_FOR_MASTER,	/* announce candidacy */
	WD_VOTE_YOU,			/* agree to the candidacy */
	WD_MASTER_EXIST,		/* disagree to the candidacy */
	WD_DECLARE_NEW_MASTER,	/* announce assumption */
	WD_SERVER_DOWN,			/* announce server down */
	WD_READY,				/* answer to the announce */
	WD_START_RECOVERY,		/* announce start online recovery */
	WD_END_RECOVERY,		/* announce end online recovery */
	WD_FAILBACK_REQUEST,	/* announce failback request */
	WD_DEGENERATE_BACKEND,	/* announce degenerate backend */
	WD_PROMOTE_BACKEND,		/* announce promote backend */
	WD_NODE_READY,			/* answer to the node announce */
	WD_NODE_FAILED			/* fail answer to the node announce */
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
}WD_STATUS;

/*
 * watchdog list
 */
typedef struct {
	WD_STATUS status;	/* status */	
	struct timeval tv;	/* startup time value */
	char hostname[WD_MAX_HOST_NAMELEN];	/* host name */
	int pgpool_port;	/* pgpool port */
	int wd_port;		/* watchdog port */
	int life;			/* life point */
}WdInfo;

typedef struct {
	int node_id_set[MAX_NUM_BACKENDS];	/* node sets */
	int node_num;						/* node number */
}WdNodeInfo;

typedef union {
	WdInfo wd_info;
	WdNodeInfo wd_node_info;
} WD_PACKET_BODY;

typedef struct {
	int num_wd;		/* number of watchdogs */
	WdInfo wd_info[MAX_WATCHDOG_NUM];
} WdDesc;

/*
 * negotiation paket
 */
typedef struct {
	WD_PACKET_NO packet_no;	/* packet number */
	WD_PACKET_BODY wd_body;			/* watchdog information */
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
 * thread argument for lifecheck of pgpool
 */
typedef struct {
	PGconn * conn;	/* PGconn */
	int retry;		/* retry times */
} WdPgpoolThreadArg;

extern WdInfo * WD_List;
extern unsigned char * WD_Node_List;

#endif /* WATCHDOG_H */
