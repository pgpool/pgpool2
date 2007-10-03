/* $Header$ */
#include "pool.h"
#include "pool_memory.h"
#include "parsenodes.h"
#include "gramparse.h"
#include "parse.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	List *tree;
	ListCell *l;

	if (argc != 2)
	{
		fprintf(stderr, "./parser-test query\n");
		exit(1);
	}

	tree = raw_parser(argv[1]);

	if (tree == NULL)
	{
		printf("syntax error: %s\n", argv[1]);
	}
	else
	{
		foreach(l, tree)
		{
			Node *node = (Node *) lfirst(l);
			printf("%s\n", nodeToString(node));
		}
	}

	free_parser();
	return 0;
}
