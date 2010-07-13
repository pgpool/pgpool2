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

#ifndef POOL_SESSION_CONTEXT_H
#define POOL_SESSION_CONTEXT_H
#define INIT_LIST_SIZE 8

#include "pool.h"
#include "pool_process_context.h"
#include "pool_session_context.h"
#include "pool_query_context.h"
#include "parser/pool_memory.h"

/*
 * Prepared Statement:
 */
typedef struct {
	char *name;		/* prepared statement name */
	int num_tsparams;
	POOL_QUERY_CONTEXT *qctxt;
} PreparedStatement;

/*
 * Prepared statement list:
 */
typedef struct {
	int capacity;	/* capacity of list */
	int size;		/* number of PreparedStatement */
	PreparedStatement **pstmts;	/* prepared statement list */
} PreparedStatementList;

/* 
 * Portal:
 */
typedef struct {
	char *name;		/* portal name */
	int num_tsparams;
	PreparedStatement *pstmt;
} Portal;

/*
 * Portal list:
 */
typedef struct {
	int capacity;	/* capacity of list */
	int size;		/* number of portal */
	Portal **portals;	/* portal list */
} PortalList;

/*
 * Per session context:
 */
typedef struct {
	POOL_PROCESS_CONTEXT *process_context;		/* belonging process */
	POOL_CONNECTION *frontend;	/* connection to frontend */
	POOL_CONNECTION_POOL *backend;		/* connection to backends */

	/* If true, we are waiting for backend response.  For SELECT this
	 * flags should be kept until all responses are returned from
	 * backend
	 */
	bool in_progress;

	/* If true write query has been appeared in this transaction */
	bool writing_transaction;

	/*
	 * Associated query context, only used for non-extended
	 * protocol. In extended protocol, the query context resides in
	 * "PreparedStatementList *pstmt_list" (see below).
	 */
	POOL_QUERY_CONTEXT *query_context;

	POOL_MEMORY_POOL *memory_context;	/* memory context for session */
	PreparedStatement *unnamed_pstmt;	/* unnamed statement */
	PreparedStatement *pending_pstmt;	/* used until receive backend response */
	Portal *unnamed_portal;	/* unnamed portal */
	Portal *pending_portal;	/* used until receive backend response */
	PreparedStatementList *pstmt_list;	/* named statement list */
	PortalList *portal_list;	/* named portal list */
	void (*pending_function)(void);	/* switched function by backend response */
	int load_balance_node_id;	/* selected load balance node id */

#ifdef NOT_USED
/* We need to override these gotchas... */
	int force_replication;
	int replication_was_enabled;		/* replication mode was enabled */
	int master_slave_was_enabled;	/* master/slave mode was enabled */
	int internal_transaction_started;		/* to issue table lock command a transaction
											   has been started internally */
	int mismatch_ntuples;	/* number of updated tuples */
	char *copy_table = NULL;  /* copy table name */
	char *copy_schema = NULL;  /* copy table name */
	char copy_delimiter; /* copy delimiter char */
	char *copy_null = NULL; /* copy null string */
	int select_in_transaction = 0; /* non 0 if select query is in transaction */
	int execute_select = 0; /* non 0 if select query is in transaction */

/* non 0 if "BEGIN" query with extended query protocol received */
	int receive_extended_begin = 0;

/*
 * Non 0 if allow to close internal transaction.  This variable was
 * introduced on 2008/4/3 not to close an internal transaction when
 * Sync message is received after receiving Parse message. This hack
 * is for PHP-PDO.
 */
	static int allow_close_transaction = 1;

	PreparedStatementList prepared_list; /* prepared statement name list */

	int is_select_pgcatalog = 0;
	int is_select_for_update = 0; /* 1 if SELECT INTO or SELECT FOR UPDATE */
	bool is_parallel_table = false;

/*
 * last query string sent to simpleQuery()
 */
	char query_string_buffer[QUERY_STRING_BUFFER_LEN];

/*
 * query string produced by nodeToString() in simpleQuery().
 * this variable only usefull when enable_query_cache is true.
 */
	char *parsed_query = NULL;


	int load_balance_node_id;	/* selected load balance node id */
	bool received_write_query;	/* have we recived a write query in this transaction? */
	bool send_ready_for_query;	/* ok to send ReadyForQuery */
	bool igore_till_sync;		/* ignore any command until Sync message */
	LOAD_BALANCE_STATUS	load_balance_status[MAX_NUM_BACKENDS];	/* to remember which DB node is selected for load balancing */
#endif

} POOL_SESSION_CONTEXT;

extern void pool_init_session_context(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
extern POOL_SESSION_CONTEXT *pool_get_session_context(void);
extern int pool_get_local_session_id(void);
extern bool pool_is_query_in_progress(void);
extern void pool_set_query_in_progress(void);
extern void pool_unset_query_in_progress(void);
extern void pool_remove_prepared_statement_by_pstmt_name(const char *name);
extern void pool_remove_prepared_statement(void);
extern void pool_remove_portal(void);
extern void pool_remove_pending_objects(void);
extern void pool_clear_prepared_statement_list(void);
extern PreparedStatement *pool_create_prepared_statement(const char *name, int num_tsparams, POOL_QUERY_CONTEXT *qc);
extern Portal *pool_create_portal(const char *name, int num_tsparams, PreparedStatement *pstmt);
extern void pool_add_prepared_statement(void);
extern void pool_add_portal(void);
extern PreparedStatement *pool_get_prepared_statement_by_pstmt_name(const char *name);
extern Portal *pool_get_portal_by_portal_name(const char *name);

extern void pool_unset_writing_transaction(void);
extern void pool_set_writing_transaction(void);
bool pool_is_writing_transaction(void);

#endif /* POOL_SESSION_CONTEXT_H */
