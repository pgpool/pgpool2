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

static RETSIGTYPE writer_exit(int sig);
static RETSIGTYPE reader_exit(int sig);
static int hton_wd_udp_packet(WdUdpPacket * to, WdUdpPacket * from);
static int ntoh_wd_udp_packet(WdUdpPacket * to, WdUdpPacket * from);
static void calc_hash(WdUdpPacket pkt, char *buf);

int
wd_create_udp_send_socket(WdUdpIf udp_if)
{
	int sock;
	int tos;

	/* create socket */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		/* socket create failed */
		pool_error("wd_create_udp_send_socket: Failed to create socket. reason: %s",
		           strerror(errno));
		return -1;
	}

	/* set socket option */
	tos = IPTOS_LOWDELAY;
	if (setsockopt(sock, IPPROTO_IP, IP_TOS, (char *) &tos, sizeof(tos)) == -1 )
	{
		pool_error("wd_create_udp_send_socket: setsockopt(IP_TOS) failed. reason: %s",
		           strerror(errno));
		close(sock);
		return -1;
	}

#if defined(SO_BINDTODEVICE)
	{
		struct ifreq i;
		strlcpy(i.ifr_name, udp_if.if_name, sizeof(i.ifr_name));

		if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, &i, sizeof(i)) == -1)
		{
			pool_error("wd_create_udp_send_socket: setsockopt(SO_BINDTODEVICE) failed. reason: %s",
			           strerror(errno));
			close(sock);
			return -1;
		}
		pool_log("wd_create_udp_send_socket: bind send socket to device: %s", i.ifr_name);
	}
#endif
#if defined(SO_REUSEPORT)
	{
		int one = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) == -1)
		{
			pool_error("wd_create_udp_send_socket: setsockopt(SO_REUSEPORT) failed. reason: %s",
			           strerror(errno));
			close(sock);
			return -1;
		}
		pool_log("wd_create_udp_send_socket: set SO_REUSEPORT");
	}
#endif

 	if (fcntl(sock, F_SETFD, FD_CLOEXEC) < 0) {
		pool_error("wd_create_udp_send_socket: setting close-on-exec flag failed. reason: %s",
		           strerror(errno));
		close(sock);
		return -1;
	}

	return sock;
}

int
wd_create_udp_recv_socket(WdUdpIf udp_if)
{
	struct sockaddr_in addr;
	int sock;
	const int one = 1;
	int bind_tries;
	int bind_is_done;

	memset(&(addr), 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pool_config->wd_udp_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	/* create socket */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		/* socket create failed */
		pool_error("wd_create_udp_recv_socket: Failed to create socket. reason: %s",
		           strerror(errno));
		return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one)) == -1 )
	{
		pool_error("wd_create_udp_recv_socket: setsockopt(SO_REUSEADDR) failed. reason: %s",
		           strerror(errno));
		close(sock);
		return -1;
	}

#if defined(SO_BINDTODEVICE)
	{
		struct ifreq i;
		strlcpy(i.ifr_name, udp_if.if_name, sizeof(i.ifr_name));

		if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, &i, sizeof(i)) == -1)
		{
			pool_error("wd_create_udp_recv_socket: setsockopt(SO_BINDTODEVICE) failed. reason: %s",
			           strerror(errno));
			close(sock);
			return -1;
		}
		pool_log("wd_create_udp_recv_socket: bind receive socket to device: %s", i.ifr_name);
	}
#endif

#if defined(SO_REUSEPORT)
	{
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) == -1)
		{
			pool_error("wd_create_udp_recv_socket: setsockopt(SO_REUSEPORT) failed. reason: %s",
			           strerror(errno));
			close(sock);
			return -1;
		}
		pool_log("wd_create_udp_recv_socket: set SO_REUSEPORT");
	}
#endif

	bind_is_done = 0;
	for (bind_tries = 0; !bind_is_done && bind_tries < MAX_BIND_TRIES; bind_tries++)
	{
		if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
		{
			pool_log("wd_crate_udp_recv_socket: bind failed. reason: %s ... retrying",
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
		pool_error("wd_crate_udp_recv_socket: unable to bind socket. reason: %s",
		           strerror(errno));
		close(sock);
		return -1;
	}

 	if (fcntl(sock, F_SETFD, FD_CLOEXEC) < 0) {
		pool_error("wd_create_udp_recv_socket: setting close-on-exec flag failed. reason: %s",
		           strerror(errno));
		close(sock);
		return -1;
	}

	return sock;
}

int
wd_udp_write(int sock, WdUdpPacket * pkt, int len, const char * host)
{
	int rtn;
	struct sockaddr_in addr;
	struct hostent *hp;
	WdUdpPacket buf;

	if (!host || !strlen(host))
	{
		pool_error("wd_udp_write: host name is empty");
		return -1;
	}

	hp = gethostbyname(host);
	if ((hp == NULL) || (hp->h_addrtype != AF_INET))
	{
		pool_error("wd_udp_write: gethostbyname() failed: %s host: %s",
		           hstrerror(h_errno), host);
		return -1;
	}
	memmove((char *) &(addr.sin_addr), (char *) hp->h_addr, hp->h_length);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(pool_config->wd_udp_port);

	hton_wd_udp_packet(&buf, pkt);

	if ((rtn = sendto(sock, &buf, sizeof(WdUdpPacket), 0,
	                  (struct sockaddr *)&addr, sizeof(addr))) != len)
	{
		pool_error("wd_udp_write: failed to sent packet to %s", host);
		return WD_NG;
	}
	pool_debug("wd_udp_write: send %d byte packet", rtn);

	return WD_OK;
}

int
wd_udp_read(int sock, WdUdpPacket * pkt)
{
	int rtn;
	struct sockaddr_in senderinfo;
	socklen_t addrlen;
	WdUdpPacket buf;

	addrlen = sizeof(senderinfo);

	rtn = recvfrom(sock, &buf, sizeof(WdUdpPacket), 0,
	               (struct sockaddr *)&senderinfo, &addrlen);
	if (rtn < 0)
	{	
		pool_error("wd_udp_read: failed to receive packet");
		return WD_NG;
	}
	else if (rtn == 0)
	{
		pool_error("wd_udp_read: received zero bytes");
		return WD_NG;
	}
	else
	{
		pool_debug("wd_udp_read: received %d byte packet", rtn);
	}

	ntoh_wd_udp_packet(pkt, &buf);

	return WD_OK;
}

pid_t wd_reader(int fork_wait_time, WdUdpIf udp_if)
{
	int sock;
	pid_t pid = 0;
	WdUdpPacket pkt;
	struct timeval tv;
	char from[WD_MAX_HOST_NAMELEN];
	char buf[(MD5_PASSWD_LEN+1)*2];

	WdInfo * p;

	pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			pool_error("wd_reader: fork() failed.");

		return pid;
	}

	if (fork_wait_time > 0)
	{
		sleep(fork_wait_time);
	}

	myargv = save_ps_display_args(myargc, myargv);

	POOL_SETMASK(&UnBlockSig);

	signal(SIGTERM, reader_exit);
	signal(SIGINT, reader_exit);
	signal(SIGQUIT, reader_exit);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);

	init_ps_display("", "", "", "");

	if ( (sock = wd_create_udp_recv_socket(udp_if)) < 0)
	{
		pool_error("wd_reader: socket create failed");
		reader_exit(SIGTERM);
	}

	set_ps_display("udp reader", false);

	for(;;)
	{
		if (wd_udp_read(sock, &pkt) == WD_OK)
		{
			/* auhtentication */
			if (strlen(pool_config->wd_udp_authkey))
			{
				/* calculate hash from packet */
				calc_hash(pkt, buf);

				if (strcmp(pkt.hash, buf))
				{
					pool_log("wd_reader: authentication failed");
					continue;
				}
			}

			/* get current time */
			gettimeofday(&tv, NULL);

			/* who send this packet? */
			strlcpy(from, pkt.from, sizeof(from));

			p = WD_List;
			while (p->status != WD_END)
			{
				if (!strcmp(p->hostname, from))
				{
					/* this is the first packet or the latest packet */
					if (!WD_TIME_ISSET(p->udp_send_time) ||
					    WD_TIME_BEFORE(p->udp_send_time, pkt.send_time))
					{
						pool_debug("wd_reader: received heartbeat signal from %s", from);
						p->udp_send_time = pkt.send_time;
						p->udp_last_recv_time = tv;
					}
					break;
				}
				p++;
			}
		}
	}

	return pid;
}

pid_t wd_writer(int fork_wait_time, WdUdpIf udp_if)
{
	int sock;
	pid_t pid = 0;
	WdUdpPacket pkt;
	WdInfo * p = WD_List;

	pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			pool_error("wd_writer: fork() failed.");

		return pid;
	}

	if (fork_wait_time > 0)
	{
		sleep(fork_wait_time);
	}

	myargv = save_ps_display_args(myargc, myargv);

	POOL_SETMASK(&UnBlockSig);

	signal(SIGTERM, writer_exit);
	signal(SIGINT, writer_exit);
	signal(SIGQUIT, writer_exit);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);

	init_ps_display("", "", "", "");

	if ( (sock = wd_create_udp_send_socket(udp_if)) < 0)
	{
		pool_error("wd_writer: socket create failed");
		writer_exit(SIGTERM);
	}

	set_ps_display("udp writer", false);

	for(;;)
	{
		gettimeofday(&pkt.send_time, NULL);
		strlcpy(pkt.from, pool_config->wd_hostname, sizeof(pkt.from));

		pkt.status = p->status;

		if (strlen(pool_config->wd_udp_authkey))
		{
			/* calculate hash from packet */
			calc_hash(pkt, pkt.hash);
		}

		wd_udp_write(sock, &pkt, sizeof(pkt), udp_if.addr); 
		pool_debug("wd_writer: send heartbeat signal to %s", udp_if.addr);
		sleep(pool_config->wd_udp_keepalive);
	}

	return pid;
}

static RETSIGTYPE
writer_exit(int sig)
{
	switch (sig)
	{
		case SIGTERM:	/* smart shutdown */
		case SIGINT:	/* fast shutdown */
		case SIGQUIT:	/* immediate shutdown */
			pool_debug("writer child receives shutdown request signal %d", sig);
			break;
		default:
			pool_error("writer child receives unknown signal %d", sig);
	}
	
	exit(0);
}

static RETSIGTYPE
reader_exit(int sig)
{
	switch (sig)
	{
		case SIGTERM:	/* smart shutdown */
		case SIGINT:	/* fast shutdown */
		case SIGQUIT:	/* immediate shutdown */
			pool_debug("reader child receives shutdown request signal %d", sig);
			break;
		default:
			pool_error("reader child receives unknown signal %d", sig);
	}
	
	exit(0);
}

static int
hton_wd_udp_packet(WdUdpPacket * to, WdUdpPacket * from)
{
	if ((to == NULL) || (from == NULL))
	{
		return WD_NG;
	}

	to->status = htonl(from->status);
	to->send_time.tv_sec = htonl(from->send_time.tv_sec);
	to->send_time.tv_usec = htonl(from->send_time.tv_usec);
	memcpy(to->from, from->from, sizeof(to->from));
	memcpy(to->hash, from->hash, sizeof(to->hash));

	return WD_OK;
}

static int
ntoh_wd_udp_packet(WdUdpPacket * to, WdUdpPacket * from)
{
	if ((to == NULL) || (from == NULL))
	{
		return WD_NG;
	}

	to->status = ntohl(from->status);
	to->send_time.tv_sec = ntohl(from->send_time.tv_sec);
	to->send_time.tv_usec = ntohl(from->send_time.tv_usec);
	memcpy(to->from, from->from, sizeof(to->from));
	memcpy(to->hash, from->hash, sizeof(to->hash));

	return WD_OK;
}

/* calculate hash for authentiction using packet contents */
static void 
calc_hash(WdUdpPacket pkt, char *buf)
{
	char pass[(MAX_PASSWORD_SIZE + 1) / 2];
	char username[(MAX_PASSWORD_SIZE + 1) / 2];
	char salt[WD_MAX_SALT];
	size_t pass_len;
	size_t username_len;
	size_t salt_len;
	size_t authkey_len;

	/* use first half of authkey as username, last half as password */
	authkey_len = strlen(pool_config->wd_udp_authkey);

	username_len = authkey_len / 2;
	pass_len = authkey_len - username_len;
	snprintf(username, username_len + 1, pool_config->wd_udp_authkey);
	snprintf(pass, pass_len + 1, pool_config->wd_udp_authkey + username_len);

	/* convert pakcet to string and use this as salt */
	salt_len = snprintf(salt, sizeof(salt), "status=%d tv_sec=%ld tv_usec=%ld from=%s",
	                    pkt.status, pkt.send_time.tv_sec, pkt.send_time.tv_usec, pkt.from);

	/* calculate hash using md5 encrypt */
	pool_md5_encrypt(pass, username, strlen(username), buf + MD5_PASSWD_LEN + 1);
	buf[(MD5_PASSWD_LEN+1)*2-1] = '\0';

	pool_md5_encrypt(buf+MD5_PASSWD_LEN+1, salt, salt_len, buf);
	buf[MD5_PASSWD_LEN] = '\0';
}
