/* -*-pgsql-c-*- */
/*
 * $Header$
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
 */
#include "pool.h"
#include "pool_config.h"
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <unistd.h>


#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include "utils/getopt_long.h"
#endif

#include <errno.h>
#include <string.h>
#include <libgen.h>
#include "utils/elog.h"
#include "utils/palloc.h"
#include "utils/memutils.h"

#include "version.h"
#include "auth/pool_passwd.h"
#include "query_cache/pool_memqcache.h"
#include "watchdog/wd_ext.h"

static void daemonize(void);
static int read_pid_file(void);
static void write_pid_file(void);
static void usage(void);
static void show_version(void);
static void stop_me(void);
static void FileUnlink(int code, Datum path);

char pcp_conf_file[POOLMAXPATHLEN+1]; /* path for pcp.conf */
char conf_file[POOLMAXPATHLEN+1];
char hba_file[POOLMAXPATHLEN+1];

static int not_detach = 0;		/* non 0 if non detach option (-n) is given */
int stop_sig = SIGTERM;		/* stopping signal default value */
int myargc;
char **myargv;
int assert_enabled = 0;
int main(int argc, char **argv)
{
	int opt;
	int debug_level = 0;
	int	optindex;
	bool discard_status = false;
	bool clear_memcache_oidmaps = false;

	static struct option long_options[] = {
		{"hba-file", required_argument, NULL, 'a'},
		{"clear", no_argument, NULL, 'c'},
		{"debug", no_argument, NULL, 'd'},
		{"config-file", required_argument, NULL, 'f'},
		{"pcp-file", required_argument, NULL, 'F'},
		{"help", no_argument, NULL, 'h'},
		{"mode", required_argument, NULL, 'm'},
		{"dont-detach", no_argument, NULL, 'n'},
		{"discard-status", no_argument, NULL, 'D'},
		{"clear-oidmaps", no_argument, NULL, 'C'},
		{"debug-assertions", no_argument, NULL, 'x'},
		{"version", no_argument, NULL, 'v'},
		{NULL, 0, NULL, 0}
	};

	myargc = argc;
	myargv = argv;

	snprintf(conf_file, sizeof(conf_file), "%s/%s", DEFAULT_CONFIGDIR, POOL_CONF_FILE_NAME);
	snprintf(pcp_conf_file, sizeof(pcp_conf_file), "%s/%s", DEFAULT_CONFIGDIR, PCP_PASSWD_FILE_NAME);
	snprintf(hba_file, sizeof(hba_file), "%s/%s", DEFAULT_CONFIGDIR, HBA_CONF_FILE_NAME);
    while ((opt = getopt_long(argc, argv, "a:df:F:hm:nDCxv", long_options, &optindex)) != -1)
	{
		switch (opt)
		{
			case 'a':    /* specify hba configuration file */
				if (!optarg)
				{
					usage();
					exit(1);
				}
				strlcpy(hba_file, optarg, sizeof(hba_file));
				break;

			case 'x':	/* enable cassert */
				assert_enabled = 1;
				break;

			case 'd':	/* debug option */
				debug_level = 1;
				break;

			case 'f':	/* specify configuration file */
				if (!optarg)
				{
					usage();
					exit(1);
				}
				strlcpy(conf_file, optarg, sizeof(conf_file));
				break;

			case 'F':   /* specify PCP password file */
				if (!optarg)
				{
					usage();
					exit(1);
				}
				strlcpy(pcp_conf_file, optarg, sizeof(pcp_conf_file));
				break;

			case 'h':
				usage();
				exit(0);
				break;

			case 'm':	/* stop mode */
				if (!optarg)
				{
					usage();
					exit(1);
				}
				if (*optarg == 's' || !strcmp("smart", optarg))
					stop_sig = SIGTERM;		/* smart shutdown */
				else if (*optarg == 'f' || !strcmp("fast", optarg))
					stop_sig = SIGINT;		/* fast shutdown */
				else if (*optarg == 'i' || !strcmp("immediate", optarg))
					stop_sig = SIGQUIT;		/* immediate shutdown */
				else
				{
					usage();
					exit(1);
				}
				break;

			case 'n':	/* no detaching control ttys */
				not_detach = 1;
				break;

			case 'D':	/* discard pgpool_status */
				discard_status = true;
				break;

			case 'C': /* discard caches in memcached */
				clear_memcache_oidmaps = true;
				break;

			case 'v':
				show_version();
				exit(0);

			default:
				usage();
				exit(1);
		}
	}
#ifdef USE_SSL
	/* global ssl init */
	SSL_library_init();
	SSL_load_error_strings();
#endif /* USE_SSL */

	myargv = save_ps_display_args(myargc, myargv);
	/* create MemoryContexts */
	MemoryContextInit();

	mypid = getpid();

	pool_init_config();
	/*
	 * Init debug level with -d option value
	 */
	pool_config->debug_level = debug_level;

	pool_get_config(conf_file, INIT_CONFIG);
	/*
	 * Override debug level
	 */
	if (pool_config->debug_level == 0)
		pool_config->debug_level = debug_level;

    /* if command line -d arg is given adjust the log_min_message config variable */
    if(debug_level > 0 && pool_config->log_min_messages > DEBUG1)
        pool_config->log_min_messages = DEBUG1;

	if (pool_config->enable_pool_hba)
		load_hba(hba_file);

	/*
	 * If a non-switch argument remains, then it should be either "reload" or "stop".
	 */
	if (optind == (argc - 1))
	{
		if (!strcmp(argv[optind], "reload"))
		{
				pid_t pid;

				pid = read_pid_file();
				if (pid < 0)
				{
					ereport(FATAL,
						(return_code(1),
                             errmsg("could not read pid file")));
				}

				if (kill(pid, SIGHUP) == -1)
				{
					ereport(FATAL,
						(return_code(1),
						 errmsg("could not reload configuration file pid: %d. reason: %s", pid, strerror(errno))));
				}
				exit(0);
		}
		if (!strcmp(argv[optind], "stop"))
		{
			stop_me();
			unlink(pool_config->pid_file_name);
			exit(0);
		}
		else
		{
			usage();
			exit(1);
		}
	}
	/*
	 * else if no non-switch argument remains, then it should be a start request
	 */
	else if (optind == argc)
	{
		int pid = read_pid_file();
		if (pid > 0)
		{
			if (kill(pid, 0) == 0)
			{
				ereport(FATAL,
					(errmsg("pid file found. is another pgpool(%d) is running?\n", pid)));
			}
			else
				ereport(NOTICE,
					(errmsg("pid file found but it seems bogus. Trying to start pgpool anyway...\n")));
		}
	}
	/*
	 * otherwise an error...
	 */
	else
	{
		usage();
		exit(1);
	}

	/* check effective user id for watchdog */
	/* watchdog must be started under the privileged user */
	if (pool_config->use_watchdog )
	{
		/* check setuid bit of network interface control commands */
		if (wd_chk_setuid() == 1)
		{
			/* if_up, if_down and arping command have a setuid bit */
			ereport(NOTICE,
				(errmsg("watchdog might call network commands which using setuid bit.")));
		}
	}

	/* set signal masks */
	poolinitmask();

	if (not_detach)
		write_pid_file();
	else
		daemonize();
	
#ifdef HAVE_VSYSLOG
    set_syslog_parameters(pool_config->syslog_ident ? pool_config->syslog_ident : "pgpool",
                          pool_config->syslog_facility);
#endif
	/*
	 * Locate pool_passwd
	 * The default file name "pool_passwd" can be changed by setting
	 * pgpool.conf's "pool_passwd" directive.
	 */
	if (strcmp("", pool_config->pool_passwd))
	{
		char pool_passwd[POOLMAXPATHLEN+1];
		char dirnamebuf[POOLMAXPATHLEN+1];
		char *dirp;

		strlcpy(dirnamebuf, conf_file, sizeof(dirnamebuf));
		dirp = dirname(dirnamebuf);
		snprintf(pool_passwd, sizeof(pool_passwd), "%s/%s",
				 dirp, pool_config->pool_passwd);
		pool_init_pool_passwd(pool_passwd);
	}

	pool_semaphore_create(MAX_NUM_SEMAPHORES);

	PgpoolMain(discard_status, clear_memcache_oidmaps); /* this is an infinate loop */

	exit(0);

}

static void show_version(void)
{
	fprintf(stderr, "%s version %s (%s)\n",	PACKAGE, VERSION, PGPOOLVERSION);
}

static void usage(void)
{
	fprintf(stderr, "%s version %s (%s),\n",	PACKAGE, VERSION, PGPOOLVERSION);
	fprintf(stderr, "  A generic connection pool/replication/load balance server for PostgreSQL\n\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  pgpool [ -c] [ -f CONFIG_FILE ] [ -F PCP_CONFIG_FILE ] [ -a HBA_CONFIG_FILE ]\n");
	fprintf(stderr, "         [ -n ] [ -D ] [ -d ]\n");
	fprintf(stderr, "  pgpool [ -f CONFIG_FILE ] [ -F PCP_CONFIG_FILE ] [ -a HBA_CONFIG_FILE ]\n");
	fprintf(stderr, "         [ -m SHUTDOWN-MODE ] stop\n");
	fprintf(stderr, "  pgpool [ -f CONFIG_FILE ] [ -F PCP_CONFIG_FILE ] [ -a HBA_CONFIG_FILE ] reload\n\n");
	fprintf(stderr, "Common options:\n");
	fprintf(stderr, "  -a, --hba-file=HBA_CONFIG_FILE\n");
	fprintf(stderr, "                      Sets the path to the pool_hba.conf configuration file\n");
	fprintf(stderr, "                      (default: %s/%s)\n",DEFAULT_CONFIGDIR, HBA_CONF_FILE_NAME);
	fprintf(stderr, "  -f, --config-file=CONFIG_FILE\n");
	fprintf(stderr, "                      Sets the path to the pgpool.conf configuration file\n");
	fprintf(stderr, "                      (default: %s/%s)\n",DEFAULT_CONFIGDIR, POOL_CONF_FILE_NAME);
	fprintf(stderr, "  -F, --pcp-file=PCP_CONFIG_FILE\n");
	fprintf(stderr, "                      Sets the path to the pcp.conf configuration file\n");
	fprintf(stderr, "                      (default: %s/%s)\n",DEFAULT_CONFIGDIR, PCP_PASSWD_FILE_NAME);
	fprintf(stderr, "  -h, --help          Prints this help\n\n");
	fprintf(stderr, "Start options:\n");
	fprintf(stderr, "  -C, --clear-oidmaps Clears query cache oidmaps when memqcache_method is memcached\n");
	fprintf(stderr, "                      (If shmem, discards whenever pgpool starts.)\n");
	fprintf(stderr, "  -n, --dont-detach   Don't run in daemon mode, does not detach control tty\n");
	fprintf(stderr, "  -x, --debug-assertions   Turns on various assertion checks, This is a debugging aid\n");
	fprintf(stderr, "  -D, --discard-status Discard pgpool_status file and do not restore previous status\n");
	fprintf(stderr, "  -d, --debug         Debug mode\n\n");
	fprintf(stderr, "Stop options:\n");
	fprintf(stderr, "  -m, --mode=SHUTDOWN-MODE\n");
	fprintf(stderr, "                      Can be \"smart\", \"fast\", or \"immediate\"\n\n");
	fprintf(stderr, "Shutdown modes are:\n");
	fprintf(stderr, "  smart       quit after all clients have disconnected\n");
	fprintf(stderr, "  fast        quit directly, with proper shutdown\n");
	fprintf(stderr, "  immediate   the same mode as fast\n");
}

/*
* detach control ttys
*/
static void daemonize(void)
{
	int			i;
	pid_t		pid;
	int			fdlimit;

	pid = fork();
	if (pid == (pid_t) -1)
	{
		ereport(FATAL,
			(errmsg("could not daemonize the pgpool-II"),
				errdetail("fork() system call failed with reason: \"%s\"", strerror(errno) )));
	}
	else if (pid > 0)
	{			/* parent */
		exit(0);
	}

#ifdef HAVE_SETSID
	if (setsid() < 0)
	{
		ereport(FATAL,
			(errmsg("could not daemonize the pgpool-II"),
				errdetail("setsid() system call failed with reason: \"%s\"", strerror(errno) )));
	}
#endif

	mypid = getpid();
	write_pid_file();
	if(chdir("/"))
		ereport(WARNING,
			(errmsg("change directory failed"),
                 errdetail("chdir() system call failed with reason: \"%s\"", strerror(errno) )));

	/* redirect stdin, stdout and stderr to /dev/null */
	i = open("/dev/null", O_RDWR);
	if(i < 0)
	{
		ereport(WARNING,
			(errmsg("failed to open \"/dev/null\", open() failed with error \"%s\"", strerror(errno))));
	}
	else
	{
		dup2(i, 0);
		dup2(i, 1);
		dup2(i, 2);
		close(i);
	}
	/* close syslog connection for daemonizing */
	if (pool_config->logsyslog) {
		closelog();
	}

	/* close other file descriptors */
    fdlimit = sysconf(_SC_OPEN_MAX);
    for (i = 3; i < fdlimit; i++)
		close(i);
}

/*
* stop myself
*/
static void stop_me(void)
{
	pid_t pid;

	pid = read_pid_file();
	if (pid < 0)
	{
		ereport(FATAL,
			(errmsg("could not read pid file")));
	}

	if (kill(pid, stop_sig) == -1)
	{
		ereport(FATAL,
			(errmsg("could not stop process with pid: %d", pid),
				errdetail("\"%s\"", strerror(errno))));
	}
	ereport(LOG,
		(errmsg ("stop request sent to pgpool. waiting for termination...")));

	while (kill(pid, 0) == 0)
	{
		fprintf(stderr, ".");
		sleep(1);
	}
	fprintf(stderr, "done.\n");
}

/*
* read the pid file
*/
static int read_pid_file(void)
{
	int fd;
	int readlen;
	char pidbuf[128];

	fd = open(pool_config->pid_file_name, O_RDONLY);
	if (fd == -1)
	{
		return -1;
	}
	if ((readlen = read(fd, pidbuf, sizeof(pidbuf))) == -1)
	{
		close(fd);
		ereport(FATAL,
			(errmsg("could not read pid file as \"%s\". reason: %s",
				   pool_config->pid_file_name, strerror(errno))));
	}
	else if (readlen == 0)
	{
		close(fd);
		ereport(FATAL,
			(errmsg("EOF detected while reading pid file \"%s\". reason: %s",
				   pool_config->pid_file_name, strerror(errno))));
	}
	close(fd);
	return(atoi(pidbuf));
}

/*
* write the pid file
*/
static void write_pid_file(void)
{
	int fd;
	char pidbuf[128];

	fd = open(pool_config->pid_file_name, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
	if (fd == -1)
	{
		ereport(FATAL,
			(errmsg("could not open pid file as %s. reason: %s",
				   pool_config->pid_file_name, strerror(errno))));
	}
	snprintf(pidbuf, sizeof(pidbuf), "%d", (int)getpid());
	if (write(fd, pidbuf, strlen(pidbuf)+1) == -1)
	{
		close(fd);
		ereport(FATAL,
			(errmsg("could not write pid file as %s. reason: %s",
				   pool_config->pid_file_name, strerror(errno))));
	}
	if (fsync(fd) == -1)
	{
		close(fd);
		ereport(FATAL,
			(errmsg("could not fsync pid file as %s. reason: %s",
				   pool_config->pid_file_name, strerror(errno))));
	}
	if (close(fd) == -1)
	{
		ereport(FATAL,
			(errmsg("could not close pid file as %s. reason: %s",
				   pool_config->pid_file_name, strerror(errno))));
	}
	/* register the call back to delete the pid file at system exit */
	on_proc_exit(FileUnlink, (Datum) pool_config->pid_file_name);
}

/*
 * get_config_file_name: return full path of pgpool.conf.
 */
char *get_config_file_name(void)
{
	return conf_file;
}

/*
 * get_hba_file_name: return full path of pool_hba.conf.
 */
char *get_hba_file_name(void)
{
	return hba_file;
}
/*
 * Call back function to unlink the file
 */
static void FileUnlink(int code, Datum path)
{
	char* filePath = (char*)path;
	if (unlink(filePath) == 0) return;
	/* 
	 * We are already exiting the system just produce a log entry to report an error
	 */
	ereport(LOG,
		(errmsg("unlink failed for file at path \"%s\"", filePath),
			errdetail("\"%s\"", strerror(errno))));
}
