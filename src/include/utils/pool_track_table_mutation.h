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
 * pool_track_table_mutation.h: In-memory tracking of
 *   recently written tables to prevent stale reads.
 */

#ifndef POOL_TRACK_TABLE_MUTATION_H
#define POOL_TRACK_TABLE_MUTATION_H

#include "pool.h"
#include <sys/time.h>

/*
 * Invalid index marker for linked lists
 */
#define TRACK_TABLE_MUTATION_INVALID_INDEX	(-1)

/*
 * Default TTL in microseconds (100ms) used when replication delay is unknown
 */
#define TRACK_TABLE_MUTATION_DEFAULT_TTL_US	(100 * 1000)

/*
 * Entry in the table mutation hash table (keyed by table/database oids)
 */
typedef struct TrackTableMutationEntry
{
	int			table_oid;		/* Table oid */
	int			dboid;			/* Database oid */
	struct timeval first_write_time;	/* When the entry was first created */
	struct timeval last_write_time; /* When the table was last written */
	uint32		hash;			/* Pre-computed hash value */
	int			next;			/* Next in collision chain */
	bool		in_use;			/* Is this entry in use? */
} TrackTableMutationEntry;

/*
 * Header for the table mutation hash table in shared memory
 */
typedef struct TrackTableMutationHashTable
{
	int			num_buckets;	/* Number of hash buckets */
	int			max_entries;	/* Maximum entries allowed */
	int			num_entries;	/* Current number of entries */
	int			free_list_head; /* Head of free entry list */

	/*
	 * Flexible array members follow in shared memory: int
	 * buckets[num_buckets]; TrackTableMutationEntry entries[max_entries];
	 */
} TrackTableMutationHashTable;

/*
 * Global state for track table mutation feature
 */
typedef struct TrackTableMutationState
{
	bool		initialized;	/* Shmem initialized? */
	uint64		current_ttl_us; /* Current TTL in microseconds */
	struct timeval ttl_last_updated;	/* When TTL was last updated */
	struct timeval last_cleanup_time;	/* When last expired cleanup ran */
	struct timeval global_cold_start_until; /* Global cold start end time */
	uint32		stats_queries_checked;	/* Queries checked */
	uint32		stats_forced_primary;	/* Forced to primary */
	uint32		stats_allowed_replica;	/* Allowed to replica */
} TrackTableMutationState;

/*
 * Main shared memory structure containing all components
 */
typedef struct TrackTableMutationShmem
{
	TrackTableMutationState state;
	TrackTableMutationHashTable *table_map;
} TrackTableMutationShmem;

/* ----------------
 * Public API functions
 * ----------------
 */

/*
 * Initialize shared memory structures for track table mutation.
 * Called from pgpool_main.c after pool_init_pool_info().
 */
extern void pool_track_table_mutation_init(void);

/*
 * Initialize per-child process state for track table mutation.
 * Called from child.c when a new child process starts.
 * Sets up cold start tracking.
 */
extern void pool_track_table_mutation_child_init(void);

/*
 * Check if the child process is in cold start period.
 * During cold start, all queries are routed to primary.
 * Returns true if in cold start, false otherwise.
 */
extern bool pool_track_table_mutation_in_cold_start(void);

/*
 * Trigger a global cold start period for all processes.
 * Used after watchdog leader change to avoid stale reads.
 */
extern void pool_track_table_mutation_trigger_global_cold_start(void);

/*
 * Get oid of current database.
 */
extern int	pool_track_table_mutation_get_database_oid(void);

/*
 * Check if a table was recently written to (is "stale").
 * If stale, reads from this table should go to primary.
 * Returns true if table is stale (recently written), false otherwise.
 */
extern bool pool_track_table_mutation_table_is_stale(
													 int table_oid, int dboid);

/*
 * Mark tables as recently written.
 * Called after INSERT/UPDATE/DELETE queries complete.
 * table_oids: array of table oids
 * num_tables: number of tables in array
 * dboid: database oid
 */
extern void pool_track_table_mutation_mark_tables_written(
														  const int *table_oids, int num_tables, int dboid);

/*
 * Convenience function to mark a single table as written.
 * table_oid: table oid
 * dboid: database oid
 */
extern void pool_track_table_mutation_mark_table_written(
														 int table_oid, int dboid);

/*
 * Update the TTL based on current replication delay.
 * Called from pool_worker_child.c when replication delay is updated.
 * delay_us: replication delay in microseconds
 */
extern void pool_track_table_mutation_update_ttl(uint64 delay_us);

/*
 * Calculate required shared memory size for track table mutation.
 */
extern Size pool_track_table_mutation_shmem_size(void);

#endif							/* POOL_TRACK_TABLE_MUTATION_H */
