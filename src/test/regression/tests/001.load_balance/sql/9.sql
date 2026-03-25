\set SHOW_ALL_RESULTS off
-- primary = node 1 and standby = node 0
DROP TABLE  IF EXISTS t1;
-- create a persistent table
CREATE TABLE t1(i int);
-- wait for replicated
SELECT pg_sleep(1);
-- ordinary read only SELECT: load balance expected
SELECT * FROM t1;
DROP TABLE t1;
-- create temp table with the same name
CREATE TEMP TABLE t1(i int);
-- expected read from primary
SELECT * FROM t1;
