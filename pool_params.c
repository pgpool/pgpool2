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
 * params.c: Paramter Status handling routines
 *
 */
#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "pool.h"
#include "parser/parser.h"

#define MAX_PARAM_ITEMS 128

/*
 * initialize parameter structure
 */
int pool_init_params(ParamStatus *params)
{
    params->num = 0;
    params->names = malloc(MAX_PARAM_ITEMS*sizeof(char *));
	if (params->names == NULL)
	{
		pool_error("pool_init_params: cannot allocate memory");
		return -1;
	}
    params->values = malloc(MAX_PARAM_ITEMS*sizeof(char *));
	if (params->values == NULL)
	{
		pool_error("pool_init_params: cannot allocate memory");
		return -1;
	}
	return 0;
}

/*
 * discard parameter structure
 */
void pool_discard_params(ParamStatus *params)
{
    int i;

    for (i=0;i<params->num;i++)
    {
		free(params->names[i]);
		free(params->values[i]);
    }
    free(params->names);
    free(params->values);
}

/*
 * find param value by name. if found, its value is returned
 * also, pos is set
 * if not found, NULL is returned
 */
char *pool_find_name(ParamStatus *params, char *name, int *pos)
{
    int i;

    for (i=0;i<params->num;i++)
    {
		if (!strcmp(name, params->names[i]))
		{
			*pos = i;
			return params->values[i];
		}
    }
    return NULL;
}

/*
 * return name and value by index.
 */
int pool_get_param(ParamStatus *params, int index, char **name, char **value)
{
	if (index < 0 || index >= params->num)
		return -1;

	*name = params->names[index];
	*value = params->values[index];

	return 0;
}

/*
 * add or replace name/value pair
 */
int pool_add_param(ParamStatus *params, char *name, char *value)
{
    int pos;

    if (pool_find_name(params, name, &pos))
    {
		/* name already exists */
		if (strlen(params->values[pos]) < strlen(value))
		{
			params->values[pos] = realloc(params->values[pos], strlen(value) + 1);
			if (params->values[pos] == NULL)
			{
				pool_error("pool_init_params: cannot allocate memory");
				return -1;
			}
		}
		strcpy(params->values[pos], value);
    }
    else
    {
		int num;

		/* add name/value pair */
		if (params->num >= MAX_PARAM_ITEMS)
		{
			pool_error("pool_add_param: no more room for num");
			return -1;
		}
		num = params->num;
		params->names[num] = strdup(name);
		if (params->names[num] == NULL)
		{
			pool_error("pool_init_params: cannot allocate memory");
			return -1;
		}
		params->values[num] = strdup(value);
		if (params->values[num] == NULL)
		{
			pool_error("pool_init_params: cannot allocate memory");
			return -1;
		}
		params->num++;
    }
	parser_set_param(name, value);
	return 0;
}

void pool_param_debug_print(ParamStatus *params)
{
	int i;

    for (i=0;i<params->num;i++)
    {
		pool_debug("No.%d: name: %s value: %s", i, params->names[i], params->values[i]);
	}
}
