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
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/elog.h"
#include "pool_config.h"
#include "auth/md5.h"
#include "watchdog/watchdog.h"
#include "watchdog/wd_ext.h"

#define MAX_BIND_TRIES 5

static RETSIGTYPE hb_sender_exit(int sig);
static RETSIGTYPE hb_receiver_exit(int sig);
static int hton_wd_hb_packet(WdHbPacket *to, WdHbPacket *from);
static int ntoh_wd_hb_packet(WdHbPacket *to, WdHbPacket *from);
static int packet_to_string_hb(WdHbPacket *pkt, char * str, int maxlen);
static void wd_set_reuseport(int sock);

/* create socket for sending heartbeat */
int
wd_create_hb_send_socket(WdHbIf *hb_if)
{
	int sock;
	int tos;

	/* create socket */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		/* socket create failed */
		ereport(ERROR,
			(errmsg("failed to create watchdog heartbeat sender socket"),
				 errdetail("create socket failed with reason: \"%s\"", strerror(errno))));
	}

	/* set socket option */
	tos = IPTOS_LOWDELAY;
	if (setsockopt(sock, IPPROTO_IP, IP_TOS, (char *) &tos, sizeof(tos)) == -1 )
	{
		close(sock);
		ereport(ERROR,
			(errmsg("failed to create watchdog heartbeat sender socket"),
				 errdetail("setsockopt(IP_TOS) failed with reason: \"%s\"", strerror(errno))));
	}

	if (hb_if->if_name[0] != '\0')
	{
#if defined(SO_BINDTODEVICE)
		{
			if (geteuid() == 0) /* check root privileges */
			{
				struct ifreq i;
				strlcpy(i.ifr_name, hb_if->if_name, sizeof(i.ifr_name));

				if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, &i, sizeof(i)) == -1)
				{
					close(sock);
					ereport(ERROR,
						(errmsg("failed to create watchdog heartbeat sender socket"),
							 errdetail("setsockopt(SO_BINDTODEVICE) failed with reason: \"%s\"", strerror(errno))));

				}
				ereport(LOG,
					(errmsg("creating socket for sending heartbeat"),
						 errdetail("bind send socket to device: %s", i.ifr_name)));
			}
			else
				ereport(LOG,
					(errmsg("creating socket for sending heartbeat"),
						 errdetail("setsockopt(SO_BINDTODEVICE) requires root privilege")));
		}
#else
		ereport(LOG,
			(errmsg("creating socket for sending heartbeat"),
				 errdetail("setsockopt(SO_BINDTODEVICE) is not available on this platform")));
#endif
	}

	wd_set_reuseport(sock);
	ereport(LOG,
		(errmsg("creating socket for sending heartbeat"),
			 errdetail("set SO_REUSEPORT")));

 	if (fcntl(sock, F_SETFD, FD_CLOEXEC) < 0)
	{
		close(sock);
		ereport(ERROR,
				(errmsg("failed to create watchdog heartbeat sender socket"),
				 errdetail("setting close-on-exec flag failed with reason: \"%s\"", strerror(errno))));
	}

	return sock;
}

/* create socket for receiving heartbeat */
int
wd_create_hb_recv_socket(WdHbIf *hb_if)
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
		ereport(ERROR,
			(errmsg("failed to create watchdog heartbeat receive socket"),
				 errdetail("create socket failed with reason: \"%s\"", strerror(errno))));
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one)) == -1 )
	{
		close(sock);
		ereport(ERROR,
			(errmsg("failed to create watchdog heartbeat receive socket"),
				 errdetail("setsockopt(SO_REUSEADDR) failed with reason: \"%s\"", strerror(errno))));
	}

	if (hb_if->if_name[0] != '\0')
	{
#if defined(SO_BINDTODEVICE)
		{
			if (geteuid() == 0) /* check root privileges */
			{
				struct ifreq i;
				strlcpy(i.ifr_name, hb_if->if_name, sizeof(i.ifr_name));

				if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, &i, sizeof(i)) == -1)
				{
					close(sock);
					ereport(ERROR,
						(errmsg("failed to create watchdog heartbeat receive socket"),
							 errdetail("setsockopt(SO_BINDTODEVICE) failed with reason: \"%s\"", strerror(errno))));
				}
				ereport(LOG,
					(errmsg("createing watchdog heartbeat receive socket."),
						 errdetail("bind receive socket to device: \"%s\"", i.ifr_name)));

			}
			else
				ereport(LOG,
					(errmsg("failed to create watchdog heartbeat receive socket."),
						 errdetail("setsockopt(SO_BINDTODEVICE) requies root privilege")));
		}
#else
		ereport(LOG,
			(errmsg("failed to create watchdog heartbeat receive socket"),
				 errdetail("setsockopt(SO_BINDTODEVICE) is not available on this platform")));
#endif
	}

	wd_set_reuseport(sock);

	ereport(LOG,
		(errmsg("creating watchdog heartbeat receive socket."),
			 errdetail("set SO_REUSEPORT")));

	bind_is_done = 0;
	for (bind_tries = 0; !bind_is_done && bind_tries < MAX_BIND_TRIES; bind_tries++)
	{
		if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
		{
			ereport(LOG,
				(errmsg("failed to create watchdog heartbeat receive socket. retrying..."),
					errdetail("bind failed with reason: \"%s\"",
						   strerror(errno))));

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
		close(sock);
		ereport(ERROR,
			(errmsg("failed to create watchdog heartbeat receive socket"),
				 errdetail("bind socket failed with reason: \"%s\"", strerror(errno))));
	}

 	if (fcntl(sock, F_SETFD, FD_CLOEXEC) < 0)
	{
		close(sock);
		ereport(ERROR,
			(errmsg("failed to create watchdog heartbeat receive socket"),
				 errdetail("setting close-on-exec flag failed with reason: \"%s\"", strerror(errno))));
	}

	return sock;
}

/* send heartbeat signal */
void
wd_hb_send(int sock, WdHbPacket * pkt, int len, const char * host, const int port)
{
	int rtn;
	struct sockaddr_in addr;
	struct hostent *hp;
	WdHbPacket buf;

	if (!host || !strlen(host))
		ereport(ERROR,
			(errmsg("failed to send watchdog heartbeat. host name is empty")));

	hp = gethostbyname(host);
	if ((hp == NULL) || (hp->h_addrtype != AF_INET))
		ereport(ERROR,
			(errmsg("failed to send watchdog heartbeat, gethostbyname() failed"),
				 errdetail("gethostbyname on \"%s\" failed with reason: \"%s\"",host, hstrerror(h_errno))));

	memmove((char *) &(addr.sin_addr), (char *) hp->h_addr, hp->h_length);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	hton_wd_hb_packet(&buf, pkt);

	if ((rtn = sendto(sock, &buf, sizeof(WdHbPacket), 0,
	                  (struct sockaddr *)&addr, sizeof(addr))) != len)
	{
		ereport(ERROR,
			(errmsg("failed to send watchdog heartbeat, sendto failed"),
				 errdetail("sending packet to \"%s\" failed with reason: \"%s\"",host, strerror(errno))));

	}
	ereport(DEBUG2,
			(errmsg("watchdog heartbeat: send %d byte packet", rtn)));
}

/* receive heartbeat signal */
void
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
		ereport(ERROR,
			(errmsg("failed to receive heartbeat packet")));
	else if (rtn == 0)
		ereport(ERROR,
				(errmsg("failed to receive heartbeat received zero length packet")));
	else
		ereport(DEBUG2,
				(errmsg("watchdog heartbeat: received %d byte packet", rtn)));

	ntoh_wd_hb_packet(pkt, &buf);
}

/* fork heartbeat receiver child */
pid_t
wd_hb_receiver(int fork_wait_time, WdHbIf *hb_if)
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
	sigjmp_buf	local_sigjmp_buf;


	WdInfo * p;

	pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			ereport(PANIC,
					(errmsg("failed to fork a heartbeat receiver child")));
		return pid;
	}

	on_exit_reset();
	processType = PT_HB_RECEIVER;

	if (fork_wait_time > 0)
	{
		sleep(fork_wait_time);
	}

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
	/* Create per loop iteration memory context */
	ProcessLoopContext = AllocSetContextCreate(TopMemoryContext,
											   "wdhb_hb_receiver",
											   ALLOCSET_DEFAULT_MINSIZE,
											   ALLOCSET_DEFAULT_INITSIZE,
											   ALLOCSET_DEFAULT_MAXSIZE);
	
	MemoryContextSwitchTo(TopMemoryContext);

	sock = wd_create_hb_recv_socket(hb_if);

	set_ps_display("heartbeat receiver", false);

	if (sigsetjmp(local_sigjmp_buf, 1) != 0)
	{
		/* Since not using PG_TRY, must reset error stack by hand */
		error_context_stack = NULL;
		
		EmitErrorReport();
		MemoryContextSwitchTo(TopMemoryContext);
		FlushErrorState();
	}
	
	/* We can now handle ereport(ERROR) */
	PG_exception_stack = &local_sigjmp_buf;

	for(;;)
	{
		MemoryContextSwitchTo(ProcessLoopContext);
		MemoryContextResetAndDeleteChildren(ProcessLoopContext);

		/* receive heartbeat signal */
		wd_hb_recv(sock, &pkt);
			/* authentication */
		if (strlen(pool_config->wd_authkey))
		{
			/* calculate hash from packet */
			pack_str_len = packet_to_string_hb(&pkt, pack_str, sizeof(pack_str));
			wd_calc_hash(pack_str, pack_str_len, buf);

			if (strcmp(pkt.hash, buf))
				ereport(ERROR,
					(errmsg("watchdog heartbeat receive"),
						 errdetail("authentication failed")));
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
				/* ignore the packet from down pgpool */
				if (pkt.status == WD_DOWN)
				{
					ereport(DEBUG1,
						(errmsg("watchdog heartbeat: received heartbeat signal from \"%s:%d\" whose status is down. ignored",
									from, from_pgpool_port)));
					break;
				}

				/* this is the first packet or the latest packet */
				if (!WD_TIME_ISSET(p->hb_send_time) ||
					WD_TIME_BEFORE(p->hb_send_time, pkt.send_time))
				{
					ereport(DEBUG1,
							(errmsg("watchdog heartbeat: received heartbeat signal from \"%s:%d\"",
									from, from_pgpool_port)));

					p->hb_send_time = pkt.send_time;
					p->hb_last_recv_time = tv;
				}
				else
				{
					ereport(DEBUG1,
							(errmsg("watchdog heartbeat: received heartbeat signal is older than the latest, ignored")));
				}
				break;
			}
			p++;
		}

	}

	return pid;
}

/* fork heartbeat sender child */
pid_t
wd_hb_sender(int fork_wait_time, WdHbIf *hb_if)
{
	int sock;
	pid_t pid = 0;
	WdHbPacket pkt;
	WdInfo * p = WD_List;
	char pack_str[WD_MAX_PACKET_STRING];
	int pack_str_len;
	sigjmp_buf	local_sigjmp_buf;

	pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			ereport(PANIC,
					(errmsg("failed to fork a heartbeat sender child")));
		return pid;
	}

	on_exit_reset();
	processType = PT_HB_SENDER;

	if (fork_wait_time > 0)
	{
		sleep(fork_wait_time);
	}

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
	/* Create per loop iteration memory context */
	ProcessLoopContext = AllocSetContextCreate(TopMemoryContext,
											   "wdhb_sender",
											   ALLOCSET_DEFAULT_MINSIZE,
											   ALLOCSET_DEFAULT_INITSIZE,
											   ALLOCSET_DEFAULT_MAXSIZE);
	
	MemoryContextSwitchTo(TopMemoryContext);

	sock = wd_create_hb_send_socket(hb_if);

	set_ps_display("heartbeat sender", false);

	if (sigsetjmp(local_sigjmp_buf, 1) != 0)
	{
		/* Since not using PG_TRY, must reset error stack by hand */
		error_context_stack = NULL;
		
		EmitErrorReport();
		MemoryContextSwitchTo(TopMemoryContext);
		FlushErrorState();
		sleep(pool_config->wd_heartbeat_keepalive);
	}
	
	/* We can now handle ereport(ERROR) */
	PG_exception_stack = &local_sigjmp_buf;

	for(;;)
	{
		MemoryContextSwitchTo(ProcessLoopContext);
		MemoryContextResetAndDeleteChildren(ProcessLoopContext);

		/* contents of packet */
		gettimeofday(&pkt.send_time, NULL);
		strlcpy(pkt.from, pool_config->wd_hostname, sizeof(pkt.from));
		pkt.from_pgpool_port = pool_config->port;

		pkt.status = p->status;

		/* authentication key */
		if (strlen(pool_config->wd_authkey))
		{
			/* calculate hash from packet */
			pack_str_len = packet_to_string_hb(&pkt, pack_str, sizeof(pack_str));
			wd_calc_hash(pack_str, pack_str_len, pkt.hash);
		}

		/* send heartbeat signal */
		wd_hb_send(sock, &pkt, sizeof(pkt), hb_if->addr, hb_if->dest_port);
		ereport(DEBUG1,
				(errmsg("watchdog heartbeat: send heartbeat signal to %s:%d", hb_if->addr, hb_if->dest_port)));
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
			ereport(DEBUG1,
					(errmsg("watchdog heartbeat sender child receives shutdown request signal %d", sig )));
			break;
		default:
			ereport(LOG,
					(errmsg("hb_sender child receives unknown signal: %d", sig )));
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
			ereport(DEBUG1,
				(errmsg("watchdog heartbeat receiver child receives shutdown request signal %d", sig )));
			break;
		default:
			ereport(LOG,
					(errmsg("hb_receiver child receives unknown signal: %d", sig )));
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
static int
packet_to_string_hb(WdHbPacket *pkt, char *str, int maxlen)
{
	int len;
	len = snprintf(str, maxlen, "status=%d tv_sec=%ld tv_usec=%ld from=%s",
	               pkt->status, pkt->send_time.tv_sec, pkt->send_time.tv_usec, pkt->from);

	return len;
}

/*
 * Set SO_REUSEPORT option to the socket.  If the option is available
 * in the compile time but not available in the run time, just emit a
 * log and treat it as normal.
 */
static void wd_set_reuseport(int sock)
{
#if defined(SO_REUSEPORT)
	int one = 1;

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) == -1)
	{
		if (errno == EINVAL || errno == ENOPROTOOPT)
		{
			ereport(LOG,
				(errmsg("error seting SO_REUSEPORT option to the socket"),
					 errdetail("setsockopt(SO_REUSEPORT) is not supported by the kernel. detail: %s",
							   strerror(errno))));
		}
		else
		{
			close(sock);
			ereport(ERROR,
				(errmsg("failed to create watchdog heartbeat socket"),
					 errdetail("setsockopt(SO_REUSEPORT) failed with reason: \"%s\"", strerror(errno))));
		}
	}
	else
		ereport(LOG,
				(errmsg("set SO_REUSEPORT option to the socket")));

#endif
}
