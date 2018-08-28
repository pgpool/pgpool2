/*-------------------------------------------------------------------------
 *
 * parser.h
 *		Definitions for the "raw" parser (flex and bison phases only)
 *
 * This is the external API for the raw lexing/parsing functions.
 *
 * Portions Copyright (c) 2003-2018, PgPool Global Development Group
 * Portions Copyright (c) 1996-2018, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/parser/parser.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSER_H
#define PARSER_H

#include "parsenodes.h"


typedef enum
{
	BACKSLASH_QUOTE_OFF,
	BACKSLASH_QUOTE_ON,
	BACKSLASH_QUOTE_SAFE_ENCODING
}			BackslashQuoteType;

/* GUC variables in scan.l (every one of these is a bad idea :-() */
extern int	backslash_quote;
extern bool escape_string_warning;
extern PGDLLIMPORT bool standard_conforming_strings;

/* Primary entry point for the raw parsing functions */
extern List *raw_parser(const char *str, bool *error);
extern Node *raw_parser2(List *parse_tree_list);

/* from src/backend/commands/define.c */
extern int32 defGetInt32(DefElem *def);

/* Utility functions exported by gram.y (perhaps these should be elsewhere) */
extern List *SystemFuncName(char *name);
extern TypeName *SystemTypeName(char *name);

extern void parser_set_param(const char *name, const char *value);

extern Node *makeTypeCast(Node *arg, TypeName *typename, int location);
extern Node *makeStringConstCast(char *str, int location, TypeName *typename);
extern Node *makeIntConst(int val, int location);

#endif							/* PARSER_H */
