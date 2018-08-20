/*
 * Copyright (c) 2017-2018	Tatsuo Ishii
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

#include "../../include/config.h"
#include "pgproto/pgproto.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "pgproto/fe_memutils.h"
#include <libpq-fe.h>
#include "pgproto/read.h"
#include "pgproto/send.h"

/*
 * Send a character to the connection.
 */
void
send_char(char c, PGconn *conn)
{
	int			status;

	status = write(PQsocket(conn), &c, 1);
}

/*
 * Send a 4-byte integer to the connection.
 */
void
send_int(int intval, PGconn *conn)
{
	int			status;
	int			l = htonl(intval);

	status = write(PQsocket(conn), &l, sizeof(l));
}

/*
 * Send a 2-byte integer to the connection.
 */
void
send_int16(short shortval, PGconn *conn)
{
	int			status;
	short		s = htons(shortval);

	status = write(PQsocket(conn), &s, sizeof(s));
}

/*
 * Send a string to the connection. buf must be NULL terminated.
 */
void
send_string(char *buf, PGconn *conn)
{
	int			status;

	status = write(PQsocket(conn), buf, strlen(buf) + 1);
}

/*
 * Send byte to the connection.
 */
void
send_byte(char *buf, int len, PGconn *conn)
{
	int			status;

	status = write(PQsocket(conn), buf, len);
}
