/* contrib/pgpool_adm/pgpool_adm--1.5--1.6.sql */

-- complain if script is sourced in psql, rather than via ALTER EXTENSION
\echo Use "ALTER EXTENSION pgpool_adm UPDATE TO '1.6'" to load this file. \quit

/**
 * input parameters: node_id, host, port, username, password
 */
CREATE FUNCTION pcp_proc_info(IN host text, IN port integer, IN username text, IN password text,
OUT database text,
OUT username text,
OUT start_time text,
OUT client_connection_count text,
OUT major text,
OUT minor text,
OUT backend_connection_time text,
OUT client_connection_time text,
OUT client_idle_duration text,
OUT client_disconnection_time text,
OUT pool_counter text,
OUT backend_pid text,
OUT connected text,
OUT pid text,
OUT backend_id text,
OUT status text,
OUT load_balance_node text,
OUT client_host text,
OUT client_port text,
OUT statement text)
RETURNS SETOF record
AS 'MODULE_PATHNAME', '_pcp_proc_info'
LANGUAGE C VOLATILE STRICT;

/**
 * input parameters: node_id, pcp_server
 */
CREATE FUNCTION pcp_proc_info(IN pcp_server text,
OUT database text,
OUT username text,
OUT start_time text,
OUT client_connection_count text,
OUT major text,
OUT minor text,
OUT backend_connection_time text,
OUT client_connection_time text,
OUT client_idle_duration text,
OUT client_disconnection_time text,
OUT pool_counter text,
OUT backend_pid text,
OUT connected text,
OUT pid text,
OUT backend_id text,
OUT status text,
OUT load_balance_node text,
OUT client_host text,
OUT client_port text,
OUT statement text)
RETURNS SETOF record
AS 'MODULE_PATHNAME', '_pcp_proc_info'
LANGUAGE C VOLATILE STRICT;

