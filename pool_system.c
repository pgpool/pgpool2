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
 * pool_system.c: systemdb
 *
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "pool.h"
#include "pool_config.h"

static int create_prepared_statement(DistDefInfo *dist_info);
static int  get_col_list(DistDefInfo *info);
static int  get_col_list2(RepliDefInfo *info);

static
int  get_col_list(DistDefInfo *info)
{
	int i;
	static char sql[1024];
	PGresult *result;

	if (!system_db_info->pgconn ||
		(PQstatus(system_db_info->pgconn) != CONNECTION_OK))
	{
		if (system_db_connect())
			return -1;
	}

	for (i = 0; i < info->col_num; i++)
	{
		snprintf(sql,
				 sizeof(sql),
				 "SELECT col_list[%d],type_list[%d] FROM %s.dist_def where dbname = '%s' and schema_name = '%s' and table_name = '%s'",
				 i + 1,
				 i + 1,
				 pool_config->system_db_schema,
				 info->dbname,info->schema_name,
				 info->table_name);

		result = PQexec(system_db_info->pgconn, sql);

		if (!result || PQresultStatus(result) != PGRES_TUPLES_OK)
		{
			pool_error("get_col_list :PQexec failed: %s",
					   PQerrorMessage(system_db_info->pgconn));
			return -1;
		}
		else
		{
			info->col_list[i]  = malloc(strlen(PQgetvalue(result,0,0)) + 1);
			info->type_list[i] = malloc(strlen(PQgetvalue(result,0,1)) + 1);

			if (info->col_list[i] == NULL || info->type_list[i] == NULL)
			{
				pool_error("get_col_list: malloc failed: %s", strerror(errno));
				PQclear(result);
				return -1;
			}
			strcpy(info->col_list[i],PQgetvalue(result,0,0));
			strcpy(info->type_list[i],PQgetvalue(result,0,1));
			if (strcmp(info->col_list[i], info->dist_key_col_name) == 0)
				info->dist_key_col_id = i;
			PQclear(result);
		}
	}
	return 0;
}

static
int  get_col_list2(RepliDefInfo *info)
{
	int i;
	static char sql[1024];
	PGresult *result;

	if (!system_db_info->pgconn ||
		(PQstatus(system_db_info->pgconn) != CONNECTION_OK))
	{
		if (system_db_connect())
			return -1;
	}

	for (i = 0; i < info->col_num; i++)
	{
		snprintf(sql,
				 sizeof(sql),
				 "SELECT col_list[%d],type_list[%d] FROM %s.replicate_def where dbname = '%s' and schema_name = '%s' and table_name = '%s'",
				 i + 1,
				 i + 1,
				 pool_config->system_db_schema,
				 info->dbname,info->schema_name,
				 info->table_name);

		result = PQexec(system_db_info->pgconn, sql);

		if (!result || PQresultStatus(result) != PGRES_TUPLES_OK)
		{
			pool_error("get_col_list2: PQexec failed: %s",
					   PQerrorMessage(system_db_info->pgconn));
			return -1;
		}
		else
		{
			info->col_list[i]  = malloc(strlen(PQgetvalue(result,0,0)) + 1);
			info->type_list[i] = malloc(strlen(PQgetvalue(result,0,1)) + 1);

			if (info->col_list[i] == NULL || info->type_list[i] == NULL)
			{
				pool_error("get_col_list2: malloc failed: %s", strerror(errno));
				PQclear(result);
				return -1;
			}
			strcpy(info->col_list[i],PQgetvalue(result,0,0));
			strcpy(info->type_list[i],PQgetvalue(result,0,1));
			PQclear(result);
		}
	}
	return 0;
}

/*
 * system_db_connect:
 *     Connects System DB by PQconnectdb().
 */
int system_db_connect (void)
{
	static char conninfo[1024];
	int i;

	snprintf(conninfo,
			 sizeof(conninfo),
			 "host='%s' port=%d dbname='%s' user='%s' password='%s'",
			 system_db_info->info->hostname,
			 system_db_info->info->port,
			 system_db_info->info->database_name,
			 system_db_info->info->user,
			 system_db_info->info->password);

	system_db_info->pgconn = PQconnectdb(conninfo);

	if (PQstatus(system_db_info->pgconn) != CONNECTION_OK)
	{
		pool_error("Connection to database failed: %s",
				   PQerrorMessage(system_db_info->pgconn));
		PQfinish(system_db_info->pgconn);
		system_db_info->pgconn = NULL;
		return 1;
	}

	for (i = 0; i < system_db_info->info->dist_def_num; i++)
	{
		DistDefInfo *info = &system_db_info->info->dist_def_slot[i];
		info->is_created_prepare = 0;
	}

	system_db_info->info->query_cache_table_info.has_prepared_statement = 0;

	return 0;
}

/*
 * pool_memset_system_db_info:
 *    Initializes distribution rules. Distribution rules are stored in
 *    System DB. So we have to execute query, and expand results on
 *    memory.
 */
int pool_memset_system_db_info (SystemDBInfo *info)
{
	int i;
	static char sql[1024],sql2[1024];
	PGresult *result;
	DistDefInfo *dist_info = NULL;
	RepliDefInfo *repli_info = NULL;

	if (!system_db_info->pgconn ||
		(PQstatus(system_db_info->pgconn) != CONNECTION_OK))
	{
		if (system_db_connect())
			return -1;
	}

  /* get distribution rules */
	snprintf(sql,
			 sizeof(sql),
			 "SELECT dbname, schema_name, table_name,col_name,array_upper(col_list,1),col_list,type_list, dist_def_func FROM %s.dist_def",
			 pool_config->system_db_schema);

	result = PQexec(system_db_info->pgconn, sql);
	if (!result || PQresultStatus(result) != PGRES_TUPLES_OK)
	{
		pool_error("PQexec failed: %s", PQerrorMessage(system_db_info->pgconn));
		return -1;
	}
	else
 	{
		info->dist_def_num = PQntuples(result);
		if (PQntuples(result) > 0)
		{
			dist_info = malloc(sizeof(DistDefInfo) * info->dist_def_num);
		}

		if (dist_info == NULL && info->dist_def_num != 0)
		{
			pool_error("pool_memset_system_db_info: malloc failed: %s",
					   strerror(errno));
			PQclear(result);
			pool_close_libpq_connection();
			return -1;
		}

		info->dist_def_slot = dist_info;

		for (i = 0; i < PQntuples(result); ++i)
		{
			char *t_dbname;
			char *t_schema_name;
			char *t_table_name;
			char *t_dist_key_col_name;
			char *t_dist_def_func;
			int num;
			int len;

			num = atol(PQgetvalue(result, i ,4));
			t_dbname = malloc(strlen(PQgetvalue(result,i,0)) + 1);
			if (t_dbname == NULL)
			{
				pool_error("pool_memset_system_db_info: malloc failed: %s",
						   strerror(errno));
				PQclear(result);
				pool_close_libpq_connection();
				return -1;
			}
			strcpy(t_dbname, PQgetvalue(result,i,0));
			dist_info[i].dbname = t_dbname;

			t_schema_name = malloc(strlen(PQgetvalue(result,i,1)) + 1);
			if (t_schema_name == NULL)
			{
				pool_error("pool_memset_system_db_info: malloc failed: %s",
						   strerror(errno));
				PQclear(result);
				pool_close_libpq_connection();
				return -1;
			}
			strcpy(t_schema_name, PQgetvalue(result,i,1));
			dist_info[i].schema_name = t_schema_name;

			t_table_name = malloc(strlen(PQgetvalue(result,i,2)) + 1);
			if (t_table_name == NULL)
			{
				pool_error("pool_memset_system_db_info: malloc failed: %s",
						   strerror(errno));
				PQclear(result);
				pool_close_libpq_connection();
				return -1;
			}
			strcpy(t_table_name, PQgetvalue(result,i,2));
			dist_info[i].table_name = t_table_name;

			t_dist_key_col_name = malloc(strlen(PQgetvalue(result,i,3)) + 1);
			if (t_dist_key_col_name == NULL)
			{
				pool_error("pool_memset_system_db_info: malloc failed: %s",
						   strerror(errno));
				PQclear(result);
				pool_close_libpq_connection();
				return -1;
			}
			strcpy(t_dist_key_col_name, PQgetvalue(result,i,3));
			dist_info[i].dist_key_col_name = t_dist_key_col_name;

			t_dist_def_func = malloc(strlen(PQgetvalue(result,i,7)) + 1);
			if (t_dist_def_func == NULL)
			{
				pool_error("pool_memset_system_db_info: malloc failed: %s",
						   strerror(errno));
				PQclear(result);
				pool_close_libpq_connection();
				return -1;
			}
			strcpy(t_dist_def_func, PQgetvalue(result,i,7));
			dist_info[i].dist_def_func = t_dist_def_func;

			dist_info[i].col_num = num;

			dist_info[i].col_list = calloc(num, sizeof(char *));
			dist_info[i].type_list = calloc(num, sizeof(char *));
			if (dist_info[i].col_list == NULL || dist_info[i].type_list == NULL)
			{
				pool_error("pool_memset_system_db_info: calloc failed: %s",
						   strerror(errno));
				PQclear(result);
				pool_close_libpq_connection();
				return -1;
			}

			if (get_col_list(&dist_info[i]) < 0)
			{
				pool_error("get_col_list() failed");
				PQclear(result);
				pool_close_libpq_connection();
				return -1;
			}

			/* create PREPARE statement */
			len = strlen(t_dbname) + strlen(t_schema_name) +
				strlen(t_table_name) + strlen("pgpool_");

			dist_info[i].prepare_name = malloc(len + 1);
			if (dist_info[i].prepare_name == NULL)
			{
				pool_error("pool_memset_system_db_info: malloc failed: %s",
						   strerror(errno));
				return -1;
			}

			snprintf(dist_info[i].prepare_name, len+1, "pgpool_%s%s%s",
					 t_dbname, t_schema_name, t_table_name);
			dist_info[i].prepare_name[len] = '\0';
		}
	}

	PQclear(result);

  /* get replication rules */
	snprintf(sql2,
			 sizeof(sql2),
			 "SELECT dbname, schema_name, table_name, array_upper(col_list,1),col_list,type_list FROM %s.replicate_def",
			 pool_config->system_db_schema);

	result = PQexec(system_db_info->pgconn, sql2);

	if (!result)
	{
		pool_error("PQexec failed: %s", PQerrorMessage(system_db_info->pgconn));
		return -1;
	}
	else if (PQresultStatus(result) != PGRES_TUPLES_OK)
	{
		info->repli_def_num = 0;
		info->repli_def_slot = NULL;
	}
	else
 	{
		info->repli_def_num = PQntuples(result);
		if (PQntuples(result) > 0)
		{
			repli_info = malloc(sizeof(RepliDefInfo) * info->repli_def_num);
		}

		if (repli_info == NULL && info->repli_def_num != 0)
		{
			pool_error("pool_memset_system_db_info: malloc failed: %s",
					   strerror(errno));
			PQclear(result);
			pool_close_libpq_connection();
			return -1;
		}

		info->repli_def_slot = repli_info;

		for (i = 0; i < PQntuples(result); ++i)
		{
			char *t_dbname;
			char *t_schema_name;
			char *t_table_name;
			int num;
			int len;

			num = atol(PQgetvalue(result, i ,3));
			t_dbname = malloc(strlen(PQgetvalue(result,i,0)) + 1);
			if (t_dbname == NULL)
			{
				pool_error("pool_memset_system_db_info: malloc failed: %s",
						   strerror(errno));
				PQclear(result);
				pool_close_libpq_connection();
				return -1;
			}
			strcpy(t_dbname, PQgetvalue(result,i,0));
			repli_info[i].dbname = t_dbname;

			t_schema_name = malloc(strlen(PQgetvalue(result,i,1)) + 1);
			if (t_schema_name == NULL)
			{
				pool_error("pool_memset_system_db_info: malloc failed: %s",
						   strerror(errno));
				PQclear(result);
				pool_close_libpq_connection();
				return -1;
			}
			strcpy(t_schema_name, PQgetvalue(result,i,1));
			repli_info[i].schema_name = t_schema_name;

			t_table_name = malloc(strlen(PQgetvalue(result,i,2)) + 1);
			if (t_table_name == NULL)
			{
				pool_error("pool_memset_system_db_info: malloc failed: %s",
						   strerror(errno));
				PQclear(result);
				pool_close_libpq_connection();
				return -1;
			}
			strcpy(t_table_name, PQgetvalue(result,i,2));
			repli_info[i].table_name = t_table_name;

			repli_info[i].col_num = num;

			repli_info[i].col_list = calloc(num, sizeof(char *));
			repli_info[i].type_list = calloc(num, sizeof(char *));
			if (repli_info[i].col_list == NULL || repli_info[i].type_list == NULL)
			{
				pool_error("pool_memset_system_db_info: calloc failed: %s",
						   strerror(errno));
				PQclear(result);
				pool_close_libpq_connection();
				return -1;
			}

			if (get_col_list2(&repli_info[i]) < 0)
			{
				pool_error("get_col_list() failed");
				PQclear(result);
				pool_close_libpq_connection();
				return -1;
			}

			/* create PREPARE statement */
			len = strlen(t_dbname) + strlen(t_schema_name) +
				strlen(t_table_name) + strlen("pgpool_");

			repli_info[i].prepare_name = malloc(len + 1);
			if (repli_info[i].prepare_name == NULL)
			{
				pool_error("pool_memset_system_db_info: malloc failed: %s",
						   strerror(errno));
				return -1;
			}

			snprintf(repli_info[i].prepare_name, len+1, "pgpool_%s%s%s",
					 t_dbname, t_schema_name, t_table_name);
			repli_info[i].prepare_name[len] = '\0';
  	}
	}

	PQclear(result);

	pool_close_libpq_connection();
	return i;
}

/*
 * pool_get_dist_def_info:
 *    Looks up distribution rule with dbname, schema_name and table_name.
 */
DistDefInfo *pool_get_dist_def_info (char *dbname, char *schema_name, char *table_name)
{
	int i;
	int dist_def_num = system_db_info->info->dist_def_num;
	char *public ="public";

	if (!dbname || !table_name)
	{
		return NULL;
	}

	if (!schema_name)
	{
		schema_name = public;
	}

	for (i = 0; i < dist_def_num; i++)
	{
		char *mem_dbname;
		char *mem_schema_name;
		char *mem_table_name;

		mem_dbname = system_db_info->info->dist_def_slot[i].dbname;
		mem_schema_name = system_db_info->info->dist_def_slot[i].schema_name;
		mem_table_name  = system_db_info->info->dist_def_slot[i].table_name;

		if ((strcmp(mem_dbname, dbname) == 0) &&
			(strcmp(mem_schema_name, schema_name) == 0) &&
			(strcmp(mem_table_name, table_name) ==0))
		{
			return &system_db_info->info->dist_def_slot[i];
		}
	}
	return NULL;
}

/*
 * pool_get_repli_def_info:
 *    Looks up replication rule with dbname, schema_name and table_name.
 */
RepliDefInfo *pool_get_repli_def_info (char *dbname, char *schema_name, char *table_name)
{
	int i;
	int repli_def_num = system_db_info->info->repli_def_num;
	char *public ="public";

	if (!dbname || !table_name)
	{
		return NULL;
	}

	if (!schema_name)
	{
		schema_name = public;
	}

	for (i = 0; i < repli_def_num; i++)
	{
		char *mem_dbname;
		char *mem_schema_name;
		char *mem_table_name;

		mem_dbname = system_db_info->info->repli_def_slot[i].dbname;
		mem_schema_name = system_db_info->info->repli_def_slot[i].schema_name;
		mem_table_name  = system_db_info->info->repli_def_slot[i].table_name;

		if ((strcmp(mem_dbname, dbname) == 0) &&
			(strcmp(mem_schema_name, schema_name) == 0) &&
			(strcmp(mem_table_name, table_name) ==0))
		{
			return &system_db_info->info->repli_def_slot[i];
		}
	}
	return NULL;
}

/*
 * pool_get_id:
 *    Returns the backend node id from value.
 */
int pool_get_id (DistDefInfo *info, const char *value)
{
	int num;
	PGresult *result;
	char *type;
	int length;

	if (!system_db_info->pgconn ||
		(PQstatus(system_db_info->pgconn) != CONNECTION_OK))
	{
		if (system_db_connect())
			return -1;
	}

	if (info->is_created_prepare == 0)
	{
		if (create_prepared_statement(info) != 0)
			return -1;
	}

	type=info->type_list[info->dist_key_col_id];
	length = strlen(value);
	result = PQexecPrepared(system_db_info->pgconn, info->prepare_name,
							1, &value, &length, NULL, 0);

	if (!result || PQresultStatus(result) != PGRES_TUPLES_OK ||
		PQgetisnull(result, 0, 0))
	{
		pool_error("PQexecPrepared failed: %s", PQerrorMessage(system_db_info->pgconn));
		return -1;
	}
	else
	{
		char *id;
		id = PQgetvalue(result, 0 ,0);

		if(strlen(id))
		{
			num = atoi(id);
			PQclear(result);

			if(num < NUM_BACKENDS)
			{
				return num;
			} else {
				return -1;
			}
		}
		return -1;
	}
}

/*
 * pool_close_libpq_connection:
 *     Closes libpq's connection.
 */
void pool_close_libpq_connection(void)
{
	PQfinish(system_db_info->pgconn);
	system_db_info->pgconn = NULL;
}

/*
 * pool_system_db_connection:
 *     Returns persistent connection to the system DB
 */
POOL_CONNECTION_POOL_SLOT *pool_system_db_connection(void)
{
	return system_db_info->connection;
}

/*
 * create_prepared_statement:
 *     Returns 0 if prepared statement is created.
 *     Returns 1 if prepared statement can't created.
 */
static int create_prepared_statement(DistDefInfo *dist_info)
{
	static char sql[1024];
	PGresult *result;

#ifdef HAVE_PQPREPARE
	snprintf(sql, 1024, "SELECT %s($1::%s)", dist_info->dist_def_func,
			 dist_info->type_list[dist_info->dist_key_col_id]);
	result = PQprepare(system_db_info->pgconn,
					   dist_info->prepare_name,
					   sql, 1, NULL);
#else
	snprintf(sql, 1024, "PREPARE %s (%s) AS SELECT %s($1::%s)",
			 dist_info->prepare_name,
			 dist_info->type_list[dist_info->dist_key_col_id],
			 dist_info->dist_def_func,
			 dist_info->type_list[dist_info->dist_key_col_id]);
	result = PQexec(system_db_info->pgconn,	sql);
#endif /* HAVE_PQPREPARE */

	if (!result || PQresultStatus(result) != PGRES_COMMAND_OK)
	{
		pool_error("PQprepare failed: %s", PQerrorMessage(system_db_info->pgconn));
		return 1;
	}
	dist_info->is_created_prepare = 1;
	return 0;
}
