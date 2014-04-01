#include <sys/time.h>

struct timeval pcp_timeout;

void
pcp_set_timeout(long sec)
{
	/* disable timeout (wait forever!) (2008/02/08 yamaguti) */
	sec = 0;
	pcp_timeout.tv_sec = sec;
}

