#include <stdio.h>
#include "watchdog.h"

extern int	wd_is_upper_ok(char *server_list);

int
main(int argc, char *argv[])
{
	int			rtn;

	rtn = wd_is_upper_ok(argv[1]);
	if (rtn == WD_OK)
	{
		printf("%s is ok", argv[1]);
	}
	else
	{
		printf("%s is ng", argv[1]);
	}
	return 0;
}
