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

#ifndef POOL_SESSION_CONTEXT_H
#define POOL_SESSION_CONTEXT_H
#define INIT_LIST_SIZE 8

#include "pool.h"
#include "pool_process_context.h"
#include "pool_session_context.h"
#include "pool_query_context.h"
#include "query_cache/pool_memqcache.h"

/*
 * Transaction isolation mode
 */
typedef enum {
	POOL_UNKNOWN,				/* Unknown. Need to ask backend */
	POOL_READ_UNCOMMITTED,		/* Read uncommitted */
	POOL_READ_COMMITTED,		/* Read committed */
	POOL_REPEATABLE_READ,		/* Rpeatable read */
	POOL_SERIALIZABLE			/* Serializable */
} POOL_TRANSACTION_ISOLATION;

/*
 * Message content of extended query
 */
typedef struct {
	char kind;	/* one of 'P':Parse, 'B':Bind or 'Q':Query(PREPARE) */
	int len;	/* in host byte order */
	char *contents;
	int num_tsparams;
	char *name;		/* object name of prepared statement or portal */
	POOL_QUERY_CONTEXT *query_context;
	/*
	 * Following members are only used when memcache is enabled.
	 */
	bool is_cache_safe;	/* true if the query can be cached */
	int param_offset;		/* Offset from contents where actual bind
							 * parameters are stored.
							 * This is meaningful only when is_cache_safe is true.
							 */
} POOL_SENT_MESSAGE;

/*
 * List of POOL_SENT_MESSAGE
 */
typedef struct {
	int capacity;	/* capacity of list */
	int size;		/* number of elements */
	POOL_SENT_MESSAGE **sent_messages;
} POOL_SENT_MESSAGE_LIST;

/*
 * Per session context:
 */
typedef struct {
	POOL_PROCESS_CONTEXT *process_context;		/* belonging process */
	POOL_CONNECTION *frontend;	/* connection to frontend */
	POOL_CONNECTION_POOL *backend;		/* connection to backends */

	/* If true, we are waiting for backend response.  For SELECT this
	 * flags should be kept until all responses are returned from
	 * backend. i.e. until "Read for Query" packet.
	 */
	bool in_progress;

	/* If true, we are doing extended query message */
	bool doing_extended_query_message;

	/* If true, the command in progress has finished successfully. */
	bool command_success;

	/* If true, write query has been appeared in this transaction */
	bool writing_transaction;

	/* If true, error occurred in this transaction */
	bool failed_transaction;

	/* If true, we skip reading from backends */
	bool skip_reading_from_backends;

	/* ignore any command until Sync message */
	bool ignore_till_sync;

	/*
	 * Transaction isolation mode.
	 */
	POOL_TRANSACTION_ISOLATION transaction_isolation;

	/*
	 * Associated query context, only used for non-extended
	 * protocol. In extended protocol, the query context resides in
	 * "PreparedStatementList *pstmt_list" (see below).
	 */
	POOL_QUERY_CONTEXT *query_context;
#ifdef NOT_USED
	/* where to send map for PREPARE/EXECUTE/DEALLOCATE */
	POOL_PREPARED_SEND_MAP prep_where;
#endif /* NOT_USED */
	MemoryContext memory_context;	/* memory context for session */

	/* message which doesn't receive complete message */
	POOL_SENT_MESSAGE *uncompleted_message;

	POOL_SENT_MESSAGE_LIST message_list;

	int load_balance_node_id;	/* selected load balance node id */

	/*
	 * If true, UPDATE/DELETE caused difference in number of affected
	 * tuples in backends.
	*/
	bool mismatch_ntuples;

	/*
	 * If mismatch_ntuples true, this array holds the number of
	 * affected tuples of each node.
	 * -1 for down nodes.
	 */
	int ntuples[MAX_NUM_BACKENDS];

	/*
	 * If true, we are executing reset query list.
	 */
	bool reset_context;

	/*
	 * Query cache management area
	 */
	POOL_QUERY_CACHE_ARRAY *query_cache_array;	/* pending SELECT results */
	long long int num_selects;	/* number of successful SELECTs in this transaction */
} POOL_SESSION_CONTEXT;

extern void pool_init_session_context(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
extern void pool_session_context_destroy(void);
extern POOL_SESSION_CONTEXT *pool_get_session_context(bool noerror);
extern int pool_get_local_session_id(void);
extern bool pool_is_query_in_progress(void);
extern void pool_set_query_in_progress(void);
extern void pool_unset_query_in_progress(void);
extern bool pool_is_skip_reading_from_backends(void);
extern void pool_set_skip_reading_from_backends(void);
extern void pool_unset_skip_reading_from_backends(void);
extern bool pool_is_doing_extended_query_message(void);
extern void pool_set_doing_extended_query_message(void);
extern void pool_unset_doing_extended_query_message(void);
extern bool pool_is_ignore_till_sync(void);
extern void pool_set_ignore_till_sync(void);
extern void pool_unset_ignore_till_sync(void);
extern POOL_SENT_MESSAGE *pool_create_sent_message(char kind, int len, char *contents,
												   int num_tsparams, const char *name,
												   POOL_QUERY_CONTEXT *query_context);
extern void pool_add_sent_message(POOL_SENT_MESSAGE *message);
extern bool pool_remove_sent_message(char kind, const char *name);
extern void pool_remove_sent_messages(char kind);
extern void pool_clear_sent_message_list(void);
extern void pool_sent_message_destroy(POOL_SENT_MESSAGE *message);
extern POOL_SENT_MESSAGE *pool_get_sent_message(char kind, const char *name);
extern void pool_unset_writing_transaction(void);
extern void pool_set_writing_transaction(void);
extern bool pool_is_writing_transaction(void);
extern void pool_unset_failed_transaction(void);
extern void pool_set_failed_transaction(void);
extern bool pool_is_failed_transaction(void);
extern void pool_unset_transaction_isolation(void);
extern void pool_set_transaction_isolation(POOL_TRANSACTION_ISOLATION isolation_level);
extern POOL_TRANSACTION_ISOLATION pool_get_transaction_isolation(void);
extern void pool_unset_command_success(void);
extern void pool_set_command_success(void);
extern bool pool_is_command_success(void);
extern void pool_copy_prep_where(bool *src, bool *dest);
extern bool can_query_context_destroy(POOL_QUERY_CONTEXT *qc);

#ifdef NOT_USED
extern void pool_add_prep_where(char *name, bool *map);
extern bool *pool_get_prep_where(char *name);
extern void pool_delete_prep_where(char *name);
#endif /* NOT_USED */
#endif /* POOL_SESSION_CONTEXT_H */
