/* -*-pgsql-c-*- */
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
 * pcp_child.c: PCP child process main
 *
 */
#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include <signal.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "pool.h"
#include "pcp/pcp_stream.h"
#include "pcp/pcp.h"
#include "md5.h"

#define MAX_FILE_LINE_LEN    512
#define MAX_USER_PASSWD_LEN  128

extern void pcp_set_timeout(long sec);
static int exit_request; /* non 0 means SIGTERM(smart shutdown) or SIGINT(fast shutdown) has arrived */

static RETSIGTYPE die(int sig);
static PCP_CONNECTION *pcp_do_accept(int unix_fd, int inet_fd);
static void unset_nonblock(int fd);
static int user_authenticate(char *buf, char *passwd_file, char *salt, int salt_len);
static void pool_random_salt(char *md5Salt);
static RETSIGTYPE wakeup_handler(int sig);
static RETSIGTYPE reload_config_handler(int sig);

extern int myargc;
extern char **myargv;

volatile sig_atomic_t pcp_got_sighup = 0;

void
pcp_do_child(int unix_fd, int inet_fd, char *pcp_conf_file)
{
	PCP_CONNECTION *frontend = NULL;
	int authenticated = 0;
	struct timeval uptime;
	char salt[4];
	int random_salt = 0;
	int i;
	char tos;
	int rsize;
	char *buf = NULL;

	pool_debug("I am PCP %d", getpid());

	/* Identify myself via ps */
	init_ps_display("", "", "", "");

	gettimeofday(&uptime, NULL);
	srandom((unsigned int) (getpid() ^ uptime.tv_usec));

	/* set pcp_read() timeout */
	pcp_set_timeout(pool_config->pcp_timeout);

	/* set up signal handlers */
	signal(SIGTERM, die);
	signal(SIGINT, die);
	signal(SIGHUP, reload_config_handler);
	signal(SIGQUIT, die);
	signal(SIGCHLD, SIG_DFL);
	signal(SIGUSR1, SIG_DFL);
	signal(SIGUSR2, wakeup_handler);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
	
	for(;;)
	{
		errno = 0;

		if (frontend == NULL)
		{
			frontend = pcp_do_accept(unix_fd, inet_fd);
			if (frontend == NULL)
				continue;

			unset_nonblock(frontend->fd);
		}

		/* read a PCP packet */
		if (pcp_read(frontend, &tos, 1))
		{
			if (errorcode == TIMEOUTERR)
			{
				pool_debug("pcp_child: pcp_read() has timed out");
				authenticated = 0;
				pcp_close(frontend); frontend = NULL;
				free(buf); buf = NULL;
				continue;
			}
			pool_error("pcp_child: pcp_read() failed. reason: %s", strerror(errno));
			exit(1);
		}
		if (pcp_read(frontend, &rsize, sizeof(int)))
		{
			if (errorcode == TIMEOUTERR)
			{
				pool_debug("pcp_child: pcp_read() has timed out");
				authenticated = 0;
				pcp_close(frontend); frontend = NULL;
				free(buf); buf = NULL;
				continue;
			}
			pool_error("pcp_child: pcp_read() failed. reason: %s", strerror(errno));
			exit(1);
		}

		rsize = ntohl(rsize);
		if ((rsize - sizeof(int)) > 0)
		{
			buf = (char *)malloc(rsize - sizeof(int));
			if (buf == NULL)
			{
				pool_error("pcp_child: malloc() failed. reason: %s", strerror(errno));
				exit(1);
			}
			if (pcp_read(frontend, buf, rsize - sizeof(int)))
			{
				if (errorcode == TIMEOUTERR)
				{
					pool_debug("pcp_child: pcp_read() has timed out");
					authenticated = 0;
					pcp_close(frontend); frontend = NULL;
					free(buf); buf = NULL;
					continue;
				}
				pool_error("pcp_child: pcp_read() failed. reason: %s", strerror(errno));
				exit(1);
			}
		}

		/* is this connection authenticated? if not disconnect immediately*/
		if ((! authenticated) && (tos != 'R' && tos != 'M'))
		{
			pool_debug("pcp_child: connection not authorized");					
			free(buf);
			buf = NULL;
			pcp_close(frontend);
			frontend = NULL;
			pool_signal(SIGALRM, SIG_IGN);
			continue;
		}

		/* process a request */
		pool_debug("pcp_child: received PCP packet type of service '%c'", tos);

		set_ps_display("PCP: processing a request", false);
			
		switch (tos)
		{
			case 'R':			/* authentication */
			{
				int wsize;

				if (random_salt)
				{
					authenticated = user_authenticate(buf, pcp_conf_file, salt, 4);					
				}
				if (!random_salt || !authenticated)
				{
					char code[] = "AuthenticationFailed";
					char mesg[] = "username and/or password do not match";
					pcp_write(frontend, "r", 1);
					wsize = htonl(sizeof(code) + sizeof(mesg) + sizeof(int));
					pcp_write(frontend, &wsize, sizeof(int));
					pcp_write(frontend, code, sizeof(code));
					pcp_write(frontend, mesg, sizeof(mesg));
					if (pcp_flush(frontend) < 0)
					{
						pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
						exit(1);
					}

					pool_error("pcp_child: authentication failed");
					pcp_close(frontend);
					frontend = NULL;
					random_salt = 0;
				}
				else
				{
					char code[] = "AuthenticationOK";
					pcp_write(frontend, "r", 1);
					wsize = htonl(sizeof(code) + sizeof(int));
					pcp_write(frontend, &wsize, sizeof(int));
					pcp_write(frontend, code, sizeof(code));
					if (pcp_flush(frontend) < 0)
					{
						pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
						exit(1);
					}
					random_salt = 0;

					pool_debug("pcp_child: authentication OK");
				}
				break;
			}

			case 'M':			/* md5 salt */
			{
				int wsize;

				pool_random_salt(salt);
				random_salt = 1;

				pcp_write(frontend, "m", 1);
				wsize = htonl(sizeof(int) + 4);
				pcp_write(frontend, &wsize, sizeof(int));
				pcp_write(frontend, salt, 4);
				if (pcp_flush(frontend) < 0)
				{
					pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
					exit(1);
				}

				pool_debug("pcp_child: salt sent to the client");
				break;
			}

			case 'L':			/* node count */
			{
				int wsize;
				int node_count = pool_get_node_count();

				char code[] = "CommandComplete";
				char mesg[16];

				snprintf(mesg, sizeof(mesg), "%d", node_count);
				
				pcp_write(frontend, "l", 1);
				wsize = htonl(sizeof(code) +
							  strlen(mesg)+1 +
							  sizeof(int));
				pcp_write(frontend, &wsize, sizeof(int));
				pcp_write(frontend, code, sizeof(code));
				pcp_write(frontend, mesg, strlen(mesg)+1);
				if (pcp_flush(frontend) < 0)
				{
					pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
					exit(1);
				}

				pool_debug("pcp_child: %d node(s) found", node_count);
				break;
			}
			case 'I':			/* node info */
			{
				int node_id;
				int wsize;
				
				BackendInfo *bi = NULL;

				node_id = atoi(buf);
				bi = pool_get_node_info(node_id);

				if (bi == NULL) {
					char code[] = "Invalid Node ID";

					pcp_write(frontend, "e", 1);
					wsize = htonl(sizeof(code) + sizeof(int));
					pcp_write(frontend, &wsize, sizeof(int));
					pcp_write(frontend, code, sizeof(code));
					if (pcp_flush(frontend) < 0)
					{
						pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
						exit(1);
					}

					pool_debug("pcp_child: invalid node ID");
				}
				else
				{
					char code[] = "CommandComplete";
					char port_str[6];
					char status[2];
					char weight_str[20];

					snprintf(port_str, sizeof(port_str), "%d", bi->backend_port);
					snprintf(status, sizeof(status), "%d", bi->backend_status);
					snprintf(weight_str, sizeof(weight_str), "%f", bi->backend_weight);
				
					pcp_write(frontend, "i", 1);
					wsize = htonl(sizeof(code) +
								  strlen(bi->backend_hostname)+1 +
								  strlen(port_str)+1 +
								  strlen(status)+1 +
								  strlen(weight_str)+1 +
								  sizeof(int));
					pcp_write(frontend, &wsize, sizeof(int));
					pcp_write(frontend, code, sizeof(code));
					pcp_write(frontend, bi->backend_hostname, strlen(bi->backend_hostname)+1);
					pcp_write(frontend, port_str, strlen(port_str)+1);
					pcp_write(frontend, status, strlen(status)+1);
					pcp_write(frontend, weight_str, strlen(weight_str)+1);
					if (pcp_flush(frontend) < 0)
					{
						pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
						exit(1);
					}

					pool_debug("pcp_child: retrieved node information from shared memory");
				}
				break;
			}

			case 'N':			/* process count */
			{
				int wsize;
				int process_count;
				char process_count_str[16];
				int *process_list = NULL;
				char code[] = "CommandComplete";
				char *mesg = NULL;
				int i;
				int total_port_len = 0;

				process_list = pool_get_process_list(&process_count);

				mesg = (char *)malloc(6*process_count);	/* port# is at most 5 characters long (MAX:65535) */
				if (mesg == NULL)
				{
					pool_error("pcp_child: malloc() failed. reason: %s", strerror(errno));
					exit(1);
				}

				snprintf(process_count_str, sizeof(process_count_str), "%d", process_count);

				for (i = 0; i < process_count; i++)
				{
					char port[6];
					snprintf(port, sizeof(port), "%d", process_list[i]);
					snprintf(mesg+total_port_len, strlen(port)+1, "%s", port);
					total_port_len += strlen(port)+1;
				}
				
				pcp_write(frontend, "n", 1);
				wsize = htonl(sizeof(code) +
							  strlen(process_count_str)+1 +
							  total_port_len +
							  sizeof(int));
				pcp_write(frontend, &wsize, sizeof(int));
				pcp_write(frontend, code, sizeof(code));
				pcp_write(frontend, process_count_str, strlen(process_count_str)+1);
				pcp_write(frontend, mesg, total_port_len);
				if (pcp_flush(frontend) < 0)
				{
					pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
					exit(1);
				}
				
				free(process_list);
				free(mesg);

				pool_debug("pcp_child: %d process(es) found", process_count);
				break;
			}
				
			case 'P':			/* process info */
			{
				int proc_id;
				int wsize;
				
				ProcessInfo *pi = NULL;

				proc_id = atoi(buf);
				pi = pool_get_process_info(proc_id);

				if (pi == NULL)
				{
					char code[] = "InvalidProcessID";

					pcp_write(frontend, "e", 1);
					wsize = htonl(sizeof(code) + sizeof(int));
					pcp_write(frontend, &wsize, sizeof(int));
					pcp_write(frontend, code, sizeof(code));
					if (pcp_flush(frontend) < 0)
					{
						pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
						exit(1);
					}

					pool_debug("pcp_child: invalid process ID");
				}
				else
				{
					/* First, send array size of connection_info */
					char arr_code[] = "ArraySize";
					char con_info_size[16];
					/* Finally, indicate that all data is sent */
					char fin_code[] = "CommandComplete";

					snprintf(con_info_size, sizeof(con_info_size), "%d", pool_config->max_pool);

					pcp_write(frontend, "p", 1);
					wsize = htonl(sizeof(arr_code) +
								  strlen(con_info_size)+1 +
								  sizeof(int));
					pcp_write(frontend, &wsize, sizeof(int));
					pcp_write(frontend, arr_code, sizeof(arr_code));
					pcp_write(frontend, con_info_size, strlen(con_info_size)+1);
					if (pcp_flush(frontend) < 0)
					{
						pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
						exit(1);
					}

					/* Second, send process information for all connection_info */

					for (i = 0; i < pool_config->max_pool; i++)
					{
						char code[] = "ProcessInfo";
						char proc_start_time[20];
						char proc_create_time[20];
						char majorversion[5];
						char minorversion[5];
						char pool_counter[16];

						snprintf(proc_start_time, sizeof(proc_start_time), "%ld", pi->start_time);
						snprintf(proc_create_time, sizeof(proc_create_time), "%ld", pi->connection_info[i].create_time);
						snprintf(majorversion, sizeof(majorversion), "%d", pi->connection_info[i].major);
						snprintf(minorversion, sizeof(minorversion), "%d", pi->connection_info[i].minor);
						snprintf(pool_counter, sizeof(pool_counter), "%d", pi->connection_info[i].counter);

						pcp_write(frontend, "p", 1);
						wsize = htonl(sizeof(code) +
									  strlen(pi->connection_info[i].database)+1 +
									  strlen(pi->connection_info[i].user)+1 +
									  strlen(proc_start_time)+1 +
									  strlen(proc_create_time)+1 +
									  strlen(majorversion)+1 +
									  strlen(minorversion)+1 +
									  strlen(pool_counter)+1 +
									  sizeof(int));
						pcp_write(frontend, &wsize, sizeof(int));
						pcp_write(frontend, code, sizeof(code));
						pcp_write(frontend, pi->connection_info[i].database, strlen(pi->connection_info[i].database)+1);
						pcp_write(frontend, pi->connection_info[i].user, strlen(pi->connection_info[i].user)+1);
						pcp_write(frontend, proc_start_time, strlen(proc_start_time)+1);
						pcp_write(frontend, proc_create_time, strlen(proc_create_time)+1);
						pcp_write(frontend, majorversion, strlen(majorversion)+1);
						pcp_write(frontend, minorversion, strlen(minorversion)+1);
						pcp_write(frontend, pool_counter, strlen(pool_counter)+1);
						if (pcp_flush(frontend) < 0)
						{
							pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
							exit(1);
						}
					}

					pcp_write(frontend, "p", 1);
					wsize = htonl(sizeof(fin_code) +
								  sizeof(int));
					pcp_write(frontend, &wsize, sizeof(int));
					pcp_write(frontend, fin_code, sizeof(fin_code));
					if (pcp_flush(frontend) < 0)
					{
						pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
						exit(1);
					}

					pool_debug("pcp_child: retrieved process information from shared memory");
				}
				break;
			}

			case 'S':			/* SystemDB info */
			{
				int wsize;
				
				SystemDBInfo *si = NULL;
				si = pool_get_system_db_info();

				/* since PCP clients can only see SystemDBInfo, set system_db_status from the shared memory */
				si->system_db_status = SYSDB_STATUS;

				if (si == NULL)
				{
					char code[] = "SystemDBNotDefined";

					pcp_write(frontend, "e", 1);
					wsize = htonl(sizeof(code) + sizeof(int));
					pcp_write(frontend, &wsize, sizeof(int));
					pcp_write(frontend, code, sizeof(code));
					if (pcp_flush(frontend) < 0)
					{
						pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
						exit(1);
					}
				}
				else
				{
					/* first, send systemDB information */
					char code[] = "SystemDBInfo";
					char port[6];
					char status[2];
					char dist_def_num[16];
					/* finally, indicate that all data is sent */
					char fin_code[] = "CommandComplete";

					snprintf(port, sizeof(port), "%d", si->port);
					snprintf(status, sizeof(status), "%d", si->system_db_status);
					snprintf(dist_def_num, sizeof(dist_def_num), "%d", si->dist_def_num);

					pcp_write(frontend, "s", 1);
					wsize = htonl(sizeof(code) +
								  strlen(si->hostname)+1 +
								  strlen(port)+1 +
								  strlen(si->user)+1 +
								  strlen(si->password)+1 +
								  strlen(si->schema_name)+1 +
								  strlen(si->database_name)+1 +
								  strlen(dist_def_num)+1 +
								  strlen(status)+1 +
								  sizeof(int));
					pcp_write(frontend, &wsize, sizeof(int));
					pcp_write(frontend, code, sizeof(code));
					pcp_write(frontend, si->hostname, strlen(si->hostname)+1);
					pcp_write(frontend, port, strlen(port)+1);
					pcp_write(frontend, si->user, strlen(si->user)+1);
					pcp_write(frontend, si->password, strlen(si->password)+1);
					pcp_write(frontend, si->schema_name, strlen(si->schema_name)+1);
					pcp_write(frontend, si->database_name, strlen(si->database_name)+1);
					pcp_write(frontend, dist_def_num, strlen(dist_def_num)+1);
					pcp_write(frontend, status, strlen(status)+1);
					if (pcp_flush(frontend) < 0)
					{
						pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
						exit(1);
					}

					/* second, send DistDefInfo if any */
					if (si->dist_def_num > 0)
					{
						char dist_code[] = "DistDefInfo";
						char col_num[16];
						int col_list_total_len;
						int type_list_total_len;
						char *col_list = NULL;
						char *type_list = NULL;
						int col_list_offset;
						int type_list_offset;
						DistDefInfo *ddi;
						int i, j;
					
						for (i = 0; i < si->dist_def_num; i++)
						{
							ddi = &si->dist_def_slot[i];
							snprintf(col_num, sizeof(col_num), "%d", ddi->col_num);

							col_list_total_len = type_list_total_len = 0;
							for (j = 0; j < ddi->col_num; j++)
							{
								col_list_total_len += strlen(ddi->col_list[j]) + 1;
								type_list_total_len += strlen(ddi->type_list[j]) + 1;
							}

							col_list = (char *)malloc(col_list_total_len);
							type_list = (char *)malloc(type_list_total_len);
							if (col_list == NULL || type_list == NULL)
							{
								pool_error("pcp_child: malloc() failed. reason: %s", strerror(errno));
								exit(1);
							}
						
							col_list_offset = type_list_offset = 0;
							for (j = 0; j < ddi->col_num; j++)
							{
								snprintf(col_list + col_list_offset, strlen(ddi->col_list[j])+1,
										 "%s", ddi->col_list[j]);
								snprintf(type_list + type_list_offset, strlen(ddi->type_list[j])+1,
										 "%s", ddi->type_list[j]);
								col_list_offset += strlen(ddi->col_list[j]) + 1;
								type_list_offset += strlen(ddi->type_list[j]) + 1;
							}

							pcp_write(frontend, "s", 1);
							wsize = htonl(sizeof(dist_code) +
										  strlen(ddi->dbname)+1 +
										  strlen(ddi->schema_name)+1 +
										  strlen(ddi->table_name)+1 +
										  strlen(ddi->dist_key_col_name)+1 +
										  strlen(col_num)+1 +
										  col_list_total_len +
										  type_list_total_len +
										  strlen(ddi->dist_def_func)+1 +
										  sizeof(int));
							pcp_write(frontend, &wsize, sizeof(int));
							pcp_write(frontend, dist_code, sizeof(dist_code));
							pcp_write(frontend, ddi->dbname, strlen(ddi->dbname)+1);
							pcp_write(frontend, ddi->schema_name, strlen(ddi->schema_name)+1);
							pcp_write(frontend, ddi->table_name, strlen(ddi->table_name)+1);
							pcp_write(frontend, ddi->dist_key_col_name, strlen(ddi->dist_key_col_name)+1);
							pcp_write(frontend, col_num, strlen(col_num)+1);
							pcp_write(frontend, col_list, col_list_total_len);
							pcp_write(frontend, type_list, type_list_total_len);
							pcp_write(frontend, ddi->dist_def_func, strlen(ddi->dist_def_func)+1);
							if (pcp_flush(frontend) < 0)
							{
								pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
								exit(1);
							}

							free(col_list);
							free(type_list);
						}
					}

					pcp_write(frontend, "s", 1);
					wsize = htonl(sizeof(fin_code) +
								  sizeof(int));
					pcp_write(frontend, &wsize, sizeof(int));
					pcp_write(frontend, fin_code, sizeof(fin_code));
					if (pcp_flush(frontend) < 0)
					{
						pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
						exit(1);
					}

					pool_debug("pcp_child: retrieved SystemDB information from shared memory");
				}
				break;
			}
			
			case 'D':			/* detach node */
			{
				int node_id;
				int wsize;
				char code[] = "CommandComplete";
				
				node_id = atoi(buf);
				pool_debug("pcp_child: detaching Node ID %d", node_id);
				notice_backend_error(node_id);

				pcp_write(frontend, "d", 1);
				wsize = htonl(sizeof(code) + sizeof(int));
				pcp_write(frontend, &wsize, sizeof(int));
				pcp_write(frontend, code, sizeof(code));
				if (pcp_flush(frontend) < 0)
				{
					pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
					exit(1);
				}
				break;
			}

			case 'C':			/* attach node */
			{
				int node_id;
				int wsize;
				char code[] = "CommandComplete";
				
				node_id = atoi(buf);
				pool_debug("pcp_child: attaching Node ID %d", node_id);
				send_failback_request(node_id);

				pcp_write(frontend, "c", 1);
				wsize = htonl(sizeof(code) + sizeof(int));
				pcp_write(frontend, &wsize, sizeof(int));
				pcp_write(frontend, code, sizeof(code));
				if (pcp_flush(frontend) < 0)
				{
					pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
					exit(1);
				}
				break;
			}
			
			case 'T':
			{
				char mode = buf[0];
				pid_t ppid = getppid();

				if (mode == 's')
				{
					pool_debug("pcp_child: sending SIGTERM to the parent process(%d)", ppid);
					kill(ppid, SIGTERM);
				}
				else if (mode == 'f')
				{
					pool_debug("pcp_child: sending SIGINT to the parent process(%d)", ppid);
					kill(ppid, SIGINT);
				}
				else if (mode == 'i')
				{
					pool_debug("pcp_child: sending SIGQUIT to the parent process(%d)", ppid);
					kill(ppid, SIGQUIT);
				}
				else
				{
					pool_debug("pcp_child: invalid shutdown mode %c", mode);
				}

				break;
			}

			case 'O': /* recovery request */
			{
				int node_id;
				int wsize;
				char code[] = "CommandComplete";
				int r;

				if (!REPLICATION)
				{
					int len = strlen("recovery request is accepted only in replication mode. ") + 1;
					pcp_write(frontend, "e", 1);
					wsize = htonl(sizeof(int) + len);
					pcp_write(frontend, &wsize, sizeof(int));
					pcp_write(frontend, "recovery request is accepted only in replication mode. ", len);
				}
				else
				{
					pool_debug("pcp_child: start online recovery");				
					node_id = atoi(buf);

					r = start_recovery(node_id);
					finish_recovery();

					if (r == 0) /* success */
					{
						pcp_write(frontend, "c", 1);
						wsize = htonl(sizeof(code) + sizeof(int));
						pcp_write(frontend, &wsize, sizeof(int));
						pcp_write(frontend, code, sizeof(code));
					}
					else
					{
						int len = strlen("recovery failed") + 1;
						pcp_write(frontend, "e", 1);
						wsize = htonl(sizeof(int) + len);
						pcp_write(frontend, &wsize, sizeof(int));
						pcp_write(frontend, "recovery failed", len);
					}
				}
				if (pcp_flush(frontend) < 0)
				{
					pool_error("pcp_child: pcp_flush() failed. reason: %s", strerror(errno));
					exit(1);
				}
			}
				break;

			case 'F':
				pool_debug("pcp_child: stop online recovery");
				break;
			
			case 'X':			/* disconnect */
				pool_debug("pcp_child: client disconnecting. close connection");
				authenticated = 0;
				pcp_close(frontend);
				frontend = NULL;
				break;

			default:
				pool_error("pcp_child: unknown packet type %c received", tos);
				exit(1);
		}

		free(buf);
		buf = NULL;

		/* seems ok. cancel idle check timer */
		pool_signal(SIGALRM, SIG_IGN);
	}

	exit(0);
}

static RETSIGTYPE
die(int sig)
{
	exit_request = 1;

	pool_debug("PCP child receives shutdown request signal %d", sig);

	switch (sig)
	{
		case SIGTERM:	/* smart shutdown */
		case SIGINT:	/* fast shutdown */
		case SIGQUIT:	/* immediate shutdown */
			exit(0);
			break;
		default:
			break;
	}

	/* send_frontend_exits(); */

	exit(0);
}

static RETSIGTYPE
wakeup_handler(int sig)
{

}

static PCP_CONNECTION *
pcp_do_accept(int unix_fd, int inet_fd)
{
	PCP_CONNECTION *pc = NULL;

	fd_set readmask;
	int fds;

	struct sockaddr addr;
	socklen_t addrlen;
	int fd = 0;
	int afd;
	int inet = 0;

	set_ps_display("PCP: wait for connection request", false);

	FD_ZERO(&readmask);
	FD_SET(unix_fd, &readmask);
	if (inet_fd)
		FD_SET(inet_fd, &readmask);

	fds = select(Max(unix_fd, inet_fd)+1, &readmask, NULL, NULL, NULL);

	if (fds == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
			return NULL;
		
		pool_error("pcp_child: select() failed. reason: %s", strerror(errno));
		return NULL;
	}

	if (FD_ISSET(unix_fd, &readmask))
	{
		fd = unix_fd;
	}

	if (FD_ISSET(inet_fd, &readmask))
	{
		fd = inet_fd;
		inet++;
	}

	addrlen = sizeof(addr);

	afd = accept(fd, &addr, &addrlen);
	if (afd < 0)
	{
		/*
		 * "Resource temporarily unavailable" (EAGAIN or EWOULDBLOCK)
		 * can be silently ignored.
		 */
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			pool_error("pcp_child: accept() failed. reason: %s", strerror(errno));
		return NULL;
	}

	if (pcp_got_sighup)
	{
		pool_get_config(get_config_file_name(), RELOAD_CONFIG);
		pcp_got_sighup = 0;
	}

	pool_debug("I am PCP %d accept fd %d", getpid(), afd);

	if (inet)
	{
		int on = 1;

		if (setsockopt(afd, IPPROTO_TCP, TCP_NODELAY,
					   (char *) &on,
					   sizeof(on)) < 0)
		{
			pool_error("pcp_child: setsockopt() failed: %s", strerror(errno));
			close(afd);
			return NULL;
		}
		if (setsockopt(afd, SOL_SOCKET, SO_KEEPALIVE,
					   (char *) &on,
					   sizeof(on)) < 0)
		{
			pool_error("pcp_child: setsockopt() failed: %s", strerror(errno));
			close(afd);
			return NULL;
		}
	}

	if ((pc = pcp_open(afd)) == NULL)
	{
		close(afd);
		return NULL;
	}
	return pc;
}

/*
 * unset non-block flag
 */
static void
unset_nonblock(int fd)
{
	int var;

	/* set fd to non-blocking */
	var = fcntl(fd, F_GETFL, 0);
	if (var == -1)
	{
		pool_error("pcp_child: fcntl failed. %s", strerror(errno));
		exit(1);
	}
	if (fcntl(fd, F_SETFL, var & ~O_NONBLOCK) == -1)
	{
		pool_error("pcp_child: fcntl failed. %s", strerror(errno));
		exit(1);
	}
}

/*
 * see if received username and password matches with one in the file
 */
static int
user_authenticate(char *buf, char *passwd_file, char *salt, int salt_len)
{
	FILE *fp = NULL;
	char packet_username[MAX_USER_PASSWD_LEN+1];
	char packet_password[MAX_USER_PASSWD_LEN+1];
	char encrypt_buf[(MD5_PASSWD_LEN+1)*2];
	char file_username[MAX_USER_PASSWD_LEN+1];
	char file_password[MD5_PASSWD_LEN+1];
	char *index = NULL;
	static char line[MAX_FILE_LINE_LEN+1];
	int i, len;

	/* strcpy() should be OK, but use strncpy() to be extra careful */
	strncpy(packet_username, buf, MAX_USER_PASSWD_LEN);
	index = (char *) memchr(buf, '\0', MAX_USER_PASSWD_LEN) + 1;
	if (index == NULL)
	{
		pool_debug("pcp_child: error while reading authentication packet");
		return 0;
	}
	strncpy(packet_password, index, MAX_USER_PASSWD_LEN);

	fp = fopen(passwd_file, "r");
	if (fp == NULL)
	{
		pool_error("pcp_child: could not open %s. reason: %s", passwd_file, strerror(errno));
		return 0;
	}

	/* for now, I don't care if duplicate username exists in the config file */
	while ((fgets(line, MAX_FILE_LINE_LEN, fp)) != NULL)
	{
		i = 0;
		len = 0;

		if (line[0] == '\n')
			continue;
		if (line[0] == '#')
			continue;

		while (line[i] != ':')
		{
			len++;
			if (++i > MAX_USER_PASSWD_LEN)
			{
				pool_error("pcp_child: user name in %s exceeds %d", passwd_file, MAX_USER_PASSWD_LEN);
				fclose(fp);
				return 0;
			}
		}
		memcpy(file_username, line, len);
		file_username[len] = '\0';

		if (strcmp(packet_username, file_username) != 0)
			continue;

		i++;
		len = 0;
		while (line[i] != '\n' && line[i] != '\0')
		{
			len++;
			if (++i > MAX_USER_PASSWD_LEN)
			{
				pool_error("pcp_child: password in %s exceeds %d", passwd_file, MAX_USER_PASSWD_LEN);
				fclose(fp);
				return 0;
			}
		}

		memcpy(file_password, line+strlen(file_username)+1, len);
		file_password[len] = '\0';

		pool_md5_encrypt(file_password, file_username, strlen(file_username),
					  encrypt_buf + MD5_PASSWD_LEN + 1);
		encrypt_buf[(MD5_PASSWD_LEN+1)*2-1] = '\0';

		pool_md5_encrypt(encrypt_buf+MD5_PASSWD_LEN+1, salt, salt_len,
					  encrypt_buf);
		encrypt_buf[MD5_PASSWD_LEN] = '\0';

		if (strcmp(encrypt_buf, packet_password) == 0)
		{
			fclose(fp);
			return 1;
		}
	}

	fclose(fp);
	return 0;
}

/*
 *  pool_random_salt
 */
static void pool_random_salt(char *md5Salt)
{
	long rand = random();

	md5Salt[0] = (rand % 255) + 1;
	rand = random();
	md5Salt[1] = (rand % 255) + 1;
	rand = random();
	md5Salt[2] = (rand % 255) + 1;
	rand = random();
	md5Salt[3] = (rand % 255) + 1;
}

/* SIGHUP handler */
static RETSIGTYPE reload_config_handler(int sig)
{
	pcp_got_sighup = 1;
}
