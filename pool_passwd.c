/* -*-pgsql-c-*- */
/*
 *
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2010	PgPool Global Development Group
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
 * Module to handle pool_passwd
 */

#include <string.h>
#include <errno.h>

#include "pool.h"
#include "pool_passwd.h"

static FILE *passwd_fd = NULL;	/* File descriptor for pool_passwd */

/*
 * Initialize this module.
 * If pool_passwd does not exist yet, create it.
 * Open pool_passwd.
 */
void pool_init_pool_passwd(char *pool_passwd_filename)
{
	if (passwd_fd)
		return;

	passwd_fd = fopen(pool_passwd_filename, "r+");
	if (!passwd_fd)
	{
		if (errno == ENOENT)
		{
			/* The file does not exist yet. Create it. */
			passwd_fd = fopen(pool_passwd_filename, "w+");
			if (passwd_fd)
				return;
		}

		pool_error("pool_init_pool_passwd: couldn't open %s. reason: %s",
				   pool_passwd_filename, strerror(errno));
	}
}

/*
 * Update passwd. If the user does not exist, create a new entry.
 * Returns 0 on success non 0 oterwise.
 */
int pool_create_passwdent(char *username, char *passwd)
{
	int len;
	int c;
	char name[32];
	char *p;
	int readlen;

	if (!passwd_fd)
	{
		pool_error("pool_create_passwdent: passwd_fd is NULL");
		return -1;
	}

	len = strlen(passwd);
	if (len != POOL_PASSWD_LEN)
	{
		pool_error("pool_create_passwdent: wrong password length:%d", len);
		return -1;
	}

	rewind(passwd_fd);
	name[0] = '\0';

	while (!feof(passwd_fd))
	{
		p = name;
		readlen = 0;

		while (readlen < sizeof(name))
		{
			c = fgetc(passwd_fd);
			if (c == EOF)
				break;
			else if (c == ':')
				break;

			readlen++;
			*p++ = c;
		}
		*p = '\0';

		if (!strcmp(username, name))
		{
			/* User name found. Update password. */
			while ((c = *passwd++))
			{
				fputc(c, passwd_fd);
			}
			fputc('\n', passwd_fd);
			return 0;
		}
		else
		{
			/* Skip password */
			while((c = fgetc(passwd_fd)) != EOF &&
				  c != '\n')
				;
		}
	}

	/*
	 * Not found the user name.
	 * Create a new entry.
	 */
	while ((c = *username++))
	{
		fputc(c, passwd_fd);
	}
	fputc(':', passwd_fd);
	while ((c = *passwd++))
	{
		fputc(c, passwd_fd);
	}
	fputc('\n', passwd_fd);

	return 0;
}

/*
 * Get password in pool_passwd by username. Returns NULL if specified
 * entry does not exist or error occurred.
 * Returned password is on the static memory.
 */
char *pool_get_passwd(char *username)
{
	int c;
	char name[32];
	static char passwd[POOL_PASSWD_LEN+1];
	char *p;
	int readlen;

	if (!passwd_fd)
	{
		pool_error("pool_get_passwd: passwd_fd is NULL");
		return NULL;
	}

	rewind(passwd_fd);
	name[0] = '\0';

	while (!feof(passwd_fd))
	{
		p = name;
		readlen = 0;

		while (readlen < sizeof(name))
		{
			c = fgetc(passwd_fd);
			if (c == EOF)
				break;
			else if (c == ':')
				break;

			readlen++;
			*p++ = c;
		}
		*p = '\0';

		if (!strcmp(username, name))
		{
			/* User name found. Return password. */
			p = passwd;
			readlen = 0;

			while((c = fgetc(passwd_fd)) != EOF &&
				  c != '\n' && readlen < (sizeof(passwd)-1))
			{
				*p++ = c;
				readlen++;
			}
			*p = '\0';
			return passwd;
		}
		else
		{
			/* Skip password */
			while((c = fgetc(passwd_fd)) != EOF &&
				  c != '\n')
				;
		}
	}
	return NULL;
}

/*
 * Delete the entry by username. If specified entry does not exist,
 * does nothing.
 */
void pool_delete_passwdent(char *username)
{
}

/*
 * Finish this moule. Close pool_passwd.
 */
void pool_finish_pool_passwd(void)
{
	if (passwd_fd)
	{
		fclose(passwd_fd);
		passwd_fd = NULL;
	}
}
