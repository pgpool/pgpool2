/*-------------------------------------------------------------------------
 *
 * keywords.h
 *	  lexical token lookup for reserved words in postgres SQL
 *
 *
 * Portions Copyright (c) 1996-2005, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/parser/keywords.h,v 1.20 2004/12/31 22:03:38 pgsql Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef KEYWORDS_H
#define KEYWORDS_H

/* Keyword categories --- should match lists in gram.y */
#define UNRESERVED_KEYWORD		0
#define COL_NAME_KEYWORD		1
#define TYPE_FUNC_NAME_KEYWORD	2
#define RESERVED_KEYWORD		3


typedef struct ScanKeyword
{
	const char *name;			/* in lower case */
	short		value;			/* grammar's token code */
	short		category;		/* see codes above */
} ScanKeyword;

extern const ScanKeyword *ScanKeywordLookup(const char *text);

#endif   /* KEYWORDS_H */
