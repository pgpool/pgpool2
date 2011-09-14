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
 * pool_memqcache.h: on memory query cache related definitions
 *
 */

#ifndef POOL_MEMQCACHE_H
#define POOL_MEMQCACHE_H

#include "pool.h"
#include <sys/time.h>

/*
 * On memory query cache on shmem is devided into fixed length "cache
 * block". Each block is assigned a "cache block id", which is
 * starting with 0.
 */
typedef char *POOL_CACH_BLOCK;		/* pointer to cache block */
typedef unsigned int POOL_CACHE_BLOCKID;		/* cache block id */
typedef unsigned int POOL_CACHE_ITEMID;		/* cache item id */

/*
 * "Cache id" represents "absolute address" of a cache item.
 */
typedef struct {
	POOL_CACHE_BLOCKID	blockid;
	POOL_CACHE_ITEMID	itemid;
} POOL_CACHEID;	/* cache id */

/*
 * Each block has management space called "cache block header" at the
 * very beginning of the cache block.
 */

#define POOL_BLOCK_USED	0x0001		/* is this block used? */

typedef struct {
	unsigned char flags;		/* flags. see above */
	unsigned int num_items;		/* number of items */
	unsigned int free_bytes;		/* total free space in bytes */	
} POOL_CACHE_BLOCK_HEADER;

typedef struct {
	char query_hash[32];
} POOL_QUERY_HASH;

#define POOL_ITEM_USED	0x0001		/* is this item used? */
#define POOL_ITEM_HAS_NEXT	0x0002		/* is this item has "next" item? */
#define POOL_ITEM_DELETED	0x0004		/* is this item deleted? */
	
typedef struct {
	POOL_QUERY_HASH query_hash;	/* md5 hashed query signature */
	POOL_CACHEID next;			/* next cache item if any */
	unsigned int offset;		/* item offset in this block */
	unsigned char flags;		/* flags. see above */
} POOL_CACHE_ITEM_POINTER;

/*
 * Each block holds several "cach item", which consists of variable
 * lenghth of Data(header plus RowDescription packet and DataRow
 * packet).  Each cache item is assigned "cache item id", which
 * represents the cache item order in a block.
 */

/*
 * "Cache Item" structure holds a SELECT result having several row
 * data in memory cache.  Cache item can be used with either shmem or
 * memcached.
 */

/*
 * "Cache Item header" structure is used to manage each cache item.
 */
typedef struct {
	unsigned int total_length;	/* total length in bytes including myself */
	time_t timestamp;	/* cache creation time */
} POOL_CACHE_ITEM_HEADER;

typedef struct {
	POOL_CACHE_ITEM_HEADER header;		/* cache item header */
	char data[1];	/* variable length data follows */
} POOL_CACHE_ITEM;

/*
 * Possible the largest free space size in bytes
 */
#define POOL_MAX_FREE_SPACE (pool_config->memqcache_cache_block_size - sizeof(POOL_CACHE_BLOCK_HEADER))

#define POOL_FSMM_RATIO (pool_config->memqcache_cache_block_size/256)

#define MAX_VALUE 8192
#define MAX_KEY 256

extern int memcached_connect(void);
extern void memcached_disconnect(void);
extern void memqcache_register(char kind, POOL_CONNECTION *frontend, char *data, int data_len);

/*
 * Cache key
 */
typedef union
{
	POOL_CACHEID		cacheid;		/* cache key (shmem configuration) */
	char hashkey[32];	/* cache key (memcached configuration) */
} POOL_CACHEKEY;

/*
 * Internal buffer structure
 */
typedef struct
{
	size_t bufsize;		/* buffer size */
	size_t buflen;		/* used length */
	char *buf;	/* buffer */
} POOL_INTERNAL_BUFFER;

/*
 * Temporary query cache buffer
 */
typedef struct
{
	bool is_exceeded;		/* true if data size exceeds memqcache_maxcache */
	bool is_discarded;	/* true if this cache entry is discarded */
	char *query;		/* SELECT query */
	POOL_INTERNAL_BUFFER *buffer;
	int num_oids;
	POOL_INTERNAL_BUFFER *oids;
} POOL_TEMP_QUERY_CACHE;

/*
 * Temporary query cache buffer array
 */
typedef struct
{
	int num_caches;
	int array_size;
	POOL_TEMP_QUERY_CACHE *caches[1];	/* actual data continues... */
} POOL_QUERY_CACHE_ARRAY;

/*
 * Query cache statistics structure. This area must be placed on shared
 * memory and protected by QUERY_CACHE_STATS_SEM.
 */
typedef struct
{
	time_t		start_time;		/* start time when the statistics begins */
	long long int num_selects;	/* number of successfull SELECTs */
	long long int num_cache_hits;		/* number of SELECTs extracted from cache */
} POOL_QUERY_CACHE_STATS;

extern POOL_STATUS pool_fetch_from_memory_cache(POOL_CONNECTION *frontend,
												POOL_CONNECTION_POOL *backend,
												char *contents, bool *foundp);

extern bool pool_is_likely_select(char *query);
extern bool pool_is_allow_to_cache(Node *node, char *query);
extern int pool_extract_table_oids(Node *node, int **oidsp);
extern void pool_add_dml_table_oid(int oid);
extern void pool_discard_oid_maps(void);
extern bool pool_is_shmem_cache(void);
extern size_t pool_shared_memory_cache_size(void);
extern int pool_init_memory_cache(size_t size);
extern size_t pool_shared_memory_fsmm_size(void);
extern int pool_init_fsmm(size_t size);
extern void pool_allocate_fsmm_clock_hand(void);

extern POOL_QUERY_CACHE_ARRAY *pool_create_query_cache_array(void);
extern void pool_discard_query_cache_array(POOL_QUERY_CACHE_ARRAY *cache_array);

extern POOL_TEMP_QUERY_CACHE *pool_create_temp_query_cache(char *query);
extern void pool_handle_query_cache(POOL_CONNECTION_POOL *backend, char *query, Node *node, char state);

extern int pool_init_memqcache_stats(void);
extern POOL_QUERY_CACHE_STATS *pool_get_memqcache_stats(void);
extern void pool_reset_memqcache_stats(void);
extern long long int pool_stats_count_up_num_selects(long long int num);
extern long long int pool_stats_count_up_num_cache_hits(void);
extern long long int pool_tmp_stats_count_up_num_selects(void);
extern long long int pool_tmp_stats_get_num_selects(void);
extern void pool_tmp_stats_reset_num_selects(void);

#endif /* POOL_MEMQCACHE_H */
