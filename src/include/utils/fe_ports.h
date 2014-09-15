/*
 *
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2014	PgPool Global Development Group
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
 * fe_memutils.h
 *	  memory management support for frontend code
 *
 *-------------------------------------------------------------------------
 */

#ifndef POOL_PRIVATE
#error "This file is not expected to be compiled for pgpool utilities only"
#endif

#include <stdlib.h>
#ifndef FE_PORTS
#define FE_PORTS


extern int _fe_error_level;

void *pg_malloc(size_t size);

void *pg_malloc0(size_t size);
void *pg_realloc(void *ptr, size_t size);
char *pg_strdup(const char *in);
void pg_free(void *ptr);
void *palloc(unsigned int size);
void *palloc0(unsigned int size);
void pfree(void *pointer);
char *pstrdup(const char *in);
void *repalloc(void *pointer, unsigned int size);

#ifdef __GNUC__
extern int errhint(const char *fmt,...)
__attribute__((format (printf, 1, 2)));

extern int errdetail(const char *fmt,...)
__attribute__((format (printf, 1, 2)));

extern void errmsg(const char *fmt,...)
__attribute__((format (printf, 1, 2)));
#else
extern int errhint(const char *fmt,...);
extern int errdetail(const char *fmt,...);
extern void errmsg(const char *fmt,...);
#endif

extern int errstart(int elevel, const char *filename, int lineno,
					 const char *funcname);
extern void errfinish(int dummy,...);

/*
 * The following defines are taken from utils/error/elog.h
 * keep these values insync with defines in elog.h
 */
#define DEBUG5		10			/* Debugging messages, in categories of
* decreasing detail. */
#define DEBUG4		11
#define DEBUG3		12
#define DEBUG2		13
#define DEBUG1		14			/* used by GUC debug_* variables */
#define LOG			15			/* Server operational messages; sent only to
* server log by default. */
#define COMMERROR	16			/* Client communication problems; same as LOG
* for server reporting, but never sent to
* client. */
#define INFO		17			/* Messages specifically requested by user (eg
* VACUUM VERBOSE output); always sent to
* client regardless of client_min_messages,
* but by default not sent to server log. */
#define NOTICE		18			/* Helpful messages to users about query
* operation; sent to client and server log by
* default. */
#define WARNING		19			/* Warnings.  NOTICE is for expected messages
* like implicit sequence creation by SERIAL.
* WARNING is for unexpected messages. */
#define ERROR		20			/* user error - abort transaction; return to
* known state */
/* Save ERROR value in PGERROR so it can be restored when Win32 includes
 * modify it.  We have to use a constant rather than ERROR because macros
 * are expanded only when referenced outside macros.
 */

#ifdef WIN32
#define PGERROR		20
#endif
#define FATAL		21			/* fatal error - abort process */
#define PANIC		22			/* take down the other backends with me */

#define ereport(elevel, rest)	\
do { \
	_fe_error_level = elevel; \
	if (errstart(elevel, __FILE__, __LINE__, __FUNCTION__)) \
		rest; \
	if(elevel >= ERROR) \
		exit(-1); \
} while(0)
#endif   /* HAVE__BUILTIN_CONSTANT_P */
