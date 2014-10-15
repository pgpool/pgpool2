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
#include "pool.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/elog.h"
#include "pool_config.h"
#include "watchdog/wd_ext.h"
#include "watchdog/watchdog.h"

static void wd_child_exit(int exit_signo);
static int wd_send_response(int sock, WdPacket * recv_pack);
static void wd_node_request_signal(WD_PACKET_NO packet_no, WdNodeInfo *node);

pid_t
wd_child(int fork_wait_time)
{
	int sock;
	volatile int fd;
	int rtn;
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

	signal(SIGTERM, wd_child_exit);
	signal(SIGINT, wd_child_exit);
	signal(SIGQUIT, wd_child_exit);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);

	init_ps_display("", "", "", "");

	if (WD_List == NULL)
	{
		/* memory allocate is not ready */
		wd_child_exit(15);
	}
	/* Create per loop iteration memory context */
	ProcessLoopContext = AllocSetContextCreate(TopMemoryContext,
											   "wd_child_main_loop",
											   ALLOCSET_DEFAULT_MINSIZE,
											   ALLOCSET_DEFAULT_INITSIZE,
											   ALLOCSET_DEFAULT_MAXSIZE);
	
	MemoryContextSwitchTo(TopMemoryContext);


	sock = wd_create_recv_socket(WD_MYSELF->wd_port);

	if (sock < 0)
	{
		/* socket create failed */
		wd_child_exit(15);
	}

	set_ps_display("watchdog", false);

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

	/* child loop */
	for(;;)
	{
		MemoryContextSwitchTo(ProcessLoopContext);
		MemoryContextResetAndDeleteChildren(ProcessLoopContext);
		fd = -1;
		WdPacket buf;

		fd = wd_accept(sock);
		if (fd < 0)
		{
			continue;
		}
		rtn = wd_recv_packet(fd, &buf);
		if (rtn == WD_OK)
		{
			wd_send_response(fd, &buf);
		}
		close(fd);
	}
	return pid;
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
			send_failback_request(node->node_id_set[0]);
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
