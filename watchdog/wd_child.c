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
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include "watchdog.h"
#include "wd_ext.h"

int wd_child(int fork_wait_time);
static void child_exit(int exit_signo);
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

	signal(SIGTERM, child_exit);
	signal(SIGINT, child_exit);
	signal(SIGQUIT, child_exit);
	signal(SIGCHLD, child_exit);
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
		child_exit(15);
	}
	sock = wd_create_recv_socket(WD_List->wd_port);
	if (sock < 0)
	{
		/* socket create failed */
		child_exit(15);
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
child_exit(int exit_signo)
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
		default:
			send_packet.packet_no = WD_INVALID;
			memcpy(&(send_packet.wd_info), WD_List, sizeof(WdInfo));
			break;
	}

	/* send response packet */
	rtn = wd_send_packet(sock, &send_packet);
	return rtn;
}
