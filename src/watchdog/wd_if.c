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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>

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
#include <ifaddrs.h>

#include "pool.h"

#include "utils/elog.h"
#include "pool_config.h"
#include "watchdog/watchdog.h"
#include "watchdog/wd_utils.h"


static int exec_if_cmd(char * path,char * command);


List* get_all_local_ips(void)
{
	struct ifaddrs *ifAddrStruct=NULL;
	struct ifaddrs *ifa=NULL;
	void *tmpAddrPtr=NULL;
	List *local_addresses = NULL;
	
	getifaddrs(&ifAddrStruct);
	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (!ifa->ifa_addr)
			continue;
		
		if (ifa->ifa_addr->sa_family == AF_INET)
		{
			char *addressBuffer;
			if (!strncasecmp("lo", ifa->ifa_name, 2))
				continue; /* We do not need loop back addresses */
			tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			addressBuffer = palloc(INET_ADDRSTRLEN);
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			printf("%s IP INET Address %s\n", ifa->ifa_name, addressBuffer);
			local_addresses = lappend(local_addresses,addressBuffer);
		}
	}
	if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
	return local_addresses;
}

#define WD_TRY_PING_AT_IPUP 3
int
wd_IP_up(void)
{
	int rtn = WD_OK;
	char path[WD_MAX_PATH_LEN];
	char cmd[128];
	int i;

	if (strlen(pool_config->delegate_IP) == 0)
		return WD_NG;

	wd_get_cmd(cmd,pool_config->if_up_cmd);
	snprintf(path,sizeof(path),"%s/%s",pool_config->if_cmd_path,cmd);
	rtn = exec_if_cmd(path,pool_config->if_up_cmd);

	if (rtn == WD_OK)
	{
		wd_get_cmd(cmd,pool_config->arping_cmd);
		snprintf(path,sizeof(path),"%s/%s",pool_config->arping_path,cmd);
		rtn = exec_if_cmd(path,pool_config->arping_cmd);
	}
	if (rtn == WD_OK)
	{
		for (i = 0; i < WD_TRY_PING_AT_IPUP; i++)
		{
			if (!wd_is_unused_ip(pool_config->delegate_IP))
				break;
			ereport(LOG,
				(errmsg("watchdog bringing up delegate IP"),
					 errdetail("waiting... count: %d", i+1)));
		}

		if (i >= WD_TRY_PING_AT_IPUP)
			rtn = WD_NG;
	}

	if (rtn == WD_OK)
		ereport(LOG,
			(errmsg("watchdog bringing up delegate IP, 'if_up_cmd' succeeded")));
	else
		ereport(WARNING,
			(errmsg("watchdog failed to bring up delegate IP, 'if_up_cmd' failed")));
	return rtn;
}

#define WD_TRY_PING_AT_IPDOWN 3
int
wd_IP_down(void)
{
	int rtn = WD_OK;
	char path[WD_MAX_PATH_LEN];
	char cmd[128];
	int i;

	if (strlen(pool_config->delegate_IP) == 0)
		return WD_NG;

	wd_get_cmd(cmd,pool_config->if_down_cmd);
	snprintf(path, sizeof(path), "%s/%s", pool_config->if_cmd_path, cmd);
	rtn = exec_if_cmd(path,pool_config->if_down_cmd);

	if (rtn == WD_OK)
	{
		for (i = 0; i < WD_TRY_PING_AT_IPDOWN; i++)
		{
			if (wd_is_unused_ip(pool_config->delegate_IP))
				break;
		}

		if (i >= WD_TRY_PING_AT_IPDOWN)
			rtn = WD_NG;
	}

	if (rtn == WD_OK)
	{
		ereport(LOG,
			(errmsg("watchdog bringing down delegate IP"),
				 errdetail("if_down_cmd succeeded")));
	}
	else
	{
		ereport(WARNING,
			(errmsg("watchdog bringing down delegate IP, if_down_cmd failed")));
	}
	return rtn;
}


int
wd_get_cmd(char * buf, char * cmd)
{
	int i,j;
	i = 0;
	while(isspace(cmd[i]) != 0)
	{
		i++;
	}
	j = 0;
	while(isspace(cmd[i]) == 0)
	{
		buf[j++] = cmd[i++];
	}
	buf[j] = '\0';
	return strlen(buf);
}

static int
exec_if_cmd(char * path,char * command)
{
	int pfd[2];
	int status;
	char * args[24];
	int pid, i = 0;
	char* buf;
	char *bp, *ep;

	if (pipe(pfd) == -1)
	{
		ereport(WARNING,
				(errmsg("while executing interface up/down command, pipe open failed with error \"%s\"",strerror(errno))));
		return WD_NG;
	}

	buf = string_replace(command,"$_IP_$",pool_config->delegate_IP);

	bp = buf;
	while (*bp == ' ')
	{
		bp ++;
	}
	while (*bp != '\0')
	{
		ep = strchr(bp,' ');
		if (ep != NULL)
		{
			*ep = '\0';
		}
		args[i++] = bp;
		if (ep != NULL)
		{
			bp = ep +1;
			while (*bp == ' ')
			{
				bp ++;
			}
		}
		else
		{
			break;
		}
	}
	args[i++] = NULL;

	pid = fork();
	if (pid == -1)
	{
		ereport(FATAL,
			(errmsg("failed to execute interface up/down command"),
				 errdetail("fork() failed with reason: \"%s\"", strerror(errno))));
	}
	if (pid == 0)
	{
		on_exit_reset();
		processType = PT_WATCHDOG_UTILITY;
		close(STDOUT_FILENO);
		dup2(pfd[1], STDOUT_FILENO);
		close(pfd[0]);
		status = execv(path,args);
		exit(0);
	}
	else
	{
		pfree(buf);
		close(pfd[1]);
		for (;;)
		{
			int result;
			result = waitpid(pid, &status, 0);
			if (result < 0)
			{
				if (errno == EINTR)
					continue;

				ereport(DEBUG1,
					(errmsg("watchdog exec waitpid()failed"),
						 errdetail("waitpid() system call failed with reason \"%s\"", strerror(errno))));

				return WD_NG;
			}

			if (WIFEXITED(status) == 0 || WEXITSTATUS(status) != 0)
			{
				ereport(DEBUG1,
					(errmsg("watchdog exec interface up/down command failed"),
						errdetail("'%s' failed. exit status: %d",command, WEXITSTATUS(status))));

				return WD_NG;
			}
			else
				break;
		}
		close(pfd[0]);
	}
	ereport(DEBUG1,
		(errmsg("watchdog exec interface up/down command: '%s' succeeded", command)));

	return WD_OK;
}


int create_monitoring_socket(void)
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
bool read_interface_change_event(int sock, char** ip_address, char** interface, bool* address_deleted)
{
	char buffer[4096];
	int len;
	struct iovec iov;
	struct msghdr hdr;
	struct ifaddrmsg *ifa;
	struct nlmsghdr *nlhdr;
	struct rtattr *rta;
	int ifa_len;
	
	*address_deleted = false;
	
	iov.iov_base = buffer;
	iov.iov_len = sizeof(buffer);
	
	memset(&hdr, 0, sizeof(hdr));
	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;
	
	len = recvmsg(sock, &hdr, 0);
	if (len < 0)
	{
		ereport(DEBUG1,
				(errmsg("VIP monitoring failed to receive from socket"),
				 errdetail("recvmsg() failed with error \"%s\"",strerror(errno))));
		return false;
	}
	
	nlhdr = (struct nlmsghdr *)buffer;
	
	for (; NLMSG_OK(nlhdr, len) ;nlhdr = NLMSG_NEXT(nlhdr, len))
	{
		bool deladdr = false;
		char addr[48];
		char* label = NULL;
		addr[0] = '\0';
		
		if(nlhdr->nlmsg_type == NLMSG_DONE)
			break;
		
		switch(nlhdr->nlmsg_type)
		{
			case RTM_DELADDR:
				*address_deleted = true; /* fallthrough */
			case RTM_NEWADDR:
				/*
				 * code taken from http://linux-hacks.blogspot.fr/2009/01/sample-code-to-learn-netlink.html
				 */
				
				ifa = (struct ifaddrmsg *)NLMSG_DATA(nlhdr);
				rta = (struct rtattr *)IFA_RTA(ifa);
				ifa_len = IFA_PAYLOAD(nlhdr);
				
				ereport(DEBUG2,
						(errmsg("VIP monitoring new event %s", *address_deleted?"RTM_DELADDR" : "RTM_NEWADDR"),
						 errdetail("index=%d fam=%d prefixlen=%d flags=%d scope=%d",
								   ifa->ifa_index, ifa->ifa_family, ifa->ifa_prefixlen,
								   ifa->ifa_flags, ifa->ifa_scope)));
				
				/* We are only concerned by INET addresses */
				if (ifa->ifa_family != AF_INET)
					continue;
				
				for(;RTA_OK(rta, ifa_len); rta = RTA_NEXT(rta, ifa_len))
				{
					
					switch(rta->rta_type)
					{
						case IFA_LOCAL:
							inet_ntop (AF_INET, RTA_DATA (rta), addr, sizeof (addr));
							break;
							
						case IFA_LABEL:
							label = (char *) RTA_DATA (rta);
							break;
							
						default:
							/* ignore all other attributes */
							break;
					}
					ereport(DEBUG2,
							(errmsg("rta_len=%d rta_type=%d '%s'", rta->rta_len, rta->rta_type, addr)));
				}
				
				if (label && addr[0] != '\0')
				{
					ereport(DEBUG1,
							(errmsg("%s: %s on %s",*address_deleted ? "ADDRESS DELETED" : "NEW ADDRESS",addr, label)));
					
					*interface = pstrdup(label);
					*ip_address = pstrdup(addr);
					return true;
				}
				break;
			default:
				ereport(DEBUG2,
						(errmsg("unknown nlmsg_type=%d", nlhdr->nlmsg_type)));
		}
	}
	return false;
}

#else /* For non linux chaps */
#if defined(__OpenBSD__) || defined(__FreeBSD__)
#define SALIGN (sizeof(long) - 1)
#else
#define SALIGN (sizeof(int32_t) - 1)
#endif

#define SA_RLEN(sa) ((sa)->sa_len ? (((sa)->sa_len + SALIGN) & ~SALIGN) : (SALIGN + 1))
/* With the help from https://github.com/miniupnp/miniupnp/blob/master/minissdpd/ifacewatch.c */
bool read_interface_change_event(int sock, char** ip_address, char** interface, bool* address_deleted)
{
	char buffer[1024];
	int len;
	struct ifa_msghdr *ifam;
	struct rt_msghdr *nlhdr;
	struct sockaddr * sa;
	char * p;
	int addr;
	int prefixlen = 0;
	char tmp[64];
	int family = AF_UNSPEC;
	
	char address[48];
	char ifname[256];
	address[0] = '\0';
	ifname[0] = '\0';
	
	*address_deleted = false;
	
	len = recv(sock, buffer, sizeof(buffer), 0);
	if (len < 0)
	{
		ereport(DEBUG1,
				(errmsg("VIP monitoring failed to receive from socket"),
				 errdetail("recv() failed with error \"%s\"",strerror(errno))));
		return false;
	}
	
	nlhdr = (struct rt_msghdr *)buffer;
	switch(nlhdr->rtm_type)
	{
		case RTM_DELADDR:
			*address_deleted = true; /* fallthrough */
		case RTM_NEWADDR:
			ifam = (struct ifa_msghdr *)buffer;
			
			p = buffer + sizeof(struct ifa_msghdr);
			addr = 1;
			while(p < buffer + len)
			{
				sa = (struct sockaddr *)p;
				while(!(addr & ifam->ifam_addrs) && (addr <= ifam->ifam_addrs))
					addr = addr << 1;
				inet_ntop(sa->sa_family,
						  &((struct sockaddr_in *)sa)->sin_addr,
						  tmp, sizeof(tmp));
				
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
			if (ifname[0] != '\0' && address[0] != '\0')
			{
				*interface = pstrdup(ifname);
				*ip_address = pstrdup(address);
				return true;
			}
			break;
		default:
			ereport(DEBUG2,
					(errmsg("unknown nlmsg_type=%d", nlhdr->rtm_type)));
	}
	return false;
}
#endif


