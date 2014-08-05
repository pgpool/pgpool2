/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * Copyright (c) 2006-2008, pgpool Global Development Group
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

#ifndef POOL_STRING_H
#define POOL_STRING_H
#include "pg_list.h"
#define STRING_SIZE 128

typedef struct
{
	int size;
	int len;
	char *data;
} String;

extern char *NameListToString(List *names);
extern String *init_string(char *str);
extern void string_append_string(String *string, String *append_data);
extern void string_append_char(String *string, char *append_data);
extern void free_string(String *string);
extern String *copy_string(String *string);

#endif /* POOL_STRING_H */
