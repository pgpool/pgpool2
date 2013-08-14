/*-------------------------------------------------------------------------
 *
 * md5.h
 *	  Interface to md5.c
 *
 * Portions Copyright (c) 1996-2005, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $Header$
 *
 *-------------------------------------------------------------------------
 */

/*
 *  This file is imported from PostgreSQL 8.1.3.
 *  Modified by Taiki Yamaguchi <yamaguchi@sraoss.co.jp>
 */

#ifndef MD5_H
#define MD5_H

#define MAX_USER_NAME_LEN 128
#define MD5_PASSWD_LEN 32

extern int pool_md5_hash(const void *buff, size_t len, char *hexsum);
extern int pool_md5_encrypt(const char *passwd, const char *salt, size_t salt_len, char *buf);

#endif
