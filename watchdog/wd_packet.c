/*
 * $Header$
 *
 * Handles watchdog connection, and protocol communication with pgpool-II
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
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "pool.h"
#include "pool_config.h"
#include "watchdog.h"
#include "wd_ext.h"

int wd_startup(void);
int wd_declare(void);
int wd_stand_for_master(void);
int wd_notice_server_down(void);
int wd_create_send_socket(char * hostname, int port);
int wd_create_recv_socket(int port);
int wd_accept(int sock);
int wd_send_packet(int sock, WdPacket * snd_pack);
int wd_recv_packet(int sock, WdPacket * buf);
int wd_escalation(void);
int wd_start_recovery(void);
int wd_end_recovery(void);
static int wd_send_packet_no(WD_PACKET_NO packet_no );
static void * wd_negotiation(void * arg);
static int send_packet_4_all(WdPacket *packet);
static int hton_wd_packet(WdPacket * to, WdPacket * from);
static int ntoh_wd_packet(WdPacket * to, WdPacket * from);

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
	rtn = wd_send_packet_no(WD_DECLARE_NEW_MASTER);
	return rtn;
}

int
wd_stand_for_master(void)
{
	int rtn;

	/* send staqnd for master packet */
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

static int
wd_send_packet_no(WD_PACKET_NO packet_no )
{
	int rtn;
	WdPacket packet;

	/* set add request packet */
	packet.packet_no = packet_no;
	memcpy(&(packet.wd_info),WD_List,sizeof(WdInfo));
	/* send packet to all watchdogs */	
	rtn = send_packet_4_all(&packet);
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
		return -1;
	}

	/* set socket option */
	if ( setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &one, sizeof(one)) == -1 )
	{
		return -1;
	}
	if ( setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &one, sizeof(one)) == -1 )
	{
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
			pool_error("gethostbyname() failed: %s host: %s", hstrerror(h_errno), hostname);
			close(sock);
			return -1;
		}
	}
	memmove((char *)&(addr.sin_addr), (char *)hp->h_addr, hp->h_length);
	addr.sin_port = htons(port);
	len = sizeof(struct sockaddr_in);

	/* try to connect */
	for (;;)
	{
		fd_set wmask;
		fd_set emask;
		struct timeval timeout;
		int sockErrVal = 0;
		int rtn;
		socklen_t sockErrValLen = sizeof(int);

		timeout.tv_sec = WD_SEND_TIMEOUT;
		timeout.tv_usec = 0;

		FD_ZERO(&wmask);
		FD_ZERO(&emask);
		FD_SET(sock,&wmask);
		FD_SET(sock,&emask);

		rtn = select(sock+1, NULL, &wmask, &emask, &timeout );
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
			/* timeout */
			break;
		}
		else if ( FD_ISSET(sock, &emask) )
		{
			/* socket exception occured */
			break;
		}

		if ( getsockopt(sock, SOL_SOCKET, SO_ERROR, (void*)&sockErrVal, &sockErrValLen) == 0 && sockErrVal != 0 )
		{
			/* error occured on this socket while connecting to target */
			break;
		}

		rtn = connect(sock,(struct sockaddr*)&addr, len);
		if ( rtn < 0 )
		{
			if ( errno == EINPROGRESS || errno == EALREADY || errno == EWOULDBLOCK || errno == EINTR || errno == 0 )
				continue;
			else if ( errno == EISCONN )
			{
				return sock;
			}
			pool_error("connect() is failed,(%s)",strerror(errno));
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
		return -1;
    }
    if ( fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
    {
		/* failed to set nonblock */
		close(sock);
        return -1;
    }
    if ( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one)) == -1 )
    {
		/* setsockopt(SO_REUSEADDR) failed */
		close(sock);
        return -1;
    }
    if ( setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &one, sizeof(one)) == -1 )
    {
        /* setsockopt(TCP_NODELAY) failed */
		close(sock);
        return -1;
    }
    if ( setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &one, sizeof(one)) == -1 )
    {
        /* setsockopt(SO_KEEPALIVE) failed */
		close(sock);
        return -1;
    }

    addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    len = sizeof(struct sockaddr_in);

    if ( bind(sock, (struct sockaddr *) & addr, len) < 0 )
    {
		/* bind failed */
		close(sock);
        return -1;
    }

    if ( listen(sock, MAX_WATCHDOG_NUM * 2) < 0 )
    {
		/* listen failed */
		close(sock);
        return -1;
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
			/* socket exception occured */
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
	hton_wd_packet((WdPacket *)&buf,snd_pack);
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
				return WD_NG;
			}
		}
		else if (r > 0)
		{
			read_size += r;
			if (read_size == len)
			{
				ntoh_wd_packet(recv_pack,&buf);
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
wd_negotiation(void * arg)
{
	WdPacketThreadArg * thread_arg;
	int sock;
	uintptr_t rtn;
	WdPacket recv_packet;
	WdInfo * p;

	thread_arg = (WdPacketThreadArg *)arg;
	sock = thread_arg->sock;

	/* packet send to target watchdog */
	rtn = (uintptr_t)wd_send_packet(sock, thread_arg->packet);
	if (rtn != WD_OK)
	{
		close(sock);
		pthread_exit((void *)rtn);
	}

	/* receive response packet */
	memset(&recv_packet,0,sizeof(WdPacket));
	rtn = (uintptr_t)wd_recv_packet(sock, &recv_packet);
	if (rtn != WD_OK)
	{
		close(sock);
		pthread_exit((void *)rtn);
	}
	rtn = WD_OK;
	switch (thread_arg->packet->packet_no)
	{
		case WD_ADD_REQ:
			if (recv_packet.packet_no == WD_ADD_ACCEPT)
			{
				memcpy(thread_arg->target, &(recv_packet.wd_info),sizeof(WdInfo));
			}
			else
			{
				rtn = WD_NG;
			}
			break;
		case WD_STAND_FOR_MASTER:
			if (recv_packet.packet_no == WD_MASTER_EXIST)
			{
				p = &(recv_packet.wd_info);
				wd_set_wd_info(p);
				rtn = WD_NG;
			}
			break;
		case WD_DECLARE_NEW_MASTER:
			if (recv_packet.packet_no != WD_READY)
			{
				rtn = WD_NG;
			}
			break;
		default:
			break;
	}
	close(sock);
	pthread_exit((void *)rtn);
}

static int
send_packet_4_all(WdPacket *packet)
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
	/* send packet to all watchdogs */
	p++;
	cnt = 0;
	while (p->status != WD_END)
	{
		sock = wd_create_send_socket(p->hostname, p->wd_port);
		if (sock == -1)
		{
			p++;
			continue;
		}
		thread_arg[cnt].sock = sock;
		thread_arg[cnt].target = p;
		thread_arg[cnt].packet = packet;
		rc = pthread_create(&thread[cnt], &attr, wd_negotiation, (void*)&thread_arg[cnt]);
		cnt ++;
		p++;
	}

	pthread_attr_destroy(&attr);
	rtn = (packet->packet_no == WD_STAND_FOR_MASTER)?WD_OK:WD_NG;
	for (i=0; i<cnt; )
	{
		int result;
		rc = pthread_join(thread[i], (void **)&result);
		if ((rc != 0) && (errno == EINTR))
		{
			usleep(100);
			continue;
		}
		if (packet->packet_no == WD_STAND_FOR_MASTER)
		{
			if (result == WD_NG)
			{
				rtn = WD_NG;
			}
		}
		else
		{
			if (result == WD_OK)
			{
				rtn = WD_OK;
			}
		}
		pthread_detach(thread[i]);
		i++;
	}

	return rtn;
}

static int
hton_wd_packet(WdPacket * to, WdPacket * from)
{
	if ((to == NULL) || (from == NULL))
	{
		return WD_NG;
	}
	to->packet_no = htonl(from->packet_no);
	to->wd_info.status = htonl(from->wd_info.status);
	to->wd_info.tv.tv_sec = htonl(from->wd_info.tv.tv_sec);
	to->wd_info.tv.tv_usec = htonl(from->wd_info.tv.tv_usec);
	to->wd_info.pgpool_port = htonl(from->wd_info.pgpool_port);
	to->wd_info.wd_port = htonl(from->wd_info.wd_port);
	memcpy(to->wd_info.hostname,from->wd_info.hostname,sizeof(to->wd_info.hostname));
	return WD_OK;
}

static int
ntoh_wd_packet(WdPacket * to, WdPacket * from)
{
	if ((to == NULL) || (from == NULL))
	{
		return WD_NG;
	}
	to->packet_no = ntohl(from->packet_no);
	to->wd_info.status = ntohl(from->wd_info.status);
	to->wd_info.tv.tv_sec = ntohl(from->wd_info.tv.tv_sec);
	to->wd_info.tv.tv_usec = ntohl(from->wd_info.tv.tv_usec);
	to->wd_info.pgpool_port = ntohl(from->wd_info.pgpool_port);
	to->wd_info.wd_port = ntohl(from->wd_info.wd_port);
	memcpy(to->wd_info.hostname,from->wd_info.hostname,sizeof(to->wd_info.hostname));
	return WD_OK;
}

int
wd_escalation(void)
{
	int rtn;

	/* interface up as delegate IP */
	wd_IP_up();
	/* set master status to the wd list */
	wd_set_wd_list(pool_config->pgpool2_hostname, pool_config->port, pool_config->wd_port, NULL, WD_MASTER);

	/* send declare packet */
	rtn = wd_declare();

	return rtn;
}

int
wd_start_recovery(void)
{
	int rtn;

	/* send staqnd for master packet */
	rtn = wd_send_packet_no(WD_START_RECOVERY);
	return rtn;
}

int
wd_end_recovery(void)
{
	int rtn;

	/* send staqnd for master packet */
	rtn = wd_send_packet_no(WD_END_RECOVERY);
	return rtn;
}

