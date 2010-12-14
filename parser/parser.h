/*-------------------------------------------------------------------------
 *
 * parser.h
 *		Definitions for the "raw" parser (flex and bison phases only)
 *
 * This is the external API for the raw lexing/parsing functions.
 *
 * Portions Copyright (c) 2003-2008, PgPool Global Development Group
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/parser/parser.h,v 1.19 2004/12/31 22:03:38 pgsql Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSER_H
#define PARSER_H

#include "pg_list.h"
#include "parsenodes.h"


typedef enum
{
	BACKSLASH_QUOTE_OFF,
	BACKSLASH_QUOTE_ON,
	BACKSLASH_QUOTE_SAFE_ENCODING
} BackslashQuoteType;

/* GUC variables in scan.l (every one of these is a bad idea :-() */
extern int	backslash_quote;
extern bool escape_string_warning;
extern PGDLLIMPORT bool standard_conforming_strings;


/* Primary entry point for the raw parsing functions */
extern List *raw_parser(const char *str);
extern void free_parser(void);

/* Utility functions exported by gram.y (perhaps these should be elsewhere) */
extern List *SystemFuncName(char *name);
extern TypeName *SystemTypeName(char *name);

extern void parser_set_param(const char *name, const char *value);

#endif   /* PARSER_H */
