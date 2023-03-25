/*-------------------------------------------------------------------------
 *
 * pgstrcasecmp.h
 *	  Header for src/utils/pgstrcasecmp.c compatibility functions.
 * Portions Copyright (c) 1996-2023, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/utils/pgstrcasecmp.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef POOL_PGSTRCASECMP
#define POOL_PGSTRCASECMP

/* Portable SQL-like case-independent comparisons and conversions */
extern int	pg_strcasecmp(const char *s1, const char *s2);
extern int	pg_strncasecmp(const char *s1, const char *s2, size_t n);
extern unsigned char pg_toupper(unsigned char ch);
extern unsigned char pg_tolower(unsigned char ch);
extern unsigned char pg_ascii_toupper(unsigned char ch);
extern unsigned char pg_ascii_tolower(unsigned char ch);
#endif
