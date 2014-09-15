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
 * pool_auth.c: authentication stuff
 *
*/

#include "pool.h"
#include "utils/pool_stream.h"
#include "pool_config.h"
#include "auth/pool_passwd.h"
#include "utils/elog.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
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

static POOL_STATUS pool_send_backend_key_data(POOL_CONNECTION *frontend, int pid, int key, int protoMajor);
static int do_clear_text_password(POOL_CONNECTION *backend, POOL_CONNECTION *frontend, int reauth, int protoMajor);
static void pool_send_auth_fail(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *cp);
static int do_crypt(POOL_CONNECTION *backend, POOL_CONNECTION *frontend, int reauth, int protoMajor);
static int do_md5(POOL_CONNECTION *backend, POOL_CONNECTION *frontend, int reauth, int protoMajor);
static int send_md5auth_request(POOL_CONNECTION *frontend, int protoMajor, char *salt);
static int read_password_packet(POOL_CONNECTION *frontend, int protoMajor, 	char *password, int *pwdSize);
static int send_password_packet(POOL_CONNECTION *backend, int protoMajor, char *password);
static int send_auth_ok(POOL_CONNECTION *frontend, int protoMajor);

/*
 * After sending the start up packet to the backend, do the
 * authentication against backend. if success return 0 otherwise non
 * 0.
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
		ereport(ERROR,
			(errmsg("invalid authentication packet from backend"),
				errdetail("failed to get response kind")));

	/* error response? */
	if (kind == 'E')
	{
		/* we assume error response at this stage is likely version
		 * protocol mismatch (v3 frontend vs. v2 backend). So we throw
		 * a V2 protocol error response in the hope that v3 frontend
		 * will negotiate again using v2 protocol.
		 */

		ErrorResponse(frontend, cp);

		ereport(ERROR,
			(errmsg("backend authentication failed"),
				errdetail("backend response with kind \'E\' when expecting \'R\'"),
					errhint("This issue can be caused by version mismatch (current version %d)", protoMajor)));
	}
	else if (kind != 'R')
		ereport(ERROR,
			(errmsg("backend authentication failed"),
				errdetail("backend response with kind \'%c\' when expecting \'R\'",kind)));

	/*
	 * message length (v3 only)
	 */
	if (protoMajor == PROTO_MAJOR_V3 && pool_read_message_length(cp) < 0)
		ereport(ERROR,
			(errmsg("invalid authentication packet from backend"),
				errdetail("failed to get the authentication packet length"),
					errhint("This is likely caused by the inconsistency of auth method among DB nodes. \
							Please check the previous error messages (hint: length field) \
							from pool_read_message_length and recheck the pg_hba.conf settings.")));


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
	 * in replication mode, we only support  kind = 0, 3. this is because "salt"
	 * cannot be replicated.
	 * in non replication mode, we support kind = 0, 3, 4, 5
	 */

	authkind = pool_read_int(cp);
	if (authkind < 0)
		ereport(ERROR,
			(errmsg("invalid authentication packet from backend"),
				errdetail("failed to get auth kind")));

	authkind = ntohl(authkind);

	ereport(DEBUG1,
		(errmsg("authentication backend"),
			 errdetail("auth kind:%d", authkind)));

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
		pool_write_and_flush(frontend, &msglen, sizeof(msglen));
		MASTER(cp)->auth_kind = 0;
	}

	/* clear text password authentication? */
	else if (authkind == 3)
	{
		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i))
				continue;

			ereport(DEBUG1,
				(errmsg("authentication backend"),
					 errdetail("trying clear text password authentication")));

			authkind = do_clear_text_password(CONNECTION(cp, i), frontend, 0, protoMajor);

			if (authkind < 0)
			{
				ereport(DEBUG1,
					(errmsg("authentication backend"),
						 errdetail("clear text password failed in slot %d", i)));
				pool_send_auth_fail(frontend, cp);
				ereport(ERROR,
					(errmsg("failed to authenticate with backend"),
						errdetail("do_clear_text_password failed in slot %d", i)));
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

			ereport(DEBUG1,
				(errmsg("authentication backend"),
					 errdetail("trying crypt authentication")));

			authkind = do_crypt(CONNECTION(cp, i), frontend, 0, protoMajor);

			if (authkind < 0)
			{
				pool_send_auth_fail(frontend, cp);
				ereport(ERROR,
					(errmsg("failed to authenticate with backend"),
						errdetail("do_crypt_text_password failed in slot %d", i)));
			}
		}
	}

	/* md5 authentication? */
	else if (authkind == 5)
	{
		/* If MD5 auth is not active in pool_hba.conf, it cannot be
		 * used with other than raw mode.
		 */
		if (frontend->auth_method != uaMD5 && !RAW_MODE && NUM_BACKENDS > 1)
		{
			pool_send_error_message(frontend, protoMajor, AUTHFAIL_ERRORCODE,
									"MD5 authentication is unsupported in replication, master-slave and parallel modes.",
									"",
									"check pg_hba.conf",
									__FILE__, __LINE__);
			ereport(ERROR,
				(errmsg("failed to authenticate with backend"),
					errdetail("MD5 authentication is not supported in replication, master-slave and parallel modes."),
						errhint("check pg_hba.conf settings on backend node")));
		}

		for (i=0;i<NUM_BACKENDS;i++)
		{
			if (!VALID_BACKEND(i))
				continue;

			ereport(DEBUG1,
				(errmsg("authentication backend"),
					 errdetail("trying md5 authentication")));

			authkind = do_md5(CONNECTION(cp, i), frontend, 0, protoMajor);

			if (authkind < 0)
			{
				pool_send_auth_fail(frontend, cp);
				ereport(ERROR,
					(errmsg("failed to authenticate with backend"),
						errdetail("MD5 authentication failed in slot [%d].",i)));
			}
		}
	}

	else
	{
		ereport(ERROR,
			(errmsg("failed to authenticate with backend"),
				errdetail("unsupported auth kind received from backend: authkind:%d", authkind)));
	}

	if (authkind != 0)
		ereport(ERROR,
			(errmsg("failed to authenticate with backend"),
				errdetail("invalid auth kind received from backend: authkind:%d", authkind)));

	/*
	 * authentication ok. now read pid and secret key from the
	 * backend
	 */
	for (;;)
	{
		char *message = NULL;
		kind = pool_read_kind(cp);

		if (kind < 0)
		{
			ereport(ERROR,
				(errmsg("authentication failed from backend"),
					errdetail("failed to read kind before BackendKeyData")));
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
					if (pool_extract_error_message(false, MASTER(cp), protoMajor, true, &message) == 1)
					{
						ereport(NOTICE,
							(errmsg("notice from backend"),
								errdetail("BACKEND NOTICE: \"%s\"",message)));
					}
					/* process notice message */
					if (SimpleForwardToFrontend(kind, frontend, cp))
						ereport(ERROR,
							(errmsg("authentication failed"),
								errdetail("failed to forward message to frontend")));
					pool_flush(frontend);
					break;

					/* process error message */
				case 'E':

					if (pool_extract_error_message(false, MASTER(cp), protoMajor, true, &message) == 1)
					{
						ereport(LOG,
								(errmsg("backend throws an error message"),
								 errdetail("%s",message)));
					}

					SimpleForwardToFrontend(kind, frontend, cp);

					pool_flush(frontend);

					ereport(ERROR,
						(errmsg("authentication failed, backend node replied with an error"),
							errdetail("SERVER ERROR:\"%s\"",message?message:"could not extract backend message")));

					break;

				default:
					ereport(ERROR,
						(errmsg("authentication failed"),
							errdetail("unknown response \"%c\" before processing BackendKeyData",kind)));
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
					NoticeResponse(frontend, cp);
					break;

					/* process error message */
				case 'E':
					ErrorResponse(frontend, cp);
					/* fallthrough*/
				default:
					ereport(ERROR,
						(errmsg("authentication failed"),
							errdetail("invalid response \"%c\" before processing BackendKeyData",kind)));
					break;
			}
		}
	}

	/*
	 * message length (V3 only)
	 */
	if (protoMajor == PROTO_MAJOR_V3)
	{
		if ((length = pool_read_message_length(cp)) != 12)
		{
			ereport(ERROR,
				(errmsg("authentication failed"),
					errdetail("invalid messages length(%d) for BackendKeyData", length)));
		}
	}

	/*
	 * OK, read pid and secret key
	 */
	sp = MASTER_CONNECTION(cp)->sp;
	pid = -1;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i))
		{
			/* read pid */
			if (pool_read(CONNECTION(cp, i), &pid, sizeof(pid)) < 0)
			{
				ereport(ERROR,
					(errmsg("authentication failed"),
						errdetail("failed to read pid in slot %d", i)));
			}
			ereport(DEBUG1,
				(errmsg("authentication backend"),
					 errdetail("cp->info[i]:%p pid:%u", &cp->info[i], ntohl(pid))));

			CONNECTION_SLOT(cp, i)->pid = cp->info[i].pid = pid;

			/* read key */
			if (pool_read(CONNECTION(cp, i), &key, sizeof(key)) < 0)
			{
				ereport(ERROR,
					(errmsg("authentication failed"),
						errdetail("failed to read key in slot %d", i)));
			}
			CONNECTION_SLOT(cp, i)->key = cp->info[i].key = key;

			cp->info[i].major = sp->major;
			cp->info[i].minor = sp->minor;
			strlcpy(cp->info[i].database, sp->database, sizeof(cp->info[i].database));
			strlcpy(cp->info[i].user, sp->user, sizeof(cp->info[i].user));
			cp->info[i].counter = 1;
		}
	}

	if (pid == -1)
	{
		ereport(ERROR,
                (errmsg("authentication failed"),
                 errdetail("pool_do_auth: all backends are down")));
	}
	if(pool_send_backend_key_data(frontend, pid, key, protoMajor))
		ereport(ERROR,
			(errmsg("authentication failed"),
				errdetail("failed to send backend data to frontend")));
	return 0;
}

/*
* do re-authentication for reused connection. if success return 0 otherwise throws ereport.
*/
int pool_do_reauth(POOL_CONNECTION *frontend, POOL_CONNECTION_POOL *cp)
{
	int protoMajor;
    int msglen;

	protoMajor = MAJOR(cp);

	switch(MASTER(cp)->auth_kind)
	{
		case 0:
			/* trust */
			break;

		case 3:
			/* clear text password */
			do_clear_text_password(MASTER(cp), frontend, 1, protoMajor);
			break;

		case 4:
			/* crypt password */
			do_crypt(MASTER(cp), frontend, 1, protoMajor);
			break;

		case 5:
			/* md5 password */
			do_md5(MASTER(cp), frontend, 1, protoMajor);
			break;

		default:
            ereport(ERROR,
                (errmsg("authentication failed"),
                     errdetail("unknown authentication request code %d",MASTER(cp)->auth_kind)));
	}


    pool_write(frontend, "R", 1);

    if (protoMajor == PROTO_MAJOR_V3)
    {
        msglen = htonl(8);
        pool_write(frontend, &msglen, sizeof(msglen));
    }

    msglen = htonl(0);
    pool_write_and_flush(frontend, &msglen, sizeof(msglen));
	pool_send_backend_key_data(frontend, MASTER_CONNECTION(cp)->pid, MASTER_CONNECTION(cp)->key, protoMajor);
    return 0;
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
	errmessage = (char *)palloc(messagelen+1);

	snprintf(errmessage, messagelen, "password authentication failed for user \"%s\"",
		 MASTER_CONNECTION(cp)->sp->user);
	if (send_error_to_frontend)
	pool_send_fatal_message(frontend, protoMajor, "XX000", errmessage,
				"", "", __FILE__, __LINE__);
	pfree(errmessage);
} 

/*
 * Send backend key data to frontend. if success return 0 otherwise non 0.
 */
static POOL_STATUS pool_send_backend_key_data(POOL_CONNECTION *frontend, int pid, int key, int protoMajor)
{
	char kind;
	int len;

	/* Send backend key data */
	kind = 'K';
	pool_write(frontend, &kind, 1);
	if (protoMajor == PROTO_MAJOR_V3)
	{
		len = htonl(12);
		pool_write(frontend, &len, sizeof(len));
	}
	ereport(DEBUG1,
		(errmsg("sending backend key data"),
			 errdetail("send pid %d to frontend", ntohl(pid))));

	pool_write(frontend, &pid, sizeof(pid));
	pool_write_and_flush(frontend, &key, sizeof(key));

	return 0;
}

/*
 * perform clear text password authentication
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
		pool_write(frontend, "R", 1);	/* authentication */
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
			pool_read(frontend, &size, sizeof(size));
		}
		else
		{
			char k;

			pool_read(frontend, &k, sizeof(k));
			if (k != 'p')
                ereport(ERROR,
                        (errmsg("clear text password authentication failed"),
                         errdetail("invalid password packet. Packet does not starts with \"p\"")));
			pool_read(frontend, &size, sizeof(size));
		}

		if ((ntohl(size) - 4) > sizeof(password))
            ereport(ERROR,
                    (errmsg("clear text password authentication failed"),
                     errdetail("password is too long. password size = %d", ntohl(size) - 4)));

		pool_read(frontend, password, ntohl(size) - 4);
	}

	/* connection reusing? */
	if (reauth)
	{
		if ((ntohl(size) - 4) != backend->pwd_size)
            ereport(ERROR,
                    (errmsg("clear text password authentication failed"),
                     errdetail("password size does not match")));

		if (memcmp(password, backend->password, backend->pwd_size) != 0)
            ereport(ERROR,
                    (errmsg("clear text password authentication failed"),
                     errdetail("password does not match")));

		return 0;
	}

	/* send password packet to backend */
	if (protoMajor == PROTO_MAJOR_V3)
		pool_write(backend, "p", 1);
	pool_write(backend, &size, sizeof(size));
	pool_write_and_flush(backend, password, ntohl(size) -4);
	
    pool_read(backend, &response, sizeof(response));

	if (response != 'R')
        ereport(ERROR,
            (errmsg("clear text password authentication failed"),
                 errdetail("invalid packet from backend. backend does not return R while processing clear text password authentication")));

	if (protoMajor == PROTO_MAJOR_V3)
	{
		pool_read(backend, &len, sizeof(len));

		if (ntohl(len) != 8)
            ereport(ERROR,
                    (errmsg("clear text password authentication failed"),
                     errdetail("invalid packet from backend. incorrect authentication packet size (%d)", ntohl(len))));
	}

	/* expect to read "Authentication OK" response. kind should be 0... */
	pool_read(backend, &kind, sizeof(kind));

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
			pool_write_and_flush(frontend, &msglen, sizeof(msglen));
		}

		backend->auth_kind = 3;
		backend->pwd_size = ntohl(size) - 4;
		memcpy(backend->password, password, backend->pwd_size);
	}
	return kind;
}

/*
 * perform crypt authentication
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
		pool_read(backend, salt, sizeof(salt));
	}
	else
	{
		memcpy(salt, backend->salt, sizeof(salt));
	}

	/* master? */
	if (IS_MASTER_NODE_ID(backend->db_node_id))
	{
		pool_write(frontend, "R", 1);	/* authentication */
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
			pool_read(frontend, &size, sizeof(size));
		}
		else
		{
			char k;

			pool_read(frontend, &k, sizeof(k));
			if (k != 'p')
			{
                ereport(ERROR,
                        (errmsg("crypt authentication failed"),
                         errdetail("invalid password packet. Packet does not starts with \"p\"")));
			}
			pool_read(frontend, &size, sizeof(size));
		}

		if ((ntohl(size) - 4) > sizeof(password))
		{
            ereport(ERROR,
				(errmsg("crypt authentication failed"),
                     errdetail("password is too long, password size is %d", ntohl(size) - 4)));
		}

		pool_read(frontend, password, ntohl(size) - 4);
	}

	/* connection reusing? */
	if (reauth)
	{
		ereport(DEBUG1,
			(errmsg("performing crypt authentication"),
				 errdetail("size: %d saved_size: %d", (ntohl(size) - 4), backend->pwd_size)));

		if ((ntohl(size) - 4) != backend->pwd_size)
            ereport(ERROR,
                    (errmsg("crypt authentication failed"),
                     errdetail("password size does not match")));


		if (memcmp(password, backend->password, backend->pwd_size) != 0)
            ereport(ERROR,
                    (errmsg("crypt authentication failed"),
                     errdetail("password does not match")));

		return 0;
	}

	/* send password packet to backend */
	if (protoMajor == PROTO_MAJOR_V3)
		pool_write(backend, "p", 1);
	pool_write(backend, &size, sizeof(size));
	pool_write_and_flush(backend, password, ntohl(size) -4);
	pool_read(backend, &response, sizeof(response));

	if (response != 'R')
        ereport(ERROR,
                (errmsg("crypt authentication failed"),
                 errdetail("invalid packet from backend. backend does not return R while processing clear text password authentication")));

	if (protoMajor == PROTO_MAJOR_V3)
	{
		pool_read(backend, &len, sizeof(len));

		if (ntohl(len) != 8)
            ereport(ERROR,
                    (errmsg("crypt authentication failed"),
                     errdetail("invalid packet from backend. incorrect authentication packet size (%d)", ntohl(len))));
	}

	/* expect to read "Authentication OK" response. kind should be 0... */
	pool_read(backend, &kind, sizeof(kind));

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
		pool_write_and_flush(frontend, &msglen, sizeof(msglen));

		backend->auth_kind = 4;
		backend->pwd_size = ntohl(size) - 4;
		memcpy(backend->password, password, backend->pwd_size);
		memcpy(backend->salt, salt, sizeof(salt));
	}
	return kind;
}

/*
 * perform MD5 authentication
 */
static int do_md5(POOL_CONNECTION *backend, POOL_CONNECTION *frontend, int reauth, int protoMajor)
{
	char salt[4];
	static int size;
	static char password[MAX_PASSWORD_SIZE];
	int kind;
	char encbuf[POOL_PASSWD_LEN+1];
	char *pool_passwd = NULL;

	if (NUM_BACKENDS > 1)
	{
		/* Read password entry from pool_passwd */
		pool_passwd = pool_get_passwd(frontend->username);
		if (!pool_passwd)
            ereport(ERROR,
				(errmsg("md5 authentication failed"),
                     errdetail("username \"%s\" does not exist in pool_passwd",frontend->username)));


		/* master? */
		if (IS_MASTER_NODE_ID(backend->db_node_id))
		{
			/* Send md5 auth request to frontend with my own salt */
			pool_random_salt(salt);
			send_md5auth_request(frontend, protoMajor, salt);

			/* Read password packet */
			read_password_packet(frontend, protoMajor, password, &size);

			/* Check the password using my salt + pool_passwd */
			pg_md5_encrypt(pool_passwd+strlen("md5"), salt, sizeof(salt), encbuf);
			if (strcmp(password, encbuf))
			{
				/* Password does not match */
                ereport(ERROR,
                    (errmsg("md5 authentication failed"),
                         errdetail("password does not match")));
			}
		}
		kind = 0;

		if (!reauth)
		{
			/*
			 * If ok, authenticate against backends using pool_passwd
			 */
			/* Read salt */
			pool_read(backend, salt, sizeof(salt));

			ereport(DEBUG1,
				(errmsg("performing md5 authentication"),
					errdetail("DB node id: %d salt: %hhx%hhx%hhx%hhx", backend->db_node_id,
						   salt[0], salt[1], salt[2], salt[3])));

			/* Encrypt password in pool_passwd using the salt */
			pg_md5_encrypt(pool_passwd+strlen("md5"), salt, sizeof(salt), encbuf);

			/* Send password packet to backend and receive auth response */
			kind = send_password_packet(backend, protoMajor, encbuf);
			if (kind < 0)
                ereport(ERROR,
                    (errmsg("md5 authentication failed"),
                         errdetail("backend replied with invalid kind")));
		}

		if (!reauth && kind == 0)
		{
			if (IS_MASTER_NODE_ID(backend->db_node_id))
			{
				/* Send auth ok to frontend */
				send_auth_ok(frontend, protoMajor);
			}

			/* Save the auth info */
			backend->auth_kind = 5;
		}
		return kind;
	}

	/*
	 * Followings are NUM_BACKEND == 1 case.
	 */
	if (!reauth)
	{
		/* read salt */
		pool_read(backend, salt, sizeof(salt));
		ereport(DEBUG1,
			(errmsg("performing md5 authentication"),
				errdetail("DB node id: %d salt: %hhx%hhx%hhx%hhx", backend->db_node_id,
					   salt[0], salt[1], salt[2], salt[3])));
	}
	else
	{
		memcpy(salt, backend->salt, sizeof(salt));
	}

	/* master? */
	if (IS_MASTER_NODE_ID(backend->db_node_id))
	{
		/* Send md5 auth request to frontend */
		send_md5auth_request(frontend, protoMajor, salt);

		/* Read password packet */
		read_password_packet(frontend, protoMajor, password, &size);
	}

	/* connection reusing? */
	if (reauth)
	{
		if (size != backend->pwd_size)
            ereport(ERROR,
				(errmsg("md5 authentication failed"),
                     errdetail("password does not match")));

		if (memcmp(password, backend->password, backend->pwd_size) != 0)
            ereport(ERROR,
				(errmsg("md5 authentication failed"),
                     errdetail("password does not match")));

		return 0;
	}

	/* Send password packet to backend and receive auth response */
	kind = send_password_packet(backend, protoMajor, password);
	if (kind < 0)
        ereport(ERROR,
			(errmsg("md5 authentication failed"),
                 errdetail("backend replied with invalid kind")));

	/* If authenticated, reply back to frontend and save info */
	if (!reauth && kind == 0)
	{
		send_auth_ok(frontend, protoMajor);

		backend->auth_kind = 5;
		backend->pwd_size = size;
		memcpy(backend->password, password, backend->pwd_size);
		memcpy(backend->salt, salt, sizeof(salt));
	}
	return kind;
}

/*
 * Send md5 authentication request packet to frontend
 */
static int send_md5auth_request(POOL_CONNECTION *frontend, int protoMajor, char *salt)
{
	int len;
	int kind;

	pool_write(frontend, "R", 1);	/* authentication */
	if (protoMajor == PROTO_MAJOR_V3)
	{
		len = htonl(12);
		pool_write(frontend, &len, sizeof(len));
	}
	kind = htonl(5);
	pool_write(frontend, &kind, sizeof(kind));	/* indicating MD5 */
	pool_write_and_flush(frontend, salt, 4);		/* salt */

	return 0;
}

/*
 * Read password packet from frontend
 */
static int read_password_packet(POOL_CONNECTION *frontend, int protoMajor, 	char *password, int *pwdSize)
{
	int size;

	/* Read password packet */
	if (protoMajor == PROTO_MAJOR_V2)
	{
		pool_read(frontend, &size, sizeof(size));
	}
	else
	{
		char k;

		pool_read(frontend, &k, sizeof(k));
		if (k != 'p')
            ereport(ERROR,
				(errmsg("authentication failed"),
					errdetail("invalid authentication packet. password packet does not start with \"p\"")));

		pool_read(frontend, &size, sizeof(size));
	}

	*pwdSize = ntohl(size) - 4;
	if (*pwdSize > MAX_PASSWORD_SIZE)
	{
        ereport(ERROR,
			(errmsg("authentication failed"),
                 errdetail("invalid authentication packet. password is too long. password length is %d", *pwdSize)));
		/*
		 * We do not read to throw away packet here. Since it is possible that
		 * it's a denial of service attack.
		 */
	}
	else if (*pwdSize <= 0)
        ereport(ERROR,
            (errmsg("authentication failed"),
                 errdetail("invalid authentication packet. invalid password length. password length is %d", *pwdSize)));

	pool_read(frontend, password, *pwdSize);
	
	password[*pwdSize] = '\0';

	return 0;
}

/*
 * Send password packet to backend and receive authentication response
 * packet.  Return value is the last field of authentication
 * response. If it's 0, authentication was successful.
 * "password" must be null-terminated.
 */
static int send_password_packet(POOL_CONNECTION *backend, int protoMajor, char *password)
{
	int size;
	int len;
	int kind;
	char response;

	/* Send password packet to backend */
	if (protoMajor == PROTO_MAJOR_V3)
		pool_write(backend, "p", 1);
	size = htonl(sizeof(size) + strlen(password)+1);
	pool_write(backend, &size, sizeof(size));
	pool_write_and_flush(backend, password, strlen(password)+1);

	pool_read(backend, &response, sizeof(response));

	if (response != 'R')
        ereport(ERROR,
			(errmsg("authentication failed"),
                 errdetail("invalid backend response. Response does not replied with \"R\"")));

	if (protoMajor == PROTO_MAJOR_V3)
	{
		pool_read(backend, &len, sizeof(len));

		if (ntohl(len) != 8)
            ereport(ERROR,
                (errmsg("authentication failed"),
                     errdetail("invalid authentication packet. incorrect authentication packet size (%d)", ntohl(len))));
	}

	/* Expect to read "Authentication OK" response. kind should be 0... */
	pool_read(backend, &kind, sizeof(kind));

	return kind;
}

/*
 * Send auth ok to frontend
 */
static int send_auth_ok(POOL_CONNECTION *frontend, int protoMajor)
{
	int msglen;

	pool_write(frontend, "R", 1);

	if (protoMajor == PROTO_MAJOR_V3)
	{
		msglen = htonl(8);
		pool_write(frontend, &msglen, sizeof(msglen));
	}

	msglen = htonl(0);
	pool_write_and_flush(frontend, &msglen, sizeof(msglen));
	return 0;
}

/*
 * read message length (V3 only)
 */
int pool_read_message_length(POOL_CONNECTION_POOL *cp)
{
	int length, length0;
	int i;

	/* read message from master node */
	pool_read(CONNECTION(cp, MASTER_NODE_ID), &length0, sizeof(length0));
	length0 = ntohl(length0);

	ereport(DEBUG1,
		(errmsg("reading message length"),
			 errdetail("slot: %d length: %d", MASTER_NODE_ID, length0)));

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i) || IS_MASTER_NODE_ID(i))
		{
			continue;
		}

		pool_read(CONNECTION(cp, i), &length, sizeof(length));

		length = ntohl(length);
		ereport(DEBUG1,
			(errmsg("reading message length"),
				 errdetail("slot: %d length: %d", i, length)));

		if (length != length0)
            ereport(ERROR,
                    (errmsg("unable to read message length"),
                     errdetail("message length (%d) in slot %d does not match with slot 0(%d)", length, i, length0)));
            
	}

	if (length0 < 0)
        ereport(ERROR,
                (errmsg("unable to read message length"),
                 errdetail("invalid message length (%d)", length)));

	return length0;
}

/*
 * read message length2 (V3 only)
 * unlike pool_read_message_length, this returns an array of message length.
 * The array is in the static storage, thus it will be destroyed by subsequent calls.
 */
int *pool_read_message_length2(POOL_CONNECTION_POOL *cp)
{
	int length, length0;
	int i;
	static int length_array[MAX_CONNECTION_SLOTS];

	/* read message from master node */
	pool_read(CONNECTION(cp, MASTER_NODE_ID), &length0, sizeof(length0));

	length0 = ntohl(length0);
	length_array[MASTER_NODE_ID] = length0;
	ereport(DEBUG1,
		(errmsg("reading message length"),
			 errdetail("master slot: %d length: %d", MASTER_NODE_ID, length0)));

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (VALID_BACKEND(i) && !IS_MASTER_NODE_ID(i))
		{
			pool_read(CONNECTION(cp, i), &length, sizeof(length));

			length = ntohl(length);
			ereport(DEBUG1,
				(errmsg("reading message length"),
					 errdetail("master slot: %d length: %d", i, length)));

			if (length != length0)
			{
				ereport(LOG,
					(errmsg("reading message length"),
						errdetail("message length (%d) in slot %d does not match with slot 0(%d)", length, i, length0)));
			}

			if (length < 0)
			{
                ereport(ERROR,
					(errmsg("unable to read message length"),
                         errdetail("invalid message length (%d)", length)));
			}

			length_array[i] = length;
		}

	}
	return &length_array[0];
}

signed char pool_read_kind(POOL_CONNECTION_POOL *cp)
{
	char kind0, kind;
	int i;

	kind = -1;
	kind0 = 0;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
		{
			continue;
		}

		pool_read(CONNECTION(cp, i), &kind, sizeof(kind));

		if (IS_MASTER_NODE_ID(i))
		{
			kind0 = kind;
		}
		else
		{
			if (kind != kind0)
			{
				char *message;

				if (kind0 == 'E')
				{
					if (pool_extract_error_message(false, MASTER(cp), MAJOR(cp), true, &message) == 1)
					{
                        ereport(LOG,
                                (errmsg("pool_read_kind: error message from master backend:%s", message)));
						pfree(message);
					}
				}
				else if (kind == 'E')
				{
					if (pool_extract_error_message(false, CONNECTION(cp, i), MAJOR(cp), true, &message) == 1)
					{
                        ereport(LOG,
                                (errmsg("pool_read_kind: error message from %d th backend:%s", i, message)));
						pfree(message);
					}
				}
                ereport(ERROR,
                    (errmsg("unable to read message kind"),
                         errdetail("kind does not match between master(%x) slot[%d] (%x)",kind0, i, kind)));
			}
		}
	}

	return kind;
}

int pool_read_int(POOL_CONNECTION_POOL *cp)
{
	int data0, data;
	int i;

	data = -1;
	data0 = 0;

	for (i=0;i<NUM_BACKENDS;i++)
	{
		if (!VALID_BACKEND(i))
		{
			continue;
		}
		pool_read(CONNECTION(cp, i), &data, sizeof(data));
		if (IS_MASTER_NODE_ID(i))
		{
			data0 = data;
		}
		else
		{
			if (data != data0)
			{
                ereport(ERROR,
                    (errmsg("unable to read int value"),
                     errdetail("data does not match between between master(%x) slot[%d] (%x)", data0, i, data)));

			}
		}
	}
	return data;
}

/*
 *  pool_random_salt
 */
void pool_random_salt(char *md5Salt)
{
	long rand = random();

	md5Salt[0] = (rand % 255) + 1;
	rand = random();
	md5Salt[1] = (rand % 255) + 1;
	rand = random();
	md5Salt[2] = (rand % 255) + 1;
	rand = random();
	md5Salt[3] = (rand % 255) + 1;
}
