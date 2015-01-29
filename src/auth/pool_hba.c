/* -*-pgsql-c-*- */
/*
 *
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Portions Copyright (c) 2003-2015	PgPool Global Development Group
 * Portions Copyright (c) 1996-2005, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
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
 * pool_hba.c.: Routines to handle host based authentication.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>

#include "pool.h"
#include "utils/pool_path.h"
#include "utils/pool_ip.h"
#include "utils/pool_stream.h"
#include "pool_config.h"
#include "pool_type.h"
#include "utils/palloc.h"
#include "utils/memutils.h"
#include "utils/elog.h"
#include "parser/pg_list.h"
#include "auth/pool_passwd.h"

#define MULTI_VALUE_SEP "\001" /* delimiter for multi-valued column strings */
#define MAX_TOKEN	256

static List *hba_lines = NIL;
static List *hba_line_nums = NIL;
static char *hbaFileName;

static void sendAuthRequest(POOL_CONNECTION *frontend, AuthRequest areq);
static void auth_failed(POOL_CONNECTION *frontend);
static void close_all_backend_connections(void);
static bool hba_getauthmethod(POOL_CONNECTION *frontend);
static bool check_hba(POOL_CONNECTION *frontend);
static void parse_hba(List *line, int line_num, POOL_CONNECTION *frontend, bool *found_p, bool *error_p);
static void parse_hba_auth(ListCell **line_item, UserAuth *userauth_p, char **auth_arg_p, bool *error_p);
static bool check_user(char *user, char *param_str);
static bool check_db(char *dbname, char *user, char *param_str);
static void free_lines(List **lines, List **line_nums);
static void tokenize_file(const char *filename, FILE *file, List **lines, List **line_nums);
static char *tokenize_inc_file(const char *outer_filename, const char *inc_filename);
static bool pg_isblank(const char c);
static void next_token(FILE *fp, char *buf, int bufsz);
static char * next_token_expand(const char *filename, FILE *file);
static POOL_STATUS CheckMd5Auth(char *username);

#ifdef USE_PAM
#ifdef HAVE_PAM_PAM_APPL_H
#include <pam/pam_appl.h>
#endif
#ifdef HAVE_SECURITY_PAM_APPL_H
#include <security/pam_appl.h>
#endif

#define PGPOOL_PAM_SERVICE "pgpool" /* Service name passed to PAM */

static POOL_STATUS CheckPAMAuth(POOL_CONNECTION *frontend, char *user, char *password);
static int pam_passwd_conv_proc(int num_msg, const struct pam_message ** msg, struct pam_response ** resp, void *appdata_ptr);
/*
 * recv_password_packet is usually used with authentications that require a client
 * password. However, pgpool's hba function only uses it for PAM authentication,
 * so declare a prototype here in "#ifdef USE_PAM" to avoid compilation warning.
 */
static char *recv_password_packet(POOL_CONNECTION *frontend);

static struct pam_conv pam_passw_conv = {
	&pam_passwd_conv_proc,
	NULL
};

static char *pam_passwd = NULL;	/* Workaround for Solaris 2.6 brokenness */
static POOL_CONNECTION *pam_frontend_kludge; /* Workaround for passing
											  * POOL_CONNECTION *frontend
											  * into pam_passwd_conv_proc */
#endif /* USE_PAM */


/*
 * read in hba config file
 */
int load_hba(char *hbapath)
{
	FILE *file;
	file = fopen(hbapath, "r");
	if (!file)
	{
		ereport(WARNING,
			(errmsg("failed while loading hba configuration from file:\"%s\"",hbapath),
				 errdetail("fopen failed with error: \"%s\"",strerror(errno))));
		return -1;
	}

	ereport(DEBUG1,
		(errmsg("loading hba configuration"),
			errdetail("loading file :\"%s\" for client authentication configuration file",
				   hbapath)));

	MemoryContext oldContext = MemoryContextSwitchTo(TopMemoryContext);

	if (hba_lines || hba_line_nums)
		free_lines(&hba_lines, &hba_line_nums);

	tokenize_file(hbapath, file, &hba_lines, &hba_line_nums);
	fclose(file);

	hbaFileName = pstrdup(hbapath);

	MemoryContextSwitchTo(oldContext);
	return 0;
}

/*
 * do frontend <-> pgpool authentication based on pool_hba.conf
 */
void ClientAuthentication(POOL_CONNECTION *frontend)
{
	POOL_STATUS status = POOL_END;
	PG_TRY();
    {
        if (! hba_getauthmethod(frontend))
            ereport(FATAL,
                (return_code(2),
                    errmsg("client authentication failed"),
                        errdetail("missing or erroneous pool_hba.conf file"),
                            errhint("see pgpool log for details")));


        switch (frontend->auth_method)
        {
            case uaReject:
            {
                /*
                 * This could have come from an explicit "reject" entry in
                 * pool_hba.conf, but more likely it means there was no matching
                 * entry.  Take pity on the poor user and issue a helpful
                 * error message.  NOTE: this is not a security breach,
                 * because all the info reported here is known at the frontend
                 * and must be assumed known to bad guys. We're merely helping
                 * out the less clueful good guys.
                 */
                char hostinfo[NI_MAXHOST];
                char *errmessage;
                int messagelen;

                getnameinfo_all(&frontend->raddr.addr, frontend->raddr.salen,
                                hostinfo, sizeof(hostinfo),
                                NULL, 0,
                                NI_NUMERICHOST);

                messagelen = sizeof(hostinfo) +
                    strlen(frontend->username) + strlen(frontend->database) + 80;
                errmessage = (char *)palloc(messagelen+1);
#ifdef USE_SSL
                snprintf(errmessage, messagelen+7, /* +7 is for "SSL off" */
                         "no pool_hba.conf entry for host \"%s\", user \"%s\", database \"%s\", %s",
                         hostinfo, frontend->username, frontend->database,
                         frontend->ssl ? "SSL on" : "SSL off");
#else
                snprintf(errmessage, messagelen,
                         "no pool_hba.conf entry for host \"%s\", user \"%s\", database \"%s\"",
                         hostinfo, frontend->username, frontend->database);
#endif
                ereport(FATAL,
                    (return_code(2),
                         errmsg("client authentication failed"),
                            errdetail("%s",errmessage),
                                errhint("see pgpool log for details")));
                break;
            }

            /*case uaKrb4:
            case uaKrb5:
            case uaIdent:
            case uaCrypt:
            case uaPassword:
                break; 
            */

            case uaMD5:
                status = CheckMd5Auth(frontend->username);
                break;


    #ifdef USE_PAM
            case uaPAM:
                pam_frontend_kludge = frontend;
                status = CheckPAMAuth(frontend, frontend->username, "");
                break;
    #endif /* USE_PAM */

            case uaTrust:
                status = POOL_CONTINUE;
                break;
        }
    }
    PG_CATCH();
    {
        close_all_backend_connections();
        PG_RE_THROW();
    }
    PG_END_TRY();

 	if (status == POOL_CONTINUE)
 		sendAuthRequest(frontend, AUTH_REQ_OK);
 	else
		auth_failed(frontend);
}


static void sendAuthRequest(POOL_CONNECTION *frontend, AuthRequest areq)
{
	int wsize;					/* number of bytes to write */
	int areq_nbo;				/* areq in network byte order */

	/*
	 * If AUTH_REQ_OK, then frontend is OK to connect __with_pgpool__.
	 * Do not send 'R' to the frontend, he still needs to authenticate
	 * himself with the backend.
	 */
	if (areq == AUTH_REQ_OK)
		return;

	/* request a password */
	pool_write(frontend, "R", 1);

	if (frontend->protoVersion == PROTO_MAJOR_V3)
	{
/* 		if (areq == AUTH_REQ_MD5) */
/* 			wsize = htonl(sizeof(int)*2+4); */
/* 		else if (areq == AUTH_REQ_CRYPT) */
/* 			wsize = htonl(sizeof(int)*2+2); */
/* 		else */
			wsize = htonl(sizeof(int)*2);
		pool_write(frontend, &wsize, sizeof(int));
	}

	areq_nbo = htonl(areq);
	pool_write(frontend, &areq_nbo, sizeof(int));

	/* Add the salt for encrypted passwords. */
/* 	if (areq == AUTH_REQ_MD5) */
/* 		pq_sendbytes(&buf, port->md5Salt, 4); */
/* 	else if (areq == AUTH_REQ_CRYPT) */
/* 		pq_sendbytes(&buf, port->cryptSalt, 2); */

	pool_flush(frontend);
}


#ifdef USE_PAM					/* see the prototype comment  */

/*
 * Collect password response packet from frontend.
 *
 * Returns NULL if couldn't get password, else palloc'd string.
 */
static char *recv_password_packet(POOL_CONNECTION *frontend)
{
	int rsize;
	char *passwd;
	char *returnVal;

	if (frontend->protoVersion == PROTO_MAJOR_V3)
	{
		/* Expect 'p' message type */
		char kind;

		if (pool_read(frontend, &kind, 1) < 0)
			return NULL;

		if (kind != 'p')
		{
			ereport(LOG,
				(errmsg("unexpected password response received. expected 'p' received '%c'",kind)));
			return NULL;		/* bad message type */
		}
	}
	/* pre-3.0 protocol does not send a message type */

	if (pool_read(frontend, &rsize, sizeof(int)) < 0)
		return NULL;

	rsize = ntohl(rsize) - 4;
	passwd = pool_read2(frontend, rsize); /* retrieve password */
	if (passwd == NULL)
		return NULL;

	/* Do not echo password to logs, for security. */
	ereport(DEBUG1,
		(errmsg("received password packet from frontend for pgpool's HBA")));

	/*
	 * Return the received string.  Note we do not attempt to do any
	 * character-set conversion on it; since we don't yet know the
	 * client's encoding, there wouldn't be much point.
	 */
	returnVal = pstrdup(passwd);
	return returnVal;
}

#endif /* USE_PAM */

/*
 * Tell the user the authentication failed.
 */
static void auth_failed(POOL_CONNECTION *frontend)
{
	int messagelen;
	char *errmessage;

	messagelen = strlen(frontend->username) + 100;
	errmessage = (char *)palloc(messagelen+1);

	switch (frontend->auth_method)
	{
		case uaReject:
			snprintf(errmessage, messagelen,
					 "authentication with pgpool failed for user \"%s\": host rejected",
					 frontend->username);
			break;
/* 		case uaKrb4: */
/* 			snprintf(errmessage, messagelen, */
/* 					 "Kerberos 4 authentication with pgpool failed for user \"%s\"", */
/* 					 frontend->username); */
/* 			break; */
/* 		case uaKrb5: */
/* 			snprintf(errmessage, messagelen, */
/* 					 "Kerberos 5 authentication with pgpool failed for user \"%s\"", */
/* 					 frontend->username); */
/* 			break; */
		case uaTrust:
			snprintf(errmessage, messagelen,
					 "\"trust\" authentication with pgpool failed for user \"%s\"",
					 frontend->username);
			break;
/* 		case uaIdent: */
/* 			snprintf(errmessage, messagelen, */
/* 					 "Ident authentication with pgpool failed for user \"%s\"", */
/* 					 frontend->username); */
/* 			break; */
 		case uaMD5:
			snprintf(errmessage, messagelen,
					 "\"MD5\" authentication with pgpool failed for user \"%s\"",
					 frontend->username);
			break;

/* 		case uaCrypt: */
/* 		case uaPassword: */
/* 			snprintf(errmessage, messagelen, */
/* 					 "password authentication with pgpool failed for user \"%s\"", */
/* 					 frontend->username); */
/* 			break; */
#ifdef USE_PAM
		case uaPAM:
			snprintf(errmessage, messagelen,
					 "PAM authentication with pgpool failed for user \"%s\"",
					 frontend->username);
			break;
#endif /* USE_PAM */
		default:
			snprintf(errmessage, messagelen,
					 "authentication with pgpool failed for user \"%s\": invalid authentication method",
					 frontend->username);
			break;
	}
	close_all_backend_connections();
    ereport(FATAL,
        (return_code(2),
             errmsg("client authentication failed"),
                errdetail("%s",errmessage),
                    errhint("see pgpool log for details")));

}


/*
 *  Close all of the cached backend connections.
 *
 *  This is exactly the same as send_frontend_exits() in child.c.
 */
static void close_all_backend_connections(void)
{
	int i;
	POOL_CONNECTION_POOL *p = pool_connection_pool;

#ifdef HAVE_SIGPROCMASK
	sigset_t oldmask;
#else
	int	oldmask;
#endif

	POOL_SETMASK2(&BlockSig, &oldmask);

	for (i=0;i<pool_config->max_pool;i++, p++)
	{
		if (!MASTER_CONNECTION(p))
			continue;
		if (MASTER_CONNECTION(p)->sp->user == NULL)
			continue;
		pool_send_frontend_exits(p);
	}

	POOL_SETMASK(&oldmask);
}


/*
 *  Determine what authentication method should be used when accessing database
 *  "database" from frontend "raddr", user "user".  Return the method and
 *  an optional argument (stored in fields of *frontend), and true for success.
 *
 *  Note that false indicates a problem with the hba config file.
 *  If the file is OK but does not contain any entry matching the request,
 *  we return true and method = uaReject.
 */
static bool hba_getauthmethod(POOL_CONNECTION *frontend)
{
	if (check_hba(frontend))
		return true;
	else
		return false;
}


/*
 *  Scan the (pre-parsed) hba file line by line, looking for a match
 *  to the port's connection request.
 */
static bool check_hba(POOL_CONNECTION *frontend)
{
	bool found_entry = false;
	bool error = false;
	ListCell *line;
	ListCell *line_num;

	forboth(line, hba_lines, line_num, hba_line_nums)
	{
		parse_hba(lfirst(line), lfirst_int(line_num),
				  frontend, &found_entry, &error);
		if (found_entry || error)
			break;
	}

	if (!error)
	{
		/* If no matching entry was found, synthesize 'reject' entry. */
		if (!found_entry)
			frontend->auth_method = uaReject;
		return true;
	}
	else
		return false;
}


/*
 *  Process one line from the hba config file.
 *
 *  See if it applies to a connection from a frontend with IP address
 *  frontend->raddr to a database named frontend->database.  If so, return
 *  *found_p true and fill in the auth arguments into the appropriate
 *  frontend fields. If not, leave *found_p as it was.  If the record has
 *  a syntax error, return *error_p true, after issuing a message to the
 *  log.  If no error, leave *error_p as it was.
 */
static void parse_hba(List *line, int line_num, POOL_CONNECTION *frontend,
					  bool *found_p, bool *error_p)
{
	char *token;
	char *db, *db_tmp;
	char *user, *user_tmp;
	struct addrinfo *gai_result;
	struct addrinfo hints;
	int ret;
	struct sockaddr_storage addr;
	struct sockaddr_storage mask;
	char *cidr_slash;
	ListCell *line_item;

	line_item = list_head(line);
	/* Check the record type. */
	token = lfirst(line_item);
	if (strcmp(token, "local") == 0)
	{
		/* Get the database. */
		line_item = lnext(line_item);
		if (!line_item)
			goto hba_syntax;
		db = lfirst(line_item);

		/* Get the user. */
		line_item = lnext(line_item);
		if (!line_item)
			goto hba_syntax;
		user = lfirst(line_item);

		line_item = lnext(line_item);
		if (!line_item)
			goto hba_syntax;

		/* Read the rest of the line. */
		parse_hba_auth(&line_item, &frontend->auth_method,
					   &frontend->auth_arg, error_p);
		if (*error_p)
			goto hba_syntax;

        /* Disallow auth methods that always need TCP/IP sockets to work */
		/*
		if (frontend->auth_method == uaKrb4 ||
			frontend->auth_method == uaKrb5)
			goto hba_syntax;
		*/

		/* Does not match if connection isn't AF_UNIX */
		if (!IS_AF_UNIX(frontend->raddr.addr.ss_family))
			return;
	}
	else if (strcmp(token, "host") == 0
			 || strcmp(token, "hostssl") == 0
			 || strcmp(token, "hostnossl") == 0)
	{
		if (token[4] == 's')    /* "hostssl" */
		{
#ifdef USE_SSL
			/* Record does not match if we are not on an SSL connection */
			if (!frontend->ssl)
				return;

			/* Placeholder to require specific SSL level, perhaps? */
			/* Or a client certificate */

			/* Since we were on SSL, proceed as with normal 'host' mode */
#else
			/* We don't accept this keyword at all if no SSL support */
			goto hba_syntax;
#endif
		}
#ifdef USE_SSL
		else if (token[4] == 'n')       /* "hostnossl" */
		{
			/* Record does not match if we are on an SSL connection */
			if (frontend->ssl)
				return;
		}
#endif

        /* Get the database. */
		line_item = lnext(line_item);
		if (!line_item)
			goto hba_syntax;
		db = lfirst(line_item);

		/* Get the user. */
		line_item = lnext(line_item);
		if (!line_item)
			goto hba_syntax;
		user = lfirst(line_item);

		/* Read the IP address field. (with or without CIDR netmask) */
		line_item = lnext(line_item);
		if (!line_item)
			goto hba_syntax;
		token = lfirst(line_item);

		/* Check if it has a CIDR suffix and if so isolate it */
		cidr_slash = strchr(token, '/');
		if (cidr_slash)
			*cidr_slash = '\0';

		/* Get the IP address either way */
		hints.ai_flags = AI_NUMERICHOST;
		hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = 0;
		hints.ai_protocol = 0;
		hints.ai_addrlen = 0;
		hints.ai_canonname = NULL;
		hints.ai_addr = NULL;
		hints.ai_next = NULL;

		ret = getaddrinfo_all(token, NULL, &hints, &gai_result);
		if (ret || !gai_result)
		{
			ereport(LOG,
				(errmsg("parsing pool hba configuration file"),
				 errdetail("invalid IP address \"%s\" in file \"%s\" line %d: %s",
						   token, hbaFileName, line_num, gai_strerror(ret))));
			if (cidr_slash)
				*cidr_slash = '/';
            if (gai_result)
				freeaddrinfo_all(hints.ai_family, gai_result);
			goto hba_other_error;
		}

		if (cidr_slash)
			*cidr_slash = '/';

		memcpy(&addr, gai_result->ai_addr, gai_result->ai_addrlen);
		freeaddrinfo_all(hints.ai_family, gai_result);

		/* Get the netmask */
		if (cidr_slash)
		{
			if (SockAddr_cidr_mask(&mask, cidr_slash + 1, addr.ss_family) < 0)
				goto hba_syntax;
		}
		else
		{
			/* Read the mask field. */
			line_item = lnext(line_item);
			if (!line_item)
				goto hba_syntax;
			token = lfirst(line_item);

			ret = getaddrinfo_all(token, NULL, &hints, &gai_result);
			if (ret || !gai_result)
			{
				ereport(LOG,
					(errmsg("parsing pool hba configuration file"),
						 errdetail("invalid IP mask \"%s\" in file \"%s\" line %d: %s",
								   token, hbaFileName, line_num, gai_strerror(ret))));

				if (gai_result)
					freeaddrinfo_all(hints.ai_family, gai_result);
				goto hba_other_error;
			}

			memcpy(&mask, gai_result->ai_addr, gai_result->ai_addrlen);
			freeaddrinfo_all(hints.ai_family, gai_result);

			if (addr.ss_family != mask.ss_family)
			{
				ereport(LOG,
					(errmsg("parsing pool hba configuration file"),
						errdetail("IP address and mask do not match in file \"%s\" line %d",
							   hbaFileName, line_num)));
				goto hba_other_error;
			}
		}

		if (addr.ss_family != frontend->raddr.addr.ss_family)
		{
			/*
			 * Wrong address family.  We allow only one case: if the file
			 * has IPv4 and the port is IPv6, promote the file address to
			 * IPv6 and try to match that way.
			 */
			if (addr.ss_family == AF_INET && frontend->raddr.addr.ss_family == AF_INET6)
			{
				promote_v4_to_v6_addr(&addr);
				promote_v4_to_v6_mask(&mask);
			}
			else
			{
				/* Line doesn't match client port, so ignore it. */
				return;
			}
		}

		/* Ignore line if client port is not in the matching addr range. */
		if (!rangeSockAddr(&frontend->raddr.addr, &addr, &mask))
			return;

		/* Read the rest of the line. */
		line_item = lnext(line_item);
		if (!line_item)
			goto hba_syntax;
		parse_hba_auth(&line_item, &frontend->auth_method,
					   &frontend->auth_arg, error_p);
		if (*error_p)
			goto hba_syntax;
	}
	else
		goto hba_syntax;

	/* Does the entry match database and user? */
	/*
	 * duplicate db and username since strtok() in check_db() and check_user()
	 * will override '\001' with '\0'.
	 */
	db_tmp = pstrdup(db);
	if (!check_db(frontend->database, frontend->username, db_tmp))
	{
		pfree(db_tmp);
		return;
	}
	pfree(db_tmp);

	user_tmp = pstrdup(user);
	if (!check_user(frontend->username, user_tmp))
	{
		pfree(user_tmp);
        return;
	}
	pfree(user_tmp);

	/* Success */
	*found_p = true;
	return;

 hba_syntax:
	if (line_item)
		ereport(LOG,
				(errmsg("parsing pool hba configuration file"),
					errdetail("invalid entry in file \"%s\" at line %d, token \"%s\"",
						   hbaFileName, line_num, (char *) lfirst(line_item))));
	else
		ereport(LOG,
				(errmsg("parsing pool hba configuration file"),
					errdetail("missing field in file \"%s\" at end of line %d",
						   hbaFileName, line_num)));
	/* Come here if suitable message already logged */
 hba_other_error:
	*error_p = true;
}


/*
 *  Scan the rest of a host record (after the mask field)
 *  and return the interpretation of it as *userauth_p, *auth_arg_p, and
 *  *error_p.  *line_item points to the next token of the line, and is
 *  advanced over successfully-read tokens.
 */
static void parse_hba_auth(ListCell **line_item, UserAuth *userauth_p,
						   char **auth_arg_p, bool *error_p)
{
	char *token;

	*auth_arg_p = NULL;

	if (!*line_item)
	{
		*error_p = true;
		return;
	}

	token = lfirst(*line_item);
	if (strcmp(token, "trust") == 0)
		*userauth_p = uaTrust;
	/*
	else if (strcmp(token, "ident") == 0)
		*userauth_p = uaIdent;
	else if (strcmp(token, "password") == 0)
		*userauth_p = uaPassword;
	else if (strcmp(token, "krb4") == 0)
		*userauth_p = uaKrb4;
	else if (strcmp(token, "krb5") == 0)
		*userauth_p = uaKrb5;
	*/
	else if (strcmp(token, "reject") == 0)
		*userauth_p = uaReject;
	else if (strcmp(token, "md5") == 0)
		*userauth_p = uaMD5;
	/*
	else if (strcmp(token, "crypt") == 0)
		*userauth_p = uaCrypt;
	*/
#ifdef USE_PAM
	else if (strcmp(token, "pam") == 0)
		*userauth_p = uaPAM;
#endif /* USE_PAM */
	else
	{
		*error_p = true;
		return;
	}
	*line_item = lnext(*line_item);

	/* Get the authentication argument token, if any */
	if (*line_item)
	{
		token = lfirst(*line_item);
        *auth_arg_p = pstrdup(token);
		*line_item = lnext(*line_item);
		/* If there is more on the line, it is an error */
		if (*line_item)
			*error_p = true;
	}
}


/*
 * Check comma user list for a specific user, handle group names.
 */
static bool check_user(char *user, char *param_str)
{
	char *tok;

	for (tok = strtok(param_str, MULTI_VALUE_SEP);
		 tok != NULL; tok = strtok(NULL, MULTI_VALUE_SEP))
	{
		if (tok[0] == '+')
		{
			/*
			 * pgpool cannot accept groups. commented lines below are the
			 * original code.
			 */
			ereport(LOG,
				(errmsg("group token \"+\" is not supported in pgpool")));
			return false;
/* 			if (check_group(tok + 1, user)) */
/* 				return true; */
		}
		else if (strcmp(tok, user) == 0 || strcmp(tok, "all\n") == 0)
			return true;
	}

	return false;
}


/*
 * Check to see if db/user combination matches param string.
 */
static bool check_db(char *dbname, char *user, char *param_str)
{
	char *tok;

	for (tok = strtok(param_str, MULTI_VALUE_SEP);
		 tok != NULL; tok = strtok(NULL, MULTI_VALUE_SEP))
	{
		if (strcmp(tok, "all\n") == 0)
			return true;
		else if (strcmp(tok, "sameuser\n") == 0)
		{
			if (strcmp(dbname, user) == 0)
				return true;
		}
		else if (strcmp(tok, "samegroup\n") == 0)
		{
			/*
			 * pgpool cannot accept groups. commented lines below are the
			 * original code.
			 */
			ereport(LOG,
				(errmsg("group token \"samegroup\" is not supported in pgpool")));
			return false;
/* 			if (check_group(dbname, user)) */
/* 				return true; */
		}
		else if (strcmp(tok, dbname) == 0)
			return true;
	}

	return false;
}


/*
 * tokenize the given file, storing the resulting data into two lists:
 * a list of sublists, each sublist containing the tokens in a line of
 * the file, and a list of line numbers.
 *
 * filename must be the absolute path to the target file.
 */
static void tokenize_file(const char *filename, FILE *file,
						  List **lines, List **line_nums)
{
	List *current_line = NIL;
	int line_number = 1;
	char *buf;

	*lines = *line_nums = NIL;

	while (!feof(file))
	{
		buf = next_token_expand(filename, file);

		/* add token to list, unless we are at EOL or comment start */
		if (buf[0])
		{
			if (current_line == NIL)
			{
				/* make a new line List, record its line number */
				current_line = lappend(current_line, buf);
				*lines = lappend(*lines, current_line);
				*line_nums = lappend_int(*line_nums, line_number);
			}
			else
			{
				/* append token to current line's list */
				current_line = lappend(current_line, buf);
			}
		}
		else
		{
			/* we are at real or logical EOL, so force a new line List */
			current_line = NIL;
			/* Advance line number whenever we reach EOL */
			line_number++;
			/* Don't forget to free the next_token_expand result */
			pfree(buf);
		}
	}
}


static char * tokenize_inc_file(const char *outer_filename,
								const char *inc_filename)
{
	char *inc_fullname;
	FILE *inc_file;
	List *inc_lines;
	List *inc_line_nums;
	ListCell *line;
	char *comma_str;

	if (is_absolute_path(inc_filename))
	{
		/* absolute path is taken as-is */
		inc_fullname = pstrdup(inc_filename);
	}
	else
	{
		/* relative path is relative to dir of calling file */
		inc_fullname = (char *)palloc(strlen(outer_filename) + 1 +
									  strlen(inc_filename) + 1);
		strcpy(inc_fullname, outer_filename);
		get_parent_directory(inc_fullname);
		join_path_components(inc_fullname, inc_fullname, inc_filename);
		canonicalize_path(inc_fullname);
	}

	inc_file = fopen(inc_fullname, "r");
	if (inc_file == NULL)
	{
		ereport(WARNING,
			(errmsg("failed to open secondary authentication file:\"%s\" as \"%s\"", inc_fullname,inc_fullname),
				 errdetail("fopen failed with error: \"%s\"",strerror(errno))));

		pfree(inc_fullname);

		/* return single space, it matches nothing */
		return pstrdup(" ");
	}

    /* There is possible recursion here if the file contains @ */
	tokenize_file(inc_fullname, inc_file, &inc_lines, &inc_line_nums);

	/*FreeFile(inc_file);*/
	fclose(inc_file);
	pfree(inc_fullname);

	/* Create comma-separated string from List */
	comma_str = pstrdup("");
	foreach(line, inc_lines)
	{
		List *token_list = (List *) lfirst(line);
		ListCell *token;

		foreach(token, token_list)
		{
			int oldlen = strlen(comma_str);
			int needed;

			needed = oldlen + strlen(lfirst(token)) + 1;
			if (oldlen > 0)
				needed++;
			comma_str = repalloc(comma_str, needed);
			if (oldlen > 0)
				strcat(comma_str, MULTI_VALUE_SEP);
			strcat(comma_str, lfirst(token));
		}
	}

	free_lines(&inc_lines, &inc_line_nums);

	/* if file is empty, return single space rather than empty string */
	if (strlen(comma_str) == 0)
	{
		char *returnVal;

		pfree(comma_str);
		returnVal = pstrdup(" ");
		return returnVal;
	}

	return comma_str;
}


/*
 * isblank() exists in the ISO C99 spec, but it's not very portable yet,
 * so provide our own version.
 */
static bool pg_isblank(const char c)
{
	return c == ' ' || c == '\t' || c == '\r';
}


/*
 *	 Tokenize file and handle file inclusion and comma lists. We have
 *	 to  break	apart  the	commas	to	expand	any  file names then
 *	 reconstruct with commas.
 *
 * The result is always a malloc'd string.  If it's zero-length then
 * we have reached EOL.
 */
static char * next_token_expand(const char *filename, FILE *file)
{
	char buf[MAX_TOKEN];
	char *comma_str;
	bool trailing_comma;
	char *incbuf;
	int needed;

	comma_str = pstrdup("");

	do
	{
		next_token(file, buf, sizeof(buf));
		if (!buf[0])
			break;

		if (buf[strlen(buf) - 1] == ',')
		{
			trailing_comma = true;
			buf[strlen(buf) - 1] = '\0';
		}
		else
			trailing_comma = false;

		/* Is this referencing a file? */
		if (buf[0] == '@')
			incbuf = tokenize_inc_file(filename, buf + 1);
		else
		{
			incbuf = pstrdup(buf);
		}

		needed = strlen(comma_str) + strlen(incbuf) + 1;
		if (trailing_comma)
			needed++;
		comma_str = repalloc(comma_str, needed);
		strcat(comma_str, incbuf);
		if (trailing_comma)
			strcat(comma_str, MULTI_VALUE_SEP);
		pfree(incbuf);
	} while (trailing_comma);

	return comma_str;
}


/*
 * Grab one token out of fp. Tokens are strings of non-blank
 * characters bounded by blank characters, beginning of line, and
 * end of line. Blank means space or tab. Return the token as
 * *buf. Leave file positioned at the character immediately after the
 * token or EOF, whichever comes first. If no more tokens on line,
 * return empty string as *buf and position the file to the beginning
 * of the next line or EOF, whichever comes first. Allow spaces in
 * quoted strings. Terminate on unquoted commas. Handle
 * comments. Treat unquoted keywords that might be user names or
 * database names specially, by appending a newline to them.
 */
static void next_token(FILE *fp, char *buf, int bufsz)
{
	int c;
	char *start_buf = buf;
	char *end_buf = buf + (bufsz - 2);
	bool in_quote = false;
	bool was_quote = false;
	bool saw_quote = false;

	/*Assert(end_buf > start_buf);*/

	/* Move over initial whitespace and commas */
	while ((c = getc(fp)) != EOF && (pg_isblank(c) || c == ','))
		;

	if (c == EOF || c == '\n')
	{
		*buf = '\0';
		return;
	}

	/*
	 * Build a token in buf of next characters up to EOF, EOL, unquoted
	 * comma, or unquoted whitespace.
	 */
	while (c != EOF && c != '\n' &&
		   (!pg_isblank(c) || in_quote == true))
	{
		/* skip comments to EOL */
		if (c == '#' && !in_quote)
		{
			while ((c = getc(fp)) != EOF && c != '\n')
				;
			/* If only comment, consume EOL too; return EOL */
			if (c != EOF && buf == start_buf)
				c = getc(fp);
			break;
		}

		if (buf >= end_buf)
		{
			*buf = '\0';
			ereport(LOG,
				(errmsg("parsing pool hba configuration file"),
					 errdetail("authentication file token too long, skipping: \"%s\"", start_buf)));

			/* Discard remainder of line */
			while ((c = getc(fp)) != EOF && c != '\n')
				;
			break;
		}

		if (c != '"' || (c == '"' && was_quote))
			*buf++ = c;

		/* We pass back the comma so the caller knows there is more */
		if ((pg_isblank(c) || c == ',') && !in_quote)
			break;

		/* Literal double-quote is two double-quotes */
		if (in_quote && c == '"')
			was_quote = !was_quote;
		else
			was_quote = false;

		if (c == '"')
		{
			in_quote = !in_quote;
			saw_quote = true;
		}

		c = getc(fp);
	}

	/*
	 * Put back the char right after the token (critical in case it is
	 * EOL, since we need to detect end-of-line at next call).
	 */
	if (c != EOF)
		ungetc(c, fp);

	*buf = '\0';

	if (!saw_quote &&
		(strcmp(start_buf, "all") == 0 ||
		 strcmp(start_buf, "sameuser") == 0 ||
		 strcmp(start_buf, "samegroup") == 0))
	{
		/* append newline to a magical keyword */
		*buf++ = '\n';
		*buf = '\0';
	}
}


/*
 * free memory used by lines and tokens built by tokenize_file()
 */
static void free_lines(List **lines, List **line_nums)
{
	if (*lines)
	{
		ListCell *line;

		foreach(line, *lines)
		{
			List *ln = lfirst(line);
			ListCell *token;

			foreach(token, ln)
				pfree(lfirst(token));

			list_free(ln);
		}

		list_free(*lines);
		*lines = NIL;
	}

	if (*line_nums)
	{
		list_free(*line_nums);
		*line_nums = NIL;
	}
}


#ifdef USE_PAM

/*
 * PAM conversation function
 */
static int pam_passwd_conv_proc(int num_msg, const struct pam_message ** msg,
								struct pam_response ** resp, void *appdata_ptr)
{
	if (num_msg != 1 || msg[0]->msg_style != PAM_PROMPT_ECHO_OFF)
	{
		switch (msg[0]->msg_style)
		{
			case PAM_ERROR_MSG:
				ereport(LOG,
					(errmsg("PAM Error"),
						errdetail("error from underlying PAM layer: %s",
							   msg[0]->msg)));
				return PAM_CONV_ERR;
			default:
				ereport(LOG,
					(errmsg("PAM Error"),
						errdetail("unsupported PAM conversation %d/%s",
							   msg[0]->msg_style, msg[0]->msg)));
				return PAM_CONV_ERR;
		}
	}

	if (!appdata_ptr)
	{
		/*
		 * Workaround for Solaris 2.6 where the PAM library is broken and
		 * does not pass appdata_ptr to the conversation routine
		 */
		appdata_ptr = pam_passwd;
	}

	/*
	 * Password wasn't passed to PAM the first time around - let's go ask
	 * the client to send a password, which we then stuff into PAM.
	 */
	if (strlen(appdata_ptr) == 0)
	{
		char *passwd;

		sendAuthRequest(pam_frontend_kludge, AUTH_REQ_PASSWORD);
		passwd = recv_password_packet(pam_frontend_kludge);

		if (passwd == NULL)
			return PAM_CONV_ERR; /* client didn't want to send password */

		if (strlen(passwd) == 0)
		{
			ereport(LOG,
				(errmsg("PAM Error"),
					 errdetail("empty password returned by client")));
			return PAM_CONV_ERR;
		}
		appdata_ptr = passwd;
	}

	/*
	 * PAM will free this memory in * pam_end()
	 */
	*resp = palloc0(num_msg* sizeof(struct pam_response));

	(*resp)[0].resp = pstrdup((char *) appdata_ptr);
	(*resp)[0].resp_retcode = 0;

	return ((*resp)[0].resp ? PAM_SUCCESS : PAM_CONV_ERR);
}


/*
 * Check authentication against PAM.
 */
static POOL_STATUS CheckPAMAuth(POOL_CONNECTION *frontend, char *user, char *password)
{
	int retval;
	pam_handle_t *pamh = NULL;

	/*
	 * Apparently, Solaris 2.6 is broken, and needs ugly static variable
	 * workaround
	 */
	pam_passwd = password;

	/*
	 * Set the application data portion of the conversation struct This is
	 * later used inside the PAM conversation to pass the password to the
	 * authentication module.
	 */
	pam_passw_conv.appdata_ptr = (char *) password;	/* from password above,
													 * not allocated */

	/* Optionally, one can set the service name in pool_hba.conf */
	if (frontend->auth_arg && frontend->auth_arg[0] != '\0')
		retval = pam_start(frontend->auth_arg, "pgpool@",
						   &pam_passw_conv, &pamh);
	else
		retval = pam_start(PGPOOL_PAM_SERVICE, "pgpool@",
						   &pam_passw_conv, &pamh);

	if (retval != PAM_SUCCESS)
	{
		pam_passwd = NULL;		/* Unset pam_passwd */
        ereport(FATAL,
            (return_code(2),
             errmsg("failed authentication against PAM"),
                    errdetail("unable to create PAM authenticator: %s", pam_strerror(pamh, retval))));
	}

	retval = pam_set_item(pamh, PAM_USER, user);
	if (retval != PAM_SUCCESS)
	{
		pam_passwd = NULL;		/* Unset pam_passwd */
        ereport(FATAL,
                (return_code(2),
                 errmsg("failed authentication against PAM"),
                 errdetail("pam_set_item(PAM_USER) failed: %s", pam_strerror(pamh, retval))));
	}

	retval = pam_set_item(pamh, PAM_CONV, &pam_passw_conv);
	if (retval != PAM_SUCCESS)
	{
		pam_passwd = NULL;		/* Unset pam_passwd */
        ereport(FATAL,
                (return_code(2),
                 errmsg("failed authentication against PAM"),
                 errdetail("pam_set_item(PAM_CONV) failed: %s", pam_strerror(pamh, retval))));
	}

	retval = pam_authenticate(pamh, 0);
	if (retval != PAM_SUCCESS)	/* service name does not exist */
	{
		pam_passwd = NULL;		/* Unset pam_passwd */
        ereport(FATAL,
            (return_code(2),
                 errmsg("failed authentication against PAM"),
                    errdetail("pam_authenticate failed: %s", pam_strerror(pamh, retval))));
	}

	retval = pam_acct_mgmt(pamh, 0);
	if (retval != PAM_SUCCESS)
	{
		pam_passwd = NULL;		/* Unset pam_passwd */
        ereport(FATAL,
            (return_code(2),
                 errmsg("failed authentication against PAM"),
                    errdetail("system call pam_acct_mgmt failed : %s", pam_strerror(pamh, retval))));
	}

	retval = pam_end(pamh, retval);
	if (retval != PAM_SUCCESS)
	{
        ereport(FATAL,
            (return_code(2),
                 errmsg("failed authentication against PAM"),
                    errdetail("unable to release PAM authenticator: %s", pam_strerror(pamh, retval))));
}

	pam_passwd = NULL;
	return POOL_CONTINUE;
}

#endif   /* USE_PAM */

static POOL_STATUS CheckMd5Auth(char *username)
{
	char *passwd;

	/* Look for the entry in pool_passwd */
	passwd = pool_get_passwd(username);

	if (passwd == NULL)
        ereport(FATAL,
            (return_code(2),
                 errmsg("md5 authentication failed"),
                    errdetail("pool_passed file does not contains entry for \"%s\"",username)));


	/*
	 * Ok for now. Actual authentication will be performed later.
	 */
	return POOL_CONTINUE;
}
