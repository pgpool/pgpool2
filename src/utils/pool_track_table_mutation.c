/* -*-pgsql-c-*- */
/*
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2026	PgPool Global Development Group
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
 * pool_track_table_mutation.c: In-memory tracking of recently
 *   written tables to prevent stale reads from replicas.
 *
 * Based on the "lagless" architecture from Tailor Brands.
 */

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "pool.h"
#include "pool_config.h"
#include "context/pool_session_context.h"
#include "utils/pool_track_table_mutation.h"
#include "utils/elog.h"
#include "utils/pool_ipc.h"
#include "utils/palloc.h"
#include "utils/pool_relcache.h"

#define DATABASE_TO_OID_QUERY \
	"SELECT oid FROM pg_catalog.pg_database" \
	" WHERE datname = '%s'"

/*
 * Helper macro: true when the feature is not active.
 */
#define TRACK_TABLE_MUTATION_DISABLED() \
	(pool_config->disable_load_balance_on_write != \
	 DLBOW_DML_ADAPTIVE_GLOBAL || \
	 track_table_mutation_shmem == NULL)

/* ----------------
 * Local variables
 * ----------------
 */

/* Pointer to shared memory structure */
static TrackTableMutationShmem *track_table_mutation_shmem = NULL;

/* Per-process cold start tracking (not in shared memory) */
static struct timeval process_start_time;
static bool cold_start_initialized = false;

/* ----------------
 * Helper macros for flexible arrays in shared memory
 * ----------------
 */

/* Get pointer to bucket array in table map */
#define TABLE_MAP_BUCKETS(map) \
	((int *)((char *)(map) + \
		sizeof(TrackTableMutationHashTable)))

/* Get pointer to entry array in table map */
#define TABLE_MAP_ENTRIES(map) \
	((TrackTableMutationEntry *)((char *)(map) + \
		sizeof(TrackTableMutationHashTable) + \
		(map)->num_buckets * sizeof(int)))

/* ----------------
 * Semaphore lock helpers
 * ----------------
 */

static inline void
table_map_lock(void)
{
	pool_semaphore_lock(TRACK_TABLE_MUTATION_TABLE_SEM);
}

static inline void
table_map_unlock(void)
{
	pool_semaphore_unlock(TRACK_TABLE_MUTATION_TABLE_SEM);
}

/* ----------------
 * Hash functions
 * ----------------
 */

/*
 * FNV-1a hash for a (table oid, database oid) pair.
 *
 * FNV-1a (Fowler-Noll-Vo, "1a" variant) is a simple, fast non-cryptographic
 * hash: starting from a fixed 32-bit "offset basis", for every input byte it
 * XORs the byte into the accumulator and then multiplies by a fixed "FNV
 * prime".  It has good dispersion for short keys and needs no table or seed,
 * which makes it a good fit for spreading oid pairs across the hash buckets
 * here.  The two magic constants below are the standard 32-bit FNV offset
 * basis and prime.  See https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
 */
static uint32
fnv1a_hash_table_key(int table_oid, int dboid)
{
	uint32		hash = 2166136261u; /* 32-bit FNV offset basis */
	uint32		data[2];
	const unsigned char *bytes;
	size_t		i;

	data[0] = (uint32) table_oid;
	data[1] = (uint32) dboid;
	bytes = (const unsigned char *) data;

	for (i = 0; i < sizeof(data); i++)
	{
		hash ^= bytes[i];
		hash *= 16777619u;		/* FNV prime */
	}

	return hash;
}

/* ----------------
 * Time utilities
 * ----------------
 */

/*
 * Get elapsed time in microseconds between two timevals
 */
static int64
elapsed_us(struct timeval *start, struct timeval *end)
{
	return ((int64) (end->tv_sec - start->tv_sec) * 1000000)
		+ (end->tv_usec - start->tv_usec);
}

/* ----------------
 * Database oid lookup
 * ----------------
 */

static int
track_table_mutation_get_database_oid_internal(void)
{
	int			oid = 0;
	static POOL_RELCACHE *relcache;
	POOL_CONNECTION_POOL *backend;
	POOL_SESSION_CONTEXT *session_context;

	/* Safety check: must have shmem initialized */
	if (track_table_mutation_shmem == NULL)
		return oid;

	session_context = pool_get_session_context(false);
	if (session_context == NULL)
		return oid;

	backend = session_context->backend;
	if (backend == NULL ||
		MAIN_CONNECTION(backend) == NULL ||
		MAIN_CONNECTION(backend)->sp == NULL)
		return oid;

	/* Ensure database name is valid */
	if (MAIN_CONNECTION(backend)->sp->database == NULL)
		return oid;

	if (!relcache)
	{
		relcache = pool_create_relcache(
										pool_config->relcache_size,
										DATABASE_TO_OID_QUERY,
										int_register_func,
										int_unregister_func,
										false);
		if (relcache == NULL)
		{
			ereport(LOG,
					(errmsg("track_table_mutation: error creating relcache")));
			return oid;
		}
	}

	oid = (int) (intptr_t) pool_search_relcache(
												relcache, backend,
												MAIN_CONNECTION(backend)->sp->database);
	return oid;
}

int
pool_track_table_mutation_get_database_oid(void)
{
	return track_table_mutation_get_database_oid_internal();
}

/* ----------------
 * Table mutation hash table operations
 * ----------------
 */

/*
 * Initialize table mutation hash table
 */
static void
table_map_init(TrackTableMutationHashTable *map,
			   int num_buckets, int max_entries)
{
	int		   *buckets;
	TrackTableMutationEntry *entries;
	int			i;
	int			invalid = TRACK_TABLE_MUTATION_INVALID_INDEX;

	map->num_buckets = num_buckets;
	map->max_entries = max_entries;
	map->num_entries = 0;
	map->free_list_head = 0;

	buckets = TABLE_MAP_BUCKETS(map);
	entries = TABLE_MAP_ENTRIES(map);

	/* Initialize all buckets to empty */
	for (i = 0; i < num_buckets; i++)
		buckets[i] = invalid;

	/* Initialize free list - chain all entries */
	for (i = 0; i < max_entries; i++)
	{
		entries[i].in_use = false;
		entries[i].next = (i < max_entries - 1) ?
			i + 1 : invalid;
	}

	ereport(DEBUG1,
			(errmsg("track_table_mutation: table map init %d buckets, %d max entries",
					num_buckets, max_entries)));
}

/*
 * Allocate an entry from the free list
 */
static int
table_map_alloc_entry(TrackTableMutationHashTable *map)
{
	TrackTableMutationEntry *entries;
	int			idx;
	int			invalid = TRACK_TABLE_MUTATION_INVALID_INDEX;

	entries = TABLE_MAP_ENTRIES(map);

	if (map->free_list_head == invalid)
		return invalid;

	idx = map->free_list_head;
	map->free_list_head = entries[idx].next;
	entries[idx].in_use = true;
	entries[idx].next = invalid;
	map->num_entries++;

	return idx;
}

/*
 * Free an entry back to the free list
 */
static void
table_map_free_entry(TrackTableMutationHashTable *map,
					 int idx)
{
	TrackTableMutationEntry *entries;

	entries = TABLE_MAP_ENTRIES(map);

	entries[idx].in_use = false;
	entries[idx].next = map->free_list_head;
	map->free_list_head = idx;
	map->num_entries--;
}

/*
 * Look up a table in the hash table.
 * Returns entry index or INVALID_INDEX if not found.
 * Must be called with lock held.
 */
static int
table_map_lookup(TrackTableMutationHashTable *map,
				 int table_oid, int dboid,
				 uint32 hash)
{
	int		   *buckets = TABLE_MAP_BUCKETS(map);
	TrackTableMutationEntry *entries;
	int			bucket = hash % map->num_buckets;
	int			idx = buckets[bucket];
	int			invalid = TRACK_TABLE_MUTATION_INVALID_INDEX;

	entries = TABLE_MAP_ENTRIES(map);

	while (idx != invalid)
	{
		if (entries[idx].hash == hash &&
			entries[idx].table_oid == table_oid &&
			entries[idx].dboid == dboid)
		{
			return idx;
		}
		idx = entries[idx].next;
	}

	return invalid;
}

/*
 * Insert or update a table entry.
 * Must be called with lock held.
 */
static void
table_map_insert(TrackTableMutationHashTable *map,
				 int table_oid, int dboid,
				 uint32 hash,
				 struct timeval *write_time)
{
	int		   *buckets = TABLE_MAP_BUCKETS(map);
	TrackTableMutationEntry *entries;
	int			bucket = hash % map->num_buckets;
	int			idx;
	int			invalid = TRACK_TABLE_MUTATION_INVALID_INDEX;

	entries = TABLE_MAP_ENTRIES(map);

	/* Check if entry already exists */
	idx = table_map_lookup(map, table_oid, dboid, hash);
	if (idx != invalid)
	{
		/* Update last write time; keep first_write_time */
		entries[idx].last_write_time = *write_time;
		return;
	}

	/* Allocate new entry */
	idx = table_map_alloc_entry(map);
	if (idx == invalid)
	{
		int			b;

		/* Table is full - evict first non-empty bucket */
		for (b = 0; b < map->num_buckets; b++)
		{
			if (buckets[b] != invalid)
			{
				int			victim = buckets[b];

				buckets[b] = entries[victim].next;
				table_map_free_entry(map, victim);
				idx = table_map_alloc_entry(map);
				break;
			}
		}

		if (idx == invalid)
		{
			ereport(WARNING,
					(errmsg("track_table_mutation: failed to allocate entry for oid %d (dboid %d)",
							table_oid, dboid)));
			return;
		}
	}

	/* Initialize new entry */
	entries[idx].table_oid = table_oid;
	entries[idx].dboid = dboid;
	entries[idx].hash = hash;
	entries[idx].first_write_time = *write_time;
	entries[idx].last_write_time = *write_time;

	/* Insert at head of bucket chain */
	entries[idx].next = buckets[bucket];
	buckets[bucket] = idx;

	ereport(DEBUG2,
			(errmsg("track_table_mutation: marked oid %d (dboid %d) written",
					table_oid, dboid)));
}

/*
 * Remove expired entries from the table map.
 * Must be called with lock held.
 */
static void
table_map_cleanup_expired(
						  TrackTableMutationHashTable *map, uint64 ttl_us)
{
	int		   *buckets = TABLE_MAP_BUCKETS(map);
	TrackTableMutationEntry *entries;
	struct timeval now;
	int64		max_stale_us;
	int			removed = 0;
	int			b;
	int			invalid = TRACK_TABLE_MUTATION_INVALID_INDEX;

	entries = TABLE_MAP_ENTRIES(map);
	gettimeofday(&now, NULL);

	max_stale_us = (int64) pool_config
		->track_table_mutation_max_staleness * 1000LL;

	for (b = 0; b < map->num_buckets; b++)
	{
		int		   *prev_ptr = &buckets[b];
		int			idx = buckets[b];

		while (idx != invalid)
		{
			int64		age;
			int64		total_age;
			bool		expired;

			age = elapsed_us(
							 &entries[idx].last_write_time, &now);
			expired = (age > (int64) ttl_us);

			/*
			 * Also evict entries that exceed max_staleness from first write.
			 */
			if (!expired && max_stale_us > 0)
			{
				total_age = elapsed_us(
									   &entries[idx].first_write_time,
									   &now);
				expired = (total_age >= max_stale_us);
			}

			if (expired)
			{
				/* Entry has expired - remove it */
				int			next = entries[idx].next;

				*prev_ptr = next;
				table_map_free_entry(map, idx);
				idx = next;
				removed++;
			}
			else
			{
				prev_ptr = &entries[idx].next;
				idx = entries[idx].next;
			}
		}
	}

	if (removed > 0)
	{
		ereport(DEBUG1,
				(errmsg("track_table_mutation: cleaned up %d expired entries",
						removed)));
	}
}


/* ----------------
 * Public API implementation
 * ----------------
 */

/*
 * Calculate the total shared memory size required
 * for the track table mutation feature.
 */
Size
pool_track_table_mutation_shmem_size(void)
{
	Size		size = 0;
	int			tbl_bkt;
	int			tbl_sz;

	tbl_bkt = pool_config->track_table_mutation_table_buckets;
	tbl_sz = pool_config->track_table_mutation_table_size;

	/* Main structure */
	size += sizeof(TrackTableMutationShmem);

	/* Table mutation hash table */
	size += sizeof(TrackTableMutationHashTable);
	size += tbl_bkt * sizeof(int);
	size += tbl_sz * sizeof(TrackTableMutationEntry);

	return size;
}

/*
 * Initialize shared memory structures for the
 * track table mutation feature.  Allocates and sets
 * up the table map and parse cache in shared memory.
 * Called once from pgpool main process at startup.
 */
void
pool_track_table_mutation_init(void)
{
#ifndef POOL_PRIVATE
	Size		shmem_size;
	char	   *shmem_ptr;
	TrackTableMutationState *st;
	int			tbl_bkt;
	int			tbl_sz;

	if (pool_config->disable_load_balance_on_write !=
		DLBOW_DML_ADAPTIVE_GLOBAL)
	{
		ereport(DEBUG1,
				(errmsg("track_table_mutation: feature disabled")));
		return;
	}

	tbl_bkt = pool_config->track_table_mutation_table_buckets;
	tbl_sz = pool_config->track_table_mutation_table_size;

	shmem_size = pool_track_table_mutation_shmem_size();

	/*
	 * Allocate from the main shared memory segment. Memory is zeroed by
	 * initialize_shared_memory_main_segment().
	 */
	shmem_ptr = pool_shared_memory_segment_get_chunk(
													 shmem_size);
	if (shmem_ptr == NULL)
	{
		ereport(ERROR,
				(errmsg("track_table_mutation: failed to allocate %zu bytes",
						shmem_size)));
		return;
	}

	/* Set up pointers within shared memory */
	track_table_mutation_shmem =
		(TrackTableMutationShmem *) shmem_ptr;
	shmem_ptr += sizeof(TrackTableMutationShmem);

	track_table_mutation_shmem->table_map =
		(TrackTableMutationHashTable *) shmem_ptr;

	/* Initialize table map */
	table_map_init(
				   track_table_mutation_shmem->table_map,
				   tbl_bkt, tbl_sz);

	/* Initialize global state */
	st = &track_table_mutation_shmem->state;
	st->initialized = true;
	st->current_ttl_us = TRACK_TABLE_MUTATION_DEFAULT_TTL_US;
	gettimeofday(&st->ttl_last_updated, NULL);
	gettimeofday(&st->last_cleanup_time, NULL);
	st->global_cold_start_until.tv_sec = 0;
	st->global_cold_start_until.tv_usec = 0;
	st->stats_queries_checked = 0;
	st->stats_forced_primary = 0;
	st->stats_allowed_replica = 0;

	ereport(LOG,
			(errmsg("track_table_mutation: initialized with %zu bytes shmem",
					shmem_size)));
#endif
}

/*
 * Initialize per-child process state.
 * Records the process start time for cold start
 * period tracking.  Called when a child process starts.
 */
void
pool_track_table_mutation_child_init(void)
{
	int			dur;

	if (TRACK_TABLE_MUTATION_DISABLED())
		return;

	gettimeofday(&process_start_time, NULL);
	cold_start_initialized = true;
	dur = pool_config->track_table_mutation_cold_start_duration;

	ereport(DEBUG1,
			(errmsg("track_table_mutation: child init, cold start %d ms",
					dur)));
}

/*
 * Check if the process is in cold start period.
 * During cold start, all queries are routed to
 * primary to avoid stale reads.  Checks both
 * per-process and global (watchdog) cold start.
 */
bool
pool_track_table_mutation_in_cold_start(void)
{
	struct timeval now;
	int64		elapsed_ms;
	int			dur;
	TrackTableMutationState *st;

	if (TRACK_TABLE_MUTATION_DISABLED())
		return false;

	dur = pool_config->track_table_mutation_cold_start_duration;
	if (dur <= 0)
		return false;

	gettimeofday(&now, NULL);
	st = &track_table_mutation_shmem->state;

	/* Check watchdog-triggered global cold start */
	if (st->global_cold_start_until.tv_sec != 0 &&
		elapsed_us(&now,
				   &st->global_cold_start_until) > 0)
	{
		return true;
	}

	/* Check per-process cold start */
	if (!cold_start_initialized)
		return false;

	elapsed_ms = elapsed_us(&process_start_time, &now) / 1000;

	if (elapsed_ms < dur)
	{
		ereport(DEBUG2,
				(errmsg("track_table_mutation: cold start (%ld/%d ms)",
						(long) elapsed_ms, dur)));
		return true;
	}

	return false;
}

/*
 * Trigger a global cold start for all processes.
 * Sets the cold start end time in shared memory.
 * Called after watchdog leader change to force all
 * queries to primary during the transition.
 */
void
pool_track_table_mutation_trigger_global_cold_start(void)
{
	struct timeval now;
	struct timeval *until;
	int			dur;

	if (TRACK_TABLE_MUTATION_DISABLED())
		return;

	dur = pool_config->track_table_mutation_cold_start_duration;
	if (dur <= 0)
		return;

	gettimeofday(&now, NULL);
	until = &track_table_mutation_shmem->state
		.global_cold_start_until;
	*until = now;
	until->tv_sec += dur / 1000;
	until->tv_usec += (dur % 1000) * 1000;
	if (until->tv_usec >= 1000000)
	{
		until->tv_sec += until->tv_usec / 1000000;
		until->tv_usec %= 1000000;
	}

	ereport(LOG,
			(errmsg("track_table_mutation: global cold start for %d ms",
					dur)));
}

/*
 * Check if a table was recently written (is "stale").
 * Returns true if reads should go to primary because
 * the table was written within the current TTL window.
 */
bool
pool_track_table_mutation_table_is_stale(
										 int table_oid, int dboid)
{
	TrackTableMutationHashTable *map;
	struct timeval now;
	uint64		ttl_us;
	uint32		hash;
	int			idx;
	bool		is_stale = false;

	if (TRACK_TABLE_MUTATION_DISABLED())
		return false;

	if (table_oid <= 0 || dboid <= 0)
	{
		is_stale = true;
		goto update_stats;
	}

	map = track_table_mutation_shmem->table_map;
	hash = fnv1a_hash_table_key(table_oid, dboid);

	table_map_lock();

	idx = table_map_lookup(map, table_oid, dboid, hash);
	if (idx != TRACK_TABLE_MUTATION_INVALID_INDEX)
	{
		TrackTableMutationEntry *entries;
		int64		age;
		int64		total_age;
		int64		max_stale_us;

		entries = TABLE_MAP_ENTRIES(map);
		gettimeofday(&now, NULL);
		ttl_us = track_table_mutation_shmem->state
			.current_ttl_us;

		age = elapsed_us(
						 &entries[idx].last_write_time, &now);
		is_stale = (age < (int64) ttl_us);

		/*
		 * Enforce max_staleness hard cap: no entry can force primary routing
		 * longer than max_staleness from its first write.
		 */
		if (is_stale)
		{
			max_stale_us = (int64) pool_config
				->track_table_mutation_max_staleness
				* 1000LL;
			if (max_stale_us > 0)
			{
				total_age = elapsed_us(
									   &entries[idx].first_write_time,
									   &now);
				if (total_age >= max_stale_us)
					is_stale = false;
			}
		}

		ereport(DEBUG2,
				(errmsg("track_table_mutation: oid %d dboid %d elapsed=%ld ttl=%lu stale=%d",
						table_oid, dboid,
						(long) age,
						(unsigned long) ttl_us,
						is_stale)));
	}

	table_map_unlock();

update_stats:
	/* Update statistics using semaphore */
	if (track_table_mutation_shmem != NULL)
	{
		TrackTableMutationState *st;

		table_map_lock();
		st = &track_table_mutation_shmem->state;
		st->stats_queries_checked++;
		if (is_stale)
			st->stats_forced_primary++;
		else
			st->stats_allowed_replica++;
		table_map_unlock();
	}

	return is_stale;
}

/*
 * Mark multiple tables as recently written.
 * Called after DML queries complete to record
 * which tables were modified.
 */
void
pool_track_table_mutation_mark_tables_written(
											  const int *table_oids, int num_tables, int dboid)
{
	TrackTableMutationHashTable *map;
	TrackTableMutationState *st;
	struct timeval now;
	int			i;

	if (TRACK_TABLE_MUTATION_DISABLED())
		return;

	if (num_tables <= 0 || table_oids == NULL ||
		dboid <= 0)
		return;

	map = track_table_mutation_shmem->table_map;
	st = &track_table_mutation_shmem->state;
	gettimeofday(&now, NULL);

	table_map_lock();

	/* Periodically clean up expired entries */
	if (map->num_entries > map->max_entries * 3 / 4)
	{
		int64		since_cleanup;

		since_cleanup = elapsed_us(
								   &st->last_cleanup_time, &now);
		/* 100ms interval */
		if (since_cleanup > 100000)
		{
			table_map_cleanup_expired(
									  map, st->current_ttl_us);
			st->last_cleanup_time = now;
		}
	}

	for (i = 0; i < num_tables; i++)
	{
		uint32		hash;
		int			table_oid = table_oids[i];

		if (table_oid > 0)
		{
			hash = fnv1a_hash_table_key(
										table_oid, dboid);
			table_map_insert(map, table_oid,
							 dboid, hash, &now);
		}
	}

	table_map_unlock();
}

/*
 * Mark a single table as recently written.
 */
void
pool_track_table_mutation_mark_table_written(
											 int table_oid, int dboid)
{
	if (table_oid > 0 && dboid > 0)
	{
		const int	tables[1] = {table_oid};

		pool_track_table_mutation_mark_tables_written(
													  tables, 1, dboid);
	}
}

/*
 * Update the staleness TTL based on observed
 * replication delay.  New TTL = delay * factor,
 * clamped to [default_ttl, 1 hour].
 */
void
pool_track_table_mutation_update_ttl(uint64 delay_us)
{
	uint64		new_ttl;
	double		factor;
	TrackTableMutationState *st;

	if (TRACK_TABLE_MUTATION_DISABLED())
		return;

	factor = pool_config->track_table_mutation_ttl_factor;
	new_ttl = (uint64) (delay_us * factor);
	if (new_ttl < TRACK_TABLE_MUTATION_DEFAULT_TTL_US)
		new_ttl = TRACK_TABLE_MUTATION_DEFAULT_TTL_US;

	/* Maximum TTL of 1 hour */
	if (new_ttl > 3600ULL * 1000000ULL)
		new_ttl = 3600ULL * 1000000ULL;

	st = &track_table_mutation_shmem->state;
	st->current_ttl_us = new_ttl;
	gettimeofday(&st->ttl_last_updated, NULL);

	ereport(DEBUG1,
			(errmsg("track_table_mutation: TTL=%lu us (delay=%lu factor=%.1f)",
					(unsigned long) new_ttl,
					(unsigned long) delay_us,
					factor)));
}
