/* -*-pgsql-c-*- */
/*
 *
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2016	PgPool Global Development Group
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
 * pool_config_variables.h.
 *
 */

#ifndef POOL_CONFIG_VARIABLES_H
#define POOL_CONFIG_VARIABLES_H


typedef enum
{
	CONNECTION_CONFIG,
	CONNECTION_POOL_CONFIG,
	LOGING_CONFIG,
	HEALTH_CHECK_CONFIG,
	FILE_LOCATION_CONFIG,
	LOAD_BALANCE_CONFIG,
	REPLICATION_CONFIG,
	STREAMING_REPLICATION_CONFIG,
	MASTER_SLAVE_CONFIG,
	WATCHDOG_CONFIG,
	SSL_CONFIG,
	FAILOVER_CONFIG,
	RECOVERY_CONFIG,
	WATCHDOG_LIFECHECK,
	GENERAL_CONFIG,
	CACHE_CONFIG
} config_group;

typedef enum
{
	CONFIG_VAR_TYPE_BOOL,
	CONFIG_VAR_TYPE_INT,
	CONFIG_VAR_TYPE_LONG,
	CONFIG_VAR_TYPE_DOUBLE,
	CONFIG_VAR_TYPE_STRING,
	CONFIG_VAR_TYPE_STRING_LIST,
	CONFIG_VAR_TYPE_ENUM,
	CONFIG_VAR_TYPE_INT_ARRAY,
	CONFIG_VAR_TYPE_DOUBLE_ARRAY,
	CONFIG_VAR_TYPE_STRING_ARRAY,
	CONFIG_VAR_TYPE_GROUP
}config_type;

/*
 * The possible values of an enum variable are specified by an array of
 * name-value pairs.  The "hidden" flag means the value is accepted but
 * won't be displayed when guc.c is asked for a list of acceptable values.
 */
struct config_enum_entry
{
	const char *name;
	int			val;
	bool		hidden;
};

typedef enum
{
	PGC_S_DEFAULT,				/* hard-wired default ("boot_val") */
	PGC_S_FILE,					/* pgpool.conf */
	PGC_S_ARGV,					/* command line */
	PGC_S_CLIENT,				/* from client connection request */
	PGC_S_SESSION				/* SET command */
} GucSource;


/* Config variable flags bit values */
#define VAR_PART_OF_GROUP			0x0001
#define VAR_HIDDEN_VALUE			0x0002	/* for password type variables */
#define VAR_HIDDEN_IN_SHOW_ALL		0x0004	/* for variables hidden in show all*/
#define VAR_NO_RESET_ALL			0x0008	/* for variables not to be reset with reset all*/

/*
 * Signatures for per-variable check/assign/show  functions
 */

typedef const char *(*VarShowHook) (void);
typedef const char *(*IndexedVarShowHook) (int index);
typedef bool (*IndexedVarEmptySlotCheck) (int index);

typedef bool (*ConfigBoolAssignFunc) (ConfigContext scontext, bool newval, int elevel);
typedef bool (*ConfigEnumAssignFunc) (ConfigContext scontext, int newval, int elevel);

typedef bool (*ConfigStringListAssignFunc) (ConfigContext scontext, char* newval, int index);

typedef bool (*ConfigInt64AssignFunc)  (ConfigContext scontext, int64 newval, int elevel);

typedef bool (*ConfigStringAssignFunc) (ConfigContext scontext, char* newval, int elevel);
typedef bool (*ConfigStringArrayAssignFunc) (ConfigContext scontext, char* newval, int index, int elevel);

typedef bool (*ConfigIntAssignFunc)	(ConfigContext scontext, int newval, int elevel);
typedef bool (*ConfigIntArrayAssignFunc) (ConfigContext scontext, int newval, int index, int elevel);

typedef bool (*ConfigDoubleAssignFunc) (ConfigContext scontext, double newval, int elevel);
typedef bool (*ConfigDoubleArrayAssignFunc) (ConfigContext scontext, double newval, int index, int elevel);

typedef bool (*ConfigStringProcessingFunc) (char* newval,int elevel);
typedef bool (*ConfigEnumProcessingFunc) (int newval,int elevel);


struct config_generic
{
	/* constant fields, must be set correctly in initial value: */
	const char		*name;			/* name of variable - MUST BE FIRST */
	ConfigContext	context;		/* context required to set the variable */
	config_group	group;			/* to help organize variables by function */
	const char		*description;	/* short desc. of this variable's purpose */
	config_type		vartype;		/* type of variable (set only at startup) */
	bool			dynamic_array_var;	/* true if the variable name contains index postfix */
	int				flags;			/* flags */
	int				status;			/* status bits, see below */
	GucSource		source;			/* source of the current actual value */
	ConfigContext	scontext;		/* context that set the current value */
	int				sourceline;		/* line in source file */
};

/* GUC records for specific variable types */

struct config_bool
{
	struct config_generic gen;
	/* constant fields, must be set correctly in initial value: */
	bool	   *variable;
	bool		boot_val;
	ConfigBoolAssignFunc assign_func;
	ConfigBoolAssignFunc check_func;
	VarShowHook show_hook;
	bool		reset_val;
};

struct config_int
{
	struct config_generic gen;
	/* constant fields, must be set correctly in initial value: */
	int		   *variable;
	int			boot_val;
	int			min;
	int			max;
	ConfigIntAssignFunc assign_func;
	ConfigIntAssignFunc check_func;
	VarShowHook show_hook;
	int		reset_val;
};

struct config_int_array
{
	struct config_generic gen;
	/* constant fields, must be set correctly in initial value: */
	int		   **variable;
	int			boot_val;
	int			min;
	int			max;
	int			max_elements;
	ConfigIntArrayAssignFunc assign_func;
	ConfigIntArrayAssignFunc check_func;
	IndexedVarShowHook show_hook;
	IndexedVarEmptySlotCheck	empty_slot_check_func;
	int			*reset_vals; /* Array of reset values */
};


struct config_double
{
	struct config_generic gen;
	/* constant fields, must be set correctly in initial value: */
	double		*variable;
	double		boot_val;
	double		min;
	double		max;
	ConfigDoubleAssignFunc assign_func;
	ConfigDoubleAssignFunc check_func;
	VarShowHook show_hook;
	double		reset_val;
};

struct config_double_array
{
	struct config_generic gen;
	/* constant fields, must be set correctly in initial value: */
	double		**variable;
	double		boot_val;
	double		min;
	double		max;
	int			max_elements;
	ConfigDoubleArrayAssignFunc assign_func;
	ConfigDoubleArrayAssignFunc check_func;
	IndexedVarShowHook show_hook;
	IndexedVarEmptySlotCheck	empty_slot_check_func;
	double		*reset_vals;
};

struct config_long
{
	struct config_generic gen;
	/* constant fields, must be set correctly in initial value: */
	int64		*variable;
	int64		boot_val;
	int64		min;
	int64		max;
	ConfigInt64AssignFunc assign_func;
	ConfigInt64AssignFunc check_func;
	VarShowHook show_hook;
	int64		reset_val;
};

struct config_string
{
	struct config_generic gen;
	/* constant fields, must be set correctly in initial value: */
	char	  **variable;
	const char *boot_val;
	ConfigStringAssignFunc assign_func;
	ConfigStringAssignFunc check_func;
	ConfigStringProcessingFunc process_func;
	VarShowHook show_hook;
	char*		reset_val;
};

struct config_string_array
{
	struct config_generic gen;
	/* constant fields, must be set correctly in initial value: */
	char		***variable;
	const char	*boot_val;
	int			max_elements;
	ConfigStringArrayAssignFunc assign_func;
	ConfigStringArrayAssignFunc check_func;
	IndexedVarShowHook show_hook;
	IndexedVarEmptySlotCheck	empty_slot_check_func;
	char		**reset_vals;
};

struct config_string_list
{
	struct config_generic gen;
	/* constant fields, must be set correctly in initial value: */
	char		***variable;
	int			*list_elements_count;
	const char	*boot_val;
	const char	*seperator;
	bool		compute_regex;
	ConfigStringListAssignFunc assign_func;
	ConfigStringListAssignFunc check_func;
	VarShowHook show_hook;
	char		*reset_val;
	char		*current_val;
};

struct config_enum
{
	struct config_generic gen;
	/* constant fields, must be set correctly in initial value: */
	int		   *variable;
	int			boot_val;
	const struct config_enum_entry *options;
	ConfigEnumAssignFunc assign_func;
	ConfigEnumAssignFunc check_func;
	ConfigEnumProcessingFunc process_func;
	VarShowHook show_hook;
	int		reset_val;
};

struct config_grouped_array_var
{
	struct config_generic gen;
	int var_count;
	struct config_generic **var_list;
};

extern void InitializeConfigOptions(void);
extern bool set_one_config_option(const char *name, const char *value,
						   ConfigContext context, GucSource source, int elevel);

extern bool set_config_options(ConfigVariable *head_p,
							   ConfigContext context, GucSource source, int elevel);
extern bool assign_variable_to_int_array_config_var(const char* name, int** variable);


#ifndef POOL_PRIVATE
extern bool report_config_variable(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, const char* var_name);
extern bool report_all_variables(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
extern bool set_config_option_for_session(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend, const char *name, const char *value);
bool reset_all_variables(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
#endif

#endif /* POOL_CONFIG_VARIABLES_H */
