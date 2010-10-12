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
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "pool.h"
#include "pool_config.h"
#include "pool_session_context.h"

static POOL_SESSION_CONTEXT session_context_d;
static POOL_SESSION_CONTEXT *session_context = NULL;

static void init_prepared_statement_list(void);
static void init_portal_list(void);
static bool can_prepared_statement_destroy(POOL_QUERY_CONTEXT *qc);
static bool can_portal_destroy(POOL_QUERY_CONTEXT *qc);

/*
 * Initialize per session context
 */
void pool_init_session_context(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	session_context = &session_context_d;

	/* Get Process context */
	session_context->process_context = pool_get_process_context();
	if (!session_context->process_context)
	{
		pool_error("pool_init_session_context: cannot get process context");
		return;
	}

	/* Set connection info */
	session_context->frontend = frontend;
	session_context->backend = backend;

	/* Initialize query context */
	session_context->query_context = NULL;

	/* Initialize local session id */
	pool_incremnet_local_session_id();

	/* Initialize prepared statement list */
	init_prepared_statement_list();

	/* Initialize portal list */
	init_portal_list();

	/* Create memory context */
	session_context->memory_context = pool_memory_create(PREPARE_BLOCK_SIZE);

	/* Choose load balancing node if neccessary */
	if (pool_config->load_balance_mode)
	{
		ProcessInfo *process_info = pool_get_my_process_info();
		if (!process_info)
		{
			pool_error("pool_init_session_context: pool_get_my_process_info failed");
			return;
		}

		session_context->load_balance_node_id = 
			process_info->connection_info->load_balancing_node =
			select_load_balancing_node();

		pool_debug("selected load balancing node: %d", backend->info->load_balancing_node);
	}

	/* Unset query is in progress */
	pool_unset_query_in_progress();

	/* The command in progress has not succeeded yet */
	pool_unset_command_success();

	/* We don't have a write query in this transaction yet */
	pool_unset_writing_transaction();

	/* Error doesn't occur in this transaction yet */
	pool_unset_failed_transaction();

	/* Forget transaction isolation mode */
	pool_unset_transaction_isolation();

	/* We don't skip reading from backends */
	pool_unset_skip_reading_from_backends();

	/* Backends have not ignored messages yet */
	pool_unset_ignore_till_sync();

	/* Initialize where to send map for PREPARE statemets */
	memset(&session_context->prep_where, 0, sizeof(session_context->prep_where));
	session_context->prep_where.nelem = POOL_MAX_PREPARED_STATEMENTS;

	/* Reset flag to indicate difference in number of affected tuples
	 * in UPDATE/DELETE.
	 */
	session_context->mismatch_ntuples = false;
}

/*
 * Destroy session context.
 */
void pool_session_context_destroy(void)
{
	if (session_context)
	{
		free(session_context->pstmt_list.pstmts);
		free(session_context->portal_list.portals);
		pool_memory_delete(session_context->memory_context, 0);
	}
	/* XXX For now, just zap memory */
	memset(&session_context_d, 0, sizeof(session_context_d));
	session_context = NULL;
}

/*
 * Return session context
 */
POOL_SESSION_CONTEXT *pool_get_session_context(void)
{
	if (!session_context)
	{
		return NULL;
	}

	return session_context;
}

/*
 * Return local session id
 */
int pool_get_local_session_id(void)
{
	if (!session_context)
	{
		pool_error("pool_get_local_session_id: session context is not initialized");
		return -1;
	}

	return session_context->process_context->local_session_id;
}

/*
 * Return true if query is in progress
 */
bool pool_is_query_in_progress(void)
{
	if (!session_context)
	{
		pool_error("pool_is_query_in_progress: session context is not initialized");
		return false;
	}

	return session_context->in_progress;
}

/*
 * Set query is in progress
 */
void pool_set_query_in_progress(void)
{
	if (!session_context)
	{
		pool_error("pool_set_query_in_progress: session context is not initialized");
		return;
	}

	pool_debug("pool_set_query_in_progress: done");

	session_context->in_progress = true;
}

/*
 * Unset query is in progress
 */
void pool_unset_query_in_progress(void)
{
	if (!session_context)
	{
		pool_error("pool_unset_query_in_progress: session context is not initialized");
		return;
	}

	pool_debug("pool_unset_query_in_progress: done");

	session_context->in_progress = false;
}

/*
 * Return true if we skip reading from backends
 */
bool pool_is_skip_reading_from_backends(void)
{
	if (!session_context)
	{
		pool_error("pool_is_skip_reading_from_backends: session context is not initialized");
		return false;
	}

	return session_context->skip_reading_from_backends;
}

/*
 * Set skip_reading_from_backends
 */
void pool_set_skip_reading_from_backends(void)
{
	if (!session_context)
	{
		pool_error("pool_set_skip_reading_from_backends: session context is not initialized");
		return;
	}

	pool_debug("pool_set_skip_reading_from_backends: done");

	session_context->skip_reading_from_backends = true;
}

/*
 * Unset skip_reading_from_backends
 */
void pool_unset_skip_reading_from_backends(void)
{
	if (!session_context)
	{
		pool_error("pool_unset_skip_reading_from_backends: session context is not initialized");
		return;
	}

	pool_debug("pool_unset_skip_reading_from_backends: done");

	session_context->skip_reading_from_backends = false;
}

/*
 * Return true if we are doing extended query message
 */
bool pool_is_doing_extended_query_message(void)
{
	if (!session_context)
	{
		pool_error("pool_is_doing_extended_query_message: session context is not initialized");
		return false;
	}

	return session_context->doing_extended_query_message;
}

/*
 * Set doing_extended_query_message
 */
void pool_set_doing_extended_query_message(void)
{
	if (!session_context)
	{
		pool_error("pool_set_doing_extended_query_message: session context is not initialized");
		return;
	}

	pool_debug("pool_set_doing_extended_query_message: done");

	session_context->doing_extended_query_message = true;
}

/*
 * Unset doing_extended_query_message
 */
void pool_unset_doing_extended_query_message(void)
{
	if (!session_context)
	{
		pool_error("pool_unset_doing_extended_query_message: session context is not initialized");
		return;
	}

	pool_debug("pool_unset_doing_extended_query_message: done");

	session_context->doing_extended_query_message = false;
}

/*
 * Return true if backends ignore extended query message
 */
bool pool_is_ignore_till_sync(void)
{
	if (!session_context)
	{
		pool_error("pool_is_ignore_till_sync: session context is not initialized");
		return false;
	}

	return session_context->ignore_till_sync;
}

/*
 * Set ignore_till_sync
 */
void pool_set_ignore_till_sync(void)
{
	if (!session_context)
	{
		pool_error("pool_set_ignore_till_sync: session context is not initialized");
		return;
	}

	pool_debug("pool_set_ignore_till_sync: done");

	session_context->ignore_till_sync = true;
}

/*
 * Unset ignore_till_sync
 */
void pool_unset_ignore_till_sync(void)
{
	if (!session_context)
	{
		pool_error("pool_unset_ignore_till_sync: session context is not initialized");
		return;
	}

	pool_debug("pool_unset_ignore_till_sync: done");

	session_context->ignore_till_sync = false;
}

/*
 * Remove a portal by portal name
 */
static void pool_remove_portal_by_portal_name(const char *name)
{
	int i;
	PortalList *plist;

	if (*name == '\0')
	{
		if (session_context->unnamed_portal)
		{
			pool_memory_free(session_context->memory_context,
							 session_context->unnamed_portal);
			session_context->unnamed_portal = NULL;
		}
		return;
	}

	plist = &session_context->portal_list;

	for (i = 0; i < plist->size; i++)
	{
		if (strcmp(plist->portals[i]->name, name) == 0)
		{
			if (can_portal_destroy(plist->portals[i]->qctxt))
				pool_query_context_destroy(plist->portals[i]->qctxt);
			pool_memory_free(session_context->memory_context, plist->portals[i]);
			break;
		}
	}
		
	/* portal not found */
	if (i == plist->size)
		return;
	
	if (i != plist->size - 1)
	{
		memmove(&plist->portals[i], &plist->portals[i+1],
				sizeof(Portal *) * (plist->size - i - 1));
	}
	plist->size--;
}

/*
 * Remove portals by prepared statement name
 * prepared statement : portal = 1 : N
 */
#ifdef NOT_USED
static void pool_remove_portal_by_pstmt_name(const char *name)
{
	int i;
	PortalList *plist;

	if (*name == '\0')
	{
		if (session_context->unnamed_portal)
		{
			pool_memory_free(session_context->memory_context,
							 session_context->unnamed_portal);
			session_context->unnamed_portal = NULL;
		}
		return;
	}

	plist = &session_context->portal_list;

	for (i = 0; i < plist->size; i++)
	{
		if (strcmp(plist->portals[i]->pstmt->name, name) == 0)
			pool_remove_portal_by_portal_name(plist->portals[i]->name);
	}
}
#endif

/*
 * Remove a prepared statement by prepared statement name
 */
void pool_remove_prepared_statement_by_pstmt_name(const char *name)
{
	int i;
	PreparedStatementList *pslist;
	bool in_progress;

	in_progress = pool_is_query_in_progress();

	if (!session_context)
	{
		pool_error("pool_remove_prepared_statement_by_pstmt_name: session context is not initialized");
		return;
	}

	if (*name == '\0')
	{
		if (session_context->unnamed_pstmt)
		{
			pool_query_context_destroy(session_context->unnamed_pstmt->qctxt);
			pool_memory_free(session_context->memory_context,
							 session_context->unnamed_pstmt);
			session_context->unnamed_pstmt = NULL;
		}
		if (in_progress)
			pool_set_query_in_progress();
		return;
	}

	pslist = &session_context->pstmt_list;

	for (i = 0; i < pslist->size; i++)
	{
		if (strcmp(pslist->pstmts[i]->name, name) == 0)
		{
			if (can_prepared_statement_destroy(pslist->pstmts[i]->qctxt))
				pool_query_context_destroy(pslist->pstmts[i]->qctxt);
			pool_memory_free(session_context->memory_context, pslist->pstmts[i]);
			break;
		}
	}

	/* prepared statement not found */
	if (i == pslist->size)
	{
		if (in_progress)
			pool_set_query_in_progress();
		return;
	}

	if (i != pslist->size - 1)
	{
		memmove(&pslist->pstmts[i], &pslist->pstmts[i+1],
				sizeof(PreparedStatement *) * (pslist->size - i - 1));
	}
	pslist->size--;

	/*
	 * prepared statements and portals are closed separately
	 * by a frontend.
	 */
	/* pool_remove_portal_by_pstmt_name(name); */

	if (in_progress)
		pool_set_query_in_progress();
}

/*
 * Remove a pending prepared statement from prepared statement list
 */
void pool_remove_prepared_statement(void)
{
	char *name;

	if (!session_context)
	{
		pool_error("pool_remove_prepared_statement: session context is not initialized");
		return;
	}

	if (session_context->pending_pstmt)
	{
		name = session_context->pending_pstmt->name;
		pool_remove_prepared_statement_by_pstmt_name(name);
	}
	else
	{
		pool_debug("pool_remove_prepared_statement: pending prepared statement is NULL");
	}
}


/*
 * Remove a pending portal from portal list
 */
void pool_remove_portal(void)
{
	char *name;

	if (!session_context)
	{
		pool_error("pool_remove_portal: session context is not initialized");
		return;
	}

	if (session_context->pending_portal)
	{
		name = session_context->pending_portal->name;
		pool_remove_portal_by_portal_name(name);
	}
}

/*
 * Remove pending objects
 */
void pool_remove_pending_objects(void)
{
	PreparedStatement *ps;
	Portal *p;

	ps = session_context->pending_pstmt;

	if (ps && ps->name)
		pool_memory_free(session_context->memory_context, ps->name);

	if (ps && ps->qctxt)
		pool_query_context_destroy(ps->qctxt);

	if (ps)
		pool_memory_free(session_context->memory_context, ps);

	p = session_context->pending_portal;

	if (p && p->name)
		pool_memory_free(session_context->memory_context, p->name);

	if (p && p->pstmt)
		pool_memory_free(session_context->memory_context, p->pstmt);

	if (p)
		pool_memory_free(session_context->memory_context, p);

	session_context->pending_pstmt = NULL;
	session_context->pending_portal = NULL;
}

/*
 * Clear prepared statement list and portal list
 */
void pool_clear_prepared_statement_list(void)
{
	PreparedStatementList *pslist;

	if (!session_context)
	{
		pool_error("pool_clear_prepared_statement_list: session context is not initialized");
		return;
	}

	pslist = &session_context->pstmt_list;

	while (pslist->size > 0)
	{
		pool_remove_prepared_statement_by_pstmt_name(pslist->pstmts[0]->name);
	}
}

/*
 * Create a prepared statement
 * len: the length of parse message which is not network byte order
 * contents: the contents of parse message
 */
PreparedStatement *pool_create_prepared_statement(const char *name,
												  int num_tsparams,
												  int len, char *contents,
												  POOL_QUERY_CONTEXT *qc)
{
	PreparedStatement *ps;

	if (!session_context)
	{
		pool_error("pool_create_prepared_statement: session context is not initialized");
		return NULL;
	}

	ps = pool_memory_alloc(session_context->memory_context,
						   sizeof(PreparedStatement));
	ps->name = pool_memory_strdup(session_context->memory_context, name);
	ps->num_tsparams = num_tsparams;
	ps->parse_len = len;
	ps->parse_contents = pool_memory_alloc(session_context->memory_context, len);
	memcpy(ps->parse_contents, contents, len);

#ifdef NOT_USED
	/* 
	 * duplicate query_context because session_context->query_context is 
	 * freed by pool_query_context_destroy()
	 */
	q = malloc(sizeof(POOL_QUERY_CONTEXT));
	if (q == NULL)
	{
		pool_error("pool_create_prepared_statement: malloc failed: %s", strerror(errno));
		exit(1);
	}
	ps->qctxt = memcpy(q, qc, sizeof(POOL_QUERY_CONTEXT));
#endif
	ps->qctxt = qc;

	return ps;
}

/* 
 * Create a portal
 */
Portal *pool_create_portal(const char *name, int num_tsparams, PreparedStatement *pstmt)
{
	Portal *portal;

	if (!session_context)
	{
		pool_error("pool_create_portal: session context is not initialized");
		return NULL;
	}

	portal = pool_memory_alloc(session_context->memory_context, sizeof(Portal));
	portal->name = pool_memory_strdup(session_context->memory_context, name);
	portal->num_tsparams = num_tsparams;
	portal->pstmt = pstmt;
	portal->qctxt = pstmt->qctxt;

	return portal;
}

/*
 * Add a prepared statement to prepared statement list
 */
void pool_add_prepared_statement(void)
{
	PreparedStatement *ps;
	PreparedStatementList *pslist;

	if (!session_context)
	{
		pool_error("pool_add_prepared_statement: session context is not initialized");
		return;
	}

	if (!session_context->pending_pstmt)
	{
		pool_debug("pool_add_prepared_statement: pending prepared statement is NULL");
		return;
	}

	ps = pool_get_prepared_statement_by_pstmt_name(session_context->pending_pstmt->name);
	pslist = &session_context->pstmt_list;

	if (ps)
	{
		pool_remove_prepared_statement_by_pstmt_name(ps->name);
		if (*session_context->pending_pstmt->name == '\0')
		{
			session_context->unnamed_pstmt = session_context->pending_pstmt;
			session_context->query_context = session_context->pending_pstmt->qctxt;
		}
		else
		{
			pool_error("pool_add_prepared_statement: prepared statement \"%s\" already exists",
					   session_context->pending_pstmt->name);
		}
	}
	else
	{
		if (*session_context->pending_pstmt->name == '\0')
		{
			session_context->unnamed_pstmt = session_context->pending_pstmt;
		}
		else
		{
			if (pslist->size == pslist->capacity)
			{
				pslist->capacity *= 2;
				pslist->pstmts = realloc(pslist->pstmts, sizeof(PreparedStatement *) * pslist->capacity);
				if (pslist->pstmts == NULL)
				{
					pool_error("pool_add_prepared_statement: realloc failed: %s", strerror(errno));
					exit(1);
				}
			}
			pslist->pstmts[pslist->size++] = session_context->pending_pstmt;
		}
	}
}

/*
 * Add a portal to portal list
 */
void pool_add_portal(void)
{
	Portal *p;
	PortalList *plist;

	if (!session_context)
	{
		pool_error("pool_add_portal: session context is not initialized");
		return;
	}

	if (!session_context->pending_portal)
	{
		pool_debug("pool_add_portal: pending portal is NULL");
		return;
	}

	p = pool_get_portal_by_portal_name(session_context->pending_portal->name);
	plist = &session_context->portal_list;

	if (p)
	{
		pool_remove_portal_by_portal_name(p->name);
		if (*session_context->pending_portal->name == '\0')
		{
			session_context->unnamed_portal = session_context->pending_portal;
		}
		else
		{
			pool_error("pool_add_portal: portal \"%s\" already exists",
					   session_context->pending_portal->name);
		}
	}
	else
	{
		if (*session_context->pending_portal->name == '\0')
		{
			session_context->unnamed_portal = session_context->pending_portal;
		}
		else
		{
			if (plist->size == plist->capacity)
			{
				plist->capacity *= 2;
				plist->portals = realloc(plist->portals, sizeof(Portal *) * plist->capacity);
				if (plist->portals == NULL)
				{
					pool_error("pool_add_portal: realloc failed: %s", strerror(errno));
					exit(1);
				}
			}
			plist->portals[plist->size++] = session_context->pending_portal;
		}
	}
}

/*
 * Get a prepared statement by prepared statement name
 */
PreparedStatement *pool_get_prepared_statement_by_pstmt_name(const char *name)
{
	int i;
	PreparedStatementList *pslist;

	if (!session_context)
	{
		pool_error("pool_get_prepared_statement_by_pstmt_name: session context is not initialized");
		return NULL;
	}

	if (*name == '\0')
		return session_context->unnamed_pstmt;

	pslist = &session_context->pstmt_list;

	for (i = 0; i < pslist->size; i++)
	{
		if (strcmp(pslist->pstmts[i]->name, name) == 0)
			return pslist->pstmts[i];
	}

	return NULL;
}

/*
 * Get a portal by portal name
 */
Portal *pool_get_portal_by_portal_name(const char *name)
{
	int i;
	PortalList *plist;

	if (!session_context)
	{
		pool_error("pool_get_portal_by_portal_name: session context is not initialized");
		return NULL;
	}

	if (*name == '\0')
		return session_context->unnamed_portal;

	plist = &session_context->portal_list;

	for (i = 0; i < plist->size; i++)
	{
		if (strcmp(plist->portals[i]->name, name) == 0)
			return plist->portals[i];
	}

	return NULL;
}

/*
 * We don't have a write query in this transaction yet.
 */
void pool_unset_writing_transaction(void)
{
	if (!session_context)
	{
		pool_error("pool_unset_writing_query: session context is not initialized");
		return;
	}
	session_context->writing_transaction = false;
}

/*
 * We have a write query in this transaction.
 */
void pool_set_writing_transaction(void)
{
	if (!session_context)
	{
		pool_error("pool_set_writing_query: session context is not initialized");
		return;
	}
	session_context->writing_transaction = true;
}

/*
 * Do we have a write query in this transaction?
 */
bool pool_is_writing_transaction(void)
{
	if (!session_context)
	{
		pool_error("pool_is_writing_query: session context is not initialized");
		return false;
	}
	return session_context->writing_transaction;
}

/*
 * Error doesn't occur in this transaction yet.
 */
void pool_unset_failed_transaction(void)
{
	if (!session_context)
	{
		pool_error("pool_unset_failed_query: session context is not initialized");
		return;
	}
	session_context->failed_transaction = false;
}

/*
 * Error occurred in this transaction.
 */
void pool_set_failed_transaction(void)
{
	if (!session_context)
	{
		pool_error("pool_set_failed_query: session context is not initialized");
		return;
	}
	session_context->failed_transaction = true;
}

/*
 * Did error occur in this transaction?
 */
bool pool_is_failed_transaction(void)
{
	if (!session_context)
	{
		pool_error("pool_is_failed_query: session context is not initialized");
		return false;
	}
	return session_context->failed_transaction;
}

/*
 * Forget transaction isolation mode
 */
void pool_unset_transaction_isolation(void)
{
	if (!session_context)
	{
		pool_error("pool_unset_transaction_isolation: session context is not initialized");
		return;
	}
	session_context->transaction_isolation = POOL_UNKNOWN;
}

/*
 * Set transaction isolation mode
 */
void pool_set_transaction_isolation(POOL_TRANSACTION_ISOLATION isolation_level)
{
	if (!session_context)
	{
		pool_error("pool_set_transaction_isolation: session context is not initialized");
		return;
	}
	session_context->transaction_isolation = isolation_level;
}

/*
 * Get or return cached transaction isolation mode
 */
POOL_TRANSACTION_ISOLATION pool_get_transaction_isolation(void)
{
	POOL_STATUS status;
	POOL_SELECT_RESULT *res;
	POOL_TRANSACTION_ISOLATION ret;

	if (!session_context)
	{
		pool_error("pool_get_transaction_isolation: session context is not initialized");
		return POOL_UNKNOWN;
	}

	/* It seems cached result is usable. Return it. */
	if (session_context->transaction_isolation != POOL_UNKNOWN)
		return session_context->transaction_isolation;

	/* No cached data is available. Ask backend. */
	status = do_query(MASTER(session_context->backend),
					  "SELECT current_setting('transaction_isolation')", &res, MAJOR(session_context->backend));

	if (res->numrows <= 0)
	{
		pool_error("pool_get_transaction_isolation: do_query returns no rows");
		free_select_result(res);
		return POOL_UNKNOWN;
	}
	if (res->data[0] == NULL)
	{
		pool_error("pool_get_transaction_isolation: do_query returns no data");
		free_select_result(res);
		return POOL_UNKNOWN;
	}
	if (res->nullflags[0] == -1)
	{
		pool_error("pool_get_transaction_isolation: do_query returns NULL");
		free_select_result(res);
		return POOL_UNKNOWN;
	}

	if (!strcmp(res->data[0], "read committed"))
		ret = POOL_READ_COMMITTED;
	else if (!strcmp(res->data[0], "serializable"))
		ret = POOL_SERIALIZABLE;
	else
	{
		pool_error("pool_get_transaction_isolation: unknown transaction isolation level:%s",
				   res->data[0]);
		ret = POOL_UNKNOWN;
	}   

	free_select_result(res);

	if (ret != POOL_UNKNOWN)
		session_context->transaction_isolation = ret;

	return ret;
}

/*
 * The command in progress has not succeeded yet.
 */
void pool_unset_command_success(void)
{
	if (!session_context)
	{
		pool_error("pool_unset_command_success: session context is not initialized");
		return;
	}
	session_context->command_success = false;
}

/*
 * The command in progress has succeeded.
 */
void pool_set_command_success(void)
{
	if (!session_context)
	{
		pool_error("pool_set_command_success: session context is not initialized");
		return;
	}
	session_context->command_success = true;
}

/*
 * Has the command in progress succeeded?
 */
bool pool_is_command_success(void)
{
	if (!session_context)
	{
		pool_error("pool_is_command_success: session context is not initialized");
		return false;
	}
	return session_context->command_success;
}

/*
 * Copy send map
 */
void pool_copy_prep_where(bool *src, bool *dest)
{
	memcpy(dest, src, sizeof(bool)*MAX_NUM_BACKENDS);
}

/*
 * Add to send map a PREPARED statement
 */
void pool_add_prep_where(char *name, bool *map)
{
	int i;

	if (!session_context)
	{
		pool_error("pool_add_prep_where: session context is not initialized");
		return;
	}

	for (i=0;i<POOL_MAX_PREPARED_STATEMENTS;i++)
	{
		if (*session_context->prep_where.name[i] == '\0')
		{
			strncpy(session_context->prep_where.name[i], name, POOL_MAX_PREPARED_NAME);
			pool_copy_prep_where(map, session_context->prep_where.where_to_send[i]);
			return;
		}
	}
	pool_error("pool_add_prep_where: no empty slot found");
}

/*
 * Search send map by PREPARED statement name
 */
bool *pool_get_prep_where(char *name)
{
	int i;

	if (!session_context)
	{
		pool_error("pool_get_prep_where: session context is not initialized");
		return NULL;
	}

	for (i=0;i<POOL_MAX_PREPARED_STATEMENTS;i++)
	{
		if (!strcmp(session_context->prep_where.name[i], name))
		{
			return session_context->prep_where.where_to_send[i];
		}
	}
	return NULL;
}

/*
 * Remove PREPARED statement by name
 */
void pool_delete_prep_where(char *name)
{
	int i;

	if (!session_context)
	{
		pool_error("pool_delete_prep_where: session context is not initialized");
		return;
	}

	for (i=0;i<POOL_MAX_PREPARED_STATEMENTS;i++)
	{
		if (!strcmp(session_context->prep_where.name[i], name))
		{
			memset(&session_context->prep_where.where_to_send[i], 0, sizeof(bool)*MAX_NUM_BACKENDS);
			*session_context->prep_where.name[i] = '\0';
			return;
		}
	}
}

/*
 * Initialize prepared statement list
 */
static void init_prepared_statement_list(void)
{
	PreparedStatementList *pslist;

	pslist = &session_context->pstmt_list;
	pslist->size = 0;
	pslist->capacity = INIT_LIST_SIZE;
	pslist->pstmts = malloc(sizeof(PreparedStatement *) * INIT_LIST_SIZE);
	if (pslist->pstmts == NULL)
	{
		pool_error("init_prepared_statement_list: malloc failed: %s", strerror(errno));
		exit(1);
	}
}

/*
 * Initialize portal list
 */
static void init_portal_list(void)
{
	PortalList *plist;

	plist = &session_context->portal_list;
	plist->size = 0;
	plist->capacity = INIT_LIST_SIZE;
	plist->portals = malloc(sizeof(Portal *) * INIT_LIST_SIZE);
	if (plist->portals == NULL)
	{
		pool_error("init_portal_list: malloc failed: %s", strerror(errno));
		exit(1);
	}
}

static bool can_prepared_statement_destroy(POOL_QUERY_CONTEXT *qc)
{
	int i;
	PortalList *plist;

	plist = &session_context->portal_list;

	for (i = 0; i < plist->size; i++)
	{
		if (plist->portals[i]->qctxt == qc)
		{
			pool_debug("can_prepared_statement_destroy: query context is still used.");
			return false;
		}
	}

	return true;
}

static bool can_portal_destroy(POOL_QUERY_CONTEXT *qc)
{
	int i;
	PreparedStatementList *pslist;

	pslist = &session_context->pstmt_list;

	for (i = 0; i < pslist->size; i++)
	{
		if (pslist->pstmts[i]->qctxt == qc)
		{
			pool_debug("can_portal_destroy: query context is still used.");
			return false;
		}
	}

	return true;
}
