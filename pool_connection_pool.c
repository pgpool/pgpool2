/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2010	PgPool Global Development Group
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
 * poo_connection_pool.c: connection pool stuff
 */
#include "config.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#include <netdb.h>

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "pool.h"

POOL_CONNECTION_POOL *pool_connection_pool;	/* connection pool */
volatile sig_atomic_t backend_timer_expired = 0; /* flag for connection closed timer is expired */

static POOL_CONNECTION_POOL_SLOT *create_cp(POOL_CONNECTION_POOL_SLOT *cp, int slot);
static POOL_CONNECTION_POOL *new_connection(POOL_CONNECTION_POOL *p);
static int check_socket_status(int fd);

/*
* initialize connection pools. this should be called once at the startup.
*/
int pool_init_cp(void)
{
	int i;

	pool_connection_pool = (POOL_CONNECTION_POOL *)malloc(sizeof(POOL_CONNECTION_POOL)*pool_config->max_pool);
	if (pool_connection_pool == NULL)
	{
		pool_error("pool_init_cp: malloc() failed");
		return -1;
	}
	memset(pool_connection_pool, 0, sizeof(POOL_CONNECTION_POOL)*pool_config->max_pool);

	for (i = 0; i < pool_config->max_pool; i++)
	{
		pool_connection_pool[i].info = &(MY_PROCESS_INFO.connection_info[i]);
		memset(pool_connection_pool[i].info, 0, sizeof(ConnectionInfo));
	}
	return 0;
}

/*
* find connection by user and database
*/
POOL_CONNECTION_POOL *pool_get_cp(char *user, char *database, int protoMajor, int check_socket)
{
#ifdef HAVE_SIGPROCMASK
	sigset_t oldmask;
#else
	int	oldmask;
#endif

	int i, j, freed = 0;
	ConnectionInfo *info;

	POOL_CONNECTION_POOL *p = pool_connection_pool;

	if (p == NULL)
	{
		pool_error("pool_get_cp: pool_connection_pool is not initialized");
		return NULL;
	}

	POOL_SETMASK2(&BlockSig, &oldmask);

	for (i=0;i<pool_config->max_pool;i++)
	{
		if (MASTER_CONNECTION(p) &&
			MASTER_CONNECTION(p)->sp &&
			MASTER_CONNECTION(p)->sp->major == protoMajor &&
			MASTER_CONNECTION(p)->sp->user != NULL &&
			strcmp(MASTER_CONNECTION(p)->sp->user, user) == 0 &&
			strcmp(MASTER_CONNECTION(p)->sp->database, database) == 0)
		{
			int sock_broken = 0;

			/* mark this connection is under use */
			MASTER_CONNECTION(p)->closetime = 0;
			p->info->counter++;
			POOL_SETMASK(&oldmask);

			if (check_socket)
			{
				for (j=0;j<NUM_BACKENDS;j++)
				{
					if (!VALID_BACKEND(j))
						continue;

					if  (CONNECTION_SLOT(p, j))
					{
						sock_broken = check_socket_status(CONNECTION(p, j)->fd);
						if (sock_broken < 0)
							break;
					}
					else
					{
						sock_broken = -1;
						break;
					}
				}

				if (sock_broken < 0)
				{
					pool_log("connection closed. retry to create new connection pool.");
					for (j=0;j<NUM_BACKENDS;j++)
					{
						if (!VALID_BACKEND(j) || (CONNECTION_SLOT(p, j) == NULL))
							continue;

						if (!freed)
						{
							pool_free_startup_packet(CONNECTION_SLOT(p, j)->sp);
							freed = 1;
						}

						pool_close(CONNECTION(p, j));
						free(CONNECTION_SLOT(p, j));
					}
					info = p->info;
					memset(p, 0, sizeof(POOL_CONNECTION_POOL_SLOT));
					p->info = info;
					memset(p->info, 0, sizeof(ConnectionInfo));
					POOL_SETMASK(&oldmask);
					return NULL;
				}
			}
			POOL_SETMASK(&oldmask);
			return p;
		}
		p++;
	}

	POOL_SETMASK(&oldmask);
	return NULL;
}

/*
 * disconnect and release a connection to the database
 */
void pool_discard_cp(char *user, char *database, int protoMajor)
{
	POOL_CONNECTION_POOL *p = pool_get_cp(user, database, protoMajor, 0);
	ConnectionInfo *info;
	int i, freed = 0;

	if (p == NULL)
	{
		pool_error("pool_discard_cp: cannot get connection pool for user %s datbase %s", user, database);
		return;
	}

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		if (!freed)
		{
			pool_free_startup_packet(CONNECTION_SLOT(p, i)->sp);
			freed = 1;
		}
		pool_close(CONNECTION(p, i));
		free(CONNECTION_SLOT(p, i));
	}

	info = p->info;
	memset(p, 0, sizeof(POOL_CONNECTION_POOL));
	p->info = info;
	memset(p->info, 0, sizeof(ConnectionInfo));
}


/*
* create a connection pool by user and database
*/
POOL_CONNECTION_POOL *pool_create_cp(void)
{
	int i, freed = 0;
	time_t closetime;
	POOL_CONNECTION_POOL *oldestp;
	ConnectionInfo *info;

	POOL_CONNECTION_POOL *p = pool_connection_pool;

	if (p == NULL)
	{
		pool_error("pool_create_cp: pool_connection_pool is not initialized");
		return NULL;
	}

	for (i=0;i<pool_config->max_pool;i++)
	{
		if (MASTER_CONNECTION(p) == NULL)
			return new_connection(p);
		p++;
	}

	pool_debug("no empty connection slot was found");

	/*
	 * no empty connection slot was found. look for the oldest connection and discard it.
	 */
	oldestp = p = pool_connection_pool;
	closetime = MASTER_CONNECTION(p)->closetime;
	for (i=0;i<pool_config->max_pool;i++)
	{
		pool_debug("user: %s database: %s closetime: %ld",
				   MASTER_CONNECTION(p)->sp->user,
				   MASTER_CONNECTION(p)->sp->database,
				   MASTER_CONNECTION(p)->closetime);
		if (MASTER_CONNECTION(p)->closetime < closetime)
		{
			closetime = MASTER_CONNECTION(p)->closetime;
			oldestp = p;
		}
		p++;
	}

	p = oldestp;
	pool_send_frontend_exits(p);

	pool_debug("discarding old %zd th connection. user: %s database: %s",
			   oldestp - pool_connection_pool,
			   MASTER_CONNECTION(p)->sp->user,
			   MASTER_CONNECTION(p)->sp->database);

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		if (!freed)
		{
			pool_free_startup_packet(CONNECTION_SLOT(p, i)->sp);
			freed = 1;
		}

		pool_close(CONNECTION(p, i));
		free(CONNECTION_SLOT(p, i));
	}

	info = p->info;
	memset(p, 0, sizeof(POOL_CONNECTION_POOL));
	p->info = info;
	memset(p->info, 0, sizeof(ConnectionInfo));

	return new_connection(p);
}

/*
 * set backend connection close timer
 */
void pool_connection_pool_timer(POOL_CONNECTION_POOL *backend)
{
	POOL_CONNECTION_POOL *p = pool_connection_pool;
	int i;

	pool_debug("pool_connection_pool_timer: set close time %ld", time(NULL));

	MASTER_CONNECTION(backend)->closetime = time(NULL);		/* set connection close time */

	if (pool_config->connection_life_time == 0)
		return;

	/* look for any other timeout */
	for (i=0;i<pool_config->max_pool;i++, p++)
	{
		if (!MASTER_CONNECTION(p))
			continue;
		if (!MASTER_CONNECTION(p)->sp)
			continue;
		if (MASTER_CONNECTION(p)->sp->user == NULL)
			continue;

		if (p != backend && MASTER_CONNECTION(p)->closetime)
			return;
	}

	/* no other timer found. set my timer */
	pool_debug("pool_connection_pool_timer: set alarm after %d seconds", pool_config->connection_life_time);
	pool_signal(SIGALRM, pool_backend_timer_handler);
	alarm(pool_config->connection_life_time);
}

/*
 * backend connection close timer handler
 */
RETSIGTYPE pool_backend_timer_handler(int sig)
{
	backend_timer_expired = 1;
}

void pool_backend_timer(void)
{
#define TMINTMAX 0x7fffffff

	POOL_CONNECTION_POOL *p = pool_connection_pool;
	int i, j;
	time_t now;
	time_t nearest = TMINTMAX;
	ConnectionInfo *info;

	POOL_SETMASK(&BlockSig);

	now = time(NULL);

	pool_debug("pool_backend_timer_handler called at %ld", now);

	for (i=0;i<pool_config->max_pool;i++, p++)
	{
		if (!MASTER_CONNECTION(p))
			continue;
		if (!MASTER_CONNECTION(p)->sp)
			continue;
		if (MASTER_CONNECTION(p)->sp->user == NULL)
			continue;

		/* timer expire? */
		if (MASTER_CONNECTION(p)->closetime)
		{
			int freed = 0;

			pool_debug("pool_backend_timer_handler: expire time: %ld",
					   MASTER_CONNECTION(p)->closetime+pool_config->connection_life_time);

			if (now >= (MASTER_CONNECTION(p)->closetime+pool_config->connection_life_time))
			{
				/* discard expired connection */
				pool_debug("pool_backend_timer_handler: expires user %s database %s",
						   MASTER_CONNECTION(p)->sp->user, MASTER_CONNECTION(p)->sp->database);

				pool_send_frontend_exits(p);

				for (j=0;j<NUM_BACKENDS;j++)
				{
					if (!VALID_BACKEND(j))
						continue;

					if (!freed)
					{
						pool_free_startup_packet(CONNECTION_SLOT(p, j)->sp);
						freed = 1;
					}

					pool_close(CONNECTION(p, j));
					free(CONNECTION_SLOT(p, j));
				}
				info = p->info;
				memset(p, 0, sizeof(POOL_CONNECTION_POOL));
				p->info = info;
				memset(p->info, 0, sizeof(ConnectionInfo));
			}
			else
			{
				/* look for nearest timer */
				if (MASTER_CONNECTION(p)->closetime < nearest)
					nearest = MASTER_CONNECTION(p)->closetime;
			}
		}
	}

	/* any remaining timer */
	if (nearest != TMINTMAX)
	{
		nearest = pool_config->connection_life_time - (now - nearest);
		if (nearest <= 0)
		  nearest = 1;
		pool_signal(SIGALRM, pool_backend_timer_handler);
		alarm(nearest);
	}

	POOL_SETMASK(&UnBlockSig);
}

/*
 * connect to postmaster through INET domain socket
 */
int connect_inet_domain_socket(int slot, bool retry)
{
	char *host;
	int port;

	host = pool_config->backend_desc->backend_info[slot].backend_hostname;
	port = pool_config->backend_desc->backend_info[slot].backend_port;

	return connect_inet_domain_socket_by_port(host, port, retry);
}

/*
 * connect to postmaster through UNIX domain socket
 */
int connect_unix_domain_socket(int slot, bool retry)
{
	int port;
	char *socket_dir;

	port = pool_config->backend_desc->backend_info[slot].backend_port;
	socket_dir = pool_config->backend_socket_dir;

	return connect_unix_domain_socket_by_port(port, socket_dir, retry);
}

/*
 * Connect to PostgreSQL server by using UNIX domain socket.
 * If retry is true, retry to call connect() upon receiving EINTR error.
 */
int connect_unix_domain_socket_by_port(int port, char *socket_dir, bool retry)
{
	struct sockaddr_un addr;
	int fd;
	int len;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1)
	{
		pool_error("connect_unix_domain_socket_by_port: setsockopt() failed: %s", strerror(errno));
		return -1;
	}

	memset((char *) &addr, 0, sizeof(addr));
	((struct sockaddr *)&addr)->sa_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/.s.PGSQL.%d", socket_dir, port);
	len = sizeof(struct sockaddr_un);

	for (;;)
	{
		if (exit_request)		/* exit request already sent */
		{
			pool_log("connect_unix_domain_socket_by_port: exit request has been sent");
			return -1;
		}

		if (connect(fd, (struct sockaddr *)&addr, len) < 0)
		{
			if ((errno == EINTR && retry) || errno == EAGAIN)
				continue;

			pool_error("connect_unix_domain_socket_by_port: connect() failed: %s", strerror(errno));
			close(fd);
			return -1;
		}
		break;
	}

	return fd;
}

/*
 * Connect to PostgreSQL server by using INET domain socket.
 * If retry is true, retry to call connect() upon receiving EINTR error.
 */
int connect_inet_domain_socket_by_port(char *host, int port, bool retry)
{
	int fd;
	int len;
	int on = 1;
	struct sockaddr_in addr;
	struct hostent *hp;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		pool_error("connect_inet_domain_socket_by_port: socket() failed: %s", strerror(errno));
		return -1;
	}

	/* set nodelay */
	if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
				   (char *) &on,
				   sizeof(on)) < 0)
	{
		pool_error("connect_inet_domain_socket_by_port: setsockopt() failed: %s", strerror(errno));
		close(fd);
		return -1;
	}

	memset((char *) &addr, 0, sizeof(addr));
	((struct sockaddr *)&addr)->sa_family = AF_INET;

	addr.sin_port = htons(port);
	len = sizeof(struct sockaddr_in);

	hp = gethostbyname(host);
	if ((hp == NULL) || (hp->h_addrtype != AF_INET))
	{
		pool_error("connect_inet_domain_socket: gethostbyname() failed: %s host: %s", hstrerror(h_errno), host);
		close(fd);
		return -1;
	}
	memmove((char *) &(addr.sin_addr),
			(char *) hp->h_addr,
			hp->h_length);

	for (;;)
	{
		if (exit_request)		/* exit request already sent */
		{
			pool_log("connect_inet_domain_socket_by_port: exit request has been sent");
			return -1;
		}

		if (connect(fd, (struct sockaddr *)&addr, len) < 0)
		{
			if ((errno == EINTR && retry) || errno == EAGAIN)
				continue;

			pool_error("connect_inet_domain_socket: connect() failed: %s",strerror(errno));
			close(fd);
			return -1;
		}
		break;
	}

	return fd;
}

/*
 * create connection pool
 */
static POOL_CONNECTION_POOL_SLOT *create_cp(POOL_CONNECTION_POOL_SLOT *cp, int slot)
{
	BackendInfo *b = &pool_config->backend_desc->backend_info[slot];
	int fd;

	if (*b->backend_hostname == '\0')
	{
		fd = connect_unix_domain_socket(slot, TRUE);
	}
	else
	{
		fd = connect_inet_domain_socket(slot, TRUE);
	}

	if (fd < 0)
	{
		pool_error("connection to %s(%d) failed", b->backend_hostname, b->backend_port);
		return NULL;
	}

	cp->con = pool_open(fd);
	cp->closetime = 0;
	return cp;
}

/*
 * create actual connections to backends
 */
static POOL_CONNECTION_POOL *new_connection(POOL_CONNECTION_POOL *p)
{
	POOL_CONNECTION_POOL_SLOT *s;
	int active_backend_count = 0;
	int i;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		pool_debug("new_connection: connecting %d backend", i);

		if (!VALID_BACKEND(i))
		{
			pool_debug("new_connection: skipping slot %d because backend_status = %d",
					   i, BACKEND_INFO(i).backend_status);
			continue;
		}

		s = malloc(sizeof(POOL_CONNECTION_POOL_SLOT));
		if (s == NULL)
		{
			pool_error("new_connection: malloc() failed");
			return NULL;
		}

		if (create_cp(s, i) == NULL)
		{
			/* connection failed. mark this backend down */
			pool_error("new_connection: create_cp() failed");

			/* send failover request to parent if operated in pgpool-I mode */
			/* notice_backend_error() returns immediately if in pgpool-II */
			notice_backend_error(i);
			child_exit(1);
		}

		p->slots[i] = s;

		if (pool_init_params(&s->con->params))
		{
			return NULL;
		}

		BACKEND_INFO(i).backend_status = CON_UP;
		active_backend_count++;
	}

	if (active_backend_count > 0)
	{
		p->info->create_time = time(NULL);
		return p;
	}

	return NULL;
}

/* check_socket_status()
 * RETURN: 0 => OK
 *        -1 => broken socket.
 */
static int check_socket_status(int fd)
{
	fd_set rfds;
	int result;
	struct timeval t;

	for (;;)
	{
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		t.tv_sec = t.tv_usec = 0;

		result = select(fd+1, &rfds, NULL, NULL, &t);
		if (result < 0 && errno == EINTR)
		{
			continue;
		}
		else
		{
			return (result == 0 ? 0 : -1);
		}
	}

	return -1;
}
