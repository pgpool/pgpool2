/*-------------------------------------------------------------------------
 *
 * parser.c
 *		Main entry point/driver for PostgreSQL grammar
 *
 * Note that the grammar is not allowed to perform any table access
 * (since we need to be able to do basic parsing even while inside an
 * aborted transaction).  Therefore, the data structures returned by
 * the grammar are "raw" parsetrees that still need to be analyzed by
 * analyze.c and related files.
 *
 *
 * Portions Copyright (c) 2003-2019, PgPool Global Development Group
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  src/backend/parser/parser.c
 *
 *-------------------------------------------------------------------------
 */

#include <string.h>
#include "pool_parser.h"
#include "utils/palloc.h"
#include "gramparse.h"			/* required before parser/gram.h! */
#include "gram.h"
#include "gram_minimal.h"
#include "parser.h"
#include "pg_list.h"
#include "kwlist_d.h"
#include "pg_wchar.h"
#include "makefuncs.h"
#include "utils/elog.h"
int			server_version_num = 0;
static pg_enc server_encoding = PG_SQL_ASCII;

static int
			parse_version(const char *versionString);


/*
 * raw_parser
 *		Given a query in string form, do lexical and grammatical analysis.
 *
 * Returns a list of raw (un-analyzed) parse trees.  The immediate elements
 * of the list are always RawStmt nodes.
 * Set *error to true if there's any parse error.
 */
List *
raw_parser(const char *str, int len, bool *error, bool use_minimal)
{
	core_yyscan_t yyscanner;
	base_yy_extra_type yyextra;
	int			yyresult;

	MemoryContext oldContext = CurrentMemoryContext;

	/* initialize error flag */
	*error = false;

	/* initialize the flex scanner */
	yyscanner = scanner_init(str, len, &yyextra.core_yy_extra,
							 &ScanKeywords, ScanKeywordTokens);

	/* base_yylex() only needs this much initialization */
	yyextra.have_lookahead = false;

	/* initialize the bison parser */
    if (use_minimal)
    {
        ereport(DEBUG2,
                (errmsg("invoking the minimal parser")));
       minimal_parser_init(&yyextra);
    }
    else
    {
        ereport(DEBUG2,
                (errmsg("invoking the standard parser")));
       parser_init(&yyextra);
    }
	PG_TRY();
	{
		/* Parse! */
        if (use_minimal)
           yyresult = minimal_base_yyparse(yyscanner);
        else
           yyresult = base_yyparse(yyscanner);
		/* Clean up (release memory) */
		scanner_finish(yyscanner);
	}
	PG_CATCH();
	{
		MemoryContextSwitchTo(oldContext);
		scanner_finish(yyscanner);
		yyresult = -1;
		FlushErrorState();
	}
	PG_END_TRY();

	if (yyresult)				/* error */
	{
		*error = true;
		return NIL;
	}

	return yyextra.parsetree;
}

/*
 * XXX: Currently we only process the first element of the parse tree.
 * rest of multiple statements are silently discarded.
 */
Node *
raw_parser2(List *parse_tree_list)
{
	Node	   *node = NULL;
	RawStmt    *rstmt;

	rstmt = (RawStmt *) lfirst(list_head(parse_tree_list));
	node = (Node *) rstmt->stmt;
	return node;
}

//"INSERT INTO foo VALUES(1)"
Node *
get_dummy_insert_query_node(void)
{
	InsertStmt *insert = makeNode(InsertStmt);
	SelectStmt *select = makeNode(SelectStmt);
	select->valuesLists = list_make1(makeInteger(1));
	insert->relation = makeRangeVar("pgpool", "foo", 0);
	insert->selectStmt = (Node*)select;
	return (Node *)insert;
}

List *
get_dummy_read_query_tree(void)
{
	RawStmt    *rs;
	SelectStmt *n = makeNode(SelectStmt);
	n->targetList = list_make1(makeString("pgpool: unable to parse the query"));
	rs = makeNode(RawStmt);
	rs->stmt = (Node *)n;
	rs->stmt_location = 0;
	rs->stmt_len = 0;			/* might get changed later */
	return list_make1((Node *)rs);
}

List *
get_dummy_write_query_tree(void)
{
	ColumnRef  *c1,*c2;
	RawStmt    *rs;
	DeleteStmt *n = makeNode(DeleteStmt);
	n->relation = makeRangeVar("pgpool", "foo", 0);

	c1 = makeNode(ColumnRef);
	c1->fields = list_make1(makeString("col"));

	c2 = makeNode(ColumnRef);
	c2->fields = list_make1(makeString("pgpool: unable to parse the query"));

	n->whereClause = (Node*)makeSimpleA_Expr(AEXPR_OP, "=", (Node*)c1, (Node*)c2, 0);
	/*
	 * Assign the node directly to the parsetree and exit the scanner
	 * we don't want to keep parsing for information we don't need
	 */
	rs = makeNode(RawStmt);
	rs->stmt = (Node *)n;
	rs->stmt_location = 0;
	rs->stmt_len = 0;			/* might get changed later */

	return list_make1((Node *)rs);
}
/*
 * from src/backend/commands/define.c
 * Extract an int32 value from a DefElem.
 */
int32
defGetInt32(DefElem *def)
{
	if (def->arg == NULL)
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				 errmsg("%s requires an integer value",
						def->defname)));
	switch (nodeTag(def->arg))
	{
		case T_Integer:
			return (int32) intVal(def->arg);
		default:
			ereport(ERROR,
					(errcode(ERRCODE_SYNTAX_ERROR),
					 errmsg("%s requires an integer value",
							def->defname)));
	}
	return 0;					/* keep compiler quiet */
}

/*
 * Intermediate filter between parser and core lexer (core_yylex in scan.l).
 *
 * This filter is needed because in some cases the standard SQL grammar
 * requires more than one token lookahead.  We reduce these cases to one-token
 * lookahead by replacing tokens here, in order to keep the grammar LALR(1).
 *
 * Using a filter is simpler than trying to recognize multiword tokens
 * directly in scan.l, because we'd have to allow for comments between the
 * words.  Furthermore it's not clear how to do that without re-introducing
 * scanner backtrack, which would cost more performance than this filter
 * layer does.
 *
 * The filter also provides a convenient place to translate between
 * the core_YYSTYPE and YYSTYPE representations (which are really the
 * same thing anyway, but notationally they're different).
 */
int
base_yylex(YYSTYPE *lvalp, YYLTYPE *llocp, core_yyscan_t yyscanner)
{
	base_yy_extra_type *yyextra = pg_yyget_extra(yyscanner);
	int			cur_token;
	int			next_token;
	int			cur_token_length;
	YYLTYPE		cur_yylloc;

	/* Get next token --- we might already have it */
	if (yyextra->have_lookahead)
	{
		cur_token = yyextra->lookahead_token;
		lvalp->core_yystype = yyextra->lookahead_yylval;
		*llocp = yyextra->lookahead_yylloc;
		*(yyextra->lookahead_end) = yyextra->lookahead_hold_char;
		yyextra->have_lookahead = false;
	}
	else
		cur_token = core_yylex(&(lvalp->core_yystype), llocp, yyscanner);

	/*
	 * If this token isn't one that requires lookahead, just return it.  If it
	 * does, determine the token length.  (We could get that via strlen(), but
	 * since we have such a small set of possibilities, hardwiring seems
	 * feasible and more efficient.)
	 */
	switch (cur_token)
	{
		case NOT:
			cur_token_length = 3;
			break;
		case NULLS_P:
			cur_token_length = 5;
			break;
		case WITH:
			cur_token_length = 4;
			break;
		default:
			return cur_token;
	}

	/*
	 * Identify end+1 of current token.  core_yylex() has temporarily stored a
	 * '\0' here, and will undo that when we call it again.  We need to redo
	 * it to fully revert the lookahead call for error reporting purposes.
	 */
	yyextra->lookahead_end = yyextra->core_yy_extra.scanbuf +
		*llocp + cur_token_length;
	Assert(*(yyextra->lookahead_end) == '\0');

	/*
	 * Save and restore *llocp around the call.  It might look like we could
	 * avoid this by just passing &lookahead_yylloc to core_yylex(), but that
	 * does not work because flex actually holds onto the last-passed pointer
	 * internally, and will use that for error reporting.  We need any error
	 * reports to point to the current token, not the next one.
	 */
	cur_yylloc = *llocp;

	/* Get next token, saving outputs into lookahead variables */
	next_token = core_yylex(&(yyextra->lookahead_yylval), llocp, yyscanner);
	yyextra->lookahead_token = next_token;
	yyextra->lookahead_yylloc = *llocp;

	*llocp = cur_yylloc;

	/* Now revert the un-truncation of the current token */
	yyextra->lookahead_hold_char = *(yyextra->lookahead_end);
	*(yyextra->lookahead_end) = '\0';

	yyextra->have_lookahead = true;

	/* Replace cur_token if needed, based on lookahead */
	switch (cur_token)
	{
		case NOT:
			/* Replace NOT by NOT_LA if it's followed by BETWEEN, IN, etc */
			switch (next_token)
			{
				case BETWEEN:
				case IN_P:
				case LIKE:
				case ILIKE:
				case SIMILAR:
					cur_token = NOT_LA;
					break;
			}
			break;

		case NULLS_P:
			/* Replace NULLS_P by NULLS_LA if it's followed by FIRST or LAST */
			switch (next_token)
			{
				case FIRST_P:
				case LAST_P:
					cur_token = NULLS_LA;
					break;
			}
			break;

		case WITH:
			/* Replace WITH by WITH_LA if it's followed by TIME or ORDINALITY */
			switch (next_token)
			{
				case TIME:
				case ORDINALITY:
					cur_token = WITH_LA;
					break;
			}
			break;
	}

	return cur_token;
}
int
minimal_base_yylex(YYSTYPE *lvalp, YYLTYPE *llocp, core_yyscan_t yyscanner)
{
    return base_yylex(lvalp, llocp, yyscanner);
}

static int
parse_version(const char *versionString)
{
	int			cnt;
	int			vmaj,
				vmin,
				vrev;

	cnt = sscanf(versionString, "%d.%d.%d", &vmaj, &vmin, &vrev);

	if (cnt < 2)
		return -1;

	if (cnt == 2)
		vrev = 0;

	return (100 * vmaj + vmin) * 100 + vrev;
}

void
parser_set_param(const char *name, const char *value)
{
	if (strcmp(name, "server_version") == 0)
	{
		server_version_num = parse_version(value);
	}
	else if (strcmp(name, "server_encoding") == 0)
	{
		if (strcmp(value, "UTF8") == 0)
			server_encoding = PG_UTF8;
		else
			server_encoding = PG_SQL_ASCII;
	}
	else if (strcmp(name, "standard_conforming_strings") == 0)
	{
		if (strcmp(value, "on") == 0)
			standard_conforming_strings = true;
		else
			standard_conforming_strings = false;
	}
}

int
GetDatabaseEncoding(void)
{
	return server_encoding;
}

int
pg_mblen(const char *mbstr)
{
	return pg_utf_mblen((const unsigned char *) mbstr);
}
