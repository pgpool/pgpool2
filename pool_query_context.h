/* -*-pgsql-c-*- */
/*
 *
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2011	PgPool Global Development Group
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
#include "parser/parsenodes.h"
#include "parser/pool_memory.h"

typedef enum {
	POOL_UNPARSED,
	POOL_PARSE_COMPLETE,
	POOL_BIND_COMPLETE,
	POOL_EXECUTE_COMPLETE
} POOL_QUERY_STATE;

/*
 * Query context:
 * Manages per query context
 */
typedef struct {
	char *original_query;		/* original query string */
	char *rewritten_query;		/* rewritten query string if any */
	int original_length;	   	/* original query length which contains '\0' */
	int rewritten_length;	   	/* rewritten query length which contains '\0' if any */
	Node *parse_tree;			/* raw parser output if any */
	Node *rewritten_parse_tree;	/* rewritten raw parser output if any */
	bool where_to_send[MAX_NUM_BACKENDS];	/* DB node map to send query */
	int  virtual_master_node_id;	   		/* the 1st DB node to send query */
	POOL_MEMORY_POOL *memory_context;		/* memory context for query */
	POOL_QUERY_STATE query_state[MAX_NUM_BACKENDS];	/* for extended query protocol */
	bool is_multi_statement;	/* true if multi statement query */
} POOL_QUERY_CONTEXT;

extern POOL_QUERY_CONTEXT *pool_init_query_context(void);
extern void pool_query_context_destroy(POOL_QUERY_CONTEXT *query_context);
extern void pool_start_query(POOL_QUERY_CONTEXT *query_context, char *query, int len, Node *node);
extern void pool_set_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id);
extern void pool_unset_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id);
extern bool pool_is_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id);
extern void pool_set_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id);
extern void pool_unset_node_to_be_sent(POOL_QUERY_CONTEXT *query_context, int node_id);
extern void pool_clear_node_to_be_sent(POOL_QUERY_CONTEXT *query_context);
extern void pool_setall_node_to_be_sent(POOL_QUERY_CONTEXT *query_context);
extern bool pool_multi_node_to_be_sent(POOL_QUERY_CONTEXT *query_context);
extern void pool_where_to_send(POOL_QUERY_CONTEXT *query_context, char *query, Node *node);
extern POOL_STATUS pool_send_and_wait(POOL_QUERY_CONTEXT *query_context, int send_type, int node_id);
extern POOL_STATUS pool_extended_send_and_wait(POOL_QUERY_CONTEXT *query_context, char *kind, int len, char *contents, int send_type, int node_id);
extern Node *pool_get_parse_tree(void);
extern char *pool_get_query_string(void);
extern bool is_set_transaction_serializable(Node *node, char *query);
extern bool is_start_transaction_query(Node *node);
extern bool is_read_write(TransactionStmt *node);
extern bool is_serializable(TransactionStmt *node);
extern bool pool_need_to_treat_as_if_default_transaction(POOL_QUERY_CONTEXT *query_context);
extern bool is_savepoint_query(Node *node);
extern bool is_2pc_transaction_query(Node *node);
extern void pool_set_query_state(POOL_QUERY_CONTEXT *query_context, POOL_QUERY_STATE state);
extern int statecmp(POOL_QUERY_STATE s1, POOL_QUERY_STATE s2);

#endif /* POOL_QUERY_CONTEXT_H */
