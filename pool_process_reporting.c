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
 * Process "show pool_status" query.
 */
#include "pool.h"
#include "pool_proto_modules.h"
#include "pool_stream.h"
#include "pool_config.h"

#include <string.h>
#include <netinet/in.h>

void process_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	static char *cursorname = "blank";
	static short num_fields = 3;
	static char *field_names[] = {"item", "value", "description"};
	static int oid = 0;
	static short fsize = -1;
	static int mod = 0;
	short n;
	int i, j;
	short s;
	int len;
	short colnum;

	static unsigned char nullmap[2] = {0xff, 0xff};
	int nbytes = (num_fields + 7)/8;

#define POOLCONFIG_MAXNAMELEN 32
#define POOLCONFIG_MAXVALLEN 512
#define POOLCONFIG_MAXDESCLEN 64

	typedef struct {
		char name[POOLCONFIG_MAXNAMELEN+1];
		char value[POOLCONFIG_MAXVALLEN+1];
		char desc[POOLCONFIG_MAXDESCLEN+1];
	} POOL_REPORT_STATUS;

/*
 * Report data buffer.
 * 128 is the max number of configuration items.
 * In addition, we need MAX_NUM_BACKENDS*4
 * for backend descriptions.
 */
#define MAXITEMS (128 + MAX_NUM_BACKENDS*4)		

	static POOL_REPORT_STATUS status[MAXITEMS];

	short nrows;
	int size;
	int hsize;

	i = 0;

	strncpy(status[i].name, "listen_addresses", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->listen_addresses);
	strncpy(status[i].desc, "host name(s) or IP address(es) to listen to", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "port", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->port);
	strncpy(status[i].desc, "pgpool accepting port number", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "socket_dir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->socket_dir);
	strncpy(status[i].desc, "pgpool socket directory", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "num_init_children", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->num_init_children);
	strncpy(status[i].desc, "# of children initially pre-forked", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "child_life_time", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->child_life_time);
	strncpy(status[i].desc, "if idle for this seconds, child exits", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "connection_life_time", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->connection_life_time);
	strncpy(status[i].desc, "if idle for this seconds, connection closes", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "client_idle_limit", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->client_idle_limit);
	strncpy(status[i].desc, "if idle for this seconds, child connection closes", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "child_max_connections", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->child_max_connections);
	strncpy(status[i].desc, "if max_connections received, chile exits", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "max_pool", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->max_pool);
	strncpy(status[i].desc, "max # of connection pool per child", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "authentication_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->authentication_timeout);
	strncpy(status[i].desc, "maximum time in seconds to complete client authentication", POOLCONFIG_MAXNAMELEN);
	i++;

	strncpy(status[i].name, "logdir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->logdir);
	strncpy(status[i].desc, "logging directory", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "pid_file_name", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->pid_file_name);
	strncpy(status[i].desc, "path to pid file", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "backend_socket_dir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->backend_socket_dir);
	strncpy(status[i].desc, "Unix domain socket directory for the PostgreSQL server", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "replication_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->replication_mode);
	strncpy(status[i].desc, "non 0 if operating in replication mode", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "load_balance_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->load_balance_mode);
	strncpy(status[i].desc, "non 0 if operating in load balancing mode", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "replication_stop_on_mismatch", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->replication_stop_on_mismatch);
	strncpy(status[i].desc, "stop replication mode on fatal error", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "replicate_select", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->replicate_select);
	strncpy(status[i].desc, "non 0 if SELECT statement is replicated", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "reset_query_list", POOLCONFIG_MAXNAMELEN);
	*(status[i].value) = '\0';
	for (j=0;j<pool_config->num_reset_queries;j++)
	{
		int len;
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		strncat(status[i].value, pool_config->reset_query_list[j], len);
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		strncat(status[i].value, ";", len);
	}
	strncpy(status[i].desc, "queries issued at the end of session", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "print_timestamp", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->print_timestamp);
	strncpy(status[i].desc, "if true print time stamp to each log line", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "master_slave_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->master_slave_mode);
	strncpy(status[i].desc, "if true, operate in master/slave mode", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "master_slave_sub_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->master_slave_sub_mode);
	strncpy(status[i].desc, "master/slave sub mode", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "connection_cache", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->connection_cache);
	strncpy(status[i].desc, "if true, cache connection pool", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "health_check_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->health_check_timeout);
	strncpy(status[i].desc, "health check timeout", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "health_check_period", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->health_check_period);
	strncpy(status[i].desc, "health check period", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "health_check_user", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->health_check_user);
	strncpy(status[i].desc, "health check user", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "failover_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->failover_command);
	strncpy(status[i].desc, "failover command", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "failback_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->failover_command);
	strncpy(status[i].desc, "failback command", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "fail_over_on_backend_error", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->insert_lock);
	strncpy(status[i].desc, "fail_over_on_backend_error", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "insert_lock", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->insert_lock);
	strncpy(status[i].desc, "insert lock", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "ignore_leading_white_space", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->ignore_leading_white_space);
	strncpy(status[i].desc, "ignore leading white spaces", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "replication_enabled", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->replication_enabled);
	strncpy(status[i].desc, "non 0 if actually operating in replication mode", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "master_slave_enabled", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->master_slave_enabled);
	strncpy(status[i].desc, "non 0 if actually operating in master/slave", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "num_reset_queries", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->num_reset_queries);
	strncpy(status[i].desc, "number of queries in reset_query_list", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "pcp_port", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->pcp_port);
	strncpy(status[i].desc, "PCP port # to bind", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "pcp_socket_dir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->pcp_socket_dir);
	strncpy(status[i].desc, "PCP socket directory", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "pcp_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->pcp_timeout);
	strncpy(status[i].desc, "PCP timeout for an idle client", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "log_statement", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->log_statement);
	strncpy(status[i].desc, "if non 0, logs all SQL statements", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "log_per_node_statement", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->log_per_node_statement);
	strncpy(status[i].desc, "if non 0, logs all SQL statements on each node", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "log_connections", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->log_connections);
	strncpy(status[i].desc, "if true, print incoming connections to the log", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "log_hostname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->log_hostname);
	strncpy(status[i].desc, "if true, resolve hostname for ps and log print", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "enable_pool_hba", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->enable_pool_hba);
	strncpy(status[i].desc, "if true, use pool_hba.conf for client authentication", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "recovery_user", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->recovery_user);
	strncpy(status[i].desc, "online recovery user", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "recovery_password", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->recovery_password);
	strncpy(status[i].desc, "online recovery password", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "recovery_1st_stage_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->recovery_1st_stage_command);
	strncpy(status[i].desc, "execute a command in first stage.", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "recovery_2nd_stage_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->recovery_2nd_stage_command);
	strncpy(status[i].desc, "execute a command in second stage.", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "recovery_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->recovery_timeout);
	strncpy(status[i].desc, "max time in seconds to wait for the recovering node's postmaster", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "client_idle_limit_in_recovery", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->client_idle_limit_in_recovery);
	strncpy(status[i].desc, "if idle for this seconds, child connection closes in recovery 2nd statge", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "lobj_lock_table", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->lobj_lock_table);
	strncpy(status[i].desc, "table name used for large object replication control", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "ssl", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->ssl);
	strncpy(status[i].desc, "SSL support", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "ssl_key", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->ssl_key);
	strncpy(status[i].desc, "path to the SSL private key file", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "ssl_cert", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->ssl_cert);
	strncpy(status[i].desc, "path to the SSL public certificate file", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "debug_level", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->debug_level);
	strncpy(status[i].desc, "debug message level", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "parallel_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->parallel_mode);
	strncpy(status[i].desc, "if non 0, run in parallel query mode", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "enable_query_cache", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->enable_query_cache);
	strncpy(status[i].desc, "if non 0, use query cache", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "pgpool2_hostname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->pgpool2_hostname);
	strncpy(status[i].desc, "pgpool2 hostname", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "system_db_hostname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_hostname);
	strncpy(status[i].desc, "system DB hostname", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "system_db_port", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->system_db_port);
	strncpy(status[i].desc, "system DB port number", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "system_db_dbname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_dbname);
	strncpy(status[i].desc, "system DB name", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "system_db_schema", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_schema);
	strncpy(status[i].desc, "system DB schema name", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "system_db_user", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_user);
	strncpy(status[i].desc, "user name to access system DB", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "system_db_password", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_password);
	strncpy(status[i].desc, "password to access system DB", POOLCONFIG_MAXDESCLEN);
	i++;

	for (j = 0; j < NUM_BACKENDS; j++)
	{
		if (BACKEND_INFO(j).backend_port == 0)
			continue;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "backend_hostname%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", BACKEND_INFO(j).backend_hostname);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "backend #%d hostname", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "backend_port%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", BACKEND_INFO(j).backend_port);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "backend #%d port number", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "backend_weight%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%f", BACKEND_INFO(j).backend_weight);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "weight of backend #%d", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "backend status%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", BACKEND_INFO(j).backend_status);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "status of backend #%d", j);
		i++;
	}

	nrows = i;

	if (MAJOR(backend) == PROTO_MAJOR_V2)
	{
		/* cursor response */
		pool_write(frontend, "P", 1);
		pool_write(frontend, cursorname, strlen(cursorname)+1);
	}

	/* row description */
	pool_write(frontend, "T", 1);

	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		len = sizeof(num_fields) + sizeof(len);

		for (i=0;i<num_fields;i++)
		{
			char *f = field_names[i];
			len += strlen(f)+1;
			len += sizeof(oid);
			len += sizeof(colnum);
			len += sizeof(oid);
			len += sizeof(s);
			len += sizeof(mod);
			len += sizeof(s);
		}

		len = htonl(len);
		pool_write(frontend, &len, sizeof(len));
	}

	n = htons(num_fields);
	pool_write(frontend, &n, sizeof(short));

	for (i=0;i<num_fields;i++)
	{
		char *f = field_names[i];

		pool_write(frontend, f, strlen(f)+1);		/* field name */

		if (MAJOR(backend) == PROTO_MAJOR_V3)
		{
			pool_write(frontend, &oid, sizeof(oid));	/* table oid */
			colnum = htons(i);
			pool_write(frontend, &colnum, sizeof(colnum));	/* column number */
		}

		pool_write(frontend, &oid, sizeof(oid));		/* data type oid */
		s = htons(fsize);
		pool_write(frontend, &s, sizeof(fsize));		/* field size */
		pool_write(frontend, &mod, sizeof(mod));		/* modifier */

		if (MAJOR(backend) == PROTO_MAJOR_V3)
		{
			s = htons(0);
			pool_write(frontend, &s, sizeof(fsize));	/* field format (text) */
		}
	}
	pool_flush(frontend);

	if (MAJOR(backend) == PROTO_MAJOR_V2)
	{
		/* ascii row */
		for (i=0;i<nrows;i++)
		{
			pool_write(frontend, "D", 1);
			pool_write_and_flush(frontend, nullmap, nbytes);

			size = strlen(status[i].name);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, status[i].name, size);

			size = strlen(status[i].value);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, status[i].value, size);

			size = strlen(status[i].desc);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, status[i].desc, size);
		}
	}
	else
	{
		/* data row */
		for (i=0;i<nrows;i++)
		{
			pool_write(frontend, "D", 1);
			len = sizeof(len) + sizeof(nrows);
			len += sizeof(int) + strlen(status[i].name);
			len += sizeof(int) + strlen(status[i].value);
			len += sizeof(int) + strlen(status[i].desc);
			len = htonl(len);
			pool_write(frontend, &len, sizeof(len));
			s = htons(3);
			pool_write(frontend, &s, sizeof(s));

			len = htonl(strlen(status[i].name));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, status[i].name, strlen(status[i].name));

			len = htonl(strlen(status[i].value));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, status[i].value, strlen(status[i].value));

			len = htonl(strlen(status[i].desc));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, status[i].desc, strlen(status[i].desc));
		}
	}

	/* complete command response */
	pool_write(frontend, "C", 1);
	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		len = htonl(sizeof(len) + strlen("SELECT")+1);
		pool_write(frontend, &len, sizeof(len));
	}
	pool_write(frontend, "SELECT", strlen("SELECT")+1);

	/* ready for query */
	pool_write(frontend, "Z", 1);
	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		len = htonl(sizeof(len) + 1);
		pool_write(frontend, &len, sizeof(len));
		pool_write(frontend, "I", 1);
	}

	pool_flush(frontend);
}
