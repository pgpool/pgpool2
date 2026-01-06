#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for memqcache.
# requires Java PostgreSQL JDBC driver.
PGBENCH=$PGBENCH_PATH

WHOAMI=`whoami`
source $TESTLIBS
TESTDIR=testdir

PSQL=$PGBIN/psql
PGPROTO=$PGPOOL_INSTALL_DIR/bin/pgproto
PCP_INVALIDATE_QUERY_CACHE=$PGPOOL_INSTALL_DIR/bin/pcp_invalidate_query_cache

# remove error/notice details (message and so on) from
# ErrorResponse or NoticeResponse messages.
# used for pgproto.
function del_details_from_error
{
    cat|sed -e '/ErrorResponse/s/ F .*//' -e '/NoticeResponse/s/ F .*$//'
}

for mode in s r n
do
	rm -fr $TESTDIR
	mkdir $TESTDIR
	cd $TESTDIR

# create test environment
	echo -n "creating test environment..."
	$PGPOOL_SETUP -m $mode -n 2 || exit 1
	echo "done."

	echo "memory_cache_enabled = on" >> etc/pgpool.conf
	echo "cache_safe_memqcache_table_list = 'cache_safe_v'" >> etc/pgpool.conf
	echo "cache_unsafe_memqcache_table_list = 'cache_unsafe_t'" >> etc/pgpool.conf
	echo "log_per_node_statement = on" >> etc/pgpool.conf
	echo "log_client_messages = on" >> etc/pgpool.conf
	echo "log_min_messages = debug5" >> etc/pgpool.conf

	source ./bashrc.ports

	export PGPORT=$PGPOOL_PORT

	echo "jdbc.url=jdbc:postgresql://localhost:$PGPOOL_PORT/test" > jdbctest.prop
	echo "jdbc.user=$WHOAMI" >> jdbctest.prop
	echo "jdbc.password=" >> jdbctest.prop
	cp ../jdbctest.java .
	javac jdbctest.java
	export CLASSPATH=.:$JDBC_DRIVER
	./startall
	wait_for_pgpool_startup

	$PSQL test <<EOF
CREATE SCHEMA other_schema;
CREATE TABLE t1 (i int);
CREATE TABLE cache_unsafe_t (i int);
CREATE TABLE with_modify (i int);
CREATE TABLE explain_analyze (i int);
CREATE VIEW normal_v AS SELECT * FROM t1;
CREATE VIEW cache_safe_v AS SELECT * FROM t1;
CREATE FUNCTION public.immutable_func(INTEGER) returns INTEGER AS 'SELECT \$1' LANGUAGE SQL IMMUTABLE;
CREATE FUNCTION other_schema.volatile_func(INTEGER) returns INTEGER AS 'SELECT \$1' LANGUAGE SQL VOLATILE;
SELECT pg_sleep(2);	-- Sleep for a while to make sure object creations are replicated
SELECT * FROM t1;
SELECT * FROM t1;
SELECT * FROM cache_unsafe_t;
SELECT * FROM cache_unsafe_t;
SELECT * FROM normal_v;
SELECT * FROM normal_v;
SELECT * FROM cache_safe_v;
SELECT * FROM cache_safe_v;
SELECT * FROM with_modify;
WITH cte AS (INSERT INTO with_modify values(1) RETURNING *) SELECT * FROM with_modify;
WITH cte AS (INSERT INTO with_modify values(1) RETURNING *) SELECT * FROM with_modify;
SELECT * FROM with_modify;
SELECT public.immutable_func(1);
SELECT public.immutable_func(1);
SELECT other_schema.volatile_func(1);
SELECT other_schema.volatile_func(1);
SELECT * FROM explain_analyze;
EXPLAIN ANALYZE INSERT INTO explain_analyze VALUES(1);
SELECT * FROM explain_analyze;
SELECT CURRENT_TIMESTAMP;
SELECT CURRENT_USER;
SELECT '2022-07-05 10:00:00'::TIMESTAMP;
SELECT '2022-07-05 10:00:00'::TIMESTAMP;
SELECT '2022-07-05 10:00:00'::TIME;
SELECT '2022-07-05 10:00:00'::TIME;
SELECT '2022-07-05 10:00:00'::DATE;
SELECT '2022-07-05 10:00:00'::DATE;
SELECT '2022-07-05 10:00:00'::TIMESTAMPTZ;
SELECT '2022-07-05 10:00:00'::TIMESTAMPTZ;
SELECT '2022-07-05 10:00:00'::TIMETZ;
SELECT '2022-07-05 10:00:00'::TIMETZ;
SELECT to_timestamp(0);
SELECT to_timestamp(0);
/*FORCE QUERY CACHE*/SELECT now();
/*FORCE QUERY CACHE*/SELECT now();
/*NO QUERY CACHE*/SELECT 1;
/*NO QUERY CACHE*/SELECT 1;
EOF

	success=true
	grep "fetched from cache" log/pgpool.log | grep t1 > /dev/null || success=false
	grep "fetched from cache" log/pgpool.log | grep cache_unsafe_t > /dev/null && success=false
	grep "fetched from cache" log/pgpool.log | grep normal_v > /dev/null && success=false
	grep "fetched from cache" log/pgpool.log | grep cache_safe_v > /dev/null || success=false
	grep "fetched from cache" log/pgpool.log | grep with_modify > /dev/null && success=false
	grep "fetched from cache" log/pgpool.log | grep immutable_func > /dev/null || success=false
	grep "fetched from cache" log/pgpool.log | grep volatile_func > /dev/null && success=false
	grep "fetched from cache" log/pgpool.log | grep explain_analyze > /dev/null && success=false
	grep "fetched from cache" log/pgpool.log | grep CURRENT_TIMESTAMP > /dev/null && success=false
	grep "fetched from cache" log/pgpool.log | grep CURRENT_USER > /dev/null && success=false
	grep "fetched from cache" log/pgpool.log | grep 'TIMESTAMP;' > /dev/null || success=false
	grep "fetched from cache" log/pgpool.log | grep 'TIME;' > /dev/null || success=false
	grep "fetched from cache" log/pgpool.log | grep 'DATE;' > /dev/null || success=false
	grep "fetched from cache" log/pgpool.log | grep 'TIMESTAMPTZ;' > /dev/null && success=false
	grep "fetched from cache" log/pgpool.log | grep 'TIMETZ;' > /dev/null && success=false
	grep "fetched from cache" log/pgpool.log | grep 'to_timestamp' > /dev/null && success=false
	grep "fetched from cache" log/pgpool.log | grep 'FORCE QUERY CACHE' > /dev/null || success=false
	grep "fetched from cache" log/pgpool.log | grep 'NO QUERY CACHE' > /dev/null && success=false

	if [ $success = false ];then
		./shutdownall
		exit 1
	fi
	    
	java jdbctest > result.txt 2>&1
	cmp ../expected.txt result.txt
	if [ $? != 0 ];then
		./shutdownall
		exit 1
	fi

	./shutdownall
	echo "backend_weight1 = 0" >> etc/pgpool.conf
	echo "notice_per_node_statement = on" >> etc/pgpool.conf
	./startall
	wait_for_pgpool_startup

	createuser foo
	createuser bar

	$PSQL -a test >> result 2>&1 <<EOF
--
-- testing an effect on a row level security enabled table and SET ROLE
--
CREATE TABLE users (user_name TEXT, data TEXT);
INSERT INTO users VALUES('foo', 'foodata');
INSERT INTO users VALUES('bar', 'bardata');
ALTER TABLE users ENABLE ROW LEVEL SECURITY;
CREATE POLICY user_policy ON users USING (user_name = CURRENT_USER);
GRANT SELECT ON users TO foo;
GRANT SELECT ON users TO bar;
SET ROLE TO foo;
-- run SELECT as foo. Only user_name = 'foo' data expected.
SELECT * FROM users;
RESET ROLE;
SET ROLE TO bar;
-- run SELECT as bar. Only user_name = 'bar' data expected.
SELECT * FROM users;
EOF

#echo '=== extended query test for row security ===' >> result
#	$PGPROTO -d test -f ../row_security.data |& del_details_from_error >> result

	$PSQL -a -U foo test >> result 2>&1 <<EOF
--
-- testing row security with row_security = off
--
SET ROW_SECURITY TO off;
-- Error expected
SELECT * FROM users;
EOF

	$PSQL -a test >> result 2>&1 <<EOF
--
-- testing SET ROLE
--
CREATE TABLE footable(t text);
INSERT INTO footable VALUES('foo');
GRANT SELECT ON footable TO foo;
GRANT INSERT ON footable TO foo;
GRANT UPDATE ON footable TO foo;
GRANT DELETE ON footable TO foo;
SELECT * FROM footable;
SET ROLE TO bar;
-- run SELECT as bar. Permission denied is expected.
SELECT * FROM footable;
EOF

#echo '==== extended query test for "testing SET ROLE" above ===' >> result
#	$PGPROTO -d test -f ../set_role1.data |& del_details_from_error >> result

	$PSQL -a test >> result 2>&1 <<EOF
--
-- testing SESSION AUTHORIZATION
--
SET SESSION AUTHORIZATION bar;
-- run SELECT as bar. Permission denied is expected.
SELECT * FROM footable;
EOF

#echo '=== extended query test for "testing SESSION AUTHORIZATION" above ===' >> result
#	$PGPROTO -d test -f ../session_authorization.data |& del_details_from_error >> result

	$PSQL -a test >> result 2>&1 <<EOF
--
-- testing SET ROLE. Make sure that query cache is not
-- created.
--
-- create cache
SELECT * FROM footable;
-- change role
SET ROLE TO foo;
-- run SELECT as foo to make sure that cache is not used.
-- If query cache was created we will NOT see
-- "NOTICE: DB node id: 1 statement: SELECT ..."
SELECT * FROM footable;
-- Modify footable to see cache invalidation works even after SET ROLE.
INSERT INTO footable VALUES ('foo1');
-- restore ROLE
RESET ROLE;
-- Make sure cache was invalidated.
SELECT * FROM footable;
EOF

#echo '=== extended query test for "testing SET ROLE" above ===' >> result
#	$PGPROTO -d test -f ../set_role2.data |& del_details_from_error >> result

		$PSQL -a test >> result 2>&1 <<EOF
--
-- explicit transaction case
--
-- create cache
SELECT * FROM footable;
SELECT * FROM footable;
-- change role
SET ROLE TO foo;
BEGIN;
-- run SELECT as foo to make sure that cache is not used.
-- If query cache was created we will NOT see
-- "NOTICE: DB node id: 1 statement: SELECT ..."
SELECT * FROM footable;
-- Modify footable to see cache invalidation works even after SET ROLE.
INSERT INTO footable VALUES ('foo2');
END;
EOF
		$PSQL -a test >> result 2>&1 <<EOF
-- Make sure cache was invalidated.
SELECT * FROM footable;
EOF

#echo '=== extended query test for "explicit transaction case" above ===' >> result
#	$PGPROTO -d test -f ../set_role3.data |& del_details_from_error >> result

#		$PSQL -a test >> result 2>&1 <<EOF
#-- Make sure cache was invalidated.
#SELECT * FROM footable;
#EOF

		$PSQL -a test >> result 2>&1 <<EOF
--
-- explicit transaction abort case
--
-- create cache
SELECT * FROM footable;
SELECT * FROM footable;
-- change role
SET ROLE TO foo;
BEGIN;
-- run SELECT as foo to make sure that cache is not used.
-- If query cache was created we will NOT see
-- "NOTICE: DB node id: 0 statement: SELECT ..."
SELECT * FROM footable;
-- Modify footable to see cache invalidation works even after SET ROLE.
INSERT INTO footable VALUES ('foo3');
SELECT * FROM footable;
ABORT;
EOF
		$PSQL -a test >> result 2>&1 <<EOF
-- Make sure we don't see 'foo3' row.
SELECT * FROM footable;
EOF

#echo '=== extended query test for "explicit transaction abort case" above ===' >> result
#	$PGPROTO -d test -f ../set_role4.data |& del_details_from_error >> result

		$PSQL -a test >> result 2>&1 <<EOF
-- Make sure we don't see 'foo3' row.
SELECT * FROM footable;
EOF
		$PSQL -a test >> result 2>&1 <<EOF
--
-- Testing REVOKE
--
-- create cache
SELECT * FROM t1;
-- REVOKE
REVOKE SELECT ON t1 FROM foo;
SET ROLE TO foo;
-- Make sure foo cannot SELECT t1
SELECT * FROM t1;
RESET ROLE;
-- GRANT again
GRANT SELECT ON t1 TO foo;
EOF
		$PSQL -a test >> result 2>&1 <<EOF
-- explicit transaction case
BEGIN;
-- REVOKE
REVOKE SELECT ON t1 FROM foo;
SET ROLE TO foo;
-- Make sure foo cannot SELECT t1
-- (thus REVOKE will be rollbacked )
SELECT * FROM t1;
END;
SET ROLE TO foo;
-- because REVOKE is rolled back, foo should be able to access t1
SELECT * FROM t1;
EOF

#echo '=== extended query test for "Tesing REVOKE" and "explicit transaction case" above ===' >> result
#	$PGPROTO -d test -f ../revoke1.data |& del_details_from_error >> result

		$PSQL -a test >> result 2>&1 <<EOF
--
-- REVOKE is executed on another session case
--
-- Make sure to create cache
SELECT * FROM t1;
SELECT * FROM t1;
-- execute REVOKE
REVOKE SELECT ON t1 FROM foo
EOF
		$PSQL -a test >> result 2>&1 <<EOF
-- Make sure this does not access cache
SELECT * FROM t1;
EOF

#echo '=== extended query test for "REVOKE is executed on another session case" above ===' >> result
#	$PGPROTO -d test -f ../revoke2.data |& del_details_from_error >> result
#	$PGPROTO -d test -f ../revoke3.data |& del_details_from_error >> result

		$PSQL -a test >> result 2>&1 <<EOF
--
-- ALTER ROLE BYPASSRLS case
--
ALTER ROLE foo BYPASSRLS;
SET ROLE TO foo;
-- expect to ignore cache and result is all rows
SELECT * FROM users;
RESET ROLE;
ALTER ROLE foo NOBYPASSRLS;
SET ROLE TO foo;
-- expect to ignore cache and result is one row
SELECT * FROM users;
EOF

#echo '=== extended query test for "ALTER ROLE BYPASSRLS case" case ===' >> result
#	$PGPROTO -d test -f ../alter_role.data |& del_details_from_error >> result

		$PSQL -a test >> result 2>&1 <<EOF
--
-- Testing ALTER TABLE
--
-- create cache
SELECT * FROM t1;
ALTER TABLE t1 ADD COLUMN j INT;
-- Make sure cache is not used
SELECT * FROM t1;
EOF
		$PSQL -a test >> result 2>&1 <<EOF
-- explicit transaction case
BEGIN;
ALTER TABLE t1 DROP COLUMN j;
-- Make sure cache is not used
SELECT * FROM t1;
END;
SELECT * FROM t1;
-- Make sure cache is used
SELECT * FROM t1;
EOF

#echo '=== extended query test for "Testing ALTER TABLE and explicit transaction" case ===' >> result
#	$PGPROTO -d test -f ../alter_table1.data |& del_details_from_error >> result

		$PSQL -a test >> result 2>&1 <<EOF
--
-- ALTER TABLE is executed on another session case
--
-- Make sure to create cache
SELECT * FROM t1;
SELECT * FROM t1;
ALTER TABLE t1 ADD COLUMN j INT;
EOF
		$PSQL -a test >> result 2>&1 <<EOF
-- Make sure this does not access cache
SELECT * FROM t1;
ALTER TABLE t1 DROP COLUMN j;
EOF

#echo '=== extended query test for "ALTER TABLE is executed on another session" case ===' >> result
#	$PGPROTO -d test -f ../alter_table2.data |& del_details_from_error >> result
#	$PGPROTO -d test -f ../alter_table3.data |& del_details_from_error >> result

		$PSQL -a test >> result 2>&1 <<EOF
--
-- Testing ALTER DATABASE
--
ALTER TABLE t1 ADD COLUMN j INT;
-- create taget database
create database test2;
-- create cache
SELECT * FROM t1;
ALTER DATABASE test2 RESET ALL;
-- Make sure cache is not used
SELECT * FROM t1;
EOF
		$PSQL -a test >> result 2>&1 <<EOF
-- explicit transaction case
BEGIN;
ALTER DATABASE test2 RESET ALL;
-- Make sure cache is not used
SELECT * FROM t1;
END;
SELECT * FROM t1;
-- Make sure cache is used
SELECT * FROM t1;
EOF
#echo '=== extended query test for "ALTER DATABSE and explicit transaction" case ===' >> result
#	$PGPROTO -d test -f ../alter_database1.data |& del_details_from_error >> result
		$PSQL -a test >> result 2>&1 <<EOF
--
-- ALTER DATABASE is executed on another session case
--
-- Make sure to create cache
SELECT * FROM t1;
SELECT * FROM t1;
ALTER DATABASE test2 RESET ALL;
EOF
		$PSQL -a test >> result 2>&1 <<EOF
-- Make sure this does not access cache
SELECT * FROM t1;
EOF

#echo '=== extended query test for "ALTER DATABASE is executed on another session" case ===' >> result
#	$PGPROTO -d test -f ../alter_database2.data |& del_details_from_error >> result
#	$PGPROTO -d test -f ../alter_database3.data |& del_details_from_error >> result

		$PSQL -a test >> result 2>&1 <<EOF
--
-- ALTER ROLE WITH ENCRYPTED PASSWORD and
-- ALTER ROLE WITH CONNECTION LIMIT 10
-- do not invalidate query cache
SELECT 10;
SELECT 10;
ALTER ROLE foo WITH ENCRYPTED PASSWORD 'foo';
ALTER ROLE foo WITH CONNECTION LIMIT 10;
SELECT 10;
EOF

		$PSQL -a test >> result 2>&1 <<EOF
--
-- PGPOOL SET CACHE DELETE test cases.
--
-- force to create cache
/*FORCE QUERY CACHE*/SELECT 1;
-- make sure the cache was created
/*FORCE QUERY CACHE*/SELECT 1;
-- delete the cache
PGPOOL SET CACHE DELETE '/*FORCE QUERY CACHE*/SELECT 1;';
-- make sure the cache was deleted
/*FORCE QUERY CACHE*/SELECT 1;
EOF
	./shutdownall

	cd ..

	log=/tmp/diff
	EXPECTED=expected.$mode
	diff -c $EXPECTED testdir/result > $log
	if [ $? != 0 ];then
	    echo "test failed in mode: $mode"
	    cat $log
	    rm $log
	    exit 1
	fi
	rm $log

	cd $TESTDIR
	./startall
	wait_for_pgpool_startup

	# test for pcp_invalidate_query_cache
	res1=`$PSQL -t -c "/*FORCE QUERY CACHE*/SELECT current_timestamp" test`
	res2=`$PSQL -t -c "/*FORCE QUERY CACHE*/SELECT current_timestamp" test`
	# make sure query cache created
	if [ "$res1" != "$res2" ];then
	    echo "query cache was not created in pcp_invalidate_query_cache test"
	    ./shutdownall
	    exit 1
	fi
	# remove query cache
	$PCP_INVALIDATE_QUERY_CACHE -p $PCP_PORT
	if [ $? != 0 ];then
	    echo "pcp_invalidate_query_cache failed"
	    ./shutdownall
	    exit 1
	fi
	# make sure query cache has gone
	$PSQL -t -c "SELECT 1" test	# this query processes query cache invalidation request
	res1=`$PSQL -t -c "/*FORCE QUERY CACHE*/SELECT current_timestamp" test`
	if [ "$res1" = "$res2" ];then
	    echo "query cache was not invalidated"
	    ./shutdownall
	    exit 1
	fi
	./shutdownall

	cd ..
done

#
# Test for extended query protocol coner cases in streaming replication mode.
# These tests are basically for a sequence of extended queries:
# 1. execute a SELECT and create query cache entry.
# 2. sync.
# 3. execute another a SELECT.
# 4. execute bind and execute to use the query cache created at #1.
# 5. sync.

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment
echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 2 || exit 1
echo "done."

echo "memory_cache_enabled = on" >> etc/pgpool.conf
echo "log_per_node_statement = on" >> etc/pgpool.conf
echo "log_client_messages = on" >> etc/pgpool.conf
echo "log_min_messages = debug5" >> etc/pgpool.conf
cd ..

for i in 1 2 3 4 4 5 6 7
do
    #
    # case 1: failed with kind mismatch error at #5.
    # "packet kind of backend 0 ['T'] does not match with main/majority nodes packet kind ['Z']"
    #
    # case 2: step #4 includes error (hung).
    #
    # case 3: step #4 includes PortalSuspended (hung).
    #
    # case 4: various cases including portal suspended
    # Note that case4 is executed twice to make sure that
    # the test works for either query cache exists or does not exist
    #
    # case 5: simple cache invalidation test.
    #
    # case 6: execute INSERT without parse message cache invalidation test.
    #
    # case 7: similar to case 6 except this uses an explicit transaction.

    cd $TESTDIR

    # case 5 includes UPDATE, and we want the result without disturbed
    # by replication delay.
    if [ $i = 5 ];then
	echo "backend_weight1 = 0" >> etc/pgpool.conf
    fi
    ./startall
    wait_for_pgpool_startup
    timeout 1 $PGPROTO -d test -f ../query_cache_bug$i.data |& del_details_from_error > result
    if [ $? != 0 ];then
	# timeout happened or pgproto returned non 0 status
	echo "test failed in test case #2 (timeout)"
	err=true
	./shutdownall
	exit 1
    fi
    ./shutdownall
    cd ..
    diff -c expected.$i $TESTDIR/result > $log
    if [ $? != 0 ];then
	echo "test failed in test case $i"
	cat $log
	rm $log
	exit 1
    fi
done

exit 0
