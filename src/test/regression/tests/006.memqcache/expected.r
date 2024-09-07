--
-- testing an effect on a row level security enabled table and SET ROLE
--
CREATE TABLE users (user_name TEXT, data TEXT);
NOTICE:  DB node id: 0 statement: CREATE TABLE users (user_name TEXT, data TEXT);
NOTICE:  DB node id: 1 statement: CREATE TABLE users (user_name TEXT, data TEXT);
CREATE TABLE
INSERT INTO users VALUES('foo', 'foodata');
NOTICE:  DB node id: 0 statement: INSERT INTO users VALUES('foo', 'foodata');
NOTICE:  DB node id: 1 statement: INSERT INTO users VALUES('foo', 'foodata');
INSERT 0 1
INSERT INTO users VALUES('bar', 'bardata');
NOTICE:  DB node id: 0 statement: INSERT INTO users VALUES('bar', 'bardata');
NOTICE:  DB node id: 1 statement: INSERT INTO users VALUES('bar', 'bardata');
INSERT 0 1
ALTER TABLE users ENABLE ROW LEVEL SECURITY;
NOTICE:  DB node id: 0 statement: ALTER TABLE users ENABLE ROW LEVEL SECURITY;
NOTICE:  DB node id: 1 statement: ALTER TABLE users ENABLE ROW LEVEL SECURITY;
ALTER TABLE
CREATE POLICY user_policy ON users USING (user_name = CURRENT_USER);
NOTICE:  DB node id: 0 statement: CREATE POLICY user_policy ON users USING (user_name = CURRENT_USER);
NOTICE:  DB node id: 1 statement: CREATE POLICY user_policy ON users USING (user_name = CURRENT_USER);
CREATE POLICY
GRANT SELECT ON users TO foo;
NOTICE:  DB node id: 0 statement: GRANT SELECT ON users TO foo;
NOTICE:  DB node id: 1 statement: GRANT SELECT ON users TO foo;
GRANT
GRANT SELECT ON users TO bar;
NOTICE:  DB node id: 0 statement: GRANT SELECT ON users TO bar;
NOTICE:  DB node id: 1 statement: GRANT SELECT ON users TO bar;
GRANT
SET ROLE TO foo;
NOTICE:  DB node id: 0 statement: SET ROLE TO foo;
NOTICE:  DB node id: 1 statement: SET ROLE TO foo;
SET
-- run SELECT as foo. Only user_name = 'foo' data expected.
SELECT * FROM users;
NOTICE:  DB node id: 0 statement: SELECT * FROM users;
 user_name |  data   
-----------+---------
 foo       | foodata
(1 row)

RESET ROLE;
NOTICE:  DB node id: 0 statement: RESET ROLE;
NOTICE:  DB node id: 1 statement: RESET ROLE;
RESET
SET ROLE TO bar;
NOTICE:  DB node id: 0 statement: SET ROLE TO bar;
NOTICE:  DB node id: 1 statement: SET ROLE TO bar;
SET
-- run SELECT as bar. Only user_name = 'bar' data expected.
SELECT * FROM users;
NOTICE:  DB node id: 0 statement: SELECT * FROM users;
 user_name |  data   
-----------+---------
 bar       | bardata
(1 row)

--
-- testing row security with row_security = off
--
SET ROW_SECURITY TO off;
NOTICE:  DB node id: 0 statement: SET ROW_SECURITY TO off;
NOTICE:  DB node id: 1 statement: SET ROW_SECURITY TO off;
SET
-- Error expected
SELECT * FROM users;
NOTICE:  DB node id: 0 statement: SELECT * FROM users;
ERROR:  query would be affected by row-level security policy for table "users"
--
-- testing SET ROLE
--
CREATE TABLE footable(t text);
NOTICE:  DB node id: 0 statement: CREATE TABLE footable(t text);
NOTICE:  DB node id: 1 statement: CREATE TABLE footable(t text);
CREATE TABLE
INSERT INTO footable VALUES('foo');
NOTICE:  DB node id: 0 statement: INSERT INTO footable VALUES('foo');
NOTICE:  DB node id: 1 statement: INSERT INTO footable VALUES('foo');
INSERT 0 1
GRANT SELECT ON footable TO foo;
NOTICE:  DB node id: 0 statement: GRANT SELECT ON footable TO foo;
NOTICE:  DB node id: 1 statement: GRANT SELECT ON footable TO foo;
GRANT
GRANT INSERT ON footable TO foo;
NOTICE:  DB node id: 0 statement: GRANT INSERT ON footable TO foo;
NOTICE:  DB node id: 1 statement: GRANT INSERT ON footable TO foo;
GRANT
GRANT UPDATE ON footable TO foo;
NOTICE:  DB node id: 0 statement: GRANT UPDATE ON footable TO foo;
NOTICE:  DB node id: 1 statement: GRANT UPDATE ON footable TO foo;
GRANT
GRANT DELETE ON footable TO foo;
NOTICE:  DB node id: 0 statement: GRANT DELETE ON footable TO foo;
NOTICE:  DB node id: 1 statement: GRANT DELETE ON footable TO foo;
GRANT
SELECT * FROM footable;
NOTICE:  DB node id: 0 statement: SELECT * FROM footable;
  t  
-----
 foo
(1 row)

SET ROLE TO bar;
NOTICE:  DB node id: 0 statement: SET ROLE TO bar;
NOTICE:  DB node id: 1 statement: SET ROLE TO bar;
SET
-- run SELECT as bar. Permission denied is expected.
SELECT * FROM footable;
NOTICE:  DB node id: 0 statement: SELECT * FROM footable;
ERROR:  permission denied for table footable
--
-- testing SESSION AUTHORIZATION
--
SET SESSION AUTHORIZATION bar;
NOTICE:  DB node id: 0 statement: SET SESSION AUTHORIZATION bar;
NOTICE:  DB node id: 1 statement: SET SESSION AUTHORIZATION bar;
SET
-- run SELECT as bar. Permission denied is expected.
SELECT * FROM footable;
NOTICE:  DB node id: 0 statement: SELECT * FROM footable;
ERROR:  permission denied for table footable
--
-- testing SET ROLE. Make sure that query cache is not
-- created.
--
-- create cache
SELECT * FROM footable;
  t  
-----
 foo
(1 row)

-- change role
SET ROLE TO foo;
NOTICE:  DB node id: 0 statement: SET ROLE TO foo;
NOTICE:  DB node id: 1 statement: SET ROLE TO foo;
SET
-- run SELECT as foo to make sure that cache is not used.
-- If query cache was created we will NOT see
-- "NOTICE: DB node id: 1 statement: SELECT ..."
SELECT * FROM footable;
NOTICE:  DB node id: 0 statement: SELECT * FROM footable;
  t  
-----
 foo
(1 row)

-- Modify footable to see cache invalidation works even after SET ROLE.
INSERT INTO footable VALUES ('foo1');
NOTICE:  DB node id: 0 statement: INSERT INTO footable VALUES ('foo1');
NOTICE:  DB node id: 1 statement: INSERT INTO footable VALUES ('foo1');
INSERT 0 1
-- restore ROLE
RESET ROLE;
NOTICE:  DB node id: 0 statement: RESET ROLE;
NOTICE:  DB node id: 1 statement: RESET ROLE;
RESET
-- Make sure cache was invalidated.
SELECT * FROM footable;
NOTICE:  DB node id: 0 statement: SELECT * FROM footable;
  t   
------
 foo
 foo1
(2 rows)

--
-- explicit transaction case
--
-- create cache
SELECT * FROM footable;
NOTICE:  DB node id: 0 statement: SELECT * FROM footable;
  t   
------
 foo
 foo1
(2 rows)

SELECT * FROM footable;
  t   
------
 foo
 foo1
(2 rows)

-- change role
SET ROLE TO foo;
NOTICE:  DB node id: 0 statement: SET ROLE TO foo;
NOTICE:  DB node id: 1 statement: SET ROLE TO foo;
SET
BEGIN;
NOTICE:  DB node id: 0 statement: BEGIN;
NOTICE:  DB node id: 1 statement: BEGIN;
BEGIN
-- run SELECT as foo to make sure that cache is not used.
-- If query cache was created we will NOT see
-- "NOTICE: DB node id: 1 statement: SELECT ..."
SELECT * FROM footable;
NOTICE:  DB node id: 0 statement: SELECT * FROM footable;
  t   
------
 foo
 foo1
(2 rows)

-- Modify footable to see cache invalidation works even after SET ROLE.
INSERT INTO footable VALUES ('foo2');
NOTICE:  DB node id: 0 statement: INSERT INTO footable VALUES ('foo2');
NOTICE:  DB node id: 1 statement: INSERT INTO footable VALUES ('foo2');
INSERT 0 1
END;
NOTICE:  DB node id: 1 statement: END;
NOTICE:  DB node id: 0 statement: END;
COMMIT
-- Make sure cache was invalidated.
SELECT * FROM footable;
NOTICE:  DB node id: 0 statement: SELECT * FROM footable;
  t   
------
 foo
 foo1
 foo2
(3 rows)

--
-- explicit transaction abort case
--
-- create cache
SELECT * FROM footable;
  t   
------
 foo
 foo1
 foo2
(3 rows)

SELECT * FROM footable;
  t   
------
 foo
 foo1
 foo2
(3 rows)

-- change role
SET ROLE TO foo;
NOTICE:  DB node id: 0 statement: SET ROLE TO foo;
NOTICE:  DB node id: 1 statement: SET ROLE TO foo;
SET
BEGIN;
NOTICE:  DB node id: 0 statement: BEGIN;
NOTICE:  DB node id: 1 statement: BEGIN;
BEGIN
-- run SELECT as foo to make sure that cache is not used.
-- If query cache was created we will NOT see
-- "NOTICE: DB node id: 0 statement: SELECT ..."
SELECT * FROM footable;
NOTICE:  DB node id: 0 statement: SELECT * FROM footable;
  t   
------
 foo
 foo1
 foo2
(3 rows)

-- Modify footable to see cache invalidation works even after SET ROLE.
INSERT INTO footable VALUES ('foo3');
NOTICE:  DB node id: 0 statement: INSERT INTO footable VALUES ('foo3');
NOTICE:  DB node id: 1 statement: INSERT INTO footable VALUES ('foo3');
INSERT 0 1
SELECT * FROM footable;
NOTICE:  DB node id: 0 statement: SELECT * FROM footable;
  t   
------
 foo
 foo1
 foo2
 foo3
(4 rows)

ABORT;
NOTICE:  DB node id: 1 statement: ABORT;
NOTICE:  DB node id: 0 statement: ABORT;
ROLLBACK
-- Make sure we don't see 'foo3' row.
SELECT * FROM footable;
  t   
------
 foo
 foo1
 foo2
(3 rows)

-- Make sure we don't see 'foo3' row.
SELECT * FROM footable;
  t   
------
 foo
 foo1
 foo2
(3 rows)

--
-- Testing REVOKE
--
-- create cache
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i 
---
 2
(1 row)

-- REVOKE
REVOKE SELECT ON t1 FROM foo;
NOTICE:  DB node id: 0 statement: REVOKE SELECT ON t1 FROM foo;
NOTICE:  DB node id: 1 statement: REVOKE SELECT ON t1 FROM foo;
REVOKE
SET ROLE TO foo;
NOTICE:  DB node id: 0 statement: SET ROLE TO foo;
NOTICE:  DB node id: 1 statement: SET ROLE TO foo;
SET
-- Make sure foo cannot SELECT t1
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
ERROR:  permission denied for table t1
RESET ROLE;
NOTICE:  DB node id: 0 statement: RESET ROLE;
NOTICE:  DB node id: 1 statement: RESET ROLE;
RESET
-- GRANT again
GRANT SELECT ON t1 TO foo;
NOTICE:  DB node id: 0 statement: GRANT SELECT ON t1 TO foo;
NOTICE:  DB node id: 1 statement: GRANT SELECT ON t1 TO foo;
GRANT
-- explicit transaction case
BEGIN;
NOTICE:  DB node id: 0 statement: BEGIN;
NOTICE:  DB node id: 1 statement: BEGIN;
BEGIN
-- REVOKE
REVOKE SELECT ON t1 FROM foo;
NOTICE:  DB node id: 0 statement: REVOKE SELECT ON t1 FROM foo;
NOTICE:  DB node id: 1 statement: REVOKE SELECT ON t1 FROM foo;
REVOKE
SET ROLE TO foo;
NOTICE:  DB node id: 0 statement: SET ROLE TO foo;
NOTICE:  DB node id: 1 statement: SET ROLE TO foo;
SET
-- Make sure foo cannot SELECT t1
-- (thus REVOKE will be rollbacked )
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
ERROR:  permission denied for table t1
END;
NOTICE:  DB node id: 1 statement: END;
NOTICE:  DB node id: 0 statement: END;
ROLLBACK
SET ROLE TO foo;
NOTICE:  DB node id: 0 statement: SET ROLE TO foo;
NOTICE:  DB node id: 1 statement: SET ROLE TO foo;
SET
-- because REVOKE is rolled back, foo should be able to access t1
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i 
---
 2
(1 row)

--
-- REVOKE is executed on another session case
--
-- Make sure to create cache
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i 
---
 2
(1 row)

SELECT * FROM t1;
 i 
---
 2
(1 row)

-- execute REVOKE
REVOKE SELECT ON t1 FROM foo
NOTICE:  DB node id: 0 statement: REVOKE SELECT ON t1 FROM foo
NOTICE:  DB node id: 1 statement: REVOKE SELECT ON t1 FROM foo
REVOKE
-- Make sure this does not access cache
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i 
---
 2
(1 row)

--
-- ALTER ROLE BYPASSRLS case
--
ALTER ROLE foo BYPASSRLS;
NOTICE:  DB node id: 0 statement: ALTER ROLE foo BYPASSRLS;
NOTICE:  DB node id: 1 statement: ALTER ROLE foo BYPASSRLS;
ALTER ROLE
SET ROLE TO foo;
NOTICE:  DB node id: 0 statement: SET ROLE TO foo;
NOTICE:  DB node id: 1 statement: SET ROLE TO foo;
SET
-- expect to ignore cache and result is all rows
SELECT * FROM users;
NOTICE:  DB node id: 0 statement: SELECT * FROM users;
 user_name |  data   
-----------+---------
 foo       | foodata
 bar       | bardata
(2 rows)

RESET ROLE;
NOTICE:  DB node id: 0 statement: RESET ROLE;
NOTICE:  DB node id: 1 statement: RESET ROLE;
RESET
ALTER ROLE foo NOBYPASSRLS;
NOTICE:  DB node id: 0 statement: ALTER ROLE foo NOBYPASSRLS;
NOTICE:  DB node id: 1 statement: ALTER ROLE foo NOBYPASSRLS;
ALTER ROLE
SET ROLE TO foo;
NOTICE:  DB node id: 0 statement: SET ROLE TO foo;
NOTICE:  DB node id: 1 statement: SET ROLE TO foo;
SET
-- expect to ignore cache and result is one row
SELECT * FROM users;
NOTICE:  DB node id: 0 statement: SELECT * FROM users;
 user_name |  data   
-----------+---------
 foo       | foodata
(1 row)

--
-- Testing ALTER TABLE
--
-- create cache
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i 
---
 2
(1 row)

ALTER TABLE t1 ADD COLUMN j INT;
NOTICE:  DB node id: 0 statement: ALTER TABLE t1 ADD COLUMN j INT;
NOTICE:  DB node id: 1 statement: ALTER TABLE t1 ADD COLUMN j INT;
ALTER TABLE
-- Make sure cache is not used
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i | j 
---+---
 2 |  
(1 row)

-- explicit transaction case
BEGIN;
NOTICE:  DB node id: 0 statement: BEGIN;
NOTICE:  DB node id: 1 statement: BEGIN;
BEGIN
ALTER TABLE t1 DROP COLUMN j;
NOTICE:  DB node id: 0 statement: ALTER TABLE t1 DROP COLUMN j;
NOTICE:  DB node id: 1 statement: ALTER TABLE t1 DROP COLUMN j;
ALTER TABLE
-- Make sure cache is not used
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i 
---
 2
(1 row)

END;
NOTICE:  DB node id: 1 statement: END;
NOTICE:  DB node id: 0 statement: END;
COMMIT
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i 
---
 2
(1 row)

-- Make sure cache is used
SELECT * FROM t1;
 i 
---
 2
(1 row)

--
-- ALTER TABLE is executed on another session case
--
-- Make sure to create cache
SELECT * FROM t1;
 i 
---
 2
(1 row)

SELECT * FROM t1;
 i 
---
 2
(1 row)

ALTER TABLE t1 ADD COLUMN j INT;
NOTICE:  DB node id: 0 statement: ALTER TABLE t1 ADD COLUMN j INT;
NOTICE:  DB node id: 1 statement: ALTER TABLE t1 ADD COLUMN j INT;
ALTER TABLE
-- Make sure this does not access cache
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i | j 
---+---
 2 |  
(1 row)

ALTER TABLE t1 DROP COLUMN j;
NOTICE:  DB node id: 0 statement: ALTER TABLE t1 DROP COLUMN j;
NOTICE:  DB node id: 1 statement: ALTER TABLE t1 DROP COLUMN j;
ALTER TABLE
--
-- Testing ALTER DATABASE
--
ALTER TABLE t1 ADD COLUMN j INT;
NOTICE:  DB node id: 0 statement: ALTER TABLE t1 ADD COLUMN j INT;
NOTICE:  DB node id: 1 statement: ALTER TABLE t1 ADD COLUMN j INT;
ALTER TABLE
-- create taget database
create database test2;
NOTICE:  DB node id: 0 statement: create database test2;
NOTICE:  DB node id: 1 statement: create database test2;
CREATE DATABASE
-- create cache
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i | j 
---+---
 2 |  
(1 row)

ALTER DATABASE test2 RESET ALL;
NOTICE:  DB node id: 0 statement: ALTER DATABASE test2 RESET ALL;
NOTICE:  DB node id: 1 statement: ALTER DATABASE test2 RESET ALL;
ALTER DATABASE
-- Make sure cache is not used
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i | j 
---+---
 2 |  
(1 row)

-- explicit transaction case
BEGIN;
NOTICE:  DB node id: 0 statement: BEGIN;
NOTICE:  DB node id: 1 statement: BEGIN;
BEGIN
ALTER DATABASE test2 RESET ALL;
NOTICE:  DB node id: 0 statement: ALTER DATABASE test2 RESET ALL;
NOTICE:  DB node id: 1 statement: ALTER DATABASE test2 RESET ALL;
ALTER DATABASE
-- Make sure cache is not used
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i | j 
---+---
 2 |  
(1 row)

END;
NOTICE:  DB node id: 1 statement: END;
NOTICE:  DB node id: 0 statement: END;
COMMIT
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i | j 
---+---
 2 |  
(1 row)

-- Make sure cache is used
SELECT * FROM t1;
 i | j 
---+---
 2 |  
(1 row)

--
-- ALTER DATABASE is executed on another session case
--
-- Make sure to create cache
SELECT * FROM t1;
 i | j 
---+---
 2 |  
(1 row)

SELECT * FROM t1;
 i | j 
---+---
 2 |  
(1 row)

ALTER DATABASE test2 RESET ALL;
NOTICE:  DB node id: 0 statement: ALTER DATABASE test2 RESET ALL;
NOTICE:  DB node id: 1 statement: ALTER DATABASE test2 RESET ALL;
ALTER DATABASE
-- Make sure this does not access cache
SELECT * FROM t1;
NOTICE:  DB node id: 0 statement: SELECT * FROM t1;
 i | j 
---+---
 2 |  
(1 row)

