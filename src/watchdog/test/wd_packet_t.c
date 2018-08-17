#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include "pool.h"
#include "pool_config.h"
#include "watchdog.h"
#include "wd_ext.h"

pid_t		mypid;
WdInfo	   *WD_List = NULL;		/* watchdog server list */

static void wdlist_dump(void);
extern void wd_exit(int exit_signo);

int
main(int argc, char *argv[])
{
	int			rtn;

	signal(SIGCHLD, SIG_DFL);
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, wd_exit);
	signal(SIGQUIT, wd_exit);
	signal(SIGTERM, wd_exit);
	signal(SIGPIPE, SIG_IGN);

	mypid = getpid();
	rtn = pool_init_config();

	pool_config->recovery_user = "mitani";
	pool_config->trusted_servers = "paris";
	pool_config->delegate_IP = "192.168.100.99";
	pool_config->pgpool2_hostname = "vm1";
	pool_config->port = 5432;
	pool_config->wd_port = 9999;
	pool_config->other_wd->num_wd = 2;
	strcpy(pool_config->other_wd->wd_info[0].hostname, "vm2");
	pool_config->other_wd->wd_info[0].pgpool_port = 5432;
	pool_config->other_wd->wd_info[0].wd_port = 9999;
	pool_config->other_wd->wd_info[0].status = WD_INIT;
	strcpy(pool_config->other_wd->wd_info[1].hostname, "paris");
	pool_config->other_wd->wd_info[1].pgpool_port = 5432;
	pool_config->other_wd->wd_info[1].wd_port = 9999;
	pool_config->other_wd->wd_info[1].status = WD_INIT;
	pool_config->ping_path = "/bin";
	pool_config->ifconfig_path = "/sbin";
	pool_config->if_up_cmd = "ifconfig eth0:0 inet $_IP_$ netmask 255.255.255.0";
	pool_config->if_down_cmd = "ifconfig eth0:0 down";
	pool_config->wd_interval = 3;
	pool_config->wd_life_point = 1;

	wd_init();

	for (;;)
	{
		wdlist_dump();
		sleep(5);
		wd_lifecheck();
	}
	wd_exit(15);

}


static void
wdlist_dump(void)
{
	int			i;
	WdInfo	   *p = WD_List;

	i = 0;
	while (p->status != WD_END)
	{
		printf("%d:s[%d] ts[%d] tu[%d] h[%s] pp[%d] wp[%d]\n",
			   i, p->status,
			   p->tv.tv_sec, p->tv.tv_usec,
			   p->hostname,
			   p->pgpool_port,
			   p->wd_port);
		p++;
		i++;
	}
}
