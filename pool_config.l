/* -*-pgsql-c-*- */
/*
 *
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
 * pool_config.l: read configuration file
 *
 */

%{

#include "pool.h"
#include "pool_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_CONTEXT(mask, context) ((mask) & (context))

/* to shut off compiler warnings */
int yylex(void);

POOL_CONFIG *pool_config;	/* configuration values */
POOL_SYSTEMDB_CONNECTION_POOL *system_db_info;
static unsigned Lineno;
static char *default_reset_query_list[] = {"ABORT", "DISCARD ALL"};
static char *default_black_function_list[] = {"nextval", "setval"};

typedef enum {
  POOL_KEY = 1,
  POOL_INTEGER,
  POOL_REAL,
  POOL_STRING,
  POOL_UNQUOTED_STRING,
  POOL_EQUALS,
  POOL_EOL,
  POOL_PARSE_ERROR
} POOL_TOKEN;

static char *extract_string(char *value, POOL_TOKEN token);
static char **extract_string_tokens(char *str, char *delim, int *n);
static void clear_host_entry(int slot);

%}

%option 8bit
%option never-interactive
%option nounput
%option noyywrap

SIGN            ("-"|"+")
DIGIT           [0-9]
HEXDIGIT        [0-9a-fA-F]

INTEGER         {SIGN}?({DIGIT}+|0x{HEXDIGIT}+)

EXPONENT        [Ee]{SIGN}?{DIGIT}+
REAL            {SIGN}?{DIGIT}*"."{DIGIT}*{EXPONENT}?

LETTER          [A-Za-z_\200-\377]
LETTER_OR_DIGIT [A-Za-z_0-9\200-\377]

KEY              {LETTER}{LETTER_OR_DIGIT}*

UNQUOTED_STRING {LETTER}({LETTER_OR_DIGIT}|[-._:/])*
STRING          \'([^'\n]|\\.)*\'

%%

\n              Lineno++; return POOL_EOL;
[ \t\r]+        /* eat whitespace */
#.*$            /* eat comment */

{KEY}           return POOL_KEY;
{STRING}        return POOL_STRING;
{UNQUOTED_STRING} return POOL_UNQUOTED_STRING;
{INTEGER}       return POOL_INTEGER;
{REAL}          return POOL_REAL;
=               return POOL_EQUALS;

.               return POOL_PARSE_ERROR;

%%

int pool_init_config(void)
{
	int res;
	static char localhostname[256];
	int i;

	pool_config = malloc(sizeof(POOL_CONFIG));
	if (pool_config == NULL)
	{
		pool_error("failed to allocate pool_config");
		return(-1);
	}

	memset(pool_config, 0, sizeof(POOL_CONFIG));

#ifndef POOL_PRIVATE
	pool_config->backend_desc = pool_shared_memory_create(sizeof(BackendDesc));
	if (pool_config->backend_desc == NULL)
	{
		pool_error("failed to allocate pool_config->backend_desc");
		return -1;
	}
#else
	pool_config->backend_desc = malloc(sizeof(BackendDesc));
	if (pool_config->backend_desc == NULL)
	{
		pool_error("failed to allocate pool_config->backend_desc");
		return -1;
	}
#endif

	/* set hardcoded default values */
	pool_config->listen_addresses = "localhost";
	pool_config->port = 9999;
	pool_config->pcp_port = 9898;
	pool_config->socket_dir = DEFAULT_SOCKET_DIR;
	pool_config->pcp_socket_dir = DEFAULT_SOCKET_DIR;
	pool_config->backend_socket_dir = NULL;
	pool_config->pcp_timeout = 10;
	pool_config->num_init_children = 32;
	pool_config->max_pool = 4;
	pool_config->child_life_time = 300;
	pool_config->client_idle_limit = 0;
	pool_config->connection_life_time = 0;
	pool_config->child_max_connections = 0;
	pool_config->authentication_timeout = 60;
	pool_config->logdir = DEFAULT_LOGDIR;
        pool_config->logsyslog = 0;
        pool_config->log_destination = "stderr";
        pool_config->syslog_facility = LOG_LOCAL0;
        pool_config->syslog_ident = "pgpool";
	pool_config->pid_file_name = DEFAULT_PID_FILE_NAME;
 	pool_config->log_statement = 0;
 	pool_config->log_per_node_statement = 0;
	pool_config->log_connections = 0;
	pool_config->log_hostname = 0;
	pool_config->enable_pool_hba = 0;
	pool_config->pool_passwd = "pool_passwd";

	pool_config->replication_mode = 0;
	pool_config->load_balance_mode = 0;
	pool_config->replication_stop_on_mismatch = 0;
	pool_config->failover_if_affected_tuples_mismatch = 0;
	pool_config->replicate_select = 0;
	pool_config->reset_query_list = default_reset_query_list;
	pool_config->num_reset_queries = sizeof(default_reset_query_list)/sizeof(char *);
	pool_config->white_function_list = NULL;
	pool_config->num_white_function_list = 0;
	pool_config->black_function_list = default_black_function_list;
	pool_config->num_black_function_list = sizeof(default_black_function_list)/sizeof(char *);
	pool_config->print_timestamp = 1;
	pool_config->master_slave_mode = 0;
	pool_config->master_slave_sub_mode = "slony";
	pool_config->delay_threshold = 0;
	pool_config->log_standby_delay = 0;
	pool_config->connection_cache = 1;
	pool_config->health_check_timeout = 20;
	pool_config->health_check_period = 0;
	pool_config->health_check_user = "nobody";
	pool_config->failover_command = "";
	pool_config->follow_master_command = "";
	pool_config->failback_command = "";
	pool_config->fail_over_on_backend_error = 1;
	pool_config->insert_lock = 1;
	pool_config->ignore_leading_white_space = 1;
	pool_config->parallel_mode = 0;
	pool_config->enable_query_cache = 0;
	pool_config->system_db_hostname = "localhost";
	pool_config->system_db_port = 5432;
	pool_config->system_db_dbname = "pgpool";
	pool_config->system_db_schema = "pgpool_catalog";
	pool_config->system_db_user = "pgpool";
	pool_config->system_db_password = "";
	pool_config->backend_desc->num_backends = 0;
    pool_config->recovery_user = "";
    pool_config->recovery_password = "";
    pool_config->recovery_1st_stage_command = "";
    pool_config->recovery_2nd_stage_command = "";
	pool_config->recovery_timeout = 90;
	pool_config->client_idle_limit_in_recovery = 0;
	pool_config->lobj_lock_table = "";
	pool_config->ssl = 0;
	pool_config->ssl_cert = "";
	pool_config->ssl_key = "";
	pool_config->ssl_ca_cert = "";
	pool_config->ssl_ca_cert_dir = "";
	pool_config->debug_level = 0;
	pool_config->relcache_expire = 0;
	pool_config->lists_patterns = NULL;
	pool_config->pattc = 0;
	pool_config->current_pattern_size = 0;

	res = gethostname(localhostname,sizeof(localhostname));
	if(res !=0 )
	{
		pool_debug("faild to get this hostname");
	}
	pool_config->pgpool2_hostname = localhostname;

	for (i=0;i<MAX_CONNECTION_SLOTS;i++)
	{
		clear_host_entry(i);
	}
	return 0;
}

/* 
 * Add regex expression to patterns array
 * The supported type are: black_function_list and white_function_list
 * Return 0 on error, 1 on success
 */
int add_regex_pattern(char *type, char *s)
{
	int regex_flags = REG_NOSUB;
	RegPattern currItem;
	/* force case insensitive pattern matching */
	regex_flags |= REG_ICASE;
	/* Add extended regex search */
	regex_flags |= REG_EXTENDED;
	/* Fill the pattern type */
	if (strcmp(type, "black_function_list") == 0)
	{
		currItem.type = BLACKLIST;
	}
	else if (strcmp(type, "white_function_list") == 0)
	{
		currItem.type = WHITELIST;
	}
	else
	{
		pool_error("add_to_patterns: bad pattern type %s", type);
		return 0;
	}
	/* Fill the pattern flag */
	currItem.flag = regex_flags;

	/* Fill pattern array */
	currItem.pattern = malloc(sizeof(char)*(strlen(s)+1));
	if (currItem.pattern == NULL)
	{
		pool_error("add_to_patterns: unable to allocate new pattern");
		return 0;
	}
	/* Force exact matching of function name with ^ and $ on the regex
	   if required to prevent partial matching. It also allow backward
	   compatibility.
	 */
	if (strncmp(s, "^", 1) != 0) {
		strncpy(currItem.pattern, "^", 2);
		strncat(currItem.pattern, s, strlen(s) + 1);
	} else {
		strncpy(currItem.pattern, s, strlen(s) + 1);
	}
	if (s[strlen(s)-1] != '$') {
		strncat(currItem.pattern, "$", 2);
	}
	pool_debug("add_to_patterns: regex pattern: %s", currItem.pattern);
	/* compile our regex */
	if (regcomp(&currItem.regexv, currItem.pattern, currItem.flag) != 0)
	{
		pool_error("add_to_patterns: invalid regex pattern: %s", currItem.pattern);
	}
	else
	{
		if (growPatternArray(currItem) < 0)
		{
			pool_error("add_to_patterns: unable to allocate new pattern");
			return 0;
		}
	}
	return 1;
}

/* 
 * Dynamically grow the regex pattern array
 * The array start with PATTERN_ARR_SIZE storage place, if required
 * it will grow of PATTERN_ARR_SIZE more each time.
 */
int growPatternArray (RegPattern item)
{
	void *_tmp = NULL;
	if (pool_config->pattc == pool_config->current_pattern_size)
	{
		pool_config->current_pattern_size += PATTERN_ARR_SIZE;
		_tmp = realloc(pool_config->lists_patterns, (pool_config->current_pattern_size * sizeof(RegPattern)));
		if (!_tmp)
		{
			return(-1);
		}

		pool_config->lists_patterns = (RegPattern*)_tmp;
	}
	pool_config->lists_patterns[pool_config->pattc] = item;
	pool_config->pattc++;

	return(pool_config->pattc);
}

int pool_get_config(char *confpath, POOL_CONFIG_CONTEXT context)
{
	FILE *fd;
	int token;
	char key[1024];
	double total_weight;
	int i;
    bool log_destination_changed = false;

#define PARSE_ERROR()		pool_error("pool_config: parse error at line %d '%s'", Lineno, yytext)

	/* open config file */
	fd = fopen(confpath, "r");
	if (!fd)
	{
		fprintf(stderr, "pool_config: could not open configuration file (%s)\n",
				POOL_CONF_FILE_NAME);
		fprintf(stderr, "pool_config: using default values...\n");
		return 0;
	}

	yyin = fd;
	Lineno = 1;

	for(;;)
	{
		token = yylex();
		if (token == 0)
		{
			break; 	 
		}
		if (token == POOL_PARSE_ERROR)
		{
			PARSE_ERROR();
			fclose(fd);
			return(-1);
		}
		if (token == POOL_EOL)
			continue;

		if (token != POOL_KEY)
		{
			PARSE_ERROR();
			fclose(fd);
			return(-1);
		}

		strncpy(key, yytext, sizeof(key));

		pool_debug("key: %s", key);

		token = yylex();

		if (token == POOL_EQUALS)
			token = yylex();

		pool_debug("value: %s kind: %d", yytext, token);

		if (!strcmp(key, "allow_inet_domain_socket") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			/* for backward compatibility */
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			if (v)
				pool_config->listen_addresses = strdup("*");
			else
				pool_config->listen_addresses = strdup("");
		}
		else if (!strcmp(key, "listen_addresses") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->listen_addresses = str;
		}

		else if (!strcmp(key, "port") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 1024)
			{
				pool_error("pool_config: %s must be 1024 or higher numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->port = v;
		}
		else if (!strcmp(key, "pcp_port") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 1024)
			{
				pool_error("pool_config: %s must be 1024 or higher numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->pcp_port = v;
		}
		else if (!strcmp(key, "socket_dir") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->socket_dir = str;
		}
		else if (!strcmp(key, "pcp_socket_dir") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->pcp_socket_dir = str;
		}
		else if (!strcmp(key, "pcp_timeout") &&
			 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be equal or greater or equal to 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->pcp_timeout = v;
		}
		else if (!strcmp(key, "num_init_children") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 1)
			{
				pool_error("pool_config: %s must be higher than 1 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->num_init_children = v;
		}
		else if (!strcmp(key, "child_life_time") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be greater or equal to 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->child_life_time = v;
		}
		else if (!strcmp(key, "client_idle_limit") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be greater or equal to 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->client_idle_limit = v;
		}
		else if (!strcmp(key, "connection_life_time") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be greater or equal to 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->connection_life_time = v;
		}
		else if (!strcmp(key, "child_max_connections") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be greater or equal to 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->child_max_connections = v;
		}
		else if (!strcmp(key, "authentication_timeout") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be higher than 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->authentication_timeout = v;
		}
		else if (!strcmp(key, "max_pool") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be greater or equal to 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->max_pool = v;
		}
		else if (!strcmp(key, "logdir") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->logdir = str;
		}
		else if (!strcmp(key, "log_destination") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
            log_destination_changed = pool_config->log_destination != str;
			pool_config->log_destination = str;
		}
		else if (!strcmp(key, "syslog_facility") && CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->syslog_facility = set_syslog_facility(str);
		}
		else if (!strcmp(key, "syslog_ident") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
            log_destination_changed = log_destination_changed || pool_config->syslog_ident != str;
			pool_config->syslog_ident = str;
		}
		else if (!strcmp(key, "pid_file_name") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->pid_file_name = str;
		}
       	else if (!strcmp(key, "log_connections") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_config->log_connections = v;
		}
       	else if (!strcmp(key, "log_hostname") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_config->log_hostname = v;
		}
       	else if (!strcmp(key, "enable_pool_hba") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_config->enable_pool_hba = v;
		}
		else if (!strcmp(key, "pool_passwd") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->pool_passwd = str;
		}

		else if (!strcmp(key, "backend_socket_dir") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;
			
			pool_log("pool_config: backend_socket_dir is deprecated, please use backend_hostname");

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->backend_socket_dir = str;
		}
		else if (!strcmp(key, "replication_mode") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_config->replication_mode = v;

			if (pool_config->master_slave_mode && pool_config->replication_mode)
			{
				pool_error("pool_config: replication_mode and master_slave_mode cannot be enabled at the same time");
				fclose(fd);
				return(-1);
			}

		}
		else if (!strcmp(key, "load_balance_mode") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_config->load_balance_mode = v;
		}
		else if (!strcmp(key, "replication_stop_on_mismatch") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_debug("replication_stop_on_mismatch: %d", v);
			pool_config->replication_stop_on_mismatch = v;
		}
		else if (!strcmp(key, "failover_if_affected_tuples_mismatch") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_debug("failover_if_affected_tuples_mismatch: %d", v);
			pool_config->failover_if_affected_tuples_mismatch = v;
		}
		else if (!strcmp(key, "replicate_select") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_debug("replicate_select: %d", v);
			pool_config->replicate_select = v;
		}
		else if (!strcmp(key, "reset_query_list") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->reset_query_list = extract_string_tokens(str, ";", &pool_config->num_reset_queries);
			if (pool_config->reset_query_list == NULL)
			{
				fclose(fd);
				return(-1);
			}
		}

		else if (!strcmp(key, "white_function_list") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->white_function_list =
				extract_string_tokens(str, ",", &pool_config->num_white_function_list);

			if (pool_config->white_function_list == NULL)
			{
				fclose(fd);
				return(-1);
			}
			for (i=0;i<pool_config->num_white_function_list;i++)
			{
				add_regex_pattern("white_function_list", pool_config->white_function_list[i]);
			}
		}

		else if (!strcmp(key, "black_function_list") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->black_function_list =
				extract_string_tokens(str, ",", &pool_config->num_black_function_list);

			if (pool_config->black_function_list == NULL)
			{
				fclose(fd);
				return(-1);
			}
			for (i=0;i<pool_config->num_black_function_list;i++)
			{
				add_regex_pattern("black_function_list", pool_config->black_function_list[i]);
			}
		}

		else if (!strcmp(key, "print_timestamp") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_config->print_timestamp = v;
		}

		else if (!strcmp(key, "master_slave_mode") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_config->master_slave_mode = v;

			if (pool_config->master_slave_mode && pool_config->replication_mode)
			{
				pool_error("pool_config: replication_mode and master_slave_mode cannot be enabled at the same time");
				fclose(fd);
				return(-1);
			}
		}

		else if (!strcmp(key, "master_slave_sub_mode") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}

			if (strcmp(str, MODE_SLONY) && strcmp(str, MODE_STREAMREP))
			{
				pool_error("pool_config: %s must be either \"slony\" or \"stream\"", key);
				fclose(fd);
				return(-1);
			}
			pool_config->master_slave_sub_mode = str;
		}

		else if (!strcmp(key, "delay_threshold") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			long long int v = atol(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be greater or equal to 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->delay_threshold = v;
		}

		else if (!strcmp(key, "log_standby_delay") && CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}

			if (strcmp(str, "always") && strcmp(str, "if_over_threshold") && strcmp(str, "none"))
			{
				pool_error("pool_config: invalid log_standby_delay %s", key);
				fclose(fd);
				return(-1);
			}
			pool_config->log_standby_delay = str;
		}

		else if (!strcmp(key, "connection_cache") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_config->connection_cache = v;
		}

		else if (!strcmp(key, "health_check_timeout") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be equal or higher than 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->health_check_timeout = v;
		}

		else if (!strcmp(key, "health_check_period") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be equal or higher than 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->health_check_period = v;
		}

		else if (!strcmp(key, "health_check_user") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->health_check_user = str;
		}

		else if (!strcmp(key, "failover_command") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->failover_command = str;
		}

		else if (!strcmp(key, "follow_master_command") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->follow_master_command = str;
		}

		else if (!strcmp(key, "failback_command") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->failback_command = str;
		}

		else if (!strcmp(key, "fail_over_on_backend_error") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_config->fail_over_on_backend_error = v;
		}

		else if (!strcmp(key, "recovery_user") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->recovery_user = str;
		}

		else if (!strcmp(key, "recovery_password") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->recovery_password = str;
		}

		else if (!strcmp(key, "recovery_1st_stage_command") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->recovery_1st_stage_command = str;
		}

		else if (!strcmp(key, "recovery_2nd_stage_command") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->recovery_2nd_stage_command = str;
		}

		else if (!strcmp(key, "recovery_timeout") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be equal or higher than 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->recovery_timeout = v;
		}

		else if (!strcmp(key, "client_idle_limit_in_recovery") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < -1)
			{
				pool_error("pool_config: %s must be greater or equal to -1 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->client_idle_limit_in_recovery = v;
		}

		else if (!strcmp(key, "insert_lock") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_config->insert_lock = v;
		}

		else if (!strcmp(key, "ignore_leading_white_space") &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_config->ignore_leading_white_space = v;
		}

		else if (!strcmp(key, "parallel_mode") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_config->parallel_mode = v;
		}

		else if (!strcmp(key, "enable_query_cache") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}
			pool_config->enable_query_cache = v;
		}

		else if (!strcmp(key, "pgpool2_hostname") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			if(strlen(str))
				pool_config->pgpool2_hostname = str;
		}

		else if (!strcmp(key, "system_db_hostname") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->system_db_hostname = str;
		}

		else if (!strcmp(key, "system_db_port") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be equal or higher than 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->system_db_port = v;
		}

		else if (!strcmp(key, "system_db_dbname") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->system_db_dbname = str;
		}

		else if (!strcmp(key, "system_db_schema") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->system_db_schema = str;
		}

		else if (!strcmp(key, "system_db_user") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->system_db_user = str;
		}

		else if (!strcmp(key, "system_db_password") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->system_db_password = str;
		}

		else if (!strncmp(key, "backend_hostname", 16) &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context) &&
				 mypid == getpid()) /* this parameter must be modified by parent pid */
		{
			int slot;
			char *str;

			slot = atoi(key + 16);
			if (slot < 0 || slot >= MAX_CONNECTION_SLOTS)
			{
				pool_error("pool_config: backend number %s for backend_hostname out of range", key);
				fclose(fd);
				return(-1);
			}

			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			if (context == INIT_CONFIG ||
				(context == RELOAD_CONFIG && BACKEND_INFO(slot).backend_status == CON_UNUSED))
				strncpy(BACKEND_INFO(slot).backend_hostname, str, MAX_DB_HOST_NAMELEN);
		}

		else if (!strncmp(key, "backend_port", 12) &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context) &&
				 mypid == getpid()) /* this parameter must be modified by parent pid */
		{
			int slot;

			slot = atoi(key + 12);
			if (slot < 0 || slot >= MAX_CONNECTION_SLOTS)
			{
				pool_error("pool_config: host number %s for port number out of range", key);
				fclose(fd);
				return(-1);
			}
			pool_debug("pool_config: port slot number %d ", slot);
			if (context == INIT_CONFIG)
			{
				BACKEND_INFO(slot).backend_port = atoi(yytext);
				BACKEND_INFO(slot).backend_status = CON_CONNECT_WAIT;
			}
			else if (context == RELOAD_CONFIG && BACKEND_INFO(slot).backend_status == CON_UNUSED)
			{
				BACKEND_INFO(slot).backend_port = atoi(yytext);
				BACKEND_INFO(slot).backend_status = CON_DOWN;
			}
		}

		else if (!strncmp(key, "backend_weight", 14) &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context) &&
				 mypid == getpid()) /* this parameter must be modified by parent pid */
		{
			int slot;
			double v;
			BACKEND_STATUS status;

			slot = atoi(key + 14);
			if (slot < 0 || slot >= MAX_CONNECTION_SLOTS)
			{
				pool_error("pool_config: weight number %s for port number out of range", key);
				fclose(fd);
				return(-1);
			}

			v = atof(yytext);

			if (v < 0.0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				fclose(fd);
				return(-1);
			}

			pool_debug("pool_config: weight slot number %d weight: %f", slot, v);
			status = BACKEND_INFO(slot).backend_status;

			if (context == INIT_CONFIG || context == RELOAD_CONFIG)
			{
				double old_v = pool_config->backend_desc->backend_info[slot].unnormalized_weight;
				pool_config->backend_desc->backend_info[slot].unnormalized_weight = v;

				/*
				 * Log weight change event only when context is
				 * reloading of pgpool.conf and weight is actually
				 * changed
				 */
				if (context == RELOAD_CONFIG && old_v != v)
				{
					pool_log("Backend weight for backend%d changed from %f to %f. This will take effect from next client session.", slot, old_v, v);
				}
			}
		}
		else if (!strncmp(key, "backend_data_directory", 22) &&
				 CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context) &&
				 mypid == getpid()) /* this parameter must be modified by parent pid */
		{
			int slot;
			char *str;
			BACKEND_STATUS status;

			slot = atoi(key + 22);
			if (slot < 0 || slot >= MAX_CONNECTION_SLOTS)
			{
				pool_error("pool_config: backend number %s for backend_data_directory out of range", key);
				fclose(fd);
				return(-1);
			}

			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			status = BACKEND_INFO(slot).backend_status;
			if (context == INIT_CONFIG ||
				(context == RELOAD_CONFIG && (status == CON_UNUSED || status == CON_DOWN)))
				strncpy(BACKEND_INFO(slot).backend_data_directory, str, MAX_PATH_LENGTH);
		}
       	else if (!strcmp(key, "log_statement") && CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				return(-1);
			}
			pool_config->log_statement = v;
		}
       	else if (!strcmp(key, "log_per_node_statement") && CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				return(-1);
			}
			pool_config->log_per_node_statement = v;
		}
       	else if (!strcmp(key, "log_statement") && CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				return(-1);
			}
			pool_config->log_statement = v;
		}

		else if (!strcmp(key, "lobj_lock_table") && CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->lobj_lock_table = str;
		}

        else if (!strcmp(key, "ssl") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			int v = eval_logical(yytext);

			if (v < 0)
			{
				pool_error("pool_config: invalid value %s for %s", yytext, key);
				return(-1);
			}
			pool_config->ssl = v;
		}
		else if (!strcmp(key, "ssl_cert") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->ssl_cert = str;
		}
		else if (!strcmp(key, "ssl_key") && CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->ssl_key = str;
		}
		else if (!strcmp(key, "ssl_ca_cert") && 
		         CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->ssl_ca_cert = str;
		}
		else if (!strcmp(key, "ssl_ca_cert_dir") && 
		         CHECK_CONTEXT(INIT_CONFIG, context))
		{
			char *str;

			if (token != POOL_STRING && token != POOL_UNQUOTED_STRING && token != POOL_KEY)
			{
				PARSE_ERROR();
				fclose(fd);
				return(-1);
			}
			str = extract_string(yytext, token);
			if (str == NULL)
			{
				fclose(fd);
				return(-1);
			}
			pool_config->ssl_ca_cert_dir = str;
		}

		else if (!strcmp(key, "debug_level") && CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be equal or higher than 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->debug_level = v;
		}

		else if (!strcmp(key, "relcache_expire") && CHECK_CONTEXT(INIT_CONFIG|RELOAD_CONFIG, context))
		{
			int v = atoi(yytext);

			if (token != POOL_INTEGER || v < 0)
			{
				pool_error("pool_config: %s must be equal or higher than 0 numeric value", key);
				fclose(fd);
				return(-1);
			}
			pool_config->relcache_expire = v;
		}

	}

	fclose(fd);

    if (log_destination_changed)
    {
        // log_destination has changed, we need to open syslog or close it
		if (!strcmp(pool_config->log_destination, "stderr"))
        {
	        closelog();
    	   	pool_config->logsyslog = 0;
        }
        else
        {
    	   	openlog(pool_config->syslog_ident, LOG_PID|LOG_NDELAY|LOG_NOWAIT, pool_config->syslog_facility);
    	   	pool_config->logsyslog = 1;
        }
    }

	pool_config->backend_desc->num_backends = 0;
	total_weight = 0.0;

	for (i=0;i<MAX_CONNECTION_SLOTS;i++)
	{
		/* port number == 0 indicates that this server is out of use */
		if (BACKEND_INFO(i).backend_port == 0)
		{
			clear_host_entry(i);
		}

		else
		{
			total_weight += BACKEND_INFO(i).unnormalized_weight;
			pool_config->backend_desc->num_backends = i+1;
			
			/* intialize backend_hostname with a default socket path if empty */
			if (*(BACKEND_INFO(i).backend_hostname) == '\0') 
			{
				if (pool_config->backend_socket_dir == NULL)
				{
					pool_debug("pool_config: empty backend_hostname%d, use PostgreSQL's default unix socket path (%s)", i, DEFAULT_SOCKET_DIR);
					strncpy(BACKEND_INFO(i).backend_hostname, DEFAULT_SOCKET_DIR, MAX_DB_HOST_NAMELEN);
				}
				else /* DEPRECATED. backward compatibility with older version. Use backend_socket_dir*/ 
				{
					pool_debug("pool_config: empty backend_hostname%d, use backend_socket_dir as unix socket path (%s)", i, pool_config->backend_socket_dir);
					strncpy(BACKEND_INFO(i).backend_hostname, pool_config->backend_socket_dir, MAX_DB_HOST_NAMELEN);
				}
			}
		}	
	}

	pool_debug("num_backends: %d total_weight: %f",
			   pool_config->backend_desc->num_backends, total_weight);
	/*
	 * Normalize load balacing weights. What we are doing here is,
	 * assign 0 to RAND_MAX to each backend's weight according to the
	 * value weightN.  For example, if two backends are assigned 1.0,
	 * then each backend will get RAND_MAX/2 normalized weight.
	 */
	for (i=0;i<MAX_CONNECTION_SLOTS;i++)
	{
#ifdef DEBUG
		print_host_entry(i);
#endif

		if (pool_config->backend_desc->backend_info[i].backend_port != 0)
		{
			pool_config->backend_desc->backend_info[i].backend_weight =
				(RAND_MAX) * pool_config->backend_desc->backend_info[i].unnormalized_weight / total_weight;
			pool_debug("backend %d weight: %f", i, pool_config->backend_desc->backend_info[i].backend_weight);
		}
	}

	if (pool_config->parallel_mode || pool_config->enable_query_cache)
	{
#ifndef POOL_PRIVATE
		int dist_num;
#endif
		SystemDBInfo *info;
		
		system_db_info = malloc(sizeof(POOL_SYSTEMDB_CONNECTION_POOL));
		if (system_db_info == NULL)
		{
			pool_error("failed to allocate sytem_db_info");
			return -1;
		}
		memset(system_db_info, 0, sizeof(*system_db_info));

#ifndef POOL_PRIVATE
		system_db_info->system_db_status = pool_shared_memory_create(sizeof(BACKEND_STATUS));
#else
		system_db_info->system_db_status = malloc(sizeof(BACKEND_STATUS));
#endif
		if (system_db_info->system_db_status == NULL)
		{
			pool_error("failed to allocate system_db_info->system_db_status");
			return -1;
		}

		*system_db_info->system_db_status = CON_CONNECT_WAIT;	/* which is the same as SYSDB_STATUS = CON_CONNECT_WAIT */

		info = malloc(sizeof(SystemDBInfo));
		if (info == NULL)
		{
			pool_error("failed to allocate info");
			return -1;
		}

		system_db_info->info = info;
		info->hostname = pool_config->system_db_hostname;
		info->port = pool_config->system_db_port;
		info->user = pool_config->system_db_user;
		info->password = pool_config->system_db_password;
		info->database_name = pool_config->system_db_dbname;
		info->schema_name = pool_config->system_db_schema;
		info->dist_def_num = 0;
		info->dist_def_slot = NULL;

#ifndef POOL_PRIVATE
		if (pool_config->parallel_mode)
		{

			dist_num = pool_memset_system_db_info(info);
			if(dist_num < 0)
			{
				pool_error("failed to get systemdb info");
				return(-1);
			}
			if (!pool_config->replication_mode && !pool_config->load_balance_mode)
			{
				pool_error("pool_config: parallel_mode requires replication_mode or load_balance_mode turned on");
				return(-1);
			}
		}
		if (pool_config->enable_query_cache)
		{
			info->query_cache_table_info.register_prepared_statement = NULL;
			if (! pool_query_cache_table_exists())
			{
				pool_error("failed to locate query_cache table. perhaps it's not defined?");
				return -1;
			}
		}
		SYSDB_STATUS = CON_UP;
#endif
	}

	if (strcmp(pool_config->recovery_1st_stage_command, "") || 
		strcmp(pool_config->recovery_2nd_stage_command, ""))
	{
		for (i=0;i<MAX_CONNECTION_SLOTS;i++)
		{
			if (pool_config->backend_desc->backend_info[i].backend_port != 0 &&
				!strcmp(pool_config->backend_desc->backend_info[i].backend_data_directory, ""))
			{
				pool_error("pool_config: recovery_1st_stage_command and recovery_2nd_stage_command requires backend_data_directory to be set");
				return -1;
			}
		}
	}

	return 0;
}

static char *extract_string(char *value, POOL_TOKEN token)
{
	char *ret;

	ret = strdup(value);
	if (!ret)
	{
		pool_error("extract_string: out of memory");
		return NULL;
	}

	if (token == POOL_STRING)
	{
		ret[strlen(ret)-1] = '\0';
		return (ret+1);
	}
	return ret;
}

/*
 * Try to interpret value as boolean value.  Valid values are: true,
 * false, yes, no, on, off, 1, 0; as well as unique prefixes thereof.
 * If the string parses okay, return true, else false.
 * If okay and result is not NULL, return the value in *result.
 * This function copied from PostgreSQL source code.
 */
static bool parse_bool_with_len(const char *value, size_t len, bool *result)
{
	switch (*value)
	{
		case 't':
		case 'T':
			if (strncasecmp(value, "true", len) == 0)
			{
				if (result)
					*result = true;
				return true;
			}
			break;
		case 'f':
		case 'F':
			if (strncasecmp(value, "false", len) == 0)
			{
				if (result)
					*result = false;
				return true;
			}
			break;
		case 'y':
		case 'Y':
			if (strncasecmp(value, "yes", len) == 0)
			{
				if (result)
					*result = true;
				return true;
			}
			break;
		case 'n':
		case 'N':
			if (strncasecmp(value, "no", len) == 0)
			{
				if (result)
					*result = false;
				return true;
			}
			break;
		case 'o':
		case 'O':
			/* 'o' is not unique enough */
			if (strncasecmp(value, "on", (len > 2 ? len : 2)) == 0)
			{
				if (result)
					*result = true;
				return true;
			}
			else if (strncasecmp(value, "off", (len > 2 ? len : 2)) == 0)
			{
				if (result)
					*result = false;
				return true;
			}
			break;
		case '1':
			if (len == 1)
			{
				if (result)
					*result = true;
				return true;
			}
			break;
		case '0':
			if (len == 1)
			{
				if (result)
					*result = false;
				return true;
			}
			break;
		default:
			break;
	}

	if (result)
		*result = false;		/* suppress compiler warning */
	return false;
}

int eval_logical(char *str)
{
	bool result;

	if (!parse_bool_with_len(str, strlen(str), &result))
		return -1;

	return (result ? 1 : 0);
}

/*
 * Extract tokens separated by delimi from str. Return value is an
 * array of pointers to malloced strings. number of tokens is set to
 * n; note that str will be destroyed by strtok().
 */
#define MAXTOKENS 1024
static char **extract_string_tokens(char *str, char *delimi, int *n)
{
	char *token;
	static char **tokens;

	*n = 0;

	tokens = malloc(MAXTOKENS*sizeof(char *));
	if (tokens == NULL)
	{
		pool_error("extract_string_tokens: out of memory");
		return NULL;
	}

	for (token = strtok(str, delimi); token != NULL && *n < MAXTOKENS; token = strtok(NULL, delimi))
	{
		tokens[*n] = strdup(token);
		if (tokens[*n] == NULL)
		{
			pool_error("extract_string_tokens: out of memory");
			return NULL;
		}
		pool_debug("extract_string_tokens: token: %s", tokens[*n]);
		(*n)++;
	}
	return tokens;
}

static void clear_host_entry(int slot)
{
	*pool_config->backend_desc->backend_info[slot].backend_hostname = '\0';
	pool_config->backend_desc->backend_info[slot].backend_port = 0;
	pool_config->backend_desc->backend_info[slot].backend_status = CON_UNUSED;
	pool_config->backend_desc->backend_info[slot].backend_weight = 0.0;
}

#ifdef DEBUG
static void print_host_entry(int slot)
{
	pool_debug("slot: %d host: %s port: %d status: %d weight: %f",
			   slot,
			   pool_config->server_hostnames[slot],
			   pool_config->server_ports[slot],
			   pool_config->server_status[slot],
			   pool_config->server_weights[slot]);
}
#endif

/* Use to set the syslog facility level if logsyslog is activated */
int set_syslog_facility(char *value)
{
   int facility = LOG_LOCAL0;

   if (value == NULL)
     return facility;

   if (strncmp(value, "LOCAL", 5) == 0 && strlen(value) == 6) {
	  switch (value[5]) {
	  case '0':
	       facility = LOG_LOCAL0;
	       break;
	  case '1':
	       facility = LOG_LOCAL1;
	       break;
	  case '2':
	       facility = LOG_LOCAL2;
	       break;
	  case '3':
	       facility = LOG_LOCAL3;
	       break;
	  case '4':
	       facility = LOG_LOCAL4;
	       break;
	  case '5':
	       facility = LOG_LOCAL5;
	       break;
	  case '6':
	       facility = LOG_LOCAL6;
	       break;
	  case '7':
	       facility = LOG_LOCAL7;
	       break;
	  }
     }
     return facility;
}

