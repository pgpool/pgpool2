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
 * pgpool-walrecvrunning checks the walreceiver state to it's running.
 * pgpool needs to know if the backend is running as a standby by
 * using pg_is_in_recover(). Problem is, it returns true even if
 * walreceiver is going to shutdown. This function checks the internal
 * variable of walreciver to know if it is going to shutdown.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "postgres.h"
#include "utils/builtins.h"
#include "utils/elog.h"
#include "fmgr.h"
#include "funcapi.h"
#include "replication/walreceiver.h"
#include <stdlib.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(pgpool_walrecrunning);
extern Datum pgpool_walrecrunning(PG_FUNCTION_ARGS);

Datum
pgpool_walrecrunning(PG_FUNCTION_ARGS)
{
	/* use volatile pointer to prevent code rearrangement */
	volatile WalRcvData *walrcv = WalRcv;

	if (walrcv->walRcvState == WALRCV_RUNNING)
		PG_RETURN_BOOL(true);

	PG_RETURN_BOOL(false);
}
