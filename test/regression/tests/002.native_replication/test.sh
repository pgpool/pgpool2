#! /bin/sh -x
#-------------------------------------------------------------------
# test script for native replication mode.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PGBENCH=$PGBENCH_DIR/pgbench

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment
echo -n "creating test environment..."
sh $PGPOOL_SETUP -m r -n 2 || exit 1
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

$PSQL -p 11000 test <<EOF
\copy (SELECT * FROM t1 ORDER BY i) to 'dump0.txt'
EOF

$PSQL -p 11001 test <<EOF
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
