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
 * Client program to send "node info" command.
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
static void myexit(ErrorCode e);

int
main(int argc, char **argv)
{
	long timeout;
	char host[MAX_DB_HOST_NAMELEN];
	int port;
	char user[MAX_USER_PASSWD_LEN];
	char pass[MAX_USER_PASSWD_LEN];
	int nodeID;
	BackendInfo *backend_info;
	int ch;
	int	optindex;
    bool verbose = false;

	static struct option long_options[] = {
		{"debug", no_argument, NULL, 'd'},
		{"help", no_argument, NULL, 'h'},
		{"verbose", no_argument, NULL, 'v'},
		{NULL, 0, NULL, 0}
	};
	
    while ((ch = getopt_long(argc, argv, "hdv", long_options, &optindex)) != -1) {
		switch (ch) {
		case 'd':
			pcp_enable_debug();
			break;

		case 'v':
			verbose = true;
			break;

		case 'h':
		case '?':
		default:
			usage();
			exit(0);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 6)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	timeout = atol(argv[0]);
	if (timeout < 0) {
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	if (strlen(argv[1]) >= MAX_DB_HOST_NAMELEN)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}
	strcpy(host, argv[1]);

	port = atoi(argv[2]);
	if (port <= 1024 || port > 65535)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	if (strlen(argv[3]) >= MAX_USER_PASSWD_LEN)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}
	strcpy(user, argv[3]);

	if (strlen(argv[4]) >= MAX_USER_PASSWD_LEN)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}
	strcpy(pass, argv[4]);

	nodeID = atoi(argv[5]);
	if (nodeID < 0 || nodeID > MAX_NUM_BACKENDS)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	pcp_set_timeout(timeout);

	if (pcp_connect(host, port, user, pass))
	{
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	if ((backend_info = pcp_node_info(nodeID)) == NULL)
	{
		pcp_errorstr(errorcode);
		pcp_disconnect();
		myexit(errorcode);
	} else {
        if (verbose)
        {
		    printf("Hostname: %s\nPort    : %d\nStatus  : %d\nWeight  : %f\n", 
		    	   backend_info->backend_hostname,
		    	   backend_info->backend_port,
		    	   backend_info->backend_status,
		    	   backend_info->backend_weight/RAND_MAX);
        } else {
		    printf("%s %d %d %f\n", 
		    	   backend_info->backend_hostname,
		    	   backend_info->backend_port,
		    	   backend_info->backend_status,
		    	   backend_info->backend_weight/RAND_MAX);
        }

		free(backend_info);
	}

	pcp_disconnect();

	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "pcp_node_info - display a pgpool-II node's information\n\n");
	fprintf(stderr, "Usage: pcp_node_info [-d] timeout hostname port# username password nodeID\n");
	fprintf(stderr, "  -d, --debug    : enable debug message (optional)\n");
	fprintf(stderr, "  timeout        : connection timeout value in seconds. command exits on timeout\n");
	fprintf(stderr, "  hostname       : pgpool-II hostname\n");
	fprintf(stderr, "  port#          : PCP port number\n");
	fprintf(stderr, "  username       : username for PCP authentication\n");
	fprintf(stderr, "  password       : password for PCP authentication\n");
	fprintf(stderr, "  nodeID         : ID of a node to get information for\n\n");
	fprintf(stderr, "Usage: pcp_node_info [options]\n");
    fprintf(stderr, "  Options available are:\n");
	fprintf(stderr, "  -h, --help     : print this help\n");
	fprintf(stderr, "  -v, --verbose  : display one line per information with a header\n");
}

static void
myexit(ErrorCode e)
{
	if (e == INVALERR)
	{
		usage();
		exit(e);
	}

	exit(e);
}
