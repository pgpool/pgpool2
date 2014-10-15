/*-------------------------------------------------------------------------
 *
 * pgpool_adm.h
 *
 *
 * Copyright (c) 2002-2011, PostgreSQL Global Development Group
 *
 * Author: Jehan-Guillaume (ioguix) de Rorthais <jgdr@dalibo.com>
 *
 * IDENTIFICATION
 *	  contrib/pgpool_adm/pgpool_adm.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef PGPOOL_ADM_H
#define PGPOOL_ADM_H

PG_MODULE_MAGIC;

typedef struct {
    char * host;
	int16 timeout;
	int16 port;
	char * user;
	char * pass;
} pcpConninfo;

void init_pcp_conninfo(pcpConninfo * pcp_conninfo);
void check_pcp_conninfo_props(pcpConninfo * pcp_conninfo);
int pcp_connect_conninfo(pcpConninfo * pcp_conninfo);
pcpConninfo get_pcp_conninfo_from_foreign_server(char * name);

Datum _pcp_node_info(PG_FUNCTION_ARGS);
Datum _pcp_pool_status(PG_FUNCTION_ARGS);
Datum _pcp_node_count(PG_FUNCTION_ARGS);
Datum _pcp_attach_node(PG_FUNCTION_ARGS);
Datum _pcp_detach_node(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(_pcp_node_info);
PG_FUNCTION_INFO_V1(_pcp_pool_status);
PG_FUNCTION_INFO_V1(_pcp_node_count);
PG_FUNCTION_INFO_V1(_pcp_attach_node);
PG_FUNCTION_INFO_V1(_pcp_detach_node);

#endif
