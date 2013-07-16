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
 *
 * pcp.h - master header file.
 */

#ifndef PCP_H
#define PCP_H

#include "pool_type.h"
#include "watchdog/watchdog.h"

#define MAX_USER_PASSWD_LEN    128

typedef enum {
	UNKNOWNERR = 1,		/* shouldn't happen */
	EOFERR,				/* EOF read by read() */
	NOMEMERR,			/* could not allocate memory */
	READERR,			/* read() error */
	WRITEERR,			/* flush() error */
	TIMEOUTERR,			/* select() timeout */
	INVALERR,			/* invalid command-line argument(s) number, length, range, etc. */
	CONNERR,			/* thrown by connect() */
	NOCONNERR,			/* not connected to server */
	SOCKERR,			/* thrown by socket() or setsockopt() */
	HOSTERR,			/* thrown by gethostbyname() */
	BACKENDERR,			/* server dependent error */
	AUTHERR				/* authorization failure */
} ErrorCode;

/* --------------------------------
 * pcp.c
 * --------------------------------
 */
extern struct timeval pcp_timeout;

/* ------------------------------
 * pcp_error.c
 * ------------------------------
 */
extern ErrorCode errorcode;
extern void pcp_errorstr(ErrorCode e);

#endif /* PCP_H */
