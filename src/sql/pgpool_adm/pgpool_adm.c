/*-------------------------------------------------------------------------
 *
 * pgpool_adm.c
 *
 *
 * Copyright (c) 2002-2018, PostgreSQL Global Development Group
 *
 * Author: Jehan-Guillaume (ioguix) de Rorthais <jgdr@dalibo.com>
 *
 * IDENTIFICATION
 *	  contrib/pgpool_adm/pgpool_adm.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"
#include "miscadmin.h"
#include "utils/builtins.h"
#include "foreign/foreign.h"
#include "nodes/pg_list.h"
#include "utils/timestamp.h"

/*
 * PostgreSQL 9.3 or later requires htup_details.h to get the definition of
 * heap_form_tuple
 */
#if defined(PG_VERSION_NUM) && (PG_VERSION_NUM >= 90300)
#include "access/htup_details.h"
#endif

#include <unistd.h>
#include <time.h>

#include "catalog/pg_type.h"
#include "funcapi.h"
#include "libpcp_ext.h"
#include "pgpool_adm.h"


static PCPConnInfo * connect_to_server(char *host, int port, char *user, char *pass);
static PCPConnInfo * connect_to_server_from_foreign_server(char *name);


/**
 * Wrapper around pcp_connect
 * pcp_conninfo: pcpConninfo structure having pcp connection properties
 */
static PCPConnInfo *
connect_to_server(char *host, int port, char *user, char *pass)
{
	PCPConnInfo *pcpConnInfo;

	pcpConnInfo = pcp_connect(host, port, user, pass, NULL);
	if (PCPConnectionStatus(pcpConnInfo) != PCP_CONNECTION_OK)
		ereport(ERROR, (0,
						errmsg("connection to PCP server failed."),
						errdetail("%s\n", pcp_get_last_error(pcpConnInfo) ? pcp_get_last_error(pcpConnInfo) : "unknown reason")));

	return pcpConnInfo;
}

/**
 * Returns a pcpConninfo structure filled from a foreign server
 * name: the name of the foreign server
 */
static PCPConnInfo *
connect_to_server_from_foreign_server(char *name)
{
	Oid			userid = GetUserId();
	char	   *user = NULL;
	char	   *host = NULL;
	int			port = 9898;
	char	   *pass = NULL;

	/* raise an error if given foreign server doesn't exists */
	ForeignServer *foreign_server = GetForeignServerByName(name, false);
	UserMapping *user_mapping;
	ListCell   *cell;

	/*
	 * raise an error if the current user isn't mapped with the given foreign
	 * server
	 */
	user_mapping = GetUserMapping(userid, foreign_server->serverid);

	foreach(cell, foreign_server->options)
	{
		DefElem    *def = lfirst(cell);

		if (strcmp(def->defname, "host") == 0)
		{
			host = pstrdup(strVal(def->arg));
		}
		else if (strcmp(def->defname, "port") == 0)
		{
			port = atoi(strVal(def->arg));
		}
	}

	foreach(cell, user_mapping->options)
	{
		DefElem    *def = lfirst(cell);

		if (strcmp(def->defname, "user") == 0)
		{
			user = pstrdup(strVal(def->arg));
		}
		else if (strcmp(def->defname, "password") == 0)
		{
			pass = pstrdup(strVal(def->arg));
		}
	}

	return connect_to_server(host, port, user, pass);
}

/**
 * nodeID: the node id to get info from
 * host_or_srv: server name or ip address of the pgpool server
 * port: pcp port number
 * user: user to connect with
 * pass: password
 **/
Datum
_pcp_node_info(PG_FUNCTION_ARGS)
{
	int16		nodeID = PG_GETARG_INT16(0);
	char	   *host_or_srv = text_to_cstring(PG_GETARG_TEXT_PP(1));

	PCPConnInfo *pcpConnInfo;
	PCPResultInfo *pcpResInfo;

	BackendInfo *backend_info = NULL;
	Datum		values[7];		/* values to build the returned tuple from */
	bool		nulls[] = {false, false, false, false, false, false, false};
	TupleDesc	tupledesc;
	HeapTuple	tuple;
	struct tm	tm;
	char		datebuf[20];

	if (nodeID < 0 || nodeID >= MAX_NUM_BACKENDS)
		ereport(ERROR, (0, errmsg("NodeID is out of range.")));

	if (PG_NARGS() == 5)
	{
		char	   *user,
				   *pass;
		int			port;

		port = PG_GETARG_INT16(2);
		user = text_to_cstring(PG_GETARG_TEXT_PP(3));
		pass = text_to_cstring(PG_GETARG_TEXT_PP(4));
		pcpConnInfo = connect_to_server(host_or_srv, port, user, pass);
	}
	else if (PG_NARGS() == 2)
	{
		pcpConnInfo = connect_to_server_from_foreign_server(host_or_srv);
	}
	else
	{
		ereport(ERROR, (0, errmsg("Wrong number of argument.")));
	}

	pcpResInfo = pcp_node_info(pcpConnInfo, nodeID);
	if (pcpResInfo == NULL || PCPResultStatus(pcpResInfo) != PCP_RES_COMMAND_OK)
	{
		char	   *error = pcp_get_last_error(pcpConnInfo) ? pstrdup(pcp_get_last_error(pcpConnInfo)) : NULL;

		pcp_disconnect(pcpConnInfo);
		pcp_free_connection(pcpConnInfo);
		ereport(ERROR, (0,
						errmsg("failed to get node information"),
						errdetail("%s\n", error ? error : "unknown reason")));
	}

	/**
	 * Construct a tuple descriptor for the result rows.
	 **/
#if defined(PG_VERSION_NUM) && (PG_VERSION_NUM >= 120000)
	tupledesc = CreateTemplateTupleDesc(7);
#else
	tupledesc = CreateTemplateTupleDesc(7, false);
#endif
	TupleDescInitEntry(tupledesc, (AttrNumber) 1, "hostname", TEXTOID, -1, 0);
	TupleDescInitEntry(tupledesc, (AttrNumber) 2, "port", INT4OID, -1, 0);
	TupleDescInitEntry(tupledesc, (AttrNumber) 3, "status", TEXTOID, -1, 0);
	TupleDescInitEntry(tupledesc, (AttrNumber) 4, "weight", FLOAT4OID, -1, 0);
	TupleDescInitEntry(tupledesc, (AttrNumber) 5, "role", TEXTOID, -1, 0);
	TupleDescInitEntry(tupledesc, (AttrNumber) 6, "replication_delay", INT8OID, -1, 0);
	TupleDescInitEntry(tupledesc, (AttrNumber) 7, "last_status_change", TIMESTAMPOID, -1, 0);
	tupledesc = BlessTupleDesc(tupledesc);

	backend_info = (BackendInfo *) pcp_get_binary_data(pcpResInfo, 0);

	/* set values */
	values[0] = CStringGetTextDatum(backend_info->backend_hostname);
	nulls[0] = false;
	values[1] = Int16GetDatum(backend_info->backend_port);
	nulls[1] = false;
	switch (backend_info->backend_status)
	{
		case CON_UNUSED:
			values[2] = CStringGetTextDatum("Connection unused");
			break;
		case CON_CONNECT_WAIT:
			values[2] = CStringGetTextDatum("Waiting for connection to start");
			break;
		case CON_UP:
			values[2] = CStringGetTextDatum("Connection in use");
			break;
		case CON_DOWN:
			values[2] = CStringGetTextDatum("Disconnected");
			break;
	}
	nulls[2] = false;
	values[3] = Float8GetDatum(backend_info->backend_weight / RAND_MAX);
	nulls[3] = false;

	nulls[4] = false;
	values[4] = backend_info->role == ROLE_PRIMARY ? CStringGetTextDatum("Primary") : CStringGetTextDatum("Standby");

	nulls[5] = false;
	values[5] = Int64GetDatum(backend_info->standby_delay);

	nulls[6] = false;
	localtime_r(&backend_info->status_changed_time, &tm);
	strftime(datebuf, sizeof(datebuf), "%F %T", &tm);
	values[6] = DatumGetTimestamp(DirectFunctionCall3(timestamp_in,
													  CStringGetDatum(datebuf),
													  ObjectIdGetDatum(InvalidOid),
													  Int32GetDatum(-1)));

	pcp_disconnect(pcpConnInfo);
	pcp_free_connection(pcpConnInfo);

	/* build and return the tuple */
	tuple = heap_form_tuple(tupledesc, values, nulls);

	ReleaseTupleDesc(tupledesc);

	PG_RETURN_DATUM(HeapTupleGetDatum(tuple));
}

/**
 * host_or_srv: server name or ip address of the pgpool server
 * port: pcp port number
 * user: user to connect with
 * pass: password
 **/
Datum
_pcp_pool_status(PG_FUNCTION_ARGS)
{
	MemoryContext oldcontext;
	FuncCallContext *funcctx;
	int32		nrows;
	int32		call_cntr;
	int32		max_calls;
	AttInMetadata *attinmeta;
	PCPConnInfo *pcpConnInfo;
	PCPResultInfo *pcpResInfo;

	/* stuff done only on the first call of the function */
	if (SRF_IS_FIRSTCALL())
	{
		TupleDesc	tupdesc;
		char	   *host_or_srv = text_to_cstring(PG_GETARG_TEXT_PP(0));

		/* create a function context for cross-call persistence */
		funcctx = SRF_FIRSTCALL_INIT();

		/* switch to memory context appropriate for multiple function calls */
		oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		if (PG_NARGS() == 4)
		{
			char	   *user,
					   *pass;
			int			port;

			port = PG_GETARG_INT16(1);
			user = text_to_cstring(PG_GETARG_TEXT_PP(2));
			pass = text_to_cstring(PG_GETARG_TEXT_PP(3));
			pcpConnInfo = connect_to_server(host_or_srv, port, user, pass);

		}
		else if (PG_NARGS() == 1)
		{
			pcpConnInfo = connect_to_server_from_foreign_server(host_or_srv);
		}
		else
		{
			MemoryContextSwitchTo(oldcontext);
			ereport(ERROR, (0, errmsg("Wrong number of argument.")));
		}

		pcpResInfo = pcp_pool_status(pcpConnInfo);
		if (pcpResInfo == NULL || PCPResultStatus(pcpResInfo) != PCP_RES_COMMAND_OK)
		{
			char	   *error = pcp_get_last_error(pcpConnInfo) ? pstrdup(pcp_get_last_error(pcpConnInfo)) : NULL;

			pcp_disconnect(pcpConnInfo);
			pcp_free_connection(pcpConnInfo);

			MemoryContextSwitchTo(oldcontext);
			ereport(ERROR, (0,
							errmsg("failed to get pool status"),
							errdetail("%s\n", error ? error : "unknown reason")));
		}

		nrows = pcp_result_slot_count(pcpResInfo);
		pcp_disconnect(pcpConnInfo);
		/* Construct a tuple descriptor for the result rows */
#if defined(PG_VERSION_NUM) && (PG_VERSION_NUM >= 120000)
		tupdesc = CreateTemplateTupleDesc(3);
#else
		tupdesc = CreateTemplateTupleDesc(3, false);
#endif
		TupleDescInitEntry(tupdesc, (AttrNumber) 1, "item", TEXTOID, -1, 0);
		TupleDescInitEntry(tupdesc, (AttrNumber) 2, "value", TEXTOID, -1, 0);
		TupleDescInitEntry(tupdesc, (AttrNumber) 3, "description", TEXTOID, -1, 0);

		/*
		 * Generate attribute metadata needed later to produce tuples from raw
		 * C strings
		 */
		attinmeta = TupleDescGetAttInMetadata(tupdesc);
		funcctx->attinmeta = attinmeta;

		if (nrows > 0)
		{
			funcctx->max_calls = nrows;

			/* got results, keep track of them */
			funcctx->user_fctx = pcpConnInfo;
		}
		else
		{
			/* fast track when no results */
			MemoryContextSwitchTo(oldcontext);
			SRF_RETURN_DONE(funcctx);
		}

		MemoryContextSwitchTo(oldcontext);
	}

	/* stuff done on every call of the function */
	funcctx = SRF_PERCALL_SETUP();

	/* initialize per-call variables */
	call_cntr = funcctx->call_cntr;
	max_calls = funcctx->max_calls;

	pcpConnInfo = (PCPConnInfo *) funcctx->user_fctx;
	pcpResInfo = (PCPResultInfo *) pcpConnInfo->pcpResInfo;
	attinmeta = funcctx->attinmeta;

	if (call_cntr < max_calls)	/* executed while there is more left to send */
	{
		char	   *values[3];
		HeapTuple	tuple;
		Datum		result;
		POOL_REPORT_CONFIG *status = (POOL_REPORT_CONFIG *) pcp_get_binary_data(pcpResInfo, call_cntr);

		values[0] = pstrdup(status->name);
		values[1] = pstrdup(status->value);
		values[2] = pstrdup(status->desc);

		/* build the tuple */
		tuple = BuildTupleFromCStrings(attinmeta, values);

		/* make the tuple into a datum */
		result = HeapTupleGetDatum(tuple);

		SRF_RETURN_NEXT(funcctx, result);
	}
	else
	{
		/* do when there is no more left */
		pcp_free_connection(pcpConnInfo);
		SRF_RETURN_DONE(funcctx);
	}
}

/**
 * nodeID: the node id to get info from
 * host_or_srv: server name or ip address of the pgpool server
 * port: pcp port number
 * user: user to connect with
 * pass: password
 **/
Datum
_pcp_node_count(PG_FUNCTION_ARGS)
{
	char	   *host_or_srv = text_to_cstring(PG_GETARG_TEXT_PP(0));
	int16		node_count = 0;

	PCPConnInfo *pcpConnInfo;
	PCPResultInfo *pcpResInfo;

	if (PG_NARGS() == 4)
	{
		char	   *user,
				   *pass;
		int			port;

		port = PG_GETARG_INT16(1);
		user = text_to_cstring(PG_GETARG_TEXT_PP(2));
		pass = text_to_cstring(PG_GETARG_TEXT_PP(3));
		pcpConnInfo = connect_to_server(host_or_srv, port, user, pass);
	}
	else if (PG_NARGS() == 1)
	{
		pcpConnInfo = connect_to_server_from_foreign_server(host_or_srv);
	}
	else
	{
		ereport(ERROR, (0, errmsg("Wrong number of argument.")));
	}

	pcpResInfo = pcp_node_count(pcpConnInfo);

	if (pcpResInfo == NULL || PCPResultStatus(pcpResInfo) != PCP_RES_COMMAND_OK)
	{
		char	   *error = pcp_get_last_error(pcpConnInfo) ? pstrdup(pcp_get_last_error(pcpConnInfo)) : NULL;

		pcp_disconnect(pcpConnInfo);
		pcp_free_connection(pcpConnInfo);
		ereport(ERROR, (0,
						errmsg("failed to get node count"),
						errdetail("%s\n", error ? error : "unknown reason")));
	}

	node_count = pcp_get_int_data(pcpResInfo, 0);

	pcp_disconnect(pcpConnInfo);
	pcp_free_connection(pcpConnInfo);

	PG_RETURN_INT16(node_count);
}

/**
 * nodeID: the node id to get info from
 * host_or_srv: server name or ip address of the pgpool server
 * port: pcp port number
 * user: user to connect with
 * pass: password
 **/
Datum
_pcp_attach_node(PG_FUNCTION_ARGS)
{
	int16		nodeID = PG_GETARG_INT16(0);
	char	   *host_or_srv = text_to_cstring(PG_GETARG_TEXT_PP(1));

	PCPConnInfo *pcpConnInfo;
	PCPResultInfo *pcpResInfo;

	if (nodeID < 0 || nodeID >= MAX_NUM_BACKENDS)
		ereport(ERROR, (0, errmsg("NodeID is out of range.")));

	if (PG_NARGS() == 5)
	{
		char	   *user,
				   *pass;
		int			port;

		port = PG_GETARG_INT16(2);
		user = text_to_cstring(PG_GETARG_TEXT_PP(3));
		pass = text_to_cstring(PG_GETARG_TEXT_PP(4));
		pcpConnInfo = connect_to_server(host_or_srv, port, user, pass);
	}
	else if (PG_NARGS() == 2)
	{
		pcpConnInfo = connect_to_server_from_foreign_server(host_or_srv);
	}
	else
	{
		ereport(ERROR, (0, errmsg("Wrong number of argument.")));
	}

	pcpResInfo = pcp_attach_node(pcpConnInfo, nodeID);

	if (pcpResInfo == NULL || PCPResultStatus(pcpResInfo) != PCP_RES_COMMAND_OK)
	{
		char	   *error = pcp_get_last_error(pcpConnInfo) ? pstrdup(pcp_get_last_error(pcpConnInfo)) : NULL;

		pcp_disconnect(pcpConnInfo);
		pcp_free_connection(pcpConnInfo);
		ereport(ERROR, (0,
						errmsg("failed to attach node"),
						errdetail("%s\n", error ? error : "unknown reason")));
	}

	pcp_disconnect(pcpConnInfo);
	pcp_free_connection(pcpConnInfo);


	PG_RETURN_BOOL(true);
}

/**
 * nodeID: the node id to get info from
 * gracefully: detach node gracefully if true
 * host_or_srv: server name or ip address of the pgpool server
 * port: pcp port number
 * user: user to connect with
 * pass: password
 **/
Datum
_pcp_detach_node(PG_FUNCTION_ARGS)
{
	int16		nodeID = PG_GETARG_INT16(0);
	bool		gracefully = PG_GETARG_BOOL(1);
	char	   *host_or_srv = text_to_cstring(PG_GETARG_TEXT_PP(2));

	PCPConnInfo *pcpConnInfo;
	PCPResultInfo *pcpResInfo;

	if (nodeID < 0 || nodeID >= MAX_NUM_BACKENDS)
		ereport(ERROR, (0, errmsg("NodeID is out of range.")));

	if (PG_NARGS() == 6)
	{
		char	   *user,
				   *pass;
		int			port;

		port = PG_GETARG_INT16(3);
		user = text_to_cstring(PG_GETARG_TEXT_PP(4));
		pass = text_to_cstring(PG_GETARG_TEXT_PP(5));
		pcpConnInfo = connect_to_server(host_or_srv, port, user, pass);
	}
	else if (PG_NARGS() == 3)
	{
		pcpConnInfo = connect_to_server_from_foreign_server(host_or_srv);
	}
	else
	{
		ereport(ERROR, (0, errmsg("Wrong number of argument.")));
	}

	if (gracefully)
	{
		pcpResInfo = pcp_detach_node_gracefully(pcpConnInfo, nodeID);
	}
	else
	{
		pcpResInfo = pcp_detach_node(pcpConnInfo, nodeID);
	}

	if (pcpResInfo == NULL || PCPResultStatus(pcpResInfo) != PCP_RES_COMMAND_OK)
	{
		char	   *error = pcp_get_last_error(pcpConnInfo) ? pstrdup(pcp_get_last_error(pcpConnInfo)) : NULL;

		pcp_disconnect(pcpConnInfo);
		pcp_free_connection(pcpConnInfo);
		ereport(ERROR, (0,
						errmsg("failed to detach node"),
						errdetail("%s\n", error ? error : "unknown reason")));
	}

	pcp_disconnect(pcpConnInfo);
	pcp_free_connection(pcpConnInfo);

	PG_RETURN_BOOL(true);
}
