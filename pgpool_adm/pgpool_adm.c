/*-------------------------------------------------------------------------
 *
 * pgpool_adm.c
 *
 *
 * Copyright (c) 2002-2011, PostgreSQL Global Development Group
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

#include <unistd.h>

#include "catalog/pg_type.h"
#include "funcapi.h"
#include "libpcp_ext.h"
#include "pgpool_adm.h"

/**
 * Init a pcp_conninfo struct with empty values
 * pcp_conninfo: pcpConninfo structure to intialize
 */
void
init_pcp_conninfo(pcpConninfo * pcp_conninfo)
{
	pcp_conninfo->host = NULL;
	pcp_conninfo->timeout = -1;
	pcp_conninfo->port = -1;
	pcp_conninfo->user = NULL;
	pcp_conninfo->pass = NULL;
}

/**
 * Checks the give pcp_conninfo has valid properties
 * pcp_conninfo: pcpConninfo structure to check
 *
 * exit the function call on error !
 */
void
check_pcp_conninfo_props(pcpConninfo * pcp_conninfo)
{
	if (pcp_conninfo->timeout < 0)
		ereport(ERROR, (0, errmsg("Timeout is out of range.")));

	if (pcp_conninfo->port < 0 || pcp_conninfo->port > 65535)
		ereport(ERROR, (0, errmsg("PCP port out of range.")));

	if (! pcp_conninfo->user)
		ereport(ERROR, (0, errmsg("No user given.")));

	if (! pcp_conninfo->pass)
		ereport(ERROR, (0, errmsg("No password given.")));
}

/**
 * Wrapper around pcp_connect
 * pcp_conninfo: pcpConninfo structure having pcp connection properties
 */
int
pcp_connect_conninfo(pcpConninfo * pcp_conninfo)
{
	pcp_set_timeout(pcp_conninfo->timeout);

	if (pcp_connect(pcp_conninfo->host, pcp_conninfo->port, pcp_conninfo->user, pcp_conninfo->pass))
	{
		return -1;
	}

	return 0;
}

/**
 * Returns a pcpConninfo structure filled from a foreign server
 * name: the name of the foreign server
 */
pcpConninfo
get_pcp_conninfo_from_foreign_server(char * name)
{
	Oid userid = GetUserId();
	pcpConninfo pcp_conninfo;
	/* raise an error if given foreign server doesn't exists */
	ForeignServer * foreign_server = GetForeignServerByName(name, false);
	UserMapping * user_mapping;
	ListCell * cell;
	
	init_pcp_conninfo(&pcp_conninfo);

	/* raise an error if the current user isn't mapped with the given foreign server */
	user_mapping = GetUserMapping(userid, foreign_server->serverid);

	foreach(cell, foreign_server->options)
	{
		DefElem * def = lfirst(cell);

		if (strcmp(def->defname, "host") == 0)
		{
			pcp_conninfo.host = pstrdup(strVal(def->arg));
		}
		else if (strcmp(def->defname, "port") == 0)
		{
			pcp_conninfo.port = atoi(strVal(def->arg));
		}
		else if (strcmp(def->defname, "timeout") == 0)
		{
			pcp_conninfo.timeout = atoi(strVal(def->arg));
		}
	}

	foreach(cell, user_mapping->options)
	{
		DefElem * def = lfirst(cell);

		if (strcmp(def->defname, "user") == 0)
		{
			pcp_conninfo.user = pstrdup(strVal(def->arg));
		}
		else if (strcmp(def->defname, "password") == 0)
		{
			pcp_conninfo.pass = pstrdup(strVal(def->arg));
		}
	}

	return pcp_conninfo;
}

/**
 * nodeID: the node id to get info from
 * host_or_srv: server name or ip address of the pgpool server
 * timeout: timeout
 * port: pcp port number
 * user: user to connect with
 * pass: password
 **/
Datum
_pcp_node_info(PG_FUNCTION_ARGS)
{
	int16  nodeID = PG_GETARG_INT16(0);
	char * host_or_srv = text_to_cstring(PG_GETARG_TEXT_PP(1));
	pcpConninfo pcp_conninfo;

	BackendInfo * backend_info = NULL;
	Datum values[4]; /* values to build the returned tuple from */
	bool nulls[] = {false, false, false, false};
	TupleDesc tupledesc;
	HeapTuple tuple;

	if (nodeID < 0 || nodeID >= MAX_NUM_BACKENDS)
		ereport(ERROR, (0, errmsg("NodeID is out of range.")));

	init_pcp_conninfo(&pcp_conninfo);

	if (PG_NARGS() == 6)
	{
		pcp_conninfo.host = host_or_srv;
		pcp_conninfo.timeout = PG_GETARG_INT16(2);
		pcp_conninfo.port = PG_GETARG_INT16(3);
		pcp_conninfo.user = text_to_cstring(PG_GETARG_TEXT_PP(4));
		pcp_conninfo.pass = text_to_cstring(PG_GETARG_TEXT_PP(5));
	}
	else if (PG_NARGS() == 2)
	{
		pcp_conninfo = get_pcp_conninfo_from_foreign_server(host_or_srv);
	}
	else
	{
		ereport(ERROR, (0, errmsg("Wrong number of argument.")));
	}

	check_pcp_conninfo_props(&pcp_conninfo);

	/**
	 * Construct a tuple descriptor for the result rows.
	 **/
	tupledesc = CreateTemplateTupleDesc(4, false);
	TupleDescInitEntry(tupledesc, (AttrNumber) 1, "hostname", TEXTOID, -1, 0);
	TupleDescInitEntry(tupledesc, (AttrNumber) 2, "port", INT4OID, -1, 0);
	TupleDescInitEntry(tupledesc, (AttrNumber) 3, "status", TEXTOID, -1, 0);
	TupleDescInitEntry(tupledesc, (AttrNumber) 4, "weight", FLOAT4OID, -1, 0);
	tupledesc = BlessTupleDesc(tupledesc);

	/**
	 * PCP session
	 **/
	if (pcp_connect_conninfo(&pcp_conninfo))
	{
		ereport(ERROR,(0, errmsg("Cannot connect to PCP server.")));
	}

	if ((backend_info = pcp_node_info(nodeID)) == NULL)
	{
		pcp_disconnect();
		ereport(ERROR,(0, errmsg("Cannot get node information.")));
	}

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
	values[3] = Float8GetDatum(backend_info->backend_weight/RAND_MAX);
	nulls[3] = false;

	free(backend_info);
	pcp_disconnect();

	/* build and return the tuple */
	tuple = heap_form_tuple(tupledesc, values, nulls);

	ReleaseTupleDesc(tupledesc);

	PG_RETURN_DATUM(HeapTupleGetDatum(tuple));
}

/**
 * host_or_srv: server name or ip address of the pgpool server
 * timeout: timeout
 * port: pcp port number
 * user: user to connect with
 * pass: password
 **/
Datum
_pcp_pool_status(PG_FUNCTION_ARGS)
{
	MemoryContext oldcontext;
	FuncCallContext *funcctx;
	POOL_REPORT_CONFIG *status;
	int32 nrows;
	int32 call_cntr;
	int32 max_calls;
	AttInMetadata *attinmeta;

	/* stuff done only on the first call of the function */
	if (SRF_IS_FIRSTCALL())
	{
		TupleDesc tupdesc;
		char * host_or_srv = text_to_cstring(PG_GETARG_TEXT_PP(0));
		pcpConninfo pcp_conninfo;

		init_pcp_conninfo(&pcp_conninfo);

		/* create a function context for cross-call persistence */
		funcctx = SRF_FIRSTCALL_INIT();

		/* switch to memory context appropriate for multiple function calls */
		oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		if (PG_NARGS() == 5)
		{
			pcp_conninfo.host = host_or_srv;
			pcp_conninfo.timeout = PG_GETARG_INT16(1);
			pcp_conninfo.port = PG_GETARG_INT16(2);
			pcp_conninfo.user = text_to_cstring(PG_GETARG_TEXT_PP(3));
			pcp_conninfo.pass = text_to_cstring(PG_GETARG_TEXT_PP(4));
		}
		else if (PG_NARGS() == 1)
		{
			pcp_conninfo = get_pcp_conninfo_from_foreign_server(host_or_srv);
		}
		else
		{
			MemoryContextSwitchTo(oldcontext);
			ereport(ERROR, (0, errmsg("Wrong number of argument.")));
		}

		check_pcp_conninfo_props(&pcp_conninfo);

		/* get configuration and status */
		/**
		 * PCP session
		 **/
		if (pcp_connect_conninfo(&pcp_conninfo))
		{
			ereport(ERROR,(0, errmsg("Cannot connect to PCP server.")));
		}

		if ((status = pcp_pool_status(&nrows)) == NULL)
		{
			pcp_disconnect();
			ereport(ERROR,(0, errmsg("Cannot pool status information.")));
		}

		pcp_disconnect();

		/* Construct a tuple descriptor for the result rows */
		tupdesc = CreateTemplateTupleDesc(3, false);
		TupleDescInitEntry(tupdesc, (AttrNumber) 1, "item", TEXTOID, -1, 0);
		TupleDescInitEntry(tupdesc, (AttrNumber) 2, "value", TEXTOID, -1, 0);
		TupleDescInitEntry(tupdesc, (AttrNumber) 3, "description", TEXTOID, -1, 0);

		/*
		 * Generate attribute metadata needed later to produce tuples from raw
		 * C strings
		 */
		attinmeta = TupleDescGetAttInMetadata(tupdesc);
		funcctx->attinmeta = attinmeta;

		if ((status != NULL) && (nrows > 0))
		{
			funcctx->max_calls = nrows;

			/* got results, keep track of them */
			funcctx->user_fctx = status;
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

	status = (POOL_REPORT_CONFIG*) funcctx->user_fctx;
	attinmeta = funcctx->attinmeta;

	if (call_cntr < max_calls)	/* executed while there is more left to send */
	{
		char * values[3];
		HeapTuple tuple;
		Datum result;

		values[0] = pstrdup(status[call_cntr].name);
		values[1] = pstrdup(status[call_cntr].value);
		values[2] = pstrdup(status[call_cntr].desc);

		/* build the tuple */
		tuple = BuildTupleFromCStrings(attinmeta, values);

		/* make the tuple into a datum */
		result = HeapTupleGetDatum(tuple);

		SRF_RETURN_NEXT(funcctx, result);
	}
	else
	{
		/* do when there is no more left */
		SRF_RETURN_DONE(funcctx);
	}

	free(status);
}

/**
 * nodeID: the node id to get info from
 * host_or_srv: server name or ip address of the pgpool server
 * timeout: timeout
 * port: pcp port number
 * user: user to connect with
 * pass: password
 **/
Datum
_pcp_node_count(PG_FUNCTION_ARGS)
{
	char * host_or_srv = text_to_cstring(PG_GETARG_TEXT_PP(0));
	pcpConninfo pcp_conninfo;
	int16 node_count = 0;

	init_pcp_conninfo(&pcp_conninfo);

	if (PG_NARGS() == 5)
	{
		pcp_conninfo.host = host_or_srv;
		pcp_conninfo.timeout = PG_GETARG_INT16(1);
		pcp_conninfo.port = PG_GETARG_INT16(2);
		pcp_conninfo.user = text_to_cstring(PG_GETARG_TEXT_PP(3));
		pcp_conninfo.pass = text_to_cstring(PG_GETARG_TEXT_PP(4));
	}
	else if (PG_NARGS() == 1)
	{
		pcp_conninfo = get_pcp_conninfo_from_foreign_server(host_or_srv);
	}
	else
	{
		ereport(ERROR, (0, errmsg("Wrong number of argument.")));
	}

	check_pcp_conninfo_props(&pcp_conninfo);

	/**
	 * PCP session
	 **/
	if (pcp_connect_conninfo(&pcp_conninfo))
	{
		ereport(ERROR,(0, errmsg("Cannot connect to PCP server.")));
	}

	if ((node_count = pcp_node_count()) == -1)
	{
		pcp_disconnect();
		ereport(ERROR,(0, errmsg("Cannot get node count.")));
	}

	pcp_disconnect();

	PG_RETURN_INT16(node_count);
}

/**
 * nodeID: the node id to get info from
 * host_or_srv: server name or ip address of the pgpool server
 * timeout: timeout
 * port: pcp port number
 * user: user to connect with
 * pass: password
 **/
Datum
_pcp_attach_node(PG_FUNCTION_ARGS)
{
	int16  nodeID = PG_GETARG_INT16(0);
	char * host_or_srv = text_to_cstring(PG_GETARG_TEXT_PP(1));
	pcpConninfo pcp_conninfo;
	int status;

	if (nodeID < 0 || nodeID >= MAX_NUM_BACKENDS)
		ereport(ERROR, (0, errmsg("NodeID is out of range.")));

	init_pcp_conninfo(&pcp_conninfo);

	if (PG_NARGS() == 6)
	{
		pcp_conninfo.host = host_or_srv;
		pcp_conninfo.timeout = PG_GETARG_INT16(2);
		pcp_conninfo.port = PG_GETARG_INT16(3);
		pcp_conninfo.user = text_to_cstring(PG_GETARG_TEXT_PP(4));
		pcp_conninfo.pass = text_to_cstring(PG_GETARG_TEXT_PP(5));
	}
	else if (PG_NARGS() == 2)
	{
		pcp_conninfo = get_pcp_conninfo_from_foreign_server(host_or_srv);
	}
	else
	{
		ereport(ERROR, (0, errmsg("Wrong number of argument.")));
	}

	check_pcp_conninfo_props(&pcp_conninfo);

	/**
	 * PCP session
	 **/
	if (pcp_connect_conninfo(&pcp_conninfo))
	{
		ereport(ERROR,(0, errmsg("Cannot connect to PCP server.")));
	}

	status = pcp_attach_node(nodeID);

	pcp_disconnect();

	if (status == -1)
	{
		PG_RETURN_BOOL(false);
	}

	PG_RETURN_BOOL(true);
}

/**
 * nodeID: the node id to get info from
 * gracefully: detach node gracefully if true
 * host_or_srv: server name or ip address of the pgpool server
 * timeout: timeout
 * port: pcp port number
 * user: user to connect with
 * pass: password
 **/
Datum
_pcp_detach_node(PG_FUNCTION_ARGS)
{
	int16 nodeID = PG_GETARG_INT16(0);
	bool gracefully = PG_GETARG_BOOL(1);
	char * host_or_srv = text_to_cstring(PG_GETARG_TEXT_PP(2));
	pcpConninfo pcp_conninfo;
	int status;

	if (nodeID < 0 || nodeID >= MAX_NUM_BACKENDS)
		ereport(ERROR, (0, errmsg("NodeID is out of range.")));

	init_pcp_conninfo(&pcp_conninfo);

	if (PG_NARGS() == 7)
	{
		pcp_conninfo.host = host_or_srv;
		pcp_conninfo.timeout = PG_GETARG_INT16(3);
		pcp_conninfo.port = PG_GETARG_INT16(4);
		pcp_conninfo.user = text_to_cstring(PG_GETARG_TEXT_PP(5));
		pcp_conninfo.pass = text_to_cstring(PG_GETARG_TEXT_PP(6));
	}
	else if (PG_NARGS() == 3)
	{
		pcp_conninfo = get_pcp_conninfo_from_foreign_server(host_or_srv);
	}
	else
	{
		ereport(ERROR, (0, errmsg("Wrong number of argument.")));
	}

	check_pcp_conninfo_props(&pcp_conninfo);

	/**
	 * PCP session
	 **/
	if (pcp_connect_conninfo(&pcp_conninfo))
	{
		ereport(ERROR,(0, errmsg("Cannot connect to PCP server.")));
	}

	if (gracefully)
	{
		status = pcp_detach_node_gracefully(nodeID);
	}
	else
	{
		status = pcp_detach_node(nodeID);
	}

	pcp_disconnect();

	if (status == -1)
	{
		PG_RETURN_BOOL(false);
	}

	PG_RETURN_BOOL(true);
}
