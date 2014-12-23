/* -*-pgsql-c-*- */
/*
 * $Header$
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
 * Process pgPool-II "SHOW" queries.
 */
#include "pool.h"
#include "protocol/pool_proto_modules.h"
#include "utils/elog.h"
#include "utils/pool_stream.h"
#include "pool_config.h"
#include "query_cache/pool_memqcache.h"
#include "version.h"

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

void send_row_description(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend,
						  short num_fields, char **field_names)
{
	static char *cursorname = "blank";
	static int oid = 0;
	static short fsize = -1;
	static int mod = 0;
	short n;
	int i;
	short s;
	int len;
	short colnum;

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
		/* information about computed byte in section "RowDescription (B)" here:
		 * http://www.postgresql.org/docs/current/static/protocol-message-formats.html */

		len = 6; /* int32 + int16 */

		for (i=0;i<num_fields;i++)
		{
			/* String + '\0' + 3* int32 + 3* int16 */
			len += strlen(field_names[i]) +1 +18;
		}

		len = htonl(len);
		pool_write(frontend, &len, sizeof(len));
	}

	n = htons(num_fields);
	pool_write(frontend, &n, sizeof(short));

	for (i=0;i<num_fields;i++)
	{
		pool_write(frontend, field_names[i], strlen(field_names[i])+1);		/* field name */

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
}

void send_complete_and_ready(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, const int num_rows)
{
	int len;
	int msg_len;
	char msg[16];

	msg_len = snprintf(msg, 16, "SELECT %d", num_rows);

	/* if we had more than 16 bytes, including '\0', the string was truncatured
	 * shouldn't happen though, as it would means more than "SELECT 99999999" */
	if (msg_len > 15) msg_len = 15;

	/* complete command response */
	pool_write(frontend, "C", 1);
	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		len = htonl(4 + strlen(msg)+1);
		pool_write(frontend, &len, sizeof(len));
	}
	pool_write(frontend, msg, strlen(msg)+1);

	/* ready for query */
	pool_write(frontend, "Z", 1);
	if (MAJOR(backend) == PROTO_MAJOR_V3)
	{
		len = htonl(5);
		pool_write(frontend, &len, sizeof(len));
		pool_write(frontend, "I", 1);
	}

	pool_flush(frontend);
}

POOL_REPORT_CONFIG* get_config(int *nrows)
{
	int i, j;
	int len;

/*
 * Report data buffer.
 * 128 is the max number of configuration items.
 * In addition, we need MAX_NUM_BACKENDS*4
 * for backend descriptions.
 */
#define MAXITEMS (256 + MAX_NUM_BACKENDS*4)		

	POOL_REPORT_CONFIG* status = palloc0(MAXITEMS * sizeof(POOL_REPORT_CONFIG));

	/* we initialize the array with NULL values so when looping
	 * on it, we can use it as stop condition */
//	memset(status, 0, sizeof(POOL_REPORT_CONFIG) * MAXITEMS);

	i = 0;

	/* CONNECTIONS */

	/* - pgpool Connection Settings - */
	StrNCpy(status[i].name, "listen_addresses", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->listen_addresses);
	StrNCpy(status[i].desc, "host name(s) or IP address(es) to listen on", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "port", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->port);
	StrNCpy(status[i].desc, "pgpool accepting port number", POOLCONFIG_MAXDESCLEN);
	i++;

	/* - pgpool Communication Manager Connection Settings - */
	StrNCpy(status[i].name, "socket_dir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->socket_dir);
	StrNCpy(status[i].desc, "pgpool socket directory", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "pcp_listen_addresses", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->pcp_listen_addresses);
	StrNCpy(status[i].desc, "host name(s) or IP address(es) for pcp process to listen on", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "pcp_port", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->pcp_port);
	StrNCpy(status[i].desc, "PCP port # to bind", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "pcp_socket_dir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->pcp_socket_dir);
	StrNCpy(status[i].desc, "PCP socket directory", POOLCONFIG_MAXDESCLEN);
	i++;

	/* # - Authentication - */
	StrNCpy(status[i].name, "enable_pool_hba", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->enable_pool_hba);
	StrNCpy(status[i].desc, "if true, use pool_hba.conf for client authentication", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "pool_passwd", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->pool_passwd);
	StrNCpy(status[i].desc, "file name of pool_passwd for md5 authentication", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "authentication_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->authentication_timeout);
	StrNCpy(status[i].desc, "maximum time in seconds to complete client authentication", POOLCONFIG_MAXNAMELEN);
	i++;

	/* - SSL Connections - */
	StrNCpy(status[i].name, "ssl", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->ssl);
	StrNCpy(status[i].desc, "SSL support", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "ssl_key", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->ssl_key);
	StrNCpy(status[i].desc, "path to the SSL private key file", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "ssl_cert", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->ssl_cert);
	StrNCpy(status[i].desc, "path to the SSL public certificate file", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "ssl_ca_cert", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->ssl_ca_cert);
	StrNCpy(status[i].desc, "path to a single PEM format file", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "ssl_ca_cert_dir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->ssl_ca_cert_dir);
	StrNCpy(status[i].desc, "directory containing CA root certificate(s)", POOLCONFIG_MAXDESCLEN);
	i++;

	/* POOLS */

	/* - Pool size -  */
	StrNCpy(status[i].name, "num_init_children", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->num_init_children);
	StrNCpy(status[i].desc, "# of children initially pre-forked", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "listen_backlog_multiplier", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->listen_backlog_multiplier);
	StrNCpy(status[i].desc, "determines the size of the queue for pending connections", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "max_pool", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->max_pool);
	StrNCpy(status[i].desc, "max # of connection pool per child", POOLCONFIG_MAXDESCLEN);
	i++;

	/* - Life time - */
	StrNCpy(status[i].name, "child_life_time", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->child_life_time);
	StrNCpy(status[i].desc, "if idle for this seconds, child exits", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "child_max_connections", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->child_max_connections);
	StrNCpy(status[i].desc, "if max_connections received, chile exits", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "connection_life_time", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->connection_life_time);
	StrNCpy(status[i].desc, "if idle for this seconds, connection closes", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "client_idle_limit", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->client_idle_limit);
	StrNCpy(status[i].desc, "if idle for this seconds, child connection closes", POOLCONFIG_MAXDESCLEN);
	i++;

	/* LOGS */

	/* - Where to log - */
	StrNCpy(status[i].name, "log_destination", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->log_destination);
	StrNCpy(status[i].desc, "logging destination", POOLCONFIG_MAXDESCLEN);
	i++;

	/* - What to log - */
	StrNCpy(status[i].name, "log_line_prefix", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->log_line_prefix);
	StrNCpy(status[i].desc, "printf-style string to output at beginning of each log line", POOLCONFIG_MAXDESCLEN);
	i++;

    StrNCpy(status[i].name, "log_error_verbosity", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->log_error_verbosity);
	StrNCpy(status[i].desc, "controls how much detail about error should be emitted", POOLCONFIG_MAXDESCLEN);
	i++;

    StrNCpy(status[i].name, "client_min_messages", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->client_min_messages);
	StrNCpy(status[i].desc, "controls which message should be sent to client", POOLCONFIG_MAXDESCLEN);
	i++;

    StrNCpy(status[i].name, "log_min_messages", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->log_min_messages);
	StrNCpy(status[i].desc, "controls which message should be emitted to server log", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "log_connections", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->log_connections);
	StrNCpy(status[i].desc, "if true, print incoming connections to the log", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "log_hostname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->log_hostname);
	StrNCpy(status[i].desc, "if true, resolve hostname for ps and log print", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "log_statement", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->log_statement);
	StrNCpy(status[i].desc, "if non 0, logs all SQL statements", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "log_per_node_statement", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->log_per_node_statement);
	StrNCpy(status[i].desc, "if non 0, logs all SQL statements on each node", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "log_standby_delay", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->log_standby_delay);
	StrNCpy(status[i].desc, "how to log standby delay", POOLCONFIG_MAXDESCLEN);
	i++;

	/* - Syslog specific -  */
	StrNCpy(status[i].name, "syslog_facility", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "LOCAL%d", (pool_config->syslog_facility/8) - 16);
	StrNCpy(status[i].desc, "syslog local faclity", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "syslog_ident", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->syslog_ident);
	StrNCpy(status[i].desc, "syslog program ident string", POOLCONFIG_MAXDESCLEN);
	i++;

	/* - Debug - */
	StrNCpy(status[i].name, "debug_level", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->debug_level);
	StrNCpy(status[i].desc, "debug message level", POOLCONFIG_MAXDESCLEN);
	i++;

	/* FILE LOCATIONS */

	StrNCpy(status[i].name, "pid_file_name", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->pid_file_name);
	StrNCpy(status[i].desc, "path to pid file", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "logdir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->logdir);
	StrNCpy(status[i].desc, "PgPool status file logging directory", POOLCONFIG_MAXDESCLEN);
	i++;

	/* CONNECTION POOLING */

	StrNCpy(status[i].name, "connection_cache", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->connection_cache);
	StrNCpy(status[i].desc, "if true, cache connection pool", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "reset_query_list", POOLCONFIG_MAXNAMELEN);
	*(status[i].value) = '\0';
	for (j=0;j<pool_config->num_reset_queries;j++)
	{
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		strncat(status[i].value, pool_config->reset_query_list[j], len);
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		if (j != pool_config->num_reset_queries-1)
			strncat(status[i].value, ";", len);
	}
	StrNCpy(status[i].desc, "queries issued at the end of session", POOLCONFIG_MAXDESCLEN);
	i++;

	/* REPLICATION MODE */

	StrNCpy(status[i].name, "replication_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->replication_mode);
	StrNCpy(status[i].desc, "non 0 if operating in replication mode", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "replicate_select", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->replicate_select);
	StrNCpy(status[i].desc, "non 0 if SELECT statement is replicated", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "insert_lock", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->insert_lock);
	StrNCpy(status[i].desc, "insert lock", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "lobj_lock_table", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->lobj_lock_table);
	StrNCpy(status[i].desc, "table name used for large object replication control", POOLCONFIG_MAXDESCLEN);
	i++;

	/* - Degenerate handling - */
	StrNCpy(status[i].name, "replication_stop_on_mismatch", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->replication_stop_on_mismatch);
	StrNCpy(status[i].desc, "stop replication mode on fatal error", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "failover_if_affected_tuples_mismatch", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->failover_if_affected_tuples_mismatch);
	StrNCpy(status[i].desc, "failover if affected tuples are mismatch", POOLCONFIG_MAXDESCLEN);
	i++;

	/* LOAD BALANCING MODE */

	StrNCpy(status[i].name, "load_balance_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->load_balance_mode);
	StrNCpy(status[i].desc, "non 0 if operating in load balancing mode", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "ignore_leading_white_space", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->ignore_leading_white_space);
	StrNCpy(status[i].desc, "ignore leading white spaces", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "white_function_list", POOLCONFIG_MAXNAMELEN);
	*(status[i].value) = '\0';
	for (j=0;j<pool_config->num_white_function_list;j++)
	{
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		strncat(status[i].value, pool_config->white_function_list[j], len);
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		if (j != pool_config->num_white_function_list-1)
			strncat(status[i].value, ",", len);
	}
	StrNCpy(status[i].desc, "functions those do not write to database", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "black_function_list", POOLCONFIG_MAXNAMELEN);
	*(status[i].value) = '\0';
	for (j=0;j<pool_config->num_black_function_list;j++)
	{
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		strncat(status[i].value, pool_config->black_function_list[j], len);
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		if (j != pool_config->num_black_function_list-1)
			strncat(status[i].value, ",", len);
	}
	StrNCpy(status[i].desc, "functions those write to database", POOLCONFIG_MAXDESCLEN);
	i++;

	/* MASTER/SLAVE MODE */

	StrNCpy(status[i].name, "master_slave_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->master_slave_mode);
	StrNCpy(status[i].desc, "if true, operate in master/slave mode", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "master_slave_sub_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->master_slave_sub_mode);
	StrNCpy(status[i].desc, "master/slave sub mode", POOLCONFIG_MAXDESCLEN);
	i++;

	/* - Streaming - */
	StrNCpy(status[i].name, "sr_check_period", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->sr_check_period);
	StrNCpy(status[i].desc, "sr check period", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "sr_check_user", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->sr_check_user);
	StrNCpy(status[i].desc, "sr check user", POOLCONFIG_MAXDESCLEN);
	i++;
#ifdef NOT_USED	/* for security reason */
	StrNCpy(status[i].name, "sr_check_password", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->sr_check_password);
	StrNCpy(status[i].desc, "sr check password", POOLCONFIG_MAXDESCLEN);
	i++;
#endif
	StrNCpy(status[i].name, "delay_threshold", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%lld", pool_config->delay_threshold);
	StrNCpy(status[i].desc, "standby delay threshold", POOLCONFIG_MAXDESCLEN);
	i++;

	/* - Special commands - */
	StrNCpy(status[i].name, "follow_master_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->follow_master_command);
	StrNCpy(status[i].desc, "follow master command", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "database_redirect_preference_list", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->database_redirect_preference_list);
	StrNCpy(status[i].desc, "redirect by database name", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "app_name_redirect_preference_list", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->app_name_redirect_preference_list);
	StrNCpy(status[i].desc, "redirect by application name", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "allow_sql_comments", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->allow_sql_comments);
	StrNCpy(status[i].desc, "allow SQL comments", POOLCONFIG_MAXDESCLEN);
	i++;

	/* PARALLEL MODE AND QUERY CACHE */

	StrNCpy(status[i].name, "parallel_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->parallel_mode);
	StrNCpy(status[i].desc, "if non 0, run in parallel query mode", POOLCONFIG_MAXDESCLEN);
	i++;


	StrNCpy(status[i].name, "pgpool2_hostname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->pgpool2_hostname);
	StrNCpy(status[i].desc, "pgpool2 hostname", POOLCONFIG_MAXDESCLEN);
	i++;

	/* - System DB info - */
	StrNCpy(status[i].name, "system_db_hostname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_hostname);
	StrNCpy(status[i].desc, "system DB hostname", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "system_db_port", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->system_db_port);
	StrNCpy(status[i].desc, "system DB port number", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "system_db_dbname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_dbname);
	StrNCpy(status[i].desc, "system DB name", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "system_db_schema", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_schema);
	StrNCpy(status[i].desc, "system DB schema name", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "system_db_user", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_user);
	StrNCpy(status[i].desc, "user name to access system DB", POOLCONFIG_MAXDESCLEN);
	i++;

	/* HEALTH CHECK */

	StrNCpy(status[i].name, "health_check_period", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->health_check_period);
	StrNCpy(status[i].desc, "health check period", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "health_check_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->health_check_timeout);
	StrNCpy(status[i].desc, "health check timeout", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "health_check_user", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->health_check_user);
	StrNCpy(status[i].desc, "health check user", POOLCONFIG_MAXDESCLEN);
	i++;
#ifdef NOT_USED	/* for security reason */
	StrNCpy(status[i].name, "health_check_password", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->health_check_password);
	StrNCpy(status[i].desc, "health check password", POOLCONFIG_MAXDESCLEN);
	i++;
#endif
	StrNCpy(status[i].name, "health_check_max_retries", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->health_check_max_retries);
	StrNCpy(status[i].desc, "health check max retries", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "health_check_retry_delay", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->health_check_retry_delay);
	StrNCpy(status[i].desc, "health check retry delay", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "connect_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->connect_timeout);
	StrNCpy(status[i].desc, "connect timeout", POOLCONFIG_MAXDESCLEN);
	i++;

	/* FAILOVER AND FAILBACK */

	StrNCpy(status[i].name, "failover_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->failover_command);
	StrNCpy(status[i].desc, "failover command", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "failback_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->failback_command);
	StrNCpy(status[i].desc, "failback command", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "fail_over_on_backend_error", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->fail_over_on_backend_error);
	StrNCpy(status[i].desc, "fail over on backend error", POOLCONFIG_MAXDESCLEN);
	i++;

	/* ONLINE RECOVERY */

	StrNCpy(status[i].name, "recovery_user", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->recovery_user);
	StrNCpy(status[i].desc, "online recovery user", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "recovery_1st_stage_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->recovery_1st_stage_command);
	StrNCpy(status[i].desc, "execute a command in first stage.", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "recovery_2nd_stage_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->recovery_2nd_stage_command);
	StrNCpy(status[i].desc, "execute a command in second stage.", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "recovery_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->recovery_timeout);
	StrNCpy(status[i].desc, "max time in seconds to wait for the recovering node's postmaster", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "search_primary_node_timeout", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->search_primary_node_timeout);
	StrNCpy(status[i].desc, "max time in seconds to search for primary node after failover", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "client_idle_limit_in_recovery", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->client_idle_limit_in_recovery);
	StrNCpy(status[i].desc, "if idle for this seconds, child connection closes in recovery 2nd statge", POOLCONFIG_MAXDESCLEN);
	i++;

	/* OTHERS */

	StrNCpy(status[i].name, "relcache_expire", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%ld", pool_config->relcache_expire);
	StrNCpy(status[i].desc, "relation cache expiration time in seconds", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "relcache_size", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->relcache_size);
	StrNCpy(status[i].desc, "number of relation cache entry", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "check_temp_table", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->check_temp_table);
	StrNCpy(status[i].desc, "enable temporary table check", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "check_unlogged_table", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->check_unlogged_table);
	StrNCpy(status[i].desc, "enable unlogged table check", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "parallel_mode", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->parallel_mode);
	StrNCpy(status[i].desc, "if non 0, run in parallel query mode", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "pgpool2_hostname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->pgpool2_hostname);
	StrNCpy(status[i].desc, "pgpool2 hostname", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "system_db_hostname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_hostname);
	StrNCpy(status[i].desc, "system DB hostname", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "system_db_port", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->system_db_port);
	StrNCpy(status[i].desc, "system DB port number", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "system_db_dbname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_dbname);
	StrNCpy(status[i].desc, "system DB name", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "system_db_schema", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_schema);
	StrNCpy(status[i].desc, "system DB schema name", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "system_db_user", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->system_db_user);
	StrNCpy(status[i].desc, "user name to access system DB", POOLCONFIG_MAXDESCLEN);
	i++;
	
	/*
	 * add for watchdog
	 */
	StrNCpy(status[i].name, "use_watchdog", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->use_watchdog);
	StrNCpy(status[i].desc, "non 0 if operating in use_watchdog", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "wd_lifecheck_method", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->wd_lifecheck_method);
	StrNCpy(status[i].desc, "method of watchdog lifecheck", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "clear_memqcache_on_escalation", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->clear_memqcache_on_escalation);
	StrNCpy(status[i].desc, "If true, clear all the query caches in shared memory when escalation occurs", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "wd_escalation_command", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->wd_escalation_command);
	StrNCpy(status[i].desc, "command executed when escalation occurs", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "trusted_servers", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->trusted_servers);
	StrNCpy(status[i].desc, "upper server list to observe connection", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "delegate_IP", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->delegate_IP);
	StrNCpy(status[i].desc, "delegate IP address of master pgpool", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "wd_hostname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->wd_hostname);
	StrNCpy(status[i].desc, "Host name or IP address of this watchdog", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "wd_port", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->wd_port);
	StrNCpy(status[i].desc, "watchdog port number", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "wd_interval", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->wd_interval);
	StrNCpy(status[i].desc, "life check interval (second)", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "ping_path", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->ping_path);
	StrNCpy(status[i].desc, "path to ping command", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "ifconfig_path", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->ifconfig_path);
	StrNCpy(status[i].desc, "path to ifconfig command", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "if_up_cmd", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->if_up_cmd);
	StrNCpy(status[i].desc, "virtual interface up command with full parameters", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "if_down_cmd", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->if_down_cmd);
	StrNCpy(status[i].desc, "virtual interface down command with full parameters", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "arping_path", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->arping_path);
	StrNCpy(status[i].desc, "path to arping command", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "arping_cmd", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->arping_cmd);
	StrNCpy(status[i].desc, "send ARP REQUESTi to neighbour host", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "wd_heartbeat_port", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->wd_heartbeat_port);
	StrNCpy(status[i].desc, "port number for receiving heartbeat signal", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "wd_heartbeat_keepalive", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->wd_heartbeat_keepalive);
	StrNCpy(status[i].desc, "interval time of sending heartbeat siganl (sec)", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "wd_heartbeat_deadtime", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->wd_heartbeat_deadtime);
	StrNCpy(status[i].desc, "deadtime interval for heartbeat siganl (sec)", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "wd_life_point", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->wd_life_point);
	StrNCpy(status[i].desc, "retry times of life check", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "wd_lifecheck_query", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->wd_lifecheck_query);
	StrNCpy(status[i].desc, "lifecheck query to pgpool from watchdog", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "wd_lifecheck_dbname", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->wd_lifecheck_dbname);
	StrNCpy(status[i].desc, "database name connected for lifecheck", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "wd_lifecheck_user", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->wd_lifecheck_user);
	StrNCpy(status[i].desc, "watchdog user monitoring pgpools in lifecheck", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "wd_lifecheck_password", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->wd_lifecheck_password);
	StrNCpy(status[i].desc, "password for watchdog user in lifecheck", POOLCONFIG_MAXDESCLEN);
	i++;

	/*
	 * end of watchdog
	 */

	StrNCpy(status[i].name, "memory_cache_enabled", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->memory_cache_enabled);
	StrNCpy(status[i].desc, "If true, use the memory cache functionality, false by default", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "memqcache_method", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->memqcache_method);
	StrNCpy(status[i].desc, "Cache store method. either shmem(shared memory) or Memcached. shmem by default", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "memqcache_memcached_host", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->memqcache_memcached_host);
	StrNCpy(status[i].desc, "Memcached host name. Mandatory if memqcache_method=memcached", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "memqcache_memcached_port", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->memqcache_memcached_port);
	StrNCpy(status[i].desc, "Memcached port number. Mondatory if memqcache_method=memcached", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "memqcache_total_size", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->memqcache_total_size);
	StrNCpy(status[i].desc, "Total memory size in bytes for storing memory cache. Mandatory if memqcache_method=shmem", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "memqcache_max_num_cache", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->memqcache_max_num_cache);
	StrNCpy(status[i].desc, "Total number of cache entries", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "memqcache_expire", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->memqcache_expire);
	StrNCpy(status[i].desc, "Memory cache entry life time specified in seconds. 60 by default", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "memqcache_auto_cache_invalidation", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->memqcache_auto_cache_invalidation);
	StrNCpy(status[i].desc, "If true, invalidation of query cache is triggered by corresponding DDL/DML/DCL(and memqcache_expire).  If false, it is only triggered  by memqcache_expire.  True by default.", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "memqcache_maxcache", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->memqcache_maxcache);
	StrNCpy(status[i].desc, "Maximum SELECT result size in bytes", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "memqcache_cache_block_size", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->memqcache_cache_block_size);
	StrNCpy(status[i].desc, "Cache block size in bytes. 8192 by default", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "memqcache_cache_oiddir", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->memqcache_oiddir);
	StrNCpy(status[i].desc, "Tempory work directory to record table oids", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "memqcache_stats_start_time", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", ctime(&pool_get_memqcache_stats()->start_time));
	StrNCpy(status[i].desc, "Start time of query cache stats", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "memqcache_no_cache_hits", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%lld", pool_get_memqcache_stats()->num_selects);
	StrNCpy(status[i].desc, "Number of SELECTs not hitting query cache", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "memqcache_cache_hits", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%lld", pool_get_memqcache_stats()->num_cache_hits);
	StrNCpy(status[i].desc, "Number of SELECTs hitting query cache", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "white_memqcache_table_list", POOLCONFIG_MAXNAMELEN);
	*(status[i].value) = '\0';
	for (j=0;j<pool_config->num_white_memqcache_table_list;j++)
	{
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		strncat(status[i].value, pool_config->white_memqcache_table_list[j], len);
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		if (j != pool_config->num_white_memqcache_table_list - 1)
			strncat(status[i].value, ",", len);
	}
	StrNCpy(status[i].desc, "tables to memqcache", POOLCONFIG_MAXDESCLEN);
	i++;

	StrNCpy(status[i].name, "black_memqcache_table_list", POOLCONFIG_MAXNAMELEN);
	*(status[i].value) = '\0';
	for (j=0;j<pool_config->num_black_memqcache_table_list;j++)
	{
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		strncat(status[i].value, pool_config->black_memqcache_table_list[j], len);
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		if (j != pool_config->num_black_memqcache_table_list - 1)
			strncat(status[i].value, ",", len);
	}
	StrNCpy(status[i].desc, "tables not to memqcache", POOLCONFIG_MAXDESCLEN);
	i++;

	/* BACKENDS */

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
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%f", BACKEND_INFO(j).backend_weight/RAND_MAX);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "weight of backend #%d", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "backend_data_directory%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", BACKEND_INFO(j).backend_data_directory);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "data directory for backend #%d", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "backend_status%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", BACKEND_INFO(j).backend_status);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "status of backend #%d", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "standby_delay%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%lld", BACKEND_INFO(j).standby_delay);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "standby delay of backend #%d", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "backend_flag%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_flag_to_str(BACKEND_INFO(j).flag));
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "backend #%d flag", j);
		i++;
	}

	for (j = 0; j < MAX_WATCHDOG_NUM; j++)
	{
		if (WD_INFO(j).pgpool_port == 0)
			continue;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "other_pgpool_hostname%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", WD_INFO(j).hostname);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "pgpool #%d hostname", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "other_pgpool_port%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", WD_INFO(j).pgpool_port);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "pgpool #%d port number", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "other_pgpool_wd_port%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", WD_INFO(j).wd_port);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "pgpool #%d watchdog port number", j);
		i++;

	}

	for (j =0; j < pool_config->num_hb_if; j++)
	{
		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "heartbeat_device%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", WD_HB_IF(j).if_name);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "name of NIC device #%d for sending hearbeat", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "heartbeat_destination%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", WD_HB_IF(j).addr);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "destination host for sending heartbeat using NIC device %d", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "heartbeat_destination_port%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", WD_HB_IF(j).dest_port);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "destination port for sending heartbeat using NIC device %d", j);
		i++;
	}

	*nrows = i;
	return status;
}

void config_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	static char *field_names[] = {"item", "value", "description"};
	static unsigned char nullmap[2] = {0xff, 0xff};
	static short num_fields = 3;
	short s;
	int nbytes = (num_fields + 7)/8;
	int len;
	int nrows;
	int size;
	int hsize;
	int i;

	POOL_REPORT_CONFIG *status = get_config(&nrows);

	send_row_description(frontend, backend, num_fields, field_names);

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
			len = 6; /* int32 + int16; */
			len += 4 + strlen(status[i].name);  /* int32 + data; */
			len += 4 + strlen(status[i].value); /* int32 + data; */
			len += 4 + strlen(status[i].desc);  /* int32 + data; */
			len = htonl(len);
			pool_write(frontend, &len, sizeof(len));
			s = htons(num_fields);
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

	send_complete_and_ready(frontend, backend, nrows);

	pfree(status);
}

POOL_REPORT_NODES* get_nodes(int *nrows)
{
	int i;
	POOL_REPORT_NODES* nodes = palloc(NUM_BACKENDS * sizeof(POOL_REPORT_NODES));
	BackendInfo *bi = NULL;

    for (i = 0; i < NUM_BACKENDS; i++)
	{
	    bi = pool_get_node_info(i);

		snprintf(nodes[i].node_id, 	POOLCONFIG_MAXIDLEN, 	"%d", 	i);
	    StrNCpy(nodes[i].hostname, 	bi->backend_hostname, 		strlen(bi->backend_hostname)+1);
	    snprintf(nodes[i].port, 	POOLCONFIG_MAXPORTLEN, "%d", 	bi->backend_port);
	    snprintf(nodes[i].status, 	POOLCONFIG_MAXSTATLEN, 	"%d", 	bi->backend_status);
	    snprintf(nodes[i].lb_weight, POOLCONFIG_MAXWEIGHTLEN, "%f", bi->backend_weight/RAND_MAX);

		if (MASTER_SLAVE && !strcmp(pool_config->master_slave_sub_mode, MODE_STREAMREP))
			if (i == REAL_PRIMARY_NODE_ID)
				snprintf(nodes[i].role, POOLCONFIG_MAXWEIGHTLEN, "%s", "primary");
			else
				snprintf(nodes[i].role, POOLCONFIG_MAXWEIGHTLEN, "%s", "standby");
		else
		{
			if (i == REAL_MASTER_NODE_ID)
				snprintf(nodes[i].role, POOLCONFIG_MAXWEIGHTLEN, "%s", "master");
			else
				snprintf(nodes[i].role, POOLCONFIG_MAXWEIGHTLEN, "%s", "slave");
		}
	}

	*nrows = i;

	return nodes;
}

void nodes_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	static char *field_names[] = {"node_id","hostname", "port", "status", "lb_weight", "role"};
	short num_fields = sizeof(field_names)/sizeof(char *);
	int i;
	short s;
	int len;
	int nrows;
	int size;
	int hsize;
	static unsigned char nullmap[2] = {0xff, 0xff};
	int nbytes = (num_fields + 7)/8;

	POOL_REPORT_NODES* nodes = get_nodes(&nrows);

	send_row_description(frontend, backend, num_fields, field_names);

	if (MAJOR(backend) == PROTO_MAJOR_V2)
	{
		/* ascii row */
		for (i=0;i<nrows;i++)
		{
			pool_write(frontend, "D", 1);
			pool_write_and_flush(frontend, nullmap, nbytes);

			size = strlen(nodes[i].node_id);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, nodes[i].node_id, size);

			size = strlen(nodes[i].hostname);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, nodes[i].hostname, size);

			size = strlen(nodes[i].port);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, nodes[i].port, size);

			size = strlen(nodes[i].status);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, nodes[i].status, size);

			size = strlen(nodes[i].lb_weight);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, nodes[i].lb_weight, size);

			size = strlen(nodes[i].role);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, nodes[i].role, size);
		}
	}
	else
	{
		/* data row */
		for (i=0;i<nrows;i++)
		{
			pool_write(frontend, "D", 1);
			len = 6; /* int32 + int16; */
			len += 4 + strlen(nodes[i].node_id);   /* int32 + data; */
			len += 4 + strlen(nodes[i].hostname);  /* int32 + data; */
			len += 4 + strlen(nodes[i].port);      /* int32 + data; */
			len += 4 + strlen(nodes[i].status);    /* int32 + data; */
			len += 4 + strlen(nodes[i].lb_weight); /* int32 + data; */
			len += 4 + strlen(nodes[i].role);      /* int32 + data; */
			len = htonl(len);
			pool_write(frontend, &len, sizeof(len));
			s = htons(num_fields);
			pool_write(frontend, &s, sizeof(s));

			len = htonl(strlen(nodes[i].node_id));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, nodes[i].node_id, strlen(nodes[i].node_id));

			len = htonl(strlen(nodes[i].hostname));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, nodes[i].hostname, strlen(nodes[i].hostname));

			len = htonl(strlen(nodes[i].port));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, nodes[i].port, strlen(nodes[i].port));

			len = htonl(strlen(nodes[i].status));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, nodes[i].status, strlen(nodes[i].status));

			len = htonl(strlen(nodes[i].lb_weight));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, nodes[i].lb_weight, strlen(nodes[i].lb_weight));

			len = htonl(strlen(nodes[i].role));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, nodes[i].role, strlen(nodes[i].role));
		}
	}

	send_complete_and_ready(frontend, backend, nrows);

	pfree(nodes);
}


POOL_REPORT_POOLS* get_pools(int *nrows)
{
	int child, pool, poolBE, backend_id;

    ProcessInfo *pi = NULL;
    int proc_id;

    int lines = 0;

    POOL_REPORT_POOLS* pools = palloc(
		pool_config->num_init_children * pool_config->max_pool * NUM_BACKENDS * sizeof(POOL_REPORT_POOLS)
		);

	for (child = 0; child < pool_config->num_init_children; child++)
	{
		proc_id = process_info[child].pid;
		pi = pool_get_process_info(proc_id);

		for (pool = 0; pool < pool_config->max_pool; pool++)
		{
			for (backend_id = 0; backend_id < NUM_BACKENDS; backend_id++)
			{
				poolBE = pool*MAX_NUM_BACKENDS+backend_id;
				pools[lines].pool_pid = proc_id;
				pools[lines].start_time = pi->start_time;
				pools[lines].pool_id = pool;
				pools[lines].backend_id = backend_id;
				if (strlen(pi->connection_info[poolBE].database) == 0)
				{
					StrNCpy(pools[lines].database, "", POOLCONFIG_MAXIDENTLEN);
					StrNCpy(pools[lines].username, "", POOLCONFIG_MAXIDENTLEN);
					pools[lines].create_time = 0;
					pools[lines].pool_majorversion = 0;
					pools[lines].pool_minorversion = 0;
				}
				else
				{
					StrNCpy(pools[lines].database, pi->connection_info[poolBE].database, POOLCONFIG_MAXIDENTLEN);
					StrNCpy(pools[lines].username, pi->connection_info[poolBE].user, POOLCONFIG_MAXIDENTLEN);
					pools[lines].create_time = pi->connection_info[poolBE].create_time;
					pools[lines].pool_majorversion = pi->connection_info[poolBE].major;
					pools[lines].pool_minorversion = pi->connection_info[poolBE].minor;
				}
				pools[lines].pool_counter = pi->connection_info[poolBE].counter;
				pools[lines].pool_backendpid = ntohl(pi->connection_info[poolBE].pid);
				pools[lines].pool_connected = pi->connection_info[poolBE].connected;
				lines++;
			}
		}
    }

	*nrows = lines;
	return pools;
}

void pools_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	static short num_fields = 12;
	static char *field_names[] = {"pool_pid", "start_time", "pool_id", "backend_id", "database", "username", "create_time",
                                  "majorversion", "minorversion", "pool_counter", "pool_backendpid", "pool_connected"};
	short s;
	int len;
	int i;
	static unsigned char nullmap[2] = {0xff, 0xff};
	int nbytes = (num_fields + 7)/8;
	int nrows;
	int size;
	int hsize;
	char proc_pid[16];
	char pool_id[16];
	char proc_start_time[20];
	char proc_create_time[20];
	char majorversion[5];
	char minorversion[5];
	char pool_counter[16];
	char backend_id[16];
	char backend_pid[16];
	char connected[2];

    POOL_REPORT_POOLS *pools = get_pools(&nrows);

	send_row_description(frontend, backend, num_fields, field_names);

	if (MAJOR(backend) == PROTO_MAJOR_V2) hsize = 4;
	else hsize = 0;

	/* ascii row */
	for (i=0;i<nrows;i++)
	{
		snprintf(proc_pid, sizeof(proc_pid), "%d", pools[i].pool_pid);
		snprintf(pool_id, sizeof(pool_id), "%d", pools[i].pool_id);
		if (pools[i].start_time)
			strftime(proc_start_time, sizeof(proc_start_time), "%Y-%m-%d %H:%M:%S", localtime(&pools[i].start_time));
		else
			*proc_start_time = '\0';
		if (pools[i].create_time)
			strftime(proc_create_time, sizeof(proc_create_time), "%Y-%m-%d %H:%M:%S", localtime(&pools[i].create_time));
		else
			*proc_create_time = '\0';
		snprintf(majorversion, sizeof(majorversion), "%d", pools[i].pool_majorversion);
		snprintf(minorversion, sizeof(minorversion), "%d", pools[i].pool_minorversion);
		snprintf(pool_counter, sizeof(pool_counter), "%d", pools[i].pool_counter);
		snprintf(backend_id, sizeof(backend_pid), "%d", pools[i].backend_id);
		snprintf(backend_pid, sizeof(backend_pid), "%d", pools[i].pool_backendpid);
		snprintf(connected, sizeof(connected), "%d", pools[i].pool_connected);

		if (MAJOR(backend) == PROTO_MAJOR_V2)
		{
			pool_write(frontend, "D", 1);
			pool_write_and_flush(frontend, nullmap, nbytes);
		}
		else
		{
			pool_write(frontend, "D", 1);
			len = 6; /* int32 + int16; */
			len += 4 + strlen(proc_pid);          /* int32 + data */
			len += 4 + strlen(proc_start_time);        /* int32 + data */
			len += 4 + strlen(pool_id);           /* int32 + data */
			len += 4 + strlen(backend_id);        /* int32 + data */
			len += 4 + strlen(pools[i].database);          /* int32 + data */
			len += 4 + strlen(pools[i].username);          /* int32 + data */
			len += 4 + strlen(proc_create_time);       /* int32 + data */
			len += 4 + strlen(majorversion); /* int32 + data */
			len += 4 + strlen(minorversion); /* int32 + data */
			len += 4 + strlen(pool_counter);      /* int32 + data */
			len += 4 + strlen(backend_pid);   /* int32 + data */
			len += 4 + strlen(connected);    /* int32 + data */
			
			len = htonl(len);
			pool_write(frontend, &len, sizeof(len));
			s = htons(num_fields);
			pool_write(frontend, &s, sizeof(s));
		}

		len = strlen(proc_pid);
		size = htonl(len+hsize);
		pool_write(frontend, &size, sizeof(size));
		pool_write(frontend, proc_pid, len);

		len = strlen(proc_start_time);
		size = htonl(len+hsize);
		pool_write(frontend, &size, sizeof(size));
		pool_write(frontend, proc_start_time, len);

		len = strlen(pool_id);
		size = htonl(len+hsize);
		pool_write(frontend, &size, sizeof(size));
		pool_write(frontend, pool_id, len);

		len = strlen(backend_id);
		size = htonl(len+hsize);
		pool_write(frontend, &size, sizeof(size));
		pool_write(frontend, backend_id, len);

		len = strlen(pools[i].database);
		size = htonl(len+hsize);
		pool_write(frontend, &size, sizeof(size));
		pool_write(frontend, pools[i].database, len);

		len = strlen(pools[i].username);
		size = htonl(len+hsize);
		pool_write(frontend, &size, sizeof(size));
		pool_write(frontend, pools[i].username, len);

		len = strlen(proc_create_time);
		size = htonl(len+hsize);
		pool_write(frontend, &size, sizeof(size));
		pool_write(frontend, proc_create_time, len);

		len = strlen(majorversion);
		size = htonl(len+hsize);
		pool_write(frontend, &size, sizeof(size));
		pool_write(frontend, majorversion, len);

		len = strlen(minorversion);
		size = htonl(len+hsize);
		pool_write(frontend, &size, sizeof(size));
		pool_write(frontend, minorversion, len);

		len = strlen(pool_counter);
		size = htonl(len+hsize);
		pool_write(frontend, &size, sizeof(size));
		pool_write(frontend, pool_counter, len);

		len = strlen(backend_pid);
		size = htonl(len+hsize);
		pool_write(frontend, &size, sizeof(size));
		pool_write(frontend, backend_pid, len);

		len = strlen(connected);
		size = htonl(len+hsize);
		pool_write(frontend, &size, sizeof(size));
		pool_write(frontend, connected, len);
	}

	send_complete_and_ready(frontend, backend, nrows);

	pfree(pools);
}

POOL_REPORT_PROCESSES* get_processes(int *nrows)
{
	int child;
    int pool;
    int poolBE;
    ProcessInfo *pi = NULL;
    int proc_id;

    POOL_REPORT_PROCESSES* processes = palloc(pool_config->num_init_children * sizeof(POOL_REPORT_PROCESSES));

	for (child = 0; child < pool_config->num_init_children; child++)
    {
		proc_id = process_info[child].pid;
	    pi = pool_get_process_info(proc_id);

        snprintf(processes[child].pool_pid, POOLCONFIG_MAXCOUNTLEN, "%d", proc_id);
	    strftime(processes[child].start_time, POOLCONFIG_MAXDATELEN, "%Y-%m-%d %H:%M:%S", localtime(&pi->start_time));
	    StrNCpy(processes[child].database, "", POOLCONFIG_MAXIDENTLEN);
	    StrNCpy(processes[child].username, "", POOLCONFIG_MAXIDENTLEN);
        StrNCpy(processes[child].create_time, "", POOLCONFIG_MAXDATELEN);
        StrNCpy(processes[child].pool_counter, "", POOLCONFIG_MAXCOUNTLEN);

        for (pool = 0; pool < pool_config->max_pool; pool++)
        {
            poolBE = pool*MAX_NUM_BACKENDS;
            if (pi->connection_info[poolBE].connected && strlen(pi->connection_info[poolBE].database) > 0 && strlen(pi->connection_info[poolBE].user) > 0)
            {
	            StrNCpy(processes[child].database, pi->connection_info[poolBE].database, POOLCONFIG_MAXIDENTLEN);
	            StrNCpy(processes[child].username, pi->connection_info[poolBE].user, POOLCONFIG_MAXIDENTLEN);
	            strftime(processes[child].create_time, POOLCONFIG_MAXDATELEN, "%Y-%m-%d %H:%M:%S", localtime(&pi->connection_info[poolBE].create_time));
                snprintf(processes[child].pool_counter, POOLCONFIG_MAXCOUNTLEN, "%d", pi->connection_info[poolBE].counter);
            }
        }
    }

	*nrows = child;

	return processes;
}

void processes_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	static short num_fields = 6;
	static char *field_names[] = {"pool_pid", "start_time", "database", "username", "create_time", "pool_counter"};
	short s;
	int len;
	int nrows;
	int size;
	int hsize;
    int i;
    static unsigned char nullmap[2] = {0xff, 0xff};
	int nbytes = (num_fields + 7)/8;

    POOL_REPORT_PROCESSES *processes = get_processes(&nrows);

	send_row_description(frontend, backend, num_fields, field_names);

	if (MAJOR(backend) == PROTO_MAJOR_V2)
	{
		/* ascii row */
		for (i=0;i<nrows;i++)
		{
			pool_write(frontend, "D", 1);
			pool_write_and_flush(frontend, nullmap, nbytes);

			size = strlen(processes[i].pool_pid);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, processes[i].pool_pid, size);

			size = strlen(processes[i].start_time);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, processes[i].start_time, size);

			size = strlen(processes[i].database);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, processes[i].database, size);

			size = strlen(processes[i].username);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, processes[i].username, size);

			size = strlen(processes[i].create_time);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, processes[i].create_time, size);

			size = strlen(processes[i].pool_counter);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, processes[i].pool_counter, size);
		}
	}
	else
	{
		/* data row */
		for (i=0;i<nrows;i++)
		{
			pool_write(frontend, "D", 1);
			len = 6; /* int32 + int16; */
			len += 4 + strlen(processes[i].pool_pid);     /* int32 + data */
			len += 4 + strlen(processes[i].start_time);   /* int32 + data */
			len += 4 + strlen(processes[i].database);     /* int32 + data */
			len += 4 + strlen(processes[i].username);     /* int32 + data */
			len += 4 + strlen(processes[i].create_time);  /* int32 + data */
			len += 4 + strlen(processes[i].pool_counter); /* int32 + data */
			len = htonl(len);
			pool_write(frontend, &len, sizeof(len));
			s = htons(num_fields);
			pool_write(frontend, &s, sizeof(s));

			len = htonl(strlen(processes[i].pool_pid));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, processes[i].pool_pid, strlen(processes[i].pool_pid));

			len = htonl(strlen(processes[i].start_time));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, processes[i].start_time, strlen(processes[i].start_time));

			len = htonl(strlen(processes[i].database));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, processes[i].database, strlen(processes[i].database));

			len = htonl(strlen(processes[i].username));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, processes[i].username, strlen(processes[i].username));

			len = htonl(strlen(processes[i].create_time));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, processes[i].create_time, strlen(processes[i].create_time));

			len = htonl(strlen(processes[i].pool_counter));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, processes[i].pool_counter, strlen(processes[i].pool_counter));
		}
	}

	send_complete_and_ready(frontend, backend, nrows);

	pfree(processes);
}

POOL_REPORT_VERSION* get_version(void)
{
	POOL_REPORT_VERSION *version = palloc(sizeof(POOL_REPORT_VERSION));

	snprintf(version[0].version, POOLCONFIG_MAXVALLEN, "%s (%s)", VERSION, PGPOOLVERSION);

	return version;
}

void version_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	static short num_fields = 1;
	static char *field_names[] = {"pool_version"};
	short s;
	int len;
	int size;
	int hsize;

	static unsigned char nullmap[2] = {0xff, 0xff};
	int nbytes = (num_fields + 7)/8;

	POOL_REPORT_VERSION *version = get_version();

	send_row_description(frontend, backend, num_fields, field_names);

	if (MAJOR(backend) == PROTO_MAJOR_V2)
	{
		/* ascii row */
		pool_write(frontend, "D", 1);
		pool_write_and_flush(frontend, nullmap, nbytes);

		size = strlen(version[0].version);
		hsize = htonl(size+4);
		pool_write(frontend, &hsize, sizeof(hsize));
		pool_write(frontend, version[0].version, size);
	}
	else
	{
		/* data row */
		pool_write(frontend, "D", 1);
		len = 6; /* int32 + int16; */
		len += 4 + strlen(version[0].version); /* int32 + data */
		len = htonl(len);
		pool_write(frontend, &len, sizeof(len));
		s = htons(num_fields);
		pool_write(frontend, &s, sizeof(s));

		len = htonl(strlen(version[0].version));
		pool_write(frontend, &len, sizeof(len));
		pool_write(frontend, version[0].version, strlen(version[0].version));
	}

	send_complete_and_ready(frontend, backend, 1);

	pfree(version);
}

/*
 * Show in memory cache reporting
 */
void cache_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	static char *field_names[] = {"num_cache_hits", "num_selects", "cache_hit_ratio", "num_hash_entries", "used_hash_entries", "num_cache_entries", "used_cache_entries_size", "free_cache_entries_size", "fragment_cache_entries_size"};
	short num_fields = sizeof(field_names)/sizeof(char *);
	int i;
	short s;
	int len;
	int size;
	int hsize;
	static unsigned char nullmap[2] = {0xff, 0xff};
	int nbytes = (num_fields + 7)/8;
	volatile POOL_SHMEM_STATS *mystats;
#ifdef HAVE_SIGPROCMASK
	sigset_t oldmask;
#else
	int	oldmask;
#endif
	double ratio;

#define POOL_CACHE_STATS_MAX_STRING_LEN 32
	typedef struct {
		int len;		/* length of string excluding null terminate */
		char string[POOL_CACHE_STATS_MAX_STRING_LEN+1];
	} MY_STRING_CACHE_STATS;

	MY_STRING_CACHE_STATS *strp;

	strp = palloc(num_fields*sizeof(MY_STRING_CACHE_STATS));

	/*
	 * Get raw cache stat data
	 */
    POOL_SETMASK2(&BlockSig, &oldmask);
    pool_shmem_lock();

    PG_TRY();
    {
        mystats = pool_get_shmem_storage_stats();
    }
    PG_CATCH();
    {
        pool_shmem_unlock();
        POOL_SETMASK(&oldmask);
        PG_RE_THROW();
    }
    PG_END_TRY();
    pool_shmem_unlock();
    POOL_SETMASK(&oldmask);

	/*
	 * Convert to string
	 */
	i = 0;
	snprintf(strp[i++].string, POOL_CACHE_STATS_MAX_STRING_LEN+1, "%lld", mystats->cache_stats.num_cache_hits);
	snprintf(strp[i++].string, POOL_CACHE_STATS_MAX_STRING_LEN+1, "%lld", mystats->cache_stats.num_selects);
	if ((mystats->cache_stats.num_cache_hits + mystats->cache_stats.num_selects) == 0)
	{
		ratio = 0.0;
	}
	else
	{
		ratio = (double)mystats->cache_stats.num_cache_hits/(mystats->cache_stats.num_selects + mystats->cache_stats.num_cache_hits);
	}
	snprintf(strp[i++].string, POOL_CACHE_STATS_MAX_STRING_LEN+1, "%.2f", ratio);
	snprintf(strp[i++].string, POOL_CACHE_STATS_MAX_STRING_LEN+1, "%d", mystats->num_hash_entries);
	snprintf(strp[i++].string, POOL_CACHE_STATS_MAX_STRING_LEN+1, "%d", mystats->used_hash_entries);
	snprintf(strp[i++].string, POOL_CACHE_STATS_MAX_STRING_LEN+1, "%d", mystats->num_cache_entries);
	snprintf(strp[i++].string, POOL_CACHE_STATS_MAX_STRING_LEN+1, "%ld", mystats->used_cache_entries_size);
	snprintf(strp[i++].string, POOL_CACHE_STATS_MAX_STRING_LEN+1, "%ld", mystats->free_cache_entries_size);
	snprintf(strp[i++].string, POOL_CACHE_STATS_MAX_STRING_LEN+1, "%ld", mystats->fragment_cache_entries_size);

	/*
	 * Calculate total data length
	 */
	len = 2;	/* number of fields (int16) */
	for (i=0;i<num_fields;i++)
	{
		strp[i].len = strlen(strp[i].string);
		len += 4 /* length of string (int32) */
			+ strp[i].len;
	}

	/* Send row description */
	send_row_description(frontend, backend, num_fields, field_names);

	/* Send each field */
	if (MAJOR(backend) == PROTO_MAJOR_V2)
	{
		pool_write(frontend, "D", 1);
		pool_write(frontend, nullmap, nbytes);

		for (i=0;i<num_fields;i++)
		{
			size = strp[i].len + 1;
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, strp[i].string, size);
		}
	}
	else
	{
		/* Kind */
		pool_write(frontend, "D", 1);
		/* Packet length */
		len = htonl(len+sizeof(int32));
		pool_write(frontend, &len, sizeof(len));
		/* Number of fields */
		s = htons(num_fields);
		pool_write(frontend, &s, sizeof(s));

		for (i=0;i<num_fields;i++)
		{
			hsize = htonl(strp[i].len);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, strp[i].string, strp[i].len);
		}
	}

	send_complete_and_ready(frontend, backend, 1);

	pfree(strp);
}
