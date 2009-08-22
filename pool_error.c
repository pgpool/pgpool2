/* -*-pgsql-c-*- */
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
 * pool_error.c: error and debug messages
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include "pool.h"

#define MAXSTRFTIME 128

extern int debug;

static char *nowsec(void);

void pool_error(const char *fmt,...)
{
	va_list		ap;
#ifdef HAVE_ASPRINTF
	char		*fmt2;
#endif

#ifdef HAVE_SIGPROCMASK
	sigset_t oldmask;
#else
	int	oldmask;
#endif

	POOL_SETMASK2(&BlockSig, &oldmask);

	if (pool_config->print_timestamp)
#ifdef HAVE_ASPRINTF
	  asprintf(&fmt2, "%s ERROR: pid %d: %s\n", nowsec(), (int)getpid(), fmt);
	else
	  asprintf(&fmt2, "ERROR: pid %d: %s\n", (int)getpid(), fmt);

   if (fmt2)
   {
     va_start(ap, fmt);
     vfprintf(stderr, fmt2, ap);
     va_end(ap);
     fflush(stderr);
	 free(fmt2);
   }
#else
	  fprintf(stderr, "%s ERROR: pid %d: ", nowsec(), (int)getpid());
	else
	  fprintf(stderr, "ERROR: pid %d: ", (int)getpid());

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
#endif

	POOL_SETMASK(&oldmask);
}

void pool_debug(const char *fmt,...)
{
	va_list		ap;
#ifdef HAVE_ASPRINTF
	char		*fmt2;
#endif

#ifdef HAVE_SIGPROCMASK
	sigset_t oldmask;
#else
	int	oldmask;
#endif

	if (!debug)
		return;

	POOL_SETMASK2(&BlockSig, &oldmask);

	if (pool_config->print_timestamp)
#ifdef HAVE_ASPRINTF
	  asprintf(&fmt2, "%s DEBUG: pid %d: %s\n", nowsec(), (int)getpid(), fmt);
	else
	  asprintf(&fmt2, "DEBUG: pid %d: %s\n", (int)getpid(), fmt);

   if (fmt2)
   {
     va_start(ap, fmt);
     vfprintf(stderr, fmt2, ap);
     va_end(ap);
     fflush(stderr);
	 free(fmt2);
   }
#else
	  fprintf(stderr, "%s DEBUG: pid %d: ", nowsec(), (int)getpid());
	else
	  fprintf(stderr, "DEBUG: pid %d: ", (int)getpid());

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
#endif

	POOL_SETMASK(&oldmask);
}

void pool_log(const char *fmt,...)
{
	va_list		ap;
#ifdef HAVE_ASPRINTF
	char		*fmt2;
#endif

#ifdef HAVE_SIGPROCMASK
	sigset_t oldmask;
#else
	int	oldmask;
#endif

	POOL_SETMASK2(&BlockSig, &oldmask);

	if (pool_config->print_timestamp)
#ifdef HAVE_ASPRINTF
	  asprintf(&fmt2, "%s LOG:   pid %d: %s\n", nowsec(), (int)getpid(), fmt);
	else
	  asprintf(&fmt2, "LOG:   pid %d: %s\n", (int)getpid(), fmt);

   if (fmt2)
   {
     va_start(ap, fmt);
     vfprintf(stderr, fmt2, ap);
     va_end(ap);
     fflush(stderr);
	 free(fmt2);
   }
#else
	  fprintf(stderr, "%s LOG:   pid %d: ", nowsec(), (int)getpid());
	else
	  fprintf(stderr, "LOG:   pid %d: ", (int)getpid());

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
#endif

	POOL_SETMASK(&oldmask);
}

static char *nowsec(void)
{
	static char strbuf[MAXSTRFTIME];
	time_t now = time(NULL);

	strftime(strbuf, MAXSTRFTIME, "%Y-%m-%d %H:%M:%S", localtime(&now));
	return strbuf;
}
