/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2011	PgPool Global Development Group
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
#include "pool_proto_modules.h"
#include "pool_stream.h"
#include "pool_config.h"
#include "version.h"

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

/* some length definitions */
#define POOLCONFIG_MAXNAMELEN 64
#define POOLCONFIG_MAXVALLEN 512
#define POOLCONFIG_MAXDESCLEN 64
#define POOLCONFIG_MAXIDENTLEN 63
#define POOLCONFIG_MAXPORTLEN 6
#define POOLCONFIG_MAXSTATLEN 2
#define POOLCONFIG_MAXWEIGHTLEN 20
#define POOLCONFIG_MAXDATELEN 128
#define POOLCONFIG_MAXCOUNTLEN 16

/* config report struct*/
typedef struct {
	char name[POOLCONFIG_MAXNAMELEN+1];
	char value[POOLCONFIG_MAXVALLEN+1];
	char desc[POOLCONFIG_MAXDESCLEN+1];
} POOL_REPORT_STATUS;

/* nodes report struct */
typedef struct {
        char node_id[POOLCONFIG_MAXSTATLEN+1];
	char hostname[POOLCONFIG_MAXIDENTLEN+1];
	char port[POOLCONFIG_MAXIDENTLEN+1];
	char status[POOLCONFIG_MAXSTATLEN+1];
	char lb_weight[POOLCONFIG_MAXWEIGHTLEN+1];
} POOL_REPORT_NODES;

/* processes report struct */
typedef struct {
	char pool_pid[POOLCONFIG_MAXCOUNTLEN+1];
	char start_time[POOLCONFIG_MAXDATELEN+1];
	char database[POOLCONFIG_MAXIDENTLEN+1];
	char username[POOLCONFIG_MAXIDENTLEN+1];
	char create_time[POOLCONFIG_MAXDATELEN+1];
	char pool_counter[POOLCONFIG_MAXCOUNTLEN+1];
} POOL_REPORT_PROCESSES;

/* pools reporting struct */
typedef struct {
	char pool_pid[POOLCONFIG_MAXCOUNTLEN+1];
	char start_time[POOLCONFIG_MAXDATELEN+1];
	char pool_id[POOLCONFIG_MAXCOUNTLEN+1];
	char backend_id[POOLCONFIG_MAXCOUNTLEN+1];
	char database[POOLCONFIG_MAXIDENTLEN+1];
	char username[POOLCONFIG_MAXIDENTLEN+1];
	char create_time[POOLCONFIG_MAXDATELEN+1];
	char pool_majorversion[POOLCONFIG_MAXCOUNTLEN+1];
	char pool_minorversion[POOLCONFIG_MAXCOUNTLEN+1];
	char pool_counter[POOLCONFIG_MAXCOUNTLEN+1];
	char pool_backendpid[POOLCONFIG_MAXCOUNTLEN+1];
    char pool_connected[POOLCONFIG_MAXCOUNTLEN+1];
} POOL_REPORT_POOLS;

/* version struct */
typedef struct {
	char version[POOLCONFIG_MAXVALLEN+1];
} POOL_REPORT_VERSION;

void config_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
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

/*
 * Report data buffer.
 * 128 is the max number of configuration items.
 * In addition, we need MAX_NUM_BACKENDS*4
 * for backend descriptions.
 */
#define MAXITEMS (128 + MAX_NUM_BACKENDS*4)		

	POOL_REPORT_STATUS status[MAXITEMS];

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
	strncpy(status[i].desc, "PgPool status file logging directory", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "log_destination", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->log_destination);
	strncpy(status[i].desc, "logging destination", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "syslog_facility", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "LOCAL%d", (pool_config->syslog_facility/8) - 16);
	strncpy(status[i].desc, "syslog local faclity", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "syslog_ident", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->syslog_ident);
	strncpy(status[i].desc, "syslog program ident string", POOLCONFIG_MAXDESCLEN);
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

	strncpy(status[i].name, "failover_if_affected_tuples_mismatch", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->failover_if_affected_tuples_mismatch);
	strncpy(status[i].desc, "failover if affected tuples are mismatch", POOLCONFIG_MAXDESCLEN);
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
		if (j != pool_config->num_reset_queries-1)
			strncat(status[i].value, ";", len);
	}
	strncpy(status[i].desc, "queries issued at the end of session", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "white_function_list", POOLCONFIG_MAXNAMELEN);
	*(status[i].value) = '\0';
	for (j=0;j<pool_config->num_white_function_list;j++)
	{
		int len;
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		strncat(status[i].value, pool_config->white_function_list[j], len);
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		if (j != pool_config->num_white_function_list-1)
			strncat(status[i].value, ",", len);
	}
	strncpy(status[i].desc, "functions those do not write to database", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "black_function_list", POOLCONFIG_MAXNAMELEN);
	*(status[i].value) = '\0';
	for (j=0;j<pool_config->num_black_function_list;j++)
	{
		int len;
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		strncat(status[i].value, pool_config->black_function_list[j], len);
		len = POOLCONFIG_MAXVALLEN - strlen(status[i].value);
		if (j != pool_config->num_black_function_list-1)
			strncat(status[i].value, ",", len);
	}
	strncpy(status[i].desc, "functions those write to database", POOLCONFIG_MAXDESCLEN);
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

	strncpy(status[i].name, "delay_threshold", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%lld", pool_config->delay_threshold);
	strncpy(status[i].desc, "standby delay threshold", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "log_standby_delay", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->log_standby_delay);
	strncpy(status[i].desc, "how to log standby delay", POOLCONFIG_MAXDESCLEN);
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
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%s", pool_config->failback_command);
	strncpy(status[i].desc, "failback command", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "fail_over_on_backend_error", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->fail_over_on_backend_error);
	strncpy(status[i].desc, "fail over on backend error", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "insert_lock", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->insert_lock);
	strncpy(status[i].desc, "insert lock", POOLCONFIG_MAXDESCLEN);
	i++;

	strncpy(status[i].name, "ignore_leading_white_space", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->ignore_leading_white_space);
	strncpy(status[i].desc, "ignore leading white spaces", POOLCONFIG_MAXDESCLEN);
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

	strncpy(status[i].name, "relcache_expire", POOLCONFIG_MAXNAMELEN);
	snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", pool_config->relcache_expire);
	strncpy(status[i].desc, "relation cache expiration time in seconds", POOLCONFIG_MAXDESCLEN);
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

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "backend status%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%d", BACKEND_INFO(j).backend_status);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "status of backend #%d", j);
		i++;

		snprintf(status[i].name, POOLCONFIG_MAXNAMELEN, "standby_delay%d", j);
		snprintf(status[i].value, POOLCONFIG_MAXVALLEN, "%lld", BACKEND_INFO(j).standby_delay);
		snprintf(status[i].desc, POOLCONFIG_MAXDESCLEN, "standby delay of backend #%d", j);
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

void nodes_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	static char *cursorname = "blank";
	static short num_fields = 5;
	static char *field_names[] = {"node_id","hostname", "port", "status", "lb_weight"};
	static int oid = 0;
	static short fsize = -1;
	static int mod = 0;
	short n;
	int i;
	short s;
	int len;
	short colnum;
	short nrows;
	int size;
	int hsize;

	static unsigned char nullmap[2] = {0xff, 0xff};
	int nbytes = (num_fields + 7)/8;

	POOL_REPORT_NODES nodes[NUM_BACKENDS];

	BackendInfo *bi = NULL;

    for (i = 0; i < NUM_BACKENDS; i++)
	{
	    bi = pool_get_node_info(i);

            snprintf(nodes[i].node_id, 	POOLCONFIG_MAXSTATLEN, 	"%d", 	i);
	    strncpy(nodes[i].hostname, 	bi->backend_hostname, 		strlen(bi->backend_hostname)+1);
	    snprintf(nodes[i].port, 	POOLCONFIG_MAXIDENTLEN, "%d", 	bi->backend_port);
	    snprintf(nodes[i].status, 	POOLCONFIG_MAXSTATLEN, 	"%d", 	bi->backend_status);
	    snprintf(nodes[i].lb_weight, POOLCONFIG_MAXWEIGHTLEN, "%f", bi->backend_weight/RAND_MAX);
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
		}
	}
	else
	{
		/* data row */
		for (i=0;i<nrows;i++)
		{
			pool_write(frontend, "D", 1);
			len = sizeof(len) + sizeof(nrows);
			len += sizeof(int) + strlen(nodes[i].node_id);
			len += sizeof(int) + strlen(nodes[i].hostname);
			len += sizeof(int) + strlen(nodes[i].port);
			len += sizeof(int) + strlen(nodes[i].status);
			len += sizeof(int) + strlen(nodes[i].lb_weight);
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

void pools_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	static char *cursorname = "blank";
	static short num_fields = 12;
	static char *field_names[] = {"pool_pid", "start_time", "pool_id", "backend_id", "database", "username", "create_time",
                                  "majorversion", "minorversion", "pool_counter", "pool_backendpid", "pool_connected"};
	static int oid = 0;
	static short fsize = -1;
	static int mod = 0;
	short n;
	int child, pool, poolBE, backend_id;
	short s;
	int len;
	short colnum;
    int i;

	static unsigned char nullmap[2] = {0xff, 0xff};
	int nbytes = (num_fields + 7)/8;

	POOL_REPORT_POOLS pools[pool_config->num_init_children * pool_config->max_pool * NUM_BACKENDS];

	short nrows;
	int size;
	int hsize;

    ProcessInfo *pi = NULL;
    int proc_id;

    int lines = 0;
	for (child = 0; child < pool_config->num_init_children; child++)
    {
		proc_id = process_info[child].pid;
	    pi = pool_get_process_info(proc_id);
    
        for (pool = 0; pool < pool_config->max_pool; pool++)
        {
			for (backend_id = 0; backend_id < NUM_BACKENDS; backend_id++)
			{
                poolBE = pool*MAX_NUM_BACKENDS+backend_id;
                snprintf(pools[lines].pool_pid, POOLCONFIG_MAXCOUNTLEN, "%d", proc_id);
	            strftime(pools[lines].start_time, POOLCONFIG_MAXDATELEN, "%Y-%m-%d %H:%M:%S", localtime(&pi->start_time));
                snprintf(pools[lines].pool_id, POOLCONFIG_MAXCOUNTLEN, "%d", pool);
                snprintf(pools[lines].backend_id, POOLCONFIG_MAXCOUNTLEN, "%d", backend_id);
                if (strlen(pi->connection_info[poolBE].database) == 0)
                {
                    strncpy(pools[lines].database, "", POOLCONFIG_MAXIDENTLEN);
	                strncpy(pools[lines].username, "", POOLCONFIG_MAXIDENTLEN);
	                strncpy(pools[lines].create_time, "", POOLCONFIG_MAXDATELEN);
                    strncpy(pools[lines].pool_majorversion, "", POOLCONFIG_MAXCOUNTLEN);
                    strncpy(pools[lines].pool_minorversion, "", POOLCONFIG_MAXCOUNTLEN);
                    strncpy(pools[lines].pool_counter, "", POOLCONFIG_MAXCOUNTLEN);
                    strncpy(pools[lines].pool_backendpid, "", POOLCONFIG_MAXCOUNTLEN);
                    strncpy(pools[lines].pool_connected, "", POOLCONFIG_MAXCOUNTLEN);
                }
                else
                {
                    strncpy(pools[lines].database, pi->connection_info[poolBE].database, POOLCONFIG_MAXIDENTLEN);
	                strncpy(pools[lines].username, pi->connection_info[poolBE].user, POOLCONFIG_MAXIDENTLEN);
	                strftime(pools[lines].create_time, POOLCONFIG_MAXDATELEN, "%Y-%m-%d %H:%M:%S", localtime(&pi->connection_info[poolBE].create_time));
                    snprintf(pools[lines].pool_majorversion, POOLCONFIG_MAXCOUNTLEN, "%d", pi->connection_info[poolBE].major);
                    snprintf(pools[lines].pool_minorversion, POOLCONFIG_MAXCOUNTLEN, "%d", pi->connection_info[poolBE].minor);
                    snprintf(pools[lines].pool_counter, POOLCONFIG_MAXCOUNTLEN, "%d", pi->connection_info[poolBE].counter);
                    snprintf(pools[lines].pool_backendpid, POOLCONFIG_MAXCOUNTLEN, "%d", ntohl(pi->connection_info[poolBE].pid));
                    snprintf(pools[lines].pool_connected, POOLCONFIG_MAXCOUNTLEN, "%d", pi->connection_info[poolBE].connected);
                }
                lines++;
            }
        }
    }

	nrows = lines;

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

			size = strlen(pools[i].pool_pid);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, pools[i].pool_pid, size);

			size = strlen(pools[i].start_time);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, pools[i].start_time, size);

			size = strlen(pools[i].pool_id);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, pools[i].pool_id, size);

			size = strlen(pools[i].backend_id);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, pools[i].backend_id, size);

			size = strlen(pools[i].database);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, pools[i].database, size);

			size = strlen(pools[i].username);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, pools[i].username, size);

			size = strlen(pools[i].create_time);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, pools[i].create_time, size);

			size = strlen(pools[i].pool_majorversion);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, pools[i].pool_majorversion, size);

			size = strlen(pools[i].pool_minorversion);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, pools[i].pool_minorversion, size);

			size = strlen(pools[i].pool_counter);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, pools[i].pool_counter, size);

			size = strlen(pools[i].pool_backendpid);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, pools[i].pool_backendpid, size);

			size = strlen(pools[i].pool_connected);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, pools[i].pool_connected, size);
		}
	}
	else
	{
		/* data row */
		for (i=0;i<nrows;i++)
		{
			pool_write(frontend, "D", 1);
			len = sizeof(len) + sizeof(nrows);
			len += sizeof(int) + strlen(pools[i].pool_pid);
			len += sizeof(int) + strlen(pools[i].start_time);
			len += sizeof(int) + strlen(pools[i].pool_id);
			len += sizeof(int) + strlen(pools[i].backend_id);
			len += sizeof(int) + strlen(pools[i].database);
			len += sizeof(int) + strlen(pools[i].username);
			len += sizeof(int) + strlen(pools[i].create_time);
			len += sizeof(int) + strlen(pools[i].pool_majorversion);
			len += sizeof(int) + strlen(pools[i].pool_minorversion);
			len += sizeof(int) + strlen(pools[i].pool_counter);
			len += sizeof(int) + strlen(pools[i].pool_backendpid);
			len += sizeof(int) + strlen(pools[i].pool_connected);
			len = htonl(len);
			pool_write(frontend, &len, sizeof(len));
			s = htons(num_fields);
			pool_write(frontend, &s, sizeof(s));

			len = htonl(strlen(pools[i].pool_pid));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, pools[i].pool_pid, strlen(pools[i].pool_pid));

			len = htonl(strlen(pools[i].start_time));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, pools[i].start_time, strlen(pools[i].start_time));

			len = htonl(strlen(pools[i].pool_id));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, pools[i].pool_id, strlen(pools[i].pool_id));

			len = htonl(strlen(pools[i].backend_id));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, pools[i].backend_id, strlen(pools[i].backend_id));

			len = htonl(strlen(pools[i].database));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, pools[i].database, strlen(pools[i].database));

			len = htonl(strlen(pools[i].username));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, pools[i].username, strlen(pools[i].username));

			len = htonl(strlen(pools[i].create_time));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, pools[i].create_time, strlen(pools[i].create_time));

			len = htonl(strlen(pools[i].pool_majorversion));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, pools[i].pool_majorversion, strlen(pools[i].pool_majorversion));

			len = htonl(strlen(pools[i].pool_minorversion));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, pools[i].pool_minorversion, strlen(pools[i].pool_minorversion));

			len = htonl(strlen(pools[i].pool_counter));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, pools[i].pool_counter, strlen(pools[i].pool_counter));

			len = htonl(strlen(pools[i].pool_backendpid));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, pools[i].pool_backendpid, strlen(pools[i].pool_backendpid));

			len = htonl(strlen(pools[i].pool_connected));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, pools[i].pool_connected, strlen(pools[i].pool_connected));
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

void processes_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	static char *cursorname = "blank";
	static short num_fields = 6;
	static char *field_names[] = {"pool_pid", "start_time", "database", "username", "create_time", "pool_counter"};
	static int oid = 0;
	static short fsize = -1;
	static int mod = 0;
	short n;
	int child;
    int pool;
    int poolBE;
	short s;
	int len;
	short colnum;
	POOL_REPORT_PROCESSES processes[pool_config->num_init_children];
	short nrows;
	int size;
	int hsize;
    int i;

	static unsigned char nullmap[2] = {0xff, 0xff};
	int nbytes = (num_fields + 7)/8;

    ProcessInfo *pi = NULL;
    int proc_id;

	for (child = 0; child < pool_config->num_init_children; child++)
    {
		proc_id = process_info[child].pid;
	    pi = pool_get_process_info(proc_id);
    
        snprintf(processes[child].pool_pid, POOLCONFIG_MAXCOUNTLEN, "%d", proc_id);
	    strftime(processes[child].start_time, POOLCONFIG_MAXDATELEN, "%Y-%m-%d %H:%M:%S", localtime(&pi->start_time));
	    strncpy(processes[child].database, "", POOLCONFIG_MAXIDENTLEN);
	    strncpy(processes[child].username, "", POOLCONFIG_MAXIDENTLEN);
        strncpy(processes[child].create_time, "", POOLCONFIG_MAXDATELEN);
        strncpy(processes[child].pool_counter, "", POOLCONFIG_MAXCOUNTLEN);

        for (pool = 0; pool < pool_config->max_pool; pool++)
        {
            poolBE = pool*MAX_NUM_BACKENDS;
            if (pi->connection_info[poolBE].connected && strlen(pi->connection_info[poolBE].database) > 0 && strlen(pi->connection_info[poolBE].user) > 0)
            {
	            strncpy(processes[child].database, pi->connection_info[poolBE].database, POOLCONFIG_MAXIDENTLEN);
	            strncpy(processes[child].username, pi->connection_info[poolBE].user, POOLCONFIG_MAXIDENTLEN);
	            strftime(processes[child].create_time, POOLCONFIG_MAXDATELEN, "%Y-%m-%d %H:%M:%S", localtime(&pi->connection_info[poolBE].create_time));
                snprintf(processes[child].pool_counter, POOLCONFIG_MAXCOUNTLEN, "%d", pi->connection_info[poolBE].counter);
            }
        }
    }

	nrows = child;

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
			len = sizeof(len) + sizeof(nrows);
			len += sizeof(int) + strlen(processes[i].pool_pid);
			len += sizeof(int) + strlen(processes[i].start_time);
			len += sizeof(int) + strlen(processes[i].database);
			len += sizeof(int) + strlen(processes[i].username);
			len += sizeof(int) + strlen(processes[i].create_time);
			len += sizeof(int) + strlen(processes[i].pool_counter);
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

void version_reporting(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	static char *cursorname = "blank";
	static short num_fields = 1;
	static char *field_names[] = {"pool_version"};
	static int oid = 0;
	static short fsize = -1;
	static int mod = 0;
	short n;
	int i;
	short s;
	int len;
	short colnum;
	short nrows;
	int size;
	int hsize;

	static unsigned char nullmap[2] = {0xff, 0xff};
	int nbytes = (num_fields + 7)/8;

	POOL_REPORT_VERSION version[1];

	snprintf(version[0].version, POOLCONFIG_MAXVALLEN, "%s (%s)", VERSION, PGPOOLVERSION);

	nrows = 1;

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

			size = strlen(version[i].version);
			hsize = htonl(size+4);
			pool_write(frontend, &hsize, sizeof(hsize));
			pool_write(frontend, version[i].version, size);
		}
	}
	else
	{
		/* data row */
		for (i=0;i<nrows;i++)
		{
			pool_write(frontend, "D", 1);
			len = sizeof(len) + sizeof(nrows);
			len += sizeof(int) + strlen(version[i].version);
			len = htonl(len);
			pool_write(frontend, &len, sizeof(len));
			s = htons(num_fields);
			pool_write(frontend, &s, sizeof(s));

			len = htonl(strlen(version[i].version));
			pool_write(frontend, &len, sizeof(len));
			pool_write(frontend, version[i].version, strlen(version[i].version));
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

