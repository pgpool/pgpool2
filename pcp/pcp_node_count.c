/*
 * $Header$
 *
 * Client program to send "node count" command.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcp.h"

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
	int node_count;

	if (argc == 2 && (strcmp(argv[1], "-h") == 0) )
	{
		usage();
		exit(0);
	}

	if (argc != 6) {
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	timeout = atol(argv[1]);
	if (timeout < 0) {
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	if (strlen(argv[2]) >= MAX_DB_HOST_NAMELEN) {
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}
	strcpy(host, argv[2]);

	port = atoi(argv[3]);
	if (port <= 1024 || port > 65535) {
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	if (strlen(argv[4]) >= MAX_USER_PASSWD_LEN) {
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}
	strcpy(user, argv[4]);

	if (strlen(argv[5]) >= MAX_USER_PASSWD_LEN) {
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}
	strcpy(pass, argv[5]);

	pcp_set_timeout(timeout);

	if (pcp_connect(host, port, user, pass))
	{
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	if ((node_count = pcp_node_count()) < 0)
	{
		pcp_errorstr(errorcode);
		pcp_disconnect();
		myexit(errorcode);
	} else {
		printf("%d\n", node_count);
	}

	pcp_disconnect();

	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "pcp_node_count - display the total number of nodes under pgpool-II's control\n\n");
	fprintf(stderr, "Usage: pcp_node_count timeout hostname port# username password\n");
	fprintf(stderr, "Usage: pcp_node_count -h\n\n");
	fprintf(stderr, "  timeout  - connection timeout value in seconds. command exits on timeout\n");
	fprintf(stderr, "  hostname - pgpool-II hostname\n");
	fprintf(stderr, "  port#    - pgpool-II port number\n");
	fprintf(stderr, "  username - username for PCP authentication\n");
	fprintf(stderr, "  password - password for PCP authentication\n");
	fprintf(stderr, "  -h       - print this help\n");
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
