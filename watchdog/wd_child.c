/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
<<<<<<< HEAD
 * Copyright (c) 2003-2012	PgPool Global Development Group
=======
 * Copyright (c) 2003-2011	PgPool Global Development Group
>>>>>>> 5fb22fa044700b511711f8808d2988c2147ff331
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
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
<<<<<<< HEAD
#include "pool.h"
=======
>>>>>>> 5fb22fa044700b511711f8808d2988c2147ff331
#include "watchdog.h"
#include "wd_ext.h"

int wd_child(int fork_wait_time);
<<<<<<< HEAD
static void wd_child_exit(int exit_signo);
=======
static void child_exit(int exit_signo);
>>>>>>> 5fb22fa044700b511711f8808d2988c2147ff331
static int send_response(int sock, WdPacket * recv_pack);

int
wd_child(int fork_wait_time)
{
	int sock;
	int fd;
	int rtn;
	pid_t pgid = 0;
	pid_t pid = 0;

	pgid = getpgid(0);
	pid = fork();
	if (pid != 0)
	{
		return WD_OK;
	}

<<<<<<< HEAD
	signal(SIGTERM, wd_child_exit);
	signal(SIGINT, wd_child_exit);
	signal(SIGQUIT, wd_child_exit);
	signal(SIGCHLD, wd_child_exit);
=======
	signal(SIGTERM, child_exit);
	signal(SIGINT, child_exit);
	signal(SIGQUIT, child_exit);
	signal(SIGCHLD, child_exit);
>>>>>>> 5fb22fa044700b511711f8808d2988c2147ff331
	signal(SIGHUP, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
	setpgid(0,pgid);

	if (fork_wait_time > 0)
	{
		sleep(fork_wait_time);
	}
	if (WD_List == NULL)
	{
		/* memory allocate is not ready */
<<<<<<< HEAD
		wd_child_exit(15);
=======
		child_exit(15);
>>>>>>> 5fb22fa044700b511711f8808d2988c2147ff331
	}
	sock = wd_create_recv_socket(WD_List->wd_port);
	if (sock < 0)
	{
		/* socket create failed */
<<<<<<< HEAD
		wd_child_exit(15);
=======
		child_exit(15);
>>>>>>> 5fb22fa044700b511711f8808d2988c2147ff331
	}
	/* child loop */
	for(;;)
	{
		WdPacket buf;
		fd = wd_accept(sock);
		if (fd < 0)
		{
			continue;
		}
		rtn = wd_recv_packet(fd, &buf);
		if (rtn == WD_OK)
		{
			send_response(fd,&buf);
		}
		close(fd);
	}
}

static void
<<<<<<< HEAD
wd_child_exit(int exit_signo)
=======
child_exit(int exit_signo)
>>>>>>> 5fb22fa044700b511711f8808d2988c2147ff331
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
send_response(int sock, WdPacket * recv_pack)
{
	int rtn = WD_NG;
	WdInfo * p;
	WdPacket send_packet;
	struct timeval tv;

	if (recv_pack == NULL)
	{
		return rtn;
	}
	memset(&send_packet, 0, sizeof(WdPacket));
	p = &(recv_pack->wd_info);	

	/* set response packet no */
	switch (recv_pack->packet_no)
	{
		case WD_ADD_REQ:
			p = &(recv_pack->wd_info);	
			if (wd_set_wd_list(p->hostname,p->pgpool_port, p->wd_port, &(p->tv), p->status) > 0)
			{
				send_packet.packet_no = WD_ADD_ACCEPT;
			}
			else
			{
				send_packet.packet_no = WD_ADD_REJECT;
			}
			memcpy(&(send_packet.wd_info), WD_List, sizeof(WdInfo));
			break;
		case WD_STAND_FOR_MASTER:
			p = &(recv_pack->wd_info);	
			wd_set_wd_list(p->hostname,p->pgpool_port, p->wd_port, &(p->tv), p->status);
			/* check exist master */
			if ((p = wd_is_exist_master()) != NULL)
			{
				/* vote against the candidate */
				send_packet.packet_no = WD_MASTER_EXIST;
				memcpy(&(send_packet.wd_info), p, sizeof(WdInfo));
			}
			else
			{
				if (WD_List->tv.tv_sec <= p->tv.tv_sec )
				{
					memcpy(&tv,&(p->tv),sizeof(struct timeval));
					tv.tv_sec += 1;
					wd_set_myself(&tv, WD_NORMAL);
				}
				/* vote for the candidate */
				send_packet.packet_no = WD_VOTE_YOU;
				memcpy(&(send_packet.wd_info), WD_List, sizeof(WdInfo));
			}
			break;
		case WD_DECLARE_NEW_MASTER:
			p = &(recv_pack->wd_info);	
			wd_set_wd_list(p->hostname,p->pgpool_port, p->wd_port, &(p->tv), p->status);
			if (WD_List->status == WD_MASTER)
			{
				/* resign master server */
				wd_IP_down();
				wd_set_myself(NULL, WD_NORMAL);
			}
			send_packet.packet_no = WD_READY;
			memcpy(&(send_packet.wd_info), WD_List, sizeof(WdInfo));
			break;
		case WD_SERVER_DOWN:
			p = &(recv_pack->wd_info);	
			wd_set_wd_list(p->hostname,p->pgpool_port, p->wd_port, &(p->tv), WD_DOWN);
			send_packet.packet_no = WD_READY;
			memcpy(&(send_packet.wd_info), WD_List, sizeof(WdInfo));
			if (wd_am_I_oldest() == WD_OK)
			{
				wd_escalation();
			}
			break;
<<<<<<< HEAD
		case WD_START_RECOVERY:
			p = &(recv_pack->wd_info);	
			wd_set_wd_list(p->hostname,p->pgpool_port, p->wd_port, &(p->tv), WD_DOWN);
			send_packet.packet_no = WD_READY;
			*InRecovery = RECOVERY_ONLINE;
			break;
		case WD_END_RECOVERY:
			p = &(recv_pack->wd_info);	
			wd_set_wd_list(p->hostname,p->pgpool_port, p->wd_port, &(p->tv), WD_DOWN);
			send_packet.packet_no = WD_READY;
			*InRecovery = RECOVERY_INIT;
			break;
		default:
=======
		defalt:
>>>>>>> 5fb22fa044700b511711f8808d2988c2147ff331
			send_packet.packet_no = WD_INVALID;
			memcpy(&(send_packet.wd_info), WD_List, sizeof(WdInfo));
			break;
	}

	/* send response packet */
	rtn = wd_send_packet(sock, &send_packet);
	return rtn;
}
