/* -*-pgsql-c-*- */
/*
 *
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
 * Module to handle pool_passwd
 */

#include <string.h>
#include <errno.h>

#include "pool.h"
#include "auth/pool_passwd.h"
#include "auth/md5.h"
#ifndef POOL_PRIVATE
#include "utils/elog.h"
#else
#include "utils/fe_ports.h"
#endif



static FILE *passwd_fd = NULL;	/* File descriptor for pool_passwd */
static char saved_passwd_filename[POOLMAXPATHLEN+1];

/*
 * Initialize this module.
 * If pool_passwd does not exist yet, create it.
 * Open pool_passwd.
 */
void pool_init_pool_passwd(char *pool_passwd_filename)
{
	if (passwd_fd)
		return;

	if (saved_passwd_filename[0] == '\0')
	{
		int len = strlen(pool_passwd_filename);
		memcpy(saved_passwd_filename, pool_passwd_filename, len);
		saved_passwd_filename[len] = '\0';
	}

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
		ereport(ERROR,
			(errmsg("initializing pool password, failed to open file:\"%s\"",pool_passwd_filename),
				 errdetail("file open failed with error:\"%s\"",strerror(errno))));
	}
}

/*
 * Update passwd. If the user does not exist, create a new entry.
 * Returns 0 on success non 0 otherwise.
 */
int pool_create_passwdent(char *username, char *passwd)
{
	int len;
	int c;
	char name[MAX_USER_NAME_LEN];
	char *p;
	int readlen;

	if (!passwd_fd)
		ereport(ERROR,
				(errmsg("error updating password, password file descriptor is NULL")));

	len = strlen(passwd);
	if (len != POOL_PASSWD_LEN)
		ereport(ERROR,
				(errmsg("error updating password, invalid password length:%d",len)));

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

	fseek(passwd_fd, 0, SEEK_END);

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
	char name[MAX_USER_NAME_LEN+1];
	static char passwd[POOL_PASSWD_LEN+1];
	char *p;
	int readlen;

	if (!username)
		ereport(ERROR,
				(errmsg("unable to get password, username is NULL")));

	if (!passwd_fd)
		ereport(ERROR,
				(errmsg("unable to get password, password file descriptor is NULL")));

	rewind(passwd_fd);
	name[0] = '\0';

	while (!feof(passwd_fd))
	{
		p = name;
		readlen = 0;

		while (readlen < (sizeof(name)-1))
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
 * Finish this moil. Close pool_passwd.
 */
void pool_finish_pool_passwd(void)
{
	if (passwd_fd)
	{
		fclose(passwd_fd);
		passwd_fd = NULL;
	}
}

void pool_reopen_passwd_file(void)
{
	pool_finish_pool_passwd();
	pool_init_pool_passwd(saved_passwd_filename);
}
