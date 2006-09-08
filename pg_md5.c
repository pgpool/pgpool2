#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md5.h"

int main(int argc, char *argv[])
{
	char md5[33];

	if (argc != 2)
	{
		fprintf(stderr, "Usage: md5 _string_\n");
		exit(1);
	}

	pool_md5_hash(argv[1], strlen(argv[1]), md5);
	printf("%s\n", md5);

	return 0;
}
