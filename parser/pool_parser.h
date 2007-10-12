#ifndef POOL_PARSER_H
#define POOL_PARSER_H

#include "../pool_type.h"
#include <setjmp.h>

extern jmp_buf jmpbuffer;

/* integer */
typedef signed int int32;
typedef signed short int16;
typedef unsigned int PoolOid;
typedef unsigned int Index;
typedef short AttrNumber;
typedef unsigned long Datum;	/* XXX sizeof(long) >= sizeof(void *) */

#define Oid PoolOid
#define InvalidOid		((Oid) 0)

/*
 * NAMEDATALEN is the max length for system identifiers (e.g. table names,
 * attribute names, function names, etc).  It must be a multiple of
 * sizeof(int) (typically 4).
 *
 * NOTE that databases with different NAMEDATALEN's cannot interoperate!
 */
#define NAMEDATALEN 64


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

/* array */
#define lengthof(array) (sizeof(array) / sizeof(((array)[0])))
#define endof(array) (&(array)[lengthof(array)])

#endif /* POOL_PARSER_H */
