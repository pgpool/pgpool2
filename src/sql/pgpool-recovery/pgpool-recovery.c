/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool-recovery: exec online recovery script from SELECT statement.
 *
 * Copyright (c) 2003-2013	PgPool Global Development Group
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
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "postgres.h"
#include "fmgr.h"
#include "miscadmin.h"
#include "executor/spi.h"
#include "funcapi.h"
#include "catalog/namespace.h"
#include "utils/syscache.h"
#include "utils/builtins.h"		/* PostgreSQL 8.4 needs this for textout */
#include "utils/guc.h"
#if defined(PG_VERSION_NUM) && (PG_VERSION_NUM >= 90300)
#include "access/htup_details.h"		/* PostgreSQL 9.3 or later needs this */
#endif

#define REMOTE_START_FILE "pgpool_remote_start"

#include <stdlib.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(pgpool_recovery);
PG_FUNCTION_INFO_V1(pgpool_remote_start);
PG_FUNCTION_INFO_V1(pgpool_pgctl);
PG_FUNCTION_INFO_V1(pgpool_switch_xlog);

extern Datum pgpool_recovery(PG_FUNCTION_ARGS);
extern Datum pgpool_remote_start(PG_FUNCTION_ARGS);
extern Datum pgpool_pgctl(PG_FUNCTION_ARGS);
extern Datum pgpool_switch_xlog(PG_FUNCTION_ARGS);

static char recovery_script[1024];
static char command_text[1024];

static Oid get_function_oid(const char *funcname, const char *argtype, const char *nspname);
char       *Log_line_prefix = NULL;

Datum
pgpool_recovery(PG_FUNCTION_ARGS)
{
	int r;
	char *script = DatumGetCString(DirectFunctionCall1(textout,
													   PointerGetDatum(PG_GETARG_TEXT_P(0))));
													   
	char *remote_host = DatumGetCString(DirectFunctionCall1(textout,
															PointerGetDatum(PG_GETARG_TEXT_P(1))));
	char *remote_data_directory = DatumGetCString(DirectFunctionCall1(textout,
																	  PointerGetDatum(PG_GETARG_TEXT_P(2))));

	if (!superuser())
#ifdef ERRCODE_INSUFFICIENT_PRIVILEGE
		ereport(ERROR,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				 (errmsg("must be superuser to use pgpool_recovery function"))));
#else
		elog(ERROR, "must be superuser to use pgpool_recovery function");
#endif

	if (PG_NARGS() >= 4)		/* post pgpool-II 3.4 */
	{
		char *remote_port = DatumGetCString(DirectFunctionCall1(textout,
														  PointerGetDatum(PG_GETARG_TEXT_P(3))));
		snprintf(recovery_script, sizeof(recovery_script), "%s/%s %s %s %s %s",
				 DataDir, script, DataDir, remote_host,
				 remote_data_directory, remote_port);
	}
	else
	{
		snprintf(recovery_script, sizeof(recovery_script), "%s/%s %s %s %s",
				 DataDir, script, DataDir, remote_host,
				 remote_data_directory);
	}

	elog(DEBUG1, "recovery_script: %s", recovery_script);
	r = system(recovery_script);

	if (r != 0)
	{
		elog(ERROR, "pgpool_recovery failed");
	}

	PG_RETURN_BOOL(true);
}

Datum
pgpool_remote_start(PG_FUNCTION_ARGS)
{
	int r;
	char *remote_host = DatumGetCString(DirectFunctionCall1(textout,
															PointerGetDatum(PG_GETARG_TEXT_P(0))));
	char *remote_data_directory = DatumGetCString(DirectFunctionCall1(textout,
																	  PointerGetDatum(PG_GETARG_TEXT_P(1))));
	if (!superuser())
#ifdef ERRCODE_INSUFFICIENT_PRIVILEGE
		ereport(ERROR,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				 (errmsg("must be superuser to use pgpool_remote_start function"))));
#else
		elog(ERROR, "must be superuser to use pgpool_remote_start function");
#endif

	snprintf(recovery_script, sizeof(recovery_script),
			 "%s/%s %s %s", DataDir, REMOTE_START_FILE,
			 remote_host, remote_data_directory);
	elog(DEBUG1, "recovery_script: %s", recovery_script);
	r = system(recovery_script);

	if (r != 0)
	{
		elog(ERROR, "pgpool_remote_start failed");
	}

	PG_RETURN_BOOL(true);
}

Datum
pgpool_pgctl(PG_FUNCTION_ARGS)
{
	int r;
	char *action = DatumGetCString(DirectFunctionCall1(textout,
	                                                   PointerGetDatum(PG_GETARG_TEXT_P(0))));
	char *stop_mode = DatumGetCString(DirectFunctionCall1(textout,
	                                                   PointerGetDatum(PG_GETARG_TEXT_P(1))));
	char *pg_ctl;
	char *data_directory;

	if (!superuser())
#ifdef ERRCODE_INSUFFICIENT_PRIVILEGE
		ereport(ERROR,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				 (errmsg("must be superuser to use pgpool_pgctl function"))));
#else
		elog(ERROR, "must be superuser to use pgpool_pgctl function");
#endif

	pg_ctl = GetConfigOptionByName("pgpool.pg_ctl", NULL);
	data_directory = GetConfigOptionByName("data_directory", NULL);

	if (strcmp(stop_mode, "") != 0)
	{
		snprintf(command_text, sizeof(command_text),
				 "%s %s -D %s -m %s 2>/dev/null 1>/dev/null < /dev/null &",
				 pg_ctl, action, data_directory, stop_mode);

	} else {
		snprintf(command_text, sizeof(command_text),
				 "%s %s -D %s 2>/dev/null 1>/dev/null < /dev/null &",
				 pg_ctl, action, data_directory);
	}

	elog(DEBUG1, "command_text: %s", command_text);
	r = system(command_text);

	if (strcmp(action, "reload") == 0 && r != 0)
	{
		elog(ERROR, "pgpool_pgctl failed");
	}

	PG_RETURN_BOOL(true);
}

/*
 * pgpool_switch_log is the same as pg_switch_xlog except that
 * it wait till archiving is completed.
 * We call xlog functions with the oid to avoid a compile error
 * at old PostgreSQL.
 */
Datum
pgpool_switch_xlog(PG_FUNCTION_ARGS)
{
	char *archive_dir;
	char *filename;
	char path[MAXPGPATH];
	struct stat fst;
	Datum location;
	text *filename_t;
	text *result;
	Oid switch_xlog_oid;
	Oid xlogfile_name_oid;

	archive_dir = DatumGetCString(DirectFunctionCall1(textout,
													  PointerGetDatum(PG_GETARG_TEXT_P(0))));

	if (stat(archive_dir, &fst) < 0)
#ifdef ERRCODE_INSUFFICIENT_PRIVILEGE
		ereport(ERROR,
                (errcode_for_file_access(),
                 errmsg("could not stat file \"%s\": %m", archive_dir)));
#else
		elog(ERROR, "could not stat file \"%s\"", archive_dir);	
#endif

	switch_xlog_oid = get_function_oid("pg_switch_xlog", NULL, "pg_catalog");
	xlogfile_name_oid = get_function_oid("pg_xlogfile_name", "text", "pg_catalog");

	if (!switch_xlog_oid || !xlogfile_name_oid)
		elog(ERROR, "cannot find xlog functions");

	location = OidFunctionCall1(switch_xlog_oid, PointerGetDatum(NULL));
	filename_t = DatumGetTextP(OidFunctionCall1(xlogfile_name_oid, location));
	filename = DatumGetCString(DirectFunctionCall1(textout,
												   PointerGetDatum(filename_t)));

	snprintf(path, MAXPGPATH, "%s/%s", archive_dir, filename);
	elog(LOG, "pgpool_switch_xlog: waiting for \"%s\"", path);

	while(stat(path, &fst) != 0 || fst.st_size == 0 ||
		  fst.st_size % (1024 * 1024) != 0)
	{
		CHECK_FOR_INTERRUPTS();
		sleep(1);
	}

    result = DatumGetTextP(DirectFunctionCall1(textin,
                                               CStringGetDatum(path)));

	PG_RETURN_TEXT_P(result);
}

static Oid
get_function_oid(const char *funcname, const char *argtype, const char *nspname)
{
#ifndef PROCNAMENSP
	Oid typid;
	Oid nspid;
	Oid funcid;
	Oid oids[1];
	oidvector *oid_v;
	HeapTuple tup;

	if (argtype)
	{
		typid = TypenameGetTypid(argtype);
		elog(DEBUG1, "get_function_oid: %s typid: %d", argtype, typid);
		oids[0] = typid;
		oid_v = buildoidvector(oids, 1);
	}
	else
	{
		oid_v = buildoidvector(NULL, 0);
	}

#if !defined(PG_VERSION_NUM) || (PG_VERSION_NUM < 90300)
	nspid = LookupExplicitNamespace(nspname);
#else
	/* LookupExplicitNamespace() of PostgreSQL 9.3 or later, has third
	 * argument "missing_ok" which suppresses ERROR exception, but
	 * returns invlaid_oid. See include/catalog/namespace.h */
	nspid = LookupExplicitNamespace(nspname, false);
#endif
	elog(DEBUG1, "get_function_oid: oid of \"%s\": %d", nspname, nspid);

	tup = SearchSysCache(PROCNAMEARGSNSP,
						 PointerGetDatum(funcname),
						 PointerGetDatum(oid_v),
						 ObjectIdGetDatum(nspid),
						 0);

	if (HeapTupleIsValid(tup))
	{
		funcid = HeapTupleGetOid(tup);
		elog(DEBUG1, "get_function_oid: oid of \"%s\": %d", funcname, funcid);
		ReleaseSysCache(tup);
		return funcid;
	}
#endif
	return 0;
}

