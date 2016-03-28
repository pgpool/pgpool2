/* -*-pgsql-c-*- */
/*
 *
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2015	PgPool Global Development Group
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
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/elog.h"
#include "pool_config.h"
#include "context/pool_session_context.h"

static POOL_SESSION_CONTEXT session_context_d;
static POOL_SESSION_CONTEXT *session_context = NULL;
static void GetTranIsolationErrorCb(void *arg);
static void init_sent_message_list(void);
static POOL_PENDING_MESSAGE *copy_pending_message(POOL_PENDING_MESSAGE *messag);
static void dump_sent_message(char *caller, POOL_SENT_MESSAGE *m);
static char *dump_sync_map(void);

/*
 * Initialize per session context
 */
void pool_init_session_context(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *backend)
{
	session_context = &session_context_d;

	/* Get Process context */
	session_context->process_context = pool_get_process_context();
	if (!session_context->process_context)
		ereport(ERROR,
				(errmsg("failed to get process context")));

	/* Set connection info */
	session_context->frontend = frontend;
	session_context->backend = backend;

	/* Initialize query context */
	session_context->query_context = NULL;

	/* Initialize local session id */
	pool_incremnet_local_session_id();

	/* Create memory context */
	/* TODO re-think about the parent for this context ??*/
	session_context->memory_context = AllocSetContextCreate(ProcessLoopContext,
									 "SessionContext",
									 ALLOCSET_SMALL_MINSIZE,
									 ALLOCSET_SMALL_INITSIZE,
									 ALLOCSET_SMALL_MAXSIZE);
									 
	/* Initialize sent message list */
	init_sent_message_list();

	/* Choose load balancing node if necessary */
	if (pool_config->load_balance_mode)
	{
		ProcessInfo *process_info = pool_get_my_process_info();
		if (!process_info)
			ereport(ERROR,
					(errmsg("failed to get process info for current process")));

		session_context->load_balance_node_id = 
			process_info->connection_info->load_balancing_node =
			select_load_balancing_node();

		ereport(DEBUG1,
			(errmsg("initializing session context"),
				 errdetail("selected load balancing node: %d", backend->info->load_balancing_node)));
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

	/* Initialize where to send map for PREPARE statements */
#ifdef NOT_USED
	memset(&session_context->prep_where, 0, sizeof(session_context->prep_where));
	session_context->prep_where.nelem = POOL_MAX_PREPARED_STATEMENTS;
#endif /* NOT_USED */
	/* Reset flag to indicate difference in number of affected tuples
	 * in UPDATE/DELETE.
	 */
	session_context->mismatch_ntuples = false;

	if (pool_config->memory_cache_enabled)
	{
		session_context->query_cache_array = pool_create_query_cache_array();
		session_context->num_selects = 0;
	}

	/* Clear sync map */
	pool_clear_sync_map();

	/* Unset pending response */
	pool_unset_pending_response();

	/* Initialize pending message list */
	pool_pending_messages_init();
}

/*
 * Destroy session context.
 */
void pool_session_context_destroy(void)
{
	if (session_context)
	{
		pool_clear_sent_message_list();
		pfree(session_context->message_list.sent_messages);
		if (pool_config->memory_cache_enabled)
		{
			pool_discard_query_cache_array(session_context->query_cache_array);
			session_context->num_selects = 0;
		}

		if (session_context->query_context)
			pool_query_context_destroy(session_context->query_context);
		MemoryContextDelete(session_context->memory_context);
	}
	/* XXX For now, just zap memory */
	memset(&session_context_d, 0, sizeof(session_context_d));
	session_context = NULL;
}

/*
 * Return session context
 */
POOL_SESSION_CONTEXT *pool_get_session_context(bool noerror)
{
	if (!session_context && !noerror)
	{
        ereport(FATAL,
            (return_code(2),
                errmsg("unable to get session context")));
	}
	return session_context;
}

/*
 * Return local session id
 */
int pool_get_local_session_id(void)
{
	return pool_get_session_context(false)->process_context->local_session_id;
}

/*
 * Return true if query is in progress
 */
bool pool_is_query_in_progress(void)
{
	return pool_get_session_context(false)->in_progress;
}

/*
 * Set query is in progress
 */
void pool_set_query_in_progress(void)
{
	ereport(DEBUG1,
		(errmsg("session context: setting query in progress. DONE")));

	pool_get_session_context(false)->in_progress = true;
}

/*
 * Unset query is in progress
 */
void pool_unset_query_in_progress(void)
{
	ereport(DEBUG1,
		(errmsg("session context: unsetting query in progress. DONE")));

	pool_get_session_context(false)->in_progress = false;
}

/*
 * Return true if we skip reading from backends
 */
bool pool_is_skip_reading_from_backends(void)
{
	return pool_get_session_context(false)->skip_reading_from_backends;
}

/*
 * Set skip_reading_from_backends
 */
void pool_set_skip_reading_from_backends(void)
{
	ereport(DEBUG1,
			(errmsg("session context: setting skip reading from backends. DONE")));


	pool_get_session_context(false)->skip_reading_from_backends = true;
}

/*
 * Unset skip_reading_from_backends
 */
void pool_unset_skip_reading_from_backends(void)
{
	ereport(DEBUG1,
			(errmsg("session context: clearing skip reading from backends. DONE")));
	
	pool_get_session_context(false)->skip_reading_from_backends = false;
}

/*
 * Return true if we are doing extended query message
 */
bool pool_is_doing_extended_query_message(void)
{
	return pool_get_session_context(false)->doing_extended_query_message;
}

/*
 * Set doing_extended_query_message
 */
void pool_set_doing_extended_query_message(void)
{
	ereport(DEBUG1,
			(errmsg("session context: setting doing extended query messaging. DONE")));
	
	pool_get_session_context(false)->doing_extended_query_message = true;
}

/*
 * Unset doing_extended_query_message
 */
void pool_unset_doing_extended_query_message(void)
{
	ereport(DEBUG1,
			(errmsg("session context: clearing doing extended query messaging. DONE")));
	
	pool_get_session_context(false)->doing_extended_query_message = false;
}

/*
 * Return true if backends ignore extended query message
 */
bool pool_is_ignore_till_sync(void)
{
	return pool_get_session_context(false)->ignore_till_sync;
}

/*
 * Set ignore_till_sync
 */
void pool_set_ignore_till_sync(void)
{
	ereport(DEBUG1,
			(errmsg("session context: setting ignore till sync. DONE")));
	
	pool_get_session_context(false)->ignore_till_sync = true;
}

/*
 * Unset ignore_till_sync
 */
void pool_unset_ignore_till_sync(void)
{
	ereport(DEBUG1,
			(errmsg("session context: clearing ignore till sync. DONE")));

	pool_get_session_context(false)->ignore_till_sync = false;
}

/*
 * Remove a sent message
 */
bool pool_remove_sent_message(char kind, const char *name)
{
	int i;
	POOL_SENT_MESSAGE_LIST *msglist;

	msglist = &pool_get_session_context(false)->message_list;

	for (i = 0; i < msglist->size; i++)
	{
		if (msglist->sent_messages[i]->kind == kind &&
			!strcmp(msglist->sent_messages[i]->name, name))
		{
			pool_sent_message_destroy(msglist->sent_messages[i]);
			break;
		}
	}

	/* sent message not found */
	if (i == msglist->size)
		return false;

	if (i != msglist->size - 1)
	{
		memmove(&msglist->sent_messages[i], &msglist->sent_messages[i+1],
				sizeof(POOL_SENT_MESSAGE *) * (msglist->size - i - 1));
	}

	msglist->size--;

	return true;
}

/*
 * Remove same kind of sent messages
 */
void pool_remove_sent_messages(char kind)
{
	int i;
	POOL_SENT_MESSAGE_LIST *msglist;

	msglist = &pool_get_session_context(false)->message_list;

	for (i = 0; i < msglist->size; i++)
	{
		if (msglist->sent_messages[i]->kind == kind)
		{
			if (pool_remove_sent_message(kind, msglist->sent_messages[i]->name))
				i--;	/* for relocation by removing */
		}
	}
}

/*
 * Destroy sent message
 */
void pool_sent_message_destroy(POOL_SENT_MESSAGE *message)
{
	bool in_progress;
	POOL_QUERY_CONTEXT *qc = NULL;

	dump_sent_message("pool_sent_message_destroy", message);

	in_progress = pool_is_query_in_progress();

	if (message)
	{
		if (message->contents)
			pfree(message->contents);
		
		if (message->name)
			pfree(message->name);

		if (message->query_context)
		{
			if (session_context->query_context != message->query_context)
				qc = session_context->query_context;

			if (can_query_context_destroy(message->query_context))
			{
				pool_query_context_destroy(message->query_context);
				/*
				 * set in_progress flag, because pool_query_context_destroy()
				 * unsets in_progress flag
				 */
				if (in_progress)
					pool_set_query_in_progress();
				/*
				 * set query_context of session_context, because
				 * pool_query_context_destroy() sets it to NULL.
				 */
				if (qc)
					session_context->query_context = qc;
			}
		}

		if (session_context->memory_context)
			pfree(message);
	}
}

/*
 * Clear sent message list
 */
void pool_clear_sent_message_list(void)
{
	POOL_SENT_MESSAGE_LIST *msglist;

	msglist = &pool_get_session_context(false)->message_list;

	while (msglist->size > 0)
	{
		pool_remove_sent_messages(msglist->sent_messages[0]->kind);
	}
}

static void dump_sent_message(char *caller, POOL_SENT_MESSAGE *m)
{
	ereport(DEBUG1,
			(errmsg("called by %s: sent message: address: %x kind: %c name: =%s=", caller, m, m->kind, m->name)));
}

/*
 * Create a sent message
 * kind: one of 'P':Parse, 'B':Bind or'Q':Query(PREPARE)
 * len: message length that is not network byte order
 * contents: message contents
 * num_tsparams: number of timestamp parameters
 * name: prepared statement name or portal name
 */
POOL_SENT_MESSAGE *pool_create_sent_message(char kind, int len, char *contents,
											int num_tsparams, const char *name,
											POOL_QUERY_CONTEXT *query_context)
{
	POOL_SENT_MESSAGE *msg;

	if (!session_context)
        ereport(ERROR,
            (errmsg("unable to create message"),
                 errdetail("cannot get the session context")));

	MemoryContext old_context = MemoryContextSwitchTo(session_context->memory_context);
	msg = palloc(sizeof(POOL_SENT_MESSAGE));
	msg->kind = kind;
	msg->len = len;
	msg->contents = palloc(len);
	memcpy(msg->contents, contents, len);
	msg->num_tsparams = num_tsparams;
	msg->name = pstrdup(name);
	msg->query_context = query_context;
	MemoryContextSwitchTo(old_context);

	return msg;
}

/*
 * Add a sent message to sent message list
 */
void pool_add_sent_message(POOL_SENT_MESSAGE *message)
{
	POOL_SENT_MESSAGE *old_msg;
	POOL_SENT_MESSAGE_LIST *msglist;

	dump_sent_message("pool_add_sent_message", message);

	if (!message)
	{
		ereport(DEBUG1,
			(errmsg("adding sent message to list"),
				 errdetail("message is null")));
		return;
	}

	old_msg = pool_get_sent_message(message->kind, message->name);

	if (old_msg == message)
	{
		/*
		 * It is likely caller tries to add the exact same message previously
		 * added. We should ignore this because pool_remove_sent_message()
		 * will free memory allocated in the message.
		 */
		ereport(DEBUG1,
				(errmsg("adding sent message to list"),
				 errdetail("adding exactly same message is prohibited")));
		return;
	}

	msglist = &session_context->message_list;

	if (old_msg)
	{
		if (message->kind == 'B')
			ereport(DEBUG1,
				(errmsg("adding sent message to list"),
					errdetail("portal \"%s\" already exists",message->name)));
		else
			ereport(DEBUG1,
				(errmsg("adding sent message to list"),
					errdetail("prepared statement \"%s\" already exists",message->name)));

		if (*message->name == '\0')
			pool_remove_sent_message(old_msg->kind, old_msg->name);
		else
			return;
	}

	if (msglist->size == msglist->capacity)
	{
		msglist->capacity *= 2;

		MemoryContext oldContext = MemoryContextSwitchTo(session_context->memory_context);

		msglist->sent_messages = repalloc(msglist->sent_messages,
										  sizeof(POOL_SENT_MESSAGE *) * msglist->capacity);

		MemoryContextSwitchTo(oldContext);
	}

	msglist->sent_messages[msglist->size++] = message;
}

/*
 * Get a sent message
 */
POOL_SENT_MESSAGE *pool_get_sent_message(char kind, const char *name)
{
	int i;
	POOL_SENT_MESSAGE_LIST *msglist;

	msglist = &pool_get_session_context(false)->message_list;

	for (i = 0; i < msglist->size; i++)
	{
		if (msglist->sent_messages[i]->kind == kind &&
			!strcmp(msglist->sent_messages[i]->name, name))
			return msglist->sent_messages[i];
	}

	return NULL;
}

/*
 * We don't have a write query in this transaction yet.
 */
void pool_unset_writing_transaction(void)
{
	ereport(DEBUG1,
			(errmsg("session context: clearing writing transaction. DONE")));

	pool_get_session_context(false)->writing_transaction = false;
}

/*
 * We have a write query in this transaction.
 */
void pool_set_writing_transaction(void)
{
	ereport(DEBUG1,
			(errmsg("session context: setting writing transaction. DONE")));
	pool_get_session_context(false)->writing_transaction = true;
}

/*
 * Do we have a write query in this transaction?
 */
bool pool_is_writing_transaction(void)
{
	return pool_get_session_context(false)->writing_transaction;
}

/*
 * Error doesn't occur in this transaction yet.
 */
void pool_unset_failed_transaction(void)
{
	ereport(DEBUG1,
			(errmsg("session context: clearing failed transaction. DONE")));
	
	pool_get_session_context(false)->failed_transaction = false;
}

/*
 * Error occurred in this transaction.
 */
void pool_set_failed_transaction(void)
{
	ereport(DEBUG1,
			(errmsg("session context: setting failed transaction. DONE")));

	pool_get_session_context(false)->failed_transaction = true;
}

/*
 * Did error occur in this transaction?
 */
bool pool_is_failed_transaction(void)
{
	return pool_get_session_context(false)->failed_transaction;
}

/*
 * Forget transaction isolation mode
 */
void pool_unset_transaction_isolation(void)
{
	ereport(DEBUG1,
			(errmsg("session context: clearing failed transaction. DONE")));
	pool_get_session_context(false)->transaction_isolation = POOL_UNKNOWN;
}

/*
 * Set transaction isolation mode
 */
void pool_set_transaction_isolation(POOL_TRANSACTION_ISOLATION isolation_level)
{
	ereport(DEBUG1,
			(errmsg("session context: setting transaction isolation. DONE")));
	pool_get_session_context(false)->transaction_isolation = isolation_level;
}

/*
 * Get or return cached transaction isolation mode
 */
POOL_TRANSACTION_ISOLATION pool_get_transaction_isolation(void)
{
	POOL_SELECT_RESULT *res;
	POOL_TRANSACTION_ISOLATION ret;
	ErrorContextCallback callback;

	if (!session_context)
	{
		ereport(WARNING,
				(errmsg("error while getting transaction isolation, session context is not initialized")));
		return POOL_UNKNOWN;
	}

	/* It seems cached result is usable. Return it. */
	if (session_context->transaction_isolation != POOL_UNKNOWN)
		return session_context->transaction_isolation;
	/*
	 * Register a error context callback to throw proper context message
	 */
	callback.callback = GetTranIsolationErrorCb;
	callback.arg = NULL;
	callback.previous = error_context_stack;
	error_context_stack = &callback;

	/* No cached data is available. Ask backend. */

	do_query(MASTER(session_context->backend),
					  "SELECT current_setting('transaction_isolation')", &res, MAJOR(session_context->backend));

	error_context_stack = callback.previous;

	if (res->numrows <= 0)
	{
		ereport(WARNING,
				(errmsg("error while getting transaction isolation, do_query returns no rows")));
		free_select_result(res);
		return POOL_UNKNOWN;
	}
	if (res->data[0] == NULL)
	{
		ereport(WARNING,
				(errmsg("error while getting transaction isolation, do_query returns no data")));

		free_select_result(res);
		return POOL_UNKNOWN;
	}
	if (res->nullflags[0] == -1)
	{
		ereport(WARNING,
				(errmsg("error while getting transaction isolation, do_query returns NULL")));
		free_select_result(res);
		return POOL_UNKNOWN;
	}

	if (!strcmp(res->data[0], "read uncommitted"))
		ret = POOL_READ_UNCOMMITTED;
	else if (!strcmp(res->data[0], "read committed"))
		ret = POOL_READ_COMMITTED;
	else if (!strcmp(res->data[0], "repeatable read"))
		ret = POOL_REPEATABLE_READ;
	else if (!strcmp(res->data[0], "serializable"))
		ret = POOL_SERIALIZABLE;
	else
	{
		ereport(WARNING,
				(errmsg("error while getting transaction isolation, unknown transaction isolation level:%s",res->data[0])));

		ret = POOL_UNKNOWN;
	}   

	free_select_result(res);

	if (ret != POOL_UNKNOWN)
		session_context->transaction_isolation = ret;

	return ret;
}

static void GetTranIsolationErrorCb(void *arg)
{
	errcontext("while getting transaction isolation");
}


/*
 * The command in progress has not succeeded yet.
 */
void pool_unset_command_success(void)
{
	ereport(DEBUG1,
			(errmsg("session context: clearing transaction isolation. DONE")));
	pool_get_session_context(false)->command_success = false;
}

/*
 * The command in progress has succeeded.
 */
void pool_set_command_success(void)
{
	ereport(DEBUG1,
			(errmsg("session context: setting command success. DONE")));

	pool_get_session_context(false)->command_success = true;
}

/*
 * Has the command in progress succeeded?
 */
bool pool_is_command_success(void)
{
	return pool_get_session_context(false)->command_success;
}

/*
 * Copy send map
 */
void pool_copy_prep_where(bool *src, bool *dest)
{
	memcpy(dest, src, sizeof(bool)*MAX_NUM_BACKENDS);
}
#ifdef NOT_USED
/*
 * Add to send map a PREPARED statement
 */
void pool_add_prep_where(char *name, bool *map)
{
	int i;

	if (!session_context)
	{
		ereport(ERROR,
				(errmsg("pool_add_prep_where: session context is not initialized")));
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
	ereport(ERROR,
			(errmsg("pool_add_prep_where: no empty slot found")));
}

/*
 * Search send map by PREPARED statement name
 */
bool *pool_get_prep_where(char *name)
{
	int i;

	if (!session_context)
		ereport(ERROR,
				(errmsg("pool_get_prep_where: session context is not initialized")));


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
		ereport(ERROR,
				(errmsg("pool_delete_prep_where: session context is not initialized")));

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
#endif /* NOT_USED */
/*
 * Initialize sent message list
 */
static void init_sent_message_list(void)
{
	POOL_SENT_MESSAGE_LIST *msglist;

	msglist = &session_context->message_list;
	msglist->size = 0;
	msglist->capacity = INIT_LIST_SIZE;

	MemoryContext oldContext = MemoryContextSwitchTo(session_context->memory_context);

	msglist->sent_messages = palloc(sizeof(POOL_SENT_MESSAGE *) * INIT_LIST_SIZE);

	MemoryContextSwitchTo(oldContext);
}

/*
 * Look for extended message list to check if given query context qc
 * is used. Returns true if it is not used.
 */
bool can_query_context_destroy(POOL_QUERY_CONTEXT *qc)
{
	int i;
	int count = 0;
	POOL_SENT_MESSAGE_LIST *msglist;

	msglist = &session_context->message_list;

	for (i = 0; i < msglist->size; i++)
	{
		if (msglist->sent_messages[i]->query_context == qc)
			count++;
	}
	if (count > 1)
	{
		ereport(DEBUG1,
			(errmsg("checking if query context can be safely destroyed"),
				 errdetail("query context %p is still used %d times. query:\"%s\"",
						   qc, count,qc->original_query)));
		return false;
	}

	return true;
}

/*
 * Set sync map
 */
void pool_set_sync_map(int node_id)
{
	if (!session_context)
		ereport(ERROR,
				(errmsg("pool_set_sync_map: session context is not initialized")));

	session_context->sync_map[node_id] = true;
}

/*
 * Check if sync map is set
 */
bool pool_is_set_sync_map(int node_id)
{
	if (!session_context)
		ereport(ERROR,
				(errmsg("pool_is_set_sync_map: session context is not initialized")));

	return session_context->sync_map[node_id];
}

/*
 * Get nth valid node id from sync map
 */
int pool_get_nth_sync_map(int nth)
{
	int i;
	int cnt = 0;

	for (i = 0; i < NUM_BACKENDS; i++)
	{
		if (!VALID_BACKEND(i))
			continue;

		if (session_context->sync_map[i])
		{
			if (cnt == nth)
				return i;	/* found nth valid node id */
		}

		cnt++;
	}

	return -1;		/* no valid node id found */
}

/*
 * Clear sync map
 */
void pool_clear_sync_map(void)
{
	if (!session_context)
		ereport(ERROR,
				(errmsg("pool_clear_sync_map: session context is not initialized")));

	memset(&session_context->sync_map, 0, sizeof(session_context->sync_map));
}

/*
 * Check whether we should 1) ignore sync map, 2) consult sync map, or 3) we
 * cannot use sync map because it's empty.
 */
bool pool_use_sync_map(void)
{
	int i;

	if (!session_context)
		return POOL_IGNORE_SYNC_MAP;

	if (STREAM && !pool_is_query_in_progress() && pool_is_doing_extended_query_message())
	{
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (pool_is_set_sync_map(i))
			{
				ereport(DEBUG1,
						(errmsg("pool_use_sync_map: we can use sync map: %s", dump_sync_map())));
				return POOL_SYNC_MAP_IS_VALID;	/* yes, we can use sync map */
			}
		}
		ereport(DEBUG1,
				(errmsg("pool_use_sync_map: we cannot use sync map because all map entries are false")));
		return POOL_SYNC_MAP_EMPTY;	/* no, we cannot use sync map */
	}

	ereport(DEBUG1,
			(errmsg("pool_use_sync_map: we cannot use sync map because STREAM: %d query in progress: %d doing extended query: %d", STREAM, pool_is_query_in_progress(), pool_is_doing_extended_query_message())));

	return POOL_IGNORE_SYNC_MAP;
}

static char *dump_sync_map(void)
{
	static char mapstr[MAX_NUM_BACKENDS+1];
	int i;

	memset(mapstr, 0, sizeof(mapstr));

	for (i=0;i<NUM_BACKENDS;i++)
	{
		mapstr[i] = session_context->sync_map[i]?'1':'0';
	}
	return mapstr;
}

/*
 * Set pending response
 */
void pool_set_pending_response(void)
{
	if (!session_context)
		ereport(ERROR,
				(errmsg("pool_set_pending_response: session context is not initialized")));

	session_context->is_pending_response = true;
}

/*
 * Unset pending response
 */
void pool_unset_pending_response(void)
{
	if (!session_context)
		ereport(ERROR,
				(errmsg("pool_unset_pending_response: session context is not initialized")));

	session_context->is_pending_response = false;
}

/*
 * Returns true if pending response is set
 */
bool pool_is_pending_response(void)
{
	if (!STREAM)
		return false;

	if (!session_context)
		ereport(ERROR,
				(errmsg("pool_is_pending_response: session context is not initialized")));

	return session_context->is_pending_response;
}


/*
 * Initialize pending message list
 */
void pool_pending_messages_init(void)
{
	if (!session_context)
		ereport(ERROR,
				(errmsg("pool_pending_message_init: session context is not initialized")));

	session_context->pending_messages = NIL;
}

/*
 * Destroy pending message list
 */
void pool_pending_messages_destroy(void)
{
	ListCell   *cell;
	POOL_PENDING_MESSAGE *msg;

	if (!session_context)
		ereport(ERROR,
				(errmsg("pool_pending_message_destory: session context is not initialized")));

	foreach(cell, session_context->pending_messages)
	{
		msg = (POOL_PENDING_MESSAGE *) lfirst(cell);
		pfree(msg->contents);
	}
	list_free(session_context->pending_messages);
}

/*
 * Create one message.
 */
POOL_PENDING_MESSAGE *pool_pending_messages_create(char kind, int len, char *contents)
{
	POOL_PENDING_MESSAGE* msg;
	MemoryContext old_context;

	if (!session_context)
		ereport(ERROR,
				(errmsg("pool_pending_message_create: session context is not initialized")));

	old_context = MemoryContextSwitchTo(session_context->memory_context);
	msg = palloc(sizeof(POOL_PENDING_MESSAGE));

	switch (kind)
	{
		case 'P':
		msg->type = POOL_PARSE;
		break;

		case 'B':
		msg->type = POOL_BIND;
		break;

		case 'C':
		msg->type = POOL_CLOSE;
		break;

		default:
			ereport(ERROR,
					(errmsg("pool_pending_message_create: unknow kind: %c", kind)));
		break;
	}

	msg->contents = palloc(len);
	memcpy(msg->contents, contents, len);
	msg->contents_len = len;

	MemoryContextSwitchTo(old_context);

	return msg;
}

/*
 * Add one message
 */
void pool_pending_message_add(POOL_PENDING_MESSAGE* message)
{
	POOL_PENDING_MESSAGE* msg;
	MemoryContext old_context;

	if (!session_context)
		ereport(ERROR,
				(errmsg("pool_pending_message_add: session context is not initialized")));

	ereport(DEBUG1,
			(errmsg("pool_pending_message_add: message type:%d message len:%d",
					message->type, message->contents_len)));

	old_context = MemoryContextSwitchTo(session_context->memory_context);
	msg = copy_pending_message(message);
	session_context->pending_messages = lappend(session_context->pending_messages, msg);
	MemoryContextSwitchTo(old_context);
}

/*
 * Try to find the first message specified by the message type in the message
 * list. If found, a copy of the message is returned and the message is
 * removed the message list. If not, returns NULL.
 */
POOL_PENDING_MESSAGE *pool_pending_message_remove(POOL_MESSAGE_TYPE type)
{
	ListCell   *cell;
	ListCell   *prev;
	ListCell   *next;
	POOL_PENDING_MESSAGE *msg;
	MemoryContext old_context;

	if (!session_context)
		ereport(ERROR,
				(errmsg("pool_pending_message_remove: session context is not initialized")));

	old_context = MemoryContextSwitchTo(session_context->memory_context);

	prev = NULL;
	msg = NULL;

	for (cell = list_head(session_context->pending_messages); cell; cell = next)
	{
		POOL_PENDING_MESSAGE *m = (POOL_PENDING_MESSAGE *) lfirst(cell);

		next = lnext(cell);

		if (m->type == type)
		{
			msg = copy_pending_message(m);

			session_context->pending_messages =
				list_delete_cell(session_context->pending_messages, cell, prev);

			break;
		}
		else
			prev = cell;
	}

	MemoryContextSwitchTo(old_context);
	return msg;
}

/*
 * Get message specification (either statement ('S') or portal ('P')) from a
 * close message.
 */
char pool_get_close_message_spec(POOL_PENDING_MESSAGE *msg)
{
	return *msg->contents;
}

/*
 * Get statement or portal name from close message.
 * The returned pointer is within "msg".
 */
char *pool_get_close_message_name(POOL_PENDING_MESSAGE *msg)
{
	return (msg->contents)+1;
}

/*
 * Perform deep copy of POOL_PENDING_MESSAGE object in the current memory context.
 */
static POOL_PENDING_MESSAGE *copy_pending_message(POOL_PENDING_MESSAGE *message)
{
	POOL_PENDING_MESSAGE *msg;

	msg = palloc(sizeof(POOL_PENDING_MESSAGE));
	memcpy(msg, message, sizeof(POOL_PENDING_MESSAGE));
	msg->contents = palloc(msg->contents_len);
	memcpy(msg->contents, message->contents, msg->contents_len);

	return msg;
}
