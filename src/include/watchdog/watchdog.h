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
 * watchdog.h.: watchdog definition header file
 *
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <sys/time.h>

#define WD_TIME_INIT(tv)      ((tv).tv_sec = (tv).tv_usec = 0)
#define WD_TIME_ISSET(tv)     ((tv).tv_sec || (tv).tv_usec)
#define WD_TIME_BEFORE(a,b)   (((a).tv_sec == (b).tv_sec) ? \
                               ((a).tv_usec < (b).tv_usec) : \
                               ((a).tv_sec < (b).tv_sec))
#define WD_TIME_DIFF_SEC(a,b) (int)(((a).tv_sec - (b).tv_sec) + \
                                    ((a).tv_usec - (b).tv_usec) / 1000000.0)


/*
 * watchdog state
 */
typedef enum {
	WD_DEAD = 0,
	WD_LOADING,
	WD_JOINING,
	WD_INITIALIZING,
	WD_WAITING_CONNECT,
	WD_COORDINATOR,
	WD_PARTICIPATE_IN_ELECTION,
	WD_STAND_FOR_COORDINATOR,
	WD_STANDBY,
	WD_WAITING_FOR_QUORUM,
	WD_LOST,
	/* the following states are only valid on local node */
	WD_IN_NW_TROUBLE,
	/* the following states are only valid on remote nodes */
	WD_SHUTDOWN,
	WD_ADD_MESSAGE_SENT
} WD_STATES;

typedef enum {
	WD_SOCK_UNINITIALIZED = 0,
	WD_SOCK_CREATED,
	WD_SOCK_WAITING_FOR_CONNECT,
	WD_SOCK_CONNECTED,
	WD_SOCK_ERROR,
	WD_SOCK_CLOSED
} WD_SOCK_STATE;

typedef enum {
	WD_EVENT_WD_STATE_CHANGED = 0,
	WD_EVENT_TIMEOUT,
	WD_EVENT_PACKET_RCV,
	WD_EVENT_COMMAND_FINISHED,
	WD_EVENT_NEW_OUTBOUND_CONNECTION,
	
	WD_EVENT_NW_IP_IS_REMOVED,
	WD_EVENT_NW_IP_IS_ASSIGNED,

	WD_EVENT_LOCAL_NODE_LOST,
	WD_EVENT_REMOTE_NODE_LOST,
	WD_EVENT_REMOTE_NODE_FOUND,
	WD_EVENT_LOCAL_NODE_FOUND,

	WD_EVENT_NODE_CON_LOST,
	WD_EVENT_NODE_CON_FOUND

} WD_EVENTS;

typedef struct SocketConnection
{
	int				sock;
	struct			timeval tv;
	char			addr[48];
	WD_SOCK_STATE	sock_state;
}SocketConnection;

typedef struct WatchdogNode
{
	WD_STATES state;
	struct timeval tv;						/* startup time value */
	char nodeName[WD_MAX_HOST_NAMELEN];
	char hostname[WD_MAX_HOST_NAMELEN];		/* host name */
	int wd_port;							/* watchdog port */
	int pgpool_port;						/* pgpool port */
	int wd_priority;						/* watchdog priority */
	char delegate_ip[WD_MAX_HOST_NAMELEN];	/* delegate IP */
	char** resolved_ips;
	int delegate_ip_flag;					/* delegate IP flag */
	unsigned int	lastCommandID;
	struct timeval hb_last_recv_time; 		/* recv time */
	int	private_id;
	SocketConnection server_socket;
	SocketConnection client_socket;
	bool is_connectable;					/* true if any of the socket is connected */
	bool is_lock_holder;					/* lock holder flag */
	bool in_interlocking;					/* interlocking is in progress */
}WatchdogNode;

extern pid_t initialize_watchdog(void);

#endif /* WATCHDOG_H */


