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
 * pool_process_context.h.: process context information
 *
 */

#ifndef POOL_QUERY_CONTEXT_H
#define POOL_QUERY_CONTEXT_H

#include "pool.h"
#include "pool_process_context.h"
#include "parser/nodes.h"

/*
 * Query context:
 * Manages per query context
 * 拡張問い合わせでは複数のクエリコンテキストが同時に走るかも知れない。
 * 内部で発生する問い合わせもあるし。
 */
typedef struct {
	char *original_query;		/* original query string */
	char *rewritten_query;		/* rewritten query string if any */
	Node *parse_tree;	/* raw parser output if any */
	Node *rewritten_parse_tree;	/* rewritten raw parser output if any */
	bool where_to_send[MAX_NUM_BACKENDS];		/* DB node map to send query */

	bool is_extended;	/* true if extended query */

	/* below are extended query only */
	bool is_named_statement;		/* true if named statement */
	bool is_named_portal;		/* true if named portal */
	/* pointer to unnamed statement or named statement */
	/* pointer to unnamed portal or named portal */
	/* query state: either 0: before parse, 1: parse done, 2: bind
	 * done, 3: describe done 4: execute done -1: in error -> or
	 * should be a bitmap? */

	
} POOL_QUERY_CONTEXT;

extern POOL_QUERY_CONTEXT *pool_init_query_context(void);
extern void pool_query_context_destroy(POOL_QUERY_CONTEXT *query_context);
extern void pool_start_query(POOL_QUERY_CONTEXT *query_context, char *query, Node *node);
extern void pool_set_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id);
extern void pool_unset_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id);
extern bool pool_is_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id);
extern void pool_set_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id);
extern void pool_unset_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id);
extern void pool_clear_node_to_be_sent(POOL_QUERY_CONTEXT *query_context);
extern void pool_setall_node_to_be_sent(POOL_QUERY_CONTEXT *query_context);
extern void pool_where_to_send(POOL_QUERY_CONTEXT *query_context, char *query, Node *node);
POOL_STATUS pool_send_and_wait(POOL_QUERY_CONTEXT *query_context, char *query, int len,
							   int send_type, int node_id);

#endif /* POOL_QUERY_CONTEXT_H */
