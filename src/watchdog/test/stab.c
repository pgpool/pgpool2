#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <wait.h>

#include "pool.h"
#include "pool_config.h"

#define MAXSTRFTIME 128

int			debug = 0;


static char *nowsec(void);
static void child_wait(int signo);

void
pool_error(const char *fmt,...)
{
	va_list		ap;
	pool_sigset_t oldmask;
#ifdef HAVE_ASPRINTF
	char	   *fmt2;
	int			len;
#endif
	/* Write error message to syslog */
	if (pool_config->logsyslog == 1)
	{
		va_start(ap, fmt);
		vsyslog(pool_config->syslog_facility | LOG_ERR, fmt, ap);
		va_end(ap);
		return;
	}

	POOL_SETMASK2(&BlockSig, &oldmask);

	/*
	 * TODO if (pool_config->print_timestamp)
	 */
#ifdef HAVE_ASPRINTF
	len = asprintf(&fmt2, "%s ERROR: pid %d: %s\n", nowsec(), (int) getpid(), fmt);

	if (len >= 0 && fmt2)
	{
		va_start(ap, fmt);
		vfprintf(stderr, fmt2, ap);
		va_end(ap);
		fflush(stderr);
		free(fmt2);
	}
#else
	fprintf(stderr, "%s ERROR: pid %d: ", nowsec(), (int) getpid());

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
#endif

	POOL_SETMASK(&oldmask);
}

static char *
nowsec(void)
{
	static char strbuf[MAXSTRFTIME];
	time_t		now = time(NULL);

	strftime(strbuf, MAXSTRFTIME, "%Y-%m-%d %H:%M:%S", localtime(&now));
	return strbuf;
}

size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	char	   *d = dst;
	const char *s = src;
	size_t		n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0)
	{
		while (--n != 0)
		{
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0)
	{
		if (siz != 0)
			*d = '\0';			/* NUL-terminate dst */
		while (*s++)
			;
	}

	return (s - src - 1);		/* count does not include NUL */
}

static void
child_wait(int signo)
{
	pid_t		pid = 0;

	do
	{
		int			ret;

		pid = waitpid(-1, &ret, WNOHANG);
	} while (pid > 0);
}

void
wd_exit(int exit_signo)
{
	sigset_t	mask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	wd_notice_server_down();

	pool_shmem_exit(1);

	kill(0, exit_signo);

	child_wait(0);

	exit(0);
}

int
pool_memset_system_db_info(SystemDBInfo * info)
{
	return 0;
}

int
pool_query_cache_table_exists(void)
{
	return 0;
}
