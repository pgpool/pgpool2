/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * Copyright (c) 2006-2014, pgpool Global Development Group
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

#ifndef POOL_PARSER_H
#define POOL_PARSER_H

#include "../pool_type.h"
#include <setjmp.h>

extern jmp_buf jmpbuffer;
extern int	server_version_num;


/* include/c.h */
/* integer */

/* 
 * move int32 and int16 to pool_types.h since there's no point that
 * these are solely used for parser
 * typedef signed int int32;
 * typedef signed short int16;
 */

/*
 * bitsN
 *		Unit of bitwise operation, AT LEAST N BITS IN SIZE.
 */
typedef unsigned int Index;
typedef short AttrNumber;

#define InvalidOid		((Oid) 0)

/*
 * NAMEDATALEN is the max length for system identifiers (e.g. table names,
 * attribute names, function names, etc).  It must be a multiple of
 * sizeof(int) (typically 4).
 *
 * NOTE that databases with different NAMEDATALEN's cannot interoperate!
 */
#define NAMEDATALEN 64

/*
 * Max
 *		Return the maximum of two numbers.
 */
#define Max(x, y)		((x) > (y) ? (x) : (y))

/*
 * Min
 *		Return the minimum of two numbers.
 */
#define Min(x, y)		((x) < (y) ? (x) : (y))
/* array */
#define lengthof(array) (sizeof(array) / sizeof(((array)[0])))
#define endof(array) (&(array)[lengthof(array)])

/* msb for char */
#define HIGHBIT					(0x80)
#define IS_HIGHBIT_SET(ch)		((unsigned char)(ch) & HIGHBIT)



/* include/utils/datetime.h */
/* date and datetime */
#define RESERV	0
#define MONTH	1
#define YEAR	2
#define DAY		3
#define JULIAN	4
#define TZ		5
#define DTZ		6
#define DTZMOD	7
#define IGNORE_DTF	8
#define AMPM	9
#define HOUR	10
#define MINUTE	11
#define SECOND	12
#define DOY		13
#define DOW		14
#define UNITS	15
#define ADBC	16
/* these are only for relative dates */
#define AGO		17
#define ABS_BEFORE		18
#define ABS_AFTER		19
/* generic fields to help with parsing */
#define ISODATE 20
#define ISOTIME 21
/* reserved for unrecognized string values */
#define UNKNOWN_FIELD	31
#define MAX_TIMESTAMP_PRECISION 6
#define MAX_INTERVAL_PRECISION 6
#define INTERVAL_FULL_RANGE (0x7FFF)
#define INTERVAL_RANGE_MASK (0x7FFF)
#define INTERVAL_FULL_PRECISION (0xFFFF)
#define INTERVAL_PRECISION_MASK (0xFFFF)

#define INTERVAL_MASK(b) (1 << (b))
#define INTERVAL_TYPMOD(p,r) ((((r) & INTERVAL_RANGE_MASK) << 16) | ((p) & INTERVAL_PRECISION_MASK))
#define INTERVAL_PRECISION(t) ((t) & INTERVAL_PRECISION_MASK)
#define INTERVAL_RANGE(t) (((t) >> 16) & INTERVAL_RANGE_MASK)


/* include/storage/lock.h */
/* lock */
/* NoLock is not a lock mode, but a flag value meaning "don't get a lock" */
#define NoLock					0

#define AccessShareLock			1		/* SELECT */
#define RowShareLock			2		/* SELECT FOR UPDATE/FOR SHARE */
#define RowExclusiveLock		3		/* INSERT, UPDATE, DELETE */
#define ShareUpdateExclusiveLock 4		/* VACUUM (non-FULL) */
#define ShareLock				5		/* CREATE INDEX */
#define ShareRowExclusiveLock	6		/* like EXCLUSIVE MODE, but allows ROW
										 * SHARE */
#define ExclusiveLock			7		/* blocks ROW SHARE/SELECT...FOR
										 * UPDATE */
#define AccessExclusiveLock		8		/* ALTER TABLE, DROP TABLE, VACUUM
										 * FULL, and unqualified LOCK TABLE */

#define DEFAULT_INDEX_TYPE	"btree"
#define NUMERIC_MAX_PRECISION		1000

#define VARHDRSZ		((int32) sizeof(int32))

#define MaxAttrSize		(10 * 1024 * 1024)
#define BITS_PER_BYTE		8
#define MAX_TIME_PRECISION 6


/* ----------------------------------------------------------------
 *              Section 1: hacks to cope with non-ANSI C compilers
 *
 * type prefixes (const, signed, volatile, inline) are handled in pg_config.h.
 * ----------------------------------------------------------------
 */

/*
 * CppAsString
 *      Convert the argument to a string, using the C preprocessor.
 * CppConcat
 *      Concatenate two arguments together, using the C preprocessor.
 *
 * Note: the standard Autoconf macro AC_C_STRINGIZE actually only checks
 * whether #identifier works, but if we have that we likely have ## too.
 */
#if defined(HAVE_STRINGIZE)

#define CppAsString(identifier) #identifier
#define CppConcat(x, y)         x##y
#else                           /* !HAVE_STRINGIZE */

#define CppAsString(identifier) "identifier"

/*
 * CppIdentity -- On Reiser based cpp's this is used to concatenate
 *      two tokens.  That is
 *              CppIdentity(A)B ==> AB
 *      We renamed it to _private_CppIdentity because it should not
 *      be referenced outside this file.  On other cpp's it
 *      produces  A  B.
 */
#define _priv_CppIdentity(x)x
#define CppConcat(x, y)         _priv_CppIdentity(x)y
#endif   /* !HAVE_STRINGIZE */

#if 0
/* from include/catalog/pg_trigger.h  start */
/* Bits within tgtype */
#define TRIGGER_TYPE_ROW                (1 << 0)
#define TRIGGER_TYPE_BEFORE             (1 << 1)
#define TRIGGER_TYPE_INSERT             (1 << 2)
#define TRIGGER_TYPE_DELETE             (1 << 3)
#define TRIGGER_TYPE_UPDATE             (1 << 4)
#define TRIGGER_TYPE_TRUNCATE           (1 << 5)
/* from include/catalog/pg_trigger.h  end */
#endif


/* include/utils/elog.h 
#define NOTICE 18
#define WARNING 19
#define ERROR 20

extern void pool_parser_error(int level, const char *file, int line);
#ifndef ereport
#define ereport(elevel, rest) pool_parser_error(elevel, __FILE__, __LINE__)
#endif
#ifndef elog
#define elog(elevel, fmt, ...) pool_parser_error(elevel, __FILE__, __LINE__)
#endif
*/

#endif /* POOL_PARSER_H */
