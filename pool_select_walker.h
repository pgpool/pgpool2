/* -*-pgsql-c-*- */
/*
 *
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2010	PgPool Global Development Group
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
 * pool_select_walker.h.: Walker functions for SELECT
 *
 */

#ifndef POOL_SELECT_WALKER_H
#define POOL_SELECT_WALKER_H

#include "pool.h"
#include "parser/nodes.h"

extern bool pool_has_function_call(Node *node);
extern bool pool_has_system_catalog(Node *node);
extern bool pool_has_temp_table(Node *node);
extern bool pool_has_pgpool_regclass(void);
extern bool pool_has_insertinto_or_locking_clause(Node *node);
extern bool raw_expression_tree_walker(Node *node, bool (*walker) (), void *context);

#endif /* POOL_SELECT_WALKER_H */
