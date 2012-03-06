/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2009	PgPool Global Development Group
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
 * pool_memory.h: Memory pooling module for SQL parser.
 *
 */

#ifndef POOL_MEMORY_H
#define POOL_MEMORY_H

#define SLOT_NUM 11

#define PARSER_BLOCK_SIZE 8192
#define PREPARE_BLOCK_SIZE 1024

typedef struct POOL_BLOCK {
	int size;
	int allocsize;
	void *block;
	void *freepoint;
	struct POOL_BLOCK *next;
} POOL_BLOCK;

typedef union {
	unsigned int size;
	struct POOL_CHUNK *next;
} POOL_CHUNK_HEADER;

typedef struct POOL_CHUNK {
	POOL_CHUNK_HEADER header;
	char data[1];
} POOL_CHUNK;

typedef struct {
	int size;
	int blocksize;
	POOL_BLOCK *blocks;
	POOL_BLOCK *largeblocks;
	POOL_CHUNK *freelist[SLOT_NUM];
} POOL_MEMORY_POOL;

extern POOL_MEMORY_POOL *pool_memory;

extern void *pool_memory_alloc(POOL_MEMORY_POOL *pool, unsigned int size);
extern void pool_memory_free(POOL_MEMORY_POOL *pool, void *ptr);
extern void *pool_memory_realloc(POOL_MEMORY_POOL *pool, void *ptr, unsigned int size);
extern POOL_MEMORY_POOL *pool_memory_create(int blocksize);
extern void pool_memory_delete(POOL_MEMORY_POOL *pool_memory, int reuse);
extern char *pool_memory_strdup(POOL_MEMORY_POOL *pool_memory, const char *string);
extern void *pool_memory_alloc_zero(POOL_MEMORY_POOL *pool_memory, unsigned int size);
extern POOL_MEMORY_POOL *pool_memory_context_switch_to(POOL_MEMORY_POOL *pm);

#define palloc(s) pool_memory_alloc(pool_memory, (s))
#define pfree(p)  pool_memory_free(pool_memory, (p))
#define repalloc(p, s)  pool_memory_realloc(pool_memory, (p), (s))
#define pstrdup(s)  pool_memory_strdup(pool_memory, (s))
#define palloc0(s) pool_memory_alloc_zero(pool_memory, (s))
#define palloc0fast(s) pool_memory_alloc_zero(pool_memory, (s))	/* Added in 8.4 */

#endif /* POOL_MEMORY_H */
