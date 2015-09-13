/* -*-pgsql-c-*- */
/*
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
 * child.c: child process main
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <fcntl.h>

#include "pool.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/elog.h"
#include "utils/json_writer.h"
#include "utils/json.h"
#include "pool_config.h"
#include "watchdog/wd_ext.h"
#include "watchdog/watchdog.h"
#include "parser/stringinfo.h"

#define MAX_PACKET_SIZE 512

#define WD_NO_MESSAGE						0
#define WD_INVALID_MESSAGE					'i'
#define WD_INFO_MESSAGE						'I'
#define WD_REQ_INFO_MESSAGE					'r'
#define WD_IAM_COORDINATOR_MESSAGE			'M'
#define WD_ADD_NODE_MESSAGE					'A'
#define WD_STAND_FOR_COORDINATOR_MESSAGE	'S'
#define WD_DECLARE_COORDINATOR_MESSAGE		'C'
#define WD_ACCEPT_MESSAGE					'G'
#define WD_REJECT_MESSAGE					'R'
#define WD_ERROR_MESSAGE					'E'
#define WD_JOIN_COORDINATOR_MESSAGE			'J'
#define WD_PGPOOL_COMMAND					'P'
#define WD_QUORUM_IS_LOST					'q'


typedef struct WDPacketData
{
	char	type;
	int		command_id;
	int		ptr;
	int		data_len;
	char	data_buf[MAX_PACKET_SIZE];
}WDPacketData;

typedef struct WDIPCCommandData
{
	WD_COMMAND_ACTIONS command_action;
	MemoryContext		memoryContext;
	int				issueing_sock;
	char			type;
	struct timeval	issue_time;
	unsigned int	internal_command_id;
	int				data_len;
	char			*data_buf;
	WDIPCCommandResult* command_result;
}WDIPCCommandData;


static void FileUnlink(int code, Datum path);
static void wd_child_exit(int exit_signo);
static int wd_send_response(int sock, WdPacket * recv_pack);
static void wd_node_request_signal(WD_PACKET_NO packet_no, WdNodeInfo *node);

static void wd_cluster_initialize(void);
static int wd_create_client_socket(char * hostname, int port, bool *connected);
static int connect_with_all_configured_nodes(void);
static int update_successful_outgoing_cons(fd_set* wmask, int pending_fds_count);
static int set_local_node_state(WD_STATES newState);
static int prepare_fds(fd_set* rmask, fd_set* wmask, fd_set* emask);

static WDPacketData* get_addnode_message(void);
static WDPacketData* get_mynode_info_message(WDPacketData* replyFor);
static WDPacketData* fill_myinfo_in_message(WDPacketData* message);
static WDPacketData* get_minimum_message(char type, WDPacketData* replyFor);

static void finish_wdMessage(WDPacketData* pkt);
static bool put_int_in_message(WDPacketData* pkt, int value);
static bool put_bytes_in_message(WDPacketData* pkt, const char* value, int len);
static void update_message_length(WDPacketData* pkt);
static void set_next_commandID_in_message(WDPacketData* pkt);
static void set_message_commandID(WDPacketData* pkt, unsigned int commandID);
static void set_message_type(WDPacketData* pkt, char type);
static void free_packet(WDPacketData *pkt);
static WDPacketData* get_empty_packet(void);
static unsigned int get_next_commandID(void);
static WatchdogNode* parse_node_info_message(WDPacketData* pkt);
static int get_quorum_status(void);

static bool write_packet_to_socket(int sock, WDPacketData* pkt);
static WDPacketData* read_packet_of_type(int sock, char ensure_type);
static WDPacketData* read_packet(int sock);
static int read_sockets(fd_set* rmask,int pending_fds_count);
static void set_timeout(unsigned int sec);
static int wd_create_command_server_socket(void);

static int send_cluster_command_packet(WatchdogNode* wdNode, WDPacketData *pkt, int timeout_sec);
static int send_message(WatchdogNode* wdNode, WDPacketData *pkt);
static bool send_message_to_node(WatchdogNode* wdNode, WDPacketData *pkt);
static bool reply_with_minimal_message(WatchdogNode* wdNode, char type, WDPacketData* replyFor);
static int send_cluster_command(WatchdogNode* wdNode, char type, int timeout_sec);
static int accept_incomming_connections(fd_set* rmask, int pending_fds_count);

static int standard_packet_processor(WatchdogNode* wdNode, WDPacketData* pkt);
static int update_connected_node_count(void);
static int get_cluster_node_count(void);
static bool reply_is_for_last_command(WDPacketData* pkt);
static inline WD_STATES get_local_node_state(void);
static int set_state(WD_STATES newState);

static int watchdog_state_machine_standby(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt);
static int watchdog_state_machine_voting(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt);
static int watchdog_state_machine_coordinator(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt);
static int watchdog_state_machine_standForCord(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt);
static int watchdog_state_machine_initializing(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt);
static int watchdog_state_machine_joing(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt);
static int watchdog_state_machine_loading(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt);
static int watchdog_state_machine(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt);
static int watchdog_state_machine_waiting_for_quorum(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt);

static int write_IPCResult_To_Socket(int sock, WDIPCCommandResult* IPCResult);
static int process_IPC_transport_command(WDIPCCommandData *IPCCommand);
static bool packet_is_received_for_pgpool_command(WatchdogNode* wdNode, WDPacketData* pkt);
static void free_IPC_command(WDIPCCommandData* ipcCommand);
static bool read_ipc_command_and_process(int socket, bool *remove_socket);

static JsonNode* get_node_list_json(void);
static bool add_nodeinfo_to_json(JsonNode* jNode, WatchdogNode* node);
static bool parse_node_status_json(char* json_data, int data_len,int* nodeID, NodeStates* nodeState);
static bool fire_node_status_event(int nodeID, NodeStates nodeState);
static int process_IPC_nodeStatusChange_command(WDIPCCommandData* IPCCommand);
static void resign_from_coordinator(void);

/* global variables */
wd_cluster g_cluster;
struct timeval g_tm_set_time;
int g_timeout_sec = 0;

static unsigned int get_next_commandID(void)
{
	return ++g_cluster.nextCommandID;
}

static void set_timeout(unsigned int sec)
{
	g_timeout_sec = sec;
	gettimeofday(&g_tm_set_time,NULL);
}

static void wd_cluster_initialize(void)
{
	int i = 0;

	if (pool_config->other_wd == NULL)
	{
		ereport(ERROR,
				(errmsg("initializing watchdog information. memory allocation error")));
	}
	if (pool_config->other_wd->num_wd <= 0)
	{
		/* should also have upper limit???*/
		ereport(ERROR,
				(errmsg("initializing watchdog failed. no watchdog nodes configured")));
	}
	/* initialize local node settings */
	g_cluster.localNode = palloc0(sizeof(WatchdogNode));
	g_cluster.localNode->wd_port = pool_config->wd_port;
	g_cluster.localNode->pgpool_port = pool_config->port;
	g_cluster.localNode->private_id = 0;
	strcpy(g_cluster.localNode->hostname, pool_config->wd_hostname);
	strcpy(g_cluster.localNode->delegate_ip, pool_config->delegate_IP);
	/* Assign the node name */
	{
		struct utsname unameData;
		uname(&unameData);
		snprintf(g_cluster.localNode->nodeName, WD_MAX_HOST_NAMELEN, "%s_%s_%d",unameData.sysname,unameData.nodename,pool_config->port);
		/* should also have upper limit???*/
		ereport(LOG,
				(errmsg("setting the local watchdog node name to \"%s\"",g_cluster.localNode->nodeName)));
		
	}
	
	/* initialize remote nodes */
	g_cluster.remoteNodeCount = pool_config->other_wd->num_wd;
	g_cluster.remoteNodes = palloc0((sizeof(WatchdogNode) * g_cluster.remoteNodeCount));

	ereport(LOG,
			(errmsg("watchdog cluster configured with %d nodes",pool_config->other_wd->num_wd)));

	for ( i = 0; i < pool_config->other_wd->num_wd; i++)
	{
		WdInfo *p = &(pool_config->other_wd->wd_info[i]);
		g_cluster.remoteNodes[i].wd_port = p->wd_port;
		g_cluster.remoteNodes[i].private_id = i+1;
		g_cluster.remoteNodes[i].pgpool_port = p->pgpool_port;
		strcpy(g_cluster.remoteNodes[i].hostname, p->hostname);
		g_cluster.remoteNodes[i].delegate_ip[0] = 0;	/*this will be populated by remote node*/

		ereport(LOG,
				(errmsg("watchdog remote node:%d on %s:%d",i,g_cluster.remoteNodes[i].hostname, g_cluster.remoteNodes[i].wd_port)));

	}
	
	g_cluster.masterNode = NULL;
	g_cluster.aliveNodeCount = 0;
	g_cluster.quorum_exists =false;
	g_cluster.lastCommand.commandID = 0;
	g_cluster.nextCommandID = 1;
	g_cluster.unidentified_socks = NULL;
	g_cluster.command_server_sock = 0;
	g_cluster.notify_clients = NULL;
	g_cluster.ipc_command_socks = NULL;
	g_cluster.localNode->state = WD_DEAD;
}

/*
 * creates a socket in non blocking mode and connects it to the hostname and port
 * the out parameter connected is set to true if the connection is successfull
 */
static int
wd_create_client_socket(char * hostname, int port, bool *connected)
{
	int sock;
	int one = 1;
	size_t len = 0;
	struct sockaddr_in addr;
	struct hostent * hp;
	
	*connected = false;
	/* create socket */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		/* socket create failed */
		printf("create socket failed with reason: \"%s\"\n", strerror(errno));
		return -1;
	}

	/* set socket option */
	if ( setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &one, sizeof(one)) == -1 )
	{
		close(sock);
		ereport(LOG,
			(errmsg("failed to set socket options"),
				 errdetail("setsockopt(TCP_NODELAY) failed with error: \"%s\"", strerror(errno))));
		return -1;
	}
	if ( setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &one, sizeof(one)) == -1 )
	{
		ereport(LOG,
			(errmsg("failed to set socket options"),
				 errdetail("setsockopt(SO_KEEPALIVE) failed with error: \"%s\"", strerror(errno))));
		close(sock);
		return -1;
	}
	
	/* set sockaddr_in */
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	hp = gethostbyname(hostname);
	if ((hp == NULL) || (hp->h_addrtype != AF_INET))
	{
		hp = gethostbyaddr(hostname,strlen(hostname),AF_INET);
		if ((hp == NULL) || (hp->h_addrtype != AF_INET))
		{
			ereport(LOG,
				(errmsg("failed to host"),
					 errdetail("gethostbyaddr failed with error: \"%s\"", hstrerror(h_errno))));
			close(sock);
			return -1;
		}
	}
	memmove((char *)&(addr.sin_addr), (char *)hp->h_addr, hp->h_length);
	addr.sin_port = htons(port);
	len = sizeof(struct sockaddr_in);
	
	/* set socket to non blocking */
	int flags = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);
	
	if (connect(sock,(struct sockaddr*)&addr, len) < 0)
	{
		if (errno == EINPROGRESS)
		{
			return sock;
		}
		if (errno == EISCONN)
		{
			*connected = true;
			/* set socket to blocking again */
			flags = fcntl(sock, F_GETFL, 0);
			fcntl(sock, F_SETFL, flags | ~O_NONBLOCK);

			return sock;
		}
		ereport(LOG,
				(errmsg("connect on socket failed"),
				 errdetail("connect failed with error: \"%s\"", strerror(errno))));
		close(sock);
		return -1;
	}
	*connected = true;
	return sock;
}

/* returns the number of successfull connections */
static int
connect_with_all_configured_nodes(void)
{
	int connect_count = 0;
	int i;
	for (i=0; i< g_cluster.remoteNodeCount; i++)
	{
		bool connected = false;
		WatchdogNode* wdNode = &(g_cluster.remoteNodes[i]);
		wdNode->client_sock = wd_create_client_socket(wdNode->hostname, wdNode->wd_port, &connected);
		if (wdNode->client_sock <= 0)
		{
			wdNode->client_sock_state = WD_SOCK_ERROR;
			ereport(DEBUG1,
					(errmsg("outbound connection to \"%s:%d\" failed", wdNode->hostname, wdNode->wd_port)));
		}
		else
		{
			if (connected)
			{
				wdNode->client_sock_state = WD_SOCK_CONNECTED;
//				WDPacketData* pkt = get_addnode_message();
//				if (write_packet_to_socket(wdNode->client_sock, pkt) == false)
//				{
//					ereport(LOG,
//						(errmsg("write failed on socket to host %s:%d",wdNode->hostname,wdNode->wd_port),
//							 errdetail("%s",strerror(errno))));
//					/* What to do? close the connection seems a reasonable option. Any other thoughts??*/
//					close(wdNode->client_sock);
//					wdNode->client_sock = -1;
//					wdNode->client_sock_state = WD_SOCK_ERROR;
//				}
//				free_packet(pkt);
			}
			else
				wdNode->client_sock_state = WD_SOCK_WAITING_FOR_CONNECT;
			connect_count++;
		}
	}
	return connect_count;
}


pid_t
wd_child(int fork_wait_time)
{
	fd_set rmask;
	fd_set wmask;
	fd_set emask;
	const int select_timeout = 1;
	struct timeval tv, ref_time;
	tv.tv_sec = select_timeout;
	tv.tv_usec = 0;
	
	
	volatile int fd;
	pid_t pid = 0;
	sigjmp_buf	local_sigjmp_buf;

	pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			ereport(PANIC,
					(errmsg("failed to fork a watchdog process")));

		return pid;
	}

	on_exit_reset();
	processType = PT_WATCHDOG;

	if (fork_wait_time > 0)
	{
		sleep(fork_wait_time);
	}

	POOL_SETMASK(&UnBlockSig);

	pool_signal(SIGTERM, wd_child_exit);
	pool_signal(SIGINT, wd_child_exit);
	pool_signal(SIGQUIT, wd_child_exit);
	pool_signal(SIGCHLD, SIG_DFL);
	pool_signal(SIGHUP, SIG_IGN);
	pool_signal(SIGUSR1, SIG_IGN);
	pool_signal(SIGUSR2, SIG_IGN);
	pool_signal(SIGPIPE, SIG_IGN);
	pool_signal(SIGALRM, SIG_IGN);

	init_ps_display("", "", "", "");

	/* Create per loop iteration memory context */
	ProcessLoopContext = AllocSetContextCreate(TopMemoryContext,
											   "wd_child_main_loop",
											   ALLOCSET_DEFAULT_MINSIZE,
											   ALLOCSET_DEFAULT_INITSIZE,
											   ALLOCSET_DEFAULT_MAXSIZE);
	
	MemoryContextSwitchTo(TopMemoryContext);

	set_ps_display("watchdog", false);

	/*DEBUGING*/
	pool_config->log_min_messages = DEBUG5;

	/* initialize all the local structures for watchdog */
	wd_cluster_initialize();
	/* create a server socket for incoming watchdog connections */
	g_cluster.localNode->server_sock = wd_create_recv_socket(g_cluster.localNode->wd_port);
	/* open the command server */
	g_cluster.command_server_sock = wd_create_command_server_socket();
	/* try connecting to all watchdog nodes */
	connect_with_all_configured_nodes();

	set_local_node_state(WD_LOADING);

	if (sigsetjmp(local_sigjmp_buf, 1) != 0)
	{
		/* Since not using PG_TRY, must reset error stack by hand */
		if(fd > 0)
			close(fd);

		error_context_stack = NULL;
		
		EmitErrorReport();
		MemoryContextSwitchTo(TopMemoryContext);
		FlushErrorState();
	}
	
	/* We can now handle ereport(ERROR) */
	PG_exception_stack = &local_sigjmp_buf;

	/* watchdog child loop */
	for(;;)
	{
		int fd_max, select_ret;
		bool timeout_event = false;

		MemoryContextSwitchTo(ProcessLoopContext);
		//MemoryContextResetAndDeleteChildren(ProcessLoopContext);

		
		fd_max = prepare_fds(&rmask,&wmask,&emask);
		
		select_ret = select(fd_max + 1, &rmask, &wmask, &emask, &tv);

		gettimeofday(&ref_time,NULL);

		if (g_timeout_sec > 0 )
		{
			if (WD_TIME_DIFF_SEC(ref_time,g_tm_set_time) >=  g_timeout_sec)
			{
				timeout_event = true;
				g_timeout_sec = 0;
			}
		}

		if (select_ret < 0 ) /* Error in select. Call select again */
		{
			if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK )
			{
				if (timeout_event)
					watchdog_state_machine(WD_EVENT_TIMEOUT, NULL, NULL);
				continue;
			}
			if (timeout_event)
				watchdog_state_machine(WD_EVENT_TIMEOUT, NULL, NULL);
			continue;
		}
		else if ( select_ret == 0 ) /* timeout*/
		{
			if (timeout_event)
				watchdog_state_machine(WD_EVENT_TIMEOUT, NULL, NULL);
			continue;
		}
		else
		{
			int processed_fds = 0;
			processed_fds += accept_incomming_connections(&rmask, (select_ret - processed_fds));
			processed_fds += update_successful_outgoing_cons(&wmask,(select_ret - processed_fds));
			processed_fds += read_sockets(&rmask,(select_ret - processed_fds));
			if (timeout_event)
				watchdog_state_machine(WD_EVENT_TIMEOUT, NULL, NULL);
		}
		
	}
	return pid;
}

static int
wd_create_command_server_socket(void)
{
	size_t	len = 0;
	struct sockaddr_un addr;
	int sock = -1;
	
	/* We use unix domain stream sockets for the purpose */
	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		/* socket create failed */
		ereport(ERROR,
				(errmsg("failed to create watchdog command server socket"),
				 errdetail("create socket failed with reason: \"%s\"", strerror(errno))));
	}
	/*
	if ( fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
	{
		close(sock);
		ereport(ERROR,
				(errmsg("failed to create watchdog command server socket"),
				 errdetail("setting non blocking mode on socket failed with reason: \"%s\"", strerror(errno))));
	}
	if ( setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &one, sizeof(one)) == -1 )
	{
		close(sock);
		ereport(ERROR,
				(errmsg("failed to create watchdog command server socket"),
				 errdetail("setsockopt(SO_KEEPALIVE) failed with reason: \"%s\"", strerror(errno))));
	}
	 */
	memset((char *) &addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path),"%s",watchdog_ipc_address);
	len = sizeof(struct sockaddr_un);

	if ( bind(sock, (struct sockaddr *) &addr, len) == -1)
	{
		close(sock);
		unlink(addr.sun_path);
		ereport(ERROR,
			(errmsg("failed to create watchdog command server socket"),
				 errdetail("bind on \"%s\" failed with reason: \"%s\"", addr.sun_path, strerror(errno))));
	}

	if ( listen(sock, 5) < 0 )
	{
		/* listen failed */
		close(sock);
		unlink(addr.sun_path);
		ereport(ERROR,
			(errmsg("failed to create watchdog command server socket"),
				 errdetail("listen failed with reason: \"%s\"", strerror(errno))));
	}
	on_proc_exit(FileUnlink, (Datum) pstrdup(addr.sun_path));
	return sock;
}

static void FileUnlink(int code, Datum path)
{
	char* filePath = (char*)path;
	unlink(filePath);
}


/*
 * sets all the valid watchdog cluster descriptors to the fd_set.
 returns the fd_max */
static int
prepare_fds(fd_set* rmask, fd_set* wmask, fd_set* emask)
{
	int i;
	ListCell *lc;
	int fd_max = g_cluster.localNode->server_sock;
	
	FD_ZERO(rmask);
	FD_ZERO(wmask);
	FD_ZERO(emask);

	/* local node server socket will set the read and exception fds */
	FD_SET(g_cluster.localNode->server_sock,rmask);
	FD_SET(g_cluster.localNode->server_sock,emask);

	/* command server socket will set the read and exception fds */
	FD_SET(g_cluster.command_server_sock,rmask);
	FD_SET(g_cluster.command_server_sock,emask);
	if (fd_max < g_cluster.command_server_sock)
		fd_max = g_cluster.command_server_sock;

	/*
	 * set write fdset for all waiting for connection sockets,
	 * while already connected will be only be waiting for read
	 */
	for (i=0; i< g_cluster.remoteNodeCount; i++)
	{
		WatchdogNode* wdNode = &(g_cluster.remoteNodes[i]);
		if (wdNode->client_sock > 0)
		{
			if (fd_max < wdNode->client_sock)
				fd_max = wdNode->client_sock;

			FD_SET(wdNode->client_sock,emask);

			if (wdNode->client_sock_state == WD_SOCK_WAITING_FOR_CONNECT)
				FD_SET(wdNode->client_sock,wmask);
			else
				FD_SET(wdNode->client_sock,rmask);
		}
		if (wdNode->server_sock > 0)
		{
			if (fd_max < wdNode->server_sock)
				fd_max = wdNode->server_sock;

			FD_SET(wdNode->server_sock,emask);
			FD_SET(wdNode->server_sock,rmask);
		}
	}
	/*
	 * I know this is getting complex but we need to add all incomming unassigned connection sockets
	 * these one will go for reading
	 */
	foreach(lc, g_cluster.unidentified_socks)
	{
		int ui_sock = lfirst_int(lc);
		if (ui_sock > 0)
		{
			FD_SET(ui_sock,rmask);
			FD_SET(ui_sock,emask);
			if (fd_max < ui_sock)
				fd_max = ui_sock;
		}
	}

	/* Add the notification connected clients */
	foreach(lc, g_cluster.notify_clients)
	{
		int ui_sock = lfirst_int(lc);
		if (ui_sock > 0)
		{
			FD_SET(ui_sock,rmask);
			FD_SET(ui_sock,emask);
			if (fd_max < ui_sock)
				fd_max = ui_sock;
		}
	}

	/* Finally Add the command IPC sockets */
	foreach(lc, g_cluster.ipc_command_socks)
	{
		int ui_sock = lfirst_int(lc);
		if (ui_sock > 0)
		{
			FD_SET(ui_sock,rmask);
			FD_SET(ui_sock,emask);
			if (fd_max < ui_sock)
				fd_max = ui_sock;
		}
	}

	return fd_max;
}

static int read_sockets(fd_set* rmask,int pending_fds_count)
{
	int i,count = 0;
	List* socks_to_del = NIL;
	ListCell *lc;

	for (i=0; i< g_cluster.remoteNodeCount; i++)
	{
		WatchdogNode* wdNode = &(g_cluster.remoteNodes[i]);

		if (wdNode->client_sock > 0 && wdNode->client_sock_state == WD_SOCK_CONNECTED)
		{
			if ( FD_ISSET(wdNode->client_sock, rmask) )
			{
				ereport(LOG,
						(errmsg("client socket %d of %s is ready for reading",wdNode->client_sock, wdNode->nodeName)));

				WDPacketData* pkt = read_packet(wdNode->client_sock);

				if (pkt == NULL)
				{
					close(wdNode->client_sock);
					wdNode->client_sock = -1;
					wdNode->client_sock_state = WD_SOCK_UNINITIALIZED;
				}
				else
				{
					watchdog_state_machine(WD_EVENT_PACKET_RCV, wdNode, pkt);
				}
				count++;
				if (count >= pending_fds_count)
					return count;
			}
		}
		if (wdNode->server_sock > 0)
		{
			if ( FD_ISSET(wdNode->server_sock, rmask) )
			{
				ereport(LOG,
						(errmsg("server socket %d of %s is ready for reading",wdNode->server_sock, wdNode->nodeName)));
				WDPacketData* pkt = read_packet(wdNode->server_sock);
				if (pkt == NULL)
				{
					close(wdNode->server_sock);
					wdNode->server_sock = -1;
					wdNode->server_sock_state = WD_SOCK_UNINITIALIZED;
				}
				else
				{
					watchdog_state_machine(WD_EVENT_PACKET_RCV, wdNode, pkt);
				}

				count++;
				if (count >= pending_fds_count)
					return count;
			}
		}
	}

	foreach(lc, g_cluster.unidentified_socks)
	{
		int ui_sock = lfirst_int(lc);
		if (ui_sock > 0 &&  FD_ISSET(ui_sock, rmask))
		{
			WDPacketData* pkt;
			ereport(LOG,
					(errmsg("un-identified socket %d is ready for reading",ui_sock)));
			/* we only entertain ADD NODE messages from unidentified sockets */
			pkt = read_packet_of_type(ui_sock,WD_ADD_NODE_MESSAGE);
			if (pkt == NULL)
			{
				close(ui_sock);
			}
			else
			{
				WatchdogNode* tempNode = parse_node_info_message(pkt);
				if (tempNode)
				{
					WatchdogNode* wdNode;
					bool found = false;
					ereport(DEBUG1,
							(errmsg("NODE ADD MESSAGE from Hostname:\"%s\" PORT:%d pgpool_port:%d",tempNode->hostname,tempNode->wd_port,tempNode->pgpool_port)));
					/* verify this node */
					for (i=0; i< g_cluster.remoteNodeCount; i++)
					{
						wdNode = &(g_cluster.remoteNodes[i]);
						ereport(DEBUG1,
								(errmsg("Comparing with NODE having Hostname:\"%s\" PORT:%d pgpool_port:%d",wdNode->hostname,wdNode->wd_port,wdNode->pgpool_port)));

						if (wdNode->server_sock_state == WD_SOCK_UNINITIALIZED &&
							wdNode->wd_port == tempNode->wd_port &&
							wdNode->pgpool_port == tempNode->pgpool_port)
							/*&&
							(strcmp(wdNode->hostname,tempNode->hostname) == 0))*/
						{
							/* We have found the match */
							found = true;
							strlcpy(wdNode->delegate_ip, tempNode->delegate_ip, WD_MAX_HOST_NAMELEN);
							strlcpy(wdNode->nodeName, tempNode->nodeName, WD_MAX_HOST_NAMELEN);
							wdNode->state = tempNode->state;
							wdNode->server_sock_state = WD_SOCK_CONNECTED;
							wdNode->server_sock = ui_sock;
							break;
						}
					}
					if (found)
					{
						/* reply with node info message */
						ereport(NOTICE,
								(errmsg("New node joined the cluster Hostname:\"%s\" PORT:%d pgpool_port:%d",tempNode->hostname,tempNode->wd_port,tempNode->pgpool_port)));

						watchdog_state_machine(WD_EVENT_PACKET_RCV, wdNode, pkt);

					}
					else
					{
						/* reply with reject message, We do not need to go to state processor */
						/* For now, create a empty temp node. TODO*/
						WatchdogNode tmpNode;
						tmpNode.client_sock = ui_sock;
						tmpNode.client_sock_state = WD_SOCK_CONNECTED;
						tmpNode.server_sock = -1;
						tmpNode.server_sock_state = WD_SOCK_UNINITIALIZED;
						reply_with_minimal_message(&tmpNode, WD_REJECT_MESSAGE, pkt);
						ereport(NOTICE,
								(errmsg("NODE ADD Message rejected Hostname:\"%s\" PORT:%d pgpool_port:%d",tempNode->hostname,tempNode->wd_port,tempNode->pgpool_port)));

						close(ui_sock);
					}
					pfree(tempNode);
					free_packet(pkt);
				}
//				watchdog_state_machine(WD_EVENT_PACKET_RCV, p, packet);
				count++;
			}
			socks_to_del = lappend_int(socks_to_del,ui_sock);
			count++;
			if (count >= pending_fds_count)
				break;
		}
	}

	/* delete all the sockets from unidentified list which are now identified */
	foreach(lc, socks_to_del)
	{
		g_cluster.unidentified_socks = list_delete_int(g_cluster.unidentified_socks,lfirst_int(lc));
	}

//	list_free(socks_to_del);
	socks_to_del = NULL;
	
	if (count >= pending_fds_count)
		return count;


	foreach(lc, g_cluster.ipc_command_socks)
	{
		int command_sock = lfirst_int(lc);
		if (command_sock > 0 &&  FD_ISSET(command_sock, rmask))
		{
			bool remove_sock = false;
			read_ipc_command_and_process(command_sock, &remove_sock);
			if (remove_sock)
			{
				close(command_sock);
				socks_to_del = lappend_int(socks_to_del,command_sock);
			}
			count++;
			if (count >= pending_fds_count)
				break;
		}
	}
	/* delete all the sockets from unidentified list which are now identified */
	foreach(lc, socks_to_del)
	{
		g_cluster.ipc_command_socks = list_delete_int(g_cluster.ipc_command_socks,lfirst_int(lc));
	}
	
	list_free(socks_to_del);
	socks_to_del = NULL;

	if (count >= pending_fds_count)
		return count;
	
	foreach(lc, g_cluster.notify_clients)
	{
		int notify_sock = lfirst_int(lc);
		if (notify_sock > 0 &&  FD_ISSET(notify_sock, rmask))
		{
			bool remove_sock = false;
			read_ipc_command_and_process(notify_sock, &remove_sock);
			if (remove_sock)
			{
				close(notify_sock);
				socks_to_del = lappend_int(socks_to_del,notify_sock);
			}
			count++;
			if (count >= pending_fds_count)
				break;
		}
	}
	/* delete all the sockets from unidentified list which are now identified */
	foreach(lc, socks_to_del)
	{
		g_cluster.notify_clients = list_delete_int(g_cluster.notify_clients,lfirst_int(lc));
	}

	list_free(socks_to_del);
	socks_to_del = NULL;

	return count;
}

static bool read_ipc_command_and_process(int socket, bool *remove_socket)
{
	char type;
	WD_COMMAND_ACTIONS command_action;
	int data_len,ret;
	*remove_socket = false;
	/* 1st byte is command type */
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	ret = read(socket, &type, sizeof(char));
	if (ret != sizeof(char))
	{
		ereport(WARNING,
			(errmsg("error reading from IPC socket"),
				 errdetail("read from socket failed with error \"%s\"",strerror(errno))));
		*remove_socket = true;
		close(socket);
		return false;
	}
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	/* Next is is command action */
	ret = read(socket, &command_action, sizeof(WD_COMMAND_ACTIONS));
	if (ret != sizeof(WD_COMMAND_ACTIONS))
	{
		ereport(WARNING,
			(errmsg("error reading from IPC socket"),
				 errdetail("read from socket failed with error \"%s\"",strerror(errno))));
		*remove_socket = true;
		close(socket);
		return false;
	}
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	/* We should have data length */
	ret = read(socket, &data_len, sizeof(int));
	if (ret != sizeof(int))
	{
		ereport(WARNING,
			(errmsg("error reading from IPC socket"),
				 errdetail("read from socket failed with error \"%s\"",strerror(errno))));
		*remove_socket = true;
		close(socket);
		return false;
	}
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	data_len = ntohl(data_len);
	/* see if we have enough information to process this command */
	if (data_len == 0)
	{
		if (type == WD_REGISTER_FOR_NOTIFICATION)
		{
			/* Add this socket to the notify socket list*/
			g_cluster.notify_clients = lappend_int(g_cluster.notify_clients,socket);
			/* remove this socket from the command socket lists */
			*remove_socket = true;
			return true;
		}
		if (type == WD_GET_NODES_LIST_COMMAND)
		{
			printf("%s:%d\n",__FUNCTION__,__LINE__);

			/* get the json for node list */
			JsonNode* jNode = get_node_list_json();
			int len = jw_get_json_length(jNode);
			int nwlen = htonl(len);
			char type = WD_NODES_LIST_DATA;
			int ret = write(socket, &type, 1);
			if (ret < 1)
				return false;
			/* write data length */
			ret = write(socket, &nwlen, 4);
			if (ret < 4)
				return false;
			/* write json data */
			ret = write(socket, jw_get_json_string(jNode), len);
			if (ret < len)
				return false;
			/* good */
			return true;
			
		}
		/* currently we do not know any other 0 length packet from IPC */
		ereport(LOG,
				(errmsg("invalid IPC command type %c",type)));
		*remove_socket = true;
		return false;
	}

	MemoryContext mCxt, oldCxt;
	mCxt = AllocSetContextCreate(TopMemoryContext,
															"WDIPCCommand",
															ALLOCSET_SMALL_MINSIZE,
															ALLOCSET_SMALL_INITSIZE,
															ALLOCSET_SMALL_MAXSIZE);
	oldCxt = MemoryContextSwitchTo(mCxt);

	WDIPCCommandData* IPCCommand = palloc0(sizeof(WDIPCCommandData));
	IPCCommand->data_buf = palloc(data_len);
	IPCCommand->data_len = 0;
	IPCCommand->memoryContext = mCxt;

	MemoryContextSwitchTo(oldCxt);

	while (IPCCommand->data_len < data_len)
	{
		int ret = read(socket, IPCCommand->data_buf + IPCCommand->data_len, (data_len - IPCCommand->data_len));
		if(ret <= 0)
		{
			*remove_socket = true;
			ereport(NOTICE,
					(errmsg("error reading IPC from socket"),
					 errdetail("read from socket failed with error \"%s\"",strerror(errno))));
			close(socket);
			pfree(IPCCommand->data_buf);
			pfree(IPCCommand);
			return false;
		}
		IPCCommand->data_len +=ret;
		IPCCommand->issueing_sock = socket;
		IPCCommand->command_action = command_action;
		IPCCommand->type = type;
		gettimeofday(&IPCCommand->issue_time, NULL);
	}
	if (type == WD_TRANSPORT_DATA_COMMAND)
	{
		/* okay we have the complete command from IPC, process it */
		ret = process_IPC_transport_command(IPCCommand);
		return false;
	}
	else if (type == WD_NODE_STATUS_CHANGE_COMMAND)
	{
		ret = process_IPC_nodeStatusChange_command(IPCCommand);
	}
	else
		ret = -1;
	
	/* Okay this is the transport data command */
	if (ret < 0)
	{
		*remove_socket = true;
		ereport(NOTICE,
				(errmsg("error processing IPC from socket")));
		close(socket);
		pfree(IPCCommand->data_buf);
		pfree(IPCCommand);
		return false;
	}
	if (ret == 1)
	{
		/* The command needs further processing. Store it in cluste command list */
		g_cluster.ipc_commands = lappend(g_cluster.ipc_commands,IPCCommand);
	}
	/*command is done and dusted*/
	return true;
}

static int process_IPC_nodeStatusChange_command(WDIPCCommandData* IPCCommand)
{
	NodeStates nodeState;
	int nodeID;
	if (parse_node_status_json(IPCCommand->data_buf, IPCCommand->data_len, &nodeID, &nodeState) ==false)
		return -1;
	if (fire_node_status_event(nodeID,nodeState) == false)
		return -1;
	return 0;
}

static bool fire_node_status_event(int nodeID, NodeStates nodeState)
{
	WatchdogNode* wdNode = NULL;

	printf("***** Firing NODE STATUS EVENT for NODE ID %d\n",nodeID);
	if (nodeID == 0) /* this is reserved for local node */
	{
		wdNode = g_cluster.localNode;
	}
	else
	{
		int i;
		for (i = 0; i < g_cluster.remoteNodeCount; i++)
		{
			if (nodeID == g_cluster.remoteNodes[i].private_id)
			{
				wdNode = &g_cluster.remoteNodes[i];
				break;
			}
		}
	}
	if (wdNode == NULL)
	{
		ereport(LOG,
				(errmsg("invalid Node id for node event")));
		return false;
	}

	switch (nodeState)
	{
		case NODE_DEAD:
		{
			printf("***** Firing NODE STATUS EVENT \tNODE(ID=%d) IS DEAD \n",nodeID);

			if (wdNode == g_cluster.localNode)
				watchdog_state_machine(WD_EVENT_LOCAL_NODE_LOST, wdNode, NULL);
			else
				watchdog_state_machine(WD_EVENT_REMOTE_NODE_LOST, wdNode, NULL);
		}
			break;
			
		case NODE_ALIVE:
			break;
  default:
			ereport(LOG,
					(errmsg("invalid Node action")));
			return false;
			break;
	}
	return true;
}

static bool parse_node_status_json(char* json_data, int data_len, int* nodeID, NodeStates* nodeState)
{
	json_value* root;
	char* ptr = NULL;
	root = json_parse(json_data,data_len);
	
	/* The root node must be object */
	if (root == NULL || root->type != json_object)
	{
		json_value_free(root);
		ereport(WARNING,
				(errmsg("unable to parse json data from node status change ipc message")));
		return false;
	}
	
	if (json_get_int_value_for_key(root, "NodeID", nodeID))
	{
		json_value_free(root);
		ereport(WARNING,
				(errmsg("invalid json data from node status change ipc message"),
				 errdetail("unable to find NodeID")));
		return false;
	}
	if (json_get_int_value_for_key(root, "NodeStatus", (int*)nodeState))
	{
		json_value_free(root);
		ereport(WARNING,
				(errmsg("invalid json data from node status change ipc message"),
				 errdetail("unable to find NodeStatus")));
		return false;
	}
	ptr = json_get_string_value_for_key(root, "Message");
	if (ptr != NULL)
	{
		ereport(LOG,
				(errmsg("received node status change ipc message"),
				 errdetail("%s",ptr)));
	}
	json_value_free(root);
	return true;
}


typedef enum NodeEvents
{
	NODE_IS_DEAD = 0,
	NODE_IS_ALIVE,
	TRUSTED_SERVER_NOT_REACHABLE
}NodeEvents;

typedef struct WDNodeAction
{
	int				node_id;
	NodeEvents		event;
	int				time_sec;
	char*			message;
}WDNodeAction;
static int process_IPC_wd_node_command(WDIPCCommandData *IPCCommand)
{
	/* The command data format in Watchdog Node commands
	 * Watchdog Node ID
	 * EVENT ID on Node
	 * Time of Event sec only
	 * Optional Message String
	 */
	/* Okay parse the data */
	const int MinimumNodeCommandDataSize = 4 + 4 + 4; /* node_id, event_id and time are compulsary */
	WDNodeAction nodeAction;
	int *ptr;
	if (IPCCommand->data_len < MinimumNodeCommandDataSize)
	{
		ereport(LOG,
				(errmsg("invalid Node command data")));
		return -1;
	}
	ptr = (int*)IPCCommand->data_buf;
	nodeAction.node_id = ntohl(ptr[0]);
	nodeAction.event = ntohl(ptr[1]);
	nodeAction.time_sec = ntohl(ptr[2]);
	/* optional message string */
	if (IPCCommand->data_len > MinimumNodeCommandDataSize)
	{
		int message_size = IPCCommand->data_len - MinimumNodeCommandDataSize;
		nodeAction.message = palloc(message_size + 1);
		memcpy(nodeAction.message, IPCCommand->data_buf + MinimumNodeCommandDataSize, message_size);
		/* put the null terminator */
		nodeAction.message[message_size] = 0;
	}
	else
		nodeAction.message = NULL;
	/* Well we got the event, process it */

	WatchdogNode* wdNode = NULL;
	if (nodeAction.node_id == 0) /* this is reserved for local node */
	{
		wdNode = g_cluster.localNode;
	}
	else
	{
		int i;
		for (i = 0; i < g_cluster.remoteNodeCount; i++)
		{
			if (nodeAction.node_id == g_cluster.remoteNodes[i].private_id)
			{
				wdNode = &g_cluster.remoteNodes[i];
				break;
			}
		}
	}
	if (wdNode == NULL)
	{
		ereport(LOG,
				(errmsg("invalid Node id for node event")));
		return -1;
	}

	switch (nodeAction.event)
	{
		case NODE_IS_DEAD:
		{
			if (wdNode == g_cluster.localNode)
				watchdog_state_machine(WD_EVENT_LOCAL_NODE_LOST, wdNode, NULL);
			else
				watchdog_state_machine(WD_EVENT_REMOTE_NODE_LOST, wdNode, NULL);
		}
			break;

		case NODE_IS_ALIVE:
			break;

		case TRUSTED_SERVER_NOT_REACHABLE:
		{
			/* This event is only valid for local node */
			if (wdNode != g_cluster.localNode)
			{
				ereport(LOG,
						(errmsg("invalid Node event for remote node")));
				return -1;
			}
			watchdog_state_machine(WD_EVENT_LOCAL_NODE_LOST, wdNode, NULL);
		}
			break;
  default:
		ereport(LOG,
				(errmsg("invalid Node action")));
			return -1;
			break;
	}
	return 0;
}
/*
 * Return value:
 * -1 in case of error
 * 0  when command is complete
 * 1  when the command is issued and needs further processing
 */
static int process_IPC_transport_command(WDIPCCommandData *IPCCommand)
{
	WatchdogNode* wdNodeToSend = NULL; /* Null effectively means, send to all nodes */
	if (IPCCommand == NULL || IPCCommand->type != WD_TRANSPORT_DATA_COMMAND)
		return -1;
	if (IPCCommand->command_action == WD_COMMAND_ACTION_LOCAL)
	{
		ereport(WARNING,(errmsg("invalid command action for watchdog IPC command")));
		return -1;
	}
	/* create the packet */
	WDPacketData wdPacket;

	set_message_type(&wdPacket, WD_PGPOOL_COMMAND);
	set_next_commandID_in_message(&wdPacket);
	put_bytes_in_message(&wdPacket, IPCCommand->data_buf , IPCCommand->data_len);
	finish_wdMessage(&wdPacket);
	wdPacket.command_id = WD_PGPOOL_COMMAND;

	IPCCommand->internal_command_id = wdPacket.command_id;

	switch (IPCCommand->command_action)
	{
	case WD_COMMAND_ACTION_DEFAULT:
		{
			/* if master exists and we are not the one, Send to master */
			if (g_cluster.masterNode != g_cluster.localNode)
				wdNodeToSend = g_cluster.masterNode;
			/* for all other case send to all nodes */
		}
			break;
		case WD_COMMAND_ACTION_SEND_MASTER:
			if (g_cluster.masterNode == NULL)
			{
				ereport(WARNING,(errmsg("failed to process watchdog IPC command, NO master node found ")));
				return -1;
			}
			if (g_cluster.masterNode != g_cluster.localNode)
				wdNodeToSend = g_cluster.masterNode;
			/* What to do if I am the master ??*/
		default:
			break;
	}
	/* Okay create the result data*/
	MemoryContext oldCxt = MemoryContextSwitchTo(IPCCommand->memoryContext);
	WDIPCCommandResult* IPCResult = palloc0(sizeof(WDIPCCommandResult));
	MemoryContextSwitchTo(oldCxt);

	IPCResult->commandSendToCount = send_message(wdNodeToSend, &wdPacket);
	/* Okay now if the send count is zero, This command is finished */
	if (IPCResult->commandSendToCount == 0)
	{
		write_IPCResult_To_Socket(IPCCommand->issueing_sock, IPCResult);
		/*
		 * This is the complete lifecycle of command.
		 * we are done with it
		 */
		return 0;
	}

	IPCCommand->command_result = IPCResult;
	return 1;
}

static int write_IPCResult_To_Socket(int sock, WDIPCCommandResult* IPCResult)
{
	int ret;
	/* update the resultSlotsCount */
	if (IPCResult->node_results)
		IPCResult->resultSlotsCount = list_length(IPCResult->node_results);
	else
		IPCResult->resultSlotsCount = 0;

	/* write the header first */
	int header_size = sizeof(char) + sizeof(int) + sizeof(int) + sizeof(int);

	ret = write(sock, IPCResult, header_size);
	if (ret < header_size)
		return ret;
	/* if we have result slots */
	if (IPCResult->resultSlotsCount)
	{
		ListCell *lc;
		
		foreach(lc, IPCResult->node_results)
		{
			WDIPCCommandNodeResultData* resultSlot = lfirst(lc);
//			ret = write(sock, resultSlot, (void*)(&resultSlot->data) - (void*)resultSlot); /* send the fixed data in slot first */
			ret = write(sock, &resultSlot->node_id, sizeof(int)); /* send the fixed data in slot first */
			ret = write(sock, resultSlot->nodeName, sizeof(resultSlot->nodeName));
			ret = write(sock, &resultSlot->data_len, sizeof(int));
			if (resultSlot->data_len)
				ret = write(sock, resultSlot->data, resultSlot->data_len); /* send the data */
		}
	}
	return ret;
}

/*  status | wd_port | pgpool_port | host_ip | delegate_ip | node_name */
static WatchdogNode* parse_node_info_message(WDPacketData* pkt)
{
	WatchdogNode* wdNode = NULL;
	int i;
	char* ptr;
	int temp;
	if (pkt == NULL || (pkt->type != WD_ADD_NODE_MESSAGE && pkt->type != WD_INFO_MESSAGE))
		return NULL;
	
	ptr = pkt->data_buf + 1 + 4 + 4; /* skip the header */
	wdNode = palloc0(sizeof(WatchdogNode));

	memcpy(&temp,ptr,4);
	wdNode->state = ntohl(temp);
	ptr+= 4;

	memcpy(&temp,ptr,4);
	wdNode->wd_port = ntohl(temp);
	ptr+= 4;
	
	memcpy(&temp,ptr,4);
	wdNode->pgpool_port = ntohl(temp);
	ptr+= 4;
	for(i =(ptr - pkt->data_buf); i < pkt->data_len; i++)
		printf("%c",pkt->data_buf[i]);
	printf("\n");
	strlcpy(wdNode->hostname, ptr, WD_MAX_HOST_NAMELEN);
	ptr += strlen(wdNode->hostname) + 1;

	strlcpy(wdNode->delegate_ip, ptr, WD_MAX_HOST_NAMELEN);
	ptr += strlen(wdNode->delegate_ip) + 1;

	strlcpy(wdNode->nodeName, ptr, WD_MAX_HOST_NAMELEN);

	printf("HOSTNAME = %s\n",wdNode->hostname);
	printf("delegate_ip = %s\n",wdNode->delegate_ip);
	printf("nodeName = %s\n",wdNode->nodeName);
	return wdNode;
}

static WDPacketData* read_packet(int sock)
{
	return read_packet_of_type(sock, WD_NO_MESSAGE);
}

static WDPacketData* read_packet_of_type(int sock, char ensure_type)
{
	char type;
	int len;
	unsigned int cmd_id;
	char* buf;
	WDPacketData* pkt = NULL;
	int ret;

	ereport(DEBUG1,
			(errmsg("** going to read packet from socket %d",sock)));

	ret = read(sock, &type, sizeof(char));
	if (ret != sizeof(char))
	{
		ereport(DEBUG1,
			(errmsg("error reading from socket, ret = %d socket = %d",ret,sock),
				 errdetail("read from socket failed with error \"%s\"",strerror(errno))));
		return NULL;
	}

	ereport(DEBUG1,
			(errmsg("PACKET TYPE %c while need packet type %c",type,ensure_type)));

	if (ensure_type != WD_NO_MESSAGE && ensure_type != type)
	{
		/* The packet type is not what we want.*/
		ereport(DEBUG1,
			(errmsg("invalid packet type. expecting %c while received %c",ensure_type,type)));
		return NULL;
	}

	ret = read(sock, &cmd_id, sizeof(int));
	if (ret != sizeof(int))
	{
		ereport(DEBUG1,
			(errmsg("error reading from socket"),
				 errdetail("read from socket failed with error \"%s\"",strerror(errno))));
		return NULL;
	}
	cmd_id = ntohl(cmd_id);

	ereport(DEBUG1,
			(errmsg("PACKET COMMAND ID %d",cmd_id)));

	ret = read(sock, &len, sizeof(int));
	if (ret != sizeof(int))
	{
		ereport(DEBUG1,
			(errmsg("error reading from socket"),
				 errdetail("read from socket failed with error \"%s\"",strerror(errno))));
		return NULL;
	}

	len = ntohl(len);
	len -=4;

	ereport(DEBUG1,
			(errmsg("PACKET DATA LENGTH %d",len)));

	pkt = get_empty_packet();
	set_message_type(pkt, type);
	set_message_commandID(pkt, cmd_id);
	pkt->data_len = len;

	buf = pkt->data_buf + 4+4+1;

	int read_len = 0;
	while (read_len < len)
	{
		ret = read(sock, buf + read_len, (len - read_len));
		if(ret <= 0)
		{
			ereport(DEBUG1,
				(errmsg("error reading from socket"),
					 errdetail("read from socket failed with error \"%s\"",strerror(errno))));
			free_packet(pkt);
			return NULL;
		}
		read_len +=ret;
	}
	return pkt;
}


/*
 * sets the state of local watchdog node, and fires an state change event
 * if the new and old state differes
 */

static int set_local_node_state(WD_STATES newState)
{
	WD_STATES oldState = g_cluster.localNode->state;
	g_cluster.localNode->state = newState;
	if (oldState != newState)
		watchdog_state_machine(WD_EVENT_WD_STATE_CHANGED, NULL, NULL);
	return 0;
}



static void
wd_child_exit(int exit_signo)
{
	sigset_t mask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	exit(0);
}

static int
wd_send_response(int sock, WdPacket * recv_pack)
{
	int rtn = WD_NG;
	WdInfo * p, *q;
	WdNodeInfo * node;
	WdLockInfo * lock;
	WdPacket send_packet;
	struct timeval tv;
	char pack_str[WD_MAX_PACKET_STRING];
	int pack_str_len;
	char hash[(MD5_PASSWD_LEN+1)*2];
	bool is_node_packet = false;

	if (recv_pack == NULL)
	{
		return rtn;
	}
	memset(&send_packet, 0, sizeof(WdPacket));
	p = &(recv_pack->wd_body.wd_info);

	/* authentication */
	if (strlen(pool_config->wd_authkey))
	{
		/* calculate hash from packet */
		pack_str_len = wd_packet_to_string(recv_pack, pack_str, sizeof(pack_str));
		wd_calc_hash(pack_str, pack_str_len, hash);

		if (strcmp(recv_pack->hash, hash))
		{
			ereport(LOG,
				(errmsg("failed sending watchdog response"),
					 errdetail("watchdog authentication failed")));
			rtn = wd_authentication_failed(sock);
			return rtn;
		}
	}

	/* set response packet no */
	switch (recv_pack->packet_no)
	{
		/* information request */
		case WD_INFO_REQ:
			p = &(recv_pack->wd_body.wd_info);
			wd_set_wd_list(p->hostname, p->pgpool_port, p->wd_port, p->delegate_ip, &(p->tv), p->status);
			send_packet.packet_no = WD_READY;
			memcpy(&(send_packet.wd_body.wd_info), WD_MYSELF, sizeof(WdInfo));
			break;

		/* add request into the watchdog list */
		case WD_ADD_REQ:
			p = &(recv_pack->wd_body.wd_info);
			if (wd_set_wd_list(p->hostname, p->pgpool_port, p->wd_port,
			                   p->delegate_ip, &(p->tv), p->status) > 0)
			{
				ereport(LOG,
					(errmsg("sending watchdog response"),
						errdetail("receive add request from %s:%d and accept it",
							   p->hostname, p->pgpool_port)));

				send_packet.packet_no = WD_ADD_ACCEPT;
			}
			else
			{
				ereport(LOG,
					(errmsg("sending watchdog response"),
						errdetail("receive add request from %s:%d and reject it",
							   p->hostname, p->pgpool_port)));
				send_packet.packet_no = WD_ADD_REJECT;
			}
			memcpy(&(send_packet.wd_body.wd_info), WD_MYSELF, sizeof(WdInfo));
			break;

		/* announce candidacy to be the new master */
		case WD_STAND_FOR_MASTER:
			p = &(recv_pack->wd_body.wd_info);
			wd_set_wd_list(p->hostname, p->pgpool_port, p->wd_port, p->delegate_ip, &(p->tv), p->status);
			/* check exist master */
			if ((q = wd_is_alive_master()) != NULL)
			{
				/* vote against the candidate */
				send_packet.packet_no = WD_MASTER_EXIST;
				memcpy(&(send_packet.wd_body.wd_info), q, sizeof(WdInfo));
				ereport(LOG,
					(errmsg("sending watchdog response"),
						errdetail("WD_STAND_FOR_MASTER received, and voting against %s:%d",
							   p->hostname, p->pgpool_port)));
			}
			else
			{
				if (WD_MYSELF->tv.tv_sec <= p->tv.tv_sec )
				{
					memcpy(&tv,&(p->tv),sizeof(struct timeval));
					tv.tv_sec += 1;
					wd_set_myself(&tv, WD_NORMAL);
				}
				/* vote for the candidate */
				send_packet.packet_no = WD_VOTE_YOU;
				memcpy(&(send_packet.wd_body.wd_info), WD_MYSELF, sizeof(WdInfo));

				ereport(LOG,
					(errmsg("sending watchdog response"),
						errdetail("WD_STAND_FOR_MASTER received, and voting for %s:%d",
							   p->hostname, p->pgpool_port)));
			}
			break;

		/* announce assumption to be the new master */
		case WD_DECLARE_NEW_MASTER:
			p = &(recv_pack->wd_body.wd_info);
			wd_set_wd_list(p->hostname, p->pgpool_port, p->wd_port, p->delegate_ip, &(p->tv), p->status);
			if (WD_MYSELF->status == WD_MASTER)
			{
				/* resign master server */
				ereport(LOG,
					(errmsg("sending watchdog response"),
						 errdetail("WD_DECLARE_NEW_MASTER received and resign master server")));

				if (strlen(pool_config->delegate_IP) != 0)
					wd_IP_down();
				wd_set_myself(NULL, WD_NORMAL);
			}
			send_packet.packet_no = WD_READY;
			memcpy(&(send_packet.wd_body.wd_info), WD_MYSELF, sizeof(WdInfo));
			break;

		/* announce to assume lock holder */
		case WD_STAND_FOR_LOCK_HOLDER:
			p = &(recv_pack->wd_body.wd_info);
			wd_set_wd_list(p->hostname, p->pgpool_port, p->wd_port, p->delegate_ip, &(p->tv), p->status);
			/* only master handles lock holder assignment */
			if (WD_MYSELF->status == WD_MASTER)
			{
				/* if lock holder exists yet */
				if (wd_get_lock_holder() != NULL)
				{
					ereport(LOG,
						(errmsg("sending watchdog response"),
							 errdetail("WD_STAND_FOR_LOCK_HOLDER received but lock holder already exists")));

					send_packet.packet_no = WD_LOCK_HOLDER_EXIST;
				}
				else
				{
					ereport(LOG,
						(errmsg("sending watchdog response"),
							 errdetail("WD_STAND_FOR_LOCK_HOLDER received it")));

					wd_set_lock_holder(p, true);
					send_packet.packet_no = WD_READY;
				}
			}
			else
			{
				send_packet.packet_no = WD_READY;
			}
			memcpy(&(send_packet.wd_body.wd_info), WD_MYSELF, sizeof(WdInfo));
			break;

		/* announce to assume lock holder */
		case WD_DECLARE_LOCK_HOLDER:
			p = &(recv_pack->wd_body.wd_info);
			wd_set_wd_list(p->hostname, p->pgpool_port, p->wd_port, p->delegate_ip, &(p->tv), p->status);
			if (WD_MYSELF->is_lock_holder)
			{
				ereport(LOG,
					(errmsg("sending watchdog response"),
						 errdetail("WD_DECLARE_LOCK_HOLDER received but lock holder already exists")));

				send_packet.packet_no = WD_LOCK_HOLDER_EXIST;
			}
			else
			{
				wd_set_lock_holder(p, true);
				send_packet.packet_no = WD_READY;
			}
			memcpy(&(send_packet.wd_body.wd_info), WD_MYSELF, sizeof(WdInfo));
			break;

		/* announce to resign lock holder */
		case WD_RESIGN_LOCK_HOLDER:
			p = &(recv_pack->wd_body.wd_info);
			wd_set_wd_list(p->hostname, p->pgpool_port, p->wd_port, p->delegate_ip, &(p->tv), p->status);
			wd_set_lock_holder(p, false);
			send_packet.packet_no = WD_READY;
			memcpy(&(send_packet.wd_body.wd_info), WD_MYSELF, sizeof(WdInfo));
			break;

		/* announce to start interlocking */
		case WD_START_INTERLOCK:
			p = &(recv_pack->wd_body.wd_info);
			wd_set_wd_list(p->hostname, p->pgpool_port, p->wd_port, p->delegate_ip, &(p->tv), p->status);
			wd_set_interlocking(p, true);
			send_packet.packet_no = WD_READY;
			memcpy(&(send_packet.wd_body.wd_info), WD_MYSELF, sizeof(WdInfo));
			break;

		/* announce to end interlocking */
		case WD_END_INTERLOCK:
			p = &(recv_pack->wd_body.wd_info);
			wd_set_wd_list(p->hostname, p->pgpool_port, p->wd_port, p->delegate_ip, &(p->tv), p->status);
			wd_set_interlocking(p, false);
			send_packet.packet_no = WD_READY;
			memcpy(&(send_packet.wd_body.wd_info), WD_MYSELF, sizeof(WdInfo));
			break;

		/* announce that server is down */
		case WD_SERVER_DOWN:
			p = &(recv_pack->wd_body.wd_info);
			wd_set_wd_list(p->hostname, p->pgpool_port, p->wd_port, p->delegate_ip, &(p->tv), WD_DOWN);
			send_packet.packet_no = WD_READY;
			memcpy(&(send_packet.wd_body.wd_info), WD_MYSELF, sizeof(WdInfo));
			if (wd_am_I_oldest() == WD_OK && WD_MYSELF->status != WD_MASTER)
			{
				wd_escalation();
			}
			break;

		/* announce start online recovery */
		case WD_START_RECOVERY:
			if (*InRecovery != RECOVERY_INIT)
			{
				send_packet.packet_no = WD_NODE_FAILED;
			}
			else
			{
				send_packet.packet_no = WD_NODE_READY;
				*InRecovery = RECOVERY_ONLINE;
				if (wait_connection_closed() != 0)
				{
					send_packet.packet_no = WD_NODE_FAILED;
				}
			}
			break;

		/* announce end online recovery */
		case WD_END_RECOVERY:
			send_packet.packet_no = WD_NODE_READY;
			*InRecovery = RECOVERY_INIT;
			kill(wd_ppid, SIGUSR2);
			break;

		/* announce failback request */
		case WD_FAILBACK_REQUEST:
			if (Req_info->switching)
			{
				ereport(LOG,
					(errmsg("sending watchdog response"),
						 errdetail("failback request from other pgpool is canceled because of switching")));
				send_packet.packet_no = WD_NODE_FAILED;
			}
			else
			{
				node = &(recv_pack->wd_body.wd_node_info);
				wd_set_node_mask(WD_FAILBACK_REQUEST,node->node_id_set,node->node_num);
				is_node_packet = true;
				send_packet.packet_no = WD_NODE_READY;
			}
			break;

		/* announce degenerate backend */
		case WD_DEGENERATE_BACKEND:
			if (Req_info->switching)
			{
				ereport(LOG,
					(errmsg("sending watchdog response"),
						 errdetail("failover request from other pgpool is canceled because of switching")));

				send_packet.packet_no = WD_NODE_FAILED;
			}
			else
			{
				node = &(recv_pack->wd_body.wd_node_info);
				wd_set_node_mask(WD_DEGENERATE_BACKEND,node->node_id_set, node->node_num);
				is_node_packet = true;
				send_packet.packet_no = WD_NODE_READY;
			}
			break;

		/* announce promote backend */
		case WD_PROMOTE_BACKEND:
			if (Req_info->switching)
			{
				ereport(LOG,
					(errmsg("sending watchdog response"),
						 errdetail("promote request from other pgpool is canceled because of switching")));

				send_packet.packet_no = WD_NODE_FAILED;
			}
			else
			{
				node = &(recv_pack->wd_body.wd_node_info);
				wd_set_node_mask(WD_PROMOTE_BACKEND,node->node_id_set, node->node_num);
				is_node_packet = true;
				send_packet.packet_no = WD_NODE_READY;
			}
			break;

		/* announce to unlock command */
		case WD_UNLOCK_REQUEST:
			lock = &(recv_pack->wd_body.wd_lock_info);
			wd_set_lock(lock->lock_id, false);
			send_packet.packet_no = WD_LOCK_READY;
			break;

		default:
			send_packet.packet_no = WD_INVALID;
			memcpy(&(send_packet.wd_body.wd_info), WD_MYSELF, sizeof(WdInfo));
			break;
	}

	/* send response packet */
	rtn = wd_send_packet(sock, &send_packet);

	/* send node request signal.
	 * wd_node_request_signal() uses a semaphore lock internally, so should be
	 * called after sending a response packet to prevent dead lock.
	 */
	if (is_node_packet)
		wd_node_request_signal(recv_pack->packet_no, node);

	return rtn;
}

/* send node request signal for other pgpool*/
static void
wd_node_request_signal(WD_PACKET_NO packet_no, WdNodeInfo *node)
{
	switch (packet_no)
	{
		case WD_FAILBACK_REQUEST:
			send_failback_request(node->node_id_set[0],false);
			break;
		case WD_DEGENERATE_BACKEND:
			degenerate_backend_set(node->node_id_set, node->node_num);
			break;
		case WD_PROMOTE_BACKEND:
			promote_backend(node->node_id_set[0]);
			break;
		default:
			ereport(WARNING,
				(errmsg("wd_node_request_signal: unknown packet number")));
			break;
	}
}

static int accept_incomming_connections(fd_set* rmask, int pending_fds_count)
{
	int processed_fds = 0;
	int fd;

	if ( FD_ISSET(g_cluster.localNode->server_sock, rmask) )
	{
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(struct sockaddr_in);
		processed_fds++;
		fd = accept(g_cluster.localNode->server_sock, (struct sockaddr *)&addr, &addrlen);
		if (fd < 0)
		{
			if ( errno == EINTR || errno == 0 || errno == EAGAIN || errno == EWOULDBLOCK )
			{
				/* nothing to accept now */
				ereport(DEBUG2,
						(errmsg("Failed to accept incoming watchdog connection, Nothing to accept")));
			}
			/* accept failed */
			ereport(DEBUG1,
					(errmsg("Failed to accept incomming watchdog connection")));
		}
		else
		{
			ereport(LOG,
					(errmsg("new watchdog node connection is received from \"%s:%d\"",inet_ntoa(addr.sin_addr),addr.sin_port)));
			g_cluster.unidentified_socks = lappend_int(g_cluster.unidentified_socks, fd);
		}
	}

	if (processed_fds >= pending_fds_count)
		return processed_fds;

	if ( FD_ISSET(g_cluster.command_server_sock, rmask) )
	{
		struct sockaddr addr;
		socklen_t addrlen = sizeof(struct sockaddr);
		processed_fds++;
		
		int fd = accept(g_cluster.command_server_sock, &addr, &addrlen);
		if (fd < 0)
		{
			if ( errno == EINTR || errno == 0 || errno == EAGAIN || errno == EWOULDBLOCK )
			{
				/* nothing to accept now */
				ereport(WARNING,
						(errmsg("Failed to accept incoming watchdog IPC connection, Nothing to accept")));
			}
			/* accept failed */
			ereport(WARNING,
					(errmsg("Failed to accept incoming watchdog IPC connection")));
		}
		else
		{
			ereport(LOG,
					(errmsg("new IPC connection is received ")));
			g_cluster.ipc_command_socks = lappend_int(g_cluster.ipc_command_socks, fd);
		}
	}

	return processed_fds;
}

static int update_successful_outgoing_cons(fd_set* wmask, int pending_fds_count)
{
	int i;
	int count = 0;
	WDPacketData *pkt = NULL;

	for (i=0; i< g_cluster.remoteNodeCount; i++)
	{
		WatchdogNode* wdNode = &(g_cluster.remoteNodes[i]);

		if (wdNode->client_sock > 0 && wdNode->client_sock_state == WD_SOCK_WAITING_FOR_CONNECT)
		{
			if ( FD_ISSET(wdNode->client_sock, wmask) )
			{
				socklen_t lon;
				int valopt;
				lon = sizeof(int);
				getsockopt(wdNode->client_sock, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon);
				if (valopt)
				{
					ereport(LOG,
							(errmsg("error in outbond connection to %s:%d",wdNode->hostname,wdNode->wd_port),
							 errdetail("%s",strerror(valopt))));
					close(wdNode->client_sock);
					wdNode->client_sock = -1;
					wdNode->client_sock_state = WD_SOCK_ERROR;
				}
				else
				{
					wdNode->client_sock_state = WD_SOCK_CONNECTED;
					ereport(LOG,
							(errmsg("new outbond connection to %s:%d ",wdNode->hostname,wdNode->wd_port)));

					/* set socket to blocking again */
					int flags = fcntl(wdNode->client_sock, F_GETFL, 0);
					fcntl(wdNode->client_sock, F_SETFL, flags | ~O_NONBLOCK);
					/* Send the Add Node message */

//					if (!pkt)
//						pkt = get_addnode_message();
//					if (write_packet_to_socket(wdNode->client_sock, pkt) == false)
//					{
//						ereport(LOG,
//							(errmsg("write failed on socket to host %s:%d",wdNode->hostname,wdNode->wd_port),
//								 errdetail("%s",strerror(errno))));
//						/* What to do? close the connection seems a reasonable option. Any other thoughts??*/
//						close(wdNode->client_sock);
//						wdNode->client_sock = -1;
//						wdNode->client_sock_state = WD_SOCK_ERROR;
//					}
//					else
						watchdog_state_machine(WD_EVENT_NEW_OUTBOUND_CONNECTION, wdNode, NULL);

				}
				count++;
				if (count >= pending_fds_count)
					break;
			}
		}
	}
	if (pkt)
		free_packet(pkt);
	return count;
}

static bool write_packet_to_socket(int sock, WDPacketData* pkt)
{
	int ret = 0;
	ereport(LOG,
			(errmsg("sending watchdog packet Socket:%d, Type:%c, Command_ID:%d, data Length:%d",sock,pkt->type, pkt->command_id,pkt->ptr)));
	do
	{
		ret = write(sock, pkt->data_buf +ret, (pkt->ptr - ret));
		if (ret <=0)
			return false;
	}while (ret < pkt->ptr);
	return true;
}

static WDPacketData* get_empty_packet(void)
{
	WDPacketData *pkt = palloc0(sizeof(WDPacketData));
	pkt->data_len = 0;
	/* set the ptr to the start of packet data(skip header) */
	pkt->ptr = 1+4+4; /* type(1) + len(4) + command_id(4) */
	return pkt;
}

static void free_packet(WDPacketData *pkt)
{
	if (pkt)
	{
		pfree(pkt);
	}
}

static void set_message_type(WDPacketData* pkt, char type)
{
	pkt->type = type;
	pkt->data_buf[0] = type;
}

static void set_message_commandID(WDPacketData* pkt, unsigned int commandID)
{
	unsigned int cmd_id = htonl(commandID);
	/* command_id sits after type*/
	memcpy(pkt->data_buf + 1, &cmd_id,4);
	pkt->command_id = commandID;
}

static void set_next_commandID_in_message(WDPacketData* pkt)
{
	set_message_commandID(pkt,get_next_commandID());
}

static void update_message_length(WDPacketData* pkt)
{
	unsigned int len = pkt->ptr - 1 - 4; /* Message type and commandId is not included in the length */;
	len = htonl(len);
	/* message length sits after type and command id */
	memcpy(pkt->data_buf + 1 + 4, &len,4);
}

static bool put_bytes_in_message(WDPacketData* pkt, const char* value, int len)
{
	if (pkt->ptr + len >= MAX_PACKET_SIZE)
		return false;
	memcpy(pkt->data_buf + pkt->ptr, value,len);
	pkt->ptr += len;
	return true;
}

static bool put_int_in_message(WDPacketData* pkt, int value)
{
	int tmp4 = htonl(value);
	return put_bytes_in_message(pkt, (const char *) &tmp4, 4);
}

static void finish_wdMessage(WDPacketData* pkt)
{
	update_message_length(pkt);
}



/* status | wd_port | pgpool_port | host_ip | delegate_ip | node_name */

static WDPacketData* fill_myinfo_in_message(WDPacketData* message)
{
	put_int_in_message(message, g_cluster.localNode->state);
	put_int_in_message(message, g_cluster.localNode->wd_port);
	put_int_in_message(message, g_cluster.localNode->pgpool_port);
	put_bytes_in_message(message,g_cluster.localNode->hostname, strlen(g_cluster.localNode->hostname) +1);
	put_bytes_in_message(message,g_cluster.localNode->delegate_ip, strlen(g_cluster.localNode->delegate_ip) +1);
	put_bytes_in_message(message, g_cluster.localNode->nodeName , strlen(g_cluster.localNode->nodeName) +1);
	finish_wdMessage(message);
	return message;
}

static bool add_nodeinfo_to_json(JsonNode* jNode, WatchdogNode* node)
{
	jw_start_object(jNode, "WatchdogNode");

	jw_put_int(jNode, "ID", node->private_id);
	jw_put_string(jNode, "NodeName", node->nodeName);
	jw_put_string(jNode, "HostName", node->hostname);
	jw_put_int(jNode, "WdPort", node->wd_port);
	jw_put_int(jNode, "PgpoolPort", node->pgpool_port);

	jw_end_element(jNode);
	
	return true;
}

static JsonNode* get_node_list_json(void)
{
	int i;
	JsonNode* jNode = jw_create_with_object(true);
	/* add the node count */
	jw_put_int(jNode, "NodeCount", g_cluster.remoteNodeCount + 1);
	/* add the array */
	jw_start_array(jNode, "WatchdogNodes");
	/* add the local node info */
	add_nodeinfo_to_json(jNode,g_cluster.localNode);
	/* add remote nodes */
	for (i=0; i< g_cluster.remoteNodeCount; i++)
	{
		WatchdogNode* wdNode = &(g_cluster.remoteNodes[i]);
		add_nodeinfo_to_json(jNode, wdNode);
	}
	jw_finish_document(jNode);
	return jNode;
}

static WDPacketData* get_addnode_message(void)
{
	WDPacketData *message = get_empty_packet();

	set_message_type(message, WD_ADD_NODE_MESSAGE);
	set_next_commandID_in_message(message);

	fill_myinfo_in_message(message);
	return message;
}

static WDPacketData* get_mynode_info_message(WDPacketData* replyFor)
{
	WDPacketData *message = get_empty_packet();
	
	set_message_type(message, WD_INFO_MESSAGE);
	if (replyFor == NULL)
		set_next_commandID_in_message(message);
	else
		set_message_commandID(message, replyFor->command_id);
	fill_myinfo_in_message(message);
	return message;
}

static WDPacketData* get_minimum_message(char type, WDPacketData* replyFor)
{
	/* TODO it is a waste of space */
	WDPacketData *message = get_empty_packet();
	set_message_type(message,type);
	if (replyFor == NULL)
		set_next_commandID_in_message(message);
	else
		set_message_commandID(message, replyFor->command_id);
	finish_wdMessage(message);
	return message;
}


static bool packet_is_received_for_pgpool_command(WatchdogNode* wdNode, WDPacketData* pkt)
{
	ListCell *lc;

	foreach(lc, g_cluster.ipc_commands)
	{
		WDIPCCommandData* ipcCommand = lfirst(lc);
		if (ipcCommand)
		{
			if (ipcCommand->internal_command_id == pkt->command_id)
			{
				/* Okay we have found the command we are looking for */
				WDIPCCommandResult* ipcResult = ipcCommand->command_result;
				if (ipcResult != NULL) /* Assert it should never be NULL */
				{
					ipcResult->commandReplyFromCount++;
					/* create the result slot */
					WDIPCCommandNodeResultData* resultSlot = palloc(sizeof(WDIPCCommandNodeResultData));
					strncpy(resultSlot->nodeName, wdNode->nodeName,WD_MAX_HOST_NAMELEN);
					resultSlot->node_id = wdNode->private_id;
					resultSlot->data_len = pkt->data_len;

					if (resultSlot->data_len > 0)
					{
						resultSlot->data = palloc(resultSlot->data_len);
						memcpy(resultSlot->data, pkt->data_buf, resultSlot->data_len);
					}
					else
						resultSlot->data = NULL;

					ipcResult->node_results = lappend(ipcResult->node_results,resultSlot);
	
					if (ipcResult->commandReplyFromCount >= ipcResult->commandSendToCount)
					{
						/* if all nodes nodes have replied, finish the command */
						write_IPCResult_To_Socket(ipcCommand->issueing_sock, ipcResult);
						/* remove this command from list */
						g_cluster.ipc_commands = list_delete_ptr(g_cluster.ipc_commands,ipcCommand);
						free_IPC_command(ipcCommand);
					}
					return true;
				}
				/*TODO handle this case, or do we can really get this situation?? */
				break;
			}
		}
	}
	return false;
}

static void free_IPC_command(WDIPCCommandData* ipcCommand)
{
	/*
	 * Everything of IPCCommand live inside its own memory context.
	 * Delete the MemoryContext and we are good
	 */
	MemoryContextDelete(ipcCommand->memoryContext);
}

static int standard_packet_processor(WatchdogNode* wdNode, WDPacketData* pkt)
{
	WDPacketData* replyPkt = NULL;
	switch (pkt->type)
	{
		case WD_ADD_NODE_MESSAGE:
		case WD_REQ_INFO_MESSAGE:
			replyPkt = get_mynode_info_message(pkt);
			break;
			
		case WD_INFO_MESSAGE:
		{
			WatchdogNode* tempNodeInfo = parse_node_info_message(pkt);
			wdNode->state = tempNodeInfo->state;
			if (wdNode->state == WD_COORDINATOR)
			{
				/* TODO check if we already have the coordinator */
				if (g_cluster.masterNode != NULL && g_cluster.masterNode != wdNode)
				{
					ereport(WARNING,(errmsg("WE already have the coordinator...")));
					/* What should be the best way to handle it */
				}
				g_cluster.masterNode = wdNode;
			}
			pfree(tempNodeInfo);
		}
			break;
		case WD_PGPOOL_COMMAND:
			packet_is_received_for_pgpool_command(wdNode, pkt);
			break;
		case WD_JOIN_COORDINATOR_MESSAGE:
		{
			/*
			 * if I am coordinator reply with accept,
			 * otherwise reject
			 */
			if (g_cluster.localNode == g_cluster.masterNode)
				reply_with_minimal_message(wdNode, WD_ACCEPT_MESSAGE, pkt);
			else
				reply_with_minimal_message(wdNode, WD_REJECT_MESSAGE, pkt);
		}

		case WD_IAM_COORDINATOR_MESSAGE:
		{
			/*
			 * if the message is received from coordinator reply with infor,
			 * otherwise reject
			 */
			if (g_cluster.masterNode != NULL && wdNode != g_cluster.masterNode)
			{
				ereport(NOTICE,
						(errmsg("cluster is in split brain")));
				reply_with_minimal_message(wdNode, WD_ERROR_MESSAGE, pkt);
			}
			else
				replyPkt = get_mynode_info_message(pkt);
		}
			break;

		default:
			break;
	}
	if (replyPkt)
	{
		send_message_to_node(wdNode,replyPkt);
		pfree(replyPkt);
	}
	return 1;
}

static bool send_message_to_node(WatchdogNode* wdNode, WDPacketData *pkt)
{
	if (wdNode->client_sock > 0 && wdNode->client_sock_state == WD_SOCK_CONNECTED)
	{
		if (write_packet_to_socket(wdNode->client_sock, pkt) == false)
		{
			close(wdNode->client_sock);
			wdNode->client_sock = -1;
			wdNode->client_sock_state = WD_SOCK_ERROR;
		}
		else
		{
			return true;
		}
	}
	if (wdNode->server_sock > 0 && wdNode->server_sock_state == WD_SOCK_CONNECTED)
	{
		if (write_packet_to_socket(wdNode->server_sock, pkt) == false)
		{
			close(wdNode->server_sock);
			wdNode->server_sock = -1;
			wdNode->server_sock_state = WD_SOCK_ERROR;
		}
		else
		{
			return true;
		}
	}
	return false;
}

/*
 * If wdNode is NULL message is sent to all nodes
 * Returns the number of nodes the message is sent to
 */
static int send_message(WatchdogNode* wdNode, WDPacketData *pkt)
{
	int i,count = 0;
	if (wdNode)
	{
		if (send_message_to_node(wdNode,pkt) == 0)
			return 1;
		return 0;
	}
	for (i=0; i< g_cluster.remoteNodeCount; i++)
	{
		wdNode = &(g_cluster.remoteNodes[i]);
		if (send_message_to_node(wdNode,pkt) == 0)
			count++;
	}
	return count;
}

/*
 * If wdNode is NULL message is sent to all nodes
 * Returns the number of nodes the message is sent to
 */
static int send_cluster_command_packet(WatchdogNode* wdNode, WDPacketData *pkt, int timeout_sec)
{
	int count = send_message(wdNode,pkt);
	if (count > 0)
	{
		g_cluster.lastCommand.commandID = pkt->command_id;
		g_cluster.lastCommand.commandMessageType = pkt->type;
		g_cluster.lastCommand.commandSendToCount = count;
		g_cluster.lastCommand.commandReplyFromCount = 0;
		g_cluster.lastCommand.commandTimeoutSec = timeout_sec;
		g_cluster.lastCommand.commandFinished = 0;
		gettimeofday(&g_cluster.lastCommand.commandTime,NULL);
	}
	return count;
}

static int update_connected_node_count(void)
{
	int i;
	g_cluster.aliveNodeCount = 0;
	for (i = 0; i< g_cluster.remoteNodeCount; i++)
	{
		WatchdogNode* wdNode = &(g_cluster.remoteNodes[i]);
		if (wdNode->state == WD_DEAD)
			continue;
		if (wdNode->client_sock > 0 && wdNode->client_sock_state == WD_SOCK_CONNECTED)
			g_cluster.aliveNodeCount++;
		else if (wdNode->server_sock > 0 && wdNode->server_sock_state == WD_SOCK_CONNECTED)
			g_cluster.aliveNodeCount++;
	}
	return g_cluster.aliveNodeCount;
}

/*
 * The function only considers the node state.
 * All node states conut towards the cluster participating nodes
 * except the dead and lost nodes.
 */
static int get_cluster_node_count(void)
{
	int i;
	int count = 0;
	for (i = 0; i< g_cluster.remoteNodeCount; i++)
	{
		WatchdogNode* wdNode = &(g_cluster.remoteNodes[i]);
		if (wdNode->state == WD_DEAD || wdNode->state == WD_LOST)
			continue;
		count++;
	}
	return count;
}


static int send_cluster_command(WatchdogNode* wdNode, char type, int timeout_sec)
{
	WDPacketData *pkt = NULL;
	int ret = 0;
	switch (type)
	{
		case WD_INFO_MESSAGE:
			pkt = get_mynode_info_message(NULL);
			break;
		case WD_ADD_NODE_MESSAGE:
			pkt = get_addnode_message();
			break;

		case WD_REQ_INFO_MESSAGE:
		case WD_IAM_COORDINATOR_MESSAGE:
		case WD_STAND_FOR_COORDINATOR_MESSAGE:
		case WD_DECLARE_COORDINATOR_MESSAGE:
		case WD_JOIN_COORDINATOR_MESSAGE:
		case WD_QUORUM_IS_LOST:
			pkt = get_minimum_message(type, NULL);
			break;
		default:
			ereport(LOG,(errmsg("invalid command message type %c",type)));
			break;
	}
	if (pkt)
	{
		ret = send_cluster_command_packet(wdNode, pkt, timeout_sec);
		free_packet(pkt);
	}
	return ret;
}

static bool reply_with_minimal_message(WatchdogNode* wdNode, char type, WDPacketData* replyFor)
{
	WDPacketData *pkt = get_minimum_message(type,replyFor);
	int ret = send_message(wdNode, pkt);
	free_packet(pkt);
	return ret;
}

static inline WD_STATES get_local_node_state(void)
{
	return g_cluster.localNode->state;
}

static bool reply_is_for_last_command(WDPacketData* pkt)
{
	return pkt->command_id == g_cluster.lastCommand.commandID;
}

char *wd_event_name[] = {"WD_EVENT_WD_STATE_CHANGED",
	"WD_EVENT_CON_OPEN",
	"WD_EVENT_CON_CLOSED",
	"WD_EVENT_CON_ERROR",
	"WD_EVENT_TIMEOUT",
	"WD_EVENT_PACKET_RCV",
	"WD_EVENT_HB_MISSED",
	"WD_EVENT_NEW_OUTBOUND_CONNECTION",
	"WD_EVENT_LOCAL_NODE_LOST",
	"WD_EVENT_REMOTE_NODE_LOST"
};

char *debug_states[] = {
	"WD_DEAD",
	"WD_LOADING",
	"WD_JOINING",
	"WD_INITIALIZING",
	"WD_WAITING_CONNECT",
	"WD_COORDINATOR",
	"WD_PARTICIPATE_IN_ELECTION",
	"WD_STAND_FOR_COORDINATOR",
	"WD_STANDBY",
	"WD_WAITING_FOR_QUORUM",
	"WD_LOST"};

static int watchdog_state_machine(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt)
{
	ereport(DEBUG1,
			(errmsg("STATE MACHINE INVOKED WITH EVENT = %s Current State = %s",wd_event_name[event], debug_states[get_local_node_state()])));

	if (event == WD_EVENT_REMOTE_NODE_LOST)
	{
		wdNode->state = WD_LOST;
		if (wdNode == g_cluster.masterNode)
			g_cluster.masterNode = NULL;
	}

	switch (get_local_node_state())
	{
		case WD_LOADING:
			watchdog_state_machine_loading(event,wdNode,pkt);
			break;
		case WD_JOINING:
			watchdog_state_machine_joing(event,wdNode,pkt);
			break;
		case WD_INITIALIZING:
			watchdog_state_machine_initializing(event,wdNode,pkt);
			break;
		case WD_COORDINATOR:
			watchdog_state_machine_coordinator(event,wdNode,pkt);
			break;
		case WD_PARTICIPATE_IN_ELECTION:
			watchdog_state_machine_voting(event,wdNode,pkt);
			break;
		case WD_STAND_FOR_COORDINATOR:
			watchdog_state_machine_standForCord(event,wdNode,pkt);
			break;
		case WD_STANDBY:
			watchdog_state_machine_standby(event,wdNode,pkt);
			break;
		case WD_WAITING_FOR_QUORUM:
			watchdog_state_machine_waiting_for_quorum(event,wdNode,pkt);
			break;
		default:
			//?????
			break;
	}
	
	return 0;
}

/*
 * This is the state where the watchdog enters when starting up.
 * upon entering this state we sends ADD node message to all reachable
 * nodes.
 * Wait for 4 seconds if some node rejects us.
 */
static int watchdog_state_machine_loading(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt)
{
	switch (event)
	{
		case WD_EVENT_WD_STATE_CHANGED:
		{
			int i;
			send_cluster_command(NULL, WD_ADD_NODE_MESSAGE, 4);
			/* set the status to ADD_MESSAGE_SEND by hand */
			for (i=0; i< g_cluster.remoteNodeCount; i++)
			{
				wdNode = &(g_cluster.remoteNodes[i]);
				if (wdNode->client_sock_state == WD_SOCK_CONNECTED && wdNode->state == WD_DEAD)
					wdNode->state= WD_ADD_MESSAGE_SENT;
			}
			set_timeout(4);
		}
			break;

		case WD_EVENT_CON_OPEN:
			break;

		case WD_EVENT_NEW_OUTBOUND_CONNECTION:
		{
			/* new node is reachable. Send add node message*/
			if (wdNode->state == WD_DEAD)
			{
				send_cluster_command(wdNode, WD_ADD_NODE_MESSAGE, 4);
				if (wdNode->client_sock_state == WD_SOCK_CONNECTED)
				wdNode->state= WD_ADD_MESSAGE_SENT;
			}
		}
			break;

		case WD_EVENT_TIMEOUT:
//			send_cluster_command(NULL, WD_ADD_NODE_MESSAGE, 0);
			set_state(WD_JOINING);
			
			break;
		case WD_EVENT_PACKET_RCV:
		{
			if(pkt == NULL)
			{
				ereport(LOG,
						(errmsg("packet is NULL")));
				break;
			}
			switch (pkt->type)
			{
				case WD_INFO_MESSAGE:
					standard_packet_processor(wdNode, pkt);
					if (update_connected_node_count() == g_cluster.remoteNodeCount)
					{
						/*
						 * we are already connected to all configured nodes
						 * Just move to Joining state
						 */
						set_state(WD_JOINING);
					}

					break;
				case WD_REJECT_MESSAGE:
					ereport(FATAL,
						(errmsg("Add to watchdog cluster request is rejected by node \"%s:%d\"",wdNode->hostname,wdNode->wd_port),
							 errhint("check the watchdog configurations.")));
					break;
				default:
					standard_packet_processor(wdNode, pkt);
					break;
			}
		}
		break;
  default:
			break;
	}
	return 0;
}

/*
 * This is the intermediate state before going to cluster initialization
 * here we update the information of all connected nodes and move to the
 * initialization state. moving to this state from loading does not make
 * much sence as at loading time we already have updated node informations
 */
static int watchdog_state_machine_joing(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt)
{
	switch (event)
	{
		case WD_EVENT_WD_STATE_CHANGED:
			g_cluster.masterNode = NULL;
			send_cluster_command(NULL, WD_REQ_INFO_MESSAGE, 5);
			set_timeout(5);
			break;

		case WD_EVENT_TIMEOUT:
			set_state(WD_INITIALIZING);
			break;
			
		case WD_EVENT_PACKET_RCV:
		{
			if(pkt == NULL)
			{
				ereport(LOG,
						(errmsg("packet is NULL")));
				break;
			}
			switch (pkt->type)
			{
				case WD_INFO_MESSAGE:
					standard_packet_processor(wdNode, pkt);
					g_cluster.lastCommand.commandReplyFromCount++;
					
					if(g_cluster.lastCommand.commandReplyFromCount == g_cluster.lastCommand.commandSendToCount)
					{
						/*
						 * All nodes replied to our infor request
						 * move to the initialization
						 */
						set_state(WD_INITIALIZING);
					}
					break;
				case WD_REJECT_MESSAGE:
					if (wdNode->state == WD_ADD_MESSAGE_SENT)
						ereport(FATAL,
							(errmsg("Add to watchdog cluster request is rejected by node \"%s:%d\"",wdNode->hostname,wdNode->wd_port),
								 errhint("check the watchdog configurations.")));
					break;
				default:
					standard_packet_processor(wdNode, pkt);
					break;
			}
		}
			break;
			
  default:
			break;
	}
	return 0;
}

/*
 * This state only works on the local data and does not
 * sends any cluster command.
 */

static int watchdog_state_machine_initializing(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt)
{
	switch (event)
	{
		case WD_EVENT_WD_STATE_CHANGED:
			/* set 1 sec timeout, save ourself from recurrsion */
			set_timeout(1);
			break;

		case WD_EVENT_CON_OPEN:
			break;
			
		case WD_EVENT_TIMEOUT:
		{
			/*
			 * If master node exists in cluser, Join it
			 * otherwise try becoming a master
			 */
			if (g_cluster.masterNode)
			{
				/*
				 * we found the coordinator node in network.
				 * Just join the network
				 */
				set_state(WD_STANDBY);
			}
			else
			{
				/* check if the quorum exists */
				int quorum_status = get_quorum_status();
				if (quorum_status == -1)
				{
					ereport(LOG,
							(errmsg("We do not have enough nodes in cluster")));
					set_state(WD_WAITING_FOR_QUORUM);
				}
				else
					/* stand for coordinator */
					set_state(WD_STAND_FOR_COORDINATOR);
			}
		}
			break;
			
		case WD_EVENT_PACKET_RCV:
		{
			switch (pkt->type)
			{
				case WD_INFO_MESSAGE:
					standard_packet_processor(wdNode, pkt);
					g_cluster.lastCommand.commandReplyFromCount++;
					
					if(g_cluster.lastCommand.commandReplyFromCount == g_cluster.lastCommand.commandSendToCount)
					{
						/*
						 * All nodes replied to our infor request
						 * move to the initialization
						 */
						set_state(WD_INITIALIZING);
					}
					break;
				case WD_REJECT_MESSAGE:
					if (wdNode->state == WD_ADD_MESSAGE_SENT)
						ereport(FATAL,
							(errmsg("Add to watchdog cluster request is rejected by node \"%s:%d\"",wdNode->hostname,wdNode->wd_port),
								 errhint("check the watchdog configurations.")));
					break;
				default:
					standard_packet_processor(wdNode, pkt);
					break;
			}

		}
			
			break;
			
		default:
			break;
	}
	return 0;
}

static int watchdog_state_machine_standForCord(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt)
{
	switch (event)
	{
		case WD_EVENT_WD_STATE_CHANGED:
			send_cluster_command(NULL, WD_STAND_FOR_COORDINATOR_MESSAGE, 0);
			/* wait for 5 seconds if someone rejects us*/
			set_timeout(5);
			break;
			
		case WD_EVENT_CON_OPEN:
			break;
			
		case WD_EVENT_TIMEOUT:
			set_state(WD_COORDINATOR);
			break;
			
		case WD_EVENT_PACKET_RCV:
		{
			if(pkt == NULL)
			{
				ereport(LOG,
						(errmsg("packet is NULL")));
				break;
			}
			if (false && reply_is_for_last_command(pkt) == false)
			{
				standard_packet_processor(wdNode, pkt);
			}
			else
			{
				switch (pkt->type)
				{
					case WD_ERROR_MESSAGE:
						ereport(LOG,
								(errmsg("our stand for coordinator request is rejected by node \"%s\"",wdNode->nodeName)));
						set_state(WD_JOINING);

					case WD_REJECT_MESSAGE:
						ereport(LOG,
								(errmsg("our stand for coordinator request is rejected by node \"%s\"",wdNode->nodeName)));
						set_state(WD_PARTICIPATE_IN_ELECTION);
						break;
					case WD_STAND_FOR_COORDINATOR_MESSAGE:
						/* for now decide on base of larger node value */
						if (g_cluster.localNode->wd_port > wdNode->wd_port)
						{
							reply_with_minimal_message(wdNode, WD_REJECT_MESSAGE, pkt);
						}
						else
						{
							reply_with_minimal_message(wdNode, WD_ACCEPT_MESSAGE, pkt);
							set_state(WD_PARTICIPATE_IN_ELECTION);
						}
						break;
						
					case WD_DECLARE_COORDINATOR_MESSAGE:
						/* meanwhile someone has declared itself coordinator accept it*/
						reply_with_minimal_message(wdNode, WD_ACCEPT_MESSAGE, pkt);
						set_state(WD_JOINING);
						break;
					default:
						standard_packet_processor(wdNode, pkt);
						break;
				}
			}
		}
			break;
			
		default:
			break;
	}
	return 0;
}

static int watchdog_state_machine_coordinator(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt)
{
	switch (event)
	{
		case WD_EVENT_WD_STATE_CHANGED:
			send_cluster_command(NULL, WD_DECLARE_COORDINATOR_MESSAGE, 0);
			g_cluster.masterNode = g_cluster.localNode;
			fork_escalation_process();
			set_timeout(10);
			break;
			
		case WD_EVENT_CON_OPEN:
			break;
			
		case WD_EVENT_TIMEOUT:
			send_cluster_command(NULL, WD_IAM_COORDINATOR_MESSAGE, 10);
			/* check if the quorum still exists */
			int quorum_status = get_quorum_status();
			if (quorum_status == -1)
			{
				ereport(LOG,
						(errmsg("We do not have enough nodes in cluster")));
				set_state(WD_WAITING_FOR_QUORUM);
			}
			else
				set_timeout(10);
			break;

		case WD_EVENT_REMOTE_NODE_LOST:
		{
			ereport(LOG,
					(errmsg("life check reported \"%s\" is lost",wdNode->nodeName)));

			/*
			 * we have lost one remote connected node
			 * check if the quorum still exists
			 */
			int quorum_status = get_quorum_status();
			if (quorum_status == -1)
			{
				ereport(LOG,
						(errmsg("We have lost the quorum after loosing \"%s\"",wdNode->nodeName)));

				/*
				 * We have lost the qurum, and left with no reason to
				 * continue as a coordinator node
				 */
				set_state(WD_WAITING_FOR_QUORUM);
			}
			else
				ereport(DEBUG1,
						(errmsg("We have lost the node \"%s\" but quorum still holds",wdNode->nodeName)));
		}
			break;

		case WD_EVENT_LOCAL_NODE_LOST:
			ereport(NOTICE,
					(errmsg("Lifecheck reported we have been lost, resigning from master ")));
			resign_from_coordinator();
			set_state(WD_LOST);
			break;

		case WD_EVENT_PACKET_RCV:
		{
			if(pkt == NULL)
			{
				ereport(LOG,
						(errmsg("packet is NULL")));
				break;
			}
			if (false && reply_is_for_last_command(pkt) == false)
			{
				standard_packet_processor(wdNode, pkt);
			}
			else
			{
				switch (pkt->type)
				{
					case WD_STAND_FOR_COORDINATOR_MESSAGE:
						reply_with_minimal_message(wdNode, WD_REJECT_MESSAGE, pkt);
						break;
					case WD_DECLARE_COORDINATOR_MESSAGE:
						ereport(NOTICE,
								(errmsg("We are corrdinator and another node tried a coup")));
						reply_with_minimal_message(wdNode, WD_ERROR_MESSAGE, pkt);
						break;

					case WD_IAM_COORDINATOR_MESSAGE:
					{
						ereport(NOTICE,
								(errmsg("We are in split brain, resigning from master")));
						reply_with_minimal_message(wdNode, WD_ERROR_MESSAGE, pkt);
						set_state(WD_JOINING);
					}
						break;

					case WD_REJECT_MESSAGE:
					{
						if (g_cluster.lastCommand.commandID == pkt->command_id)
						{
							g_cluster.masterNode = NULL;
							ereport(NOTICE,
								(errmsg("possible split brain scenario detected by \"%s\" node", wdNode->nodeName),
									 (errdetail("re-initializing cluster"))));
							set_state(WD_JOINING);
						}
						else
						{
							ereport(NOTICE,
									(errmsg("out of sync error message from \"%s\" node, ignoring", wdNode->nodeName)));
						}
							
					}
						break;

					case WD_ERROR_MESSAGE:
					{
						if (g_cluster.lastCommand.commandMessageType == WD_DECLARE_COORDINATOR_MESSAGE &&
							g_cluster.lastCommand.commandID == pkt->command_id)
						{
							g_cluster.masterNode = NULL;
							ereport(NOTICE,
									(errmsg("our declare for coordinator is rejected by \"%s\" node", wdNode->nodeName)));
							set_state(WD_JOINING);
						}
						else
						{
							/* This is out of sync reject message, reply with error */
							reply_with_minimal_message(wdNode, WD_ERROR_MESSAGE, pkt);
						}
						
					}
						break;

					default:
						standard_packet_processor(wdNode, pkt);
						break;
				}
			}
		}
			break;
			
		default:
			break;
	}
	return 0;
}

static void resign_from_coordinator(void)
{
	
}

static int watchdog_state_machine_voting(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt)
{
	switch (event)
	{
		case WD_EVENT_WD_STATE_CHANGED:
			set_timeout(6);
			break;
			
		case WD_EVENT_CON_OPEN:
			break;
			
		case WD_EVENT_TIMEOUT:
			set_state(WD_JOINING);
			break;
		
		case WD_EVENT_LOCAL_NODE_LOST:
			set_state(WD_JOINING);
			break;
			
		case WD_EVENT_PACKET_RCV:
		{
			if(pkt == NULL)
			{
				ereport(LOG,
						(errmsg("packet is NULL")));
				break;
			}
			switch (pkt->type)
			{
				case WD_STAND_FOR_COORDINATOR_MESSAGE:
					reply_with_minimal_message(wdNode, WD_ACCEPT_MESSAGE, pkt);
					break;
				case WD_IAM_COORDINATOR_MESSAGE:
					set_state(WD_JOINING);
					break;
				case WD_DECLARE_COORDINATOR_MESSAGE:
					reply_with_minimal_message(wdNode, WD_ACCEPT_MESSAGE, pkt);
					set_state(WD_INITIALIZING);
					break;
				default:
					standard_packet_processor(wdNode, pkt);
					break;
			}
		}
			break;
			
		default:
			break;
	}
	return 0;
}

static int watchdog_state_machine_standby(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt)
{
	switch (event)
	{
		case WD_EVENT_WD_STATE_CHANGED:
			send_cluster_command(g_cluster.masterNode, WD_JOIN_COORDINATOR_MESSAGE, 0);

//			set_timeout(6);
			break;

		case WD_EVENT_CON_OPEN:
			break;
			
		case WD_EVENT_TIMEOUT:
			break;

		case WD_EVENT_REMOTE_NODE_LOST:
		{
			ereport(LOG,
					(errmsg("life check reported \"%s\" is lost",wdNode->nodeName)));
			
			/*
			 * we have lost one remote connected node
			 * check if the node was coordinator
			 */
			if (g_cluster.masterNode == NULL)
				set_state(WD_JOINING);
			else
			{
				int quorum_status = get_quorum_status();
				if (quorum_status == -1)
				{
					ereport(LOG,
							(errmsg("We have lost the quorum after loosing \"%s\"",wdNode->nodeName)));
					
					/*
					 * We have lost the qurum, and left with no reason to
					 * continue as a coordinator node
					 */
					set_state(WD_WAITING_FOR_QUORUM);
				}
				else
					ereport(DEBUG1,
							(errmsg("We have lost the node \"%s\" but quorum still holds",wdNode->nodeName)));
			}
		}
			break;
		case WD_EVENT_PACKET_RCV:
			switch (pkt->type)
		{
			case WD_STAND_FOR_COORDINATOR_MESSAGE:
			{
				if (g_cluster.masterNode == NULL)
				{
					reply_with_minimal_message(wdNode, WD_ACCEPT_MESSAGE, pkt);
					set_state(WD_PARTICIPATE_IN_ELECTION);
				}
				else
				{
					reply_with_minimal_message(wdNode, WD_ERROR_MESSAGE, pkt);
					set_state(WD_JOINING);
				}
			}
				break;
			case WD_DECLARE_COORDINATOR_MESSAGE:
				if (wdNode != g_cluster.masterNode)
				{
					/*
					 * we already have a master node
					 * and we got a new node trying to be master
					 * re-initialize the cluster, something is wrong
					 */
					reply_with_minimal_message(wdNode, WD_ERROR_MESSAGE, pkt);
					set_state(WD_JOINING);
				}
				break;
			case WD_REJECT_MESSAGE:
			{
				if (g_cluster.lastCommand.commandMessageType == WD_JOIN_COORDINATOR_MESSAGE &&
					g_cluster.lastCommand.commandID == pkt->command_id)
				{
					ereport(NOTICE,
						(errmsg("our join coordinator is rejected by node \"%s\"",wdNode->nodeName),
							errhint("rejoining the cluster.")));
					set_state(WD_JOINING);
				}
			}
				break;

			default:
				standard_packet_processor(wdNode, pkt);
				break;
		}
			break;
		default:
			break;
	}
	return 0;
}

static int watchdog_state_machine_waiting_for_quorum(WD_EVENTS event, WatchdogNode* wdNode, WDPacketData* pkt)
{
	switch (event)
	{
		case WD_EVENT_WD_STATE_CHANGED:
			send_cluster_command(NULL, WD_QUORUM_IS_LOST, 10);
			break;
			
		case WD_EVENT_CON_OPEN:
			break;
			
		case WD_EVENT_TIMEOUT:
			break;
			
		case WD_EVENT_PACKET_RCV:
		{
			standard_packet_processor(wdNode, pkt);
			if (pkt->type == WD_ADD_NODE_MESSAGE)
				set_state(WD_JOINING);
		}
			break;
		case WD_EVENT_REMOTE_NODE_FOUND:
		{
			if (get_quorum_status() >= 0)
			{
				/* quorum is complete again, start cluster initializing */
				ereport(LOG,
						(errmsg("node \"%s\" is found, and quorum is complete again",wdNode->nodeName),
						 errdetail("initializing the cluster")));
				set_state(WD_JOINING);
			}
			break;
		}

		case WD_EVENT_LOCAL_NODE_FOUND:
			break;

		default:
			break;
	}
	return 0;

}
/*
 * The function identifies the current quorum state
 * return values:
 * -1:
 *     quorum is lost or does not exisits
 * 0:
 *     The quorum is on the edge. (when participating cluster is configured
 *     with even number of nodes, and we have exectly 50% nodes
 * 1:
 *     quorum exists
 */
static int get_quorum_status(void)
{
	int total_nodes = g_cluster.remoteNodeCount + 1;
	int current_cluster_nodes = get_cluster_node_count(); /* this one excluding self */
	int fifty_prcnt_node_count = total_nodes / 2;
	current_cluster_nodes++; /* count myself in*/
	if (total_nodes % 2 == 0)
	{
		/* We have even number of nodes */
		if (current_cluster_nodes == fifty_prcnt_node_count)
			return 0;
		if (current_cluster_nodes < fifty_prcnt_node_count)
			return -1;
		return 1;
	}
	if (current_cluster_nodes <= fifty_prcnt_node_count) /* <= because we use int to store 50% node count */
		return -1;
	return 1;
}

static int set_state(WD_STATES newState)
{
	WD_STATES oldState = g_cluster.localNode->state;
	ereport(DEBUG2,
			(errmsg("setting watchdog state to %s from old state %s",debug_states[newState],debug_states[oldState])));
	g_cluster.localNode->state = newState;
	if (oldState != newState)
		watchdog_state_machine(WD_EVENT_WD_STATE_CHANGED, NULL, NULL);
	return 0;
}
