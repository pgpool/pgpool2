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
static POOL_SESSION_CONTEXT *session_context;

/*
 * Initialize prepared statement list
 */
static void init_prepared_statement_list(void)
{
	PreparedStatementList *pslist = NULL;

	session_context->pstmt_list = malloc(sizeof(PreparedStatementList));
	pslist = session_context->pstmt_list;
	pslist->pstmts = malloc(sizeof(PreparedStatement *) * INIT_LIST_SIZE);
	if (pslist == NULL || pslist->pstmts == NULL)
	{
		pool_error("init_prepared_statement_list: malloc failed: %s", strerror(errno));
		exit(1);
	}
	pslist->size = 0;
	pslist->capacity = INIT_LIST_SIZE;
}

/*
 * Initialize portal list
 */
static void init_portal_list(void)
{
	PortalList *plist;

	session_context->portal_list = malloc(sizeof(PortalList));
	plist = session_context->portal_list;
	plist->portals = malloc(sizeof(Portal *) * INIT_LIST_SIZE);
	if (plist == NULL || plist->portals == NULL)
	{
		pool_error("init_portal_list: malloc failed: %s", strerror(errno));
		exit(1);
	}
	plist->size = 0;
	plist->capacity = INIT_LIST_SIZE;
}

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

	/* We don't have a write query in this transaction yet. */
	pool_unset_writing_transaction();
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

	session_context->in_progress = false;
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

	plist = session_context->portal_list;

	for (i = 0; i < plist->size; i++)
	{
		if (strcmp(plist->portals[i]->name, name) == 0)
		{
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

	plist = session_context->portal_list;

	for (i = 0; i < plist->size; i++)
	{
		if (strcmp(plist->portals[i]->pstmt->name, name) == 0)
			pool_remove_portal_by_portal_name(plist->portals[i]->name);
	}
}

/*
 * Remove a prepared statement by prepared statement name
 */
void pool_remove_prepared_statement_by_pstmt_name(const char *name)
{
	int i;
	PreparedStatementList *pslist;

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
		return;
	}

	pslist = session_context->pstmt_list;

	for (i = 0; i < pslist->size; i++)
	{
		if (strcmp(pslist->pstmts[i]->name, name) == 0)
		{
			pool_query_context_destroy(pslist->pstmts[i]->qctxt);
			pool_memory_free(session_context->memory_context, pslist->pstmts[i]);
			break;
		}
	}

	/* prepared statement not found */
	if (i == pslist->size)
		return;
	
	if (i != pslist->size - 1)
	{
		memmove(&pslist->pstmts[i], &pslist->pstmts[i+1],
				sizeof(PreparedStatement *) * (pslist->size - i - 1));
	}
	pslist->size--;

	pool_remove_portal_by_pstmt_name(name);
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
		pool_error("pool_remove_prepared_statement: pending prepared statement is NULL");
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
	session_context->pending_function = NULL;
}

/*
 * Clear prepared statement list and portal list
 */
void pool_clear_prepared_statement_list(void)
{
	int i;
	PreparedStatementList *pslist;

	if (!session_context)
	{
		pool_error("pool_clear_prepared_statement_list: session context is not initialized");
		return;
	}

	pslist = session_context->pstmt_list;

	for (i = 0; i < pslist->size; i++)
	{
		pool_remove_prepared_statement_by_pstmt_name(pslist->pstmts[i]->name);
	}
}

/* 
 * Create a prepared statement
 */
void *pool_create_prepared_statement(const char *name, int num_tsparams, POOL_QUERY_CONTEXT *qc)
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
	ps->qctxt = qc;

	return ps;
}

/* 
 * Create a portal
 */
void *pool_create_portal(const char *name, int num_tsparams, PreparedStatement *pstmt)
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
		pool_error("pool_add_prepared_statement: pending prepared statement is NULL");
		return;
	}

	ps = pool_get_prepared_statement_by_pstmt_name(session_context->pending_pstmt->name);
	pslist = session_context->pstmt_list;

	if (ps)
	{
		pool_remove_prepared_statement_by_pstmt_name(ps->name);
		if (*session_context->pending_pstmt->name == '\0')
		{
			session_context->unnamed_pstmt = session_context->pending_pstmt;
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
		pool_error("pool_add_portal: pending portal is NULL");
		return;
	}

	p = pool_get_portal_by_portal_name(session_context->pending_portal->name);
	plist = session_context->portal_list;

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

	pslist = session_context->pstmt_list;

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

	plist = session_context->portal_list;

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
	session_context->writing_trasnction = false;
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
	session_context->writing_trasnction = true;
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
	return session_context->writing_trasnction;
}
