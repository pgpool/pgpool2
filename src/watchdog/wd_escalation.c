/*
 *
 * wd_escalation
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
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "pool.h"
#include "utils/elog.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "pool_config.h"
#include "watchdog/watchdog.h"
#include "watchdog/wd_ext.h"

#include "query_cache/pool_memqcache.h"

#include <sys/socket.h>
#ifdef __linux__
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#else	/* __linux__ */
#include <net/route.h>
#include <net/if.h>

#ifdef AF_LINK
#include <net/if_dl.h>
#endif
#endif	/* __linux__ */
#include <arpa/inet.h>

static int open_monitoring_socket(void);
static int watch_interface_changes(int sock);

static void
wd_exit(int exit_signo)
{
	sigset_t mask;
	
	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, NULL);
	
//	wd_notice_server_down();
	
	exit(0);
}

typedef enum NETWORK_EVENTS
{
	INTERFACE_DOWN,
	INTERFACE_UP,
	VIP_DOWN,
	VIP_UP
}NETWORK_EVENTS;


static int create_monitoring_socket(void)
{
	int sock = -1;
#ifdef __linux__
	sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);

#else
	sock = socket(PF_ROUTE, SOCK_RAW, AF_UNSPEC);
#endif
	if (sock < 0)
		ereport(ERROR,
			(errmsg("watchdog: VIP monitoring failed to create socket"),
				 errdetail("socket() failed with error \"%s\"",strerror(errno))));

#ifdef __linux__
	struct sockaddr_nl addr;

	memset(&addr, 0x00, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_groups = RTMGRP_IPV4_IFADDR;

	if (bind(sock,(struct sockaddr *)&addr,sizeof(addr)) < 0)
	{
		close(sock);
		ereport(ERROR,
			(errmsg("watchdog: VIP monitoring failed to bind socket"),
				 errdetail("bind() failed with error \"%s\"",strerror(errno))));
	}
#endif

	return sock;
}

#ifdef __linux__
static int watch_interface_changes(int sock)
{
	char buffer[4096];
	int len;
	struct iovec iov;
	struct msghdr hdr;
	struct ifaddrmsg *ifa;
	struct nlmsghdr *nlhdr;
	struct rtattr *rta;
	int ifa_len;

	iov.iov_base = buffer;
	iov.iov_len = sizeof(buffer);

	memset(&hdr, 0, sizeof(hdr));
	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;

	len = recvmsg(sock, &hdr, 0);
	if (len < 0)
		ereport(ERROR,
			(errmsg("VIP monitoring failed to receive from socket"),
				 errdetail("recvmsg() failed with error \"%s\"",strerror(errno))));

	nlhdr = (struct nlmsghdr *)buffer;
	
	for (; NLMSG_OK(nlhdr, len) ;nlhdr = NLMSG_NEXT(nlhdr, len))
	{
		bool deladdr = false;
		char address[48];
		char ifname[256];
		address[0] = '\0';
		ifname[0] = '\0';

		if(nlhdr->nlmsg_type == NLMSG_DONE)
			break;

		switch(nlhdr->nlmsg_type)
		{
			case RTM_DELADDR:
				deladdr = true; /* fallthrough */
			case RTM_NEWADDR:
				/* code from http://linux-hacks.blogspot.fr/2009/01/sample-code-to-learn-netlink.html */

				ifa = (struct ifaddrmsg *)NLMSG_DATA(nlhdr);
				rta = (struct rtattr *)IFA_RTA(ifa);
				ifa_len = IFA_PAYLOAD(nlhdr);

				ereport(DEBUG1,
					(errmsg("VIP monitoring new event %s", deladdr?"RTM_DELADDR" : "RTM_NEWADDR"),
						errdetail("index=%d fam=%d prefixlen=%d flags=%d scope=%d",
							   ifa->ifa_index, ifa->ifa_family, ifa->ifa_prefixlen,
							   ifa->ifa_flags, ifa->ifa_scope)));

				for(;RTA_OK(rta, ifa_len); rta = RTA_NEXT(rta, ifa_len)) {
					/*RTA_DATA(rta)*/
					/*rta_type : IFA_ADDRESS, IFA_LOCAL, etc. */
					char tmp[128];
					memset(tmp, 0, sizeof(tmp));

					switch(rta->rta_type)
					{
						case IFA_ADDRESS:
						case IFA_LOCAL:
						case IFA_BROADCAST:
						case IFA_ANYCAST:
							inet_ntop(ifa->ifa_family, RTA_DATA(rta), tmp, sizeof(tmp));
							if(rta->rta_type == IFA_ADDRESS)
								strncpy(address, tmp, sizeof(address));
							break;
						case IFA_LABEL:
							strncpy(tmp, RTA_DATA(rta), sizeof(tmp));
							strncpy(ifname, tmp, sizeof(ifname));
							break;
						case IFA_CACHEINFO:
						{
							struct ifa_cacheinfo *cache_info;
							cache_info = RTA_DATA(rta);
							snprintf(tmp, sizeof(tmp), "valid=%u prefered=%u",
									 cache_info->ifa_valid, cache_info->ifa_prefered);
						}
							break;
						default:
							strncpy(tmp, "*unknown*", sizeof(tmp));
					}
					ereport(DEBUG2,
							(errmsg("rta_len=%d rta_type=%d '%s'", rta->rta_len, rta->rta_type, tmp)));
				}

				ereport(DEBUG2,
						(errmsg("%s: %s/%d %s",deladdr ? "RTM_DELADDR" : "RTM_NEWADDR",address, ifa->ifa_prefixlen, ifname)));
				/* check of the address is our VIP */
				if (strcmp(pool_config->delegate_IP,address) == 0 )
				{
					ereport(WARNING,
							(errmsg("VIP monitoring detected VIP is %s",deladdr ?"droped":"brought up")));
//					WD_List->delegate_ip_flag = 0; //TEMP
//					wd_IP_up();
				}
				break;
			default:
				ereport(DEBUG2,
						(errmsg("unknown nlmsg_type=%d", nlhdr->nlmsg_type)));
		}
	}
}

#else /* For non linux chaps */
#if defined(__OpenBSD__) || defined(__FreeBSD__)
#define SALIGN (sizeof(long) - 1)
#else
#define SALIGN (sizeof(int32_t) - 1)
#endif

#define SA_RLEN(sa) ((sa)->sa_len ? (((sa)->sa_len + SALIGN) & ~SALIGN) : (SALIGN + 1))
/* With the help from https://github.com/miniupnp/miniupnp/blob/master/minissdpd/ifacewatch.c */
static int watch_interface_changes(int sock)
{
	return -1;
	char buffer[4096];
	int len;
	struct ifa_msghdr *ifam;
	struct rt_msghdr *nlhdr;
	struct sockaddr * sa;
	bool deladdr = false;
	char * p;
	int addr;
	int prefixlen = 0;
	char tmp[64];
	int family = AF_UNSPEC;
//	struct rtattr *rta;
//	int ifa_len;

	char address[48];
	char ifname[256];
	address[0] = '\0';
	ifname[0] = '\0';

	len = recv(sock, buffer, sizeof(buffer), 0);
	if (len < 0)
		ereport(ERROR,
			(errmsg("VIP monitoring failed to receive from socket"),
				 errdetail("recv() failed with error \"%s\"",strerror(errno))));
	
	nlhdr = (struct rt_msghdr *)buffer;

	switch(nlhdr->rtm_type)
	{
		case RTM_DELADDR:
			deladdr = true; /* fallthrough */
		case RTM_NEWADDR:
			ifam = (struct ifa_msghdr *)buffer;

			p = buffer + sizeof(struct ifa_msghdr);
			addr = 1;
			while(p < buffer + len) {
				sa = (struct sockaddr *)p;
				while(!(addr & ifam->ifam_addrs) && (addr <= ifam->ifam_addrs))
					addr = addr << 1;
				inet_ntop(sa->sa_family,
						  &((struct sockaddr_in *)sa)->sin_addr,
						  tmp, sizeof(tmp));

//				sockaddr_to_string(sa, tmp, sizeof(tmp));
//				syslog(LOG_DEBUG, " %s", tmp);
				switch(addr) {
					case RTA_DST:
					case RTA_GATEWAY:
						break;
					case RTA_NETMASK:
						if(sa->sa_family == AF_INET
#if defined(__OpenBSD__)
						   || (sa->sa_family == 0 &&
							   sa->sa_len <= sizeof(struct sockaddr_in))
#endif
						   ) {
							uint32_t sin_addr = ntohl(((struct sockaddr_in *)sa)->sin_addr.s_addr);
							while((prefixlen < 32) &&
								  ((sin_addr & (1 << (31 - prefixlen))) != 0))
								prefixlen++;
						} else if(sa->sa_family == AF_INET6
#if defined(__OpenBSD__)
								  || (sa->sa_family == 0 &&
									  sa->sa_len == sizeof(struct sockaddr_in6))
#endif
								  ) {
							int i = 0;
							uint8_t * q =  ((struct sockaddr_in6 *)sa)->sin6_addr.s6_addr;
							while((*q == 0xff) && (i < 16)) {
								prefixlen += 8;
								q++; i++;
							}
							if(i < 16) {
								i = 0;
								while((i < 8) &&
									  ((*q & (1 << (7 - i))) != 0))
									i++;
								prefixlen += i;
							}
						}
						break;
					case RTA_GENMASK:
						break;
					case RTA_IFP:
#ifdef AF_LINK
						if(sa->sa_family == AF_LINK) {
							struct sockaddr_dl * sdl = (struct sockaddr_dl *)sa;
							memset(ifname, 0, sizeof(ifname));
							memcpy(ifname, sdl->sdl_data, sdl->sdl_nlen);
						}
#endif
						break;
					case RTA_IFA:
						family = sa->sa_family;
						if(sa->sa_family == AF_INET) {
							inet_ntop(sa->sa_family,
									  &((struct sockaddr_in *)sa)->sin_addr,
									  address, sizeof(address));
						} else if(sa->sa_family == AF_INET6) {
							inet_ntop(sa->sa_family,
									  &((struct sockaddr_in6 *)sa)->sin6_addr,
									  address, sizeof(address));
						}
						break;
					case RTA_AUTHOR:
						break;
					case RTA_BRD:
						break;
				}
				p += SA_RLEN(sa);
				addr = addr << 1;
			}
//			ereport(DEBUG2,
//					(errmsg("%s: %s/%d %s",deladdr ? "RTM_DELADDR" : "RTM_NEWADDR",address, ifa->ifa_prefixlen, ifname)));
			/* check of the address is our VIP */
			if (strcmp(pool_config->delegate_IP, address) == 0 )
			{
				ereport(WARNING,
						(errmsg("VIP monitoring detected VIP is %s",deladdr ?"droped":"brought up")));
				//					WD_List->delegate_ip_flag = 0; //TEMP
				//					wd_IP_up();
			}
			break;
		default:
			ereport(DEBUG2,
					(errmsg("unknown nlmsg_type=%d", nlhdr->rtm_type)));
	}
}
#endif

/*
 * fork escalation process
 */
pid_t
fork_escalation_process(void)
{
	pid_t pid;
	int has_vip = 0;
	int sock = -1;
	sigjmp_buf	local_sigjmp_buf;
	
	pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			ereport(NOTICE,
					(errmsg("failed to fork a escalation process")));
		return pid;
	}
	on_exit_reset();
	processType = PT_WATCHDOG_UTILITY;

	POOL_SETMASK(&UnBlockSig);
	
	init_ps_display("", "", "", "");
	
	pool_signal(SIGTERM, wd_exit);
	pool_signal(SIGINT, wd_exit);
	pool_signal(SIGQUIT, wd_exit);
	pool_signal(SIGCHLD, SIG_DFL);
	pool_signal(SIGHUP, SIG_IGN);
	pool_signal(SIGPIPE, SIG_IGN);
	
	/* Create per loop iteration memory context */
	ProcessLoopContext = AllocSetContextCreate(TopMemoryContext,
											   "wd_escalation_main_loop",
											   ALLOCSET_DEFAULT_MINSIZE,
											   ALLOCSET_DEFAULT_INITSIZE,
											   ALLOCSET_DEFAULT_MAXSIZE);
	
	MemoryContextSwitchTo(TopMemoryContext);
	
	set_ps_display("escalation",false);
	
	ereport(LOG,
			(errmsg("watchdog: escalation started")));
	/* 
	 * STEP 1
	 * clear shared memory cache
	 */
	if (pool_config->memory_cache_enabled && pool_is_shmem_cache() &&
		pool_config->clear_memqcache_on_escalation)
	{
		ereport(LOG,
				(errmsg("watchdog escalation"),
				 errdetail("clearing all the query cache on shared memory")));
		
		pool_clear_memory_cache();
	}

	/*
	 * STEP 2
	 * execute escalation command
	 */
	if (strlen(pool_config->wd_escalation_command))
	{
		int r = system(pool_config->wd_escalation_command);
		if (WIFEXITED(r))
		{
			if (WEXITSTATUS(r) == EXIT_SUCCESS)
				ereport(LOG,
						(errmsg("watchdog escalation successful")));
			else
			{
				ereport(WARNING,
						(errmsg("watchdog escalation command failed with exit status: %d", WEXITSTATUS(r))));
			}
		}
		else
		{
			ereport(WARNING,
					(errmsg("watchdog escalation command exit abnormally")));
		}
	}
	/*
	 * STEP 3
	 * interface up as delegate IP
	 */

	if (strlen(pool_config->delegate_IP) != 0)
	{
		has_vip = wd_IP_up();
	}

	/*
	 * STEP 4
	 * start vip monitoring
	 * create monitoring socket
	 */
//	sock = create_monitoring_socket();

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
	
	/* watchdog loop */
	for (;;)
	{
		if (has_vip && sock > 0)
		{
			MemoryContextSwitchTo(ProcessLoopContext);
			MemoryContextResetAndDeleteChildren(ProcessLoopContext);

			watch_interface_changes(sock);

		}
		else /* got noting to do, keep sleeping till death */
			sleep(10);
	}

	return pid;
}
