#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for load balancing.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
# sleep time after reload in seconds
st=10

for mode in s
do
	rm -fr $TESTDIR
	mkdir $TESTDIR
	cd $TESTDIR

# create test environment
	echo -n "creating test environment..."
	$PGPOOL_SETUP -m $mode -n 2 || exit 1
	echo "done."

	source ./bashrc.ports
	echo "backend_weight0 = 0" >> etc/pgpool.conf
	echo "backend_weight1 = 1" >> etc/pgpool.conf
	echo "disable_load_balance_on_write = 'dml_adaptive'" >> etc/pgpool.conf
	echo "write_function_list = 'currval,lastval,nextval,setval,f1'" >> etc/pgpool.conf
	echo "dml_adaptive_object_relationship_list= 't1:t2,f1():tF'" >> etc/pgpool.conf

	./startall

	export PGPORT=$PGPOOL_PORT

	wait_for_pgpool_startup

	$PSQL test <<EOF
CREATE TABLE t1(i INTEGER);
CREATE TABLE t2(i INTEGER);
CREATE TABLE t3(i INTEGER);
CREATE TABLE tF(i INTEGER);
CREATE FUNCTION f1(INTEGER) returns INTEGER AS 'SELECT \$1' LANGUAGE SQL;
EOF
# wait for the newly created tables replicated
sleep 1;
$PSQL test <<EOF

BEGIN;
SELECT i, 'QUERY ID T1-1' FROM t2; 		-- LOAD balance
INSERT INTO t1 values(1);			-- NO LOAD balance
SELECT i, 'QUERY ID T1-2' FROM t1;		-- NO LOAD balance
SELECT i, 'QUERY ID T1-3' FROM t3;		-- LOAD balance
SELECT i, 'QUERY ID T1-4' FROM t2;		-- NO LOAD balance
COMMIT;
EOF

# check if simple load balance worked
	fgrep "SELECT i, 'QUERY ID T1-1' FROM t2;" log/pgpool.log |grep "DB node id: 1">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		echo fail: "SELECT i, QUERY ID T1-1 FROM t2" was not load balanced.
		./shutdownall
		exit 1
	fi
	echo ok: first read was load balanced.

# check if simple load balance worked
	fgrep "SELECT i, 'QUERY ID T1-2' FROM t1;" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		echo fail: "SELECT i, QUERY ID T1-2 FROM t1;" was wrongly load balanced.
		./shutdownall
		exit 1
	fi
	echo ok: adaptive load balance test 1 works.

# check if simple load balance worked
	fgrep "SELECT i, 'QUERY ID T1-3' FROM t3;" log/pgpool.log |grep "DB node id: 1">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		echo fail: "SELECT i, 'QUERY ID T1-3' FROM t3" was not load balanced.
		./shutdownall
		exit 1
	fi
	echo ok: unrelated read was load balanced.

# check if simple load balance worked
	fgrep "SELECT i, 'QUERY ID T1-4' FROM t2;" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		echo fail: "SELECT i, 'QUERY ID T1-4' FROM t2;" was wrongly load balanced.
		./shutdownall
		exit 1
	fi
	echo ok: adaptive load balance test 2 works.

$PSQL test <<EOF
BEGIN;
SELECT i, 'QUERY ID T2-5' FROM tF; 			-- LOAD balance
SELECT f1(190), 'QUERY ID T2-6';			-- NO LOAD balance
SELECT i, 'QUERY ID T2-7' FROM tF; 			-- NO LOAD balance
SELECT i, 'QUERY ID T2-8' FROM t1;			-- LOAD balance
SELECT i, 'QUERY ID T2-9' FROM t3;			-- LOAD balance
COMMIT;
EOF

# check if simple load balance worked
	fgrep "SELECT i, 'QUERY ID T2-5' FROM tF;" log/pgpool.log |grep "DB node id: 1">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		echo fail: "SELECT i, 'QUERY ID T2-5' FROM tF;" was not load balanced.
		./shutdownall
		exit 1
	fi
	echo ok: T2 first read was load balanced.

# check if simple load balance worked
	fgrep "SELECT i, 'QUERY ID T2-7' FROM tF;" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		echo fail: "SELECT i, 'QUERY ID T2-7' FROM tF;" was wrongly load balanced.
		./shutdownall
		exit 1
	fi
	echo ok: adaptive load balance test 3 works.

# check if simple load balance worked
	fgrep "SELECT i, 'QUERY ID T2-8' FROM t1;" log/pgpool.log |grep "DB node id: 1">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		echo fail: "SELECT i, 'QUERY ID T2-8' FROM t1;" was not load balanced.
		./shutdownall
		exit 1
	fi
	echo ok: T2 third read was load balanced.

# check if simple load balance worked
	fgrep "SELECT i, 'QUERY ID T2-9' FROM t3;" log/pgpool.log |grep "DB node id: 1">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		echo fail: "SELECT i, 'QUERY ID T2-9' FROM t3;" was not load balanced.
		./shutdownall
		exit 1
	fi
	echo ok: T2 final read was load balanced.

# check_node QUERY_ID TABLE NODE: check that the marker SELECT went to NODE
check_node()
{
	fgrep "SELECT i, '$1' FROM $2;" log/pgpool.log |grep "DB node id: $3">/dev/null 2>&1
	if [ $? != 0 ];then
		echo fail: "SELECT i, '$1' FROM $2;" was not sent to node $3.
		./shutdownall
		exit 1
	fi
	echo ok: "$1" was sent to node $3.
}

# Savepoints: ROLLBACK TO must not end the transaction, and must forget
# only the writes made after the savepoint.
$PSQL test <<EOF
BEGIN;
INSERT INTO t1 values(1);			-- t1, and t2 via the relationship list
SAVEPOINT s1;
INSERT INTO t3 values(1);
ROLLBACK TO SAVEPOINT s1;
SELECT i, 'QUERY ID T3-1' FROM t1;		-- NO LOAD balance
SELECT i, 'QUERY ID T3-2' FROM t2;		-- NO LOAD balance
SELECT i, 'QUERY ID T3-3' FROM t3;		-- LOAD balance
COMMIT;
EOF
	check_node 'QUERY ID T3-1' t1 0
	check_node 'QUERY ID T3-2' t2 0
	check_node 'QUERY ID T3-3' t3 1

# Nested savepoints, RELEASE, and a write after ROLLBACK TO
$PSQL test <<EOF
BEGIN;
SAVEPOINT s1;
INSERT INTO t3 values(1);
SAVEPOINT s2;
INSERT INTO tF values(1);
RELEASE SAVEPOINT s2;
SELECT i, 'QUERY ID T4-1' FROM tF;		-- NO LOAD balance
ROLLBACK TO SAVEPOINT s1;
SELECT i, 'QUERY ID T4-2' FROM t3;		-- LOAD balance
SELECT i, 'QUERY ID T4-3' FROM tF;		-- LOAD balance
INSERT INTO t3 values(2);
SELECT i, 'QUERY ID T4-4' FROM t3;		-- NO LOAD balance
COMMIT;
EOF
	check_node 'QUERY ID T4-1' tF 0
	check_node 'QUERY ID T4-2' t3 1
	check_node 'QUERY ID T4-3' tF 1
	check_node 'QUERY ID T4-4' t3 0

# Reused savepoint names: ROLLBACK TO/RELEASE pick the innermost one
$PSQL test <<EOF
BEGIN;
SAVEPOINT a;
INSERT INTO t1 values(1);
SAVEPOINT a;
INSERT INTO t3 values(1);
ROLLBACK TO SAVEPOINT a;
SELECT i, 'QUERY ID T5-1' FROM t1;		-- NO LOAD balance
SELECT i, 'QUERY ID T5-2' FROM t3;		-- LOAD balance
RELEASE SAVEPOINT a;
ROLLBACK TO SAVEPOINT a;
SELECT i, 'QUERY ID T5-3' FROM t1;		-- LOAD balance
SELECT i, 'QUERY ID T5-4' FROM t2;		-- LOAD balance
COMMIT;
EOF
	check_node 'QUERY ID T5-1' t1 0
	check_node 'QUERY ID T5-2' t3 1
	check_node 'QUERY ID T5-3' t1 1
	check_node 'QUERY ID T5-4' t2 1

        ./shutdownall
        cd ..
done
exit 0
