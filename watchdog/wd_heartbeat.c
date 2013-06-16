/*
 * $Header$
 *
 * Handles watchdog connection, and protocol communication with pgpool-II
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2013	PgPool Global Development Group
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
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#if defined(SO_BINDTODEVICE)
#include <net/if.h>
#endif

#include "pool.h"
#include "pool_config.h"
#include "md5.h"
#include "watchdog.h"
#include "wd_ext.h"

#define MAX_BIND_TRIES 5

static RETSIGTYPE hb_sender_exit(int sig);
static RETSIGTYPE hb_receiver_exit(int sig);
static int hton_wd_hb_packet(WdHbPacket * to, WdHbPacket * from);
static int ntoh_wd_hb_packet(WdHbPacket * to, WdHbPacket * from);
static int packet_to_string_hb(WdHbPacket pkt, char *str, int maxlen);

/* create socket for sending heartbeat */
int
wd_create_hb_send_socket(WdHbIf hb_if)
{
	int sock;
	int tos;

	/* create socket */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		/* socket create failed */
		pool_error("wd_create_hb_send_socket: Failed to create socket. reason: %s",
		           strerror(errno));
		return -1;
	}

	/* set socket option */
	tos = IPTOS_LOWDELAY;
	if (setsockopt(sock, IPPROTO_IP, IP_TOS, (char *) &tos, sizeof(tos)) == -1 )
	{
		pool_error("wd_create_hb_send_socket: setsockopt(IP_TOS) failed. reason: %s",
		           strerror(errno));
		close(sock);
		return -1;
	}
/*
#if defined(SO_BINDTODEVICE)
	{
		struct ifreq i;
		strlcpy(i.ifr_name, hb_if.if_name, sizeof(i.ifr_name));

		if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, &i, sizeof(i)) == -1)
		{
			pool_error("wd_create_hb_send_socket: setsockopt(SO_BINDTODEVICE) failed. reason: %s, device: %s",
			           strerror(errno), i.ifr_name);
			close(sock);
			return -1;
		}
		pool_log("wd_create_hb_send_socket: bind send socket to device: %s", i.ifr_name);
	}
#endif
*/
#if defined(SO_REUSEPORT)
	{
		int one = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) == -1)
		{
			pool_error("wd_create_hb_send_socket: setsockopt(SO_REUSEPORT) failed. reason: %s",
			           strerror(errno));
			close(sock);
			return -1;
		}
		pool_log("wd_create_hb_send_socket: set SO_REUSEPORT");
	}
#endif

 	if (fcntl(sock, F_SETFD, FD_CLOEXEC) < 0) {
		pool_error("wd_create_hb_send_socket: setting close-on-exec flag failed. reason: %s",
		           strerror(errno));
		close(sock);
		return -1;
	}

	return sock;
}

/* create socket for receiving heartbeat */
int
wd_create_hb_recv_socket(WdHbIf hb_if)
{
	struct sockaddr_in addr;
	int sock;
	const int one = 1;
	int bind_tries;
	int bind_is_done;

	memset(&(addr), 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pool_config->wd_heartbeat_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	/* create socket */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		/* socket create failed */
		pool_error("wd_create_hb_recv_socket: Failed to create socket. reason: %s",
		           strerror(errno));
		return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one)) == -1 )
	{
		pool_error("wd_create_hb_recv_socket: setsockopt(SO_REUSEADDR) failed. reason: %s",
		           strerror(errno));
		close(sock);
		return -1;
	}
/*
#if defined(SO_BINDTODEVICE)
	{
		struct ifreq i;
		strlcpy(i.ifr_name, hb_if.if_name, sizeof(i.ifr_name));

		if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, &i, sizeof(i)) == -1)
		{
			pool_error("wd_create_hb_recv_socket: setsockopt(SO_BINDTODEVICE) failed. reason: %s, devise: %s",
			           strerror(errno), i.ifr_name);
			close(sock);
			return -1;
		}
		pool_log("wd_create_hb_recv_socket: bind receive socket to device: %s", i.ifr_name);
	}
#endif
*/

#if defined(SO_REUSEPORT)
	{
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) == -1)
		{
			pool_error("wd_create_hb_recv_socket: setsockopt(SO_REUSEPORT) failed. reason: %s",
			           strerror(errno));
			close(sock);
			return -1;
		}
		pool_log("wd_create_hb_recv_socket: set SO_REUSEPORT");
	}
#endif

	bind_is_done = 0;
	for (bind_tries = 0; !bind_is_done && bind_tries < MAX_BIND_TRIES; bind_tries++)
	{
		if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
		{
			pool_log("wd_crate_hb_recv_socket: bind failed. reason: %s ... retrying",
			         strerror(errno));
			sleep(1);
		}
		else
		{
			bind_is_done = 1;
		}
	}

	/* bind failed finally */
	if (!bind_is_done)
	{
		pool_error("wd_crate_hb_recv_socket: unable to bind socket. reason: %s",
		           strerror(errno));
		close(sock);
		return -1;
	}

 	if (fcntl(sock, F_SETFD, FD_CLOEXEC) < 0) {
		pool_error("wd_create_hb_recv_socket: setting close-on-exec flag failed. reason: %s",
		           strerror(errno));
		close(sock);
		return -1;
	}

	return sock;
}

/* send heartbeat signal */
int
wd_hb_send(int sock, WdHbPacket * pkt, int len, const char * host, const int port)
{
	int rtn;
	struct sockaddr_in addr;
	struct hostent *hp;
	WdHbPacket buf;

	if (!host || !strlen(host))
	{
		pool_error("wd_hb_send: host name is empty");
		return -1;
	}

	hp = gethostbyname(host);
	if ((hp == NULL) || (hp->h_addrtype != AF_INET))
	{
		pool_error("wd_hb_send: gethostbyname() failed: %s host: %s",
		           hstrerror(h_errno), host);
		return -1;
	}
	memmove((char *) &(addr.sin_addr), (char *) hp->h_addr, hp->h_length);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	hton_wd_hb_packet(&buf, pkt);

	if ((rtn = sendto(sock, &buf, sizeof(WdHbPacket), 0,
	                  (struct sockaddr *)&addr, sizeof(addr))) != len)
	{
		pool_error("wd_hb_send: failed to sent packet to %s", host);
		return WD_NG;
	}
	pool_debug("wd_hb_send: send %d byte packet", rtn);

	return WD_OK;
}

/* receive heartbeat signal */
int
wd_hb_recv(int sock, WdHbPacket * pkt)
{
	int rtn;
	struct sockaddr_in senderinfo;
	socklen_t addrlen;
	WdHbPacket buf;

	addrlen = sizeof(senderinfo);

	rtn = recvfrom(sock, &buf, sizeof(WdHbPacket), 0,
	               (struct sockaddr *)&senderinfo, &addrlen);
	if (rtn < 0)
	{	
		pool_error("wd_hb_recv: failed to receive packet");
		return WD_NG;
	}
	else if (rtn == 0)
	{
		pool_error("wd_hb_recv: received zero bytes");
		return WD_NG;
	}
	else
	{
		pool_debug("wd_hb_recv: received %d byte packet", rtn);
	}

	ntoh_wd_hb_packet(pkt, &buf);

	return WD_OK;
}

/* fork heartbeat receiver child */
pid_t wd_hb_receiver(int fork_wait_time, WdHbIf hb_if)
{
	int sock;
	pid_t pid = 0;
	WdHbPacket pkt;
	struct timeval tv;
	char from[WD_MAX_HOST_NAMELEN];
	int from_pgpool_port;
	char buf[(MD5_PASSWD_LEN+1)*2];
	char pack_str[WD_MAX_PACKET_STRING];
	int pack_str_len;

	WdInfo * p;

	pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			pool_error("wd_hb_receiver: fork() failed.");

		return pid;
	}

	if (fork_wait_time > 0)
	{
		sleep(fork_wait_time);
	}

	myargv = save_ps_display_args(myargc, myargv);

	POOL_SETMASK(&UnBlockSig);

	signal(SIGTERM, hb_receiver_exit);
	signal(SIGINT, hb_receiver_exit);
	signal(SIGQUIT, hb_receiver_exit);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);

	init_ps_display("", "", "", "");

	if ( (sock = wd_create_hb_recv_socket(hb_if)) < 0)
	{
		pool_error("wd_hb_receiver: socket create failed");
		hb_receiver_exit(SIGTERM);
	}

	set_ps_display("heartbeat receiver", false);

	for(;;)
	{
		/* receive heartbeat signal */
		if (wd_hb_recv(sock, &pkt) == WD_OK)
		{
			/* authentication */
			if (strlen(pool_config->wd_authkey))
			{
				/* calculate hash from packet */
				pack_str_len = packet_to_string_hb(pkt, pack_str, sizeof(pack_str));
				wd_calc_hash(pack_str, pack_str_len, buf);

				if (strcmp(pkt.hash, buf))
				{
					pool_log("wd_hb_receiver: authentication failed");
					continue;
				}
			}

			/* get current time */
			gettimeofday(&tv, NULL);

			/* who send this packet? */
			strlcpy(from, pkt.from, sizeof(from));
			from_pgpool_port = pkt.from_pgpool_port;

			p = WD_List;
			while (p->status != WD_END)
			{
				if (!strcmp(p->hostname, from) && p->pgpool_port == from_pgpool_port)
				{
					/* this is the first packet or the latest packet */
					if (!WD_TIME_ISSET(p->hb_send_time) ||
					    WD_TIME_BEFORE(p->hb_send_time, pkt.send_time))
					{
						pool_debug("wd_hb_receiver: received heartbeat signal from %s:%d",
						           from, from_pgpool_port);
						p->hb_send_time = pkt.send_time;
						p->hb_last_recv_time = tv;
					}
					break;
				}
				p++;
			}
		}
	}

	return pid;
}

/* fork heartbeat sender child */
pid_t wd_hb_sender(int fork_wait_time, WdHbIf hb_if)
{
	int sock;
	pid_t pid = 0;
	WdHbPacket pkt;
	WdInfo * p = WD_List;
	char pack_str[WD_MAX_PACKET_STRING];
	int pack_str_len;

	pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			pool_error("wd_hb_sender: fork() failed.");

		return pid;
	}

	if (fork_wait_time > 0)
	{
		sleep(fork_wait_time);
	}

	myargv = save_ps_display_args(myargc, myargv);

	POOL_SETMASK(&UnBlockSig);

	signal(SIGTERM, hb_sender_exit);
	signal(SIGINT, hb_sender_exit);
	signal(SIGQUIT, hb_sender_exit);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);

	init_ps_display("", "", "", "");

	if ( (sock = wd_create_hb_send_socket(hb_if)) < 0)
	{
		pool_error("wd_hb_sender: socket create failed");
		hb_sender_exit(SIGTERM);
	}

	set_ps_display("heartbeat sender", false);

	for(;;)
	{
		/* contents of packet */
		gettimeofday(&pkt.send_time, NULL);
		strlcpy(pkt.from, pool_config->wd_hostname, sizeof(pkt.from));
		pkt.from_pgpool_port = pool_config->port;

		pkt.status = p->status;

		/* authentication key */
		if (strlen(pool_config->wd_authkey))
		{
			/* calculate hash from packet */
			pack_str_len = packet_to_string_hb(pkt, pack_str, sizeof(pack_str));
			wd_calc_hash(pack_str, pack_str_len, pkt.hash);
		}

		/* send heartbeat signal */
		wd_hb_send(sock, &pkt, sizeof(pkt), hb_if.addr, hb_if.dest_port);
		pool_debug("wd_hb_sender: send heartbeat signal to %s:%d", hb_if.addr, hb_if.dest_port);
		sleep(pool_config->wd_heartbeat_keepalive);
	}

	return pid;
}

static RETSIGTYPE
hb_sender_exit(int sig)
{
	switch (sig)
	{
		case SIGTERM:	/* smart shutdown */
		case SIGINT:	/* fast shutdown */
		case SIGQUIT:	/* immediate shutdown */
			pool_debug("hb_sender child receives shutdown request signal %d", sig);
			break;
		default:
			pool_error("hb_sender child receives unknown signal %d", sig);
	}
	
	exit(0);
}

static RETSIGTYPE
hb_receiver_exit(int sig)
{
	switch (sig)
	{
		case SIGTERM:	/* smart shutdown */
		case SIGINT:	/* fast shutdown */
		case SIGQUIT:	/* immediate shutdown */
			pool_debug("hb_receiver child receives shutdown request signal %d", sig);
			break;
		default:
			pool_error("hb_receiver child receives unknown signal %d", sig);
	}
	
	exit(0);
}

static int
hton_wd_hb_packet(WdHbPacket * to, WdHbPacket * from)
{
	if ((to == NULL) || (from == NULL))
	{
		return WD_NG;
	}

	to->status = htonl(from->status);
	to->send_time.tv_sec = htonl(from->send_time.tv_sec);
	to->send_time.tv_usec = htonl(from->send_time.tv_usec);
	memcpy(to->from, from->from, sizeof(to->from));
	to->from_pgpool_port = htonl(from->from_pgpool_port);
	memcpy(to->hash, from->hash, sizeof(to->hash));

	return WD_OK;
}

static int
ntoh_wd_hb_packet(WdHbPacket * to, WdHbPacket * from)
{
	if ((to == NULL) || (from == NULL))
	{
		return WD_NG;
	}

	to->status = ntohl(from->status);
	to->send_time.tv_sec = ntohl(from->send_time.tv_sec);
	to->send_time.tv_usec = ntohl(from->send_time.tv_usec);
	memcpy(to->from, from->from, sizeof(to->from));
	to->from_pgpool_port = ntohl(from->from_pgpool_port);
	memcpy(to->hash, from->hash, sizeof(to->hash));

	return WD_OK;
}

/* convert packet to string and return length of the string */
static int packet_to_string_hb(WdHbPacket pkt, char *str, int maxlen)
{
	int len;
	len = snprintf(str, maxlen, "status=%d tv_sec=%ld tv_usec=%ld from=%s",
	               pkt.status, pkt.send_time.tv_sec, pkt.send_time.tv_usec, pkt.from);

	return len;
}
