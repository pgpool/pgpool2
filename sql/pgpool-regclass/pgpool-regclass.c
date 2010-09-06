/* -*-pgsql-c-*- */
/*
 * $Header$
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
 * pgpool-regclass.c is similar to PostgreSQL builtin function
 * reglcass but do not throw exception.
 * If something goes wrong, it returns InvalidOid.
 *
 * CAUTION: This function uses PG_TRY() and related family macros which
 * are only available on 8.0 or later.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "postgres.h"
#include "utils/builtins.h"
#include "utils/elog.h"
#include "fmgr.h"
#include "funcapi.h"

#include <stdlib.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(pgpool_regclass);

extern Datum pgpool_regclass(PG_FUNCTION_ARGS);

Datum
pgpool_regclass(PG_FUNCTION_ARGS)
{
	char		*pro_name_or_oid = PG_GETARG_CSTRING(0);
	Oid	result;

	PG_TRY();
	{
		result = DirectFunctionCall1(regclassin, 
									 CStringGetDatum(pro_name_or_oid));
	}
	PG_CATCH();
	{
		result = InvalidOid;
	}
	PG_END_TRY();

	PG_RETURN_OID(result);
}
