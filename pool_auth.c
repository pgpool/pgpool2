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
 * pool_auth.c: authenticaton stuff
 *
*/

#include "pool.h"
#include "pool_stream.h"
#include "pool_config.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_PARAM_H
#include <param.h>
#endif
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define AUTHFAIL_ERRORCODE "28000"

static POOL_STATUS pool_send_auth_ok(POOL_CONNECTION *frontend, int pid, int key, int protoMajor);
static int do_clear_text_password(POOL_CONNECTION *backend, POOL_CONNECTION *frontend, int reauth, int protoMajor);
static void pool_send_auth_fail(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *cp);
static int do_crypt(POOL_CONNECTION *backend, POOL_CONNECTION *frontend, int reauth, int protoMajor);
static int do_md5(POOL_CONNECTION *backend, POOL_CONNECTION *frontend, int reauth, int protoMajor);


/*
* do authentication against backend. if success return 0 otherwise non 0.
*/
int pool_do_auth(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *cp)
{
	signed char kind;
	int pid;
	int key;
	int protoMajor;
	int length;
	int authkind;
	int i;
	StartupPacket *sp;

	protoMajor = MAJOR(cp);

	kind = pool_read_kind(cp);
	if (kind < 0)
	{
		return -1;
	}

	/* error response? */
	if (kind == 'E')
	{
		/* we assume error response at this stage is likely version
		 * protocol mismatch (v3 frontend vs. v2 backend). So we throw
		 * a V2 protocol error response in the hope that v3 frontend
		 * will negotiate again using v2 protocol.
		 */
		pool_log("pool_do_auth: maybe protocol version mismatch (current version %d)", protoMajor);
		ErrorResponse(frontend, cp);
		return -1;
	}
	else if (kind != 'R')
	{
		pool_error("pool_do_auth: expect \"R\" got %c", kind);
		return -1;
	}

	/*
	 * message length (v3 only)
	 */
	if (protoMajor == PROTO_MAJOR_V3 && pool_read_message_length(cp) < 0)
	{
		pool_error("Failed to read the authentication packet length. \
This is likely caused by the inconsistency of auth method among DB nodes. \
In this case you can check the previous error messages (hint: length field) \
from pool_read_message_length and recheck the pg_hba.conf settings.");
		return -1;
	}

	/*
	 * read authentication request kind.
	 *
	 * 0: authentication ok
	 * 1: kerberos v4
	 * 2: kerberos v5
	 * 3: clear text password
	 * 4: crypt password
	 * 5: md5 password
	 * 6: scm credential
	 *
	 * in replication mode, we only support  kind = 0, 3. this is because to "salt"
	 * cannot be replicated.
	 * in non replication mode, we support kind = 0, 3, 4, 5
	 */

	authkind = pool_read_int(cp);
	if (authkind < 0)
	{
		pool_error("pool_do_auth: read auth kind failed");
		return -1;
	}

	authkind = ntohl(authkind);

	/* trust? */
	if (authkind == 0)
	{
		int msglen;

		pool_write(frontend, "R", 1);

		if (protoMajor == PROTO_MAJOR_V3)
		{
			msglen = htonl(8);
			pool_write(frontend, &msglen, sizeof(msglen));
		}

		msglen = htonl(0);
		if (pool_write_and_flush(frontend, &msglen, sizeof(msglen)) < 0)
		{
			return -1;
		}
		MASTER(cp)->auth_kind = 0;
	}

	/* clear text password authentication? */
	else if (authkind == 3)
	{
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i))
				continue;

			pool_debug("trying clear text password authentication");

			authkind = do_clear_text_password(CONNECTION(cp, i), frontend, 0, protoMajor);

			if (authkind < 0)
			{
				pool_debug("do_clear_text_password failed in slot %d", i);
				pool_send_auth_fail(frontend, cp);
				return -1;
			}
		}
	}

	/* crypt authentication? */
	else if (authkind == 4)
	{
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i))
				continue;

			pool_debug("trying crypt authentication");

			authkind = do_crypt(CONNECTION(cp, i), frontend, 0, protoMajor);

			if (authkind < 0)
			{
				pool_debug("do_crypt_text_password failed in slot %d", i);
				pool_send_auth_fail(frontend, cp);
				return -1;
			}
		}
	}

	/* md5 authentication? */
	else if (authkind == 5)
	{
		if (!RAW_MODE && NUM_BACKENDS > 1)
		{
			pool_send_error_message(frontend, protoMajor, AUTHFAIL_ERRORCODE,
									"MD5 authentication is unsupported in replication, master-slave and parallel modes.",
									"",
									"check pg_hba.conf",
									__FILE__, __LINE__);
			return -1;
		}

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i))
				continue;

			pool_debug("trying md5 authentication");

			authkind = do_md5(CONNECTION(cp, i), frontend, 0, protoMajor);

			if (authkind < 0)
			{
				pool_debug("do_md5failed in slot %d", i);
				pool_send_auth_fail(frontend, cp);
				return -1;
			}
		}
	}

	else
	{
		pool_error("pool_do_auth: unsupported auth kind received: %d", authkind);
		return -1;
	}

	if (authkind != 0)
	{
		pool_error("pool_do_auth: unknown authentication response from backend %d", authkind);
		return -1;
	}

	/*
	 * authentication ok. now read pid and secret key from the
	 * backend
	 */
	for (;;)
	{
		kind = pool_read_kind(cp);
		if (kind < 0)
		{
			pool_error("pool_do_auth: failed to read kind before BackendKeyData");
			return -1;
		}
		else if (kind == 'K')
			break;

		if (protoMajor == PROTO_MAJOR_V3)
		{
			switch (kind)
			{
				case 'S':
					/* process parameter status */
					if (ParameterStatus(frontend, cp) != POOL_CONTINUE)
						return -1;
					pool_flush(frontend);
					break;

				case 'N':
					/* process notice message */
					if (SimpleForwardToFrontend(kind, frontend, cp))
						return -1;
					pool_flush(frontend);
					break;

					/* process error message */
				case 'E':
					SimpleForwardToFrontend(kind, frontend, cp);
					pool_flush(frontend);
					return -1;
					break;

				default:
					pool_error("pool_do_auth: unknown response \"%c\" before processing BackendKeyData",
							   kind);
					return -1;
					break;
			}
		}
		else
		{
			/* V2 case */
			switch (kind)
			{
				case 'N':
					/* process notice message */
					if (NoticeResponse(frontend, cp) != POOL_CONTINUE)
						return -1;
					break;

					/* process error message */
				case 'E':
					ErrorResponse(frontend, cp);
					return -1;
					break;

				default:
					pool_error("pool_do_auth: unknown response \"%c\" before processing V2 BackendKeyData",
							   kind);
					return -1;
					break;
			}
		}
	}

	/*
	 * message length (V3 only)
	 */
	if (protoMajor == PROTO_MAJOR_V3)
	{
		if (kind != 'K')
		{
			pool_error("pool_do_auth: expect \"K\" got %c", kind);
			return -1;
		}

		if ((length = pool_read_message_length(cp)) != 12)
		{
			pool_error("pool_do_auth: invalid messages length(%d) for BackendKeyData", length);
			return -1;
		}
	}

	/*
	 * OK, read pid and secret key
	 */
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			/* read pid */
			if (pool_read(CONNECTION(cp, i), &pid, sizeof(pid)) < 0)
			{
				pool_error("pool_do_auth: failed to read pid in slot %d", i);
				return -1;
			}

			CONNECTION_SLOT(cp, i)->pid = cp->info[i].pid = pid;

			/* read key */
			if (pool_read(CONNECTION(cp, i), &key, sizeof(key)) < 0)
			{
				pool_error("pool_do_auth: failed to read key in slot %d", i);
				return -1;
			}
			CONNECTION_SLOT(cp, i)->key = cp->info[i].key = key;

		}
	}

	sp = MASTER_CONNECTION(cp)->sp;
	cp->info->major = sp->major;
	cp->info->minor = sp->minor;
	strncpy(cp->info->database, sp->database, sizeof(cp->info->database) - 1);
	strncpy(cp->info->user, sp->user, sizeof(cp->info->user) - 1);
	cp->info->counter = 1;

	return pool_send_auth_ok(frontend, pid, key, protoMajor);
}

/*
* do re-authentication for reused connection. if success return 0 otherwise non 0.
*/
int pool_do_reauth(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *cp)
{
	int status;
	int protoMajor;

	protoMajor = MAJOR(cp);

	switch(MASTER(cp)->auth_kind)
	{
		case 0:
			/* trust */
			status = 0;
			break;

		case 3:
			/* clear text password */
			status = do_clear_text_password(MASTER(cp), frontend, 1, protoMajor);
			break;

		case 4:
			/* crypt password */
			status = do_crypt(MASTER(cp), frontend, 1, protoMajor);
			break;

		case 5:
			/* md5 password */
			status = do_md5(MASTER(cp), frontend, 1, protoMajor);
			break;

		default:
			pool_error("pool_do_reauth: unknown authentication request code %d",
					   MASTER(cp)->auth_kind);
			return -1;
	}

	if (status == 0)
	{
		int msglen;

		pool_write(frontend, "R", 1);

		if (protoMajor == PROTO_MAJOR_V3)
		{
			msglen = htonl(8);
			pool_write(frontend, &msglen, sizeof(msglen));
		}

		msglen = htonl(0);
		if (pool_write_and_flush(frontend, &msglen, sizeof(msglen)) < 0)
		{
			return -1;
		}
	}
	else
	{
		pool_debug("pool_do_reauth: authentication failed");
		pool_send_auth_fail(frontend, cp);               
		return -1;
	}

	return (pool_send_auth_ok(frontend, MASTER_CONNECTION(cp)->pid, MASTER_CONNECTION(cp)->key, protoMajor) != POOL_CONTINUE);
}

/*
* send authentication failure message text to frontend
*/
static void pool_send_auth_fail(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *cp)
{
	int messagelen;
	char *errmessage;
	int protoMajor;

	bool send_error_to_frontend = true;

	protoMajor = MAJOR(cp);

	messagelen = strlen(MASTER_CONNECTION(cp)->sp->user) + 100;
	if ((errmessage = (char *)malloc(messagelen+1)) == NULL)
	{
		pool_error("pool_send_auth_fail_failed: malloc failed: %s", strerror(errno));
		child_exit(1);
	}

	snprintf(errmessage, messagelen, "password authentication failed for user \"%s\"",
		 MASTER_CONNECTION(cp)->sp->user);
	if (send_error_to_frontend)
	pool_send_fatal_message(frontend, protoMajor, "XX000", errmessage,
				"", "", __FILE__, __LINE__);
	free(errmessage);
} 

/*
* send authentication ok to frontend. if success return 0 otherwise non 0.
*/
static POOL_STATUS pool_send_auth_ok(POOL_CONNECTION *frontend, int pid, int key, int protoMajor)
{
	char kind;
	int len;

#ifdef NOT_USED
	if (protoMajor == PROTO_MAJOR_V2)
	{
		/* return "Authentication OK" to the frontend */
		kind = 'R';
		pool_write(frontend, &kind, 1);
		len = htonl(0);
		if (pool_write_and_flush(frontend, &len, sizeof(len)) < 0)
		{
			return -1;
		}
	}
#endif

	/* send backend key data */
	kind = 'K';
	pool_write(frontend, &kind, 1);
	if (protoMajor == PROTO_MAJOR_V3)
	{
		len = htonl(12);
		pool_write(frontend, &len, sizeof(len));
	}

	pool_debug("pool_send_auth_ok: send pid %d to frontend", ntohl(pid));

	pool_write(frontend, &pid, sizeof(pid));
	if (pool_write_and_flush(frontend, &key, sizeof(key)) < 0)
	{
		return -1;
	}

	return 0;
}

/*
 * perform clear text password authetication
 */
static int do_clear_text_password(POOL_CONNECTION *backend, POOL_CONNECTION *frontend, int reauth, int protoMajor)
{
	static int size;
	static char password[MAX_PASSWORD_SIZE];
	char response;
	int kind;
	int len;

	/* master? */
	if (IS_MASTER_NODE_ID(backend->db_node_id))
	{
		pool_write(frontend, "R", 1);	/* authenticaton */
		if (protoMajor == PROTO_MAJOR_V3)
		{
			len = htonl(8);
			pool_write(frontend, &len, sizeof(len));
		}
		kind = htonl(3);		/* clear text password authentication */
		pool_write_and_flush(frontend, &kind, sizeof(kind));	/* indicating clear text password authentication */

		/* read password packet */
		if (protoMajor == PROTO_MAJOR_V2)
		{
			if (pool_read(frontend, &size, sizeof(size)))
			{
				pool_debug("do_clear_text_password: failed to read password packet size");
				return -1;
			}
		}
		else
		{
			char k;

			if (pool_read(frontend, &k, sizeof(k)))
			{
				pool_debug("do_clear_text_password: failed to read password packet \"p\"");
				return -1;
			}
			if (k != 'p')
			{
				pool_error("do_clear_text_password: password packet does not start with \"p\"");
				return -1;
			}
			if (pool_read(frontend, &size, sizeof(size)))
			{
				pool_error("do_clear_text_password: failed to read password packet size");
				return -1;
			}
		}

		if ((ntohl(size) - 4) > sizeof(password))
		{
			pool_error("do_clear_text_password: password is too long (size: %d)", ntohl(size) - 4);
			return -1;
		}

		if (pool_read(frontend, password, ntohl(size) - 4))
		{
			pool_error("do_clear_text_password: failed to read password (size: %d)", ntohl(size) - 4);
			return -1;
		}
	}

	/* connection reusing? */
	if (reauth)
	{
		if ((ntohl(size) - 4) != backend->pwd_size)
		{
			pool_debug("do_clear_text_password; password size does not match in re-authetication");
			return -1;
		}

		if (memcmp(password, backend->password, backend->pwd_size) != 0)
		{
			pool_debug("do_clear_text_password; password does not match in re-authetication");
			return -1;
		}

		return 0;
	}

	/* send password packet to backend */
	if (protoMajor == PROTO_MAJOR_V3)
		pool_write(backend, "p", 1);
	pool_write(backend, &size, sizeof(size));
	pool_write_and_flush(backend, password, ntohl(size) -4);
	if (pool_read(backend, &response, sizeof(response)))
	{
		pool_error("do_clear_text_password: failed to read authentication response");
		return -1;
	}

	if (response != 'R')
	{
		pool_debug("do_clear_text_password: backend does not return R while processing clear text password authentication");
		return -1;
	}

	if (protoMajor == PROTO_MAJOR_V3)
	{
		if (pool_read(backend, &len, sizeof(len)))
		{
			pool_error("do_clear_text_password: failed to read authentication packet size");
			return -1;
		}

		if (ntohl(len) != 8)
		{
			pool_error("do_clear_text_password: incorrect authentication packet size (%d)", ntohl(len));
			return -1;
		}
	}

	/* expect to read "Authentication OK" response. kind should be 0... */
	if (pool_read(backend, &kind, sizeof(kind)))
	{
		pool_debug("do_clear_text_password: failed to read Authentication OK response");
		return -1;
	}

	/* if authenticated, save info */
	if (!reauth && kind == 0)
	{
		if (IS_MASTER_NODE_ID(backend->db_node_id))
		{
			int msglen;

			pool_write(frontend, "R", 1);

			if (protoMajor == PROTO_MAJOR_V3)
			{
				msglen = htonl(8);
				pool_write(frontend, &msglen, sizeof(msglen));
			}

			msglen = htonl(0);
			if (pool_write_and_flush(frontend, &msglen, sizeof(msglen)) < 0)
			{
				return -1;
			}
		}

		backend->auth_kind = 3;
		backend->pwd_size = ntohl(size) - 4;
		memcpy(backend->password, password, backend->pwd_size);
	}
	return kind;
}

/*
 * perform crypt authetication
 */
static int do_crypt(POOL_CONNECTION *backend, POOL_CONNECTION *frontend, int reauth, int protoMajor)
{
	char salt[2];
	static int size;
	static char password[MAX_PASSWORD_SIZE];
	char response;
	int kind;
	int len;

	if (!reauth)
	{
		/* read salt */
		if (pool_read(backend, salt, sizeof(salt)))
		{
			pool_error("do_crypt: failed to read salt");
			return -1;
		}
	}
	else
	{
		memcpy(salt, backend->salt, sizeof(salt));
	}

	/* master? */
	if (IS_MASTER_NODE_ID(backend->db_node_id))
	{
		pool_write(frontend, "R", 1);	/* authenticaton */
		if (protoMajor == PROTO_MAJOR_V3)
		{
			len = htonl(10);
			pool_write(frontend, &len, sizeof(len));
		}
		kind = htonl(4);		/* crypt authentication */
		pool_write(frontend, &kind, sizeof(kind));	/* indicating crypt authentication */
		pool_write_and_flush(frontend, salt, sizeof(salt));		/* salt */

		/* read password packet */
		if (protoMajor == PROTO_MAJOR_V2)
		{
			if (pool_read(frontend, &size, sizeof(size)))
			{
				pool_error("do_crypt: failed to read password packet size");
				return -1;
			}
		}
		else
		{
			char k;

			if (pool_read(frontend, &k, sizeof(k)))
			{
				pool_debug("do_crypt_password: failed to read password packet \"p\"");
				return -1;
			}
			if (k != 'p')
			{
				pool_error("do_crypt_password: password packet does not start with \"p\"");
				return -1;
			}
			if (pool_read(frontend, &size, sizeof(size)))
			{
				pool_error("do_crypt_password: failed to read password packet size");
				return -1;
			}
		}

		if ((ntohl(size) - 4) > sizeof(password))
		{
			pool_error("do_crypt: password is too long(size: %d)", ntohl(size) - 4);
			return -1;
		}

		if (pool_read(frontend, password, ntohl(size) - 4))
		{
			pool_error("do_crypt: failed to read password (size: %d)", ntohl(size) - 4);
			return -1;
		}
	}

	/* connection reusing? */
	if (reauth)
	{
		pool_debug("size: %d saved_size: %d", (ntohl(size) - 4), backend->pwd_size);
		if ((ntohl(size) - 4) != backend->pwd_size)
		{
			pool_debug("do_crypt: password size does not match in re-authentication");
			return -1;
		}

		if (memcmp(password, backend->password, backend->pwd_size) != 0)
		{
			pool_debug("do_crypt: password does not match in re-authentication");
			return -1;
		}

		return 0;
	}

	/* send password packet to backend */
	if (protoMajor == PROTO_MAJOR_V3)
		pool_write(backend, "p", 1);
	pool_write(backend, &size, sizeof(size));
	pool_write_and_flush(backend, password, ntohl(size) -4);
	if (pool_read(backend, &response, sizeof(response)))
	{
		pool_error("do_crypt: failed to read authentication response");
		return -1;
	}

	if (response != 'R')
	{
		pool_debug("do_crypt: backend does not return R while processing crypt authentication(%02x) DB node id: %d", response, backend->db_node_id);
		return -1;
	}

	if (protoMajor == PROTO_MAJOR_V3)
	{
		if (pool_read(backend, &len, sizeof(len)))
		{
			pool_error("do_crypt: failed to read authentication packet size");
			return -1;
		}

		if (ntohl(len) != 8)
		{
			pool_error("do_crypt: incorrect authentication packet size (%d)", ntohl(len));
			return -1;
		}
	}

	/* expect to read "Authentication OK" response. kind should be 0... */
	if (pool_read(backend, &kind, sizeof(kind)))
	{
		pool_debug("do_crypt: failed to read Authentication OK response");
		return -1;
	}

	/* if authenticated, save info */
	if (!reauth && kind == 0)
	{
		int msglen;

		pool_write(frontend, "R", 1);

		if (protoMajor == PROTO_MAJOR_V3)
		{
			msglen = htonl(8);
			pool_write(frontend, &msglen, sizeof(msglen));
		}

		msglen = htonl(0);
		if (pool_write_and_flush(frontend, &msglen, sizeof(msglen)) < 0)
		{
			return -1;
		}

		backend->auth_kind = 4;
		backend->pwd_size = ntohl(size) - 4;
		memcpy(backend->password, password, backend->pwd_size);
		memcpy(backend->salt, salt, sizeof(salt));
	}
	return kind;
}

/*
 * perform MD5 authetication
 */
static int do_md5(POOL_CONNECTION *backend, POOL_CONNECTION *frontend, int reauth, int protoMajor)
{
	char salt[4];
	static int size;
	static char password[MAX_PASSWORD_SIZE];
	char response;
	int kind;
	int len;

	if (!reauth)
	{
		/* read salt */
		if (pool_read(backend, salt, sizeof(salt)))
		{
			pool_error("do_md5: failed to read salt");
			return -1;
		}
		pool_debug("DB node id: %d salt: %hhx%hhx%hhx%hhx", backend->db_node_id,
				   salt[0], salt[1], salt[2], salt[3]);
	}
	else
	{
		memcpy(salt, backend->salt, sizeof(salt));
	}

	/* master? */
	if (IS_MASTER_NODE_ID(backend->db_node_id))
	{
		pool_write(frontend, "R", 1);	/* authenticaton */
		if (protoMajor == PROTO_MAJOR_V3)
		{
			len = htonl(12);
			pool_write(frontend, &len, sizeof(len));
		}
		kind = htonl(5);
		pool_write(frontend, &kind, sizeof(kind));	/* indicating MD5 */
		pool_write_and_flush(frontend, salt, sizeof(salt));		/* salt */

		/* read password packet */
		if (protoMajor == PROTO_MAJOR_V2)
		{
			if (pool_read(frontend, &size, sizeof(size)))
			{
				pool_error("do_md5: failed to read password packet size");
				return -1;
			}
		}
		else
		{
			char k;

			if (pool_read(frontend, &k, sizeof(k)))
			{
				pool_debug("do_md5_password: failed to read password packet \"p\"");
				return -1;
			}
			if (k != 'p')
			{
				pool_error("do_md5_password: password packet does not start with \"p\"");
				return -1;
			}
			if (pool_read(frontend, &size, sizeof(size)))
			{
				pool_error("do_md5_password: failed to read password packet size");
				return -1;
			}
		}

		if ((ntohl(size) - 4) > sizeof(password))
		{
			pool_error("do_md5: password is too long(size: %d)", ntohl(size) - 4);
			return -1;
		}

		if (pool_read(frontend, password, ntohl(size) - 4))
		{
			pool_error("do_md5: failed to read password (size: %d)", ntohl(size) - 4);
			return -1;
		}
	}

	/* connection reusing? */
	if (reauth)
	{
		if ((ntohl(size) - 4) != backend->pwd_size)
		{
			pool_debug("do_md5; password size does not match in re-authentication");
			return -1;
		}

		if (memcmp(password, backend->password, backend->pwd_size) != 0)
		{
			pool_debug("do_md5; password does not match in re-authentication");
			return -1;
		}

		return 0;
	}

	/* send password packet to backend */
	if (protoMajor == PROTO_MAJOR_V3)
		pool_write(backend, "p", 1);
	pool_write(backend, &size, sizeof(size));
	pool_write_and_flush(backend, password, ntohl(size) -4);
	if (pool_read(backend, &response, sizeof(response)))
	{
		pool_error("do_md5: failed to read authentication response");
		return -1;
	}

	if (response != 'R')
	{
		pool_debug("do_md5: backend does not return R while processing MD5 authentication %c", response);
		return -1;
	}

	if (protoMajor == PROTO_MAJOR_V3)
	{
		if (pool_read(backend, &len, sizeof(len)))
		{
			pool_error("do_md5: failed to read authentication packet size");
			return -1;
		}

		if (ntohl(len) != 8)
		{
			pool_error("do_md5: incorrect authentication packet size (%d)", ntohl(len));
			return -1;
		}
	}

	/* expect to read "Authentication OK" response. kind should be 0... */
	if (pool_read(backend, &kind, sizeof(kind)))
	{
		pool_debug("do_md5: failed to read Authentication OK response");
		return -1;
	}

	/* if authenticated, save info */
	if (!reauth && kind == 0)
	{
		int msglen;

		pool_write(frontend, "R", 1);

		if (protoMajor == PROTO_MAJOR_V3)
		{
			msglen = htonl(8);
			pool_write(frontend, &msglen, sizeof(msglen));
		}

		msglen = htonl(0);
		if (pool_write_and_flush(frontend, &msglen, sizeof(msglen)) < 0)
		{
			return -1;
		}

		backend->auth_kind = 5;
		backend->pwd_size = ntohl(size) - 4;
		memcpy(backend->password, password, backend->pwd_size);
		memcpy(backend->salt, salt, sizeof(salt));
	}
	return kind;
}

/*
 * read message length (V3 only)
 */
int pool_read_message_length(POOL_CONNECTION_POOL *cp)
{
	int status;
	int length, length0;
	int i;

	/* read message from master node */
	status = pool_read(CONNECTION(cp, MASTER_NODE_ID), &length0, sizeof(length0));
	if (status < 0)
	{
		pool_error("pool_read_message_length: error while reading message length in slot %d", MASTER_NODE_ID);
		return -1;
	}
	length0 = ntohl(length0);
	pool_debug("pool_read_message_length: slot: %d length: %d", MASTER_NODE_ID, length0);

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i) || IS_MASTER_NODE_ID(i))
		{
			continue;
		}

		status = pool_read(CONNECTION(cp, i), &length, sizeof(length));
		if (status < 0)
		{
			pool_error("pool_read_message_length: error while reading message length in slot %d", i);
			return -1;
		}

		length = ntohl(length);
		pool_debug("pool_read_message_length: slot: %d length: %d", i, length);

		if (length != length0)
		{
			pool_error("pool_read_message_length: message length (%d) in slot %d does not match with slot 0(%d)", length, i, length0);
			return -1;
		}
	}

	if (length0 < 0)
	{
		pool_error("pool_read_message_length: invalid message length (%d)", length);
		return -1;
	}

	return length0;
}

/*
 * read message length2 (V3 only)
 * unlike pool_read_message_length, this returns an array of message length.
 * The array is in the static storage, thus it will be destroyed by subsequent calls.
 */
int *pool_read_message_length2(POOL_CONNECTION_POOL *cp)
{
	int status;
	int length, length0;
	int i;
	static int length_array[MAX_CONNECTION_SLOTS];

	/* read message from master node */
	status = pool_read(CONNECTION(cp, MASTER_NODE_ID), &length0, sizeof(length0));
	if (status < 0)
	{
		pool_error("pool_read_message_length2: error while reading message length in slot %d", MASTER_NODE_ID);
		return NULL;
	}

	length0 = ntohl(length0);
	length_array[MASTER_NODE_ID] = length0;
	pool_debug("pool_read_message_length2: master slot: %d length: %d", MASTER_NODE_ID, length0);
	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
		{
			status = pool_read(CONNECTION(cp, i), &length, sizeof(length));
			if (status < 0)
			{
				pool_error("pool_read_message_length2: error while reading message length in slot %d", i);
				return NULL;
			}

			length = ntohl(length);
			pool_debug("pool_read_message_length2: master slot: %d length: %d", i, length);

			if (length != length0)
			{
				pool_log("pool_read_message_length2: message length (%d) in slot %d does not match with slot 0(%d)", length, i, length0);
			}

			if (length < 0)
			{
				pool_error("pool_read_message_length2: invalid message length (%d)", length);
				return NULL;
			}

			length_array[i] = length;
		}

	}
	return &length_array[0];
}

signed char pool_read_kind(POOL_CONNECTION_POOL *cp)
{
	int status;
	char kind0, kind;
	int i;

	kind0 = 0;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
		{
			continue;
		}

		status = pool_read(CONNECTION(cp, i), &kind, sizeof(kind));
		if (status < 0)
		{
			pool_error("pool_read_kind: error while reading message kind");
			return -1;
		}

		if (IS_MASTER_NODE_ID(i))
		{
			kind0 = kind;
		}
		else
		{
			if (kind != kind0)
			{
				pool_error("pool_read_kind: kind does not match between master(%x) slot[%d] (%x)",
						   kind0, i, kind);
				return -1;
			}
		}
	}

	return kind;
}

int pool_read_int(POOL_CONNECTION_POOL *cp)
{
	int status;
	int data0, data;
	int i;

	data0 = 0;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
		{
			continue;
		}

		status = pool_read(CONNECTION(cp, i), &data, sizeof(data));
		if (status < 0)
		{
			pool_error("pool_read_int: error while reading message data");
			return -1;
		}

		if (IS_MASTER_NODE_ID(i))
		{
			data0 = data;
		}
		else
		{
			if (data != data0)
			{
				pool_error("pool_read_int: data does not match between between master(%x) slot[%d] (%x)",
						   data0, i, data);
				return -1;
			}
		}
	}

	return data;
}
