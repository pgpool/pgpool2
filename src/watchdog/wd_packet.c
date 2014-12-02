/*
 * $Header$
 *
 * Handles watchdog connection, and protocol communication with pgpool-II
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2014	PgPool Global Development Group
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
#include "pool_config.h"
#include "watchdog/watchdog.h"
#include "watchdog/wd_ext.h"
#include "query_cache/pool_memqcache.h"

typedef enum {
	WD_SEND_TO_MASTER = 0,
	WD_SEND_WITHOUT_MASTER,
	WD_SEND_ALL_NODES
} WD_SEND_TYPE;

static int wd_send_node_packet(WD_PACKET_NO packet_no, int *node_id_set, int count);
static int wd_chk_node_mask (WD_PACKET_NO packet_no, int *node_id_set, int count);

static void * wd_thread_negotiation(void * arg);
static int send_packet_for_all(WdPacket *packet);
static int send_packet_4_nodes(WdPacket *packet, WD_SEND_TYPE type);
static int hton_wd_packet(WdPacket * to, WdPacket * from);
static int ntoh_wd_packet(WdPacket * to, WdPacket * from);
static int hton_wd_node_packet(WdPacket * to, WdPacket * from);
static int ntoh_wd_node_packet(WdPacket * to, WdPacket * from);
static int hton_wd_lock_packet(WdPacket * to, WdPacket * from);
static int ntoh_wd_lock_packet(WdPacket * to, WdPacket * from);

int
wd_startup(void)
{
	int rtn;

	/* send add request packet */
	rtn = wd_send_packet_no(WD_ADD_REQ);
	return rtn;
}

int
wd_declare(void)
{
	int rtn;

	/* send declare new master packet */
	ereport(DEBUG1,
		(errmsg("watchdog standing for master"),
			 errdetail("send the packet to declare the new master")));

	rtn = wd_send_packet_no(WD_DECLARE_NEW_MASTER);
	return rtn;
}

int
wd_stand_for_master(void)
{
	int rtn;

	/* send stand for master packet */
	ereport(DEBUG1,
		(errmsg("watchdog standing for master"),
			 errdetail("send the packet to be the new master")));

	rtn = wd_send_packet_no(WD_STAND_FOR_MASTER);
	return rtn;
}

int
wd_notice_server_down(void)
{
	int rtn;

	wd_IP_down();
	/* send notice server down packet */
	rtn = wd_send_packet_no(WD_SERVER_DOWN);
	return rtn;
}

int
wd_update_info(void)
{
	int rtn;

	/* send info request packet */
	rtn = wd_send_packet_no(WD_INFO_REQ);
	return rtn;
}

/* send authentication failed packet */
int
wd_authentication_failed(int sock)
{
	int rtn;
	WdPacket send_packet;

	memset(&send_packet, 0, sizeof(WdPacket));

	send_packet.packet_no = WD_AUTH_FAILED;
	memcpy(&(send_packet.wd_body.wd_info), WD_MYSELF, sizeof(WdInfo));

	rtn = wd_send_packet(sock, &send_packet);

	return rtn;
}

int
wd_send_packet_no(WD_PACKET_NO packet_no )
{
	int rtn = WD_OK;
	WdPacket packet;

	memset(&packet, 0, sizeof(WdPacket));

	/* set packet no and self information */
	packet.packet_no = packet_no;
	memcpy(&(packet.wd_body.wd_info), WD_MYSELF, sizeof(WdInfo));

	/* send packet for all watchdogs */
	rtn = send_packet_for_all(&packet);

	return rtn;
}

int
wd_create_send_socket(char * hostname, int port)
{
	int sock;
	int one = 1;
	size_t len = 0;
	struct sockaddr_in addr;
	struct hostent * hp;

	/* create socket */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		/* socket create failed */
		ereport(WARNING,
			(errmsg("failed to create watchdog sending socket"),
				 errdetail("create socket failed with reason: \"%s\"", strerror(errno))));
		return -1;
	}

	/* set socket option */
	if ( setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &one, sizeof(one)) == -1 )
	{
		close(sock);
		ereport(WARNING,
			(errmsg("failed to create watchdog sending socket"),
				 errdetail("setsockopt(TCP_NODELAY) failed with reason: \"%s\"", strerror(errno))));
		return -1;
	}
	if ( setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &one, sizeof(one)) == -1 )
	{
		close(sock);
		ereport(WARNING,
			(errmsg("failed to create watchdog sending socket"),
				 errdetail("setsockopt(SO_KEEPALIVE) failed with reason: \"%s\"", strerror(errno))));
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
			close(sock);
			ereport(WARNING,
				(errmsg("failed to create watchdog sending socket"),
					 errdetail("gethostbyname for \"%s\" failed with error: \"%s\"", hostname,hstrerror(h_errno))));
			return -1;
		}
	}
	memmove((char *)&(addr.sin_addr), (char *)hp->h_addr, hp->h_length);
	addr.sin_port = htons(port);
	len = sizeof(struct sockaddr_in);

	/* try to connect */
	for (;;)
	{
		if (connect(sock,(struct sockaddr*)&addr, len) < 0)
		{
			if (errno == EINTR)
				continue;
			else if (errno == EISCONN)
				return sock;

			ereport(LOG,
				(errmsg("failed to create watchdog sending socket"),
					 errdetail("connect() reports failure \"%s\"",strerror(errno)),
						errhint("You can safely ignore this while starting up.")));
			break;
		}
		return sock;
	}
	close(sock);
	return -1;
}

int
wd_create_recv_socket(int port)
{
    size_t	len = 0;
    struct sockaddr_in addr;
    int one = 1;
    int sock = -1;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
		/* socket create failed */
		ereport(ERROR,
			(errmsg("failed to create watchdog receive socket"),
				 errdetail("create socket failed with reason: \"%s\"", strerror(errno))));
    }
    if ( fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
    {
		/* failed to set nonblock */
		close(sock);
		ereport(ERROR,
			(errmsg("failed to create watchdog receive socket"),
				 errdetail("setting non blocking mode on socket failed with reason: \"%s\"", strerror(errno))));
    }
    if ( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one)) == -1 )
    {
		/* setsockopt(SO_REUSEADDR) failed */
		close(sock);
		ereport(ERROR,
			(errmsg("failed to create watchdog receive socket"),
				 errdetail("setsockopt(SO_REUSEADDR) failed with reason: \"%s\"", strerror(errno))));
    }
    if ( setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &one, sizeof(one)) == -1 )
    {
        /* setsockopt(TCP_NODELAY) failed */
		close(sock);
		ereport(ERROR,
			(errmsg("failed to create watchdog receive socket"),
				 errdetail("setsockopt(TCP_NODELAY) failed with reason: \"%s\"", strerror(errno))));
    }
    if ( setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &one, sizeof(one)) == -1 )
    {
        /* setsockopt(SO_KEEPALIVE) failed */
		close(sock);
		ereport(ERROR,
			(errmsg("failed to create watchdog receive socket"),
				 errdetail("setsockopt(SO_KEEPALIVE) failed with reason: \"%s\"", strerror(errno))));
    }

    addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    len = sizeof(struct sockaddr_in);

    if ( bind(sock, (struct sockaddr *) & addr, len) < 0 )
    {
		/* bind failed */
		char *host = "", *serv = "";
		char hostname[NI_MAXHOST], servname[NI_MAXSERV];
		if (getnameinfo((struct sockaddr *) &addr, len, hostname, sizeof(hostname), servname, sizeof(servname), 0) == 0) {
			host = hostname;
			serv = servname;
		}
		close(sock);
		ereport(ERROR,
			(errmsg("failed to create watchdog receive socket"),
				 errdetail("bind on \"%s:%s\" failed with reason: \"%s\"", host, serv, strerror(errno))));
    }

    if ( listen(sock, MAX_WATCHDOG_NUM * 2) < 0 )
    {
		/* listen failed */
		close(sock);
		ereport(ERROR,
			(errmsg("failed to create watchdog receive socket"),
				 errdetail("listen failed with reason: \"%s\"", strerror(errno))));
    }

    return sock;
}

int
wd_accept(int sock)
{
	int fd = -1;
	fd_set rmask;
	fd_set emask;
	int rtn;
    struct sockaddr addr;
    socklen_t addrlen = sizeof(struct sockaddr);

	for (;;)
	{
		FD_ZERO(&rmask);
		FD_ZERO(&emask);
		FD_SET(sock,&rmask);
		FD_SET(sock,&emask);

		rtn = select(sock+1, &rmask, NULL, &emask, NULL );
		if ( rtn < 0 )
		{
			if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK )
			{
				continue;
			}
			/* connection failed */
			break;
		}
		else if ( rtn == 0 )
		{
			/* connection failed */
			break;
		}
		else if ( FD_ISSET(sock, &emask) )
		{
			/* socket exception occurred */
			break;
		}
		else if ( FD_ISSET(sock, &rmask) )
		{
			fd = accept(sock, &addr, &addrlen);
			if (fd < 0)
			{
				if ( errno == EINTR || errno == 0 || errno == EAGAIN || errno == EWOULDBLOCK )
				{
					/* nothing to accept now */
					continue;
				}
				/* accept failed */
				return -1;
			}
			return fd;
		}
	}
	return -1;
}

/*
 * Note: Since wd_send_packet() is called from wd_thread_negotiation()
 * which is a thread function and our Exception Manager
 * and Memory Manager are not thread safe so do not use
 * ereport(ERROR,..) and MemoryContextSwitchTo() functions
 * All ereports other than ereport(ERROR) that do not performs longjump
 * are fine to be used from thread function
 */
int
wd_send_packet(int sock, WdPacket * snd_pack)
{
	fd_set wmask;
	struct timeval timeout;
	char * send_ptr = NULL;
	int send_size= 0;
	int buf_size = 0;
	int s = 0;
	int flag = 0;
	int rtn;
	WdPacket buf;

	memset(&buf,0,sizeof(WdPacket));
	if ((snd_pack->packet_no >= WD_INVALID) &&
		(snd_pack->packet_no <= WD_READY ))
	{
		hton_wd_packet((WdPacket *)&buf,snd_pack);
	}
	else if ((snd_pack->packet_no >= WD_START_RECOVERY) &&
	         (snd_pack->packet_no <= WD_NODE_FAILED))
	{
		hton_wd_node_packet((WdPacket *)&buf,snd_pack);
	}
	else
	{
		hton_wd_lock_packet((WdPacket *)&buf,snd_pack);
	}

	send_ptr = (char*)&buf;
	buf_size = sizeof(WdPacket);

	for (;;)
	{
		timeout.tv_sec = WD_SEND_TIMEOUT;
		timeout.tv_usec = 0;

		FD_ZERO(&wmask);
		FD_SET(sock,&wmask);
		rtn = select(sock+1, (fd_set *)NULL, &wmask, (fd_set *)NULL, &timeout);

		if (rtn < 0 )
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				continue;
			}
			return WD_NG;
		}
		else if (rtn & FD_ISSET(sock, &wmask))
		{
			s = send(sock,send_ptr + send_size,buf_size - send_size ,flag);
			if (s < 0)
			{
				if (errno == EINTR || errno == EAGAIN)
					continue;
				else
				{
					/* send failed */
					return WD_NG;
				}
			}
			else if (s == 0)
			{
				/* send failed */
				return WD_NG;
			}
			else /* s > 0 */
			{
				send_size += s;
				if (send_size == buf_size)
				{
					return WD_OK;
				}
			}
		}
	}
	return WD_NG;
}

/*
 * Note: Since wd_recv_packet() is called from wd_thread_negotiation()
 * which is a thread function and our Exception Manager
 * and Memory Manager are not thread safe so do not use
 * ereport(ERROR,..) and MemoryContextSwitchTo() functions
 * All ereports other than ereport(ERROR) that do not performs longjump
 * are fine to be used from thread function
 */
int
wd_recv_packet(int sock, WdPacket * recv_pack)
{
	int r = 0;
	WdPacket buf;
	char * read_ptr = (char *)&buf;
	int read_size = 0;
	int len = sizeof(WdPacket);

	memset(&buf,0,sizeof(WdPacket));
	for (;;)
	{
		r = recv(sock,read_ptr + read_size ,len - read_size, 0);
		if (r < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
				continue;
			else
            {
				ereport(WARNING,
					(errmsg("watchdog failed to receive packet"),
						 errdetail("recv() failed with reason: \"%s\"", strerror(errno))));
				return WD_NG;
            }
		}
		else if (r > 0)
		{
			read_size += r;
			if (read_size == len)
			{

				if (ntohl(buf.packet_no) <= WD_READY)
				{
					ntoh_wd_packet(recv_pack,&buf);
				}
				else if (ntohl((buf.packet_no) >= WD_START_RECOVERY) &&
	         			(ntohl(buf.packet_no) <= WD_NODE_FAILED))
				{
					ntoh_wd_node_packet(recv_pack,&buf);
				}
				else
				{
					ntoh_wd_lock_packet(recv_pack,&buf);
				}

				return WD_OK;
			}
		}
		else /* r == 0 */
		{
			return WD_NG;
		}
	}
	return WD_NG;
}

static void *
wd_thread_negotiation(void * arg)
{
	WdPacketThreadArg * thread_arg;
	int sock;
	uintptr_t rtn;
	WdPacket recv_packet;
	WdInfo * p;
	char pack_str[WD_MAX_PACKET_STRING];
	int pack_str_len;

	thread_arg = (WdPacketThreadArg *)arg;
	sock = thread_arg->sock;
	p = thread_arg->target;
	gettimeofday(&(thread_arg->packet->send_time), NULL);

	if (strlen(pool_config->wd_authkey))
	{
		/* calculate hash from packet */
		pack_str_len = wd_packet_to_string(thread_arg->packet, pack_str, sizeof(pack_str));
		wd_calc_hash(pack_str, pack_str_len, thread_arg->packet->hash);
	}

	/* packet send to target watchdog */
	rtn = (uintptr_t)wd_send_packet(sock, thread_arg->packet);
	if (rtn != WD_OK)
	{
		close(sock);
		ereport(WARNING,
			(errmsg("watchdog negotiation failed"),
				 errdetail("failed to send watchdog packet to \"%s:%d\"", p->hostname, p->wd_port)));
		pthread_exit((void *)rtn);
	}

	/* receive response packet */
	memset(&recv_packet,0,sizeof(WdPacket));
	rtn = (uintptr_t)wd_recv_packet(sock, &recv_packet);
	if (rtn != WD_OK)
	{
		p = thread_arg->target;
		close(sock);
		ereport(WARNING,
			(errmsg("watchdog negotiation failed"),
				 errdetail("failed to receive watchdog packet from \"%s:%d\"", p->hostname, p->wd_port)));
		pthread_exit((void *)rtn);
	}
	rtn = WD_OK;

	switch (thread_arg->packet->packet_no)
	{
		case WD_ADD_REQ:
			if (recv_packet.packet_no == WD_ADD_ACCEPT)
			{
				memcpy(thread_arg->target, &(recv_packet.wd_body.wd_info),sizeof(WdInfo));
			}
			else
			{
				p = &(recv_packet.wd_body.wd_info);
				if(recv_packet.packet_no == WD_ADD_REJECT)
				{
					ereport(WARNING,
						(errmsg("watchdog negotiation failed"),
							errdetail("watchdog add request is rejected by pgpool-II on %s:%d",
								   p->hostname, p->pgpool_port)));
				}
				else
					ereport(WARNING,
						(errmsg("watchdog negotiation failed"),
							 errdetail("invalid response received for watchdog add request from pgpool-II on %s:%d",
									   p->hostname, p->pgpool_port)));

				rtn = WD_NG;
			}
			break;
		case WD_STAND_FOR_MASTER:
			if (recv_packet.packet_no == WD_MASTER_EXIST)
			{
				p = &(recv_packet.wd_body.wd_info);
				wd_set_wd_info(p);
				rtn = WD_NG;
			}
			break;
		case WD_STAND_FOR_LOCK_HOLDER:
		case WD_DECLARE_LOCK_HOLDER:
			if (recv_packet.packet_no == WD_LOCK_HOLDER_EXIST)
			{
				rtn = WD_NG;
			}
			break;
		case WD_DECLARE_NEW_MASTER:
		case WD_RESIGN_LOCK_HOLDER:

			if (recv_packet.packet_no != WD_READY)
			{
				rtn = WD_NG;
			}
			break;
		case WD_START_RECOVERY:
		case WD_FAILBACK_REQUEST:
		case WD_DEGENERATE_BACKEND:
		case WD_PROMOTE_BACKEND:
			rtn = (recv_packet.packet_no == WD_NODE_FAILED) ? WD_NG : WD_OK;
			break;
		case WD_UNLOCK_REQUEST:
			rtn = (recv_packet.packet_no == WD_LOCK_FAILED) ? WD_NG : WD_OK;
			break;
		case WD_AUTH_FAILED:
			ereport(WARNING,
				(errmsg("watchdog negotiation failed"),
					 errdetail("watchdog authentication failed")));
			rtn = WD_NG;
			break;
		default:
			break;
	}
	close(sock);
	pthread_exit((void *)rtn);
}

static int
send_packet_for_all(WdPacket *packet)
{
	int rtn = WD_OK;

	/* send packet to master watchdog */
	if (WD_MYSELF->status != WD_MASTER)
		rtn = send_packet_4_nodes(packet, WD_SEND_TO_MASTER );

	/* send packet to other watchdogs */
	if (rtn == WD_OK)
	{
		rtn = send_packet_4_nodes(packet, WD_SEND_WITHOUT_MASTER);
	}

	return rtn;
}

static int
send_packet_4_nodes(WdPacket *packet, WD_SEND_TYPE type)
{
	int rtn;
	WdInfo * p = WD_List;
	int i,cnt;
	int sock;
	int rc;
	pthread_attr_t attr;
	pthread_t thread[MAX_WATCHDOG_NUM];
	WdPacketThreadArg thread_arg[MAX_WATCHDOG_NUM];

	if (packet == NULL)
	{
		return WD_NG;
	}

	/* thread init */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	/* skip myself */
	p++;
	WD_MYSELF->is_contactable = true;

	/* send packet to other watchdogs */
	cnt = 0;
	while (p->status != WD_END)
	{
		/* don't send packet to pgpool in down */
		if (p->status == WD_DOWN ||
		    (packet->packet_no != WD_ADD_REQ && p->status == WD_INIT))
		{
			p->is_contactable = false;
			p++;
			continue;
		}

		if (type == WD_SEND_TO_MASTER )
		{
			if (p->status != WD_MASTER)
			{
				p++;
				continue;
			}
		}
		else if (type == WD_SEND_WITHOUT_MASTER )
		{
			if (p->status == WD_MASTER)
			{
				p++;
				continue;
			}
		}

		sock = wd_create_send_socket(p->hostname, p->wd_port);
		if (sock == -1)
		{
			ereport(LOG,
				(errmsg("watchdog sending packet for nodes"),
					 errdetail("packet for \"%s:%d\" is canceled", p->hostname, p->wd_port)));
			p->is_contactable = false;
			p++;
			continue;
		}
		else
		{
			p->is_contactable = true;
		}

		thread_arg[cnt].sock = sock;
		thread_arg[cnt].target = p;
		thread_arg[cnt].packet = packet;
		rc = watchdog_thread_create(&thread[cnt], &attr, wd_thread_negotiation, (void*)&thread_arg[cnt]);

		cnt ++;
		p++;
	}

	pthread_attr_destroy(&attr);

	/* no packet is sent */
	if (cnt == 0)
	{
		return WD_OK;
	}

	/* default return value */
	if ((packet->packet_no == WD_STAND_FOR_MASTER) ||
		(packet->packet_no == WD_STAND_FOR_LOCK_HOLDER) ||
		(packet->packet_no == WD_DECLARE_LOCK_HOLDER) ||
		(packet->packet_no == WD_START_RECOVERY))
	{
		rtn = WD_OK;
	}
	else
	{
		rtn = WD_NG;
	}

	/* receive the results */
	for (i=0; i<cnt; )
	{
		int result;
		rc = pthread_join(thread[i], (void **)&result);
		if ((rc != 0) && (errno == EINTR))
		{
			usleep(100);
			continue;
		}

		/*  aggregate results according to the packet type */
		if ((packet->packet_no == WD_STAND_FOR_MASTER) ||
			(packet->packet_no == WD_STAND_FOR_LOCK_HOLDER) ||
			(packet->packet_no == WD_DECLARE_LOCK_HOLDER) ||
			(packet->packet_no == WD_START_RECOVERY))
		{
			/* if any result is NG then return NG */
			if (result == WD_NG)
			{
				rtn = WD_NG;
			}
		}

		else
		{
			/* if any result is OK then return OK */
			if (result == WD_OK)
			{
				rtn = WD_OK;
			}
		}
		i++;
	}

	return rtn;
}

static int
hton_wd_packet(WdPacket * to, WdPacket * from)
{
	WdInfo * to_info = NULL;
	WdInfo * from_info = NULL;

	if ((to == NULL) || (from == NULL))
	{
		return WD_NG;
	}

	to_info = &(to->wd_body.wd_info);
	from_info = &(from->wd_body.wd_info);

	to->packet_no = htonl(from->packet_no);
	to->send_time.tv_sec = htonl(from->send_time.tv_sec);
	to->send_time.tv_usec = htonl(from->send_time.tv_usec);

	memcpy(to->hash, from->hash, sizeof(to->hash));

	to_info->status = htonl(from_info->status);
	to_info->tv.tv_sec = htonl(from_info->tv.tv_sec);
	to_info->tv.tv_usec = htonl(from_info->tv.tv_usec);
	to_info->pgpool_port = htonl(from_info->pgpool_port);
	to_info->wd_port = htonl(from_info->wd_port);

	memcpy(to_info->hostname, from_info->hostname, sizeof(to_info->hostname));
	memcpy(to_info->delegate_ip, from_info->delegate_ip, sizeof(to_info->delegate_ip));

	return WD_OK;
}

static int
ntoh_wd_packet(WdPacket * to, WdPacket * from)
{
	WdInfo * to_info = NULL;
	WdInfo * from_info = NULL;

	if ((to == NULL) || (from == NULL))
	{
		return WD_NG;
	}

	to_info = &(to->wd_body.wd_info);
	from_info = &(from->wd_body.wd_info);

	to->packet_no = ntohl(from->packet_no);
	to->send_time.tv_sec = ntohl(from->send_time.tv_sec);
	to->send_time.tv_usec = ntohl(from->send_time.tv_usec);

	memcpy(to->hash, from->hash, sizeof(to->hash));

	to_info->status = ntohl(from_info->status);
	to_info->tv.tv_sec = ntohl(from_info->tv.tv_sec);
	to_info->tv.tv_usec = ntohl(from_info->tv.tv_usec);
	to_info->pgpool_port = ntohl(from_info->pgpool_port);
	to_info->wd_port = ntohl(from_info->wd_port);

	memcpy(to_info->hostname, from_info->hostname, sizeof(to_info->hostname));
	memcpy(to_info->delegate_ip, from_info->delegate_ip, sizeof(to_info->delegate_ip));

	return WD_OK;
}

static int
hton_wd_node_packet(WdPacket * to, WdPacket * from)
{
	WdNodeInfo * to_info = NULL;
	WdNodeInfo * from_info = NULL;
	int i;

	if ((to == NULL) || (from == NULL))
	{
		return WD_NG;
	}

	to_info = &(to->wd_body.wd_node_info);
	from_info = &(from->wd_body.wd_node_info);

	to->packet_no = htonl(from->packet_no);
	to->send_time.tv_sec = htonl(from->send_time.tv_sec);
	to->send_time.tv_usec = htonl(from->send_time.tv_usec);

	memcpy(to->hash, from->hash, sizeof(to->hash));

	to_info->node_num = htonl(from_info->node_num);

	for (i = 0 ; i < from_info->node_num ; i ++)
	{
		to_info->node_id_set[i] = htonl(from_info->node_id_set[i]);
	}

	return WD_OK;
}

static int
ntoh_wd_node_packet(WdPacket * to, WdPacket * from)
{
	WdNodeInfo * to_info = NULL;
	WdNodeInfo * from_info = NULL;
	int i;

	if ((to == NULL) || (from == NULL))
	{
		return WD_NG;
	}

	to_info = &(to->wd_body.wd_node_info);
	from_info = &(from->wd_body.wd_node_info);

	to->packet_no = ntohl(from->packet_no);
	to->send_time.tv_sec = ntohl(from->send_time.tv_sec);
	to->send_time.tv_usec = ntohl(from->send_time.tv_usec);

	memcpy(to->hash, from->hash, sizeof(to->hash));

	to_info->node_num = ntohl(from_info->node_num);

	for (i = 0 ; i < to_info->node_num ; i ++)
	{
		to_info->node_id_set[i] = ntohl(from_info->node_id_set[i]);
	}

	return WD_OK;
}

static int
hton_wd_lock_packet(WdPacket * to, WdPacket * from)
{
	WdLockInfo * to_info = NULL;
	WdLockInfo * from_info = NULL;

	if ((to == NULL) || (from == NULL))
	{
		return WD_NG;
	}

	to_info = &(to->wd_body.wd_lock_info);
	from_info = &(from->wd_body.wd_lock_info);

	to->packet_no = htonl(from->packet_no);
	to->send_time.tv_sec = htonl(from->send_time.tv_sec);
	to->send_time.tv_usec = htonl(from->send_time.tv_usec);

	memcpy(to->hash, from->hash, sizeof(to->hash));

	to_info->lock_id = htonl(from_info->lock_id);

	return WD_OK;
}

static int
ntoh_wd_lock_packet(WdPacket * to, WdPacket * from)
{
	WdLockInfo * to_info = NULL;
	WdLockInfo * from_info = NULL;

	if ((to == NULL) || (from == NULL))
	{
		return WD_NG;
	}

	to_info = &(to->wd_body.wd_lock_info);
	from_info = &(from->wd_body.wd_lock_info);

	to->packet_no = ntohl(from->packet_no);
	to->send_time.tv_sec = ntohl(from->send_time.tv_sec);
	to->send_time.tv_usec = ntohl(from->send_time.tv_usec);

	memcpy(to->hash, from->hash, sizeof(to->hash));

	to_info->lock_id = ntohl(from_info->lock_id);

	return WD_OK;
}

int
wd_escalation(void)
{
	int rtn, r;
	bool has_error = false;

	ereport(LOG,
		(errmsg("watchdog escalation"),
			 errdetail("escalating to master pgpool")));

	/* clear shared memory cache */
	if (pool_config->memory_cache_enabled && pool_is_shmem_cache() &&
	    pool_config->clear_memqcache_on_escalation)
	{
		ereport(LOG,
			(errmsg("watchdog escalation"),
				 errdetail("clearing all the query cache on shared memory")));

		pool_clear_memory_cache();
	}

	/* execute escalation command */
	if (strlen(pool_config->wd_escalation_command))
	{
		r = system(pool_config->wd_escalation_command);
		if (WIFEXITED(r))
		{
			if (WEXITSTATUS(r) == EXIT_SUCCESS)
				ereport(LOG,
					(errmsg("watchdog escalation successful")));
			else
			{
				ereport(WARNING,
						(errmsg("watchdog escalation command failed with exit status: %d", WEXITSTATUS(r))));
				has_error = true;
			}
		}
		else
		{
			ereport(WARNING,
				(errmsg("watchdog escalation command exit abnormally")));
			has_error = true;
		}
	}

	/* interface up as delegate IP */
	if (strlen(pool_config->delegate_IP) != 0)
	{
		r = wd_IP_up();
		if (r == WD_NG)
			has_error = true;
	}

	/* set master status to the wd list */
	wd_set_wd_list(pool_config->wd_hostname, pool_config->port,
	               pool_config->wd_port, pool_config->delegate_IP,
	               NULL, WD_MASTER);

	/* send declare packet */
	rtn = wd_declare();
	if (rtn == WD_OK)
	{
		if (has_error)
			ereport(NOTICE,
				(errmsg("watchdog escalation successful, escalated to master pgpool with some errors")));
		else
			ereport(LOG,
				(errmsg("watchdog escalation successful, escalated to master pgpool")));
	}

	return rtn;
}

int
wd_start_recovery(void)
{
	int rtn;

	/* send start recovery packet */
	rtn = wd_send_packet_no(WD_START_RECOVERY);
	return rtn;
}

int
wd_end_recovery(void)
{
	int rtn;

	/* send end recovery packet */
	rtn = wd_send_packet_no(WD_END_RECOVERY);
	return rtn;
}

int
wd_send_failback_request(int node_id)
{
	int rtn = 0;
	int n = node_id;

	/* if failback packet is received already, do nothing */
	if (wd_chk_node_mask(WD_FAILBACK_REQUEST,&n,1))
	{
		return WD_OK;
	}

	/* send failback packet */
	rtn = wd_send_node_packet(WD_FAILBACK_REQUEST, &n, 1);
	return rtn;
}

int
wd_degenerate_backend_set(int *node_id_set, int count)
{
	int rtn = 0;

	/* if degenerate packet is received already, do nothing */
	if (wd_chk_node_mask(WD_DEGENERATE_BACKEND,node_id_set,count))
	{
		return WD_OK;
	}

	/* send degenerate packet */
	rtn = wd_send_node_packet(WD_DEGENERATE_BACKEND, node_id_set, count);
	return rtn;
}

int
wd_promote_backend(int node_id)
{
	int rtn = 0;
	int n = node_id;

	/* if promote packet is received already, do nothing */
	if (wd_chk_node_mask(WD_PROMOTE_BACKEND,&n,1))
	{
		return WD_OK;
	}

	/* send promote packet */
	rtn = wd_send_node_packet(WD_PROMOTE_BACKEND, &n, 1);
	return rtn;
}

static int
wd_send_node_packet(WD_PACKET_NO packet_no, int *node_id_set, int count)
{
	int rtn = 0;
	WdPacket packet;

	memset(&packet, 0, sizeof(WdPacket));
	/* set add request packet */
	packet.packet_no = packet_no;
	memcpy(packet.wd_body.wd_node_info.node_id_set,node_id_set,sizeof(int)*count);
	packet.wd_body.wd_node_info.node_num = count;

	/* send packet to all watchdogs */
	rtn = send_packet_for_all(&packet);

	return rtn;
}

int
wd_send_lock_packet(WD_PACKET_NO packet_no,  WD_LOCK_ID lock_id)
{
	int rtn = 0;
	WdPacket packet;

	memset(&packet, 0, sizeof(WdPacket));

	/* set add request packet */
	packet.packet_no = packet_no;
	packet.wd_body.wd_lock_info.lock_id= lock_id;

	/* send packet to all watchdogs */
	rtn = send_packet_for_all(&packet);

	return rtn;
}

/* check mask, and if maskted return 1 and clear it, otherwise return 0 */
static int
wd_chk_node_mask (WD_PACKET_NO packet_no, int *node_id_set, int count)
{
	int rtn = 0;
	unsigned char mask = 0;
	int i;
	int offset = 0;
	mask = 1 << (packet_no - WD_START_RECOVERY);
	for ( i = 0 ; i < count ; i ++)
	{
		offset = *(node_id_set+i);
		if ((*(WD_Node_List + offset) & mask) != 0)
		{
			*(WD_Node_List + offset) ^= mask;
			rtn = 1;
		}
	}
	return rtn;
}

/* set mask */
int
wd_set_node_mask (WD_PACKET_NO packet_no, int *node_id_set, int count)
{
	int rtn = 0;
	unsigned char mask = 0;
	int i;
	int offset = 0;
	mask = 1 << (packet_no - WD_START_RECOVERY);
	for ( i = 0 ; i < count ; i ++)
	{
		offset = *(node_id_set+i);
		*(WD_Node_List + offset) |= mask;
	}
	return rtn;
}

/* calculate hash for authentication using packet contents */
void
wd_calc_hash(const char *str, int len, char *buf)
{
	char pass[(MAX_PASSWORD_SIZE + 1) / 2];
	char username[(MAX_PASSWORD_SIZE + 1) / 2];
	size_t pass_len;
	size_t username_len;
	size_t authkey_len;

	/* use first half of authkey as username, last half as password */
	authkey_len = strlen(pool_config->wd_authkey);

	username_len = authkey_len / 2;
	pass_len = authkey_len - username_len;
	snprintf(username, username_len + 1, "%s", pool_config->wd_authkey);
	snprintf(pass, pass_len + 1, "%s", pool_config->wd_authkey + username_len);

	/* calculate hash using md5 encrypt */
	pool_md5_encrypt(pass, username, strlen(username), buf + MD5_PASSWD_LEN + 1);
	buf[(MD5_PASSWD_LEN+1)*2-1] = '\0';

	pool_md5_encrypt(buf+MD5_PASSWD_LEN+1, str, len, buf);
	buf[MD5_PASSWD_LEN] = '\0';
}

int
wd_packet_to_string(WdPacket *pkt, char *str, int maxlen)
{
	int len;

	len = snprintf(str, maxlen, "no=%d tv_sec=%ld tv_usec=%ld",
	               pkt->packet_no, pkt->send_time.tv_sec, pkt->send_time.tv_usec);

	return len;
}
