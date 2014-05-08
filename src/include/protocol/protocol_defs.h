#ifndef pgpool_protocol_defs_h
#define pgpool_protocol_defs_h

#define SEQUENCETABLEQUERY "SELECT d.adsrc FROM pg_catalog.pg_class c, pg_catalog.pg_attribute a LEFT JOIN pg_catalog.pg_attrdef d ON (a.attrelid = d.adrelid AND a.attnum = d.adnum) WHERE c.oid = a.attrelid AND a.attnum >= 1 AND a.attisdropped = 'f' AND c.relname = '%s' AND d.adsrc ~ 'nextval'"

#define SEQUENCETABLEQUERY2 "SELECT d.adsrc FROM pg_catalog.pg_class c, pg_catalog.pg_attribute a LEFT JOIN pg_catalog.pg_attrdef d ON (a.attrelid = d.adrelid AND a.attnum = d.adnum) WHERE c.oid = a.attrelid AND a.attnum >= 1 AND a.attisdropped = 'f' AND c.oid = pgpool_regclass('%s') AND d.adsrc ~ 'nextval'"

#define SEQUENCETABLEQUERY3 "SELECT d.adsrc FROM pg_catalog.pg_class c, pg_catalog.pg_attribute a LEFT JOIN pg_catalog.pg_attrdef d ON (a.attrelid = d.adrelid AND a.attnum = d.adnum) WHERE c.oid = a.attrelid AND a.attnum >= 1 AND a.attisdropped = 'f' AND c.oid = to_regclass('%s') AND d.adsrc ~ 'nextval'"

/* query to lock a row by only the specified table name without regard to the schema */
#define ROWLOCKQUERY "SELECT 1 FROM pgpool_catalog.insert_lock WHERE reloid = (SELECT oid FROM pg_catalog.pg_class WHERE relname = '%s' ORDER BY oid LIMIT 1) FOR UPDATE"

#define ROWLOCKQUERY2 "SELECT 1 FROM pgpool_catalog.insert_lock WHERE reloid = pgpool_regclass('%s') FOR UPDATE"

#define MAX_NAME_LEN 128


#define HASINSERT_LOCKQUERY "SELECT count(*) FROM pg_catalog.pg_class c JOIN pg_catalog.pg_namespace n ON (c.relnamespace = n.oid) WHERE nspname = 'pgpool_catalog' AND relname = '%s'"


#endif
