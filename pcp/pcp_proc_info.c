/*
 * $Header$
 *
 * Client program to send "process info" command.
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
	int processID;
	ProcessInfo *process_info;
	int array_size;

	if (argc == 2 && (strcmp(argv[1], "-h") == 0) )
	{
		usage();
		exit(0);
	}

	if (argc != 7)
	{
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

	if (strlen(argv[2]) >= MAX_DB_HOST_NAMELEN)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}
	strcpy(host, argv[2]);

	port = atoi(argv[3]);
	if (port <= 1024 || port > 65535)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	if (strlen(argv[4]) >= MAX_USER_PASSWD_LEN)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}
	strcpy(user, argv[4]);

	if (strlen(argv[5]) >= MAX_USER_PASSWD_LEN)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}
	strcpy(pass, argv[5]);

	processID = atoi(argv[6]);
	if (processID < 0)
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

	if ((process_info = pcp_process_info(processID, &array_size)) == NULL)
	{
		pcp_errorstr(errorcode);
		pcp_disconnect();
		myexit(errorcode);
	} else {
		int i;
		for (i = 0; i < array_size; i++)
		{
			if (process_info->connection_info[i].database[0] == '\0')
				continue;
			
			printf("%s %s %ld %ld %d %d %d\n",
				   process_info->connection_info[i].database,
				   process_info->connection_info[i].user,
				   process_info->start_time,
				   process_info->connection_info[i].create_time,
				   process_info->connection_info[i].major,
				   process_info->connection_info[i].minor,
				   process_info->connection_info[i].counter);
		}
		free(process_info);
	}

	pcp_disconnect();

	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "pcp_proc_info - display a pgpool-II child process' information\n\n");
	fprintf(stderr, "Usage: pcp_proc_info timeout hostname port# username password PID\n");
	fprintf(stderr, "Usage: pcp_proc_info -h\n\n");
	fprintf(stderr, "  timeout  - connection timeout value in seconds. command exits on timeout\n");
	fprintf(stderr, "  hostname - pgpool-II hostname\n");
	fprintf(stderr, "  port#    - pgpool-II port number\n");
	fprintf(stderr, "  username - username for PCP authentication\n");
	fprintf(stderr, "  password - password for PCP authentication\n");
	fprintf(stderr, "  PID      - PID of a child process to get information for\n");
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
