/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2013	PgPool Global Development Group
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
#include "pool_config.h"
#ifndef POOL_TOOLS
#include "context/pool_session_context.h"
#endif

#define MAXSTRFTIME 128

extern int debug;

#ifndef POOL_TOOLS
static char *nowsec(void);
#endif
static char *optstring(int kind);

void pool_error(const char *fmt,...)
{
	va_list		ap;
#ifdef HAVE_ASPRINTF
	char		*fmt2;
    int         len;
#endif

#ifdef HAVE_SIGPROCMASK
	sigset_t oldmask;
#else
	int	oldmask;
#endif
	POOL_SETMASK2(&BlockSig, &oldmask);

	/* Write message to syslog */
	if (pool_config->logsyslog == 1)
	{
		va_start(ap, fmt);
		vsyslog(pool_config->syslog_facility | LOG_ERR, fmt, ap);
		va_end(ap);
		POOL_SETMASK(&oldmask);
		return;
	}

#ifdef HAVE_ASPRINTF
	len = asprintf(&fmt2, "%s %s\n", optstring(0), fmt);

	if (len >= 0 && fmt2)
	{
		va_start(ap, fmt);
		vfprintf(stderr, fmt2, ap);
		va_end(ap);
		fflush(stderr);
		free(fmt2);
	}
#else
	fprintf(stderr, "%s %s", optstring(0));

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
#endif

	POOL_SETMASK(&oldmask);
}


#ifndef POOL_TOOLS
static char *nowsec(void)
{
	static char strbuf[MAXSTRFTIME];
	time_t now = time(NULL);

	strftime(strbuf, MAXSTRFTIME, "%Y-%m-%d %H:%M:%S", localtime(&now));
	return strbuf;
}
#endif

#ifndef HAVE_VSYSLOG
void vsyslog (int priority, const char *format, va_list ap)
{
#define MAXSYSLOGMSGLEN 1024

	char *msg = NULL;

#ifdef HAVE_VASPRINTF
	vasprintf(&msg, format, ap);
	if (!msg)
		return;
#else
	msg = malloc(MAXSYSLOGMSGLEN);
	if (!msg)
		return;

	va_start(ap, format);
	vsnprintf(msg, MAXSYSLOGMSGLEN, format, ap);
	va_end(ap);
#endif

	syslog(priority, "%s", msg);
	free(msg);
}
#endif /* HAVE_VSYSLOG */

/*
 * Create "ERROR" etc., timestamp, pid, user name string and return
 * it. The returned string is in a static buff and subsequent calls
 * will overwrite it.
 */
static char *optstring(int kind)
{
	static char *kindstr[] = {"ERROR:", "DEBUG:", "LOG:  "};
	char timebuf[MAXSTRFTIME];
	time_t now;
	static char optbuf[MAXSTRFTIME+7+8+NAMEDATALEN];
#ifndef POOL_TOOLS
	char username[NAMEDATALEN];
#endif
	char buf[128];
#ifndef POOL_TOOLS
	POOL_SESSION_CONTEXT *c;
#endif

	optbuf[0] = '\0';

	if (pool_config->print_timestamp)
	{
		now = time(NULL);
		strftime(timebuf, MAXSTRFTIME, "%Y-%m-%d %H:%M:%S ", localtime(&now));
		strcat(optbuf, timebuf);
	}

	snprintf(buf, sizeof(buf), "%s pid: %d", kindstr[kind], (int)getpid());
	strcat(optbuf, buf);

#ifndef POOL_TOOLS
	if (pool_config->print_user)
	{
		if ((c = pool_get_session_context(true)))
			if (MASTER_CONNECTION(c->backend) && MASTER_CONNECTION(c->backend)->sp &&
				MASTER_CONNECTION(c->backend)->sp->user)
			{
				strlcpy(username, MASTER_CONNECTION(c->backend)->sp->user, sizeof(username));
				strcat(optbuf, " user: ");
				strcat(optbuf, username);
			}
	}
#endif

	return optbuf;
}
