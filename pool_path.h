/* -*-pgsql-c-*- */
/*
 *
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Portions Copyright (c) 2003-2007,	PgPool Global Development Group
 * Portions Copyright (c) 2004, PostgreSQL Global Development Group
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
 * pool_path.h.: interface to pool_path.c
 *
 */

#ifndef POOL_PATH_H
#define POOL_PATH_H

/*
 * MAXPGPATH: standard size of a pathname buffer in PostgreSQL (hence,
 * maximum usable pathname length is one less).
 *
 * We'd use a standard system header symbol for this, if there weren't
 * so many to choose from: MAXPATHLEN, MAX_PATH, PATH_MAX are all
 * defined by different "standards", and often have different values
 * on the same platform!  So we just punt and use a reasonably
 * generous setting here.
 */
#define MAXPGPATH       1024

#define IS_DIR_SEP(ch)  ((ch) == '/')
#define is_absolute_path(filename) \
( \
    ((filename)[0] == '/') \
)

/*
 * StrNCpy
 *  Like standard library function strncpy(), except that result string
 *  is guaranteed to be null-terminated --- that is, at most N-1 bytes
 *  of the source string will be kept.
 *  Also, the macro returns no result (too hard to do that without
 *  evaluating the arguments multiple times, which seems worse).
 *
 *  BTW: when you need to copy a non-null-terminated string (like a text
 *  datum) and add a null, do not do it with StrNCpy(..., len+1).  That
 *  might seem to work, but it fetches one byte more than there is in the
 *  text object.  One fine day you'll have a SIGSEGV because there isn't
 *  another byte before the end of memory.  Don't laugh, we've had real
 *  live bug reports from real live users over exactly this mistake.
 *  Do it honestly with "memcpy(dst,src,len); dst[len] = '\0';", instead.
 */
#define StrNCpy(dst,src,len) \
    do \
    { \
        char * _dst = (dst); \
        size_t _len = (len); \
\
        if (_len > 0) \
        { \
            strncpy(_dst, (src), _len); \
            _dst[_len-1] = '\0'; \
        } \
    } while (0)

extern void get_parent_directory(char *path);
extern void join_path_components(char *ret_path, const char *head, const char *tail);
extern void canonicalize_path(char *path);
	
#endif /* POOL_PATH_H */
