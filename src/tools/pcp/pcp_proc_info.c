/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2008	PgPool Global Development Group
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
 * Client program to send "process info" command.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include "utils/getopt_long.h"
#endif
#include <time.h>

#include "pcp/pcp.h"

static void usage(void);
static void myexit(PCPConnInfo* pcpConn);

int
main(int argc, char **argv)
{
	long timeout;
	char host[MAX_DB_HOST_NAMELEN];
	int port;
	char user[MAX_USER_PASSWD_LEN];
	char pass[MAX_USER_PASSWD_LEN];
	int processID;
	ProcessInfo *process_info;
	int array_size;
	int ch;
	int	optindex;
	char * frmt;
	bool verbose = false;
	bool all = false;
	bool debug = false;
	PCPConnInfo* pcpConn;
	PCPResultInfo* pcpResInfo;

	static struct option long_options[] = {
		{"debug", no_argument, NULL, 'd'},
		{"help", no_argument, NULL, 'h'},
		{"verbose", no_argument, NULL, 'v'},
		{NULL, 0, NULL, 0}
	};
	
	while ((ch = getopt_long(argc, argv, "hdva", long_options, &optindex)) != -1) {
		switch (ch) {
		case 'd':
			debug = true;
			break;

		case 'v':
			verbose = true;
			break;

		case 'a':
			all = true;
			break;

		case 'h':
		case '?':
		default:
			myexit(NULL);
		}
	}
	argc -= optind;
	argv += optind;

	if (verbose)
	{
		if (all)
			frmt =	"Database     : %s\n"
					"Username     : %s\n"
					"Start time   : %s\n"
					"Creation time: %s\n"
					"Major        : %d\n"
					"Minor        : %d\n"
					"Counter      : %d\n"
					"Backend PID  : %d\n"
					"Connected    : %d\n"
					"PID          : %d\n"
					"Backend ID   : %d\n";
		else
			frmt =	"Database     : %s\n"
					"Username     : %s\n"
					"Start time   : %s\n"
					"Creation time: %s\n"
					"Major        : %d\n"
					"Minor        : %d\n"
					"Counter      : %d\n"
					"Backend PID  : %d\n"
					"Connected    : %d\n";
	}
	else
	{
		if (all)
			frmt = "%s %s %s %s %d %d %d %d %d %d %d\n";
		else
			frmt = "%s %s %s %s %d %d %d %d %d\n";
	}

	if (!(argc == 5 || argc == 6))
		myexit(NULL);

	timeout = atol(argv[0]);
	if (timeout < 0)
		myexit(NULL);

	if (strlen(argv[1]) >= MAX_DB_HOST_NAMELEN)
		myexit(NULL);
	strcpy(host, argv[1]);

	port = atoi(argv[2]);
	if (port <= 1024 || port > 65535)
		myexit(NULL);

	if (strlen(argv[3]) >= MAX_USER_PASSWD_LEN)
		myexit(NULL);
	strcpy(user, argv[3]);

	if (strlen(argv[4]) >= MAX_USER_PASSWD_LEN)
		myexit(NULL);
	strcpy(pass, argv[4]);
	
	if (argc == 6) {

		processID = atoi(argv[5]);
		if (processID < 0)
			myexit(NULL);
	}
	else {
		processID = 0;
	}

	pcp_set_timeout(timeout);

	pcpConn = pcp_connect(host, port, user, pass, debug?stdout:NULL);
	if(PCPConnectionStatus(pcpConn) != PCP_CONNECTION_OK)
		myexit(pcpConn);

	pcpResInfo = pcp_process_info(pcpConn, processID);
	if(PCPResultStatus(pcpResInfo) != PCP_RES_COMMAND_OK)
		myexit(pcpConn);

	
	array_size = pcp_result_slot_count(pcpResInfo);
	int i;
	char strcreatetime[128];
	char strstarttime[128];

	for (i = 0; i < array_size; i++)
	{
		
		process_info = (ProcessInfo*) pcp_get_binary_data(pcpResInfo, i);
		if(process_info == NULL)
			break;
		if ((!all) && (process_info->connection_info->database[0] == '\0'))
			continue;

		*strcreatetime = *strstarttime = '\0';

		if (process_info->start_time)
			strftime(strstarttime, 128, "%Y-%m-%d %H:%M:%S", localtime(&process_info->start_time));
		if (process_info->connection_info->create_time)
			strftime(strcreatetime, 128, "%Y-%m-%d %H:%M:%S", localtime(&process_info->connection_info->create_time));

		printf(frmt,
			process_info->connection_info->database,
			process_info->connection_info->user,
			strstarttime,
			strcreatetime,
			process_info->connection_info->major,
			process_info->connection_info->minor,
			process_info->connection_info->counter,
			process_info->connection_info->pid,
			process_info->connection_info->connected,
			process_info->pid,
			process_info->connection_info->backend_id);
	}

	pcp_disconnect(pcpConn);
	pcp_free_connection(pcpConn);

	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "pcp_proc_info - display a pgpool-II child process' information\n\n");
	fprintf(stderr, "Usage: pcp_proc_info [-d] timeout hostname port# username password [PID]\n");
	fprintf(stderr, "  -d, --debug    : enable debug message (optional)\n");
	fprintf(stderr, "  timeout        : connection timeout value in seconds. command exits on timeout\n");
	fprintf(stderr, "  hostname       : pgpool-II hostname\n");
	fprintf(stderr, "  port#          : PCP port number\n");
	fprintf(stderr, "  username       : username for PCP authentication\n");
	fprintf(stderr, "  password       : password for PCP authentication\n");
	fprintf(stderr, "  PID            : if given, PID of the only child process to get information for\n\n");
	fprintf(stderr, "Usage: pcp_proc_info [options]\n");
	fprintf(stderr, "  Options available are:\n");
	fprintf(stderr, "  -h, --help     : print this help\n");
	fprintf(stderr, "  -v, --verbose  : display one line per information with a header\n");
	fprintf(stderr, "  -a, --all      : display all child processes and their available connection slots.\n");
}

static void
myexit(PCPConnInfo* pcpConn)
{
	if (pcpConn == NULL)
	{
		usage();
	}
	else
	{
		fprintf(stderr, "%s\n",pcp_get_last_error(pcpConn)?pcp_get_last_error(pcpConn):"Unknown Error");
		pcp_disconnect(pcpConn);
		pcp_free_connection(pcpConn);
	}
	exit(-1);
}
