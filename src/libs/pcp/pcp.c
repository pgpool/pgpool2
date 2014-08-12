/*
 * $Header$
 *
 * Handles PCP connection, and protocol communication with pgpool-II
 * These are client APIs. Server program should use APIs in pcp_stream.c
 *
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>

#include "pool.h"
#include "pcp/pcp.h"
#include "pcp/pcp_stream.h"
#include "utils/palloc.h"
#include "utils/pool_process_reporting.h"
#include "auth/md5.h"


static PCP_CONNECTION *pc;
#ifdef DEBUG
static int debug = 1;
#else
static int debug = 0;
#endif
static int pcp_authorize(char *username, char *password);

static int _pcp_detach_node(int nid, bool gracefully);
static int _pcp_promote_node(int nid, bool gracefully);

/* --------------------------------
 * pcp_connect - open connection to pgpool using given arguments
 *
 * return 0 on success, -1 otherwise
 * --------------------------------
 */
int
pcp_connect(char *hostname, int port, char *username, char *password)
{
	struct sockaddr_in addr;
	struct sockaddr_un unix_addr;
	struct hostent *hp;
	int fd;
	int on = 1;
	int len;

	if (pc != NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection to backend \"%s\" already exists\n", hostname);
		return 0;
	}

	if (hostname == NULL || *hostname == '\0' || *hostname == '/')
	{
		char *path;

		fd = socket(AF_UNIX, SOCK_STREAM, 0);

		if (fd < 0)
		{
			if (debug) fprintf(stderr, "DEBUG: could not create socket\n");
			errorcode = SOCKERR;
			return -1;
		}

		memset(&unix_addr, 0, sizeof(unix_addr));
		unix_addr.sun_family = AF_UNIX;

		if (hostname == NULL || *hostname == '\0')
		{
			path = UNIX_DOMAIN_PATH;
		}
		else
		{
			path = hostname;
		}

		snprintf(unix_addr.sun_path, sizeof(unix_addr.sun_path), "%s/.s.PGSQL.%d",
				 path, port);

		if (connect(fd, (struct sockaddr *) &unix_addr, sizeof(unix_addr)) < 0)
		{
			if (debug) fprintf(stderr, "DEBUG: could not connect to \"%s\"\n", unix_addr.sun_path);
			close(fd);
			errorcode = CONNERR;
			return -1;
		}
	}
	else
	{
		fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0)
		{
		  	if (debug) fprintf(stderr, "DEBUG: could not create socket\n");
			errorcode = SOCKERR;
			return -1;
		}

		if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
					   (char *) &on, sizeof(on)) < 0)
		{
			if (debug) fprintf(stderr, "DEBUG: could not set socket option\n");
			close(fd);
			errorcode = SOCKERR;
			return -1;
		}

		memset((char *) &addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		hp = gethostbyname(hostname);
		if ((hp == NULL) || (hp->h_addrtype != AF_INET))
		{
			if (debug) fprintf(stderr, "DEBUG: could not retrieve hostname\n");
			close(fd);
			errorcode = HOSTERR;
			return -1;
		}
		memmove((char *) &(addr.sin_addr),
				(char *) hp->h_addr,
				hp->h_length);
		addr.sin_port = htons(port);

		len = sizeof(struct sockaddr_in);
		if (connect(fd, (struct sockaddr *) &addr, len) < 0)
		{
			if (debug) fprintf(stderr, "DEBUG: could not connect to \"%s\"\n", hostname);
			close(fd);
			errorcode = CONNERR;
			return -1;
		}
	}

	pc = pcp_open(fd);
	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: could not allocate buffer space\n");
		close(fd);
		return -1;
	}

	if (pcp_authorize(username, password) < 0)
	{
		pcp_close(pc);
		pc = NULL;
		return -1;
	}

	return 0;
}

/* --------------------------------
 * pcp_authorize - authenticate with pgpool using username and password
 *
 * return 0 on success, -1 otherwise
 * --------------------------------
 */
static int
pcp_authorize(char *username, char *password)
{
	char tos;
	char *buf = NULL;
	int wsize;
	int rsize;
	char salt[4];
	char encrypt_buf[(MD5_PASSWD_LEN+1)*2];
	char md5[MD5_PASSWD_LEN+1];

	/* request salt */
	pcp_write(pc, "M", 1);
	wsize = htonl(sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	if (pcp_flush(pc) < 0)
	{
		if (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return -1;
	}

	if (pcp_read(pc, &tos, 1))
		return -1;
	if (pcp_read(pc, &rsize, sizeof(int)))
		return -1;
	rsize = ntohl(rsize);
	buf = (char *)palloc(rsize);
	if (pcp_read(pc, buf, rsize - sizeof(int)))
		return -1;
	memcpy(salt, buf, 4);
	pfree(buf);

	/* encrypt password */
	pool_md5_hash(password, strlen(password), md5);
	md5[MD5_PASSWD_LEN] = '\0';

	pool_md5_encrypt(md5, username, strlen(username),
					 encrypt_buf + MD5_PASSWD_LEN + 1);
	encrypt_buf[(MD5_PASSWD_LEN+1)*2-1] = '\0';

	pool_md5_encrypt(encrypt_buf+MD5_PASSWD_LEN+1, salt, 4,
					 encrypt_buf);
	encrypt_buf[MD5_PASSWD_LEN] = '\0';

	pcp_write(pc, "R", 1);
	wsize = htonl((strlen(username)+1 + strlen(encrypt_buf)+1) + sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	pcp_write(pc, username, strlen(username)+1);
	pcp_write(pc, encrypt_buf, strlen(encrypt_buf)+1);
	if (pcp_flush(pc) < 0)
	{
		if  (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return -1;
	}
	if (debug) fprintf(stderr, "DEBUG: send: tos=\"R\", len=%d\n", ntohl(wsize));

	if (pcp_read(pc, &tos, 1))
		return -1;
	if (pcp_read(pc, &rsize, sizeof(int)))
		return -1;
	rsize = ntohl(rsize);
	buf = (char *)palloc(rsize);
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		free(buf);
		return -1;
	}
	if (debug) fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);

	if (tos == 'e')
	{
		if (debug) fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
		errorcode = BACKENDERR;
	}
	else if (tos == 'r')
	{
		if (strcmp(buf, "AuthenticationOK") == 0)
		{
			pfree(buf);
			return 0;
		}

		if (debug) fprintf(stderr, "DEBUG: authentication failed. reason=%s\n", buf);
		errorcode = AUTHERR;
	}
	pfree(buf);

	return -1;
}

/* --------------------------------
 * pcp_disconnect - close connection to pgpool
 * --------------------------------
 */
void
pcp_disconnect(void)
{
	int wsize;

	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection does not exist\n");
		return;
	}

	pcp_write(pc, "X", 1);
	wsize = htonl(sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	if (pcp_flush(pc) < 0)
	{
		/* backend had closed connection already */
	}
	if (debug) fprintf(stderr, "DEBUG: send: tos=\"X\", len=%d\n", (int) sizeof(int));

	pcp_close(pc);
	pc = NULL;
}

/* --------------------------------
 * pcp_terminate_pgpool - send terminate packet
 *
 * return 0 on success, -1 otherwise
 * --------------------------------
 */
int
pcp_terminate_pgpool(char mode)
{
	int wsize;

	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection does not exist\n");
		errorcode = NOCONNERR;
		return -1;
	}

	pcp_write(pc, "T", 1);
	wsize = htonl(sizeof(int) + sizeof(char));
	pcp_write(pc, &wsize, sizeof(int));
	pcp_write(pc, &mode, sizeof(char));
	if (pcp_flush(pc) < 0)
	{
		if (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return -1;
	}
	if (debug) fprintf(stderr, "DEBUG: send: tos=\"T\", len=%d\n", ntohl(wsize));

	return 0;
}

/* --------------------------------
 * pcp_node_count - get number of nodes currently connected to pgpool
 *
 * return array of node IDs on success, -1 otherwise
 * --------------------------------
 */
int
pcp_node_count(void)
{
	char tos;
	char *buf = NULL;
	int wsize;
	int rsize;
	char *index = NULL;

	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection does not exist\n");
		errorcode = NOCONNERR;
		return -1;
	}

	pcp_write(pc, "L", 1);
	wsize = htonl(sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	if (pcp_flush(pc) < 0)
	{
		if (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return -1;
	}
	if (debug) fprintf(stderr, "DEBUG: send: tos=\"L\", len=%d\n", ntohl(wsize));

	if (pcp_read(pc, &tos, 1))
		return -1;
	if (pcp_read(pc, &rsize, sizeof(int)))
		return -1;
	rsize = ntohl(rsize);
	buf = (char *)palloc(rsize);
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		pfree(buf);
		return -1;
	}

	if (debug) fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);

	if (tos == 'e')
	{
		if (debug) fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
		errorcode = BACKENDERR;
	}
	else if (tos == 'l')
	{
		if (strcmp(buf, "CommandComplete") == 0)
		{
			index = (char *) memchr(buf, '\0', rsize) + 1;
			if (index != NULL)
			{
				int ret = atoi(index);
				pfree(buf);
				return ret;
			}
		}
	}

	pfree(buf);

	return -1;
}

/* --------------------------------
 * pcp_node_info - get information of node pointed by given argument
 *
 * return structure of node information on success, -1 otherwise
 * --------------------------------
 */
BackendInfo *
pcp_node_info(int nid)
{
	int wsize;
	char node_id[16];
	char tos;
	char *buf = NULL;
	int rsize;

	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection does not exist\n");
		errorcode = NOCONNERR;
		return NULL;
	}

	snprintf(node_id, sizeof(node_id), "%d", nid);

	pcp_write(pc, "I", 1);
	wsize = htonl(strlen(node_id)+1 + sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	pcp_write(pc, node_id, strlen(node_id)+1);
	if (pcp_flush(pc) < 0)
	{
		if (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return NULL;
	}
	if (debug) fprintf(stderr, "DEBUG: send: tos=\"I\", len=%d\n", ntohl(wsize));

	if (pcp_read(pc, &tos, 1))
		return NULL;	
	if (pcp_read(pc, &rsize, sizeof(int)))
		return NULL;	
	rsize = ntohl(rsize);
	buf = (char *)palloc(rsize);
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		pfree(buf);
		return NULL;
	}

	if (debug) fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);

	if (tos == 'e')
	{
		if (debug) fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
		errorcode = BACKENDERR;
		pfree(buf);
		return NULL;
	}
	else if (tos == 'i')
	{
		if (strcmp(buf, "CommandComplete") == 0)
		{
			char *index = NULL;
			BackendInfo* backend_info = NULL;

			backend_info = (BackendInfo *)palloc(sizeof(BackendInfo));
			if (backend_info == NULL)
			{
				errorcode = NOMEMERR;
				pfree(buf);
				return NULL;
			}

 			index = (char *) memchr(buf, '\0', rsize) + 1;
			if (index != NULL)
				strlcpy(backend_info->backend_hostname, index, sizeof(backend_info->backend_hostname));

			index = (char *) memchr(index, '\0', rsize) + 1;
			if (index != NULL)
				backend_info->backend_port = atoi(index);

			index = (char *) memchr(index, '\0', rsize) + 1;
			if (index != NULL)
				backend_info->backend_status = atoi(index);

			index = (char *) memchr(index, '\0', rsize) + 1;
			if (index != NULL)
				backend_info->backend_weight = atof(index);

			pfree(buf);
			return backend_info;
		}
	}

	pfree(buf);
	return NULL;
}

/* --------------------------------
 * pcp_node_count - get number of nodes currently connected to pgpool
 *
 * return array of pids on success, NULL otherwise
 * --------------------------------
 */
int *
pcp_process_count(int *pnum)
{
	char tos;
	char *buf = NULL;
	int wsize;
	int rsize;
	
	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection does not exist\n");
		errorcode = NOCONNERR;
		return NULL;
	}

	pcp_write(pc, "N", 1);
	wsize = htonl(sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	if (pcp_flush(pc) < 0)
	{
		if (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return NULL;
	}
	if (debug) fprintf(stderr, "DEBUG: send: tos=\"N\", len=%d\n", ntohl(wsize));

	if (pcp_read(pc, &tos, 1))
		return NULL;			
	if (pcp_read(pc, &rsize, sizeof(int)))
		return NULL;	
	rsize = ntohl(rsize);
	buf = (char *)palloc(rsize);
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		pfree(buf);
		return NULL;		
	}
	if (debug) fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);

	if (tos == 'e')
	{
		if (debug) fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
		pfree(buf);
		errorcode = BACKENDERR;
		return NULL;
	}
	else if (tos == 'n')
	{
		if (strcmp(buf, "CommandComplete") == 0)
		{
			int process_count;
			int *process_list = NULL;
			char *index = NULL;
			int i;

			index = (char *) memchr(buf, '\0', rsize) + 1;
			process_count = atoi(index);

			process_list = (int *)palloc(sizeof(int) * process_count);

			for (i = 0; i < process_count; i++)
			{
				index = (char *) memchr(index, '\0', rsize) + 1;
				process_list[i] = atoi(index);
			}

			*pnum = process_count;
			pfree(buf);
			return process_list;
		}
	}

	pfree(buf);
	return NULL;
}

/* --------------------------------
 * pcp_process_info - get information of node pointed by given argument
 *
 * return structure of process information on success, -1 otherwise
 * --------------------------------
 */
ProcessInfo *
pcp_process_info(int pid, int *array_size)
{
	int wsize;
	char process_id[16];
	char tos;
	char *buf = NULL;
	int rsize;

	ProcessInfo *process_info = NULL;
	ConnectionInfo *conn_info = NULL;
	int ci_size = 0;
	int offset = 0;

	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection does not exist\n");
		errorcode = NOCONNERR;
		return NULL;
	}

	snprintf(process_id, sizeof(process_id), "%d", pid);

	pcp_write(pc, "P", 1);
	wsize = htonl(strlen(process_id)+1 + sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	pcp_write(pc, process_id, strlen(process_id)+1);
	if (pcp_flush(pc) < 0)
	{
		if (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return NULL;
	}
	if (debug) fprintf(stderr, "DEBUG: send: tos=\"P\", len=%d\n", ntohl(wsize));

	while (1)
	{
		if (pcp_read(pc, &tos, 1))
			return NULL;
		if (pcp_read(pc, &rsize, sizeof(int)))
			return NULL;
		rsize = ntohl(rsize);
		buf = (char *)palloc(rsize);
		if (pcp_read(pc, buf, rsize - sizeof(int)))
		{
			pfree(buf);
			return NULL;
		}
		if (debug) fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);

		if (tos == 'e')
		{
			if (debug) fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
			pfree(buf);
			errorcode = BACKENDERR;
			return NULL;
		}
		else if (tos == 'p')
		{
			char *index;

			if (strcmp(buf, "ArraySize") == 0)
			{
				index = (char *) memchr(buf, '\0', rsize) + 1;
				if (index != NULL)
					ci_size = atoi(index);

				*array_size = ci_size;

				process_info = (ProcessInfo *)palloc(sizeof(ProcessInfo) * ci_size);
				conn_info = (ConnectionInfo *)palloc(sizeof(ConnectionInfo) * ci_size);

				continue;
			}
			else if (strcmp(buf, "ProcessInfo") == 0)
			{
				if(process_info == NULL)
				{
					if (debug) fprintf(stderr, "DEBUG: invalid data.\"%s\"\n", buf);
					pfree(buf);
					errorcode = UNKNOWNERR;
					return NULL;
				}

				process_info[offset].connection_info = &conn_info[offset];

				index = (char *) memchr(buf, '\0', rsize) + 1;
				if (index != NULL)
					process_info[offset].pid = atoi(index);
			
				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					strlcpy(process_info[offset].connection_info->database, index, SM_DATABASE);
			
				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					strlcpy(process_info[offset].connection_info->user, index, SM_USER);
			
				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					process_info[offset].start_time = atol(index);

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					process_info[offset].connection_info->create_time = atol(index);

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					process_info[offset].connection_info->major = atoi(index);

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					process_info[offset].connection_info->minor = atoi(index);

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					process_info[offset].connection_info->counter = atoi(index);

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					process_info[offset].connection_info->backend_id = atoi(index);

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					process_info[offset].connection_info->pid = atoi(index);

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					process_info[offset].connection_info->connected = atoi(index);

				offset++;
			}
			else if (strcmp(buf, "CommandComplete") == 0)
			{
				pfree(buf);
				return process_info;
			}
			else
			{
				/* never reached */
			}
		}
	}

	pfree(buf);
	return NULL;
}

/* --------------------------------
 * pcp_systemdb_info - get information of system DB
 *
 * return structure of system DB information on success, -1 otherwise
 * --------------------------------
 */
SystemDBInfo *
pcp_systemdb_info(void)
{
	char tos;
	char *buf = NULL;
	int wsize;
	int rsize;
	SystemDBInfo *systemdb_info = NULL;
	int offset = 0;

	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection does not exist\n");
		errorcode = NOCONNERR;
		return NULL;
	}

	pcp_write(pc, "S", 1);
	wsize = htonl(sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	if (pcp_flush(pc) < 0)
	{
		if (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return NULL;
	}
	if (debug) fprintf(stderr, "DEBUG: send: tos=\"S\", len=%d\n", ntohl(wsize));

	while (1)
	{
		if (pcp_read(pc, &tos, 1))
		{
			if (buf != NULL)
				pfree(buf);
			return NULL;
		}

		if (pcp_read(pc, &rsize, sizeof(int)))
		{
			if (buf != NULL)
				pfree(buf);
			return NULL;
		}

		rsize = ntohl(rsize);
		buf = (char *)palloc(rsize);
		if (buf == NULL)
		{
			errorcode = NOMEMERR;
			return NULL;
		}
		if (pcp_read(pc, buf, rsize - sizeof(int)))
		{
			pfree(buf);
			return NULL;
		}
		if (debug) fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);

		if (tos == 'e')
		{
			if (debug) fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
			pfree(buf);
			errorcode = BACKENDERR;
			return NULL;
		}
		else if (tos == 's')
		{
			char *index;

			if (strcmp(buf, "SystemDBInfo") == 0)
			{
				systemdb_info = (SystemDBInfo *)palloc(sizeof(SystemDBInfo));
				if (systemdb_info == NULL)
				{
					pfree(buf);
					errorcode = NOMEMERR;
					return NULL;
				}

				index = (char *) memchr(buf, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->hostname = pstrdup(index);
				if (systemdb_info->hostname == NULL)
				{
					pfree(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}				
			
				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->port = atoi(index);
			
				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->user = pstrdup(index);
				if (systemdb_info->user == NULL)
				{
					pfree(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->password = pstrdup(index);
				if (systemdb_info->password == NULL)
				{
					pfree(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->schema_name = pstrdup(index);
				if (systemdb_info->schema_name == NULL)
				{
					pfree(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->database_name = pstrdup(index);
				if (systemdb_info->database_name == NULL)
				{
					pfree(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}
			
				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->dist_def_num = atoi(index);

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->system_db_status = atoi(index);

				if (systemdb_info->dist_def_num > 0)
				{
					systemdb_info->dist_def_slot = NULL;
					systemdb_info->dist_def_slot = (DistDefInfo *)palloc(sizeof(DistDefInfo) * systemdb_info->dist_def_num);
				}
			}
			else if (strcmp(buf, "DistDefInfo") == 0)
			{
				DistDefInfo *dist_def_info = NULL;
				int i;

				dist_def_info = (DistDefInfo *)palloc(sizeof(DistDefInfo));

				index = (char *) memchr(buf, '\0', rsize) + 1;
				if (index != NULL)
					dist_def_info->dbname = pstrdup(index);
			
				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					dist_def_info->schema_name = pstrdup(index);
				if (dist_def_info->schema_name == NULL)
				{
					pfree(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}
			
				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					dist_def_info->table_name = pstrdup(index);
				if (dist_def_info->table_name == NULL)
				{
					pfree(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					dist_def_info->dist_key_col_name = pstrdup(index);
				if (dist_def_info->dist_key_col_name == NULL)
				{
					pfree(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					dist_def_info->col_num = atoi(index);

				dist_def_info->col_list = NULL;
				dist_def_info->col_list = (char **)palloc(sizeof(char *) * dist_def_info->col_num);
				if (dist_def_info->col_list == NULL)
				{
					pfree(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}
				for (i = 0; i < dist_def_info->col_num; i++)
				{
					index = (char *) memchr(index, '\0', rsize) + 1;
					if (index != NULL)
						dist_def_info->col_list[i] = pstrdup(index);
					if (dist_def_info->col_list[i] == NULL)
					{
						pfree(buf);
						free_systemdb_info(systemdb_info);
						errorcode = NOMEMERR;
						return NULL;
					}
				}
			
				dist_def_info->type_list = NULL;
				dist_def_info->type_list = (char **)palloc(sizeof(char *) * dist_def_info->col_num);
				if (dist_def_info->type_list == NULL)
				{
					pfree(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}
				for (i = 0; i < dist_def_info->col_num; i++)
				{
					index = (char *) memchr(index, '\0', rsize) + 1;
					if (index != NULL)
						dist_def_info->type_list[i] = pstrdup(index);
					if (dist_def_info->type_list[i] == NULL)
					{
						pfree(buf);
						free_systemdb_info(systemdb_info);
						errorcode = NOMEMERR;
						return NULL;
					}
				}

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					dist_def_info->dist_def_func = pstrdup(index);
				if (dist_def_info->dist_def_func == NULL)
				{
					pfree(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}

				memcpy(&systemdb_info->dist_def_slot[offset++], dist_def_info, sizeof(DistDefInfo));
			}
			else if (strcmp(buf, "CommandComplete") == 0)
			{
				pfree(buf);
				return systemdb_info;
			}
			else
			{
				/* never reached */
			}
		}
	}
	
	pfree(buf);
	return NULL;
}

void
free_systemdb_info(SystemDBInfo * si)
{
	int i, j;

	if (si == NULL)
		return;
	pfree(si->hostname);
	pfree(si->user);
	pfree(si->password);
	pfree(si->schema_name);
	pfree(si->database_name);

	if (si->dist_def_slot != NULL)
	{
		for (i = 0; i < si->dist_def_num; i++)
		{
			DistDefInfo *di = &si->dist_def_slot[i];
			pfree(di->dbname);
			pfree(di->schema_name);
			pfree(di->table_name);
			pfree(di->dist_def_func);
			for (j = 0; j < di->col_num; j++)
			{
				pfree(di->col_list[j]);
				pfree(di->type_list[j]);
			}
		}
	}

	pfree(si);
}

/* --------------------------------
 * pcp_detach_node - detach a node given by the argument from pgpool's control
 *
 * return 0 on success, -1 otherwise
 * --------------------------------
 */
int
pcp_detach_node(int nid)
{
  return _pcp_detach_node(nid, FALSE);
}

/* --------------------------------

 * and detach a node given by the argument from pgpool's control
 *
 * return 0 on success, -1 otherwise
 * --------------------------------
 */
int
pcp_detach_node_gracefully(int nid)
{
  return _pcp_detach_node(nid, TRUE);
}

static int _pcp_detach_node(int nid, bool gracefully)
{
	int wsize;
	char node_id[16];
	char tos;
	char *buf = NULL;
	int rsize;
	char *sendchar;

	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection does not exist\n");
		errorcode = NOCONNERR;
		return -1;
	}

	snprintf(node_id, sizeof(node_id), "%d", nid);

	if (gracefully)
	  sendchar = "d";
	else
	  sendchar = "D";

	pcp_write(pc, sendchar, 1);
	wsize = htonl(strlen(node_id)+1 + sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	pcp_write(pc, node_id, strlen(node_id)+1);
	if (pcp_flush(pc) < 0)
	{
		if (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return -1;
	}
	if (debug) fprintf(stderr, "DEBUG: send: tos=\"D\", len=%d\n", ntohl(wsize));

	if (pcp_read(pc, &tos, 1))
		return -1;
	if (pcp_read(pc, &rsize, sizeof(int)))
		return -1;
	rsize = ntohl(rsize);
	buf = (char *)palloc(rsize);
	if (buf == NULL)
	{
		errorcode = NOMEMERR;
		return -1;
	}
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		pfree(buf);
		return -1;
	}
	if (debug) fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);

	if (tos == 'e')
	{
		if (debug) fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
		errorcode = BACKENDERR;
	}
	else if (tos == 'd')
	{
		/* strcmp() for success message, or fail */
		if(strcmp(buf, "CommandComplete") == 0)
		{
			pfree(buf);
			return 0;
		}
	}

	pfree(buf);
	return -1;
}


/* --------------------------------
 * pcp_attach_node - attach a node given by the argument from pgpool's control
 *
 * return 0 on success, -1 otherwise
 * --------------------------------
 */
int
pcp_attach_node(int nid)
{
	int wsize;
	char node_id[16];
	char tos;
	char *buf = NULL;
	int rsize;

	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection does not exist\n");
		errorcode = NOCONNERR;
		return -1;
	}

	snprintf(node_id, sizeof(node_id), "%d", nid);

	pcp_write(pc, "C", 1);
	wsize = htonl(strlen(node_id)+1 + sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	pcp_write(pc, node_id, strlen(node_id)+1);
	if (pcp_flush(pc) < 0)
	{
		if (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return -1;
	}
	if (debug) fprintf(stderr, "DEBUG: send: tos=\"D\", len=%d\n", ntohl(wsize));

	if (pcp_read(pc, &tos, 1))
		return -1;
	if (pcp_read(pc, &rsize, sizeof(int)))
		return -1;
	rsize = ntohl(rsize);
	buf = (char *)palloc(rsize);
	if (buf == NULL)
	{
		errorcode = NOMEMERR;
		return -1;
	}
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		pfree(buf);
		return -1;
	}
	if (debug) fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);

	if (tos == 'e')
	{
		if (debug) fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
		errorcode = BACKENDERR;
	}
	else if (tos == 'c')
	{
		/* strcmp() for success message, or fail */
		if(strcmp(buf, "CommandComplete") == 0)
		{
			pfree(buf);
			return 0;
		}
	}

	pfree(buf);
	return -1;
}

/* --------------------------------
 * pcp_pool_status - return setup parameters and status
 *
 * returns and array of POOL_REPORT_CONFIG, NULL otherwise
 * --------------------------------
 */
POOL_REPORT_CONFIG*
pcp_pool_status(int *array_size)
{
	char tos;
	char *buf = NULL;
	int wsize;
	int rsize;
	POOL_REPORT_CONFIG *status = NULL;
	int ci_size = 0;
	int offset = 0;

	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection does not exist\n");
		errorcode = NOCONNERR;
		return NULL;
	}

	pcp_write(pc, "B", 1);
	wsize = htonl(sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	if (pcp_flush(pc) < 0)
	{
		if (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return NULL;
	}
	if (debug) fprintf(stderr, "DEBUG pcp_pool_status: send: tos=\"B\", len=%d\n", ntohl(wsize));

	while (1) {
		if (pcp_read(pc, &tos, 1))
			return NULL;
		if (pcp_read(pc, &rsize, sizeof(int)))
			return NULL;
		rsize = ntohl(rsize);
		buf = (char *)palloc(rsize);
		if (buf == NULL)
		{
			errorcode = NOMEMERR;
			return NULL;
		}
		if (pcp_read(pc, buf, rsize - sizeof(int)))
		{
			pfree(buf);
			return NULL;
		}
		if (debug) fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);

		if (tos == 'e')
		{
			if (debug) fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
			pfree(buf);
			errorcode = BACKENDERR;
			return NULL;
		}
		else if (tos == 'b')
		{
			char *index;

			if (strcmp(buf, "ArraySize") == 0)
			{
				index = (char *) memchr(buf, '\0', rsize) + 1;
				ci_size = ntohl(*((int *)index));

				*array_size = ci_size;

				status = (POOL_REPORT_CONFIG *) palloc(ci_size * sizeof(POOL_REPORT_CONFIG));

				continue;
			}
			else if (strcmp(buf, "ProcessConfig") == 0)
			{
				if(status == NULL)
				{
					if (debug) fprintf(stderr, "DEBUG: invalid data.\"%s\"\n", buf);
					pfree(buf);
					errorcode = UNKNOWNERR;
					return NULL;
				}

				index = (char *) memchr(buf, '\0', rsize) + 1;
				if (index != NULL)
					strlcpy(status[offset].name, index, POOLCONFIG_MAXNAMELEN+1);

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					strlcpy(status[offset].value, index, POOLCONFIG_MAXVALLEN+1);

				index = (char *) memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					strlcpy(status[offset].desc, index, POOLCONFIG_MAXDESCLEN+1);

				offset++;
			}
			else if (strcmp(buf, "CommandComplete") == 0)
			{
				pfree(buf);
				return status;
			}
			else
			{
				/* never reached */
			}
		}
	}

	pfree(buf);
	return NULL;
}


int
pcp_recovery_node(int nid)
{
	int wsize;
	char node_id[16];
	char tos;
	char *buf = NULL;
	int rsize;

	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection does not exist\n");
		errorcode = NOCONNERR;
		return -1;
	}

	snprintf(node_id, sizeof(node_id), "%d", nid);

	pcp_write(pc, "O", 1);
	wsize = htonl(strlen(node_id)+1 + sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	pcp_write(pc, node_id, strlen(node_id)+1);
	if (pcp_flush(pc) < 0)
	{
		if (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return -1;
	}
	if (debug) fprintf(stderr, "DEBUG: send: tos=\"D\", len=%d\n", ntohl(wsize));

	if (pcp_read(pc, &tos, 1))
		return -1;
	if (pcp_read(pc, &rsize, sizeof(int)))
		return -1;
	rsize = ntohl(rsize);
	buf = (char *)palloc(rsize);
	if (buf == NULL)
	{
		errorcode = NOMEMERR;
		return -1;
	}
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		pfree(buf);
		return -1;
	}
	if (debug) fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);

	if (tos == 'e')
	{
		if (debug) fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
		errorcode = BACKENDERR;
	}
	else if (tos == 'c')
	{
		/* strcmp() for success message, or fail */
		if(strcmp(buf, "CommandComplete") == 0)
		{
			pfree(buf);
			return 0;
		}
	}

	pfree(buf);
	return -1;
}

void
pcp_enable_debug(void)
{
	debug = 1;
}

void
pcp_disable_debug(void)
{
	debug = 0;
}

/* --------------------------------
 * pcp_promote_node - promote a node given by the argument as new pgpool's master
 *
 * return 0 on success, -1 otherwise
 * --------------------------------
 */
int
pcp_promote_node(int nid)
{
  return _pcp_promote_node(nid, FALSE);
}

/* --------------------------------

 * and promote a node given by the argument as new pgpool's master
 *
 * return 0 on success, -1 otherwise
 * --------------------------------
 */
int
pcp_promote_node_gracefully(int nid)
{
  return _pcp_promote_node(nid, TRUE);
}

static int _pcp_promote_node(int nid, bool gracefully)
{
	int wsize;
	char node_id[16];
	char tos;
	char *buf = NULL;
	int rsize;
	char *sendchar;

	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection does not exist\n");
		errorcode = NOCONNERR;
		return -1;
	}

	snprintf(node_id, sizeof(node_id), "%d", nid);

	if (gracefully)
	  sendchar = "j";
	else
	  sendchar = "J";

	pcp_write(pc, sendchar, 1);
	wsize = htonl(strlen(node_id)+1 + sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	pcp_write(pc, node_id, strlen(node_id)+1);
	if (pcp_flush(pc) < 0)
	{
		if (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return -1;
	}
	if (debug) fprintf(stderr, "DEBUG: send: tos=\"E\", len=%d\n", ntohl(wsize));

	if (pcp_read(pc, &tos, 1))
		return -1;
	if (pcp_read(pc, &rsize, sizeof(int)))
		return -1;
	rsize = ntohl(rsize);
	buf = (char *)palloc(rsize);
	if (buf == NULL)
	{
		errorcode = NOMEMERR;
		return -1;
	}
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		pfree(buf);
		return -1;
	}
	if (debug) fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);

	if (tos == 'e')
	{
		if (debug) fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
		errorcode = BACKENDERR;
	}
	else if (tos == 'd')
	{
		/* strcmp() for success message, or fail */
		if(strcmp(buf, "CommandComplete") == 0)
		{
			pfree(buf);
			return 0;
		}
	}

	pfree(buf);
	return -1;
}

/* --------------------------------
 * pcp_watchdog_info - get information of watchdog
 *
 * return structure of watchdog information on success, -1 otherwise
 * --------------------------------
 */
WdInfo *
pcp_watchdog_info(int nid)
{
	int wsize;
	char wd_index[16];
	char tos;
	char *buf = NULL;
	int rsize;

	if (pc == NULL)
	{
		if (debug) fprintf(stderr, "DEBUG: connection does not exist\n");
		errorcode = NOCONNERR;
		return NULL;
	}

	snprintf(wd_index, sizeof(wd_index), "%d", nid);

	pcp_write(pc, "W", 1);
	wsize = htonl(strlen(wd_index)+1 + sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	pcp_write(pc, wd_index, strlen(wd_index)+1);
	if (pcp_flush(pc) < 0)
	{
		if (debug) fprintf(stderr, "DEBUG: could not send data to backend\n");
		return NULL;
	}
	if (debug) fprintf(stderr, "DEBUG: send: tos=\"W\", len=%d\n", ntohl(wsize));

	if (pcp_read(pc, &tos, 1))
		return NULL;	
	if (pcp_read(pc, &rsize, sizeof(int)))
		return NULL;	
	rsize = ntohl(rsize);
	buf = (char *)palloc(rsize);
	if (buf == NULL)
	{
		errorcode = NOMEMERR;
		return NULL;
	}
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		pfree(buf);
		return NULL;
	}

	if (debug) fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);

	if (tos == 'e')
	{
		if (debug) fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
		errorcode = BACKENDERR;
		pfree(buf);
		return NULL;
	}
	else if (tos == 'w')
	{
		if (strcmp(buf, "CommandComplete") == 0)
		{
			char *index = NULL;
			WdInfo* watchdog_info = NULL;

			watchdog_info = (WdInfo *)palloc(sizeof(WdInfo));
			if (watchdog_info == NULL)
			{
				errorcode = NOMEMERR;
				pfree(buf);
				return NULL;
			}

			index = (char *) memchr(buf, '\0', rsize) + 1;
			if (index != NULL)
				strlcpy(watchdog_info->hostname, index, sizeof(watchdog_info->hostname));

			index = (char *) memchr(index, '\0', rsize) + 1;
			if (index != NULL)
				watchdog_info->pgpool_port = atoi(index);

			index = (char *) memchr(index, '\0', rsize) + 1;
			if (index != NULL)
				watchdog_info->wd_port = atoi(index);

			index = (char *) memchr(index, '\0', rsize) + 1;
			if (index != NULL)
				watchdog_info->status = atof(index);

			pfree(buf);
			return watchdog_info;
		}
	}

	pfree(buf);
	return NULL;
}
