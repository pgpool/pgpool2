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
 * pool_passwd.h: pool_passwd related definitions.
 *
 */

#ifndef POOL_PASSWD_H
#define POOL_PASSWD_H

#include "pool.h"

#define POOL_PASSWD_FILENAME "pool_passwd"
#define POOL_PASSWD_LEN 35

extern void pool_init_pool_passwd(char *pool_passwd_filename);
extern int pool_create_passwdent(char *username, char *passwd);
extern char *pool_get_passwd(char *username);
extern void pool_delete_passwdent(char *username);
extern void pool_finish_pool_passwd(void);
extern void pool_reopen_passwd_file(void);

#endif /* POOL_PASSWD_H */
