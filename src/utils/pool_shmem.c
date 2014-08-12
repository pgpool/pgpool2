/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Portions Copyright (c) 2003-2011, PgPool Global Development Group
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
#include "utils/elog.h"
#include <errno.h>
#include <string.h>
#include <sys/shm.h>

#include "utils/pool_ipc.h"


#ifdef SHM_SHARE_MMU			/* use intimate shared memory on Solaris */
#define PG_SHMAT_FLAGS			SHM_SHARE_MMU
#else
#define PG_SHMAT_FLAGS			0
#endif

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
		ereport(FATAL,
			(errmsg("could not create shared memory for request size: %ld",size),
				errdetail("shared memory creation failed with error \"%s\"",strerror(errno))));

	/* Register on-exit routine to delete the new segment */
	on_shmem_exit(IpcMemoryDelete, shmid);

	/* OK, should be able to attach to the segment */
	memAddress = shmat(shmid, NULL, PG_SHMAT_FLAGS);

	if (memAddress == (void *) -1)
		ereport(FATAL,
			(errmsg("could not create shared memory for request size: %ld",size),
				errdetail("attach to shared memory [id:%d] failed with reason: \"%s\"",shmid,strerror(errno))));


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
		ereport(LOG,
			(errmsg("removing shared memory segments"),
				 errdetail("shmdt(%p) failed: %s", (void *) shmaddr, strerror(errno))));

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
		ereport(LOG,
			(errmsg("deleting shared memory segments"),
				errdetail("shmctl(%lu, %d, 0) failed: %s",
					   shmId, IPC_RMID, strerror(errno))));
}

void
pool_shmem_exit(int code)
{
	shmem_exit(code);
	/* Close syslog connection here as this function is always called on exit */
	closelog();
}
