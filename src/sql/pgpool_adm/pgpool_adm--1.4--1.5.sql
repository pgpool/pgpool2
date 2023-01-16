/* contrib/pgpool_adm/pgpool_adm--1.4--1.5.sql */

-- complain if script is sourced in psql, rather than via ALTER EXTENSION
\echo Use "ALTER EXTENSION pgpool_adm UPDATE TO '1.5'" to load this file. \quit

/**
 * input parameters: host, port, username, password
 */
ALTER EXTENSION pgpool_adm DROP FUNCTION pcp_pool_status(IN host text, IN port integer, IN username text, IN password text, OUT item text, OUT value text, OUT description text);

DROP FUNCTION pcp_pool_status(IN host text, IN port integer, IN username text, IN password text, OUT item text, OUT value text, OUT description text);

CREATE FUNCTION pcp_pool_status(IN host text, IN port integer, IN username text, IN password text, OUT item text, OUT value text, OUT description text)
RETURNS SETOF record
AS 'MODULE_PATHNAME', '_pcp_pool_status'
LANGUAGE C VOLATILE STRICT;

/**
 * input parameters: pcp_server
 */
ALTER EXTENSION pgpool_adm DROP FUNCTION pcp_pool_status(IN pcp_server text, OUT item text, OUT value text, OUT description text);

DROP FUNCTION pcp_pool_status(IN pcp_server text, OUT item text, OUT value text, OUT description text);

CREATE FUNCTION pcp_pool_status(IN pcp_server text, OUT item text, OUT value text, OUT description text)
RETURNS SETOF record
AS 'MODULE_PATHNAME', '_pcp_pool_status'
LANGUAGE C VOLATILE STRICT;
