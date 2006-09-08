-- $Header$

INSERT INTO pgpool_catalog.dist_def VALUES (
    'bench_parallel',
    'public',
    'branches',
    'bid',
    ARRAY['bid', 'bbalance', 'filler'],
    ARRAY['integer', 'integer', 'character(88)'],
    'pgpool_catalog.dist_def_branches'
);

INSERT INTO pgpool_catalog.dist_def VALUES (
    'bench_parallel',
    'public',
    'tellers',
    'tid',
    ARRAY['tid', 'bid', 'tbalance', 'filler'],
    ARRAY['integer', 'integer', 'integer', 'character(84)'],
    'pgpool_catalog.dist_def_tellers'
);

INSERT INTO pgpool_catalog.dist_def VALUES (
    'bench_parallel',
    'public',
    'accounts',
    'aid',
    ARRAY['aid', 'bid', 'abalance', 'filler'],
    ARRAY['integer', 'integer', 'integer', 'character(84)'],
    'pgpool_catalog.dist_def_accounts'
);

INSERT INTO pgpool_catalog.dist_def VALUES (
    'bench_parallel',
    'public',
    'history',
    'tid',
    ARRAY['tid', 'bid', 'aid', 'delta', 'mtime', 'filler'],
    ARRAY['integer', 'integer', 'integer', 'integer', 'timestamp without time zone', 'character(22)'],
    'pgpool_catalog.dist_def_history'
);

CREATE OR REPLACE FUNCTION pgpool_catalog.dist_def_branches(anyelement)
RETURNS integer AS $$
    SELECT CASE WHEN $1 > 0 AND $1 <= 1 THEN 0
        WHEN $1 > 1 AND $1 <= 2 THEN 1
        ELSE 2
    END;
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION pgpool_catalog.dist_def_tellers(anyelement)
RETURNS integer AS $$
    SELECT CASE WHEN $1 > 0 AND $1 <= 10 THEN 0
        WHEN $1 > 10 AND $1 <= 20 THEN 1
        ELSE 2
    END;
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION pgpool_catalog.dist_def_accounts(anyelement)
RETURNS integer AS $$
    SELECT CASE WHEN $1 > 0 AND $1 <= 100000 THEN 0
        WHEN $1 > 100000 AND $1 <= 200000 THEN 1
        ELSE 2
    END;
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION pgpool_catalog.dist_def_history(anyelement)
RETURNS integer AS $$
    SELECT CASE WHEN $1 > 0 AND $1 <= 10 THEN 0
        WHEN $1 > 10 AND $1 <= 20 THEN 1
        ELSE 2
    END;
$$ LANGUAGE sql;
