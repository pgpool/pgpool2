/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Portions Copyright (c) 2003-2009, PgPool Global Development Group
 * Portions Copyright (c) 2003-2004, PostgreSQL Global Development Group
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

#include <errno.h>
#include <string.h>
#include <sys/shm.h>

#include "pool_ipc.h"


#ifdef SHM_SHARE_MMU			/* use intimate shared memory on Solaris */
#define PG_SHMAT_FLAGS			SHM_SHARE_MMU
#else
#define PG_SHMAT_FLAGS			0
#endif


#define MAX_ON_EXITS 20

static struct ONEXIT
{
	void		(*function) (int code, Datum arg);
	Datum		arg;
}	on_shmem_exit_list[MAX_ON_EXITS];

static int	on_shmem_exit_index;


static void IpcMemoryDetach(int status, Datum shmaddr);
static void IpcMemoryDelete(int status, Datum shmId);


/*
 * Create a shared memory segment of the given size and initialize.  Also,
 * register an on_shmem_exit callback to release the storage.
 */
void *
pool_shared_memory_create(size_t size)
{
	int			shmid;
	void	   *memAddress;

	/* Try to create new segment */
	shmid = shmget(IPC_PRIVATE, size, IPC_CREAT | IPC_EXCL | IPCProtection);

	if (shmid < 0)
	{
		pool_error("could not create shared memory segment: %s",
				   strerror(errno));
		return NULL;
	}

	/* Register on-exit routine to delete the new segment */
	on_shmem_exit(IpcMemoryDelete, shmid);

	/* OK, should be able to attach to the segment */
	memAddress = shmat(shmid, NULL, PG_SHMAT_FLAGS);

	if (memAddress == (void *) -1)
	{
		pool_error("shmat(id=%d) failed: %s", shmid, strerror(errno));
		return NULL;
	}

	/* Register on-exit routine to detach new segment before deleting */
	on_shmem_exit(IpcMemoryDetach, (Datum) memAddress);

	return memAddress;
}

/*
 * Removes a shared memory segment from process' address spaceq (called as
 * an on_shmem_exit callback, hence funny argument list)
 */
static void
IpcMemoryDetach(int status, Datum shmaddr)
{
	if (shmdt((void *) shmaddr) < 0)
		pool_log("shmdt(%p) failed: %s", (void *) shmaddr, strerror(errno));
}

/*
 * Deletes a shared memory segment (called as an on_shmem_exit callback,
 * hence funny argument list)
 */
static void
IpcMemoryDelete(int status, Datum shmId)
{
  	struct shmid_ds shmStat;

  	/*
  	 * Is a previously-existing shmem segment still existing and in use?
  	 */
  	if (shmctl(shmId, IPC_STAT, &shmStat) < 0
  		&& (errno == EINVAL || errno == EACCES))
  		return;
  	else if (shmStat.shm_nattch != 0)
  		return;

	if (shmctl(shmId, IPC_RMID, NULL) < 0)
		pool_log("shmctl(%lu, %d, 0) failed: %s",
				 shmId, IPC_RMID, strerror(errno));
}

void
pool_shmem_exit(int code)
{
	shmem_exit(code);
	/* Close syslog connection here as this function is always called on exit */
	closelog();
}

/*
 * Run all of the on_shmem_exit routines --- but don't actually exit.  This
 * is used by the postmaster to re-initialize shared memory and semaphores
 * after a backend dies horribly.
 */
void
shmem_exit(int code)
{
	pool_debug("shmem_exit(%d)", code);

	/*
	 * Call all the registered callbacks.
	 *
	 * As with proc_exit(), we remove each callback from the list before
	 * calling it, to avoid infinite loop in case of error.
	 */
	while (--on_shmem_exit_index >= 0)
		(*on_shmem_exit_list[on_shmem_exit_index].function) (code,
								on_shmem_exit_list[on_shmem_exit_index].arg);

	on_shmem_exit_index = 0;
}

/*
 * This function adds a callback function to the list of functions invoked
 * by shmem_exit().
 */
void
on_shmem_exit(void (*function) (int code, Datum arg), Datum arg)
{
	if (on_shmem_exit_index >= MAX_ON_EXITS)
		pool_error("out of on_shmem_exit slots");

	on_shmem_exit_list[on_shmem_exit_index].function = function;
	on_shmem_exit_list[on_shmem_exit_index].arg = arg;

	++on_shmem_exit_index;
}

/*
 * This function clears all on_proc_exit() and on_shmem_exit() registered
 * functions.  This is used just after forking a backend, so that the
 * backend doesn't believe it should call the postmaster's on-exit routines
 * when it exits...
 */
void
on_exit_reset(void)
{
	on_shmem_exit_index = 0;
}
