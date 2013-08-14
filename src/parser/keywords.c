/*-------------------------------------------------------------------------
 *
 * keywords.c
 *	  lexical token lookup for key words in PostgreSQL
 *
 *
 * Portions Copyright (c) 2003-2013, PgPool Global Development Group
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/parser/keywords.c
 *
 *-------------------------------------------------------------------------
 */

#include "parsenodes.h"
#include "gramparse.h"	/* required before parser/parse.h! */
#include "keywords.h"
#include "gram.h"

#define PG_KEYWORD(a,b,c) {a,b,c},


const ScanKeyword ScanKeywords[] = {
#include "kwlist.h"
};

/* End of ScanKeywords, for use in kwlookup.c and elsewhere */
const ScanKeyword *LastScanKeyword = endof(ScanKeywords);
const int	NumScanKeywords = lengthof(ScanKeywords);
