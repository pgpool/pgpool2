/* -*-pgsql-c-*- */
/*
* $Header$
*
* pgpool: a language independent connection pool server for PostgreSQL
* written by Tatsuo Ishii
*
* Copyright (c) 2003-2013	PgPool Global Development Group
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
* pool_stream.c: stream I/O modules
*
*/

#include "config.h"

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "pool.h"
#include "utils/elog.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/pool_stream.h"
#include "pool_config.h"

static int mystrlen(char *str, int upper, int *flag);
static int mystrlinelen(char *str, int upper, int *flag);
static int save_pending_data(POOL_CONNECTION *cp, void *data, int len);
static int consume_pending_data(POOL_CONNECTION *cp, void *data, int len);
static MemoryContext SwitchToConnectionContext(bool backend_connection);

static MemoryContext
    SwitchToConnectionContext(bool backend_connection)
{
    /*
     * backend connection can live as long as process life,
     * while the frontend connection is only for one session life.
     * So we create backend connection in long living memory context
     */
    if(backend_connection)
        return MemoryContextSwitchTo(TopMemoryContext);
    else
        return MemoryContextSwitchTo(ProcessLoopContext);
}

/*
* open read/write file descriptors.
* returns POOL_CONNECTION on success otherwise NULL.
*/
POOL_CONNECTION *pool_open(int fd, bool backend_connection)
{
	POOL_CONNECTION *cp;

    MemoryContext oldContext = SwitchToConnectionContext(backend_connection);

	cp = (POOL_CONNECTION *)palloc0(sizeof(POOL_CONNECTION));

	/* initialize write buffer */
	cp->wbuf = palloc(WRITEBUFSZ);
	cp->wbufsz = WRITEBUFSZ;
	cp->wbufpo = 0;

	/* initialize pending data buffer */
	cp->hp = palloc(READBUFSZ);
	cp->bufsz = READBUFSZ;
	cp->po = 0;
	cp->len = 0;
	cp->sbuf = NULL;
	cp->sbufsz = 0;
	cp->buf2 = NULL;
	cp->bufsz2 = 0;

	cp->fd = fd;
    
    MemoryContextSwitchTo(oldContext);

	return cp;
}

/*
* close read/write file descriptors.
*/
void pool_close(POOL_CONNECTION *cp)
{
	/*
	 * shutdown connection to the client so that pgpool is not blocked
	 */
	if (!cp->isbackend)
		shutdown(cp->fd, 1);
	close(cp->fd);

	pfree(cp->wbuf);
	pfree(cp->hp);
	if (cp->sbuf)
		pfree(cp->sbuf);
	if (cp->buf2)
		pfree(cp->buf2);
	pool_discard_params(&cp->params);

	pool_ssl_close(cp);

	pfree(cp);
}

void pool_read_with_error(POOL_CONNECTION *cp, void *buf, int len,
                          const char* err_context )
{
	if (pool_read(cp, buf, len) < 0)
	{
        ereport(ERROR,
			(errmsg("failed to read data of length %d from DB node: %d",len,cp->db_node_id),
                 errdetail("error occurred when reading: %s",err_context?err_context:"")));
	}
}
/*
* read len bytes from cp
* returns 0 on success otherwise throws an ereport.
*/
int pool_read(POOL_CONNECTION *cp, void *buf, int len)
{
	static char readbuf[READBUFSZ];

	int consume_size;
	int readlen;

	consume_size = consume_pending_data(cp, buf, len);
	len -= consume_size;
	buf += consume_size;

	while (len > 0)
	{
		if (pool_check_fd(cp))
		{
			if (!IS_MASTER_NODE_ID(cp->db_node_id) && (getpid() != mypid))
			{
                ereport(FATAL,
                    (errmsg("unable to read data from DB node %d",cp->db_node_id),
                         errdetail("data is not ready in DB node")));

			}
			else
			{
                ereport(ERROR,
                    (errmsg("unable to read data from DB node %d",cp->db_node_id),
                         errdetail("pool_check_fd call failed with an error \"%s\"", strerror(errno))));
			}
		}

		if (cp->ssl_active > 0) {
		  readlen = pool_ssl_read(cp, readbuf, READBUFSZ);
		} else {
		  readlen = read(cp->fd, readbuf, READBUFSZ);
		}

		if (readlen == -1)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				ereport(DEBUG1,
					(errmsg("read on socket failed with error :\"%s\"",strerror(errno)),
						 errdetail("retrying...")));
				continue;
			}

			if (cp->isbackend)
			{
				/* if fail_over_on_backend_error is true, then trigger failover */
				if (pool_config->fail_over_on_backend_error)
				{
					notice_backend_error(cp->db_node_id);
					child_exit(1);
                    /* we are in main process */
                    ereport(ERROR,
						(errmsg("unable to read data from DB node %d",cp->db_node_id),
                             errdetail("socket read failed with an error \"%s\"", strerror(errno))));
				}
				else
				{
                    ereport(ERROR,
						(errmsg("unable to read data from DB node %d",cp->db_node_id),
                             errdetail("socket read failed with an error \"%s\"", strerror(errno))));
				}
			}
			else
			{
                ereport(ERROR,
					(errmsg("unable to read data from frontend"),
                         errdetail("socket read failed with an error \"%s\"", strerror(errno))));
			}
		}
		else if (readlen == 0)
		{
			cp->EOF_on_socket = true;
			if (cp->isbackend)
			{
                ereport(FATAL,
                        (errmsg("unable to read data from DB node %d",cp->db_node_id),
                         errdetail("EOF encountered with backend")));

#ifdef NOT_USED
			    /* fatal error, notice to parent and exit */
			    notice_backend_error(IS_MASTER_NODE_ID(cp->db_node_id));
				child_exit(1);
#endif
			}
			else
			{
				/*
				 * if backend offers authentication method, frontend could close connection
				 */
                ereport(ERROR,
					(errmsg("unable to read data from frontend"),
                         errdetail("EOF encountered with frontend")));
			}
		}

		if (len < readlen)
		{
			/* overrun. we need to save remaining data to pending buffer */
			save_pending_data(cp, readbuf+len, readlen-len);
			memmove(buf, readbuf, len);
			break;
		}

		memmove(buf, readbuf, readlen);
		buf += readlen;
		len -= readlen;
	}

	return 0;
}

/*
* read exactly len bytes from cp
* returns buffer address on success otherwise NULL.
*/
char *pool_read2(POOL_CONNECTION *cp, int len)
{
	char *buf;
	int req_size;
	int alloc_size;
	int consume_size;
	int readlen;
    MemoryContext oldContext = SwitchToConnectionContext(cp->isbackend);

	req_size = cp->len + len;

	if (req_size > cp->bufsz2)
	{
		alloc_size = ((req_size+1)/READBUFSZ+1)*READBUFSZ;
		cp->buf2 = repalloc(cp->buf2, alloc_size);
		cp->bufsz2 = alloc_size;
	}

	buf = cp->buf2;

	consume_size = consume_pending_data(cp, buf, len);
	len -= consume_size;
	buf += consume_size;

	while (len > 0)
	{
		if (pool_check_fd(cp))
		{
			if (!IS_MASTER_NODE_ID(cp->db_node_id))
			{
                ereport(FATAL,
                    (errmsg("unable to read data from DB node %d",cp->db_node_id),
                         errdetail("data is not ready in DB node")));
                
			}
			else
			{
                ereport(ERROR,
                    (errmsg("unable to read data from DB node %d",cp->db_node_id),
                         errdetail("pool_check_fd call failed with an error \"%s\"", strerror(errno))));
            }
		}

		if (cp->ssl_active > 0) {
		  readlen = pool_ssl_read(cp, buf, len);
		} else {
		  readlen = read(cp->fd, buf, len);
		}

		if (readlen == -1)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				ereport(DEBUG1,
					(errmsg("read on socket failed with error :\"%s\"",strerror(errno)),
						 errdetail("retrying...")));
				continue;
			}

			if (cp->isbackend)
			{
				/* if fail_over_on_backend_error is true, then trigger failover */
				if (pool_config->fail_over_on_backend_error)
				{
					notice_backend_error(cp->db_node_id);
					child_exit(1);
                    /* we are in main process */
                    ereport(ERROR,
                            (errmsg("unable to read data from DB node %d",cp->db_node_id),
                             errdetail("socket read failed with an error \"%s\"", strerror(errno))));
				}
				else
				{
                    ereport(ERROR,
                        (errmsg("unable to read data from DB node %d",cp->db_node_id),
                             errdetail("do not failover because fail_over_on_backend_error is off")));
				}
			}
			else
			{
                ereport(ERROR,
                    (errmsg("unable to read data from frontend"),
                         errdetail("socket read function returned -1")));
			}
		}
		else if (readlen == 0)
		{
			cp->EOF_on_socket = true;
			if (cp->isbackend)
			{
                ereport(ERROR,
                    (errmsg("unable to read data from backend"),
                         errdetail("EOF read on socket")));

#ifdef NOT_USED
			    /* fatal error, notice to parent and exit */
			    notice_backend_error(IS_MASTER_NODE_ID(cp->db_node_id));
				child_exit(1);
#endif
			}
			else
			{
				/*
				 * if backend offers authentication method, frontend could close connection
				 */
                ereport(ERROR,
                    (errmsg("unable to read data from frontend"),
                         errdetail("EOF read on socket")));

			}
		}

		buf += readlen;
		len -= readlen;
	}
    MemoryContextSwitchTo(oldContext);
	return cp->buf2;
}

/*
 * write len bytes to cp the write buffer.
 * returns 0 on success otherwise -1.
 */
int pool_write_noerror(POOL_CONNECTION *cp, void *buf, int len)
{
	if (len < 0)
        return -1;

	if (cp->no_forward)
		return 0;
    
	while (len > 0)
	{
		int remainder = WRITEBUFSZ - cp->wbufpo;
        
		if (cp->wbufpo >= WRITEBUFSZ)
		{
			/*
			 * Write buffer is full. so flush buffer.
			 * wbufpo is reset in pool_flush_it().
			 */
			if (pool_flush_it(cp) == -1)
                return -1;
            remainder = WRITEBUFSZ;
		}
        
		/* check buffer size */
		if (remainder >= len)
		{
			/* OK, buffer size is enough. */
			remainder = len;
		}
		memcpy(cp->wbuf+cp->wbufpo, buf, remainder);
		cp->wbufpo += remainder;
		buf += remainder;
		len -= remainder;
	}
	return 0;
}

/*
 * write len bytes to cp the write buffer.
 * returns 0 on success otherwise ereport.
 */
int pool_write(POOL_CONNECTION *cp, void *buf, int len)
{
	if (len < 0)
        ereport(ERROR,
			(errmsg("unable to write data to %s",cp->isbackend?"backend":"frontend"),
                 errdetail("invalid data size: %d", len)));
    
    if(pool_write_noerror(cp,buf,len))
        ereport(ERROR,
            (errmsg("unable to write data to %s",cp->isbackend?"backend":"frontend"),
                 errdetail("pool_flush failed")));

    return 0;
}


/*
 * flush write buffer
 * This function does not throws an ereport in case of an error
 */
int pool_flush_it(POOL_CONNECTION *cp)
{
	int sts;
	int wlen;
	int offset;
	wlen = cp->wbufpo;

	if (wlen == 0)
	{
		return 0;
	}

	offset = 0;

	for (;;)
	{
		errno = 0;

#ifdef NOT_USED
		if (!cp->isbackend)
		{
			fd_set	writemask;
			fd_set	exceptmask;

			FD_ZERO(&writemask);
			FD_ZERO(&exceptmask);
			FD_SET(cp->fd, &writemask);
			FD_SET(cp->fd, &exceptmask);

			sts = select(cp->fd+1, NULL, &writemask, &exceptmask, NULL);
			if (sts == -1)
			{
				if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
					continue;
				ereport(WARNING,
						(errmsg("pool_flush_it: select() failed. reason: %s", strerror(errno))));
				cp->wbufpo = 0;
				return -1;
			}
			else if (sts == 0)
			{
				continue;
			}
			else if (FD_ISSET(cp->fd, &exceptmask))
			{
				ereport(LOG,
					(errmsg("unable to flush data"),
						 errdetail("exception occurred while in waiting select() ")));

				cp->wbufpo = 0;
				return -1;
			}
		}
#endif
		if (cp->ssl_active > 0) {
		  sts = pool_ssl_write(cp, cp->wbuf + offset, wlen);
		} else {
		  sts = write(cp->fd, cp->wbuf + offset, wlen);
		}

		if (sts > 0)
		{
			wlen -= sts;

			if (wlen == 0)
			{
				/* write completed */
				break;
			}

			else if (wlen < 0)
			{
				ereport(WARNING,
						(errmsg("pool_flush_it: invalid write size %d", sts)));
				cp->wbufpo = 0;
				return -1;
			}

			else
			{
				/* need to write remaining data */
				offset += sts;
				continue;
			}
		}

		else if (errno == EAGAIN || errno == EINTR)
		{
			continue;
		}

		else
		{
			/* If this is the backend stream, report error. Otherwise
			 * just report debug message.
			 */
			if (cp->isbackend)
				ereport(DEBUG1,
					(errmsg("write on backend %d failed with error :\"%s\"",cp->db_node_id,strerror(errno)),
						 errdetail("while trying to write data from offset: %d wlen: %d",offset, wlen)));
			else
				ereport(DEBUG1,
					(errmsg("write on frontend failed with error :\"%s\"",strerror(errno)),
						 errdetail("while trying to write data from offset: %d wlen: %d",offset, wlen)));
			cp->wbufpo = 0;
			return -1;
		}
	}

	cp->wbufpo = 0;

	return 0;
}

/*
 * flush write buffer and degenerate/failover if error occurs
 */
int pool_flush(POOL_CONNECTION *cp)
{
	if (pool_flush_it(cp) == -1)
	{
		if (cp->isbackend)
		{
			/* if fail_over_on_backend_error is true, then trigger failover */
			if (pool_config->fail_over_on_backend_error)
			{
				notice_backend_error(cp->db_node_id);
				ereport(LOG,
					(errmsg("unable to flush data to backend"),
						 errdetail("do not failover because I am the main process")));

				child_exit(1);
				return -1;
			}
			else
			{
                ereport(FATAL,
                    (errmsg("unable to flush data to backend"),
                         errdetail("do not failover because fail_over_on_backend_error is off")));
			}
		}
		else
		{
			/*
			 * If we are in replication mode, we need to continue the
			 * processing with backends to keep consistency among
			 * backends, thus ignore error.
			 */
			if (REPLICATION)
                ereport(NOTICE,
                    (errmsg("unable to flush data to frontend"),
                         errdetail("pgpool is in replication mode, ignoring error to keep consistency among backends")));
			else
                ereport(ERROR,
                    (errmsg("unable to flush data to frontend")));

		}
	}
	return 0;
}

/*
 * same as pool_flush() but returns -ve value instead of ereport in case of failure
 */
int pool_flush_noerror(POOL_CONNECTION *cp)
{
    if (pool_flush_it(cp) == -1)
    {
        if (cp->isbackend)
        {
            /* if fail_over_on_backend_erro is true, then trigger failover */
            if (pool_config->fail_over_on_backend_error)
            {
                notice_backend_error(cp->db_node_id);
                child_exit(1);
				ereport(LOG,
					(errmsg("unable to flush data to backend"),
						 errdetail("do not failover because I am the main process")));
                return -1;
            }
            else
            {
				ereport(LOG,
					(errmsg("unable to flush data to backend"),
                         errdetail("do not failover because fail_over_on_backend_error is off")));
                return -1;
            }
        }
        else
        {
            /*
             * If we are in replication mode, we need to continue the
             * processing with backends to keep consistency among
             * backends, thus ignore error.
             */
            if (REPLICATION)
                return 0;
            else
                return -1;
        }
    }
    return 0;
}

/*
 * combo of pool_write and pool_flush
 */
void pool_write_and_flush(POOL_CONNECTION *cp, void *buf, int len)
{
	pool_write(cp, buf, len);
	pool_flush(cp);
}

/*
 * same as pool_write_and_flush() but does not throws ereport when error occures
 */
int pool_write_and_flush_noerror(POOL_CONNECTION *cp, void *buf, int len)
{
	int ret;
	ret = pool_write_noerror(cp,buf,len);
	if(ret == 0)
		return pool_flush_noerror(cp);
	return ret;
}

/*
 * read a string until EOF or NULL is encountered.
 * if line is not 0, read until new line is encountered.
*/
char *pool_read_string(POOL_CONNECTION *cp, int *len, int line)
{
	int readp;
	int readsize;
	int readlen;
	int strlength;
	int flag;
	int consume_size;

#ifdef DEBUG
	static char pbuf[READBUFSZ];
#endif

	*len = 0;
	readp = 0;

	/* initialize read buffer */
	if (cp->sbufsz == 0)
	{
        MemoryContext oldContext = SwitchToConnectionContext(cp->isbackend);
		cp->sbuf = palloc(READBUFSZ);
        MemoryContextSwitchTo(oldContext);

		cp->sbufsz = READBUFSZ;
		*cp->sbuf = '\0';
	}

	/* any pending data? */
	if (cp->len)
	{
		if (line)
			strlength = mystrlinelen(cp->hp+cp->po, cp->len, &flag);
		else
			strlength = mystrlen(cp->hp+cp->po, cp->len, &flag);

		/* buffer is too small? */
		if ((strlength + 1) > cp->sbufsz)
		{
            MemoryContext oldContext = SwitchToConnectionContext(cp->isbackend);
			cp->sbufsz = ((strlength+1)/READBUFSZ+1)*READBUFSZ;
			cp->sbuf = repalloc(cp->sbuf, cp->sbufsz);
            MemoryContextSwitchTo(oldContext);
		}

		/* consume pending and save to read string buffer */
		consume_size = consume_pending_data(cp, cp->sbuf, strlength);

		*len = strlength;

		/* is the string null terminated? */
		if (consume_size == strlength && !flag)
		{
			/* not null or line terminated.
			 * we need to read more since we have not encountered NULL or new line yet
			 */
			readsize = cp->sbufsz - strlength;
			readp = strlength;
		}
		else
		{
			ereport(DEBUG1,
				(errmsg("reading string data"),
					 errdetail("read all from pending data. po:%d len:%d",
							   cp->po, cp->len)));
			return cp->sbuf;
		}
	} else
	{
		readsize = cp->sbufsz;
	}

	for (;;)
	{
		if (pool_check_fd(cp))
		{
			if (!IS_MASTER_NODE_ID(cp->db_node_id))
			{
                ereport(FATAL,
                    (errmsg("unable to read data from DB node %d",cp->db_node_id),
                         errdetail("data is not ready in DB node")));
                
			}
			else
			{
                ereport(ERROR,
                    (errmsg("unable to read data from DB node %d",cp->db_node_id),
                         errdetail("pool_check_fd call failed with an error \"%s\"", strerror(errno))));
            }
		}

		if (cp->ssl_active > 0) {
		  readlen = pool_ssl_read(cp, cp->sbuf+readp, readsize);
		} else {
		  readlen = read(cp->fd, cp->sbuf+readp, readsize);
		}

		if (readlen == -1)
		{
			if (cp->isbackend)
			{
				notice_backend_error(cp->db_node_id);
				child_exit(1);
                ereport(ERROR,
                        (errmsg("unable to read data from frontend"),
                         errdetail("socket read function returned -1")));
			}
			else
			{
                ereport(ERROR,
                    (errmsg("unable to read data from frontend"),
                         errdetail("socket read function returned -1")));

			}
		}
		else if (readlen == 0)	/* EOF detected */
		{
			/*
			 * just returns an error, not trigger failover or degeneration
			 */
			cp->EOF_on_socket = true;
            ereport(ERROR,
                (errmsg("unable to read data from %s",cp->isbackend?"backend":"frontend"),
                     errdetail("EOF read on socket")));

		}

		/* check overrun */
		if (line)
			strlength = mystrlinelen(cp->sbuf+readp, readlen, &flag);
		else
			strlength = mystrlen(cp->sbuf+readp, readlen, &flag);

		if (strlength < readlen)
		{
			save_pending_data(cp, cp->sbuf+readp+strlength, readlen-strlength);
			*len += strlength;
			ereport(DEBUG1,
				(errmsg("reading string data"),
					 errdetail("total read %d with pending data po:%d len:%d", *len, cp->po, cp->len)));
			return cp->sbuf;
		}

		*len += readlen;

		/* encountered null or newline? */
		if (flag)
		{
			/* ok we have read all data */
			ereport(DEBUG1,
				(errmsg("reading string data"),
					 errdetail("all data read: total read %d", *len)));
			break;
		}

		readp += readlen;
		readsize = READBUFSZ;

		if ((*len+readsize) > cp->sbufsz)
		{
			cp->sbufsz += READBUFSZ;
            MemoryContext oldContext = SwitchToConnectionContext(cp->isbackend);
			cp->sbuf = repalloc(cp->sbuf, cp->sbufsz);
            MemoryContextSwitchTo(oldContext);
		}
	}
	return cp->sbuf;
}

/*
 * returns the byte length of str, including \0, no more than upper.
 * if encountered \0, flag is set to non 0.
 * example:
 *	mystrlen("abc", 2) returns 2
 *	mystrlen("abc", 3) returns 3
 *	mystrlen("abc", 4) returns 4
 *	mystrlen("abc", 5) returns 4
 */
static int mystrlen(char *str, int upper, int *flag)
{
	int len;

	*flag = 0;

	for (len = 0;len < upper; len++, str++)
	{
	    if (!*str)
	    {
			len++;
			*flag = 1;
			break;
	    }
	}
	return len;
}

/*
 * returns the byte length of str terminated by \n or \0 (including \n or \0), no more than upper.
 * if encountered \0 or \n, flag is set to non 0.
 * example:
 *	mystrlinelen("abc", 2) returns 2
 *	mystrlinelen("abc", 3) returns 3
 *	mystrlinelen("abc", 4) returns 4
 *	mystrlinelen("abc", 5) returns 4
 *	mystrlinelen("abcd\nefg", 4) returns 4
 *	mystrlinelen("abcd\nefg", 5) returns 5
 *	mystrlinelen("abcd\nefg", 6) returns 5
 */
static int mystrlinelen(char *str, int upper, int *flag)
{
	int len;

	*flag = 0;

	for (len = 0;len < upper; len++, str++)
	{
	    if (!*str || *str == '\n')
	    {
			len++;
			*flag = 1;
			break;
	    }
	}
	return len;
}

/*
 * save pending data
 */
static int save_pending_data(POOL_CONNECTION *cp, void *data, int len)
{
	int reqlen;
	size_t realloc_size;
	char *p;

	/* to be safe */
	if (cp->len == 0)
		cp->po = 0;

	reqlen = cp->po + cp->len + len;

	/* pending buffer is enough? */
	if (reqlen > cp->bufsz)
	{
		/* too small, enlarge it */
		realloc_size = (reqlen/READBUFSZ+1)*READBUFSZ;

        MemoryContext oldContext = SwitchToConnectionContext(cp->isbackend);
		p = repalloc(cp->hp, realloc_size);
        MemoryContextSwitchTo(oldContext);

		cp->bufsz = realloc_size;
		cp->hp = p;
	}

	memmove(cp->hp + cp->po + cp->len, data, len);
	cp->len += len;

	return 0;
}

/*
 * consume pending data. returns actually consumed data length.
 */
static int consume_pending_data(POOL_CONNECTION *cp, void *data, int len)
{
	int consume_size;

	if (cp->len <= 0)
		return 0;

	consume_size = Min(len, cp->len);
	memmove(data, cp->hp + cp->po, consume_size);
	cp->len -= consume_size;

	if (cp->len <= 0)
		cp->po = 0;
	else
		cp->po += consume_size;

	return consume_size;
}

/*
 * pool_unread: Put back data to input buffer
 */
int pool_unread(POOL_CONNECTION *cp, void *data, int len)
{
	void *p = cp->hp;
	int n = cp->len + len;
	int realloc_size;

	if (cp->bufsz < n)
	{
		realloc_size = (n/READBUFSZ+1)*READBUFSZ;

        MemoryContext oldContext = SwitchToConnectionContext(cp->isbackend);
		p = repalloc(cp->hp, realloc_size);
        MemoryContextSwitchTo(oldContext);
		
        cp->hp = p;
	}
	if (cp->len != 0)
		memmove(p + len, cp->hp + cp->po, cp->len);
	memmove(p, data, len);
	cp->len = n;
	cp->po = 0;
	return 0;
}

/*
 * pool_push: Push data into buffer stack.
 */
int pool_push(POOL_CONNECTION *cp, void *data, int len)
{
	char *p;

	ereport(DEBUG1,
		(errmsg("flushing data of len: %d", len)));


    MemoryContext oldContext = SwitchToConnectionContext(cp->isbackend);

	if (cp->bufsz3 == 0)
	{
		p = cp->buf3 = palloc(len);
	}
	else
	{
		p = cp->buf3 + cp->bufsz3;
		cp->buf3 = repalloc(cp->buf3, cp->bufsz3 + len);
	}

	memcpy(p, data, len);
	cp->bufsz3 += len;

    MemoryContextSwitchTo(oldContext);
	return 0;
}

/*
 * pool_pop: Pop data from buffer stack and put back data using
 * pool_unread.
 */
void pool_pop(POOL_CONNECTION *cp, int *len)
{
	if (cp->bufsz3 == 0)
	{
		*len = 0;
		ereport(DEBUG1,
				(errmsg("pop data of len: %d", *len)));
		return;
	}

	pool_unread(cp, cp->buf3, cp->bufsz3);
	*len = cp->bufsz3;
	pfree(cp->buf3);
	cp->bufsz3 = 0;
	cp->buf3 = NULL;
	ereport(DEBUG1,
			(errmsg("pop data of len: %d", *len)));
}

/*
 * pool_stacklen: Returns buffer stack length
 * pool_unread.
 */
int pool_stacklen(POOL_CONNECTION *cp)
{
	return cp->bufsz3;
}

/*
 * set non-block flag
 */
void pool_set_nonblock(int fd)
{
	int var;

	/* set fd to none blocking */
	var = fcntl(fd, F_GETFL, 0);
	if (var == -1)
	{
        ereport(FATAL,
            (errmsg("unable to set options on socket"),
                 errdetail("fcntl system call failed with error \"%s\"", strerror(errno))));

	}
	if (fcntl(fd, F_SETFL, var | O_NONBLOCK) == -1)
	{
        ereport(FATAL,
            (errmsg("unable to set options on socket"),
                 errdetail("fcntl system call failed with error \"%s\"", strerror(errno))));
	}
}

/*
 * unset non-block flag
 */
void pool_unset_nonblock(int fd)
{
	int var;

	/* set fd to none blocking */
	var = fcntl(fd, F_GETFL, 0);
	if (var == -1)
	{
        ereport(FATAL,
            (errmsg("unable to set options on socket"),
                 errdetail("fcntl system call failed with error \"%s\"", strerror(errno))));
	}
	if (fcntl(fd, F_SETFL, var & ~O_NONBLOCK) == -1)
	{
        ereport(FATAL,
            (errmsg("unable to set options on socket"),
                 errdetail("fcntl system call failed with error \"%s\"", strerror(errno))));
	}
}
