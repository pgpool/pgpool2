#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "pool.h"
#include "pool_config.h"
#include "pool_config_variables.h"
#include "utils/regex_array.h"

#ifndef POOL_PRIVATE
#include "utils/elog.h"
#include "parser/stringinfo.h"
#include "utils/pool_process_reporting.h"
#include "utils/pool_stream.h"
#else
#include "utils/fe_ports.h"
#endif

#define default_reset_query_list	"ABORT;DISCARD ALL"
#define default_black_function_list "nextval,setval"

extern POOL_CONFIG g_pool_config;
struct config_generic **all_parameters = NULL;
static int num_all_parameters = 0;

static void initialize_variables_with_default(struct config_generic * gconf);
static bool config_enum_lookup_by_name(struct config_enum * record, const char *value, int *retval);

static void build_variable_groups(void);
static void build_config_variables(void);

static struct config_generic *find_option(const char *name, int elevel);

static bool config_post_processor(ConfigContext context, int elevel);

static void sort_config_vars(void);
static bool setConfigOption(const char *name, const char *value,
							ConfigContext context, GucSource source, int elevel);
static bool setConfigOptionVar(struct config_generic *record, const char *name, int index_val,
				const char *value, ConfigContext context, GucSource source, int elevel);

static bool get_index_in_var_name(struct config_generic* record,
								  const char* name, int *index, int elevel);


static bool MakeDBRedirectListRegex (char* newval, int elevel);
static bool MakeAppRedirectListRegex (char* newval, int elevel);
static bool check_redirect_node_spec(char *node_spec);
static char **get_list_from_string(const char *str, const char *delimi, int *n);


/*show functions */
static const char* HBDestinationPortShowFunc(int index);
static const char* HBDestinationShowFunc(int index);
static const char* HBDeviceShowFunc(int index);
static const char* OtherWDPortShowFunc(int index);
static const char* OtherPPPortShowFunc(int index);
static const char* OtherPPHostShowFunc(int index);
static const char* BackendFlagsShowFunc(int index);
static const char* BackendDataDirShowFunc(int index);
static const char* BackendHostShowFunc(int index);
static const char* BackendPortShowFunc(int index);
static const char* BackendWeightShowFunc(int index);
/* check empty slot functions */
static bool WdIFSlotEmptyCheckFunc(int index);
static bool WdSlotEmptyCheckFunc(int index);
static bool BackendSlotEmptyCheckFunc(int index);
/*variable custom assign functions */
static bool BackendPortAssignFunc (ConfigContext context, int newval, int index, int elevel);
static bool BackendHostAssignFunc (ConfigContext context, char* newval, int index, int elevel);
static bool BackendDataDirAssignFunc (ConfigContext context, char* newval, int index, int elevel);
static bool BackendFlagsAssignFunc (ConfigContext context, char* newval, int index, int elevel);
static bool BackendWeightAssignFunc (ConfigContext context, double newval, int index, int elevel);
static bool HBDestinationAssignFunc (ConfigContext context, char* newval, int index, int elevel);
static bool HBDestinationPortAssignFunc (ConfigContext context, int newval, int index, int elevel);
static bool HBDeviceAssignFunc (ConfigContext context, char* newval, int index, int elevel);
static bool OtherWDPortAssignFunc (ConfigContext context, int newval, int index, int elevel);
static bool OtherPPPortAssignFunc (ConfigContext context, int newval, int index, int elevel);
static bool OtherPPHostAssignFunc (ConfigContext context, char* newval, int index, int elevel);

static bool LogDestinationProcessFunc (char* newval, int elevel);
static bool SyslogIdentProcessFunc (char* newval, int elevel);
static bool SyslogFacilityProcessFunc (int newval, int elevel);


#ifndef POOL_PRIVATE
/* These functions are used to provide Hints for enum type config parameters and
 * to output the values of the parameters.
 * These functuons are not available for tools since they use the stringInfo that is
 * not present for tools.
 */

static const char* config_enum_lookup_by_value(struct config_enum * record, int val);

static char *ShowOption(struct config_generic * record, int index, int elevel);
static int get_max_elements_for_config_record(struct config_generic* record);

static char *config_enum_get_options(struct config_enum * record, const char *prefix,
						const char *suffix, const char *separator);
static void send_row_description_for_detail_view(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
static int send_grouped_type_variable_to_frontend(struct config_grouped_array_var* grouped_record,
												  POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
static int send_array_type_variable_to_frontend(struct config_generic* record,
												POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);

#endif


static const struct config_enum_entry log_error_verbosity_options[] = {
	{"terse", PGERROR_TERSE, false},
	{"default", PGERROR_DEFAULT, false},
	{"verbose", PGERROR_VERBOSE, false},
	{NULL, 0, false}
};
static const struct config_enum_entry server_message_level_options[] = {
	{"debug", DEBUG2, true},
	{"debug5", DEBUG5, false},
	{"debug4", DEBUG4, false},
	{"debug3", DEBUG3, false},
	{"debug2", DEBUG2, false},
	{"debug1", DEBUG1, false},
	{"info", INFO, false},
	{"notice", NOTICE, false},
	{"warning", WARNING, false},
	{"error", ERROR, false},
	{"log", LOG, false},
	{"fatal", FATAL, false},
	{"panic", PANIC, false},
	{NULL, 0, false}
};


static const struct config_enum_entry master_slave_sub_mode_options[] = {
	{"slony", SLONY_MODE, false},
	{"stream", STREAM_MODE, false},
	{NULL, 0, false}
};


static const struct config_enum_entry log_standby_delay_options[] = {
	{"always", LSD_ALWAYS, false},
	{"if_over_threshold", LSD_OVER_THRESHOLD, false},
	{"none", LSD_NONE, false},
	{NULL, 0, false}
};

static const struct config_enum_entry memqcache_method_options[] = {
	{"shmem", SHMEM_CACHE, false},
	{"memcached", MEMCACHED_CACHE, false},
	{NULL, 0, false}
};

static const struct config_enum_entry wd_lifecheck_method_options[] = {
	{"query", LIFECHECK_BY_QUERY, false},
	{"heartbeat", LIFECHECK_BY_HB, false},
	{"external", LIFECHECK_BY_EXTERNAL, false},
	{NULL, 0, false}
};

static const struct config_enum_entry syslog_facility_options[] = {
	{"LOCAL0", LOG_LOCAL0, false},
	{"LOCAL1", LOG_LOCAL1, false},
	{"LOCAL2", LOG_LOCAL2, false},
	{"LOCAL3", LOG_LOCAL3, false},
	{"LOCAL4", LOG_LOCAL4, false},
	{"LOCAL5", LOG_LOCAL5, false},
	{"LOCAL6", LOG_LOCAL6, false},
	{"LOCAL7", LOG_LOCAL7, false},
	{NULL, 0, false}
};


static struct config_bool ConfigureNamesBool[] =
{
	{
		{"serialize_accept", CFGCXT_INIT, CONNECTION_CONFIG,
			"whether to serialize accept() call to avoid thundering herd problem",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.serialize_accept,	/* variable */
		false,								/* boot value */
		NULL,								/* assign func */
		NULL,								/* check func */
		NULL								/* show hook */
	},

	{
		{"log_connections", CFGCXT_RELOAD, LOGING_CONFIG,
			"Logs each successful connection.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.log_connections,
		false,
		NULL, NULL,NULL
	},

	{
		{"log_hostname", CFGCXT_RELOAD, LOGING_CONFIG,
			"Logs the host name in the connection logs.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.log_hostname,
		false,
		NULL, NULL,NULL
	},

	{
		{"enable_pool_hba", CFGCXT_RELOAD, CONNECTION_CONFIG,
			"Use pool_hba.conf for client authentication.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.enable_pool_hba,
		false,
		NULL, NULL,NULL
	},

	{
		{"replication_mode", CFGCXT_INIT, LOGING_CONFIG,
			"Enables replication mode.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.replication_mode,
		false,
		NULL, NULL,NULL
	},

	{
		{"load_balance_mode", CFGCXT_INIT, LOAD_BALANCE_CONFIG,
			"Enables load balancing of queries.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.load_balance_mode,
		false,
		NULL, NULL,NULL
	},

	{
		{"replication_stop_on_mismatch", CFGCXT_RELOAD, REPLICATION_CONFIG,
			"Starts degeneration and stops replication, If there's a data mismatch between master and secondary.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.replication_stop_on_mismatch,
		false,
		NULL, NULL,NULL
	},

	{
		{"failover_if_affected_tuples_mismatch", CFGCXT_RELOAD, REPLICATION_CONFIG,
			"Starts degeneration, If there's a data mismatch between master and secondary.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.failover_if_affected_tuples_mismatch,
		false,
		NULL, NULL,NULL
	},

	{
		{"replicate_select", CFGCXT_RELOAD, REPLICATION_CONFIG,
			"Replicate SELECT statements when load balancing is disabled.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.replicate_select,
		false,
		NULL, NULL,NULL
	},
	
	{
		{"master_slave_mode", CFGCXT_INIT, MASTER_SLAVE_CONFIG,
			"Enables Master/Slave mode.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.master_slave_mode,
		false,
		NULL, NULL,NULL
	},
	
	{
		{"connection_cache", CFGCXT_INIT, CONNECTION_POOL_CONFIG,
			"Caches connections to backends.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.connection_cache,
		true,
		NULL, NULL,NULL
	},

	{
		{"fail_over_on_backend_error", CFGCXT_RELOAD, FAILOVER_CONFIG,
			"Triggers fail over when reading/writing to backend socket fails.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.fail_over_on_backend_error,
		true,
		NULL, NULL,NULL
	},
	
	{
		{"insert_lock", CFGCXT_RELOAD, REPLICATION_CONFIG,
			"Automatically locks table with INSERT to keep SERIAL data consistency",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.insert_lock,
		true,
		NULL, NULL,NULL
	},
	
	{
		{"ignore_leading_white_space", CFGCXT_RELOAD, LOAD_BALANCE_CONFIG,
			"Ignores leading white spaces of each query string.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.ignore_leading_white_space,
		true,
		NULL, NULL,NULL
	},

	{
		{"log_statement", CFGCXT_SESSION, LOGING_CONFIG,
			"Logs all statements in the pgpool logs.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.log_statement,
		false,
		NULL, NULL,NULL
	},

	{
		{"log_per_node_statement", CFGCXT_SESSION, LOGING_CONFIG,
			"Logs per node detailed SQL statements.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.log_per_node_statement,
		false,
		NULL, NULL,NULL
	},

	{
		{"use_watchdog", CFGCXT_INIT, WATCHDOG_CONFIG,
			"Enables the pgpool-II watchdog.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.use_watchdog,
		false,
		NULL, NULL,NULL
	},

	{
		{"clear_memqcache_on_escalation", CFGCXT_RELOAD, WATCHDOG_CONFIG,
			"Clears the query cache in the shared memory when pgpool-II escaltes to master watchdog node.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.clear_memqcache_on_escalation,
		false,
		NULL, NULL,NULL
	},

	{
		{"ssl", CFGCXT_INIT, SSL_CONFIG,
			"Enables SSL support for frontend and backend connections",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.ssl,
		false,
		NULL, NULL,NULL
	},

	{
		{"ssl_prefer_server_ciphers", CFGCXT_INIT, SSL_CONFIG,
			"Use server's SSL cipher preferences, rather than the client's",
			CONFIG_VAR_TYPE_BOOL, false, 0
		},
		&g_pool_config.ssl_prefer_server_ciphers,
		false,
		NULL, NULL, NULL
	},

	{
		{"check_temp_table", CFGCXT_SESSION, GENERAL_CONFIG,
			"Enables temporary table check.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.check_temp_table,
		true,
		NULL, NULL,NULL
	},
	
	{
		{"check_unlogged_table", CFGCXT_SESSION, GENERAL_CONFIG,
			"Enables unlogged table check.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.check_unlogged_table,
		true,
		NULL, NULL,NULL
	},
	
	{
		{"memory_cache_enabled", CFGCXT_INIT, CACHE_CONFIG,
			"Enables the memory cache functionality.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.memory_cache_enabled,
		false,
		NULL, NULL,NULL
	},

	{
		{"memqcache_auto_cache_invalidation", CFGCXT_RELOAD, CACHE_CONFIG,
			"Automatically deletes the cache related to the updated tables.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.memqcache_auto_cache_invalidation,
		true,
		NULL, NULL,NULL
	},

	{
		{"allow_sql_comments", CFGCXT_SESSION, LOAD_BALANCE_CONFIG,
			"Ignore SQL comments, while judging if load balance or query cache is possible.",
			CONFIG_VAR_TYPE_BOOL,false, 0
		},
		&g_pool_config.allow_sql_comments,
		false,
		NULL, NULL,NULL
	},

	/* End-of-list marker */
	{
		{NULL, 0, 0, NULL}, NULL, false, NULL, NULL,NULL
	}

};

static struct config_string ConfigureNamesString[] =
{
	{
		{"database_redirect_preference_list", CFGCXT_RELOAD, STREAMING_REPLICATION_CONFIG,
			"redirect by database name.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.database_redirect_preference_list, /* variable */
		NULL,/* boot value */
		NULL,/* assign_func */
		NULL,/* check_func */
		MakeDBRedirectListRegex, /* process func */
		NULL	/* show hook */
	},

	{
		{"app_name_redirect_preference_list", CFGCXT_RELOAD, STREAMING_REPLICATION_CONFIG,
			"redirect by application name.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.app_name_redirect_preference_list,
		NULL,
		NULL,NULL,
		MakeAppRedirectListRegex, NULL
	},

	{
		{"listen_addresses", CFGCXT_INIT, CONNECTION_CONFIG,
			"hostname or IP address on which pgpool will listen on.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.listen_addresses,
		"localhost",
		NULL, NULL, NULL, NULL
	},

	{
		{"pcp_listen_addresses", CFGCXT_INIT, CONNECTION_CONFIG,
			"hostname(s) or IP address(es) on which pcp will listen on.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.pcp_listen_addresses,
		"*",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"socket_dir", CFGCXT_INIT, CONNECTION_CONFIG,
			"The directory to create the UNIX domain socket for accepting pgpool-II client connections.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.socket_dir,
		DEFAULT_SOCKET_DIR,
		NULL, NULL, NULL, NULL
	},

	{
		{"pcp_socket_dir", CFGCXT_INIT, CONNECTION_CONFIG,
			"The directory to create the UNIX domain socket for accepting pgpool-II PCP connections.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.pcp_socket_dir,
		DEFAULT_SOCKET_DIR,
		NULL, NULL, NULL, NULL
	},

	{
		{"wd_ipc_socket_dir", CFGCXT_INIT, CONNECTION_CONFIG,
		"The directory to create the UNIX domain socket for accepting pgpool-II watchdog IPC connections.",
		CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.wd_ipc_socket_dir,
		DEFAULT_SOCKET_DIR,
		NULL, NULL, NULL, NULL
	},

	{
		{"log_destination", CFGCXT_RELOAD, LOGING_CONFIG,
			"destination of pgpool-II log",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.log_destination_str,
		"stderr",
		NULL, NULL,
		LogDestinationProcessFunc,
		NULL
	},

	{
		{"syslog_ident", CFGCXT_RELOAD, LOGING_CONFIG,
			"syslog program ident string.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.syslog_ident,
		"pgpool",
		NULL, NULL,
		SyslogIdentProcessFunc,
		NULL
	},

	{
		{"pid_file_name", CFGCXT_INIT, FILE_LOCATION_CONFIG,
			"Path to store pgpool-II pid file.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.pid_file_name,
		DEFAULT_PID_FILE_NAME,
		NULL, NULL, NULL, NULL
	},

	{
		{"pool_passwd", CFGCXT_INIT, FILE_LOCATION_CONFIG,
			"File name of pool_passwd for md5 authentication.",
			CONFIG_VAR_TYPE_STRING,false, VAR_HIDDEN_VALUE
		},
		&g_pool_config.pool_passwd,
		"pool_passwd",
		NULL, NULL, NULL, NULL
	},

	{
		{"log_line_prefix", CFGCXT_RELOAD, LOGING_CONFIG,
			"printf-style string to output at beginning of each log line.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.log_line_prefix,
		"%t: pid %p: ",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"health_check_user", CFGCXT_RELOAD, HEALTH_CHECK_CONFIG,
			"User name for PostgreSQL backend health check.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.health_check_user,
		"nobody",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"health_check_password", CFGCXT_RELOAD, HEALTH_CHECK_CONFIG,
			"Password for PostgreSQL backend health check database user.",
			CONFIG_VAR_TYPE_STRING,false, VAR_HIDDEN_VALUE
		},
		&g_pool_config.health_check_password,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"health_check_database", CFGCXT_RELOAD, HEALTH_CHECK_CONFIG,
			"The database name to be used to perform PostgreSQL backend health check.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.health_check_database,
		"postgres",
		NULL, NULL, NULL, NULL
	},

	
	{
		{"sr_check_user", CFGCXT_RELOAD, STREAMING_REPLICATION_CONFIG,
			"The User name to perform streaming replication delay check.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.sr_check_user,
		"nobody",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"sr_check_password", CFGCXT_RELOAD, STREAMING_REPLICATION_CONFIG,
			"The password for user to perform streaming replication delay check.",
			CONFIG_VAR_TYPE_STRING,false, VAR_HIDDEN_VALUE
		},
		&g_pool_config.sr_check_password,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"sr_check_database", CFGCXT_RELOAD, STREAMING_REPLICATION_CONFIG,
			"The database name to perform streaming replication delay check.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.sr_check_database,
		"postgres",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"failback_command", CFGCXT_RELOAD, FAILOVER_CONFIG,
			"Command to execute when backend node is attached.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.failback_command,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"follow_master_command", CFGCXT_RELOAD, FAILOVER_CONFIG,
			"Command to execute in master/slave streaming replication mode after a master node failover.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.follow_master_command,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"failover_command", CFGCXT_RELOAD, FAILOVER_CONFIG,
			"Command to execute when backend node is detached.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.failover_command,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"recovery_user", CFGCXT_RELOAD, RECOVERY_CONFIG,
			"User name for online recovery.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.recovery_user,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"recovery_password", CFGCXT_RELOAD, RECOVERY_CONFIG,
			"Password for online recovery.",
			CONFIG_VAR_TYPE_STRING,false, VAR_HIDDEN_VALUE
		},
		&g_pool_config.recovery_password,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"recovery_1st_stage_command", CFGCXT_RELOAD, RECOVERY_CONFIG,
			"Command to execute in first stage recovery.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.recovery_1st_stage_command,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"recovery_2nd_stage_command", CFGCXT_RELOAD, RECOVERY_CONFIG,
			"Command to execute in second stage recovery.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.recovery_2nd_stage_command,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"lobj_lock_table", CFGCXT_RELOAD, REPLICATION_CONFIG,
			"Table name used for large object replication control.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.lobj_lock_table,
		"",
		NULL, NULL, NULL, NULL
	},

	{
		{"wd_escalation_command", CFGCXT_RELOAD, WATCHDOG_CONFIG,
			"Command to execute when watchdog node becomes cluster master/leader node.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.wd_escalation_command,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"wd_de_escalation_command", CFGCXT_RELOAD, WATCHDOG_CONFIG,
			"Command to execute when watchdog node resigns from the cluster master/leader node.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.wd_de_escalation_command,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"trusted_servers", CFGCXT_INIT, WATCHDOG_CONFIG,
			"List of servers to verify connectivity.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.trusted_servers,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"delegate_IP", CFGCXT_INIT, WATCHDOG_CONFIG,
			"Delegate IP address to be used when pgpool node become a watchdog cluster master/leader.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.delegate_IP,
		"",
		NULL, NULL, NULL, NULL
	},

	{
		{"wd_hostname", CFGCXT_INIT, WATCHDOG_CONFIG,
			"Host name or IP address of this watchdog.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.wd_hostname,
		"",
		NULL, NULL, NULL, NULL
	},

	{
		{"ping_path", CFGCXT_INIT, WATCHDOG_CONFIG,
			"path to ping command.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.ping_path,
		"/bin",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"if_cmd_path", CFGCXT_INIT, WATCHDOG_CONFIG,
			"Path to interface command.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.if_cmd_path,
		"/sbin",
		NULL, NULL, NULL, NULL
	},

	{
		{"if_up_cmd", CFGCXT_INIT, WATCHDOG_CONFIG,
			"Complete command to bring UP virtual interface.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.if_up_cmd,
		"ip addr add $_IP_$/24 dev eth0 label eth0:0",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"if_down_cmd", CFGCXT_INIT, WATCHDOG_CONFIG,
			"Complete command to bring down virtual interface.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.if_down_cmd,
		"ip addr del $_IP_$/24 dev eth0",
		NULL, NULL, NULL, NULL
	},

	{
		{"arping_path", CFGCXT_INIT, WATCHDOG_CONFIG,
			"path to arping command.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.arping_path,
		"/usr/sbin",
		NULL, NULL, NULL, NULL
	},

	{
		{"arping_cmd", CFGCXT_INIT, WATCHDOG_CONFIG,
			"arping command.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.arping_cmd,
		"arping -U $_IP_$ -w 1",
		NULL, NULL, NULL, NULL
	},

	{
		{"wd_lifecheck_query", CFGCXT_INIT, WATCHDOG_CONFIG,
			"SQL query to be used by watchdog lifecheck.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.wd_lifecheck_query,
		"SELECT 1",
		NULL, NULL, NULL, NULL
	},

	{
		{"wd_lifecheck_dbname", CFGCXT_RELOAD, WATCHDOG_CONFIG,
			"Database name to be used for by watchdog lifecheck.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.wd_lifecheck_dbname,
		"postgres",
		NULL, NULL, NULL, NULL
	},

	{
		{"wd_lifecheck_user", CFGCXT_RELOAD, WATCHDOG_CONFIG,
			"User name to be used for by watchdog lifecheck.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.wd_lifecheck_user,
		"nobody",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"wd_lifecheck_password", CFGCXT_RELOAD, WATCHDOG_CONFIG,
			"Password for watchdog user in lifecheck.",
			CONFIG_VAR_TYPE_STRING,false, VAR_HIDDEN_VALUE
		},
		&g_pool_config.wd_lifecheck_password,
		"postgres",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"wd_authkey", CFGCXT_RELOAD, WATCHDOG_CONFIG,
			"Authentication key to be used in watchdog communication.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.wd_authkey,
		"",
		NULL, NULL, NULL, NULL
	},

	{
		{"ssl_cert", CFGCXT_INIT, SSL_CONFIG,
			"Path to the SSL public certificate file.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.ssl_cert,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"ssl_key", CFGCXT_INIT, SSL_CONFIG,
			"Path to the SSL private key file.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.ssl_key,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"ssl_ca_cert", CFGCXT_INIT, SSL_CONFIG,
			"Path to a single PEM format file.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.ssl_ca_cert,
		"",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"ssl_ca_cert_dir", CFGCXT_INIT, SSL_CONFIG,
			"Directory containing CA root certificate(s).",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.ssl_ca_cert_dir,
		"",
		NULL, NULL, NULL, NULL
	},

	{
		{"ssl_ciphers", CFGCXT_INIT, SSL_CONFIG,
			"Allowed SSL ciphers.",
			CONFIG_VAR_TYPE_STRING, false, 0
		},
		&g_pool_config.ssl_ciphers,
		"HIGH:MEDIUM:+3DES:!aNULL",
		NULL, NULL, NULL, NULL
	},

	{
		{"memqcache_oiddir", CFGCXT_INIT, CACHE_CONFIG,
			"Tempory directory to record table oids.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.memqcache_oiddir,
		"/var/log/pgpool/oiddir",
		NULL, NULL, NULL, NULL
	},

	{
		{"memqcache_memcached_host", CFGCXT_INIT, CACHE_CONFIG,
			"Hostname or IP address of memcached.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.memqcache_memcached_host,
		"localhost",
		NULL, NULL, NULL, NULL
	},
	
	{
		{"logdir", CFGCXT_INIT, LOGING_CONFIG,
			"PgPool status file logging directory.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.logdir,
		DEFAULT_LOGDIR,
		NULL, NULL, NULL, NULL
	},

	/* End-of-list marker */
	{
		{NULL, 0, 0, NULL}, NULL, NULL, NULL, NULL, NULL, NULL
	}
};

static struct config_string_list ConfigureNamesStringList[] =
{
	{
		{"reset_query_list", CFGCXT_RELOAD, CONNECTION_POOL_CONFIG,
			"list of commands sent to reset the backend connection when user session exits.",
			CONFIG_VAR_TYPE_STRING_LIST,false, 0
		},
		&g_pool_config.reset_query_list,		/* variable */
		&g_pool_config.num_reset_queries,		/* item count var  */
		(const char*)default_reset_query_list,	/* boot value */
		";",/* token seperator */
		false,									/* compute_regex ?*/
		NULL, NULL, NULL						/* assign, check, show funcs */
	},

	{
		{"white_function_list", CFGCXT_RELOAD, CONNECTION_POOL_CONFIG,
			"list of functions that does not writes to database.",
			CONFIG_VAR_TYPE_STRING_LIST,false, 0
		},
		&g_pool_config.white_function_list,
		&g_pool_config.num_white_function_list,
		NULL,
		",",
		true,
		NULL, NULL, NULL
	},

	{
		{"black_function_list", CFGCXT_RELOAD, CONNECTION_POOL_CONFIG,
			"list of functions that writes to database.",
			CONFIG_VAR_TYPE_STRING_LIST,false, 0
		},
		&g_pool_config.black_function_list,
		&g_pool_config.num_black_function_list,
		(const char*)default_black_function_list,
		",",
		true,
		NULL, NULL, NULL
	},
	{
		{"white_memqcache_table_list", CFGCXT_RELOAD, CACHE_CONFIG,
			"list of tables to be cached.",
			CONFIG_VAR_TYPE_STRING_LIST,false, 0
		},
		&g_pool_config.white_memqcache_table_list,
		&g_pool_config.num_white_memqcache_table_list,
		NULL,
		",",
		true,
		NULL, NULL, NULL
	},

	{
		{"black_memqcache_table_list", CFGCXT_RELOAD, CACHE_CONFIG,
			"list of tables should not be cached.",
			CONFIG_VAR_TYPE_STRING_LIST,false, 0
		},
		&g_pool_config.black_memqcache_table_list,
		&g_pool_config.num_black_memqcache_table_list,
		NULL,
		",",
		true,
		NULL, NULL, NULL
	},

	{
		{"wd_monitoring_interfaces_list", CFGCXT_INIT, WATCHDOG_CONFIG,
			"List of network device names, to be monitored by the watchdog process for the network link state.",
			CONFIG_VAR_TYPE_STRING,false, 0
		},
		&g_pool_config.wd_monitoring_interfaces_list,
		&g_pool_config.num_wd_monitoring_interfaces_list,
		NULL,
		",",
		false,
		NULL, NULL, NULL
	},

	/* End-of-list marker */
	{
		{NULL, 0, 0, NULL}, NULL, NULL, NULL,NULL,false, NULL, NULL, NULL
	}
};

/* long configs*/
static struct config_long ConfigureNamesLong[] =
{
	{
		{"delay_threshold", CFGCXT_RELOAD, STREAMING_REPLICATION_CONFIG,
			"standby delay threshold.",
			CONFIG_VAR_TYPE_LONG,false, 0
		},
		&g_pool_config.delay_threshold,
		0,
		0,LONG_MAX,
		NULL, NULL, NULL
	},

	{
		{"relcache_expire", CFGCXT_INIT, CACHE_CONFIG,
		"Relation cache expiration time in seconds.",
			CONFIG_VAR_TYPE_LONG,false, 0
		},
		&g_pool_config.relcache_expire,
		0,
		0,LONG_MAX,
		NULL, NULL, NULL
	},

	{
		{"memqcache_total_size", CFGCXT_INIT, CACHE_CONFIG,
			"Total memory size in bytes for storing memory cache.",
			CONFIG_VAR_TYPE_LONG,false, 0
		},
		&g_pool_config.memqcache_total_size,
		(int64)67108864,
		0,LONG_MAX,
		NULL, NULL, NULL
	},

	/* End-of-list marker */
	{
		{NULL, 0, 0, NULL}, NULL, 0, 0, 0, NULL, NULL, NULL
	}
};


static struct config_int_array ConfigureNamesIntArray[] =
{
	{
		{"backend_port", CFGCXT_RELOAD, CONNECTION_CONFIG,
			"port number of PostgreSQL backend.",
			CONFIG_VAR_TYPE_INT_ARRAY,true, 0
		},
		NULL,
		0,
		1024,65535,
		MAX_NUM_BACKENDS,
		BackendPortAssignFunc, NULL, BackendPortShowFunc, BackendSlotEmptyCheckFunc
	},

	{
		{"heartbeat_destination_port", CFGCXT_RELOAD, WATCHDOG_LIFECHECK,
			"Destination port for sending heartbeat.",
			CONFIG_VAR_TYPE_INT_ARRAY,true, 0
		},
		NULL,
		0,
		1024,65535,
		WD_MAX_IF_NUM,
		HBDestinationPortAssignFunc, NULL, HBDestinationPortShowFunc, WdIFSlotEmptyCheckFunc
	},

	{
		{"other_wd_port", CFGCXT_RELOAD, WATCHDOG_CONFIG,
			"tcp/ip watchdog port number of other pgpool node for watchdog connection..",
			CONFIG_VAR_TYPE_INT_ARRAY,true, 0
		},
		NULL,
		0,
		1024,65535,
		MAX_WATCHDOG_NUM,
		OtherWDPortAssignFunc, NULL, OtherWDPortShowFunc,WdSlotEmptyCheckFunc
	},

	{
		{"other_pgpool_port", CFGCXT_RELOAD, WATCHDOG_CONFIG,
			"tcp/ip pgpool port number of other pgpool node for watchdog connection.",
			CONFIG_VAR_TYPE_INT_ARRAY,true, 0
		},
		NULL,
		0,
		1024,65535,
		MAX_WATCHDOG_NUM,
		OtherPPPortAssignFunc, NULL, OtherPPPortShowFunc,WdSlotEmptyCheckFunc
	},

	/* End-of-list marker */
	{
		{NULL, 0, 0, NULL}, NULL, 0, 0, 0,-1, NULL, NULL, NULL
	}
};


static struct config_double ConfigureNamesDouble[] =
{
	/* End-of-list marker */
	{
		{NULL, 0, 0, NULL}, NULL, 0, 0, 0, NULL, NULL
	}
};


static struct config_double_array ConfigureNamesDoubleArray[] =
{
	{
		{"backend_weight", CFGCXT_RELOAD, CONNECTION_CONFIG,
			"load balance weight of backend.",
			CONFIG_VAR_TYPE_DOUBLE_ARRAY,true, 0
		},
		NULL,
		0,
		0.0,100000000.0,
		MAX_NUM_BACKENDS,
		BackendWeightAssignFunc, NULL, BackendWeightShowFunc,BackendSlotEmptyCheckFunc
	},
	
	/* End-of-list marker */
	{
		{NULL, 0, 0, NULL}, NULL, 0, 0, 0,-1, NULL, NULL, NULL
	}
};


static struct config_string_array ConfigureNamesStringArray[] =
{
	{
		{"backend_hostname", CFGCXT_RELOAD, CONNECTION_CONFIG,
			"hostname or IP address of PostgreSQL backend.",
			CONFIG_VAR_TYPE_STRING_ARRAY,true, 0
		},
		NULL,
		"",
		MAX_NUM_BACKENDS,
		BackendHostAssignFunc, NULL, BackendHostShowFunc,BackendSlotEmptyCheckFunc
	},

	{
		{"backend_data_directory", CFGCXT_RELOAD, CONNECTION_CONFIG,
			"data directory of the backend.",
			CONFIG_VAR_TYPE_STRING_ARRAY,true, 0
		},
		NULL,
		"",
		MAX_NUM_BACKENDS,
		BackendDataDirAssignFunc, NULL, BackendDataDirShowFunc,BackendSlotEmptyCheckFunc
	},
	
	{
		{"backend_flag", CFGCXT_RELOAD, CONNECTION_CONFIG,
			"Controls various backend behavior.",
			CONFIG_VAR_TYPE_STRING_ARRAY,true, 0
		},
		NULL,
		"ALLOW_TO_FAILOVER",
		MAX_NUM_BACKENDS,
		BackendFlagsAssignFunc, NULL, BackendFlagsShowFunc,BackendSlotEmptyCheckFunc
	},

	{
		{"heartbeat_destination", CFGCXT_RELOAD, WATCHDOG_LIFECHECK,
			"destination host for sending heartbeat signal.",
			CONFIG_VAR_TYPE_STRING_ARRAY,true, 0
		},
		NULL,
		"",
		WD_MAX_IF_NUM,
		HBDestinationAssignFunc, NULL, HBDestinationShowFunc,WdIFSlotEmptyCheckFunc
	},

	{
		{"heartbeat_device", CFGCXT_RELOAD, WATCHDOG_LIFECHECK,
			"Name of NIC device for sending hearbeat.",
			CONFIG_VAR_TYPE_STRING_ARRAY,true, 0
		},
		NULL,
		"",
		WD_MAX_IF_NUM,
		HBDeviceAssignFunc, NULL, HBDeviceShowFunc,WdIFSlotEmptyCheckFunc
	},

	{
		{"other_pgpool_hostname", CFGCXT_RELOAD, WATCHDOG_LIFECHECK,
			"Hostname of other pgpool node for watchdog connection.",
			CONFIG_VAR_TYPE_STRING_ARRAY,true, 0
		},
		NULL,
		"localhost",
		MAX_WATCHDOG_NUM,
		OtherPPHostAssignFunc, NULL, OtherPPHostShowFunc,WdSlotEmptyCheckFunc
	},

	/* End-of-list marker */
	{
		{NULL, 0, 0, NULL}, NULL, NULL, 0, NULL, NULL, NULL
	}
};


/* int configs*/
static struct config_int ConfigureNamesInt[] =
{

	{
		{"port", CFGCXT_INIT, CONNECTION_CONFIG,
			"tcp/IP port number on which pgpool will listen on.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.port,
		9999,
		1024,65535,
		NULL, NULL, NULL
	},

	{
		{"pcp_port", CFGCXT_INIT, CONNECTION_CONFIG,
			"tcp/IP port number on which pgpool PCP process will listen on.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.pcp_port,
		9898,
		1024,65535,
		NULL, NULL, NULL
	},

	{
		{"num_init_children", CFGCXT_INIT, CONNECTION_POOL_CONFIG,
			"Number of children pre-forked for client connections.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.num_init_children,
		32,
		1,INT_MAX,
		NULL, NULL, NULL
	},

	{
		{"listen_backlog_multiplier", CFGCXT_INIT, CONNECTION_CONFIG,
			"length of connection queue from frontend to pgpool-II",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.listen_backlog_multiplier,
		32,
		1,INT_MAX,
		NULL, NULL, NULL
	},

	{
		{"child_life_time", CFGCXT_INIT, CONNECTION_POOL_CONFIG,
			"pgpool-II child process life time in seconds.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.child_life_time,
		300,
		0,INT_MAX,
		NULL, NULL, NULL
	},

	{
		{"client_idle_limit", CFGCXT_SESSION, CONNECTION_POOL_CONFIG,
			"idle time in seconds to disconnects a client.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.client_idle_limit,
		0,
		0,INT_MAX,
		NULL, NULL, NULL
	},

	{
		{"connection_life_time", CFGCXT_INIT, CONNECTION_POOL_CONFIG,
			"Cached connections expiration time in seconds.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.connection_life_time,
		0,
		0,INT_MAX,
		NULL, NULL, NULL
	},

	{
		{"child_max_connections", CFGCXT_INIT, CONNECTION_POOL_CONFIG,
			"A pgpool-II child process will be terminated after this many connections from clients.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.child_max_connections,
		0,
		0,INT_MAX,
		NULL, NULL, NULL
	},

	{
		{"authentication_timeout", CFGCXT_INIT, CONNECTION_CONFIG,
			"Time out value in seconds for client authentication.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.authentication_timeout,
		0,
		0,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"max_pool", CFGCXT_INIT, CONNECTION_POOL_CONFIG,
			"Maximum number of connection pools per child process.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.max_pool,
		4,
		0,INT_MAX,
		NULL, NULL, NULL
	},

	{
		{"health_check_timeout", CFGCXT_RELOAD, HEALTH_CHECK_CONFIG,
			"Time out value in seconds for one health check.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.health_check_timeout,
		20,
		0,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"health_check_period", CFGCXT_RELOAD, HEALTH_CHECK_CONFIG,
			"Time interval in seconds between the health checks.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.health_check_period,
		0,
		0,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"health_check_max_retries", CFGCXT_RELOAD, HEALTH_CHECK_CONFIG,
			"The maximum number of times to retry a failed health check before giving up and initiating failover.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.health_check_max_retries,
		0,
		0,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"health_check_retry_delay", CFGCXT_RELOAD, HEALTH_CHECK_CONFIG,
			"The amount of time in seconds to wait between failed health check retries.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.health_check_retry_delay,
		1,
		0,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"connect_timeout", CFGCXT_RELOAD, HEALTH_CHECK_CONFIG,
			"Timeout in milliseconds before giving up connecting to backend.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.connect_timeout,
		10000,
		0,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"sr_check_period", CFGCXT_RELOAD, STREAMING_REPLICATION_CONFIG,
			"Time interval in seconds between the streaming replication delay checks.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.sr_check_period,
		0,
		0,INT_MAX,
		NULL, NULL, NULL
	},

	{
		{"recovery_timeout", CFGCXT_RELOAD, RECOVERY_CONFIG,
			"Maximum time in seconds to wait for the recovering PostgreSQL node.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.recovery_timeout,
		90,
		0,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"client_idle_limit_in_recovery", CFGCXT_SESSION, RECOVERY_CONFIG,
			"Time limit is seconds for the child connection, before it is terminated during the 2nd stage recovery.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.client_idle_limit_in_recovery,
		0,
		-1,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"search_primary_node_timeout", CFGCXT_RELOAD, HEALTH_CHECK_CONFIG,
			"Max time in seconds to search for primary node after failover.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.search_primary_node_timeout,
		300,
		0,INT_MAX,
		NULL, NULL, NULL
	},

	{
		{"wd_port", CFGCXT_INIT, WATCHDOG_CONFIG,
			"tcp/IP port number on which watchdog of process of pgpool will listen on.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.wd_port,
		9000,
		1024,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"wd_priority", CFGCXT_INIT, WATCHDOG_CONFIG,
			"Watchdog node priority for leader election.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.wd_priority,
		1,
		0,INT_MAX,
		NULL, NULL, NULL
	},

	{
		{"wd_interval", CFGCXT_INIT, WATCHDOG_CONFIG,
			"Time interval in seconds between life check.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.wd_interval,
		10,
		0,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"wd_life_point", CFGCXT_INIT, WATCHDOG_CONFIG,
			"Maximum number of retries before failing the life check.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.wd_life_point,
		3,
		0,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"wd_heartbeat_port", CFGCXT_INIT, WATCHDOG_CONFIG,
			"Port number for receiving heartbeat signal.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.wd_heartbeat_port,
		9694,
		1024,65535,
		NULL, NULL, NULL
	},
	
	{
		{"wd_heartbeat_keepalive", CFGCXT_INIT, WATCHDOG_CONFIG,
			"Time interval in seconds between sending the heartbeat siganl.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.wd_heartbeat_keepalive,
		2,
		1,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"wd_heartbeat_deadtime", CFGCXT_INIT, WATCHDOG_CONFIG,
			"Deadtime interval in seconds for heartbeat siganl.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.wd_heartbeat_deadtime,
		30,
		1,INT_MAX,
		NULL, NULL, NULL
	},

	{
		{"relcache_size", CFGCXT_INIT, CACHE_CONFIG,
			"Number of relation cache entry.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.relcache_size,
		256,
		0,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"memqcache_memcached_port", CFGCXT_INIT, CACHE_CONFIG,
			"Port number of Memcached server.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.memqcache_memcached_port,
		11211,
		0,INT_MAX,
		NULL, NULL, NULL
	},

	{
		{"memqcache_max_num_cache", CFGCXT_INIT, CACHE_CONFIG,
			"Total number of cache entries.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.memqcache_max_num_cache,
		1000000,
		0,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"memqcache_expire", CFGCXT_INIT, CACHE_CONFIG,
			"Memory cache entry life time specified in seconds.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.memqcache_expire,
		0,
		0,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"memqcache_maxcache", CFGCXT_INIT, CACHE_CONFIG,
			"Maximum SELECT result size in bytes.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.memqcache_maxcache,
		409600,
		0,INT_MAX,
		NULL, NULL, NULL
	},
	
	{
		{"memqcache_cache_block_size", CFGCXT_INIT, CACHE_CONFIG,
			"Cache block size in bytes.",
			CONFIG_VAR_TYPE_INT,false, 0
		},
		&g_pool_config.memqcache_cache_block_size,
		1048576,
		512,INT_MAX,
		NULL, NULL, NULL
	},

	/* End-of-list marker */
	{
		{NULL, 0, 0, NULL}, NULL, 0, 0, 0, NULL, NULL, NULL
	}
};

static struct config_enum ConfigureNamesEnum[] =
{
	{
		{"syslog_facility", CFGCXT_RELOAD, LOGING_CONFIG,
			"syslog local faclity.",
			CONFIG_VAR_TYPE_ENUM,false, 0
		},
		&g_pool_config.syslog_facility,
		LOG_LOCAL0,
		syslog_facility_options,
		NULL, NULL,
		SyslogFacilityProcessFunc,
		NULL
	},

	{
		{"log_error_verbosity", CFGCXT_SESSION, LOGING_CONFIG,
			"How much details about error should be emitted.",
			CONFIG_VAR_TYPE_ENUM,false, 0
		},
		&g_pool_config.log_error_verbosity,
		PGERROR_DEFAULT,
		log_error_verbosity_options,
		NULL, NULL, NULL, NULL
	},

	{
		{"client_min_messages", CFGCXT_SESSION, LOGING_CONFIG,
			"Which messages should be sent to client.",
			CONFIG_VAR_TYPE_ENUM,false, 0
		},
		&g_pool_config.client_min_messages,
		NOTICE,
		server_message_level_options,
		NULL, NULL, NULL, NULL
	},

	{
		{"log_min_messages", CFGCXT_SESSION, LOGING_CONFIG,
			"Which messages should be emitted to server log.",
			CONFIG_VAR_TYPE_ENUM,false, 0
		},
		&g_pool_config.log_min_messages,
		WARNING,
		server_message_level_options,
		NULL, NULL, NULL, NULL
	},
	
	{
		{"master_slave_sub_mode", CFGCXT_INIT, MASTER_SLAVE_CONFIG,
			"master/slave sub mode.",
			CONFIG_VAR_TYPE_ENUM,false, 0
		},
		(int*)&g_pool_config.master_slave_sub_mode,
		SLONY_MODE,
		master_slave_sub_mode_options,
		NULL, NULL, NULL, NULL
	},

	{
		{"log_standby_delay", CFGCXT_RELOAD, MASTER_SLAVE_CONFIG,
			"When to log standby delay.",
			CONFIG_VAR_TYPE_ENUM,false, 0
		},
		(int*)&g_pool_config.log_standby_delay,
		LSD_NONE,
		log_standby_delay_options,
		NULL, NULL, NULL, NULL
	},

	{
		{"wd_lifecheck_method", CFGCXT_INIT, WATCHDOG_CONFIG,
			"method for watchdog lifecheck.",
			CONFIG_VAR_TYPE_ENUM,false, 0
		},
		(int*)&g_pool_config.wd_lifecheck_method,
		LIFECHECK_BY_HB,
		wd_lifecheck_method_options,
		NULL, NULL, NULL, NULL
	},
	
	{
		{"memqcache_method", CFGCXT_INIT, CACHE_CONFIG,
			"Cache store method. either shmem(shared memory) or Memcached. shmem by default.",
			CONFIG_VAR_TYPE_ENUM,false, 0
		},
		(int*)&g_pool_config.memqcache_method,
		SHMEM_CACHE,
		memqcache_method_options,
		NULL, NULL, NULL, NULL
	},

	/* End-of-list marker */
	{
		{NULL, 0, 0, NULL}, NULL, 0, NULL, NULL, NULL, NULL, NULL
	}
};

/* finally the groups */
static struct config_grouped_array_var ConfigureVarGroups[] =
{
	{
		{"backend", CFGCXT_BOOT, CONNECTION_CONFIG,
			"backend configuration group.",
			CONFIG_VAR_TYPE_GROUP,false, 0
		},
		-1, /*until initialized*/
		NULL
	},
	{
		{"other_pgpool", CFGCXT_BOOT, WATCHDOG_CONFIG,
			"watchdog nodes configuration group.",
			CONFIG_VAR_TYPE_GROUP,false, 0
		},
		-1, /*until initialized*/
		NULL
	},
	{
		{"heartbeat", CFGCXT_BOOT, WATCHDOG_LIFECHECK,
			"heartbeat configuration group.",
			CONFIG_VAR_TYPE_GROUP,false, 0
		},
		-1, /*until initialized*/
		NULL
	},
	/* End-of-list marker */
	{
		{NULL, 0, 0, NULL}, -1 , NULL
	}
};


bool assign_variable_to_int_array_config_var(const char* name, int** variable)
{
	int i;
	for (i = 0; ConfigureNamesIntArray[i].gen.name; i++)
	{
		if (strcasecmp(ConfigureNamesIntArray[i].gen.name, name) == 0)
		{
			ConfigureNamesIntArray[i].variable = variable;
			return true;
		}
	}
	return false;
}

static void
build_config_variables(void)
{
	struct config_generic **all_vars;
	int			i;
	int num_vars = 0;

	for (i = 0; ConfigureNamesBool[i].gen.name; i++)
	{
		struct config_bool *conf = &ConfigureNamesBool[i];
		
		/* Rather than requiring vartype to be filled in by hand, do this: */
		conf->gen.vartype = CONFIG_VAR_TYPE_BOOL;
		num_vars++;
	}
	
	for (i = 0; ConfigureNamesInt[i].gen.name; i++)
	{
		struct config_int *conf = &ConfigureNamesInt[i];
		
		conf->gen.vartype = CONFIG_VAR_TYPE_INT;
		num_vars++;
	}

	for (i = 0; ConfigureNamesIntArray[i].gen.name; i++)
	{
		struct config_int_array *conf = &ConfigureNamesIntArray[i];

		conf->gen.vartype = CONFIG_VAR_TYPE_INT_ARRAY;
		/* Assign the memory for reset vals */
		conf->reset_vals = palloc(sizeof(int) * conf->max_elements);
		num_vars++;
	}
	
	for (i = 0; ConfigureNamesLong[i].gen.name; i++)
	{
		struct config_long *conf = &ConfigureNamesLong[i];
		
		conf->gen.vartype = CONFIG_VAR_TYPE_LONG;
		num_vars++;
	}
	
	for (i = 0; ConfigureNamesDouble[i].gen.name; i++)
	{
		struct config_double *conf = &ConfigureNamesDouble[i];
		
		conf->gen.vartype = CONFIG_VAR_TYPE_DOUBLE;
		num_vars++;
	}

	for (i = 0; ConfigureNamesString[i].gen.name; i++)
	{
		struct config_string *conf = &ConfigureNamesString[i];

		conf->gen.vartype = CONFIG_VAR_TYPE_STRING;
		num_vars++;
	}
	
	for (i = 0; ConfigureNamesEnum[i].gen.name; i++)
	{
		struct config_enum *conf = &ConfigureNamesEnum[i];
		
		conf->gen.vartype = CONFIG_VAR_TYPE_ENUM;
		num_vars++;
	}

	for (i = 0; ConfigureNamesStringList[i].gen.name; i++)
	{
		struct config_string_list *conf = &ConfigureNamesStringList[i];
		
		conf->gen.vartype = CONFIG_VAR_TYPE_STRING_LIST;
		num_vars++;
	}

	for (i = 0; ConfigureNamesStringArray[i].gen.name; i++)
	{
		struct config_string_array *conf = &ConfigureNamesStringArray[i];
		conf->gen.dynamic_array_var = true;
		conf->gen.vartype = CONFIG_VAR_TYPE_STRING_ARRAY;
		/* Assign the memory for reset vals */
		conf->reset_vals = palloc(sizeof(char*) * conf->max_elements);
		num_vars++;
	}

	for (i = 0; ConfigureNamesDoubleArray[i].gen.name; i++)
	{
		struct config_double_array *conf = &ConfigureNamesDoubleArray[i];
		conf->gen.dynamic_array_var = true;
		conf->gen.vartype = CONFIG_VAR_TYPE_DOUBLE_ARRAY;
		/* Assign the memory for reset vals */
		conf->reset_vals = palloc(sizeof(double) * conf->max_elements);
		num_vars++;
	}

	/* For end marker */
	num_vars++;

	all_vars = (struct config_generic **)
	palloc(num_vars * sizeof(struct config_generic *));
	
	num_vars = 0;
	
	for (i = 0; ConfigureNamesBool[i].gen.name; i++)
		all_vars[num_vars++] = &ConfigureNamesBool[i].gen;

	for (i = 0; ConfigureNamesInt[i].gen.name; i++)
		all_vars[num_vars++] = &ConfigureNamesInt[i].gen;

	for (i = 0; ConfigureNamesLong[i].gen.name; i++)
		all_vars[num_vars++] = &ConfigureNamesLong[i].gen;

	for (i = 0; ConfigureNamesDouble[i].gen.name; i++)
		all_vars[num_vars++] = &ConfigureNamesDouble[i].gen;

	for (i = 0; ConfigureNamesString[i].gen.name; i++)
		all_vars[num_vars++] = &ConfigureNamesString[i].gen;

	for (i = 0; ConfigureNamesEnum[i].gen.name; i++)
		all_vars[num_vars++] = &ConfigureNamesEnum[i].gen;

	for (i = 0; ConfigureNamesStringList[i].gen.name; i++)
		all_vars[num_vars++] = &ConfigureNamesStringList[i].gen;
	
	for (i = 0; ConfigureNamesStringArray[i].gen.name; i++)
		all_vars[num_vars++] = &ConfigureNamesStringArray[i].gen;
	
	for (i = 0; ConfigureNamesIntArray[i].gen.name; i++)
		all_vars[num_vars++] = &ConfigureNamesIntArray[i].gen;

	for (i = 0; ConfigureNamesDoubleArray[i].gen.name; i++)
		all_vars[num_vars++] = &ConfigureNamesDoubleArray[i].gen;

	if (all_parameters)
		pfree(all_parameters);
	all_parameters = all_vars;
	num_all_parameters = num_vars;
	sort_config_vars();
	build_variable_groups();
}

static void build_variable_groups(void)
{
	/* we build these by hand */
	/* group 1. Backend config vars */
	ConfigureVarGroups[0].var_count = 5;
	ConfigureVarGroups[0].var_list = palloc0(sizeof(struct config_generic*) * ConfigureVarGroups[0].var_count);
	ConfigureVarGroups[0].var_list[0] = find_option("backend_hostname", FATAL);
	ConfigureVarGroups[0].var_list[0]->flags |= VAR_PART_OF_GROUP;
	ConfigureVarGroups[0].var_list[1] = find_option("backend_port", FATAL);
	ConfigureVarGroups[0].var_list[1]->flags |= VAR_PART_OF_GROUP;
	ConfigureVarGroups[0].var_list[2] = find_option("backend_weight", FATAL);
	ConfigureVarGroups[0].var_list[2]->flags |= VAR_PART_OF_GROUP;
	ConfigureVarGroups[0].var_list[3] = find_option("backend_data_directory", FATAL);
	ConfigureVarGroups[0].var_list[3]->flags |= VAR_PART_OF_GROUP;
	ConfigureVarGroups[0].var_list[4] = find_option("backend_flag", FATAL);
	ConfigureVarGroups[0].var_list[4]->flags |= VAR_PART_OF_GROUP;


	/* group 2. other_pgpool config vars */
	ConfigureVarGroups[1].var_count = 3;
	ConfigureVarGroups[1].var_list = palloc0(sizeof(struct config_generic*) * ConfigureVarGroups[1].var_count);
	/* backend hostname */
	ConfigureVarGroups[1].var_list[0] = find_option("other_pgpool_hostname", FATAL);
	ConfigureVarGroups[1].var_list[0]->flags |= VAR_PART_OF_GROUP;
	ConfigureVarGroups[1].var_list[1] = find_option("other_pgpool_port", FATAL);
	ConfigureVarGroups[1].var_list[1]->flags |= VAR_PART_OF_GROUP;
	ConfigureVarGroups[1].var_list[2] = find_option("other_wd_port", FATAL);
	ConfigureVarGroups[1].var_list[2]->flags |= VAR_PART_OF_GROUP;


	/* group 3. heartbeat config vars */
	ConfigureVarGroups[2].var_count = 3;
	ConfigureVarGroups[2].var_list = palloc0(sizeof(struct config_generic*) * ConfigureVarGroups[2].var_count);
	/* backend hostname */
	ConfigureVarGroups[2].var_list[0] = find_option("heartbeat_device", FATAL);
	ConfigureVarGroups[2].var_list[0]->flags |= VAR_PART_OF_GROUP;
	ConfigureVarGroups[2].var_list[1] = find_option("heartbeat_destination", FATAL);
	ConfigureVarGroups[2].var_list[1]->flags |= VAR_PART_OF_GROUP;
	ConfigureVarGroups[2].var_list[2] = find_option("heartbeat_destination_port", FATAL);
	ConfigureVarGroups[2].var_list[2]->flags |= VAR_PART_OF_GROUP;

}

/* Sort the config variables on based of string length of
 * variable names. Since we want to compare long variable names to be compared
 * before the smaller ones. This ensure heartbeat_destination should not match
 * with heartbeat_destination_port, when we use strncmp to cater for the index at
 * the end of the variable names.*/
static void sort_config_vars(void)
{
	int i,j;
	for (i = 0; i < num_all_parameters; ++i)
	{
		struct config_generic* gconfi = all_parameters[i];
		int leni = strlen(gconfi->name);
		for (j = i + 1; j < num_all_parameters; ++j)
		{
			struct config_generic* gconfj = all_parameters[j];
			int lenj = strlen(gconfj->name);

			if (leni < lenj)
			{
				all_parameters[i] = gconfj;
				all_parameters[j] = gconfi;
				gconfi = all_parameters[i];
				leni = lenj;
			}
		}
	}
}

/*
 * Initialize all variables to its compiled-in default.
 */
static void
initialize_variables_with_default(struct config_generic * gconf)
{
	gconf->status = 0;
	gconf->source = PGC_S_DEFAULT;
	gconf->scontext = CFGCXT_BOOT;
	gconf->sourceline = 0;
	
	switch (gconf->vartype)
	{
		case CONFIG_VAR_TYPE_GROUP: /* just to keep compiler quite */
			break;

		case CONFIG_VAR_TYPE_BOOL:
		{
			struct config_bool *conf = (struct config_bool *) gconf;
			bool		newval = conf->boot_val;

			if (conf->assign_func)
			{
				(*conf->assign_func)(gconf->scontext, newval, ERROR);
			}
			else
			{
				*conf->variable = conf->reset_val = newval;
			}
			break;
		}

		case CONFIG_VAR_TYPE_INT:
		{
			struct config_int *conf = (struct config_int *) gconf;
			int	newval = conf->boot_val;
			
			if (conf->assign_func)
			{
				(*conf->assign_func)(gconf->scontext, newval, ERROR);
			}
			else
			{
				*conf->variable = newval;
			}
			conf->reset_val = newval;

			break;
		}

		case CONFIG_VAR_TYPE_DOUBLE:
		{
			struct config_double *conf = (struct config_double *) gconf;
			double	newval = conf->boot_val;
			
			if (conf->assign_func)
			{
				(*conf->assign_func)(gconf->scontext, newval, ERROR);
			}
			else
			{
				*conf->variable = newval;
			}
			conf->reset_val = newval;
			
			break;
		}

		case CONFIG_VAR_TYPE_INT_ARRAY:
		{
			struct config_int_array *conf = (struct config_int_array *) gconf;
			int	newval = conf->boot_val;
			int i;

			for (i = 0; i < conf->max_elements; i ++)
			{
				if (conf->assign_func)
				{
					(*conf->assign_func)(gconf->scontext, newval, i,ERROR);
				}
				else
				{
					*conf->variable[i] = newval;
				}
				conf->reset_vals[i] = newval;
			}
			break;
		}

		case CONFIG_VAR_TYPE_DOUBLE_ARRAY:
		{
			struct config_double_array *conf = (struct config_double_array *) gconf;
			double	newval = conf->boot_val;
			int i;

			for (i = 0; i < conf->max_elements; i ++)
			{
				if (conf->assign_func)
				{
					(*conf->assign_func)(gconf->scontext, newval, i,ERROR);
				}
				else
				{
					*conf->variable[i] = newval;
				}
				conf->reset_vals[i] = newval;
			}
			break;
		}

		case CONFIG_VAR_TYPE_LONG:
		{
			struct config_long *conf = (struct config_long *) gconf;
			long	newval = conf->boot_val;
			
			if (conf->assign_func)
			{
				(*conf->assign_func)(gconf->scontext, newval, ERROR);
			}
			else
			{
				*conf->variable = conf->reset_val = newval;
			}
			break;
		}

		case CONFIG_VAR_TYPE_STRING:
		{
			struct config_string *conf = (struct config_string *) gconf;
			char	   *newval = NULL;

			/* non-NULL boot_val must always get strdup'd */
			if (conf->boot_val != NULL)
				newval = pstrdup(conf->boot_val);

			if (conf->assign_func)
			{
				(*conf->assign_func)(gconf->scontext, newval, ERROR);
			}
			else
			{
				*conf->variable = newval;
			}
			conf->reset_val = newval;

			if (conf->process_func)
			{
				(*conf->process_func)(newval, ERROR);
			}
			break;
		}

		case CONFIG_VAR_TYPE_STRING_ARRAY:
		{
			struct config_string_array *conf = (struct config_string_array *) gconf;
			int i;
			char	   *newval = NULL;
			
			/* non-NULL boot_val must always get strdup'd
			 * also check if max_elements > 0 before doing pstrdup to silent
			 * the coverity scan report
			 */
			if (conf->boot_val != NULL && conf->max_elements > 0)
				newval = pstrdup(conf->boot_val);

			for (i = 0; i < conf->max_elements; i ++)
			{
				if (conf->assign_func)
				{
					(*conf->assign_func)(gconf->scontext, newval, i, ERROR);
				}
				else
				{
					*conf->variable[i] = newval;
				}
				conf->reset_vals[i] = newval;
			}
			break;
		}

		case CONFIG_VAR_TYPE_ENUM:
		{
			struct config_enum *conf = (struct config_enum *) gconf;
			int			newval = conf->boot_val;

			if (conf->assign_func)
			{
				(*conf->assign_func)(gconf->scontext, newval, ERROR);
			}
			else
			{
				*conf->variable = newval;
			}
			conf->reset_val = newval;

			if (conf->process_func)
			{
				(*conf->process_func)(newval, ERROR);
			}

			break;
		}

		case CONFIG_VAR_TYPE_STRING_LIST:
		{
			struct config_string_list *conf = (struct config_string_list *) gconf;
			char	   *newval = NULL;

			/* non-NULL boot_val must always get strdup'd */
			if (conf->boot_val != NULL)
				newval = pstrdup(conf->boot_val);
			else
				newval = NULL;

			if (conf->assign_func)
			{
				(*conf->assign_func)(gconf->scontext, newval, ERROR);
			}
			else
			{
				*conf->variable = get_list_from_string(newval,conf->seperator, conf->list_elements_count);
				if (conf->compute_regex)
				{
					int i;
					for (i=0;i < *conf->list_elements_count; i++)
					{
						add_regex_pattern((const char*)conf->gen.name, (*conf->variable)[i]);
					}
				}
			}
			/* save the string value */
			if (newval)
				conf->reset_val = pstrdup(newval);
			else
				conf->reset_val = NULL;
			conf->current_val = newval;

			break;
		}

	}
}

/*
 * Extract tokens separated by delimi from str. Return value is an
 * array of pointers in pallocd strings. number of elements are set to
 * n.
 */
static char **get_list_from_string(const char *str, const char *delimi, int *n)
{
	char *token;
	char **tokens;
	char *temp_string;
	const int MAXTOKENS = 256;

	*n = 0;

	if (str == NULL || *str == '\0')
		return NULL;

	temp_string = pstrdup(str);
	tokens = palloc(MAXTOKENS * sizeof(char *));

	ereport(DEBUG3,
		(errmsg("extracting string tokens from [%s] based on %s", temp_string,delimi)));
	
	for (token = strtok(temp_string, delimi); token != NULL ; token = strtok(NULL, delimi))
	{
		tokens[*n] = pstrdup(token);
		ereport(DEBUG3,
			(errmsg("initializing pool configuration"),
				 errdetail("extracting string tokens [token[%d]: %s]", *n, tokens[*n])));
		
		(*n)++;
		
		if ( ((*n) % MAXTOKENS ) == 0)
		{
			tokens = repalloc(tokens, (MAXTOKENS * sizeof(char *) * (((*n)/MAXTOKENS) +1) ));
		}
	}
	/* how about reclaiming the unused space */
	if (*n > 0)
		tokens = repalloc(tokens, (sizeof(char *) * (*n) ));
	pfree(temp_string);

	return tokens;
}

/*
 * Memory of the array type variables must be initialized befor calling this function
 */
void
InitializeConfigOptions(void)
{
	int			i;

	/*
	 * Before we do anything set the log_min_messages to ERROR.
	 * Reason for doing that is before the log_min_messages gets initialized
	 * with the actual value the pgpool-II log should not get flooded by DEBUG
	 * messages
	 */
	g_pool_config.log_min_messages = ERROR;
	g_pool_config.syslog_facility = LOG_LOCAL0;
	build_config_variables();

	/*
	 * Load all variables with their compiled-in defaults, and initialize
	 * status fields as needed.
	 */
	for (i = 0; i < num_all_parameters; i++)
	{
		initialize_variables_with_default(all_parameters[i]);
	}
	/* do the post processing */
	config_post_processor(CFGCXT_BOOT, FATAL);
}

/*
 * returns the index value postfixed with the variable name
 * for example if the if name contains "backend_hostname11" and
 * the record name must be for the variable nameed "backend_hostname"
 * if the index is not present at end of the name the function
 * will return true and out parameter index will be assigned with -ve value
 */
static bool get_index_in_var_name(struct config_generic* record, const char* name, int *index, int elevel)
{
	char *ptr;
	int index_start_index = strlen(record->name);
	if (strlen(name) <= index_start_index)
	{
		/* no index is provided */
		*index = -1;
		return true;
	}
	ptr = (char*)(name + index_start_index);
	while (*ptr)
	{
		if (isdigit(*ptr) == 0)
		{
			ereport(elevel,
					(errmsg("invalid index value for parameter \"%s\" ",name),
					 (errdetail("index part contains the invalid non digit character"))));
			return false;
		}
		ptr++;
	}
	*index = atoi(name + index_start_index);
	return true;
}

/*
 * Look up option NAME.  If it exists, return a pointer to its record,
 * else return NULL.
 */
static struct config_generic *
find_option(const char *name, int elevel)
{
	int	i;
	for (i = 0; i < num_all_parameters; i++)
	{
		struct config_generic* gconf = all_parameters[i];
		if (gconf->dynamic_array_var)
		{
			int index_start_index = strlen(gconf->name);
			/*
			 * For dynamic array type vars the key also have the index at the end
			 * e.g. backend_hostname0 so we only comapare the key's name part
			 */
			if (!strncmp(gconf->name, name, index_start_index))
				return gconf;
		}
		else
		{
			if (!strcmp(gconf->name, name))
				return gconf;
		}
	}
	/* Unknown name */
	return NULL;
}

/*
 * Lookup the value for an enum option with the selected name
 * (case-insensitive).
 * If the enum option is found, sets the retval value and returns
 * true. If it's not found, return FALSE and retval is set to 0.
 */
static bool
config_enum_lookup_by_name(struct config_enum * record, const char *value,
						   int *retval)
{
	const struct config_enum_entry *entry;
	
	for (entry = record->options; entry && entry->name; entry++)
	{
		if (strcasecmp(value, entry->name) == 0)
		{
			*retval = entry->val;
			return TRUE;
		}
	}
	
	*retval = 0;
	return FALSE;
}


bool
set_config_options(ConfigVariable *head_p,
				  ConfigContext context, GucSource source, int elevel)
{
	ConfigVariable *item = head_p;
	while (item)
	{
		ConfigVariable *next = item->next;
		setConfigOption(item->name, item->value, context, source, elevel);
		item = next;
	}
	return config_post_processor(context,elevel);
}

bool set_one_config_option(const char *name, const char *value,
						   ConfigContext context, GucSource source, int elevel)
{
	if (setConfigOption(name, value, context, source, elevel) == true)
		return config_post_processor(context,elevel);
	return false;
}

static bool
setConfigOption(const char *name, const char *value,
				ConfigContext context, GucSource source, int elevel)
{
	struct config_generic *record;
	int		index_val = -1;

	ereport(DEBUG2,
			(errmsg("set_config_option \"%s\" = \"%s\"", name,value)));
	
	record = find_option(name, elevel);
	
	if (record == NULL)
	{
		/* we emit only INFO message when setting the option from
		 * configuration file.
		 * As the conf file may still contain some configuration parameters only exist
		 * in older version and does not exist anymore
		 */
		ereport(source == PGC_S_FILE?INFO:elevel,
				(errmsg("unrecognized configuration parameter \"%s\"", name)));
		return false;
	}
	if (record->dynamic_array_var)
	{
		if (get_index_in_var_name(record, name, &index_val, elevel) == false)
			return false;
		
		if (index_val < 0)
		{
			/* index is not provided */
			ereport(elevel,
					(errmsg("parameter \"%s\" expects the index value",
							name)));
			return false;
		}
	}

	return setConfigOptionVar(record, name, index_val, value, context, source, elevel);
}

static bool
setConfigOptionVar(struct config_generic *record, const char* name, int index_val, const char *value,
				  ConfigContext context, GucSource source, int elevel)
{
	/*
	 * Check if the option can be set at this time. See guc.h for the precise
	 * rules.
	 */
	switch (record->context)
	{
		case CFGCXT_BOOT:
			if (context != CFGCXT_BOOT)
			{
				if (context == CFGCXT_RELOAD)
				{
					/* Do not treat it as an error. Since the RELOAD context is used by reload config mechanism of
					 * pgpool-II and the configuration file always contain all the values, including
					 * those that are not allowed to be changed in reload context.
					 * So silently ignoring this for the time being is the best way to go until we enhance the logic
					 * around this
					 */
					ereport(DEBUG2,
							(errmsg("invalid Context, value for parameter \"%s\" cannot be changed",
									name)));
					return true;
				}

				ereport(elevel,
						(errmsg("invalid Context, value for parameter \"%s\" cannot be changed",
								name)));
				return false;
			}
			break;
		case CFGCXT_INIT:
			if (context != CFGCXT_INIT && context != CFGCXT_BOOT)
			{
				if (context == CFGCXT_RELOAD)
				{
					ereport(DEBUG2,
							(errmsg("invalid Context, value for parameter \"%s\" cannot be changed",
									name)));
					return true;
				}

				ereport(elevel,
						(errmsg("invalid Context, value for parameter \"%s\" cannot be changed",
								name)));
				return false;
			}
			break;
		case CFGCXT_RELOAD:
			if (context > CFGCXT_RELOAD)
			{
				ereport(elevel,
						(errmsg("invalid Context, value for parameter \"%s\" cannot be changed",
								name)));
				return false;
			}
			break;
		case CFGCXT_PCP:
			if (context > CFGCXT_PCP)
			{
				ereport(elevel,
						(errmsg("invalid Context, value for parameter \"%s\" cannot be changed",
								name)));
				return false;
			}
			break;

		case CFGCXT_SESSION:
			break;

		default:
		{
			ereport(elevel,
					(errmsg("invalid record context, value for parameter \"%s\" cannot be changed",
							name)));
			return false;
		}
			break;
	}

	if (record->dynamic_array_var)
	{
		if (index_val < 0)
		{
			/* index is not provided */
			ereport(elevel,
					(errmsg("parameter \"%s\" expects the index value",
							name)));
			return false;
		}
	}

	/*
	 * Evaluate value and set variable.
	 */
	switch (record->vartype)
	{
		case CONFIG_VAR_TYPE_GROUP:
			ereport(ERROR,(errmsg("invalid config variable type. operation not allowed")));
			break;

		case CONFIG_VAR_TYPE_BOOL:
		{
			struct config_bool *conf = (struct config_bool *) record;
			bool		newval = conf->boot_val;

			if (value != NULL)
			{
				newval = eval_logical(value);

			}
			else if (source == PGC_S_DEFAULT)
			{
				newval = conf->boot_val;
			}
			else
			{
				/* Reset */
				newval = conf->reset_val;
			}
			if (conf->assign_func)
			{
				if ((*conf->assign_func)(context, newval, elevel) == false)
					return false;
			}
			else
				*conf->variable = newval;

			if (context == CFGCXT_INIT)
				conf->reset_val = newval;
		}
			break;

		case CONFIG_VAR_TYPE_INT:
		{
			struct config_int *conf = (struct config_int *) record;
			int			newval;
			
			if (value != NULL)
			{
				newval = atoi(value);
			}
			else if (source == PGC_S_DEFAULT)
			{
				newval = conf->boot_val;
			}
			else
			{
				/* Reset */
				newval = conf->reset_val;
			}
			
			if (newval < conf->min || newval > conf->max)
			{
				ereport(elevel,
						(errmsg("%d is outside the valid range for parameter \"%s\" (%d .. %d)",
								newval, name,
								conf->min, conf->max)));
				return false;
			}

			if (conf->assign_func)
			{
				if ((*conf->assign_func)(context, newval, elevel) == false)
					return false;
			}
			else
			{
				*conf->variable = newval;
			}

			if (context == CFGCXT_INIT)
				conf->reset_val = newval;
		}
			break;

		case CONFIG_VAR_TYPE_DOUBLE:
		{
			struct config_double *conf = (struct config_double *) record;
			double			newval;
			
			if (value != NULL)
			{
				newval = atof(value);
			}
			else if (source == PGC_S_DEFAULT)
			{
				newval = conf->boot_val;
			}
			else
			{
				/* Reset */
				newval = conf->reset_val;
			}
			
			if (newval < conf->min || newval > conf->max)
			{
				ereport(elevel,
						(errmsg("%f is outside the valid range for parameter \"%s\" (%f .. %f)",
								newval, name,
								conf->min, conf->max)));
				return false;
			}
			
			if (conf->assign_func)
			{
				if ((*conf->assign_func)(context, newval, elevel) == false)
					return false;
			}
			else
			{
				*conf->variable = newval;
			}

			if (context == CFGCXT_INIT)
				conf->reset_val = newval;

		}
			break;

		case CONFIG_VAR_TYPE_INT_ARRAY:
		{
			struct config_int_array *conf = (struct config_int_array *) record;
			int			newval;

			if (index_val < 0 || index_val > conf->max_elements)
			{
				ereport(elevel,
						(errmsg("%d index outside the valid range for parameter \"%s\" (%d .. %d)",
								index_val, name,
								0, conf->max_elements)));
				return false;
			}

			if (value != NULL)
			{
				newval = atoi(value);
			}
			else if (source == PGC_S_DEFAULT)
			{
				newval = conf->boot_val;
			}
			else
			{
				/* Reset */
				newval = conf->reset_vals[index_val];
			}

			if (newval < conf->min || newval > conf->max)
			{
				ereport(elevel,
						(errmsg("%d is outside the valid range for parameter \"%s\" (%d .. %d)",
								newval, name,
								conf->min, conf->max)));
				return false;
			}

			if (conf->assign_func)
			{
				if ((*conf->assign_func)(context, newval, index_val, elevel) == false)
					return false;
			}
			else
			{
				*conf->variable[index_val] = newval;
			}

			if (context == CFGCXT_INIT)
				conf->reset_vals[index_val] = newval;

		}
			break;

		case CONFIG_VAR_TYPE_DOUBLE_ARRAY:
		{
			struct config_double_array *conf = (struct config_double_array *) record;
			double			newval;
			
			if (index_val < 0 || index_val > conf->max_elements)
			{
				ereport(elevel,
						(errmsg("%d index outside the valid range for parameter \"%s\" (%d .. %d)",
								index_val, name,
								0, conf->max_elements)));
				return false;
			}
			
			if (value != NULL)
			{
				newval = atof(value);
			}
			else if (source == PGC_S_DEFAULT)
			{
				newval = conf->boot_val;
			}
			else
			{
				/* Reset */
				newval = conf->reset_vals[index_val];
			}
			
			if (newval < conf->min || newval > conf->max)
			{
				ereport(elevel,
						(errmsg("%f is outside the valid range for parameter \"%s\" (%f .. %f)",
								newval, name,
								conf->min, conf->max)));
				return false;
			}
			
			if (conf->assign_func)
			{
				if ((*conf->assign_func)(context, newval, index_val, elevel) ==false)
					return false;
			}
			else
			{
				*conf->variable[index_val] = newval;
			}

			if (context == CFGCXT_INIT)
				conf->reset_vals[index_val] = newval;

		}
			break;


		case CONFIG_VAR_TYPE_LONG:
		{
			struct config_long *conf = (struct config_long *) record;
			int64	newval;
			
			if (value != NULL)
			{
				newval = pool_atoi64(value);
			}
			else if (source == PGC_S_DEFAULT)
			{
				newval = conf->boot_val;
			}
			else
			{
				/* Reset */
				newval = conf->reset_val;
			}
			
			if (newval < conf->min || newval > conf->max)
			{
				ereport(elevel,
						(errmsg("%ld is outside the valid range for parameter \"%s\" (%ld .. %ld)",
								newval, name,
								conf->min, conf->max)));
				return false;
			}
			
			if (conf->assign_func)
			{
				if ((*conf->assign_func)(context, newval, elevel) == false)
					return false;
			}
			else
			{
				*conf->variable = newval;
			}

			if (context == CFGCXT_INIT)
				conf->reset_val = newval;

		}
			break;

		case CONFIG_VAR_TYPE_STRING:
		{
			struct config_string *conf = (struct config_string *) record;
			char	   *newval = NULL;

			if (value != NULL)
			{
				newval = pstrdup(value);
			}
			else if (source == PGC_S_DEFAULT)
			{
				if (conf->boot_val)
					newval = pstrdup(conf->boot_val);
			}
			else
			{
				/* Reset */
				if (conf->reset_val)
					newval = pstrdup(conf->reset_val);
			}

			if (conf->assign_func)
			{
				if ((*conf->assign_func)(context, newval, elevel) == false)
					return false;
			}
			else
			{
				if (*conf->variable)
					pfree(*conf->variable);
				*conf->variable = newval;
			}
			if (context == CFGCXT_INIT)
			{
				conf->reset_val = newval;
			}
			if (conf->process_func)
			{
				(*conf->process_func)(newval, elevel);
			}

		}
			break;

		case CONFIG_VAR_TYPE_STRING_ARRAY:
		{
			struct config_string_array *conf = (struct config_string_array *) record;
			char	   *newval = NULL;

			if (index_val < 0 || index_val > conf->max_elements)
			{
				ereport(elevel,
						(errmsg("%d index outside the valid range for parameter \"%s\" (%d .. %d)",
								index_val, name,
								0, conf->max_elements)));
				return false;
			}

			if (value != NULL)
			{
				newval = pstrdup(value);
			}
			else if (source == PGC_S_DEFAULT)
			{
				if (conf->boot_val)
					newval = pstrdup(conf->boot_val);
			}
			else
			{
				/* Reset */
				if (conf->reset_vals[index_val])
					newval = pstrdup(conf->reset_vals[index_val]);
			}
			
			if (conf->assign_func)
			{
				if ((*conf->assign_func)(context, newval, index_val,elevel) == false)
					return false;
			}
			else
			{
				if (*conf->variable[index_val])
					pfree(*conf->variable[index_val]);
				*conf->variable[index_val] = newval;
			}
			if (context == CFGCXT_INIT)
			{
				conf->reset_vals[index_val] = newval;
			}
			
		}
			break;

		case CONFIG_VAR_TYPE_STRING_LIST:
		{
			struct config_string_list *conf = (struct config_string_list *) record;
			char	   *newval = NULL;

			if (value != NULL)
			{
				newval = (char*)pstrdup(value);
			}
			else if (source == PGC_S_DEFAULT)
			{
				if (conf->boot_val)
					newval = pstrdup((char*)conf->boot_val);
			}
			else
			{
				/* Reset */
				if (conf->reset_val)
					newval = pstrdup(conf->reset_val);
			}

			if (conf->assign_func)
			{
				if ((*conf->assign_func)(context, newval, elevel) == false)
				{
					pfree(newval);
					return false;
				}
			}
			else
			{
				if (*conf->variable)
				{
					int i;
					for (i=0;i < *conf->list_elements_count; i++)
					{
						if ((*conf->variable)[i])
							pfree((*conf->variable)[i]);
						(*conf->variable)[i] = NULL;
					}
					pfree(*conf->variable);
				}

				*conf->variable = get_list_from_string(newval, conf->seperator, conf->list_elements_count);
				if (conf->compute_regex)
				{
					/* TODO clear the old regex array please */
					int i;
					for (i=0;i < *conf->list_elements_count; i++)
					{
						add_regex_pattern(conf->gen.name, (*conf->variable)[i]);
					}
				}
			}

			if (context == CFGCXT_INIT)
			{
				if (conf->reset_val)
					pfree(conf->reset_val);

				if (newval)
					conf->reset_val = pstrdup(newval);
				else
					conf->reset_val = NULL;
			}

			/* save the string value */
			if (conf->current_val)
				pfree(conf->current_val);
			conf->current_val = newval;
		}
			break;

		case CONFIG_VAR_TYPE_ENUM:
		{
			struct config_enum *conf = (struct config_enum *) record;
			int			newval;

			if (value != NULL)
			{
				newval = atoi(value);
			}
			else if (source == PGC_S_DEFAULT)
			{
				newval = conf->boot_val;
			}
			else
			{
				/* Reset */
				newval = conf->reset_val;
			}
			
			if (value && !config_enum_lookup_by_name(conf, value, &newval))
			{

				char	   *hintmsg = NULL;
				
#ifndef POOL_PRIVATE
				hintmsg = config_enum_get_options(conf,
												  "Available values: ",
												  ".", ", ");
#endif
				
				ereport(elevel,
						(errmsg("invalid value for parameter \"%s\": \"%s\"",
								name, value),
						 hintmsg ? errhint("%s",hintmsg) : 0));
				
				if (hintmsg)
					pfree(hintmsg);
				return false;
			}

			if (conf->assign_func)
			{
				if ((*conf->assign_func)(context, newval, elevel) == false)
					return false;
			}
			else
			{
				*conf->variable = newval;
			}

			if (context == CFGCXT_INIT)
				conf->reset_val = newval;

			if (conf->process_func)
			{
				(*conf->process_func)(newval, elevel);
			}

			break;
		}
	}

	record->scontext = context;
	return true;
}

#ifndef POOL_PRIVATE

/*
 * Return a list of all available options for an enum, excluding
 * hidden ones, separated by the given separator.
 * If prefix is non-NULL, it is added before the first enum value.
 * If suffix is non-NULL, it is added to the end of the string.
 */
static char *
config_enum_get_options(struct config_enum * record, const char *prefix,
						const char *suffix, const char *separator)
{
	const struct config_enum_entry *entry;
	StringInfoData retstr;
	int			seplen;
	
	initStringInfo(&retstr);
	appendStringInfoString(&retstr, prefix);
	
	seplen = strlen(separator);
	for (entry = record->options; entry && entry->name; entry++)
	{
		if (!entry->hidden)
		{
			appendStringInfoString(&retstr, entry->name);
			appendBinaryStringInfo(&retstr, separator, seplen);
		}
	}
	
	/*
	 * All the entries may have been hidden, leaving the string empty if no
	 * prefix was given. This indicates a broken GUC setup, since there is no
	 * use for an enum without any values, so we just check to make sure we
	 * don't write to invalid memory instead of actually trying to do
	 * something smart with it.
	 */
	if (retstr.len >= seplen)
	{
		/* Replace final separator */
		retstr.data[retstr.len - seplen] = '\0';
		retstr.len -= seplen;
	}
	
	appendStringInfoString(&retstr, suffix);
	
	return retstr.data;
}
#endif

static bool BackendWeightAssignFunc (ConfigContext context, double newval, int index, int elevel)
{
	double old_v = g_pool_config.backend_desc->backend_info[index].unnormalized_weight;
	g_pool_config.backend_desc->backend_info[index].unnormalized_weight = newval;
	/*
	 * Log weight change event only when context is
	 * reloading of pgpool.conf and weight is actually
	 * changed
	 */
	if (context == CFGCXT_RELOAD && old_v != newval)
	{
		ereport(LOG,
			(errmsg("initializing pool configuration: backend weight for backend:%d changed from %f to %f", index, old_v, newval),
				 errdetail("This change will be effective from next client session")));
	}
	return true;
}


static bool BackendPortAssignFunc (ConfigContext context, int newval, int index, int elevel)
{
	BACKEND_STATUS backend_status = g_pool_config.backend_desc->backend_info[index].backend_status;

	if (context <= CFGCXT_INIT)
	{
		g_pool_config.backend_desc->backend_info[index].backend_port = newval;
		g_pool_config.backend_desc->backend_info[index].backend_status = CON_CONNECT_WAIT;
	}
	else if (backend_status == CON_UNUSED)
	{
		g_pool_config.backend_desc->backend_info[index].backend_port = newval;
		g_pool_config.backend_desc->backend_info[index].backend_status = CON_DOWN;
	}
	else
	{
		if (context != CFGCXT_RELOAD)
			ereport(WARNING,
					(errmsg("backend_port%d cannot be changed in context %d and backend status = %d",index,context,backend_status)));
		return false;
	}
	return true;
}


static bool BackendHostAssignFunc (ConfigContext context, char* newval, int index, int elevel)
{
	BACKEND_STATUS backend_status = g_pool_config.backend_desc->backend_info[index].backend_status;
	if (context <= CFGCXT_INIT || backend_status == CON_UNUSED)
	{
		if (newval == NULL || strlen(newval) == 0)
			g_pool_config.backend_desc->backend_info[index].backend_hostname[0] = '\0';
		else
			strlcpy(g_pool_config.backend_desc->backend_info[index].backend_hostname, newval, MAX_DB_HOST_NAMELEN-1);
		return true;
	}
	/* silent the warning in reload contxt */
	if (context != CFGCXT_RELOAD)
		ereport(WARNING,
			(errmsg("backend_hostname%d cannot be changed in context %d and backend status = %d",index,context,backend_status)));
	return false;
}

static bool BackendDataDirAssignFunc (ConfigContext context, char* newval, int index, int elevel)
{
	BACKEND_STATUS backend_status = g_pool_config.backend_desc->backend_info[index].backend_status;
	if (context <= CFGCXT_INIT || backend_status == CON_UNUSED || backend_status == CON_DOWN)
	{
		if (newval == NULL || strlen(newval) == 0)
			g_pool_config.backend_desc->backend_info[index].backend_data_directory[0] = '\0';
		else
			strlcpy(g_pool_config.backend_desc->backend_info[index].backend_data_directory, newval, MAX_PATH_LENGTH-1);
		return true;
	}
	/* silent the warning in reload contxt */
	if (context != CFGCXT_RELOAD)
		ereport(WARNING,
				(errmsg("backend_data_directory%d cannot be changed in context %d and backend status = %d",index,context,backend_status)));
	return false;
}

static bool BackendFlagsAssignFunc (ConfigContext context, char* newval, int index, int elevel)
{

	unsigned short flag = 0;
	int i,n;
	bool allow_to_failover_is_specified = false;
	bool disallow_to_failover_is_specified = false;
	char **flags;

	flags = get_list_from_string(newval, "|", &n);
	if (!flags || n < 0)
	{
		if (flags)
			pfree(flags);

		ereport(elevel,
			(errmsg("invalid configuration for key \"backend_flag%d\"",index),
				 errdetail("unable to get backend flags")));
		return false;
	}

	for (i=0;i<n;i++)
	{
		int k;
		if (!strcmp(flags[i], "ALLOW_TO_FAILOVER"))
		{
			if (disallow_to_failover_is_specified)
			{
				for (k = i; k < n; k++)
					pfree(flags[k]);
				pfree(flags);
				ereport(elevel,
					(errmsg("invalid configuration for key \"backend_flag%d\"",index),
						 errdetail("cannot set ALLOW_TO_FAILOVER and DISALLOW_TO_FAILOVER at the same time")));
				return false;
			}
			flag &= ~POOL_FAILOVER;
			allow_to_failover_is_specified = true;

		}
		
		else if (!strcmp(flags[i], "DISALLOW_TO_FAILOVER"))
		{
			if (allow_to_failover_is_specified)
			{
				for (k = i; k < n; k++)
					pfree(flags[k]);
				pfree(flags);

				ereport(elevel,
					(errmsg("invalid configuration for key \"backend_flag%d\"",index),
						 errdetail("cannot set ALLOW_TO_FAILOVER and DISALLOW_TO_FAILOVER at the same time")));
				return false;
			}
			flag |= POOL_FAILOVER;
			disallow_to_failover_is_specified = true;
		}

		else
		{
			ereport(elevel,
				(errmsg("invalid configuration for key \"backend_flag%d\"",index),
					 errdetail("unknown backend flag:%s", flags[i])));
			for (k = i; k < n; k++)
				pfree(flags[k]);
			pfree(flags);
			return false;
		}
		pfree(flags[i]);
	}

	g_pool_config.backend_desc->backend_info[index].flag = flag;
	ereport(DEBUG1,
		(errmsg("setting \"backend_flag%d\" flag: %04x ",index, flag)));
	pfree(flags);
	return true;
}

static bool LogDestinationProcessFunc (char* newval, int elevel)
{
#ifndef POOL_PRIVATE
	char **destinations;
	int n,i;
	int log_destination = 0;
	destinations = get_list_from_string(newval, ",", &n);
	if (!destinations || n < 0)
	{
		if (destinations)
			pfree(destinations);

		ereport(elevel,
			(errmsg("invalid value \"%s\" for log_destination",newval)));
		return false;
	}
	for (i=0;i<n;i++)
	{
		if (!strcmp(destinations[i], "syslog"))
		{
			log_destination |= LOG_DESTINATION_SYSLOG;
		}
		else if (!strcmp(destinations[i], "stderr"))
		{
			log_destination |= LOG_DESTINATION_STDERR;
		}
		else
		{
			int k;
			ereport(elevel,
				(errmsg("invalid configuration for \"log_destination\""),
					errdetail("unknown destination :%s", destinations[i])));
			for (k = i; k < n; k++)
				pfree(destinations[k]);
			pfree(destinations);
			return false;
		}
		pfree(destinations[i]);
	}
	if (g_pool_config.log_destination & LOG_DESTINATION_SYSLOG )
	{
		if (!(log_destination & LOG_DESTINATION_SYSLOG))
			closelog();
	}
	g_pool_config.log_destination = log_destination;
	pfree(destinations);
#endif
	return true;
}

static bool SyslogFacilityProcessFunc (int newval, int elevel)
{
#ifndef POOL_PRIVATE
#ifdef HAVE_SYSLOG
	/* set syslog parameters */
	set_syslog_parameters(g_pool_config.syslog_ident ? g_pool_config.syslog_ident : "pgpool",
							  g_pool_config.syslog_facility);
#endif
#endif
	return true;
}

static bool SyslogIdentProcessFunc (char* newval, int elevel)
{
#ifndef POOL_PRIVATE
#ifdef HAVE_SYSLOG
	/* set syslog parameters */
	set_syslog_parameters(g_pool_config.syslog_ident ? g_pool_config.syslog_ident : "pgpool",
						  g_pool_config.syslog_facility);
#endif
#endif
	return true;
}

static const char* BackendWeightShowFunc(int index)
{
	static char buffer[10];
	snprintf(buffer, sizeof(buffer), "%g" ,
			 g_pool_config.backend_desc->backend_info[index].unnormalized_weight);
	return buffer;
}

static const char* BackendPortShowFunc(int index)
{
	static char buffer[10];
	snprintf(buffer, sizeof(buffer), "%d" ,
			 g_pool_config.backend_desc->backend_info[index].backend_port);
	return buffer;
}

static const char* BackendHostShowFunc(int index)
{
	return g_pool_config.backend_desc->backend_info[index].backend_hostname;
}

static const char* BackendDataDirShowFunc(int index)
{
	return g_pool_config.backend_desc->backend_info[index].backend_data_directory;
}

static const char* BackendFlagsShowFunc(int index)
{
	static char buffer[21];

	unsigned short flag = g_pool_config.backend_desc->backend_info[index].flag;
	if (POOL_ALLOW_TO_FAILOVER(flag))
		snprintf(buffer, sizeof(buffer), "ALLOW_TO_FAILOVER");
	else if (POOL_DISALLOW_TO_FAILOVER(flag))
		snprintf(buffer, sizeof(buffer), "DISALLOW_TO_FAILOVER");
	return buffer;
}

static bool BackendSlotEmptyCheckFunc(int index)
{
	return (g_pool_config.backend_desc->backend_info[index].backend_port == 0);
}

static bool WdSlotEmptyCheckFunc(int index)
{
	return (g_pool_config.wd_remote_nodes.wd_remote_node_info[index].pgpool_port == 0);
}

static bool WdIFSlotEmptyCheckFunc(int index)
{
	
	return (index >= g_pool_config.num_hb_if);
}

static const char* OtherPPHostShowFunc(int index)
{
	return g_pool_config.wd_remote_nodes.wd_remote_node_info[index].hostname;
}

static const char* OtherPPPortShowFunc(int index)
{
	static char buffer[10];
	snprintf(buffer, sizeof(buffer), "%d" ,
			 g_pool_config.wd_remote_nodes.wd_remote_node_info[index].pgpool_port);
	return buffer;
}

static const char* OtherWDPortShowFunc(int index)
{
	static char buffer[10];
	snprintf(buffer, sizeof(buffer), "%d" ,
			 g_pool_config.wd_remote_nodes.wd_remote_node_info[index].wd_port);
	return buffer;
}

static const char* HBDeviceShowFunc(int index)
{
	return g_pool_config.hb_if[index].if_name;
}

static const char* HBDestinationShowFunc(int index)
{
	return g_pool_config.hb_if[index].addr;
}

static const char* HBDestinationPortShowFunc(int index)
{
	static char buffer[10];
	snprintf(buffer, sizeof(buffer), "%d" ,
			 g_pool_config.hb_if[index].dest_port);
	return buffer;
}

/* Watchdog Assign functions */
/*other_pgpool_hostname*/
static bool OtherPPHostAssignFunc (ConfigContext context, char* newval, int index, int elevel)
{
	if (newval == NULL || strlen(newval) == 0)
		g_pool_config.wd_remote_nodes.wd_remote_node_info[index].hostname[0] = '\0';
	else
		strlcpy(g_pool_config.wd_remote_nodes.wd_remote_node_info[index].hostname, newval, MAX_DB_HOST_NAMELEN-1);
	return true;
}

/*other_pgpool_port*/
static bool OtherPPPortAssignFunc (ConfigContext context, int newval, int index, int elevel)
{
	g_pool_config.wd_remote_nodes.wd_remote_node_info[index].pgpool_port = newval;
	return true;
}

/*other_wd_port*/
static bool OtherWDPortAssignFunc (ConfigContext context, int newval, int index, int elevel)
{
	g_pool_config.wd_remote_nodes.wd_remote_node_info[index].wd_port = newval;
	return true;
}

/*heartbeat_device*/
static bool HBDeviceAssignFunc (ConfigContext context, char* newval, int index, int elevel)
{
	if (newval == NULL || strlen(newval) == 0)
		g_pool_config.hb_if[index].if_name[0] = '\0';
	else
		strlcpy(g_pool_config.hb_if[index].if_name, newval, WD_MAX_IF_NAME_LEN);
	return true;
}

/*heartbeat_destination*/
static bool HBDestinationAssignFunc (ConfigContext context, char* newval, int index, int elevel)
{
	if (newval == NULL || strlen(newval) == 0)
		g_pool_config.hb_if[index].addr[0] = '\0';
	else
		strlcpy(g_pool_config.hb_if[index].addr, newval, WD_MAX_HOST_NAMELEN -1);
	return true;
}

/*heartbeat_destination_port*/
static bool HBDestinationPortAssignFunc (ConfigContext context, int newval, int index, int elevel)
{
	g_pool_config.hb_if[index].dest_port = newval;
	return true;
}

/*
 * Check DB node spec. node spec should be either "primary", "standby" or
 * numeric DB node id.
 */
static bool check_redirect_node_spec(char *node_spec)
{
	int len = strlen(node_spec);
	int i;
	int64 val;
	
	if (len <= 0)
		return false;
	
	if (strcasecmp("primary", node_spec) == 0)
	{
		return true;
	}
	
	if (strcasecmp("standby", node_spec) == 0)
	{
		return true;
	}

	for (i=0;i<len;i++)
	{
		if (!isdigit((int)node_spec[i]))
			return false;
	}

	val = pool_atoi64(node_spec);
	
	if (val >=0 && val < MAX_NUM_BACKENDS)
		return true;

	return false;
}

static bool config_post_processor(ConfigContext context, int elevel)
{
	double total_weight = 0.0;
	sig_atomic_t local_num_backends = 0;
	int i;

	if (context == CFGCXT_BOOT)
	{
		char localhostname[256];
		int res = gethostname(localhostname,sizeof(localhostname));
		if(res !=0 )
		{
			ereport(WARNING,
				(errmsg("initializing pool configuration"),
					 errdetail("failed to get the local hostname")));
			return false;
		}
		g_pool_config.wd_hostname = pstrdup(localhostname);
		return true;
	}
	for (i=0;i<MAX_CONNECTION_SLOTS;i++)
	{
		BackendInfo *backend_info = &g_pool_config.backend_desc->backend_info[i];

		/* port number == 0 indicates that this server is out of use */
		if (backend_info->backend_port == 0)
		{
			*backend_info->backend_hostname = '\0';
			backend_info->backend_status = CON_UNUSED;
			backend_info->backend_weight = 0.0;
		}
		else
		{
			total_weight += backend_info->unnormalized_weight;
			local_num_backends = i+1;
			
			/* initialize backend_hostname with a default socket path if empty */
			if (*(backend_info->backend_hostname) == '\0')
			{
				ereport(LOG,
					(errmsg("initializing pool configuration"),
						 errdetail("empty backend_hostname%d, use PostgreSQL's default unix socket path (%s)",
								   i, DEFAULT_SOCKET_DIR)));
				strlcpy(backend_info->backend_hostname, DEFAULT_SOCKET_DIR, MAX_DB_HOST_NAMELEN);
			}
		}
	}

	if (local_num_backends != pool_config->backend_desc->num_backends)
		pool_config->backend_desc->num_backends = local_num_backends;

	ereport(DEBUG1,
		(errmsg("initializing pool configuration"),
			 errdetail("num_backends: %d total_weight: %f",
					   pool_config->backend_desc->num_backends, total_weight)));
	/*
	 * Normalize load balancing weights. What we are doing here is,
	 * assign 0 to RAND_MAX to each backend's weight according to the
	 * value weightN.  For example, if two backends are assigned 1.0,
	 * then each backend will get RAND_MAX/2 normalized weight.
	 */
	for (i=0;i<MAX_CONNECTION_SLOTS;i++)
	{
		BackendInfo *backend_info = &g_pool_config.backend_desc->backend_info[i];

		if (backend_info->backend_port != 0)
		{
			backend_info->backend_weight =
			(RAND_MAX) * backend_info->unnormalized_weight / total_weight;

			ereport(DEBUG1,
					(errmsg("initializing pool configuration"),
					 errdetail("backend %d weight: %f flag: %04x", i, backend_info->backend_weight,backend_info->flag)));
		}
	}

	/* Set the number of configured Watchdog nodes */
	g_pool_config.wd_remote_nodes.num_wd = 0;
	for (i=0; i< MAX_WATCHDOG_NUM; i++ )
	{
		WdRemoteNodeInfo* wdNode = &g_pool_config.wd_remote_nodes.wd_remote_node_info[i];
		if (wdNode->wd_port > 0)
			g_pool_config.wd_remote_nodes.num_wd = i + 1;
	}

	/* Set the number of configured heartbeat interfaces */
	g_pool_config.num_hb_if = 0;
	for (i=0; i< WD_MAX_IF_NUM; i++ )
	{
		if (g_pool_config.hb_if[i].dest_port > 0)
			g_pool_config.num_hb_if = i + 1;
	}


	if (strcmp(pool_config->recovery_1st_stage_command, "") ||
		strcmp(pool_config->recovery_2nd_stage_command, ""))
	{
		for (i=0;i<MAX_CONNECTION_SLOTS;i++)
		{
			BackendInfo *backend_info = &g_pool_config.backend_desc->backend_info[i];

			if (backend_info->backend_port != 0 &&
				!strcmp(backend_info->backend_data_directory, ""))
			{
				ereport(elevel,
						(errmsg("invalid configuration, recovery_1st_stage_command and recovery_2nd_stage_command requires backend_data_directory to be set")));
				return false;
			}
		}
	}

	return true;
}

static bool MakeAppRedirectListRegex (char* newval, int elevel)
{
	/* TODO Deal with the memory */
	int i;
	Left_right_tokens *lrtokens;
	
	if (newval == NULL)
	{
		pool_config->redirect_app_names = NULL;
		pool_config->app_name_redirect_tokens = NULL;
		return true;
	}
	
	lrtokens = create_lrtoken_array();
	extract_string_tokens2(newval, ",", ':', lrtokens);
	
	pool_config->redirect_app_names = create_regex_array();
	pool_config->app_name_redirect_tokens = lrtokens;
	
	for (i=0;i<lrtokens->pos;i++)
	{
		if (!check_redirect_node_spec(lrtokens->token[i].right_token))
		{
			ereport(elevel,
					(errmsg("invalid configuration for key \"app_name_redirect_preference_list\""),
					 errdetail("wrong redirect db node spec: \"%s\"", lrtokens->token[i].right_token)));
			return false;
		}
		
		
		if (*(lrtokens->token[i].left_token) == '\0' ||
			add_regex_array(pool_config->redirect_app_names, lrtokens->token[i].left_token))
		{
			ereport(elevel,
					(errmsg("invalid configuration for key \"app_name_redirect_preference_list\""),
					 errdetail("wrong redirect app name regular expression: \"%s\"", lrtokens->token[i].left_token)));
			return false;
		}
	}
	
	return true;
}

static bool MakeDBRedirectListRegex (char* newval, int elevel)
{
	/* TODO Deal with the memory */
	int i;
	Left_right_tokens *lrtokens;
	if (newval == NULL)
	{
		pool_config->redirect_dbnames = NULL;
		pool_config->db_redirect_tokens = NULL;
		return true;
	}
	
	lrtokens = create_lrtoken_array();
	extract_string_tokens2(newval, ",", ':', lrtokens);
	
	pool_config->redirect_dbnames = create_regex_array();
	pool_config->db_redirect_tokens = lrtokens;
	
	for (i=0;i<lrtokens->pos;i++)
	{
		if (!check_redirect_node_spec(lrtokens->token[i].right_token))
		{
			ereport(elevel,
				(errmsg("invalid configuration for key \"database_redirect_preference_list\""),
					 errdetail("wrong redirect db node spec: \"%s\"", lrtokens->token[i].right_token)));
			return false;
		}
		
		if (*(lrtokens->token[i].left_token) == '\0' ||
			add_regex_array(pool_config->redirect_dbnames, lrtokens->token[i].left_token))
		{
			ereport(elevel,
				(errmsg("invalid configuration for key \"database_redirect_preference_list\""),
					 errdetail("wrong redirect dbname regular expression: \"%s\"", lrtokens->token[i].left_token)));
			return false;
		}
	}
	return true;
}

#ifndef POOL_PRIVATE

/*
 * Lookup the name for an enum option with the selected value.
 * The returned string is a pointer to static data and not
 * allocated for modification.
 */
static const char *
config_enum_lookup_by_value(struct config_enum * record, int val)
{
	const struct config_enum_entry *entry;

	for (entry = record->options; entry && entry->name; entry++)
	{
		if (entry->val == val)
			return entry->name;
	}
	/* should never happen*/
	return NULL;
}

static char *
ShowOption(struct config_generic * record, int index, int elevel)
{
	char		buffer[256];
	const char *val;
	
	/* if the value needs to be hidden
	 * no need to dig further
	 */
	if (record->flags & VAR_HIDDEN_VALUE)
		return pstrdup("*****");

	switch (record->vartype)
	{
		case CONFIG_VAR_TYPE_BOOL:
		{
			struct config_bool *conf = (struct config_bool *) record;
			
			if (conf->show_hook)
				val = (*conf->show_hook) ();
			else
				val = *conf->variable ? "on" : "off";
		}
			break;
			
		case CONFIG_VAR_TYPE_INT:
		{
			struct config_int *conf = (struct config_int *) record;
			
			if (conf->show_hook)
				val = (*conf->show_hook) ();
			else
			{
				int		result = *conf->variable;
				snprintf(buffer, sizeof(buffer), "%d" ,
						 result);
				val = buffer;
			}
		}
			break;

		case CONFIG_VAR_TYPE_LONG:
		{
			struct config_long *conf = (struct config_long *) record;

			if (conf->show_hook)
				val = (*conf->show_hook) ();
			else
			{
				int64		result = (int64)*conf->variable;
				snprintf(buffer, sizeof(buffer), INT64_FORMAT ,
						 result);
				val = buffer;
			}
		}
			break;
			
		case CONFIG_VAR_TYPE_DOUBLE:
		{
			struct config_double *conf = (struct config_double *) record;
			
			if (conf->show_hook)
				val = (*conf->show_hook)();
			else
			{
				snprintf(buffer, sizeof(buffer), "%g",
						 *conf->variable);
				val = buffer;
			}
		}
			break;
			
		case CONFIG_VAR_TYPE_STRING:
		{
			struct config_string *conf = (struct config_string *) record;
			
			if (conf->show_hook)
				val = (*conf->show_hook) ();
			else if (*conf->variable && **conf->variable)
				val = *conf->variable;
			else
				val = "";
		}
			break;
			
		case CONFIG_VAR_TYPE_ENUM:
		{
			struct config_enum *conf = (struct config_enum *) record;
			
			if (conf->show_hook)
				val = (*conf->show_hook) ();
			else
				val = config_enum_lookup_by_value(conf, *conf->variable);
		}

			break;

		case CONFIG_VAR_TYPE_INT_ARRAY:
		{
			struct config_int_array *conf = (struct config_int_array *) record;
			
			if (index >= conf->max_elements || index < 0)
			{
				ereport(elevel,
					(errmsg("invalid index %d for configuration parameter \"%s\"",index, conf->gen.name),
						 errdetail("allowed index is between 0 and %d",conf->max_elements -1)));

				val = NULL;
			}
			else
			{
				if (conf->show_hook)
					val = (*conf->show_hook) (index);
				else
				{
					int		result = *conf->variable[index];
					snprintf(buffer, sizeof(buffer), "%d" ,
							 result);
					val = buffer;
				}
			}
		}
			break;

		case CONFIG_VAR_TYPE_DOUBLE_ARRAY:
		{
			struct config_double_array *conf = (struct config_double_array *) record;
			
			if (index >= conf->max_elements || index < 0)
			{
				ereport(elevel,
					(errmsg("invalid index %d for configuration parameter \"%s\"",index, conf->gen.name),
						 errdetail("allowed index is between 0 and %d",conf->max_elements -1)));
				
				val = NULL;
			}
			else
			{
				if (conf->show_hook)
					val = (*conf->show_hook) (index);
				else
				{
					double		result = *conf->variable[index];
					snprintf(buffer, sizeof(buffer), "%g" ,
							 result);
					val = buffer;
				}
			}
		}
			break;

		case CONFIG_VAR_TYPE_STRING_ARRAY:
		{
			struct config_string_array *conf = (struct config_string_array *) record;
			
			if (index >= conf->max_elements || index < 0)
			{
				ereport(elevel,
					(errmsg("invalid index %d for configuration parameter \"%s\"",index, conf->gen.name),
						 errdetail("allowed index is between 0 and %d",conf->max_elements -1)));
				
				val = NULL;
			}
			else
			{
				if (conf->show_hook)
					val = (*conf->show_hook) (index);
				else if ( (*conf->variable)[index] && ***conf->variable)
					val = (*conf->variable)[index];
				else
					val = "";
			}
		}
			break;
		case CONFIG_VAR_TYPE_STRING_LIST:
		{
			struct config_string_list *conf = (struct config_string_list *) record;
			
			if (conf->show_hook)
				val = (*conf->show_hook) ();
			else if (conf->current_val)
				val = conf->current_val;
			else
				val = "";
		}
			break;
			
		default:
			/* just to keep compiler quiet */
			val = "???";
			break;
	}
	if (val)
		return pstrdup(val);
	return NULL;
}


static bool value_slot_for_config_record_is_empty(struct config_generic* record, int index)
{
	switch (record->vartype)
	{
		case CONFIG_VAR_TYPE_BOOL:
		case CONFIG_VAR_TYPE_INT:
		case CONFIG_VAR_TYPE_LONG:
		case CONFIG_VAR_TYPE_DOUBLE:
		case CONFIG_VAR_TYPE_STRING:
		case CONFIG_VAR_TYPE_ENUM:
			return false;
			break;
			
		case CONFIG_VAR_TYPE_INT_ARRAY:
		{
			struct config_int_array *conf = (struct config_int_array *) record;
			if (conf->empty_slot_check_func)
				return (*conf->empty_slot_check_func)(index);
		}
			break;
			
		case CONFIG_VAR_TYPE_DOUBLE_ARRAY:
		{
			struct config_double_array *conf = (struct config_double_array *) record;
			if (conf->empty_slot_check_func)
				return (*conf->empty_slot_check_func)(index);
		}
			break;
			
		case CONFIG_VAR_TYPE_STRING_ARRAY:
		{
			struct config_string_array *conf = (struct config_string_array *) record;
			if (conf->empty_slot_check_func)
				return (*conf->empty_slot_check_func)(index);
		}
			break;
			
		default:
			break;
	}
	return false;
}

static int get_max_elements_for_config_record(struct config_generic* record)
{
	switch (record->vartype)
	{
		case CONFIG_VAR_TYPE_BOOL:
		case CONFIG_VAR_TYPE_INT:
		case CONFIG_VAR_TYPE_LONG:
		case CONFIG_VAR_TYPE_DOUBLE:
		case CONFIG_VAR_TYPE_STRING:
		case CONFIG_VAR_TYPE_ENUM:
			return 1;
			break;
			
		case CONFIG_VAR_TYPE_INT_ARRAY:
		{
			struct config_int_array *conf = (struct config_int_array *) record;
			return conf->max_elements;
		}
			break;
			
		case CONFIG_VAR_TYPE_DOUBLE_ARRAY:
		{
			struct config_double_array *conf = (struct config_double_array *) record;
			return conf->max_elements;
		}
			break;
			
		case CONFIG_VAR_TYPE_STRING_ARRAY:
		{
			struct config_string_array *conf = (struct config_string_array *) record;
			return conf->max_elements;
		}
			break;
			
		default:
			break;
	}
	return 0;
}

bool set_config_option_for_session(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, const char *name, const char *value)
{
	if (set_one_config_option(name, value, CFGCXT_SESSION, PGC_S_SESSION, FRONTEND_ONLY_ERROR) == true)
	{
		send_complete_and_ready(frontend, backend, "SET", -1);
		return true;
	}
	return false;
}

bool reset_all_variables(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	int i;
	int elevel = (frontend == NULL)?FATAL:FRONTEND_ONLY_ERROR;

	ereport(DEBUG2,
		(errmsg("RESET ALL CONFIG VARIABLES")));

	for (i = 0; i < num_all_parameters; ++i)
	{
		struct config_generic* record = all_parameters[i];
		/* do nothing if variable is not changed in session */
		if (record->scontext != CFGCXT_SESSION)
			continue;
		/* Don't reset if special exclusion from RESET ALL */
		if (record->flags & VAR_NO_RESET_ALL)
			continue;

		
		if (record->dynamic_array_var)
		{
			int max_elements = get_max_elements_for_config_record(record);
			int index;
			for (index = 0; index < max_elements; index++)
			{
				if (value_slot_for_config_record_is_empty(record,index))
					continue;

				setConfigOptionVar(record, record->name, index, NULL,
								   CFGCXT_SESSION, PGC_S_FILE, elevel);
			}
		}
		else
		{
			setConfigOptionVar(record, record->name, -1, NULL,
							   CFGCXT_SESSION, PGC_S_FILE, elevel);
		}
	}
	if (frontend)
		send_complete_and_ready(frontend, backend, "RESET", -1);
	return true;
}

bool report_all_variables(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	int i;
	int num_rows = 0;
	const char* value;

	send_row_description_for_detail_view(frontend, backend);

	/*
	 * grouped variables are not listed in all parameter array
	 * so start with group variables.
	 */
	for (i = 0; ConfigureVarGroups[i].gen.name; i++)
	{
		int rows = send_grouped_type_variable_to_frontend(&ConfigureVarGroups[i], frontend, backend);
		if (rows < 0)
			return false;
		num_rows += rows;
	}

	for (i = 0; i < num_all_parameters; ++i)
	{
		struct config_generic* record = all_parameters[i];
		if (record->flags & VAR_PART_OF_GROUP)
			continue;

		if (record->flags & VAR_HIDDEN_IN_SHOW_ALL)
			continue;

		if (record->dynamic_array_var)
		{
			int rows = send_array_type_variable_to_frontend(record, frontend, backend);
			if (rows < 0)
				return false;
			num_rows += rows;

		}
		else
		{
			value = ShowOption(record, 0, FRONTEND_ONLY_ERROR);
			if (value == NULL)
				return false;
			send_config_var_detail_row(frontend, backend, record->name, value, record->description);
			pfree((void*)value);
			num_rows++;
		}
	}
	send_complete_and_ready(frontend, backend, "SELECT", num_rows);
	return true;
}


bool report_config_variable(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, const char* var_name)
{
	int index = 0;
	char *value;
	int num_rows = 0;
	struct config_generic* record;

	if ( strcmp(var_name,"all") == 0)
		return report_all_variables(frontend,backend);

	/* search the variable */
	record = find_option(var_name, WARNING);
	if (record == NULL)
	{
		int i;
		/* check if the var name match the grouped var */
		for (i = 0; ConfigureVarGroups[i].gen.name; i++)
		{
			if (strcmp(ConfigureVarGroups[i].gen.name,var_name) == 0)
			{
				send_row_description_for_detail_view(frontend, backend);
				num_rows = send_grouped_type_variable_to_frontend(&ConfigureVarGroups[i], frontend, backend);
				if (num_rows < 0)
					return false;
				send_complete_and_ready(frontend, backend, "SELECT", num_rows);
				return true;
			}
		}

		ereport(FRONTEND_ONLY_ERROR,
			(errmsg("config variable \"%s\" not found",var_name)));

		return false;
	}

	if (record->dynamic_array_var)
	{
		if (get_index_in_var_name(record, var_name, &index, FRONTEND_ONLY_ERROR) == false)
			return false;

		if (index < 0)
		{
			/* Index is not included in parameter name.
			 * this is the multi value config variable */
			ereport(DEBUG3,
					(errmsg("show parameter \"%s\" with out index",var_name)));
			send_row_description_for_detail_view(frontend, backend);
			num_rows = send_array_type_variable_to_frontend(record, frontend, backend);
			if (num_rows < 0)
				return false;
		}
	}

	if (index >= 0)
	{
		/* single value only */
		num_rows = 1;
		send_row_description(frontend, backend, 1, (char**)&var_name);
		value = ShowOption(record, index, FRONTEND_ONLY_ERROR);
		if (value == NULL)
			return false;
		send_config_var_value_only_row(frontend, backend, value);
		pfree((void*)value);
	}

	send_complete_and_ready(frontend, backend, "SELECT" ,num_rows);

	return true;
}

static int send_array_type_variable_to_frontend(struct config_generic* record, POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	if (record->dynamic_array_var)
	{
		const int MAX_NAME_LEN = 255;
		char name[MAX_NAME_LEN +1];
		int max_elements = get_max_elements_for_config_record(record);
		int index;
		int num_rows = 0;

		for (index = 0; index < max_elements; index++)
		{
			const char *value;
			if (value_slot_for_config_record_is_empty(record,index))
				continue;
			/* construct the name with index */
			snprintf(name, MAX_NAME_LEN, "%s%d",record->name,index);
			value = ShowOption(record, index, FRONTEND_ONLY_ERROR);
			if (value == NULL)
				return -1;
			send_config_var_detail_row(frontend, backend, (const char*)name, value, record->description);
			pfree((void*)value);
			num_rows++;
		}
		return num_rows;
	}
	return -1; /* error */
}

static int send_grouped_type_variable_to_frontend(struct config_grouped_array_var* grouped_record, POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	int k,index;
	int max_elements = get_max_elements_for_config_record(grouped_record->var_list[0]);
	int num_rows = 0;
	for (index = 0; index < max_elements; index++)
	{
		for (k =0; k < grouped_record->var_count; k++)
		{
			const int MAX_NAME_LEN = 255;
			char name[MAX_NAME_LEN +1];
			const char *value;

			struct config_generic* record = grouped_record->var_list[k];

			if (value_slot_for_config_record_is_empty(record,index))
				break;
			/* construct the name with index */
			snprintf(name, MAX_NAME_LEN, "%s%d",record->name,index);
			value = ShowOption(record, index,FRONTEND_ONLY_ERROR);
			if (value == NULL)
				return -1;
			send_config_var_detail_row(frontend, backend, (const char*)name, value, record->description);
			pfree((void*)value);
			num_rows++;
		}
	}

	return num_rows;
}

static void send_row_description_for_detail_view(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	static char *field_names[] = {"item", "value", "description"};
	send_row_description(frontend, backend, 3, field_names);
}
#endif /* not defined POOL_FRONTEND*/
