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

#include "pool.h"
#include "pool_config.h"
#include "pool_session_context.h"

static POOL_SESSION_CONTEXT session_context_d;
static POOL_SESSION_CONTEXT *session_context;

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

	/* Initialize transaction state to idle */
	session_context->transaction_state = 'I';

	/* Choose load balancing node if neccessary */
	if (pool_config->load_balance_mode)
	{
		ProcessInfo *process_info = pool_get_my_process_info();
		if (!process_info)
		{
			pool_error("pool_init_session_context: pool_get_my_process_info failed");
			return;
		}

		process_info[backend->pool_index].connection_info->load_balancing_node = select_load_balancing_node();

		pool_debug("selected load balancing node: %d", backend->info->load_balancing_node);
	}
}

/*
 * Return session context
 */
POOL_SESSION_CONTEXT *pool_get_session_context(void)
{
	if (!session_context)
	{
		pool_error("pool_get_session_context: session context is not initialized");
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
