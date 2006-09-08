/*
 * $Header$
 *
 * Handles errors occured in PCP modules.
 */

#include <stdio.h>

#include "pcp.h"

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
