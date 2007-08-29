/*
 * $Header$
 *
 * Handles PCP connection, and protocol communication with pgpool-II
 * These are client APIs. Server program should use APIs in pcp_stream.c
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

#include "pcp.h"
#include "pcp_stream.h"
#include "md5.h"

struct timeval pcp_timeout;

static PCP_CONNECTION *pc;
static int pcp_authorize(char *username, char *password);

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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: connection to backend \"%s\" already exists\n", hostname);
#endif
		return 0;
	}

	if (hostname == NULL || *hostname == '\0' || *hostname == '/')
	{
		char *path;

		fd = socket(AF_UNIX, SOCK_STREAM, 0);

		if (fd < 0)
		{
#ifdef DEBUG
			fprintf(stderr, "DEBUG: could not create socket\n");
#endif
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
#ifdef DEBUG
			fprintf(stderr, "DEBUG: could not connect to \"%s\"\n", unix_addr.sun_path);
#endif
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
#ifdef DEBUG
			fprintf(stderr, "DEBUG: could not create socket\n");
#endif
			errorcode = SOCKERR;
			return -1;
		}

		if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
					   (char *) &on, sizeof(on)) < 0)
		{
#ifdef DEBUG
			fprintf(stderr, "DEBUG: could not set socket option\n");
#endif
			close(fd);
			errorcode = SOCKERR;
			return -1;
		}

		memset((char *) &addr, 0, sizeof(addr));
		((struct sockaddr *) &addr)->sa_family = AF_INET;
		hp = gethostbyname(hostname);
		if ((hp == NULL) || (hp->h_addrtype != AF_INET))
		{
#ifdef DEBUG
			fprintf(stderr, "DEBUG: could not retrieve hostname\n");
#endif
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
#ifdef DEBUG
			fprintf(stderr, "DEBUG: could not connect to \"%s\"\n", hostname);
#endif
			close(fd);
			errorcode = CONNERR;
			return -1;
		}
	}

	pc = pcp_open(fd);
	if (pc == NULL)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: could not allocate buffer space\n");
#endif
		close(fd);
		return -1;
	}

	if (pcp_authorize(username, password) < 0)
	{
		pcp_close(pc);
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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: could not send data to backend\n");
#endif
		return -1;
	}

	if (pcp_read(pc, &tos, 1))
		return -1;
	if (pcp_read(pc, &rsize, sizeof(int)))
		return -1;
	rsize = ntohl(rsize);
	buf = (char *)malloc(rsize);
	if (buf == NULL)
	{
		errorcode = NOMEMERR;
		return -1;
	}
	if (pcp_read(pc, buf, rsize - sizeof(int)))
		return -1;
	memcpy(salt, buf, 4);
	free(buf);

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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: could not send data to backend\n");
#endif
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: send: tos=\"R\", len=%d\n", ntohl(wsize));
#endif

	if (pcp_read(pc, &tos, 1))
		return -1;
	if (pcp_read(pc, &rsize, sizeof(int)))
		return -1;
	rsize = ntohl(rsize);
	buf = (char *)malloc(rsize);
	if (buf == NULL)
	{
		errorcode = NOMEMERR;
		return -1;
	}
	if (pcp_read(pc, buf, rsize - sizeof(int)))
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);
#endif

	if (tos == 'e')
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
#endif
		errorcode = BACKENDERR;
	}
	else if (tos == 'r')
	{
		if (strcmp(buf, "AuthenticationOK") == 0)
		{
			free(buf);
			return 0;
		}

#ifdef DEBUG
		fprintf(stderr, "DEBUG: authentication failed. reason=%s\n", buf);
#endif
		errorcode = AUTHERR;
	}
	free(buf);

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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: connection does not exist\n");
#endif
		return;
	}

	pcp_write(pc, "X", 1);
	wsize = htonl(sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	if (pcp_flush(pc) < 0)
	{
		/* backend had closed connection already */
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: send: tos=\"X\", len=%d\n", sizeof(int));
#endif

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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: connection does not exist\n");
#endif
		errorcode = NOCONNERR;
		return -1;
	}

	pcp_write(pc, "T", 1);
	wsize = htonl(sizeof(int) + sizeof(char));
	pcp_write(pc, &wsize, sizeof(int));
	pcp_write(pc, &mode, sizeof(char));
	if (pcp_flush(pc) < 0)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: could not send data to backend\n");
#endif
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: send: tos=\"T\", len=%d\n", ntohl(wsize));
#endif

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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: connection does not exist\n");
#endif
		errorcode = NOCONNERR;
		return -1;
	}

	pcp_write(pc, "L", 1);
	wsize = htonl(sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	if (pcp_flush(pc) < 0)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: could not send data to backend\n");
#endif
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: send: tos=\"L\", len=%d\n", ntohl(wsize));
#endif

	if (pcp_read(pc, &tos, 1))
		return -1;
	if (pcp_read(pc, &rsize, sizeof(int)))
		return -1;
	rsize = ntohl(rsize);
	buf = (char *)malloc(rsize);
	if (buf == NULL)
	{
		errorcode = NOMEMERR;
		return -1;
	}
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		free(buf);
		return -1;
	}

#ifdef DEBUG
	fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);
#endif

	if (tos == 'e')
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
#endif
		errorcode = BACKENDERR;
	}
	else if (tos == 'l')
	{
		if (strcmp(buf, "CommandComplete") == 0)
		{
			index = memchr(buf, '\0', rsize) + 1;
			if (index != NULL)
			{
				int ret = atoi(index);
				free(buf);
				return ret;
			}
		}
	}

	free(buf);

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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: connection does not exist\n");
#endif
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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: could not send data to backend\n");
#endif
		return NULL;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: send: tos=\"I\", len=%d\n", ntohl(wsize));
#endif

	if (pcp_read(pc, &tos, 1))
		return NULL;	
	if (pcp_read(pc, &rsize, sizeof(int)))
		return NULL;	
	rsize = ntohl(rsize);
	buf = (char *)malloc(rsize);
	if (buf == NULL)
	{
		errorcode = NOMEMERR;
		return NULL;
	}
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		free(buf);
		return NULL;
	}

#ifdef DEBUG
	fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);
#endif

	if (tos == 'e')
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
#endif
		errorcode = BACKENDERR;
		free(buf);
		return NULL;
	}
	else if (tos == 'i')
	{
		if (strcmp(buf, "CommandComplete") == 0)
		{
			char *index = NULL;
			BackendInfo* backend_info = NULL;

			backend_info = (BackendInfo *)malloc(sizeof(BackendInfo));
			if (backend_info == NULL)
			{
				errorcode = NOMEMERR;
				free(buf);
				return NULL;
			}
			// FIXME
//			rsize -= strlen("CommandComplete") + 1;

 			index = memchr(buf, '\0', rsize) + 1;
			if (index != NULL)
				strcpy(backend_info->backend_hostname, index);

			index = memchr(index, '\0', rsize) + 1;
			if (index != NULL)
				backend_info->backend_port = atoi(index);

			index = memchr(index, '\0', rsize) + 1;
			if (index != NULL)
				backend_info->backend_status = atoi(index);

			index = memchr(index, '\0', rsize) + 1;
			if (index != NULL)
				backend_info->backend_weight = atof(index);

			free(buf);
			return backend_info;
		}
	}

	free(buf);
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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: connection does not exist\n");
#endif
		errorcode = NOCONNERR;
		return NULL;
	}

	pcp_write(pc, "N", 1);
	wsize = htonl(sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	if (pcp_flush(pc) < 0)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: could not send data to backend\n");
#endif
		return NULL;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: send: tos=\"N\", len=%d\n", ntohl(wsize));
#endif

	if (pcp_read(pc, &tos, 1))
		return NULL;			
	if (pcp_read(pc, &rsize, sizeof(int)))
		return NULL;	
	rsize = ntohl(rsize);
	buf = (char *)malloc(rsize);
	if (buf == NULL)
	{
		errorcode = NOMEMERR;
		return NULL;
	}
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		free(buf);
		return NULL;		
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);
#endif

	if (tos == 'e')
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
#endif
		free(buf);
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

			index = memchr(buf, '\0', rsize) + 1;
			process_count = atoi(index);

			process_list = (int *)malloc(sizeof(int) * process_count);
			if (process_list == NULL)
			{
				free(buf);
				errorcode = NOMEMERR;
				return NULL;
			}

			for (i = 0; i < process_count; i++)
			{
				index = memchr(index, '\0', rsize) + 1;
				process_list[i] = atoi(index);
			}

			*pnum = process_count;
			free(buf);
			return process_list;
		}
	}

	free(buf);
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
	int ci_size = 0;
	int offset = 0;

	if (pc == NULL)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: connection does not exist\n");
#endif
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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: could not send data to backend\n");
#endif
		return NULL;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: send: tos=\"P\", len=%d\n", ntohl(wsize));
#endif

	while (1)
	{
		if (pcp_read(pc, &tos, 1))
			return NULL;
		if (pcp_read(pc, &rsize, sizeof(int)))
			return NULL;
		rsize = ntohl(rsize);
		buf = (char *)malloc(rsize);
		if (buf == NULL)
		{
			errorcode = NOMEMERR;
			return NULL;
		}
		if (pcp_read(pc, buf, rsize - sizeof(int)))
		{
			free(buf);
			return NULL;
		}
#ifdef DEBUG
		fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);
#endif

		if (tos == 'e')
		{
#ifdef DEBUG
			fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
#endif
			free(buf);
			errorcode = BACKENDERR;
			return NULL;
		}
		else if (tos == 'p')
		{
			char *index;

			if (strcmp(buf, "ArraySize") == 0)
			{
				index = memchr(buf, '\0', rsize) + 1;
				if (index != NULL)
					ci_size = atoi(index);

				*array_size = ci_size;
				
				process_info = (ProcessInfo *)malloc(sizeof(ProcessInfo));
				if (process_info == NULL)
				{
					free(buf);
					errorcode = NOMEMERR;
					return NULL;
				}
				process_info->connection_info = NULL;
				process_info->connection_info = (ConnectionInfo *)malloc(sizeof(ConnectionInfo)*ci_size);
				if (process_info->connection_info == NULL)
				{
					free(buf);
					errorcode = NOMEMERR;
					return NULL;
				}

				continue;
			}
			else if (strcmp(buf, "ProcessInfo") == 0)
			{
				index = memchr(buf, '\0', rsize) + 1;
				if (index != NULL)
					strcpy(process_info->connection_info[offset].database, index);
			
				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					strcpy(process_info->connection_info[offset].user, index);
			
				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					process_info->start_time = atol(index);

				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					process_info->connection_info[offset].create_time = atol(index);

				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					process_info->connection_info[offset].major = atoi(index);

				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					process_info->connection_info[offset].minor = atoi(index);

				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					process_info->connection_info[offset].counter = atoi(index);

				offset++;
			}
			else if (strcmp(buf, "CommandComplete") == 0)
			{
				free(buf);
				return process_info;
			}
			else
			{
				// never reached
			}
		}
	}

	free(buf);
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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: connection does not exist\n");
#endif
		errorcode = NOCONNERR;
		return NULL;
	}

	pcp_write(pc, "S", 1);
	wsize = htonl(sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	if (pcp_flush(pc) < 0)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: could not send data to backend\n");
#endif
		return NULL;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: send: tos=\"S\", len=%d\n", ntohl(wsize));
#endif

	while (1) {
		if (pcp_read(pc, &tos, 1))
			return NULL;
		if (pcp_read(pc, &rsize, sizeof(int)))
			return NULL;
		rsize = ntohl(rsize);
		buf = (char *)malloc(rsize);
		if (buf == NULL)
		{
			errorcode = NOMEMERR;
			return NULL;
		}
		if (pcp_read(pc, buf, rsize - sizeof(int)))
		{
			free(buf);
			return NULL;
		}
#ifdef DEBUG
		fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);
#endif

		if (tos == 'e')
		{
#ifdef DEBUG
			fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
#endif
			free(buf);
			errorcode = BACKENDERR;
			return NULL;
		}
		else if (tos == 's')
		{
			char *index;

			if (strcmp(buf, "SystemDBInfo") == 0)
			{
				systemdb_info = (SystemDBInfo *)malloc(sizeof(SystemDBInfo));
				if (systemdb_info == NULL)
				{
					free(buf);
					errorcode = NOMEMERR;
					return NULL;
				}

				index = memchr(buf, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->hostname = strdup(index);
				if (systemdb_info->hostname == NULL)
				{
					free(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}				
			
				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->port = atoi(index);
			
				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->user = strdup(index);
				if (systemdb_info->user == NULL)
				{
					free(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}

				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->password = strdup(index);
				if (systemdb_info->password == NULL)
				{
					free(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}

				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->schema_name = strdup(index);
				if (systemdb_info->schema_name == NULL)
				{
					free(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}

				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->database_name = strdup(index);
				if (systemdb_info->database_name == NULL)
				{
					free(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}
			
				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->dist_def_num = atoi(index);

				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					systemdb_info->system_db_status = atoi(index);

				if (systemdb_info->dist_def_num > 0)
				{
					systemdb_info->dist_def_slot = NULL;
					systemdb_info->dist_def_slot = (DistDefInfo *)malloc(sizeof(DistDefInfo) * systemdb_info->dist_def_num);
					if (systemdb_info->dist_def_slot == NULL)
					{
						free(buf);
						free_systemdb_info(systemdb_info);
						errorcode = NOMEMERR;
						return NULL;
					}
				}
			}
			else if (strcmp(buf, "DistDefInfo") == 0)
			{
				DistDefInfo *dist_def_info = NULL;
				int i;

				dist_def_info = (DistDefInfo *)malloc(sizeof(DistDefInfo));
				if (dist_def_info == NULL)
				{
					free(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}

				index = memchr(buf, '\0', rsize) + 1;
				if (index != NULL)
					dist_def_info->dbname = strdup(index);
				if (dist_def_info->dbname == NULL)
				{
					free(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}
			
				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					dist_def_info->schema_name = strdup(index);
				if (dist_def_info->schema_name == NULL)
				{
					free(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}
			
				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					dist_def_info->table_name = strdup(index);
				if (dist_def_info->table_name == NULL)
				{
					free(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}

				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					dist_def_info->dist_key_col_name = strdup(index);
				if (dist_def_info->dist_key_col_name == NULL)
				{
					free(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}

				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					dist_def_info->col_num = atoi(index);

				dist_def_info->col_list = NULL;
				dist_def_info->col_list = (char **)malloc(sizeof(char *) * dist_def_info->col_num);
				if (dist_def_info->col_list == NULL)
				{
					free(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}
				for (i = 0; i < dist_def_info->col_num; i++)
				{
					index = memchr(index, '\0', rsize) + 1;
					if (index != NULL)
						dist_def_info->col_list[i] = strdup(index);
					if (dist_def_info->col_list[i] == NULL)
					{
						free(buf);
						free_systemdb_info(systemdb_info);
						errorcode = NOMEMERR;
						return NULL;
					}
				}
			
				dist_def_info->type_list = NULL;
				dist_def_info->type_list = (char **)malloc(sizeof(char *) * dist_def_info->col_num);
				if (dist_def_info->type_list == NULL)
				{
					free(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}
				for (i = 0; i < dist_def_info->col_num; i++)
				{
					index = memchr(index, '\0', rsize) + 1;
					if (index != NULL)
						dist_def_info->type_list[i] = strdup(index);
					if (dist_def_info->type_list[i] == NULL)
					{
						free(buf);
						free_systemdb_info(systemdb_info);
						errorcode = NOMEMERR;
						return NULL;
					}
				}

				index = memchr(index, '\0', rsize) + 1;
				if (index != NULL)
					dist_def_info->dist_def_func = strdup(index);
				if (dist_def_info->dist_def_func == NULL)
				{
					free(buf);
					free_systemdb_info(systemdb_info);
					errorcode = NOMEMERR;
					return NULL;
				}

				memcpy(&systemdb_info->dist_def_slot[offset++], dist_def_info, sizeof(DistDefInfo));
			}
			else if (strcmp(buf, "CommandComplete") == 0)
			{
				free(buf);
				return systemdb_info;
			}
			else
			{
				// never reached
			}
		}
	}
	
	free(buf);
	return NULL;
}

void
free_systemdb_info(SystemDBInfo * si)
{
	int i, j;

	free(si->hostname);
	free(si->user);
	free(si->password);
	free(si->schema_name);
	free(si->database_name);

	if (si->dist_def_slot != NULL)
	{
		for (i = 0; i < si->dist_def_num; i++)
		{
			DistDefInfo *di = &si->dist_def_slot[i];
			free(di->dbname);
			free(di->schema_name);
			free(di->table_name);
			free(di->dist_def_func);
			for (j = 0; j < di->col_num; j++)
			{
				free(di->col_list[j]);
				free(di->type_list[j]);
			}
		}
	}

	free(si);
}

/* --------------------------------
 * pcp_detach_node - dettach a node given by the argument from pgpool's control
 *
 * return 0 on success, -1 otherwise
 * --------------------------------
 */
int
pcp_detach_node(int nid)
{
	int wsize;
	char node_id[16];
	char tos;
	char *buf = NULL;
	int rsize;

	if (pc == NULL)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: connection does not exist\n");
#endif
		errorcode = NOCONNERR;
		return -1;
	}

	snprintf(node_id, sizeof(node_id), "%d", nid);

	pcp_write(pc, "D", 1);
	wsize = htonl(strlen(node_id)+1 + sizeof(int));
	pcp_write(pc, &wsize, sizeof(int));
	pcp_write(pc, node_id, strlen(node_id)+1);
	if (pcp_flush(pc) < 0)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: could not send data to backend\n");
#endif
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: send: tos=\"D\", len=%d\n", ntohl(wsize));
#endif

	if (pcp_read(pc, &tos, 1))
		return -1;
	if (pcp_read(pc, &rsize, sizeof(int)))
		return -1;
	rsize = ntohl(rsize);
	buf = (char *)malloc(rsize);
	if (buf == NULL)
	{
		errorcode = NOMEMERR;
		return -1;
	}
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		free(buf);
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);
#endif

	if (tos == 'e')
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
#endif
		errorcode = BACKENDERR;
	}
	else if (tos == 'd')
	{
		/* strcmp() for success message, or fail */
		if(strcmp(buf, "CommandComplete") == 0)
		{
			free(buf);
			return 0;
		}
	}

	free(buf);
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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: connection does not exist\n");
#endif
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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: could not send data to backend\n");
#endif
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: send: tos=\"D\", len=%d\n", ntohl(wsize));
#endif

	if (pcp_read(pc, &tos, 1))
		return -1;
	if (pcp_read(pc, &rsize, sizeof(int)))
		return -1;
	rsize = ntohl(rsize);
	buf = (char *)malloc(rsize);
	if (buf == NULL)
	{
		errorcode = NOMEMERR;
		return -1;
	}
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		free(buf);
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);
#endif

	if (tos == 'e')
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
#endif
		errorcode = BACKENDERR;
	}
	else if (tos == 'c')
	{
		/* strcmp() for success message, or fail */
		if(strcmp(buf, "CommandComplete") == 0)
		{
			free(buf);
			return 0;
		}
	}

	free(buf);
	return -1;
}

void
pcp_set_timeout(long sec)
{
	pcp_timeout.tv_sec = sec;
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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: connection does not exist\n");
#endif
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
#ifdef DEBUG
		fprintf(stderr, "DEBUG: could not send data to backend\n");
#endif
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: send: tos=\"D\", len=%d\n", ntohl(wsize));
#endif

	if (pcp_read(pc, &tos, 1))
		return -1;
	if (pcp_read(pc, &rsize, sizeof(int)))
		return -1;
	rsize = ntohl(rsize);
	buf = (char *)malloc(rsize);
	if (buf == NULL)
	{
		errorcode = NOMEMERR;
		return -1;
	}
	if (pcp_read(pc, buf, rsize - sizeof(int)))
	{
		free(buf);
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: recv: tos=\"%c\", len=%d, data=%s\n", tos, rsize, buf);
#endif

	if (tos == 'e')
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: command failed. reason=%s\n", buf);
#endif
		errorcode = BACKENDERR;
	}
	else if (tos == 'c')
	{
		/* strcmp() for success message, or fail */
		if(strcmp(buf, "CommandComplete") == 0)
		{
			free(buf);
			return 0;
		}
	}

	free(buf);
	return -1;
}
