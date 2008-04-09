-- $Header$

DROP TABLE pgpool_catalog.dist_def;
DROP TABLE pgpool_catalog.replicate_def;
DROP TABLE pgpool_catalog.query_cache;
DROP SCHEMA pgpool_catalog;


CREATE SCHEMA pgpool_catalog;

CREATE TABLE pgpool_catalog.dist_def(
	dbname TEXT,
	schema_name TEXT,
	table_name TEXT,
	col_name TEXT NOT NULL CHECK (col_name = ANY (col_list)),
	col_list TEXT[] NOT NULL,
	type_list TEXT[] NOT NULL,
	dist_def_func TEXT NOT NULL,
	PRIMARY KEY (dbname,schema_name,table_name)
);

CREATE TABLE pgpool_catalog.replicate_def(
	dbname TEXT,
	schema_name TEXT,
	table_name TEXT,
	col_list TEXT[] NOT NULL,
	type_list TEXT[] NOT NULL,
	PRIMARY KEY (dbname,schema_name,table_name)
);

CREATE TABLE pgpool_catalog.query_cache (
	hash TEXT,
	query TEXT,
	value bytea,
	dbname TEXT,
	create_time TIMESTAMP WITH TIME ZONE,
	PRIMARY KEY(hash, dbname)
);
