/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Portions Copyright (c) 2003-2014, PgPool Global Development Group
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
#include <sys/sem.h>
#include "utils/elog.h"
#include "utils/pool_ipc.h"


#ifndef HAVE_UNION_SEMUN
union semun
{
	int			val;
	struct semid_ds *buf;
	unsigned short *array;
};
#endif


static int	semId;


/*
 * Removes a semaphore set.
 */
static void
IpcSemaphoreKill(int status, Datum semId)
{
	union semun semun;
	struct semid_ds seminfo;

 	/*
 	 * Is a previously-existing sema segment still existing and in use?
 	 */
	semun.buf = &seminfo;
 	if (semctl(semId, 0, IPC_STAT, semun) < 0
 		&& (errno == EINVAL || errno == EACCES
#ifdef EIDRM
			|| errno == EIDRM
#endif
		))
 		return;

	semun.val = 0;				/* unused, but keep compiler quiet */

	if (semctl(semId, 0, IPC_RMID) < 0)
		ereport(LOG,
			(errmsg("removing semaphore set"),
				errdetail("semctl(%lu, 0, IPC_RMID, ...) failed: %s", semId, strerror(errno))));
}

/*
 * Create a semaphore set and initialize.
 */
void
pool_semaphore_create(int numSems)
{
	int			i;

	/* Try to create new semaphore set */
	semId = semget(IPC_PRIVATE, numSems, IPC_CREAT | IPC_EXCL | IPCProtection);

	if (semId < 0)
		ereport(FATAL,
			(errmsg("Unable to create semaphores:%d error:\"%s\"",numSems,strerror(errno))));

	on_shmem_exit(IpcSemaphoreKill, semId);

	/* Initialize it to count 1 */
	for (i = 0; i < numSems; i++)
	{
		union semun semun;

		semun.val = 1;
		if (semctl(semId, i, SETVAL, semun) < 0)
			ereport(FATAL,
				(errmsg("Unable to create semaphores:%d error:\"%s\"",numSems,strerror(errno)),
						errdetail("semctl(%d, %d, SETVAL, %d) failed",semId,i,semun.val)));
	}
}

/*
 * Lock a semaphore (decrement count), blocking if count would be < 0
 */
void
pool_semaphore_lock(int semNum)
{
	int			errStatus;
	struct sembuf sops;

	sops.sem_op = -1;			/* decrement */
	sops.sem_flg = 0;
	sops.sem_num = semNum;

	/*
	 * Note: if errStatus is -1 and errno == EINTR then it means we returned
	 * from the operation prematurely because we were sent a signal.  So we
	 * try and lock the semaphore again.
	 */
	do
	{
		errStatus = semop(semId, &sops, 1);
	} while (errStatus < 0 && errno == EINTR);

	if (errStatus < 0)
		ereport(WARNING,
			(errmsg("failed to lock semaphore error:\"%s\"",strerror(errno))));
}

/*
 * Unlock a semaphore (increment count)
 */
void
pool_semaphore_unlock(int semNum)
{
	int			errStatus;
	struct sembuf sops;

	sops.sem_op = 1;			/* increment */
	sops.sem_flg = 0;
	sops.sem_num = semNum;

	/*
	 * Note: if errStatus is -1 and errno == EINTR then it means we returned
	 * from the operation prematurely because we were sent a signal.  So we
	 * try and unlock the semaphore again. Not clear this can really happen,
	 * but might as well cope.
	 */
	do
	{
		errStatus = semop(semId, &sops, 1);
	} while (errStatus < 0 && errno == EINTR);

	if (errStatus < 0)
		ereport(WARNING,
				(errmsg("failed to unlock semaphore error:\"%s\"",strerror(errno))));
}
