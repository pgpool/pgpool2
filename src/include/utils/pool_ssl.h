/*
 *
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Portions Copyright (c) 2003-2026	PgPool Global Development Group
 * Portions Copyright (c) 1996-2026, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
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
 */

#ifndef pool_ssl_h
#define pool_ssl_h

#ifdef USE_SSL
/*
 *	Hardcoded DH parameters, used in ephemeral DH keying.
 *
 *	This is the 2048-bit DH parameter from RFC 3526.  The generation of the
 *	prime is specified in RFC 2412 Appendix E, which also discusses the
 *	design choice of the generator.  Note that when loaded with OpenSSL
 *	this causes DH_check() to fail on DH_NOT_SUITABLE_GENERATOR, where
 *	leaking a bit is preferred.
 */
#define FILE_DH2048 \
"-----BEGIN DH PARAMETERS-----\n\
MIIBCAKCAQEA///////////JD9qiIWjCNMTGYouA3BzRKQJOCIpnzHQCC76mOxOb\n\
IlFKCHmONATd75UZs806QxswKwpt8l8UN0/hNW1tUcJF5IW1dmJefsb0TELppjft\n\
awv/XLb0Brft7jhr+1qJn6WunyQRfEsf5kkoZlHs5Fs9wgB8uKFjvwWY2kg2HFXT\n\
mmkWP6j9JM9fg2VdI9yjrZYcYvNWIIVSu57VKQdwlpZtZww1Tkq8mATxdGwIyhgh\n\
fDKQXkYuNs474553LBgOhgObJ4Oi7Aeij7XFXfBvTFLJ3ivL9pVYFxg5lUl86pVq\n\
5RXSJhiY+gUQFXKOWoqsqmj//////////wIBAg==\n\
-----END DH PARAMETERS-----\n"
#endif

/*
 * Macro that allows to cast constness away from an expression, but doesn't
 * allow changing the underlying type.  Enforcement of the latter
 * currently only works for gcc like compilers.
 *
 * Please note IT IS NOT SAFE to cast constness away if the result will ever
 * be modified (it would be undefined behaviour). Doing so anyway can cause
 * compiler misoptimizations or runtime crashes (modifying readonly memory).
 * It is only safe to use when the the result will not be modified, but API
 * design or language restrictions prevent you from declaring that
 * (e.g. because a function returns both const and non-const variables).
 *
 * Note that this only works in function scope, not for global variables (it'd
 * be nice, but not trivial, to improve that).
 */
#if defined(HAVE__BUILTIN_TYPES_COMPATIBLE_P)
#define unconstify(underlying_type, expr) \
	(StaticAssertExpr(__builtin_types_compatible_p(__typeof(expr), const underlying_type), \
					  "wrong cast"), \
	  (underlying_type) (expr))
#else
#define unconstify(underlying_type, expr) \
	((underlying_type) (expr))
#endif


extern void pool_ssl_negotiate_serverclient(POOL_CONNECTION *cp);
extern void pool_ssl_negotiate_clientserver(POOL_CONNECTION *cp);
extern void pool_ssl_close(POOL_CONNECTION *cp);
extern int	pool_ssl_read(POOL_CONNECTION *cp, void *buf, int size);
extern int	pool_ssl_write(POOL_CONNECTION *cp, const void *buf, int size);
extern bool pool_ssl_pending(POOL_CONNECTION *cp);
extern int	SSL_ServerSide_init(void);


#endif							/* pool_ssl_h */
