/*-------------------------------------------------------------------------
 *
 * keywords.c
 *	  lexical token lookup for key words in PostgreSQL
 *
 *
 * Portions Copyright (c) 2003-2009, PgPool Global Development Group
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/parser/keywords.c,v 1.212 2009/03/08 16:53:30 alvherre Exp $
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
