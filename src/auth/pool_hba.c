/* -*-pgsql-c-*- */
/*
 *
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Portions Copyright (c) 2003-2018	PgPool Global Development Group
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
#include "auth/pool_hba.h"
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
#define MAX_LINE	8192

static MemoryContext parsed_hba_context = NULL;
static List *parsed_hba_lines = NIL;
static char *HbaFileName;


#define token_is_keyword(t, k)	(!t->quoted && strcmp(t->string, k) == 0)
#define token_matches(t, k)  (strcmp(t->string, k) == 0)


/*
 * TokenizedLine represents one line lexed from a config file.
 * Each item in the "fields" list is a sub-list of HbaTokens.
 * We don't emit a TokenizedLine for empty or all-comment lines,
 * so "fields" is never NIL (nor are any of its sub-lists).
 * Exception: if an error occurs during tokenization, we might
 * have fields == NIL, in which case err_msg != NULL.
 */
typedef struct TokenizedLine
{
	List	   *fields;			/* List of lists of HbaTokens */
	int			line_num;		/* Line number */
	char	   *raw_line;		/* Raw line text */
	char	   *err_msg;		/* Error message if any */
} TokenizedLine;

/*
 * A single string token lexed from a config file, together with whether
 * the token had been quoted.
 */
typedef struct HbaToken
{
	char	   *string;
	bool		quoted;
} HbaToken;
/* callback data for check_network_callback */
typedef struct check_network_data
{
	IPCompareMethod method;		/* test method */
	SockAddr   *raddr;			/* client's actual address */
	bool		result;			/* set to true if match */
} check_network_data;


static HbaToken *
copy_hba_token(HbaToken *in);
static HbaToken *
make_hba_token(const char *token, bool quoted);

static bool
parse_hba_auth_opt(char *name, char *val, HbaLine *hbaline,
				   int elevel, char **err_msg);

static MemoryContext tokenize_file(const char *filename, FILE *file,
								   List **tok_lines, int elevel);
static void sendAuthRequest(POOL_CONNECTION *frontend, AuthRequest areq);
static void auth_failed(POOL_CONNECTION *frontend);
static void close_all_backend_connections(void);
static bool hba_getauthmethod(POOL_CONNECTION *frontend);
static bool check_hba(POOL_CONNECTION *frontend);
static bool check_user(char *user, List *tokens);
static bool check_db(const char *dbname, const char *user, List *tokens);
static List * tokenize_inc_file(List *tokens,
				  const char *outer_filename,
				  const char *inc_filename,
				  int elevel,
								char **err_msg);
static bool
check_hostname(POOL_CONNECTION *frontend, const char *hostname);
static bool
check_ip(SockAddr *raddr, struct sockaddr *addr, struct sockaddr *mask);
static bool
check_same_host_or_net(SockAddr *raddr, IPCompareMethod method);
static void
check_network_callback(struct sockaddr *addr, struct sockaddr *netmask,
					   void *cb_data);

static HbaLine *
parse_hba_line(TokenizedLine *tok_line, int elevel);
static bool pg_isblank(const char c);
static bool
next_token(char **lineptr, char *buf, int bufsz,
		   bool *initial_quote, bool *terminating_comma,
		   int elevel, char **err_msg);
static List *
next_field_expand(const char *filename, char **lineptr,
				  int elevel, char **err_msg);
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
 * Read the config file and create a List of HbaLine records for the contents.
 *
 * The configuration is read into a temporary list, and if any parse error
 * occurs the old list is kept in place and false is returned.  Only if the
 * whole file parses OK is the list replaced, and the function returns true.
 *
 * On a false result, caller will take care of reporting a FATAL error in case
 * this is the initial startup.  If it happens on reload, we just keep running
 * with the old data.
 */
bool
load_hba(char *hbapath)
{
	FILE	   *file;
	List	   *hba_lines = NIL;
	ListCell   *line;
	List	   *new_parsed_lines = NIL;
	bool		ok = true;
	MemoryContext linecxt;
	MemoryContext oldcxt;
	MemoryContext hbacxt;

	HbaFileName = pstrdup(hbapath);

	file = fopen(hbapath, "r");
	if (file == NULL)
	{
		ereport(LOG,
				(errmsg("could not open configuration file \"%s\": %m",
						HbaFileName)));
		return false;
	}

	linecxt = tokenize_file(HbaFileName, file, &hba_lines, LOG);
	fclose(file);

	/* Now parse all the lines */
	hbacxt = AllocSetContextCreate(TopMemoryContext,
								   "hba parser context",
								   ALLOCSET_SMALL_SIZES);
	oldcxt = MemoryContextSwitchTo(hbacxt);
	foreach(line, hba_lines)
	{
		TokenizedLine *tok_line = (TokenizedLine *) lfirst(line);
		HbaLine    *newline;

		/* don't parse lines that already have errors */
		if (tok_line->err_msg != NULL)
		{
			ok = false;
			continue;
		}

		if ((newline = parse_hba_line(tok_line, LOG)) == NULL)
		{
			/* Parse error; remember there's trouble */
			ok = false;

			/*
			 * Keep parsing the rest of the file so we can report errors on
			 * more than the first line.  Error has already been logged, no
			 * need for more chatter here.
			 */
			continue;
		}

		new_parsed_lines = lappend(new_parsed_lines, newline);
	}

	/*
	 * A valid HBA file must have at least one entry; else there's no way to
	 * connect to the postmaster.  But only complain about this if we didn't
	 * already have parsing errors.
	 */
	if (ok && new_parsed_lines == NIL)
	{
		ereport(LOG,
				(errcode(ERRCODE_CONFIG_FILE_ERROR),
				 errmsg("configuration file \"%s\" contains no entries",
						HbaFileName)));
		ok = false;
	}

	/* Free tokenizer memory */
	MemoryContextDelete(linecxt);
	MemoryContextSwitchTo(oldcxt);

	if (!ok)
	{
		/* File contained one or more errors, so bail out */
		MemoryContextDelete(hbacxt);
		return false;
	}

	/* Loaded new file successfully, replace the one we use */
	if (parsed_hba_context != NULL)
		MemoryContextDelete(parsed_hba_context);
	parsed_hba_context = hbacxt;
	parsed_hba_lines = new_parsed_lines;

	return true;
}

static HbaLine *
parse_hba_line(TokenizedLine *tok_line, int elevel)
{
	int			line_num = tok_line->line_num;
	char	  **err_msg = &tok_line->err_msg;
	char	   *str;
	struct addrinfo *gai_result;
	struct addrinfo hints;
	int			ret;
	char	   *cidr_slash;
	ListCell   *field;
	List	   *tokens;
	ListCell   *tokencell;
	HbaToken   *token;
	HbaLine    *parsedline;

	parsedline = palloc0(sizeof(HbaLine));
	parsedline->linenumber = line_num;
	parsedline->rawline = pstrdup(tok_line->raw_line);

	/* Check the record type. */
	Assert(tok_line->fields != NIL);
	field = list_head(tok_line->fields);
	tokens = lfirst(field);
	if (tokens->length > 1)
	{
		ereport(elevel,
				(errcode(ERRCODE_CONFIG_FILE_ERROR),
				 errmsg("multiple values specified for connection type"),
				 errhint("Specify exactly one connection type per line."),
				 errcontext("line %d of configuration file \"%s\"",
							line_num, HbaFileName)));
		*err_msg = "multiple values specified for connection type";
		return NULL;
	}
	token = linitial(tokens);
	if (strcmp(token->string, "local") == 0)
	{
		parsedline->conntype = ctLocal;
	}
	else if (strcmp(token->string, "host") == 0 ||
			 strcmp(token->string, "hostssl") == 0 ||
			 strcmp(token->string, "hostnossl") == 0)
	{

		if (token->string[4] == 's')	/* "hostssl" */
		{
			parsedline->conntype = ctHostSSL;
			/* Log a warning if SSL support is not active */
#ifdef USE_SSL
			if (!pool_config->ssl)
			{
				ereport(elevel,
						(errcode(ERRCODE_CONFIG_FILE_ERROR),
						 errmsg("hostssl record cannot match because SSL is disabled"),
						 errhint("Set ssl = on in pgpool.conf"),
						 errcontext("line %d of configuration file \"%s\"",
									line_num, HbaFileName)));
				*err_msg = "hostssl record cannot match because SSL is disabled";
			}
#else
			ereport(elevel,
					(errmsg("hostssl record cannot match because SSL is not supported by this build"),
					 errhint("Compile with --with-openssl to use SSL connections."),
					 errcontext("line %d of configuration file \"%s\"",
								line_num, HbaFileName)));
			*err_msg = "hostssl record cannot match because SSL is not supported by this build";
#endif
		}
		else if (token->string[4] == 'n')	/* "hostnossl" */
		{
			parsedline->conntype = ctHostNoSSL;
		}
		else
		{
			/* "host" */
			parsedline->conntype = ctHost;
		}
	}							/* record type */
	else
	{
		ereport(elevel,
				(errcode(ERRCODE_CONFIG_FILE_ERROR),
				 errmsg("invalid connection type \"%s\"",
						token->string),
				 errcontext("line %d of configuration file \"%s\"",
							line_num, HbaFileName)));
		*err_msg = psprintf("invalid connection type \"%s\"", token->string);
		return NULL;
	}

	/* Get the databases. */
	field = lnext(field);
	if (!field)
	{
		ereport(elevel,
				(errcode(ERRCODE_CONFIG_FILE_ERROR),
				 errmsg("end-of-line before database specification"),
				 errcontext("line %d of configuration file \"%s\"",
							line_num, HbaFileName)));
		*err_msg = "end-of-line before database specification";
		return NULL;
	}
	parsedline->databases = NIL;
	tokens = lfirst(field);
	foreach(tokencell, tokens)
	{
		parsedline->databases = lappend(parsedline->databases,
										copy_hba_token(lfirst(tokencell)));
	}

	/* Get the users. */
	field = lnext(field);
	if (!field)
	{
		ereport(elevel,
				(errcode(ERRCODE_CONFIG_FILE_ERROR),
				 errmsg("end-of-line before role specification"),
				 errcontext("line %d of configuration file \"%s\"",
							line_num, HbaFileName)));
		*err_msg = "end-of-line before role specification";
		return NULL;
	}
	parsedline->users = NIL;
	tokens = lfirst(field);
	foreach(tokencell, tokens)
	{
		parsedline->users = lappend(parsedline->users,
									copy_hba_token(lfirst(tokencell)));
	}

	if (parsedline->conntype != ctLocal)
	{
		/* Read the IP address field. (with or without CIDR netmask) */
		field = lnext(field);
		if (!field)
		{
			ereport(elevel,
					(errcode(ERRCODE_CONFIG_FILE_ERROR),
					 errmsg("end-of-line before IP address specification"),
					 errcontext("line %d of configuration file \"%s\"",
								line_num, HbaFileName)));
			*err_msg = "end-of-line before IP address specification";
			return NULL;
		}
		tokens = lfirst(field);
		if (tokens->length > 1)
		{
			ereport(elevel,
					(errcode(ERRCODE_CONFIG_FILE_ERROR),
					 errmsg("multiple values specified for host address"),
					 errhint("Specify one address range per line."),
					 errcontext("line %d of configuration file \"%s\"",
								line_num, HbaFileName)));
			*err_msg = "multiple values specified for host address";
			return NULL;
		}
		token = linitial(tokens);

		if (token_is_keyword(token, "all"))
		{
			parsedline->ip_cmp_method = ipCmpAll;
		}
		else if (token_is_keyword(token, "samehost"))
		{
			/* Any IP on this host is allowed to connect */
			parsedline->ip_cmp_method = ipCmpSameHost;
		}
		else if (token_is_keyword(token, "samenet"))
		{
			/* Any IP on the host's subnets is allowed to connect */
			parsedline->ip_cmp_method = ipCmpSameNet;
		}
		else
		{
			/* IP and netmask are specified */
			parsedline->ip_cmp_method = ipCmpMask;

			/* need a modifiable copy of token */
			str = pstrdup(token->string);

			/* Check if it has a CIDR suffix and if so isolate it */
			cidr_slash = strchr(str, '/');
			if (cidr_slash)
				*cidr_slash = '\0';

			/* Get the IP address either way */
			hints.ai_flags = AI_NUMERICHOST;
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = 0;
			hints.ai_protocol = 0;
			hints.ai_addrlen = 0;
			hints.ai_canonname = NULL;
			hints.ai_addr = NULL;
			hints.ai_next = NULL;

			ret = getaddrinfo_all(str, NULL, &hints, &gai_result);
			if (ret == 0 && gai_result)
				memcpy(&parsedline->addr, gai_result->ai_addr,
					   gai_result->ai_addrlen);
			else if (ret == EAI_NONAME)
				parsedline->hostname = str;
			else
			{
				ereport(elevel,
						(errcode(ERRCODE_CONFIG_FILE_ERROR),
						 errmsg("invalid IP address \"%s\": %s",
								str, gai_strerror(ret)),
						 errcontext("line %d of configuration file \"%s\"",
									line_num, HbaFileName)));
				*err_msg = psprintf("invalid IP address \"%s\": %s",
									str, gai_strerror(ret));
				if (gai_result)
					freeaddrinfo_all(hints.ai_family, gai_result);
				return NULL;
			}

			freeaddrinfo_all(hints.ai_family, gai_result);

			/* Get the netmask */
			if (cidr_slash)
			{
				if (parsedline->hostname)
				{
					ereport(elevel,
							(errcode(ERRCODE_CONFIG_FILE_ERROR),
							 errmsg("specifying both host name and CIDR mask is invalid: \"%s\"",
									token->string),
							 errcontext("line %d of configuration file \"%s\"",
										line_num, HbaFileName)));
					*err_msg = psprintf("specifying both host name and CIDR mask is invalid: \"%s\"",
										token->string);
					return NULL;
				}

				if (SockAddr_cidr_mask(&parsedline->mask, cidr_slash + 1,
										  parsedline->addr.ss_family) < 0)
				{
					ereport(elevel,
							(errcode(ERRCODE_CONFIG_FILE_ERROR),
							 errmsg("invalid CIDR mask in address \"%s\"",
									token->string),
							 errcontext("line %d of configuration file \"%s\"",
										line_num, HbaFileName)));
					*err_msg = psprintf("invalid CIDR mask in address \"%s\"",
										token->string);
					return NULL;
				}
				pfree(str);
			}
			else if (!parsedline->hostname)
			{
				/* Read the mask field. */
				pfree(str);
				field = lnext(field);
				if (!field)
				{
					ereport(elevel,
							(errcode(ERRCODE_CONFIG_FILE_ERROR),
							 errmsg("end-of-line before netmask specification"),
							 errhint("Specify an address range in CIDR notation, or provide a separate netmask."),
							 errcontext("line %d of configuration file \"%s\"",
										line_num, HbaFileName)));
					*err_msg = "end-of-line before netmask specification";
					return NULL;
				}
				tokens = lfirst(field);
				if (tokens->length > 1)
				{
					ereport(elevel,
							(errcode(ERRCODE_CONFIG_FILE_ERROR),
							 errmsg("multiple values specified for netmask"),
							 errcontext("line %d of configuration file \"%s\"",
										line_num, HbaFileName)));
					*err_msg = "multiple values specified for netmask";
					return NULL;
				}
				token = linitial(tokens);

				ret = getaddrinfo_all(token->string, NULL,
										 &hints, &gai_result);
				if (ret || !gai_result)
				{
					ereport(elevel,
							(errcode(ERRCODE_CONFIG_FILE_ERROR),
							 errmsg("invalid IP mask \"%s\": %s",
									token->string, gai_strerror(ret)),
							 errcontext("line %d of configuration file \"%s\"",
										line_num, HbaFileName)));
					*err_msg = psprintf("invalid IP mask \"%s\": %s",
										token->string, gai_strerror(ret));
					if (gai_result)
						freeaddrinfo_all(hints.ai_family, gai_result);
					return NULL;
				}

				memcpy(&parsedline->mask, gai_result->ai_addr,
					   gai_result->ai_addrlen);
				freeaddrinfo_all(hints.ai_family, gai_result);

				if (parsedline->addr.ss_family != parsedline->mask.ss_family)
				{
					ereport(elevel,
							(errcode(ERRCODE_CONFIG_FILE_ERROR),
							 errmsg("IP address and mask do not match"),
							 errcontext("line %d of configuration file \"%s\"",
										line_num, HbaFileName)));
					*err_msg = "IP address and mask do not match";
					return NULL;
				}
			}
		}
	}							/* != ctLocal */

	/* Get the authentication method */
	field = lnext(field);
	if (!field)
	{
		ereport(elevel,
				(errcode(ERRCODE_CONFIG_FILE_ERROR),
				 errmsg("end-of-line before authentication method"),
				 errcontext("line %d of configuration file \"%s\"",
							line_num, HbaFileName)));
		*err_msg = "end-of-line before authentication method";
		return NULL;
	}
	tokens = lfirst(field);
	if (tokens->length > 1)
	{
		ereport(elevel,
				(errcode(ERRCODE_CONFIG_FILE_ERROR),
				 errmsg("multiple values specified for authentication type"),
				 errhint("Specify exactly one authentication type per line."),
				 errcontext("line %d of configuration file \"%s\"",
							line_num, HbaFileName)));
		*err_msg = "multiple values specified for authentication type";
		return NULL;
	}
	token = linitial(tokens);

	if (strcmp(token->string, "trust") == 0)
		parsedline->auth_method = uaTrust;
	else if (strcmp(token->string, "reject") == 0)
		parsedline->auth_method = uaReject;
	else if (strcmp(token->string, "md5") == 0)
		parsedline->auth_method = uaMD5;
#ifdef USE_PAM
	else if (strcmp(token->string, "pam") == 0)
		parsedline->auth_method = uaPAM;
#endif
	else
	{
		ereport(elevel,
				(errcode(ERRCODE_CONFIG_FILE_ERROR),
				 errmsg("invalid authentication method \"%s\"",
						token->string),
				 errcontext("line %d of configuration file \"%s\"",
							line_num, HbaFileName)));
		*err_msg = psprintf("invalid authentication method \"%s\"",
							token->string);
		return NULL;
	}
	/* Parse remaining arguments */
	while ((field = lnext(field)) != NULL)
	{
		tokens = lfirst(field);
		foreach(tokencell, tokens)
		{
			char	   *val;

			token = lfirst(tokencell);

			str = pstrdup(token->string);
			val = strchr(str, '=');
			if (val == NULL)
			{
				/*
				 * Got something that's not a name=value pair.
				 */
				ereport(elevel,
						(errcode(ERRCODE_CONFIG_FILE_ERROR),
						 errmsg("authentication option not in name=value format: %s", token->string),
						 errcontext("line %d of configuration file \"%s\"",
									line_num, HbaFileName)));
				*err_msg = psprintf("authentication option not in name=value format: %s",
									token->string);
				pfree(str);
				return NULL;
			}

			*val++ = '\0';		/* str now holds "name", val holds "value" */
			if (!parse_hba_auth_opt(str, val, parsedline, elevel, err_msg))
			{
			/* parse_hba_auth_opt already logged the error message */
				pfree(str);
				return NULL;
			}
			pfree(str);
		}
	}

	return parsedline;
}

/*
 * Parse one name-value pair as an authentication option into the given
 * HbaLine.  Return true if we successfully parse the option, false if we
 * encounter an error.  In the event of an error, also log a message at
 * ereport level elevel, and store a message string into *err_msg.
 */
static bool
parse_hba_auth_opt(char *name, char *val, HbaLine *hbaline,
				   int elevel, char **err_msg)
{
	int			line_num = hbaline->linenumber;
	if (strcmp(name, "pamservice") == 0)
	{
#ifdef USE_PAM
		if (hbaline->auth_method != uaPAM)
		{
			ereport(elevel,
					(errmsg("pamservice authentication option can only be configured for authentication method \"pam\""),
					 errcontext("line %d of configuration file \"%s\"",
								line_num, HbaFileName)));
			*err_msg = "pamservice authentication option can only be configured for authentication method \"pam\"";
		}
		else
			hbaline->pamservice = pstrdup(val);
#else
		ereport(elevel,
				(errmsg("pamservice authentication option can only be configured for authentication method \"pam\""),
				 errhint("Compile with --with-pam to use PAM authentication."),
				 errcontext("line %d of configuration file \"%s\"",
							line_num, HbaFileName)));
		*err_msg = "pamservice authentication option cannot be used because PAM is not supported by this build";

#endif
	}
	else if (strcmp(name, "pam_use_hostname") == 0)
	{
#ifdef USE_PAM
		if (hbaline->auth_method != uaPAM)
		{
			ereport(elevel,
					(errmsg("pamservice authentication option can only be configured for authentication method \"pam\""),
					 errcontext("line %d of configuration file \"%s\"",
								line_num, HbaFileName)));
			*err_msg = "pamservice authentication option can only be configured for authentication method \"pam\"";
		}
		else
		{
			if (strcmp(val, "1") == 0)
				hbaline->pam_use_hostname = true;
			else
				hbaline->pam_use_hostname = false;

		}
#else
		ereport(elevel,
				(errmsg("pamservice authentication option can only be configured for authentication method \"pam\""),
				 errhint("Compile with --with-pam to use PAM authentication."),
				 errcontext("line %d of configuration file \"%s\"",
							line_num, HbaFileName)));
		*err_msg = "pamservice authentication option cannot be used because PAM is not supported by this build";

#endif
	}
	else
	{
		ereport(elevel,
				(errmsg("unrecognized authentication option name: \"%s\"",
						name),
				 errcontext("line %d of configuration file \"%s\"",
							line_num, HbaFileName)));
		*err_msg = psprintf("unrecognized authentication option name: \"%s\"",
							name);
		return false;
	}
	return true;
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


        switch (frontend->pool_hba->auth_method)
        {
			case uaImplicitReject:
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

	switch (frontend->pool_hba->auth_method)
	{
		case uaImplicitReject:
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

	pool_sigset_t oldmask;

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
*	Scan the pre-parsed hba file, looking for a match to the port's connection
*	request.
*/
static bool
check_hba(POOL_CONNECTION *frontend)
{
	ListCell   *line;
	HbaLine    *hba;
	MemoryContext oldcxt;

	if (parsed_hba_lines == NULL)
		return false;

	foreach(line, parsed_hba_lines)
	{
		hba = (HbaLine *) lfirst(line);

		/* Check connection type */
		if (hba->conntype == ctLocal)
		{
			if (!IS_AF_UNIX(frontend->raddr.addr.ss_family))
				continue;
		}
		else
		{
			if (IS_AF_UNIX(frontend->raddr.addr.ss_family))
				continue;

			/* Check SSL state */
#ifdef USE_SSL
			if (frontend->ssl)
			{
				/* Connection is SSL, match both "host" and "hostssl" */
				if (hba->conntype == ctHostNoSSL)
					continue;
			}
			else
#endif
			{
				/* Connection is not SSL, match both "host" and "hostnossl" */
				if (hba->conntype == ctHostSSL)
					continue;
			}

			/* Check IP address */
			switch (hba->ip_cmp_method)
			{
				case ipCmpMask:
					if (hba->hostname)
					{
						if (!check_hostname(frontend,
											hba->hostname))
							continue;
					}
					else
					{
						if (!check_ip(&frontend->raddr,
									  (struct sockaddr *) &hba->addr,
									  (struct sockaddr *) &hba->mask))
							continue;
					}
					break;
				case ipCmpAll:
					break;
				case ipCmpSameHost:
				case ipCmpSameNet:
					if (!check_same_host_or_net(&frontend->raddr,
												hba->ip_cmp_method))
						continue;
					break;
				default:
					/* shouldn't get here, but deem it no-match if so */
					continue;
			}
		}

		/* Check database and role */
		if (!check_db(frontend->database, frontend->username, hba->databases))
			continue;

		if (!check_user(frontend->username, hba->users))
			continue;

		/* Found a record that matched! */
		frontend->pool_hba = hba;
		return true;
	}

	/* If no matching entry was found, then implicitly reject. */
	oldcxt = MemoryContextSwitchTo(ProcessLoopContext);
	hba = palloc0(sizeof(HbaLine));
	MemoryContextSwitchTo(oldcxt);
	hba->auth_method = uaImplicitReject;
	frontend->pool_hba = hba;
	return true;
}

static bool
ipv4eq(struct sockaddr_in *a, struct sockaddr_in *b)
{
	return (a->sin_addr.s_addr == b->sin_addr.s_addr);
}


static bool
ipv6eq(struct sockaddr_in6 *a, struct sockaddr_in6 *b)
{
	int			i;

	for (i = 0; i < 16; i++)
		if (a->sin6_addr.s6_addr[i] != b->sin6_addr.s6_addr[i])
			return false;

	return true;
}


/*
 * Check whether host name matches pattern.
 */
static bool
hostname_match(const char *pattern, const char *actual_hostname)
{
	if (pattern[0] == '.')		/* suffix match */
	{
		size_t		plen = strlen(pattern);
		size_t		hlen = strlen(actual_hostname);

		if (hlen < plen)
			return false;

		return (strcasecmp(pattern, actual_hostname + (hlen - plen)) == 0);
	}
	else
		return (strcasecmp(pattern, actual_hostname) == 0);
}

/*
 * Check to see if a connecting IP matches a given host name.
 */
static bool
check_hostname(POOL_CONNECTION *frontend, const char *hostname)
{
	struct addrinfo *gai_result,
	*gai;
	int			ret;
	bool		found;

	/* Quick out if remote host name already known bad */
	if (frontend->remote_hostname_resolv < 0)
		return false;

	/* Lookup remote host name if not already done */
	if (!frontend->remote_hostname)
	{
		char		remote_hostname[NI_MAXHOST];

		ret = getnameinfo_all(&frontend->raddr.addr, frontend->raddr.salen,
								 remote_hostname, sizeof(remote_hostname),
								 NULL, 0,
								 NI_NAMEREQD);
		if (ret != 0)
		{
			/* remember failure; don't complain in the Pgpool-II log yet */
			frontend->remote_hostname_resolv = -2;
			//frontend->remote_hostname_errcode = ret;
			return false;
		}

		frontend->remote_hostname = pstrdup(remote_hostname);
	}

	/* Now see if remote host name matches this pg_hba line */
	if (!hostname_match(hostname, frontend->remote_hostname))
		return false;

	/* If we already verified the forward lookup, we're done */
	if (frontend->remote_hostname_resolv == +1)
		return true;

	/* Lookup IP from host name and check against original IP */
	ret = getaddrinfo(frontend->remote_hostname, NULL, NULL, &gai_result);
	if (ret != 0)
	{
		/* remember failure; don't complain in the postmaster log yet */
		frontend->remote_hostname_resolv = -2;
		//frontend->remote_hostname_errcode = ret;
		return false;
	}

	found = false;
	for (gai = gai_result; gai; gai = gai->ai_next)
	{
		if (gai->ai_addr->sa_family == frontend->raddr.addr.ss_family)
		{
			if (gai->ai_addr->sa_family == AF_INET)
			{
				if (ipv4eq((struct sockaddr_in *) gai->ai_addr,
						   (struct sockaddr_in *) &frontend->raddr.addr))
				{
					found = true;
					break;
				}
			}
			else if (gai->ai_addr->sa_family == AF_INET6)
			{
				if (ipv6eq((struct sockaddr_in6 *) gai->ai_addr,
						   (struct sockaddr_in6 *) &frontend->raddr.addr))
				{
					found = true;
					break;
				}
			}
		}
	}

	if (gai_result)
		freeaddrinfo(gai_result);

	if (!found)
		ereport(DEBUG2,
				(errmsg("pool_hba.conf host name \"%s\" rejected because address resolution did not return a match with IP address of client",
			 hostname)));

	frontend->remote_hostname_resolv = found ? +1 : -1;

	return found;
}
/*
 * pg_foreach_ifaddr callback: does client addr match this machine interface?
 */
static void
check_network_callback(struct sockaddr *addr, struct sockaddr *netmask,
					   void *cb_data)
{
	check_network_data *cn = (check_network_data *) cb_data;
	struct sockaddr_storage mask;

	/* Already found a match? */
	if (cn->result)
		return;

	if (cn->method == ipCmpSameHost)
	{
		/* Make an all-ones netmask of appropriate length for family */
		SockAddr_cidr_mask(&mask, NULL, addr->sa_family);
		cn->result = check_ip(cn->raddr, addr, (struct sockaddr *) &mask);
	}
	else
	{
		/* Use the netmask of the interface itself */
		cn->result = check_ip(cn->raddr, addr, netmask);
	}
}

/*
 * Use pg_foreach_ifaddr to check a samehost or samenet match
 */
static bool
check_same_host_or_net(SockAddr *raddr, IPCompareMethod method)
{
	check_network_data cn;

	cn.method = method;
	cn.raddr = raddr;
	cn.result = false;

	errno = 0;
	if (pg_foreach_ifaddr(check_network_callback, &cn) < 0)
	{
		elog(LOG, "error enumerating network interfaces: %m");
		return false;
	}

	return cn.result;
}
/*
 * Check to see if a connecting IP matches the given address and netmask.
 */
static bool
check_ip(SockAddr *raddr, struct sockaddr *addr, struct sockaddr *mask)
{
	if (raddr->addr.ss_family == addr->sa_family &&
		rangeSockAddr(&raddr->addr,
						  (struct sockaddr_storage *) addr,
						  (struct sockaddr_storage *) mask))
		return true;
	return false;
}


/*
 * Check comma user list for a specific user, handle group names.
 */
static bool check_user(char *user, List *tokens)
{
	ListCell   *cell;
	HbaToken   *tok;

	foreach(cell, tokens)
	{
		tok = lfirst(cell);
		if (!tok->quoted && tok->string[0] == '+')
		{
			/*
			 * pgpool cannot accept groups. commented lines below are the
			 * original code.
			 */
			ereport(LOG,
					(errmsg("group token \"+\" is not supported by Pgpool-II")));
			return false;
		}
		else if (token_matches(tok, user) ||
				 token_is_keyword(tok, "all"))
			return true;
	}
	return false;

}



/*
 * Check to see if db/user combination matches param string.
 */

static bool
check_db(const char *dbname, const char *user, List *tokens)
{
	ListCell   *cell;
	HbaToken   *tok;

	foreach(cell, tokens)
	{
		tok = lfirst(cell);
		if (token_is_keyword(tok, "all"))
			return true;
		else if (token_is_keyword(tok, "sameuser"))
		{
			if (strcmp(dbname, user) == 0)
				return true;
		}
		else if (token_is_keyword(tok, "samegroup") ||
				 token_is_keyword(tok, "samerole"))
		{
			ereport(LOG,
					(errmsg("group tokens \"samegroup\" and \"samerole\" are not supported by Pgpool-II")));
			return false;
		}
		else if (token_matches(tok, dbname))
			return true;
	}
	return false;
}

/*
 * tokenize_inc_file
 *		Expand a file included from another file into an hba "field"
 *
 * Opens and tokenises a file included from another HBA config file with @,
 * and returns all values found therein as a flat list of HbaTokens.  If a
 * @-token is found, recursively expand it.  The newly read tokens are
 * appended to "tokens" (so that foo,bar,@baz does what you expect).
 * All new tokens are allocated in caller's memory context.
 *
 * In event of an error, log a message at ereport level elevel, and also
 * set *err_msg to a string describing the error.  Note that the result
 * may be non-NIL anyway, so *err_msg must be tested to determine whether
 * there was an error.
 */
static List *
tokenize_inc_file(List *tokens,
				  const char *outer_filename,
				  const char *inc_filename,
				  int elevel,
				  char **err_msg)
{
	char	   *inc_fullname;
	FILE	   *inc_file;
	List	   *inc_lines;
	ListCell   *inc_line;
	MemoryContext linecxt;

	if (is_absolute_path(inc_filename))
	{
		/* absolute path is taken as-is */
		inc_fullname = pstrdup(inc_filename);
	}
	else
	{
		/* relative path is relative to dir of calling file */
		inc_fullname = (char *) palloc(strlen(outer_filename) + 1 +
									   strlen(inc_filename) + 1);
		strcpy(inc_fullname, outer_filename);
		get_parent_directory(inc_fullname);
		join_path_components(inc_fullname, inc_fullname, inc_filename);
		canonicalize_path(inc_fullname);
	}

	inc_file = fopen(inc_fullname, "r");
	if (inc_file == NULL)
	{
		int			save_errno = errno;

		ereport(elevel,
				(errmsg("could not open secondary authentication file \"@%s\" as \"%s\": %m",
						inc_filename, inc_fullname)));
		*err_msg = psprintf("could not open secondary authentication file \"@%s\" as \"%s\": %s",
							inc_filename, inc_fullname, strerror(save_errno));
		pfree(inc_fullname);
		return tokens;
	}

	/* There is possible recursion here if the file contains @ */
	linecxt = tokenize_file(inc_fullname, inc_file, &inc_lines, elevel);
	fclose(inc_file);

	/* Copy all tokens found in the file and append to the tokens list */
	foreach(inc_line, inc_lines)
	{
		TokenizedLine *tok_line = (TokenizedLine *) lfirst(inc_line);
		ListCell   *inc_field;

		/* If any line has an error, propagate that up to caller */
		if (tok_line->err_msg)
		{
			*err_msg = pstrdup(tok_line->err_msg);
			break;
		}

		foreach(inc_field, tok_line->fields)
		{
			List	   *inc_tokens = lfirst(inc_field);
			ListCell   *inc_token;

			foreach(inc_token, inc_tokens)
			{
				HbaToken   *token = lfirst(inc_token);

				tokens = lappend(tokens, copy_hba_token(token));
			}
		}
	}

	MemoryContextDelete(linecxt);
	return tokens;
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
 * Tokenize the given file.
 *
 * The output is a list of TokenizedLine structs; see struct definition above.
 *
 * filename: the absolute path to the target file
 * file: the already-opened target file
 * tok_lines: receives output list
 * elevel: message logging level
 *
 * Errors are reported by logging messages at ereport level elevel and by
 * adding TokenizedLine structs containing non-null err_msg fields to the
 * output list.
 *
 * Return value is a memory context which contains all memory allocated by
 * this function (it's a child of caller's context).
 */
static MemoryContext
tokenize_file(const char *filename, FILE *file, List **tok_lines, int elevel)
{
	int			line_number = 1;
	MemoryContext linecxt;
	MemoryContext oldcxt;

	linecxt = AllocSetContextCreate(CurrentMemoryContext,
									"tokenize_file",
									ALLOCSET_SMALL_SIZES);
	oldcxt = MemoryContextSwitchTo(linecxt);

	*tok_lines = NIL;

	while (!feof(file) && !ferror(file))
	{
		char		rawline[MAX_LINE];
		char	   *lineptr;
		List	   *current_line = NIL;
		char	   *err_msg = NULL;

		if (!fgets(rawline, sizeof(rawline), file))
		{
			int			save_errno = errno;

			if (!ferror(file))
				break;			/* normal EOF */
			/* I/O error! */
			ereport(elevel,
					(errmsg("could not read file \"%s\": %m", filename)));
			err_msg = psprintf("could not read file \"%s\": %s",
							   filename, strerror(save_errno));
			rawline[0] = '\0';
		}
		if (strlen(rawline) == MAX_LINE - 1)
		{
			/* Line too long! */
			ereport(elevel,
					(errcode(ERRCODE_CONFIG_FILE_ERROR),
					 errmsg("authentication file line too long"),
					 errcontext("line %d of configuration file \"%s\"",
								line_number, filename)));
			err_msg = "authentication file line too long";
		}

		/* Strip trailing linebreak from rawline */
		lineptr = rawline + strlen(rawline) - 1;
		while (lineptr >= rawline && (*lineptr == '\n' || *lineptr == '\r'))
			*lineptr-- = '\0';

		/* Parse fields */
		lineptr = rawline;
		while (*lineptr && err_msg == NULL)
		{
			List	   *current_field;

			current_field = next_field_expand(filename, &lineptr,
											  elevel, &err_msg);
			/* add field to line, unless we are at EOL or comment start */
			if (current_field != NIL)
				current_line = lappend(current_line, current_field);
		}

		/* Reached EOL; emit line to TokenizedLine list unless it's boring */
		if (current_line != NIL || err_msg != NULL)
		{
			TokenizedLine *tok_line;

			tok_line = (TokenizedLine *) palloc(sizeof(TokenizedLine));
			tok_line->fields = current_line;
			tok_line->line_num = line_number;
			tok_line->raw_line = pstrdup(rawline);
			tok_line->err_msg = err_msg;
			*tok_lines = lappend(*tok_lines, tok_line);
		}

		line_number++;
	}

	MemoryContextSwitchTo(oldcxt);

	return linecxt;
}


/*
 * Tokenize one HBA field from a line, handling file inclusion and comma lists.
 *
 * filename: current file's pathname (needed to resolve relative pathnames)
 * *lineptr: current line pointer, which will be advanced past field
 *
 * In event of an error, log a message at ereport level elevel, and also
 * set *err_msg to a string describing the error.  Note that the result
 * may be non-NIL anyway, so *err_msg must be tested to determine whether
 * there was an error.
 *
 * The result is a List of HbaToken structs, one for each token in the field,
 * or NIL if we reached EOL.
 */
static List *
next_field_expand(const char *filename, char **lineptr,
				  int elevel, char **err_msg)
{
	char		buf[MAX_TOKEN];
	bool		trailing_comma;
	bool		initial_quote;
	List	   *tokens = NIL;

	do
	{
		if (!next_token(lineptr, buf, sizeof(buf),
						&initial_quote, &trailing_comma,
						elevel, err_msg))
			break;

		/* Is this referencing a file? */
		if (!initial_quote && buf[0] == '@' && buf[1] != '\0')
			tokens = tokenize_inc_file(tokens, filename, buf + 1,
									   elevel, err_msg);
		else
			tokens = lappend(tokens, make_hba_token(buf, initial_quote));
	} while (trailing_comma && (*err_msg == NULL));

	return tokens;
}

/*
 * Grab one token out of the string pointed to by *lineptr.
 *
 * Tokens are strings of non-blank
 * characters bounded by blank characters, commas, beginning of line, and
 * end of line. Blank means space or tab. Tokens can be delimited by
 * double quotes (this allows the inclusion of blanks, but not newlines).
 * Comments (started by an unquoted '#') are skipped.
 *
 * The token, if any, is returned at *buf (a buffer of size bufsz), and
 * *lineptr is advanced past the token.
 *
 * Also, we set *initial_quote to indicate whether there was quoting before
 * the first character.  (We use that to prevent "@x" from being treated
 * as a file inclusion request.  Note that @"x" should be so treated;
 * we want to allow that to support embedded spaces in file paths.)
 *
 * We set *terminating_comma to indicate whether the token is terminated by a
 * comma (which is not returned).
 *
 * In event of an error, log a message at ereport level elevel, and also
 * set *err_msg to a string describing the error.  Currently the only
 * possible error is token too long for buf.
 *
 * If successful: store null-terminated token at *buf and return TRUE.
 * If no more tokens on line: set *buf = '\0' and return FALSE.
 * If error: fill buf with truncated or misformatted token and return FALSE.
 */
static bool
next_token(char **lineptr, char *buf, int bufsz,
		   bool *initial_quote, bool *terminating_comma,
		   int elevel, char **err_msg)
{
	int			c;
	char	   *start_buf = buf;
	char	   *end_buf = buf + (bufsz - 1);
	bool		in_quote = false;
	bool		was_quote = false;
	bool		saw_quote = false;

	Assert(end_buf > start_buf);

	*initial_quote = false;
	*terminating_comma = false;

	/* Move over any whitespace and commas preceding the next token */
	while ((c = (*(*lineptr)++)) != '\0' && (pg_isblank(c) || c == ','))
		;

	/*
	 * Build a token in buf of next characters up to EOL, unquoted comma, or
	 * unquoted whitespace.
	 */
	while (c != '\0' &&
		   (!pg_isblank(c) || in_quote))
	{
		/* skip comments to EOL */
		if (c == '#' && !in_quote)
		{
			while ((c = (*(*lineptr)++)) != '\0')
				;
			break;
		}

		if (buf >= end_buf)
		{
			*buf = '\0';
			ereport(elevel,
					(errcode(ERRCODE_CONFIG_FILE_ERROR),
					 errmsg("authentication file token too long, skipping: \"%s\"",
							start_buf)));
			*err_msg = "authentication file token too long";
			/* Discard remainder of line */
			while ((c = (*(*lineptr)++)) != '\0')
				;
			/* Un-eat the '\0', in case we're called again */
			(*lineptr)--;
			return false;
		}

		/* we do not pass back a terminating comma in the token */
		if (c == ',' && !in_quote)
		{
			*terminating_comma = true;
			break;
		}

		if (c != '"' || was_quote)
			*buf++ = c;

		/* Literal double-quote is two double-quotes */
		if (in_quote && c == '"')
			was_quote = !was_quote;
		else
			was_quote = false;

		if (c == '"')
		{
			in_quote = !in_quote;
			saw_quote = true;
			if (buf == start_buf)
				*initial_quote = true;
		}

		c = *(*lineptr)++;
	}

	/*
	 * Un-eat the char right after the token (critical in case it is '\0',
	 * else next call will read past end of string).
	 */
	(*lineptr)--;

	*buf = '\0';

	return (saw_quote || buf > start_buf);
}

/*
 * Construct a palloc'd HbaToken struct, copying the given string.
 */
static HbaToken *
make_hba_token(const char *token, bool quoted)
{
	HbaToken   *hbatoken;
	int			toklen;

	toklen = strlen(token);
	/* we copy string into same palloc block as the struct */
	hbatoken = (HbaToken *) palloc(sizeof(HbaToken) + toklen + 1);
	hbatoken->string = (char *) hbatoken + sizeof(HbaToken);
	hbatoken->quoted = quoted;
	memcpy(hbatoken->string, token, toklen + 1);

	return hbatoken;
}

/*
 * Copy a HbaToken struct into freshly palloc'd memory.
 */
static HbaToken *
copy_hba_token(HbaToken *in)
{
	HbaToken   *out = make_hba_token(in->string, in->quoted);

	return out;
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
     * Do not use Palloc and freinds to allocate this memory,
     * Since the PAM library will be freeing this memory who
     * knowns nothing about our MemoryManager
	 */
	*resp = calloc(num_msg, sizeof(struct pam_response));

	(*resp)[0].resp = strdup((char *) appdata_ptr);
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
	if (frontend->pool_hba->pamservice && frontend->pool_hba->pamservice[0] != '\0')
		retval = pam_start(frontend->pool_hba->pamservice, "pgpool@",
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
                    errdetail("pool_passwd file does not contain an entry for \"%s\"",username)));


	/*
	 * Ok for now. Actual authentication will be performed later.
	 */
	return POOL_CONTINUE;
}
