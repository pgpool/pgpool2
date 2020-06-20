#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for snapshot isolation mode.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PG_CTL=$PGBIN/pg_ctl
PGBENCH=$PGBIN/pgbench
export PGDATABASE=test

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment.
echo -n "creating test environment..."
$PGPOOL_SETUP -m i|| exit 1
echo "done."

dir=`pwd`

# SI mode requires REPEATABLE READ transaction isolation mode.
echo "default_transaction_isolation = 'repeatable read'" >> data0/postgresql.conf
echo "default_transaction_isolation = 'repeatable read'" >> data1/postgresql.conf

source ./bashrc.ports

./startall

export PGPORT=$PGPOOL_PORT

wait_for_pgpool_startup

$PSQL <<EOF
DROP TABLE t1;
CREATE TABLE t1(i int);
INSERT INTO t1 VALUES(0);
DROP TABLE log;
CREATE TABLE log(i int);
EOF

# Do updating.
$PGBENCH -n -c 1 -T 30 -f ../inconsistency1.sql&

# Do SELECT INTO while updating. This will create different rows among
# node 0 log table and node 1 log table if we cannot keep global
# snapshot isolation visibly.
$PGBENCH -n -c 1 -T 30 -f ../inconsistency2.sql&
wait

# Ok let's see if rows in the log tables are identical.
psql -p 11002 -c "\copy log to '11002.txt'"
psql -p 11003 -c "\copy log to '11003.txt'"
cmp 11002.txt 11003.txt >/dev/null

if [ $? != 0 ];then
    echo "Transaction results are not consistent."
    ./shutdownall
    exit 1
fi
echo "Transaction results are consistent."

./shutdownall

exit 0
