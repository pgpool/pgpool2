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
 * pool_query_cache.c: query cache
 *
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include "pool.h"
#include "md5.h"

#define QUERY_CACHE_TABLE_NAME "query_cache"
#define CACHE_REGISTER_PREPARED_STMT "register_prepared_stmt"
#define DEFAULT_CACHE_SIZE 8192

#define SYSDB_CON (SYSDB_CONNECTION->con)
#define SYSDB_MAJOR (SYSDB_CONNECTION->sp->major)
#define CACHE_TABLE_INFO (SYSDB_INFO->query_cache_table_info)

/* data structure to store RowDescription and DataRow cache */
typedef struct
{
	char *md5_query;			/* query in md5 hushed format*/
	char *query;				/* query string */
	char *cache;				/* cached data */
	int   cache_size;			/* cached data size */
	int   cache_offset;			/* points the end of cache */
	char *db_name;				/* database name */
	char *create_time;			/* cache create timestamp in ISO format */
} QueryCacheInfo;

typedef enum
{
	CACHE_FOUND, CACHE_NOT_FOUND, CACHE_ERROR
} CACHE_STATUS;

static QueryCacheInfo *query_cache_info;

static CACHE_STATUS search_system_db_for_cache(POOL_CONNECTION *frontend, char *sql, int sql_len, struct timeval *t, char tstate);
static int ForwardCacheToFrontend(POOL_CONNECTION *frontend, char *cache, char tstate);
static int init_query_cache_info(POOL_CONNECTION *pc, char *database, char *query);
static void free_query_cache_info(void);
static int malloc_failed(void *p);
static int write_cache(void *buf, int len);
static char *pq_time_to_str(time_t t);
static int system_db_connection_exists(void);
static void define_prepared_statements(void);

/* --------------------------------
 * pool_clear_cache - clears cache data from the SystemDB
 *
 * return 0 on success, -1 otherwise
 * --------------------------------
 */
int
pool_clear_cache_by_time(Interval *interval, int size)
{
	PGresult *pg_result = NULL;
	long interval_in_days = 0;
	long interval_in_seconds = 0;
	time_t query_delete_timepoint;
	char *query_delete_timepoint_in_str = NULL;
	int sql_len;
	char *sql = NULL;
	int i;

	if (! system_db_connection_exists())
		return -1;

	for (i = 0; i < size; i++)
	{
		int q = interval[i].quantity;

		switch (interval[i].unit)
		{
			case millennium:
			case millenniums:
				q *= 10;

			case century:
			case centuries:
				q *= 10;

			case decade:
			case decades:
				q *= 10;

			case year:
			case years:
				q *= 12;

			case month:
			case months:
				q *= 31;		/* should I change this according to the month? */
				interval_in_days += q;
				break;

			case week:
			case weeks:
				q *= 7;

			case day:
			case days:
				q *= 24;

			case hour:
			case hours:
				q *= 60;

			case minute:
			case minutes:
				q *= 60;

			case second:
			case seconds:
				interval_in_seconds += q;
				break;
		}
	}

	interval_in_seconds = (interval_in_days * 86400) + interval_in_seconds;
	query_delete_timepoint = time(NULL) - interval_in_seconds;

	query_delete_timepoint_in_str = pq_time_to_str(query_delete_timepoint);
	if (malloc_failed(query_delete_timepoint_in_str))
		return -1;

	sql_len =
		strlen(pool_config->system_db_schema) +
		strlen(QUERY_CACHE_TABLE_NAME) +
		strlen(query_delete_timepoint_in_str) +
		64;

	sql = (char *)malloc(sql_len);
	if (malloc_failed(sql))
	{
		free(query_delete_timepoint_in_str);
		return -1;
	}

	snprintf(sql, sql_len,
			 "DELETE FROM %s.%s WHERE create_time <= '%s'",
			 pool_config->system_db_schema,
			 QUERY_CACHE_TABLE_NAME,
			 query_delete_timepoint_in_str);

	pool_debug("pool_clear_cache: delete all query cache created before '%s'", query_delete_timepoint_in_str);

	pg_result = PQexec(system_db_info->pgconn, sql);
	if (!pg_result || PQresultStatus(pg_result) != PGRES_COMMAND_OK)
	{
		pool_error("pool_clear_cache: PQexec() failed. reason: %s",
				   PQerrorMessage(system_db_info->pgconn));

		PQclear(pg_result);
		free(query_delete_timepoint_in_str);
		free(sql);
		return -1;
	}

	PQclear(pg_result);
	free(query_delete_timepoint_in_str);
	free(sql);

	return 0;
}

/* --------------------------------
 * pool_query_cache_table_exists - checks if query_cache table exists in the SystemDB
 *
 * This function is called once and only once from the pgpool parent process.
 * return 1 if query_cache table exists, 0 otherwise.
 * --------------------------------
 */
int
pool_query_cache_table_exists(void)
{
	PGresult *pg_result = NULL;
	char *sql = NULL;
	int sql_len = strlen(pool_config->system_db_schema) + strlen(QUERY_CACHE_TABLE_NAME) + 64;

	if (! system_db_connection_exists())
		return 0;

	sql = (char *)malloc(sql_len);
	if (malloc_failed(sql))
		return 0;

	snprintf(sql, sql_len,
			 "SELECT hash, query, value, dbname, create_time FROM %s.%s LIMIT 1",
			 pool_config->system_db_schema,
			 QUERY_CACHE_TABLE_NAME);

	pg_result = PQexec(system_db_info->pgconn, sql);
	if (!pg_result || PQresultStatus(pg_result) != PGRES_TUPLES_OK)
	{
		pool_error("pool_query_cache_table_exists: PQexec() failed. reason: %s",
				   PQerrorMessage(system_db_info->pgconn));

		PQclear(pg_result);
		free(sql);
		return 0;
	}

	PQclear(pg_result);
	pool_close_libpq_connection();
	free(sql);

	return 1;
}

/* --------------------------------
 * pool_query_cache_lookup - retrieve query cache from the SystemDB
 *
 * creates a SQL query string for searching a cache from the SystemDB.
 *
 * returns POOL_CONTINUE if cache is found. returns POOL_END if cache was
 * not found. returns POOL_ERROR if an error has been encountered while
 * searching.
 *
 * Note that POOL_END and POOL_ERROR are treated the same by the caller
 * (pool_process_query.c).
 * POOL_END and POOL_ERROR both indicates to the caller that the search
 * query must be forwarded to the backends in order to retrieve data and
 * the result be cached.
 * Only difference is that POOL_ERROR indicates that some fatal error has
 * occured; query cache function, however, should be seemless to the user
 * whether cache was not found or error has occured during cache retrieve.
 * --------------------------------
 */
POOL_STATUS
pool_query_cache_lookup(POOL_CONNECTION *frontend, char *query, char *database, char tstate)
{
	char *sql = NULL;
	int sql_len;
	char md5_query[33];
	struct timeval timeout;
	int status;

	if (! system_db_connection_exists())
		return POOL_ERROR;		/* same as POOL_END ... at least for now */

	sql_len =
		strlen(pool_config->system_db_schema) +
		strlen(QUERY_CACHE_TABLE_NAME) +
		sizeof(md5_query) +
		strlen(database) +
		64;
	sql = (char *)malloc(sql_len);
	if (malloc_failed(sql))
		return POOL_ERROR;		/* should I exit here rather than returning an error? */

	/* cached data lookup */
	pool_md5_hash(query, strlen(query), md5_query);
	snprintf(sql, sql_len, "SELECT value FROM %s.%s WHERE hash = '%s' AND dbname = '%s'",
			 pool_config->system_db_schema,
			 QUERY_CACHE_TABLE_NAME,
			 md5_query,
			 database);

	/* set timeout value for select */
	timeout.tv_sec = pool_config->child_life_time;
	timeout.tv_usec = 0;

	pool_debug("pool_query_cache_lookup: searching cache for query: \"%s\"", query);
	status = search_system_db_for_cache(frontend, sql, strlen(sql)+1, &timeout, tstate);

	/* make sure that the remaining data is discarded */
	SYSDB_CON->po = 0;
	SYSDB_CON->len = 0;

	free(sql);

	/* cache found, and no backend communication needed */
	if (status == CACHE_FOUND)
	{
		return POOL_CONTINUE;
	}

	/* cache not found */

	if (status == CACHE_ERROR)
	{
		pool_error("pool_query_cache_lookup: query cache lookup failed");
		/* reset the SystemDB connection */
		if (system_db_info->pgconn)
			pool_close_libpq_connection();
		return POOL_ERROR;		/* same as POOL_END ... at least for now */
	}

	pool_debug("pool_query_cache_lookup: query cache not found");
	return POOL_END;
}

/* --------------------------------
 * search_system_db_for_cache - search for query cache in libpq protocol level
 *
 * sends a cache searching query string using libpq protocol to the SystemDB.
 * if the SystemDB returns cache, forward the data to the frontend, and return
 * CACHE_FOUND. if cache was not found, silently discards the remaining data
 * returned by the SystemDB, and return CACHE_NOT_FOUND. returns CACHE_ERROR
 * if an error was encountered.
 * --------------------------------
 */
static CACHE_STATUS
search_system_db_for_cache(POOL_CONNECTION *frontend, char *sql, int sql_len, struct timeval *t, char tstate)
{
	fd_set readmask;
	int fds;
	int num_fds;
	struct timeval *timeout = NULL;
	char kind;
	int readlen;
	char *data = NULL;
	CACHE_STATUS return_value = CACHE_ERROR;
	int cache_found = 0;

	pool_debug("pool_query_cache_lookup: executing query: \"%s\"", sql);

	pool_write(SYSDB_CON, "Q", 1);
	if (SYSDB_MAJOR == PROTO_MAJOR_V3)
	{
		int sendlen = htonl(sql_len + 4);
		pool_write(SYSDB_CON, &sendlen, sizeof(sendlen));
	}
	if (pool_write_and_flush(SYSDB_CON, sql, sql_len) < 0)
	{
		pool_error("pool_query_cache_lookup: error while sending data to the SystemDB");
		return CACHE_ERROR;
	}

	if ((t->tv_sec + t->tv_usec) == 0)
		timeout = NULL;
	else
		timeout = t;

	/* don't really need select() or for(;;) here, but we may need it someday... or not */
	for (;;)
	{
		FD_ZERO(&readmask);
		num_fds = 0;

		num_fds = SYSDB_CON->fd + 1;
		FD_SET(SYSDB_CON->fd, &readmask);
		fds = select(num_fds, &readmask, NULL, NULL, timeout);
		if (fds == -1)
		{
			if (errno == EINTR)
				continue;

			pool_error("pool_query_cache_lookup: select() failed. reason: %s", strerror(errno));
			return CACHE_ERROR;
		}

		/* select() timeout */
		if (fds == 0)
			return CACHE_ERROR;

		for (;;)
		{
			if (! FD_ISSET(SYSDB_CON->fd, &readmask))
			{
				pool_error("pool_query_cache_lookup: select() failed");
				return CACHE_ERROR;
			}

			/* read kind */
			if (pool_read(SYSDB_CON, &kind, sizeof(kind)) < 0)
			{
				pool_error("pool_query_cache_lookup: error while reading message kind");
				return CACHE_ERROR;
			}
			pool_debug("pool_query_cache_lookup: received %c from systemdb", kind);

			/* just do the routine work of reading data in. data won't be used */
			if (kind == 'T')
			{
				if (SYSDB_MAJOR == PROTO_MAJOR_V3)
				{
					if (pool_read(SYSDB_CON, &readlen, sizeof(int)) < 0)
					{
						pool_error("pool_query_cache_lookup: error while reading message length");
						return CACHE_ERROR;
					}
					readlen = ntohl(readlen) - sizeof(int);
					data = pool_read2(SYSDB_CON, readlen);
				}
				else
				{
					data = pool_read_string(SYSDB_CON, &readlen, 0);
				}
			}
			else if (kind == 'D') /* cache found! forward it to the frontend */
			{
				char *cache;
				int status;

				cache_found = 1;

				if (SYSDB_MAJOR == PROTO_MAJOR_V3)
				{
					if (pool_read(SYSDB_CON, &readlen, sizeof(readlen)) < 0)
					{
						pool_error("pool_query_cache_lookup: error while reading message length");
						return CACHE_ERROR;
					}
					readlen = ntohl(readlen) - sizeof(int);
					cache = pool_read2(SYSDB_CON, readlen);
				}
				else
				{
					cache = pool_read_string(SYSDB_CON, &readlen, 0);
				}

				if (cache == NULL)
				{
					pool_error("pool_query_cache_lookup: error while reading message body");
					return CACHE_ERROR;
				}

				cache[readlen] = '\0';

				cache += sizeof(short);	/* number of columns in 'D' (we know it's always going to be 1, so skip) */
				cache += sizeof(int); /* length of escaped bytea cache in string format. don't need the length */

				status = ForwardCacheToFrontend(frontend, cache, tstate);
				if (status < 0)
				{
					/* fatal error has occured while forwarding cache */
					pool_error("pool_query_cache_lookup: query cache forwarding failed");
					return_value = CACHE_ERROR;
				}
			}
			else if (kind == 'C') /* see if 'D' was received */
			{
				if (cache_found)
					return_value = CACHE_FOUND;
				else
					return_value = CACHE_NOT_FOUND;

				/* must discard the remaining data */
				if (SYSDB_MAJOR == PROTO_MAJOR_V3)
				{
					if (pool_read(SYSDB_CON, &readlen, sizeof(int)) < 0)
					{
						pool_error("pool_query_cache_lookup: error while reading message length");
						return CACHE_ERROR;
					}
					readlen = ntohl(readlen) - sizeof(int);
					data = pool_read2(SYSDB_CON, readlen);
				}
				else
				{
					data = pool_read_string(SYSDB_CON, &readlen, 0);
				}
			}
			else if (kind == 'Z')
			{
				/* must discard the remaining data */
				if (SYSDB_MAJOR == PROTO_MAJOR_V3)
				{
					if (pool_read(SYSDB_CON, &readlen, sizeof(int)) < 0)
					{
						pool_error("pool_query_cache_lookup: error while reading message length");
						return CACHE_ERROR;
					}
					readlen = ntohl(readlen) - sizeof(int);
					data = pool_read2(SYSDB_CON, readlen);
				}
				else
				{
					data = pool_read_string(SYSDB_CON, &readlen, 0);
				}

				break;
			}
			else if (kind == 'E')
			{
				/* must discard the remaining data */
				if (SYSDB_MAJOR == PROTO_MAJOR_V3)
				{
					if (pool_read(SYSDB_CON, &readlen, sizeof(int)) < 0)
					{
						pool_error("pool_query_cache_lookup: error while reading message length");
						return CACHE_ERROR;
					}
					readlen = ntohl(readlen) - sizeof(int);
					data = pool_read2(SYSDB_CON, readlen);
				}
				else
				{
					data = pool_read_string(SYSDB_CON, &readlen, 0);
				}

				return_value = CACHE_ERROR;
			}
			else
			{
				/* shouldn't get here, but just in case */
				return CACHE_ERROR;
			}
		}

		break;
	}

	return return_value;
}

/* --------------------------------
 * ForwardCacheToFrontend - simply forwards cached data to the frontend
 *
 * since the cached data passed from the caller is in escaped binary string
 * format, unescape it and send it to the frontend appending 'Z' at the end.
 * returns 0 on success, -1 otherwise.
 * --------------------------------
 */
static int ForwardCacheToFrontend(POOL_CONNECTION *frontend, char *cache, char tstate)
{
	int sendlen;
	size_t sz;
	char *binary_cache = NULL;

	binary_cache = (char *)PQunescapeBytea((unsigned char *)cache, &sz);
	sendlen = (int) sz;
	if (malloc_failed(binary_cache))
		return -1;

	pool_debug("ForwardCacheToFrontend: query cache found (%d bytes)", sendlen);

	/* forward cache to the frontend */
	pool_write(frontend, binary_cache, sendlen);

	/* send ReadyForQuery to the frontend*/
	pool_write(frontend, "Z", 1);
	sendlen = htonl(5);
	pool_write(frontend, &sendlen, sizeof(int));
	if (pool_write_and_flush(frontend, &tstate, 1) < 0)
	{
		pool_error("pool_query_cache_lookup: error while writing data to the frontend");
		PQfreemem(binary_cache);
		return -1;
	}

	PQfreemem(binary_cache);
	return 0;
}

/* --------------------------------
 * pool_query_cache_register() - register query cache to the SystemDB
 *
 * returns 0 on sucess, -1 otherwise
 * --------------------------------
 */
int
pool_query_cache_register(char kind,
						  POOL_CONNECTION *frontend,
						  char *database,
						  char *data,
						  int data_len,
						  char *query)
{
	int ret;
	int send_len;

	if (! system_db_connection_exists())
		return -1;
	if (! CACHE_TABLE_INFO.has_prepared_statement)
		define_prepared_statements();

	switch (kind)
	{
		case 'T':				/* RowDescription */
		{
			/* for all SELECT result data from the backend, 'T' must come first */
			if (query_cache_info != NULL)
			{
				pool_error("pool_query_cache_register: received RowDescription in the wrong order");
				free_query_cache_info();
				return -1;
			}

			pool_debug("pool_query_cache_register: saving cache for query: \"%s\"", query);

			/* initialize query_cache_info and save the query */
			ret = init_query_cache_info(frontend, database, query);
			if (ret)
				return ret;

			/* store data into the cache */
			write_cache(&kind, 1);
			send_len = htonl(data_len + sizeof(int));
			write_cache(&send_len, sizeof(int));
			write_cache(data, data_len);

			break;
		}

		case 'D':				/* DataRow */
		{
			/* for all SELECT result data from the backend, 'T' must come first */
			if (query_cache_info == NULL)
			{
				pool_error("pool_query_cache_register: received DataRow in the wrong order");
				return -1;
			}

			write_cache(&kind, 1);
			send_len = htonl(data_len + sizeof(int));
			write_cache(&send_len, sizeof(int));
			write_cache(data, data_len);

			break;
		}

		case 'C':				/* CommandComplete */
		{
			PGresult *pg_result = NULL;
			char *escaped_query = NULL;
			size_t escaped_query_len;
			time_t now = time(NULL);
			char *values[5];
			int values_len[5];
			int values_format[5];
			int i;

			/* for all SELECT result data from the backend, 'T' must come first */
			if (query_cache_info == NULL)
			{
				pool_error("pool_query_cache_register: received CommandComplete in the wrong order");
				return -1;
			}

			/* pack CommandComplete data into the cache */
			write_cache(&kind, 1);
			send_len = htonl(data_len + sizeof(int));
			write_cache(&send_len, sizeof(int));
			write_cache(data, data_len);

			query_cache_info->create_time = pq_time_to_str(now);
			if (malloc_failed(query_cache_info->create_time))
			{
				free_query_cache_info();
				return -1;
			}

			escaped_query = (char *)malloc(strlen(query_cache_info->query) * 2 + 1);
			if (malloc_failed(escaped_query))
			{
				free_query_cache_info();
				return -1;
			}

/* 			escaped_query_len = PQescapeStringConn(system_db_info->pgconn, */
/* 												   escaped_query, */
/* 												   query_cache_info->query, */
/* 												   strlen(query_cache_info->query))); */
			escaped_query_len = PQescapeString(escaped_query, query_cache_info->query, strlen(query_cache_info->query));

			/* all the result data have been received. store into the SystemDB */
			values[0] = strdup(query_cache_info->md5_query);
			values[1] = strdup(escaped_query);
			values[2] = (char *)malloc(query_cache_info->cache_offset);
			memcpy(values[2], query_cache_info->cache, query_cache_info->cache_offset);
			values[3] = strdup(query_cache_info->db_name);
			values[4] = strdup(query_cache_info->create_time);
			for (i = 0; i < 5; i++)
			{
				if (malloc_failed(values[i]))
				{
					pool_error("pool_query_cache_register: malloc() failed");
					free_query_cache_info();
					{
						int j;
						for (j = 0; j < i; j++)
							free(values[j]);
					}
					return -1;
				}

				values_len[i] = strlen(values[i]);
			}
			values_format[0] = values_format[1] = values_format[3] = values_format[4] = 0;
			values_format[2] = 1;
			values_len[2] = query_cache_info->cache_offset;

			pg_result = PQexecPrepared(system_db_info->pgconn,
									   CACHE_TABLE_INFO.register_prepared_statement,
									   5,
									   (const char * const *)values,
									   values_len,
									   values_format,
									   0);
			if (!pg_result || PQresultStatus(pg_result) != PGRES_COMMAND_OK)
			{
				pool_error("pool_query_cache_register: PQexecPrepared() failed. reason: %s",
						   PQerrorMessage(system_db_info->pgconn));

				PQclear(pg_result);
				PQfreemem(escaped_query);
				free_query_cache_info();
				for (i = 0; i < 5; i++)
					free(values[i]);
				return -1;
			}

			PQclear(pg_result);
			PQfreemem(escaped_query);
			for (i = 0; i < 5; i++)
				free(values[i]);
			free_query_cache_info();

			break;
		}

		case 'E':
		{
			pool_debug("pool_query_cache_register: received 'E': free query cache buffer");

			pool_close_libpq_connection();
			free_query_cache_info();

			break;
		}
	}

	return 0;
}

/* --------------------------------
 * init_query_cache_info() - allocate memory for query_cache_info and stores query
 *
 * returns 0 on success, -1 otherwise
`* --------------------------------
 */
static int
init_query_cache_info(POOL_CONNECTION *pc, char *database, char *query)
{
	int query_len;				/* length of the SELECT query to be cached */

	query_cache_info = (QueryCacheInfo *)malloc(sizeof(QueryCacheInfo));
	if (malloc_failed(query_cache_info))
		return -1;

	/* query */
	query_len = strlen(query);
	query_cache_info->query = (char *)malloc(query_len + 1);
	if (malloc_failed(query_cache_info->query))
		return -1;
	memcpy(query_cache_info->query, query, query_len + 1);

	/* md5_query */
	query_cache_info->md5_query = (char *)malloc(33); /* md5sum is always 33 bytes (including the '\0') */
	if (malloc_failed(query_cache_info->md5_query))
		return -1;
	pool_md5_hash(query_cache_info->query, query_len, query_cache_info->md5_query);

	/* malloc DEFAULT_CACHE_SIZE for query_cache_info->cache */
	query_cache_info->cache = (char *)malloc(DEFAULT_CACHE_SIZE);
	if (malloc_failed(query_cache_info->cache))
		return -1;
	query_cache_info->cache_size = DEFAULT_CACHE_SIZE;
	query_cache_info->cache_offset = 0;

	/* save database name */
	query_cache_info->db_name = (char *)malloc(strlen(database)+1);
	if (malloc_failed(query_cache_info->db_name))
		return -1;
	strcpy(query_cache_info->db_name, database);

	return 0;
}

/* --------------------------------
 * free_query_cache_info() - free query_cache_info and its members
 * --------------------------------
 */
static void
free_query_cache_info(void)
{
	if (query_cache_info == NULL)
		return;

	free(query_cache_info->md5_query);
	free(query_cache_info->query);
	free(query_cache_info->cache);
	free(query_cache_info->db_name);
	free(query_cache_info->create_time);
	free(query_cache_info);
	query_cache_info = NULL;
}

/* --------------------------------
 * malloc_failed() - checks if the caller's most recent malloc() has succeeded
 *
 * returns 0 if malloc() was a success, -1 otherwise
`* --------------------------------
 */
static int
malloc_failed(void *p)
{
	if (p != NULL)
		return 0;

	pool_error("pool_query_cache: malloc() failed");
	free_query_cache_info();

	return -1;
}

/* --------------------------------
 * write_cache() - append result data to buffer
 *
 * returns 0 on success, -1 otherwise
 * --------------------------------
 */
static int
write_cache(void *buf, int len)
{
	int required_len;

	if (len < 0)
		return -1;

	required_len = query_cache_info->cache_offset + len;
	if (required_len > query_cache_info->cache_size)
	{
		char *ptr;

		required_len = query_cache_info->cache_size * 2;
		ptr = (char *)realloc(query_cache_info->cache, required_len);
		if (malloc_failed(ptr))
			return -1;

		query_cache_info->cache = ptr;
		query_cache_info->cache_size = required_len;

		pool_debug("pool_query_cache: extended cache buffer size to %d", query_cache_info->cache_size);
	}

	memcpy(query_cache_info->cache + query_cache_info->cache_offset, buf, len);
	query_cache_info->cache_offset += len;

	return 0;
}

/* --------------------------------
 * pq_time_to_str() - convert time_t to ISO standard time output string
 *
 * returns a pointer to newly allocated string, NULL if malloc fails
 * --------------------------------
 */
static char *
pq_time_to_str(time_t t)
{
	char *time_p;
	char iso_time[32];
	struct tm *tm;

	tm = localtime(&t);
	strftime(iso_time, sizeof(iso_time), "%Y-%m-%d %H:%M:%S%z", tm);

	time_p = strdup(iso_time);

	return time_p;
}


/* --------------------------------
 * system_db_connection_exists() - checks and if a connection to the SystemDB exists
 *
 * if not connected, it makes an attempt to connect to the SystemDB. If a connection
 * exists, returns 1, returns 0 otherwise.
 * --------------------------------
 */
static int
system_db_connection_exists(void)
{
	if (!system_db_info->pgconn ||
		(PQstatus(system_db_info->pgconn) != CONNECTION_OK))
	{
		if (system_db_connect())
			return 0;
	}

	return 1;
}


/* --------------------------------
 * define_prepared_statements() - defines prepared statements for the current session
 * --------------------------------
 */
static void
define_prepared_statements(void)
{
	PGresult *pg_result;
	char *sql = NULL;
	int sql_len;

	sql_len =
		strlen(pool_config->system_db_schema) +
		strlen(QUERY_CACHE_TABLE_NAME) +
		1024;

	sql = (char *)malloc(sql_len);
	if (malloc_failed(sql))
	{
		pool_error("pool_query_cache: malloc() failed");
		return;
	}

	free(CACHE_TABLE_INFO.register_prepared_statement);
	CACHE_TABLE_INFO.register_prepared_statement
		= strdup(CACHE_REGISTER_PREPARED_STMT);
	if (malloc_failed(CACHE_TABLE_INFO.register_prepared_statement))
	{
		pool_error("pool_query_cache: malloc() failed");
		free(sql);
		return;
	}

#ifdef HAVE_PQPREPARE
	snprintf(sql, sql_len,
			 "INSERT INTO %s.%s VALUES ( $1, $2, $3, $4, $5 )",
			 pool_config->system_db_schema,
			 QUERY_CACHE_TABLE_NAME);
	pg_result = PQprepare(system_db_info->pgconn,
						  CACHE_TABLE_INFO.register_prepared_statement,
						  sql,
						  5,
						  NULL);
#else
	snprintf(sql, sql_len,
			 "PREPARE %s (TEXT, TEXT, BYTEA, TEXT, TIMESTAMP WITH TIME ZONE) AS INSERT INTO %s.%s VALUES ( $1, $2, $3, $4, $5 )",
			 CACHE_TABLE_INFO.register_prepared_statement,
			 pool_config->system_db_schema,
			 QUERY_CACHE_TABLE_NAME);
	pg_result = PQexec(system_db_info->pgconn, sql);
#endif
	if (!pg_result || PQresultStatus(pg_result) != PGRES_COMMAND_OK)
	{
		pool_error("pool_query_cache: PQprepare() failed: %s", PQerrorMessage(system_db_info->pgconn));
		free(CACHE_TABLE_INFO.register_prepared_statement);
		free(sql);
		return;
	}

	pool_debug("pool_query_cache: prepared statements created");
	CACHE_TABLE_INFO.has_prepared_statement = 1;

	free(sql);
	PQclear(pg_result);
}
