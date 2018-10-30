/* -*-pgsql-c-*- */
/*
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2016	PgPool Global Development Group
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
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/pool_stream.h"
#include "pool_config.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


#ifdef USE_SSL

static SSL_CTX *SSL_frontend_context = NULL;
static bool SSL_initialized = false;
static bool ssl_passwd_cb_called = false;
static int	ssl_passwd_cb(char *buf, int size, int rwflag, void *userdata);
static int	verify_cb(int ok, X509_STORE_CTX *ctx);
static const char *SSLerrmessage(unsigned long ecode);
static void fetch_pool_ssl_cert(POOL_CONNECTION * cp);

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

#include <arpa/inet.h>			/* for htonl() */

/* Major/minor codes to negotiate SSL prior to startup packet */
#define NEGOTIATE_SSL_CODE ( 1234<<16 | 5679 )

/* enum flag for differentiating server->client vs client->server SSL */
enum ssl_conn_type
{
	ssl_conn_clientserver, ssl_conn_serverclient
};

/* perform per-connection ssl initialization.  returns nonzero on error */
static int	init_ssl_ctx(POOL_CONNECTION * cp, enum ssl_conn_type conntype);

/* OpenSSL error message */
static void perror_ssl(const char *context);

/* Attempt to negotiate a secure connection
 * between pgpool-II and PostgreSQL backends
 */
void
pool_ssl_negotiate_clientserver(POOL_CONNECTION * cp)
{
	int			ssl_packet[2] = {htonl(sizeof(int) * 2), htonl(NEGOTIATE_SSL_CODE)};
	char		server_response;

	cp->ssl_active = -1;

	if ((!pool_config->ssl) || init_ssl_ctx(cp, ssl_conn_clientserver))
		return;

	ereport(DEBUG1,
			(errmsg("attempting to negotiate a secure connection"),
			 errdetail("sending client->server SSL request")));
	pool_write_and_flush(cp, ssl_packet, sizeof(int) * 2);

	if (pool_read(cp, &server_response, 1) < 0)
	{
		ereport(WARNING,
				(errmsg("error while attempting to negotiate a secure connection, pool_read failed")));
		return;
	}

	ereport(DEBUG1,
			(errmsg("attempting to negotiate a secure connection"),
			 errdetail("client->server SSL response: %c", server_response)));

	switch (server_response)
	{
		case 'S':
			SSL_set_fd(cp->ssl, cp->fd);
			SSL_RETURN_VOID_IF((SSL_connect(cp->ssl) < 0),
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


/* attempt to negotiate a secure connection
 * between frontend and Pgpool-II
 */
void
pool_ssl_negotiate_serverclient(POOL_CONNECTION * cp)
{

	cp->ssl_active = -1;
	if ((!pool_config->ssl) || !SSL_frontend_context)
	{
		pool_write_and_flush(cp, "N", 1);
	}
	else
	{
		cp->ssl = SSL_new(SSL_frontend_context);

		/* write back an "SSL accept" response */
		pool_write_and_flush(cp, "S", 1);

		SSL_set_fd(cp->ssl, cp->fd);
		SSL_RETURN_VOID_IF((SSL_accept(cp->ssl) < 0), "SSL_accept");
		cp->ssl_active = 1;
		fetch_pool_ssl_cert(cp);
	}
}

void
pool_ssl_close(POOL_CONNECTION * cp)
{
	if (cp->ssl)
	{
		SSL_shutdown(cp->ssl);
		SSL_free(cp->ssl);
	}

	if (cp->ssl_ctx)
		SSL_CTX_free(cp->ssl_ctx);
}

int
pool_ssl_read(POOL_CONNECTION * cp, void *buf, int size)
{
	int			n;
	int			err;

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
			 * We assume that the connection is broken. Returns 0 rather than
			 * -1 in this case because -1 triggers unwanted failover in the
			 * caller (pool_read).
			 */
			n = 0;
			break;
	}

	return n;
}

int
pool_ssl_write(POOL_CONNECTION * cp, const void *buf, int size)
{
	int			n;
	int			err;

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

static int
init_ssl_ctx(POOL_CONNECTION * cp, enum ssl_conn_type conntype)
{
	int			error = 0;
	char	   *cacert = NULL,
			   *cacert_dir = NULL;

	/* initialize SSL members */
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined (LIBRESSL_VERSION_NUMBER))
	cp->ssl_ctx = SSL_CTX_new(TLS_method());
#else
	cp->ssl_ctx = SSL_CTX_new(SSLv23_method());
#endif

	SSL_RETURN_ERROR_IF((!cp->ssl_ctx), "SSL_CTX_new");

	/*
	 * Disable OpenSSL's moving-write-buffer sanity check, because it causes
	 * unnecessary failures in nonblocking send cases.
	 */
	SSL_CTX_set_mode(cp->ssl_ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

	if (conntype == ssl_conn_serverclient)
	{
		/* between frontend and pgpool */
		error = SSL_CTX_use_certificate_chain_file(cp->ssl_ctx,
												   pool_config->ssl_cert);
		SSL_RETURN_ERROR_IF((error != 1), "Loading SSL certificate");

		error = SSL_CTX_use_PrivateKey_file(cp->ssl_ctx,
											pool_config->ssl_key,
											SSL_FILETYPE_PEM);
		SSL_RETURN_ERROR_IF((error != 1), "Loading SSL private key");
	}
	else
	{
		/* between pgpool and backend */
		/* set extra verification if ssl_ca_cert or ssl_ca_cert_dir are set */
		if (strlen(pool_config->ssl_ca_cert))
			cacert = pool_config->ssl_ca_cert;
		if (strlen(pool_config->ssl_ca_cert_dir))
			cacert_dir = pool_config->ssl_ca_cert_dir;

		if (cacert || cacert_dir)
		{
			error = SSL_CTX_load_verify_locations(cp->ssl_ctx,
												  cacert,
												  cacert_dir);
			SSL_RETURN_ERROR_IF((error != 1), "SSL verification setup");
			SSL_CTX_set_verify(cp->ssl_ctx, SSL_VERIFY_PEER, NULL);
		}
	}

	cp->ssl = SSL_new(cp->ssl_ctx);
	SSL_RETURN_ERROR_IF((!cp->ssl), "SSL_new");

	return 0;
}

static void
perror_ssl(const char *context)
{
	unsigned long err;
	static const char *no_err_reason = "no SSL error reported";
	const char *reason;

	err = ERR_get_error();
	if (!err)
	{
		reason = no_err_reason;
	}
	else
	{
		reason = ERR_reason_error_string(err);
	}

	if (reason != NULL)
	{
		ereport(LOG,
				(errmsg("pool_ssl: \"%s\": \"%s\"", context, reason)));
	}
	else
	{
		ereport(LOG,
				(errmsg("pool_ssl: \"%s\": Unknown SSL error %lu", context, err)));
	}
}

/*
 * Obtain reason string for passed SSL errcode
 *
 * ERR_get_error() is used by caller to get errcode to pass here.
 *
 * Some caution is needed here since ERR_reason_error_string will
 * return NULL if it doesn't recognize the error code.  We don't
 * want to return NULL ever.
 */
static const char *
SSLerrmessage(unsigned long ecode)
{
	const char *errreason;
	static char errbuf[32];

	if (ecode == 0)
		return _("no SSL error reported");
	errreason = ERR_reason_error_string(ecode);
	if (errreason != NULL)
		return errreason;
	snprintf(errbuf, sizeof(errbuf), _("SSL error code %lu"), ecode);
	return errbuf;
}


/*
 * Return true if SSL layer has any pending data in buffer
 */
bool
pool_ssl_pending(POOL_CONNECTION * cp)
{
	if (cp->ssl_active > 0 && SSL_pending(cp->ssl) > 0)
		return true;
	return false;
}

static void
fetch_pool_ssl_cert(POOL_CONNECTION * cp)
{
	int			len;
	X509	   *peer = SSL_get_peer_certificate(cp->ssl);

	cp->peer = peer;
	if (peer)
	{
		ereport(DEBUG1,
				(errmsg("got the SSL certificate")));
		len = X509_NAME_get_text_by_NID(X509_get_subject_name(peer), NID_commonName, NULL, 0);
		if (len != -1)
		{
			char	   *peer_cn;

			peer_cn = palloc(len + 1);
			int			r = X509_NAME_get_text_by_NID(X509_get_subject_name(peer), NID_commonName, peer_cn, len + 1);

			peer_cn[len] = '\0';
			if (r != len)
			{
				/* shouldn't happen */
				pfree(peer_cn);
				return;
			}
			cp->client_cert_loaded = true;
			cp->cert_cn = MemoryContextStrdup(TopMemoryContext, peer_cn);
			pfree(peer_cn);
		}
		else
		{
			cp->client_cert_loaded = false;
		}
	}
	else
	{
		cp->client_cert_loaded = false;
	}
}

/*
 *	Passphrase collection callback
 *
 * If OpenSSL is told to use a passphrase-protected server key, by default
 * it will issue a prompt on /dev/tty and try to read a key from there.
 * That's no good during a postmaster SIGHUP cycle, not to mention SSL context
 * reload in an EXEC_BACKEND postmaster child.  So override it with this dummy
 * function that just returns an empty passphrase, guaranteeing failure.
 */
static int
ssl_passwd_cb(char *buf, int size, int rwflag, void *userdata)
{
	/* Set flag to change the error message we'll report */
	ssl_passwd_cb_called = true;
	/* And return empty string */
	Assert(size > 0);
	buf[0] = '\0';
	return 0;
}

/*
 *	Certificate verification callback
 *
 *	This callback allows us to log intermediate problems during
 *	verification, but for now we'll see if the final error message
 *	contains enough information.
 *
 *	This callback also allows us to override the default acceptance
 *	criteria (e.g., accepting self-signed or expired certs), but
 *	for now we accept the default checks.
 */
static int
verify_cb(int ok, X509_STORE_CTX *ctx)
{
	return ok;
}

/*
 *	Initialize global SSL context.
 *
 * If isServerStart is true, report any errors as FATAL (so we don't return).
 * Otherwise, log errors at LOG level and return -1 to indicate trouble,
 * preserving the old SSL state if any.  Returns 0 if OK.
 */
int
SSL_ServerSide_init(void)
{
	STACK_OF(X509_NAME) *root_cert_list = NULL;
	SSL_CTX    *context;
	struct stat buf;

	/* This stuff need be done only once. */
	if (!SSL_initialized)
	{
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined (LIBRESSL_VERSION_NUMBER))
		OPENSSL_init_ssl(0, NULL);
#else
		SSL_library_init();
#endif
		SSL_load_error_strings();

		SSL_initialized = true;
	}

	/*
	 * We use SSLv23_method() because it can negotiate use of the highest
	 * mutually supported protocol version, while alternatives like
	 * TLSv1_2_method() permit only one specific version.  Note that we don't
	 * actually allow SSL v2 or v3, only TLS protocols (see below).
	 */
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined (LIBRESSL_VERSION_NUMBER))
	context = SSL_CTX_new(TLS_method());
#else
	context = SSL_CTX_new(SSLv23_method());
#endif

	if (!context)
	{
		ereport(WARNING,
				(errmsg("could not create SSL context: %s",
						SSLerrmessage(ERR_get_error()))));
		goto error;
	}

	/*
	 * Disable OpenSSL's moving-write-buffer sanity check, because it causes
	 * unnecessary failures in nonblocking send cases.
	 */
	SSL_CTX_set_mode(context, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

	/*
	 * prompt for password for passphrase-protected files
	 */
	SSL_CTX_set_default_passwd_cb(context, ssl_passwd_cb);

	/*
	 * Load and verify server's certificate and private key
	 */
	if (SSL_CTX_use_certificate_chain_file(context, pool_config->ssl_cert) != 1)
	{
		ereport(WARNING,
				(errmsg("could not load server certificate file \"%s\": %s",
						pool_config->ssl_cert, SSLerrmessage(ERR_get_error()))));
		goto error;
	}

	if (stat(pool_config->ssl_key, &buf) != 0)
	{
		ereport(WARNING,
				(errmsg("could not access private key file \"%s\": %m",
						pool_config->ssl_key)));
		goto error;
	}

	if (!S_ISREG(buf.st_mode))
	{
		ereport(WARNING,
				(errmsg("private key file \"%s\" is not a regular file",
						pool_config->ssl_key)));
		goto error;
	}

	/*
	 * Require no public access to key file. If the file is owned by us,
	 * require mode 0600 or less. If owned by root, require 0640 or less to
	 * allow read access through our gid, or a supplementary gid that allows
	 * to read system-wide certificates.
	 *
	 * XXX temporarily suppress check when on Windows, because there may not
	 * be proper support for Unix-y file permissions.  Need to think of a
	 * reasonable check to apply on Windows.  (See also the data directory
	 * permission check in postmaster.c)
	 */
#if !defined(WIN32) && !defined(__CYGWIN__)
	if ((buf.st_uid == geteuid() && buf.st_mode & (S_IRWXG | S_IRWXO)) ||
		(buf.st_uid == 0 && buf.st_mode & (S_IWGRP | S_IXGRP | S_IRWXO)))
	{
		ereport(WARNING,
				(errmsg("private key file \"%s\" has group or world access",
						pool_config->ssl_key),
				 errdetail("File must have permissions u=rw (0600) or less if owned by the Pgpool-II user, or permissions u=rw,g=r (0640) or less if owned by root.")));
	}
#endif

	/*
	 * OK, try to load the private key file.
	 */
	ssl_passwd_cb_called = false;

	if (SSL_CTX_use_PrivateKey_file(context,
									pool_config->ssl_key,
									SSL_FILETYPE_PEM) != 1)
	{
		if (ssl_passwd_cb_called)
			ereport(WARNING,
					(errmsg("private key file \"%s\" cannot be reloaded because it requires a passphrase",
							pool_config->ssl_key)));
		else
			ereport(WARNING,
					(errmsg("could not load private key file \"%s\": %s",
							pool_config->ssl_key, SSLerrmessage(ERR_get_error()))));
		goto error;
	}

	if (SSL_CTX_check_private_key(context) != 1)
	{
		ereport(WARNING,
				(errmsg("check of private key failed: %s",
						SSLerrmessage(ERR_get_error()))));
		goto error;
	}

	/* disallow SSL v2/v3 */
	SSL_CTX_set_options(context, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);

	/* disallow SSL session tickets */
#ifdef SSL_OP_NO_TICKET			/* added in openssl 0.9.8f */
	SSL_CTX_set_options(context, SSL_OP_NO_TICKET);
#endif

	/* disallow SSL session caching, too */
	SSL_CTX_set_session_cache_mode(context, SSL_SESS_CACHE_OFF);

	/*
	 * Load CA store, so we can verify client certificates if needed.
	 */
	if (pool_config->ssl_ca_cert && strlen(pool_config->ssl_ca_cert))
	{
		char	   *cacert = NULL,
				   *cacert_dir = NULL;

		if (strlen(pool_config->ssl_ca_cert))
			cacert = pool_config->ssl_ca_cert;
		if (strlen(pool_config->ssl_ca_cert_dir))
			cacert_dir = pool_config->ssl_ca_cert_dir;

		if (SSL_CTX_load_verify_locations(context, cacert, cacert_dir) != 1 ||
			(root_cert_list = SSL_load_client_CA_file(cacert)) == NULL)
		{
			ereport(WARNING,
					(errmsg("could not load root certificate file \"%s\": %s",
							pool_config->ssl_ca_cert, SSLerrmessage(ERR_get_error()))));
			goto error;
		}

		/*
		 * Always ask for SSL client cert, but don't fail if it's not
		 * presented.  We might fail such connections later, depending on what
		 * we find in pg_hba.conf.
		 */
		SSL_CTX_set_verify(context,
						   (SSL_VERIFY_PEER |
							SSL_VERIFY_CLIENT_ONCE),
						   verify_cb);

		/*
		 * Tell OpenSSL to send the list of root certs we trust to clients in
		 * CertificateRequests.  This lets a client with a keystore select the
		 * appropriate client certificate to send to us.
		 */
		SSL_CTX_set_client_CA_list(context, root_cert_list);
	}

	/*
	 * Success!  Replace any existing SSL_context.
	 */
	if (SSL_frontend_context)
		SSL_CTX_free(SSL_frontend_context);

	SSL_frontend_context = context;

	return 0;

error:
	if (context)
		SSL_CTX_free(context);
	return -1;
}

#else							/* USE_SSL: wrap / no-op ssl functionality if
								 * it's not available */

void
pool_ssl_negotiate_serverclient(POOL_CONNECTION * cp)
{
	ereport(DEBUG1,
			(errmsg("SSL is requested but SSL support is not available")));
	pool_write_and_flush(cp, "N", 1);
	cp->ssl_active = -1;
}

void
pool_ssl_negotiate_clientserver(POOL_CONNECTION * cp)
{

	ereport(DEBUG1,
			(errmsg("SSL is requested but SSL support is not available")));

	cp->ssl_active = -1;
}

void
pool_ssl_close(POOL_CONNECTION * cp)
{
	return;
}

int
pool_ssl_read(POOL_CONNECTION * cp, void *buf, int size)
{
	ereport(WARNING,
			(errmsg("pool_ssl: SSL i/o called but SSL support is not available")));
	notice_backend_error(cp->db_node_id, REQ_DETAIL_SWITCHOVER);
	child_exit(POOL_EXIT_AND_RESTART);
	return -1;					/* never reached */
}

int
pool_ssl_write(POOL_CONNECTION * cp, const void *buf, int size)
{
	ereport(WARNING,
			(errmsg("pool_ssl: SSL i/o called but SSL support is not available")));
	notice_backend_error(cp->db_node_id, REQ_DETAIL_SWITCHOVER);
	child_exit(POOL_EXIT_AND_RESTART);
	return -1;					/* never reached */
}

int
SSL_ServerSide_init(void)
{
	return 0;
}

bool
pool_ssl_pending(POOL_CONNECTION * cp)
{
	return false;
}

#endif							/* USE_SSL */
