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
 * Client program to send "process count" command.
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
	int process_count;
	int *process_list = NULL;
	int ch,i;
	int	optindex;
	bool debug = false;
	PCPConnInfo* pcpConn;
	PCPResultInfo* pcpResInfo;


	static struct option long_options[] = {
		{"debug", no_argument, NULL, 'd'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};
	
    while ((ch = getopt_long(argc, argv, "hd", long_options, &optindex)) != -1) {
		switch (ch) {
		case 'd':
			debug = true;
			break;

		case 'h':
		case '?':
		default:
			myexit(NULL);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 5)
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

	pcp_set_timeout(timeout);

	pcpConn = pcp_connect(host, port, user, pass, debug?stdout:NULL);
	if(PCPConnectionStatus(pcpConn) != PCP_CONNECTION_OK)
		myexit(pcpConn);

	pcpResInfo = pcp_process_count(pcpConn);
	if(PCPResultStatus(pcpResInfo) != PCP_RES_COMMAND_OK)
		myexit(pcpConn);

	process_count = pcp_get_data_length(pcpResInfo, 0) / sizeof(int);
	process_list = (int *)pcp_get_binary_data(pcpResInfo, 0);

	
	for (i = 0; i < process_count; i++)
		printf("%d ", process_list[i]);
	printf("\n");

	pcp_disconnect(pcpConn);
	pcp_free_connection(pcpConn);

	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "pcp_proc_count - display the list of pgpool-II child process PIDs\n\n");
	fprintf(stderr, "Usage: pcp_proc_count timeout hostname port# username password\n");
	fprintf(stderr, "Usage: pcp_node_info -h\n\n");
	fprintf(stderr, "  timeout    : connection timeout value in seconds. command exits on timeout\n");
	fprintf(stderr, "  hostname   : pgpool-II hostname\n");
	fprintf(stderr, "  port#      : PCP port number\n");
	fprintf(stderr, "  username   : username for PCP authentication\n");
	fprintf(stderr, "  password   : password for PCP authentication\n");
	fprintf(stderr, "  -h, --help : print this help\n");
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

