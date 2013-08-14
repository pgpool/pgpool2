-- $Header$

INSERT INTO pgpool_catalog.replicate_def VALUES (
    'bench_parallel',
    'public',
    'branches',
    ARRAY['bid', 'bbalance', 'filler'],
    ARRAY['integer', 'integer', 'character(88)']
);

INSERT INTO pgpool_catalog.replicate_def VALUES (
    'bench_parallel',
    'public',
    'tellers',
    ARRAY['tid', 'bid', 'tbalance', 'filler'],
    ARRAY['integer', 'integer', 'integer', 'character(84)']
);

