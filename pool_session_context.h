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

#include "pool.h"
#include "pool_process_context.h"
#include "pool_session_context.h"

/*
 * Per session context:
 */
typedef struct {
	POOL_PROCESS_CONTEXT *process_context;		/* belonging process */
	char transaction_state;		/* either 'U'(unknown), 'I' (idle), 'T'(in transaction), 'E'(error) */
	POOL_CONNECTION *frontend;	/* connection to frontend */
	POOL_CONNECTION_POOL *backend;		/* connection to backends */

	bool in_progress;	/* true if we are busy and do not accept input from frontend */
	int load_balance_node_id;	/* selected load balance node id */
	bool received_write_query;	/* have we recived a write query in this transaction? */
	bool send_ready_for_query;	/* ok to send ReadyForQuery */
	bool igore_till_sync;		/* ignore any command until Sync message */
	LOAD_BALANCE_STATUS	load_balance_status[MAX_NUM_BACKENDS];	/* to remember which DB node is selected for load balancing */
	/* unnamed statement */
	/* unnamed portal */
	/* named statement list */
	/* named portal list */
} POOL_SESSION_CONTEXT;

extern void pool_init_session_context(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend);
extern POOL_SESSION_CONTEXT *pool_get_session_context(void);
extern int pool_get_local_session_id(void);

#endif /* POOL_SESSION_CONTEXT_H */
