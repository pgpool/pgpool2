#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for native replication mode.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PGBENCH=$PGBENCH_PATH

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment
echo -n "creating test environment..."
$PGPOOL_SETUP -m r -n 2 || exit 1
echo "done."

source ./bashrc.ports

./startall

export PGPORT=$PGPOOL_PORT

wait_for_pgpool_startup

$PSQL test <<EOF
CREATE TABLE t1(i SERIAL, j TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP);
EOF

cat > pgbench.sql <<EOF
INSERT INTO t1 VALUES (DEFAULT);
EOF

$PGBENCH -i test
$PGBENCH -f pgbench.sql -c 10 -t 10 test

# test with extended protocol (autocommit on)
# per [pgpool-general: 2144].
cp ../PgTester.java .
javac PgTester.java
export CLASSPATH=.:$JDBC_DRIVER
psql -f ../create.sql test
env
psql -f $PGPOOL_INSTALL_DIR/share/pgpool-II/insert_lock.sql test

java PgTester 0 &
java PgTester 10 &
java PgTester 100 &
java PgTester 1000 &
wait

$PSQL -p 11002 test <<EOF
\copy (SELECT * FROM t1 ORDER BY i) to 'dump0.txt'
EOF

$PSQL -p 11003 test <<EOF
\copy (SELECT * FROM t1 ORDER BY i) to 'dump1.txt'
EOF

# check if database contents are identical
diff dump0.txt dump1.txt
if [ $? != 0 ];then
	# contents are not identical
	./shutdownall
	exit 1
fi

./shutdownall

exit 0
