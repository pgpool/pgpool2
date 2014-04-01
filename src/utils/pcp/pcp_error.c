/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2008	PgPool Global Development Group
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
 * Handles errors occured in PCP modules.
 */

#include <stdio.h>

#include "pcp/pcp.h"

ErrorCode errorcode;

void
pcp_errorstr(ErrorCode e)
{
	switch (e)
	{
		case EOFERR:
			fprintf(stdout, "EOFError\n");
			break;

		case NOMEMERR:
			fprintf(stdout, "NoMemoryError\n");
			break;

		case READERR:
			fprintf(stdout, "ReadError\n");
			break;

		case WRITEERR:
			fprintf(stdout, "WriteError\n");
			break;

		case TIMEOUTERR:
			fprintf(stdout, "TimeoutError\n");
			break;

		case INVALERR:
			fprintf(stdout, "InvalidArgumentError\n");
			break;

		case CONNERR:
			fprintf(stdout, "ConnectionError\n");
			break;

		case NOCONNERR:
			fprintf(stdout, "NoConnectionError\n");
			break;

		case SOCKERR:
			fprintf(stdout, "SocketError\n");
			break;

		case HOSTERR:
			fprintf(stdout, "HostError\n");
			break;

		case BACKENDERR:
			fprintf(stdout, "BackendError\n");
			break;

		case AUTHERR:
			fprintf(stdout, "AuthorizationError\n");
			break;

		case UNKNOWNERR:
		default:
			fprintf(stdout, "UnknownError\n");
			break;
	}
}
