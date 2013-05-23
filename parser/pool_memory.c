/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2012	PgPool Global Development Group
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
 * pool_memory.c: Memory pooling module for SQL parser.
 *
 */

#undef POOL_MEMORY_DEBUG

#include "pool.h"
#include "pool_memory.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN 3
#define POOL_HEADER_SIZE (sizeof (POOL_CHUNK_HEADER))

POOL_MEMORY_POOL *pool_memory = NULL;

static int get_free_index(unsigned int size);

static int get_free_index(unsigned int size)
{
	int idx = 0;

	if (size > 0)
	{
		size = (size - 1) >> ALIGN;
		while (size)
		{
			size >>= 1;
			idx++;
		}
	}
	return idx;
}

/*
 * pool_memory_alloc:
 *     Returns pointer to allocated memory of given size.
 */
void *pool_memory_alloc(POOL_MEMORY_POOL *pool, unsigned int size)
{
	POOL_BLOCK *block;
	POOL_CHUNK *chunk;

#ifdef POOL_MEMORY_DEBUG
	pool_log("pool_memory_alloc: pool:%p size:%d", pool, size);
#endif

	if ((size + POOL_HEADER_SIZE) > pool->blocksize)
	{
		block = malloc(sizeof(POOL_BLOCK));
		if (block == NULL)
		{
			pool_error("pool_memory_alloc: malloc failed: %s", strerror(errno));
			child_exit(1);
		}
		block->allocsize = block->size = size + POOL_HEADER_SIZE;
		block->block = malloc(size + POOL_HEADER_SIZE);
		if (block->block == NULL)
		{
			pool_error("pool_memory_alloc: malloc failed: %s", strerror(errno));
			child_exit(1);
		}
		block->freepoint = block + (size + POOL_HEADER_SIZE);
		chunk = block->block;
		chunk->header.size = size + POOL_HEADER_SIZE;
		block->next = pool->largeblocks;
		pool->largeblocks = block;
	}
	else
	{
		int fidx = get_free_index(size + POOL_HEADER_SIZE);
		int allocsize = 1 << (fidx + ALIGN);

		/* pick up from freelist */
		if (pool->freelist[fidx] != NULL)
		{
			chunk = pool->freelist[fidx];
			pool->freelist[fidx] = chunk->header.next;
			chunk->header.size = allocsize;
			return chunk->data;
		}
		
		block = pool->blocks;
		if (block == NULL ||
			block->freepoint + allocsize > block->block + block->size)
		{
			block = malloc(sizeof(POOL_BLOCK));
			if (block == NULL)
			{
				pool_error("pool_memory_alloc: malloc failed: %s", strerror(errno));
				child_exit(1);
			}
			block->size = pool->blocksize;
			block->allocsize = 0;
			block->block = malloc(pool->blocksize);
			if (block->block == NULL)
			{
				pool_error("pool_memory_alloc: malloc failed: %s", strerror(errno));
				child_exit(1);
			}
			block->freepoint = block->block;
			block->next = pool->blocks;
			pool->blocks = block;
		}

		block = pool->blocks;
		chunk = block->freepoint;
		block->freepoint += allocsize;
		block->allocsize += allocsize;
		chunk->header.size = allocsize;
	}
	return chunk->data;
}


/*
 * pool_memory_alloc_zero:
 *     Returns pointer to allocated memory of given size.
 *     The allocated memory is cleared.
 */
void *pool_memory_alloc_zero(POOL_MEMORY_POOL *pool, unsigned int size)
{
	void *ptr = pool_memory_alloc(pool, size);
	memset(ptr, 0, size);
	return ptr;
}

/*
 * pool_memory_free:
 *    Frees allocated memory into memory pool.
 */
void pool_memory_free(POOL_MEMORY_POOL *pool, void *ptr)
{
	POOL_CHUNK *chunk = ptr - POOL_HEADER_SIZE;
	int fidx;

#ifdef POOL_MEMORY_DEBUG
	pool_log("pool_memory_free: pool:%p ptr:%p", pool, ptr);
#endif

	if (ptr == NULL)
		return;

	if (chunk->header.size > pool->blocksize)
	{
		POOL_BLOCK *block, *ptr = NULL;

		for (block = pool->largeblocks; block; ptr = block, block = block->next)
		{
			if (block->block == chunk)
				break;
		}

		if (block == NULL)
		{
			pool_log("An address \"%p\" does not exist in memory pool.", chunk);
			return;
		}

		if (ptr == NULL)
		{
			pool->largeblocks = block->next;
		}
		else
		{
			ptr->next = block->next;
		}
		free(block->block);
		free(block);
	}
	else
	{
		fidx = get_free_index(chunk->header.size);
		chunk->header.next = pool->freelist[fidx];
		pool->freelist[fidx] = chunk;
	}
}

/*
 * pool_memory_realloc:
 *     Returns new pointer to allocated memory of given size.
 *     The new memory is copied from the old memory, and the old
 *     memory is freed.
 */
void *pool_memory_realloc(POOL_MEMORY_POOL *pool, void *ptr, unsigned int size)
{
	int fidx;
	void *p;
	POOL_CHUNK *chunk = ptr - POOL_HEADER_SIZE;

	if (size <= chunk->header.size - POOL_HEADER_SIZE)
		return ptr;

	fidx = get_free_index(size + POOL_HEADER_SIZE);
	if (size + POOL_HEADER_SIZE <= pool->blocksize &&
		chunk->header.size <= pool->blocksize &&
		fidx == get_free_index(chunk->header.size))
	{
		return ptr;
	}

	p = pool_memory_alloc(pool, size);
	memmove(p, ptr, chunk->header.size - POOL_HEADER_SIZE);
	pool_memory_free(pool, ptr);

	return p;
}

/*
 * pool_memory_create:
 *     Create a new memory pool.
 */
POOL_MEMORY_POOL *pool_memory_create(int blocksize)
{
	POOL_MEMORY_POOL *pool;
	int i;

	pool = malloc(sizeof(POOL_MEMORY_POOL));
	if (pool == NULL)
	{
		pool_error("pool_memory_create: malloc failed: %s", strerror(errno));
		child_exit(1);
	}

#ifdef POOL_MEMORY_DEBUG
	pool_log("pool_memory_create: blocksize: %d pool:%p", blocksize, pool);
#endif

	pool->blocks = NULL;
	pool->largeblocks = NULL;
	pool->blocksize = blocksize;
	
	for (i = 0; i < SLOT_NUM; i++)
	{
		pool->freelist[i] = NULL;
	}

	return pool;
}

/*
 * pool_memory_delete:
 *     Frees all memory which is allocated in the memory pool.
 */
void pool_memory_delete(POOL_MEMORY_POOL *pool_memory, int reuse)
{
	POOL_BLOCK *block, *ptr;

#ifdef POOL_MEMORY_DEBUG
	pool_log("pool_memory_delete: pool:%p reuse:%d", pool_memory, reuse);
#endif

	/* Reuse the first memory block */
	if (reuse && pool_memory->blocks)
		block = pool_memory->blocks->next;
	else
		block = pool_memory->blocks;

	while (block)
	{
		ptr = block->next;
		free(block->block);
		free(block);
		block = ptr;
	}

	for (block = pool_memory->largeblocks; block;)
	{
		ptr = block->next;
		free(block->block);
		free(block);
		block = ptr;
	}

	if (reuse)
	{
		int i;

		if (pool_memory->blocks)
		{
			pool_memory->blocks->next = NULL;
			pool_memory->blocks->allocsize = 0;
			pool_memory->blocks->freepoint = pool_memory->blocks->block;
		}
		pool_memory->largeblocks = NULL;
		for (i = 0; i < SLOT_NUM; i++)
		{
			pool_memory->freelist[i] = NULL;
		}
	}
	else
	{
		free(pool_memory);
		pool_memory = NULL;
	}
}

/*
 * pool_memory_strdup:
 *     Creates the new string which is copied the given string.
 */
char *pool_memory_strdup(POOL_MEMORY_POOL *pool_memory, const char *string)
{
	int len = strlen(string);
	char *str = pool_memory_alloc(pool_memory, len + 1);

	memmove(str, string, len);
	str[len] = '\0';
	return str;
}

/*
 * Switch memory context from pool_memory to pm
 */
POOL_MEMORY_POOL *pool_memory_context_switch_to(POOL_MEMORY_POOL *pm)
{
	POOL_MEMORY_POOL *old = pool_memory;
	pool_memory = pm;
	return old;
}

