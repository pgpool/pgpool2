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
 * pool_ssl.c: ssl negotiation functions
 *
 */

#include <string.h>

#include "config.h"
#include "pool.h"
#include "utils/elog.h"
#include "utils/pool_stream.h"
#include "pool_config.h"

#ifdef USE_SSL

#define SSL_RETURN_VOID_IF(cond, msg) \
	do { \
		if ( (cond) ) { \
			perror_ssl( (msg) ); \
			return; \
		} \
	} while (0);

#define SSL_RETURN_ERROR_IF(cond, msg) \
	do { \
		if ( (cond) ) { \
			perror_ssl( (msg) ); \
			return -1; \
		} \
	} while (0);

#include <arpa/inet.h> /* for htonl() */

/* Major/minor codes to negotiate SSL prior to startup packet */
#define NEGOTIATE_SSL_CODE ( 1234<<16 | 5679 )

/* enum flag for differentiating server->client vs client->server SSL */
enum ssl_conn_type { ssl_conn_clientserver, ssl_conn_serverclient };

/* perform per-connection ssl initialization.  returns nonzero on error */
static int init_ssl_ctx(POOL_CONNECTION *cp, enum ssl_conn_type conntype);

/* OpenSSL error message */
static void perror_ssl(const char *context);

/* attempt to negotiate a secure connection */
void pool_ssl_negotiate_clientserver(POOL_CONNECTION *cp) {
	int ssl_packet[2] = { htonl(sizeof(int)*2), htonl(NEGOTIATE_SSL_CODE) };
	char server_response;

	cp->ssl_active = -1;

	if ( (!pool_config->ssl) || init_ssl_ctx(cp, ssl_conn_clientserver))
		return;

	ereport(DEBUG1,
		(errmsg("attempting to negotiate a secure connection"),
			 errdetail("sending client->server SSL request")));
	pool_write_and_flush(cp, ssl_packet, sizeof(int)*2);

	if (pool_read(cp, &server_response, 1) < 0)
	{
		ereport(WARNING,
				(errmsg("error while attempting to negotiate a secure connection, pool_read failed")));
 		return;
	}

	ereport(DEBUG1,
		(errmsg("attempting to negotiate a secure connection"),
			 errdetail("client->server SSL response: %c", server_response)));

	switch (server_response) {
		case 'S':
			SSL_set_fd(cp->ssl, cp->fd);
			SSL_RETURN_VOID_IF( (SSL_connect(cp->ssl) < 0),
			                    "SSL_connect");
			cp->ssl_active = 1;
			break;
		case 'N':
			/*
			 * If backend does not support SSL but pgpool does, we get this.
			 * i.e. This is normal.
			 */
			ereport(DEBUG1,
				(errmsg("attempting to negotiate a secure connection"),
					 errdetail("server doesn't want to talk SSL")));
			break;
		default:
			ereport(WARNING,
					(errmsg("error while attempting to negotiate a secure connection, unhandled response: %c", server_response)));
			break;
	}
}


/* attempt to negotiate a secure connection */
void pool_ssl_negotiate_serverclient(POOL_CONNECTION *cp) {

	cp->ssl_active = -1;

	if ( (!pool_config->ssl) || init_ssl_ctx(cp, ssl_conn_serverclient)) {
		/* write back an "SSL reject" response before returning */
		pool_write_and_flush(cp, "N", 1);
	} else {
		/* write back an "SSL accept" response */
		pool_write_and_flush(cp, "S", 1);

		SSL_set_fd(cp->ssl, cp->fd);
		SSL_RETURN_VOID_IF( (SSL_accept(cp->ssl) < 0), "SSL_accept");
		cp->ssl_active = 1;
	}
}

void pool_ssl_close(POOL_CONNECTION *cp) {
	if (cp->ssl) { 
		SSL_shutdown(cp->ssl); 
		SSL_free(cp->ssl); 
	} 

	if (cp->ssl_ctx) 
		SSL_CTX_free(cp->ssl_ctx);
}

int pool_ssl_read(POOL_CONNECTION *cp, void *buf, int size) {
	int n;
	int err;

 retry:
	errno = 0;
	n = SSL_read(cp->ssl, buf, size);
	err = SSL_get_error(cp->ssl, n);

	switch (err)
	{
		case SSL_ERROR_NONE:
			break;
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:

			/*
			 * Returning 0 here would cause caller to wait for read-ready,
			 * which is not correct since what SSL wants is wait for
			 * write-ready.  The former could get us stuck in an infinite
			 * wait, so don't risk it; busy-loop instead.
			 */
			goto retry;

		case SSL_ERROR_SYSCALL:
			if (n == -1)
			{
				ereport(WARNING,
						(errmsg("ssl read: error: %d", err)));
			}
			else
			{
				ereport(WARNING,
						(errmsg("ssl read: EOF detected")));
				n = -1;
			}
			break;

		case SSL_ERROR_SSL:
		case SSL_ERROR_ZERO_RETURN:
			perror_ssl("SSL_read");
			n = -1;
			break;
		default:
			ereport(WARNING,
					(errmsg("ssl read: unrecognized error code: %d", err)));
			/*
			 * We assume that the connection is broken. Returns 0
			 * rather than -1 in this case because -1 triggers
			 * unwanted failover in the caller (pool_read).
			 */
			n = 0;
			break;
	}

	return n;
}

int pool_ssl_write(POOL_CONNECTION *cp, const void *buf, int size)
{
	int n;
	int err;

retry:
	errno = 0;
	n = SSL_write(cp->ssl, buf, size);
	err = SSL_get_error(cp->ssl, n);
	switch (err)
	{
		case SSL_ERROR_NONE:
			break;

		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			goto retry;

		case SSL_ERROR_SYSCALL:
			if (n == -1)
			{
				ereport(WARNING,
						(errmsg("ssl write: error: %d", err)));
			}
			else
			{
				ereport(WARNING,
						(errmsg("ssl write: EOF detected")));
				n = -1;
			}
			break;

		case SSL_ERROR_SSL:
		case SSL_ERROR_ZERO_RETURN:
			perror_ssl("SSL_write");
			n = -1;
			break;

		default:
		   ereport(WARNING,
				   (errmsg("ssl write: unrecognized error code: %d", err)));
			/*
			 * We assume that the connection is broken.
			 */
			n = -1;
			break;
	}
	return n;
}

static int init_ssl_ctx(POOL_CONNECTION *cp, enum ssl_conn_type conntype) {
	int error = 0;
	char *cacert = NULL, *cacert_dir = NULL;

	/* initialize SSL members */
	cp->ssl_ctx = SSL_CTX_new(TLSv1_method());
	SSL_RETURN_ERROR_IF( (! cp->ssl_ctx), "SSL_CTX_new" );

	if ( conntype == ssl_conn_serverclient) {
		error = SSL_CTX_use_certificate_chain_file(cp->ssl_ctx,
		                                     pool_config->ssl_cert);
		SSL_RETURN_ERROR_IF( (error != 1), "Loading SSL certificate");

		error = SSL_CTX_use_PrivateKey_file(cp->ssl_ctx,
		                                    pool_config->ssl_key,
		                                    SSL_FILETYPE_PEM);
		SSL_RETURN_ERROR_IF( (error != 1), "Loading SSL private key");
	} else {
		/* set extra verification if ssl_ca_cert or ssl_ca_cert_dir are set */
		if (strlen(pool_config->ssl_ca_cert))
			cacert = pool_config->ssl_ca_cert;
		if (strlen(pool_config->ssl_ca_cert_dir))
			cacert_dir = pool_config->ssl_ca_cert_dir;
    
		if ( cacert || cacert_dir ) {
			error = SSL_CTX_load_verify_locations(cp->ssl_ctx,
			                                        cacert,
			                                        cacert_dir);
			SSL_RETURN_ERROR_IF((error != 1), "SSL verification setup");
			SSL_CTX_set_verify(cp->ssl_ctx, SSL_VERIFY_PEER, NULL);
		}
	}

	cp->ssl = SSL_new(cp->ssl_ctx);
	SSL_RETURN_ERROR_IF( (! cp->ssl), "SSL_new");

	return 0;
}

static void perror_ssl(const char *context) {
	unsigned long err;
	static const char *no_err_reason = "no SSL error reported";
	const char *reason;

	err = ERR_get_error();
	if (! err) {
		reason = no_err_reason;
	} else {
		reason = ERR_reason_error_string(err);
	}

	if (reason != NULL) {
		ereport(LOG,
			(errmsg("pool_ssl: \"%s\": \"%s\"", context, reason)));
	} else {
		ereport(LOG,
				(errmsg("pool_ssl: \"%s\": Unknown SSL error %lu", context, err)));
	}
}

/*
 * Return true if SSL layer has any pending data in buffer
 */
bool pool_ssl_pending(POOL_CONNECTION *cp)
{
	if (cp->ssl_active > 0 && SSL_pending(cp->ssl) > 0)
		return true;
	return false;
}

#else /* USE_SSL: wrap / no-op ssl functionality if it's not available */

void pool_ssl_negotiate_serverclient(POOL_CONNECTION *cp) {
	ereport(DEBUG1,
			(errmsg("SSL is requested but SSL support is not available")));
	pool_write_and_flush(cp, "N", 1);
	cp->ssl_active = -1;
}

void pool_ssl_negotiate_clientserver(POOL_CONNECTION *cp) {

	ereport(DEBUG1,
			(errmsg("SSL is requested but SSL support is not available")));

	cp->ssl_active = -1;
}

void pool_ssl_close(POOL_CONNECTION *cp) { return; }

int pool_ssl_read(POOL_CONNECTION *cp, void *buf, int size) {
	ereport(WARNING,
			(errmsg("pool_ssl: SSL i/o called but SSL support is not available")));
	notice_backend_error(cp->db_node_id);
	child_exit(1);
	return -1; /* never reached */
}

int pool_ssl_write(POOL_CONNECTION *cp, const void *buf, int size) {
	ereport(WARNING,
			(errmsg("pool_ssl: SSL i/o called but SSL support is not available")));
	notice_backend_error(cp->db_node_id);
	child_exit(1);
	return -1; /* never reached */
}

bool pool_ssl_pending(POOL_CONNECTION *cp)
{
	return false;
}

#endif /* USE_SSL */
