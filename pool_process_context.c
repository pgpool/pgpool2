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
#include "pool_process_context.h"
#include "pool_config.h"		/* remove me afterwards */

static POOL_PROCESS_CONTEXT process_context_d;
static POOL_PROCESS_CONTEXT *process_context;

/*
 * Initialize per process context
 */
void pool_init_process_context(void)
{
	process_context = &process_context_d;

	if (!process_info)
	{
		pool_error("pool_init_process_context: process_info is not set");
		child_exit(1);
	}
	process_context->process_info = process_info;

	if (!pool_config->backend_desc)
	{
		pool_error("pool_init_process_context: backend_desc is not set");
		child_exit(1);
	}
	process_context->backend_desc = pool_config->backend_desc;

	process_context->local_session_id = 0;		/* initialize local session counter */
}

/*
 * Return process context
 */
POOL_PROCESS_CONTEXT *pool_get_process_context(void)
{
	if (!process_context)
	{
		pool_error("pool_get_process_context: process context is not initialized");
	}
	return process_context;
}

/*
 * Return my process info
 */
ProcessInfo *pool_get_my_process_info(void)
{
	if (!process_context)
	{
		pool_error("pool_get_my_process_info: process context is not initialized");
		return NULL;
	}

	return &process_context->process_info[process_context->proc_id];
}

/*
 * Increment local session id
 */
void pool_incremnet_local_session_id(void)
{
	POOL_PROCESS_CONTEXT *p = pool_get_process_context();

	if (!p)
	{
		pool_error("pool_incremnet_local_session_id: cannot get process context");
		return;
	}
	p->local_session_id++;
}
