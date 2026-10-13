#!/usr/bin/env bash
#-------------------------------------------------------------------
# Test script for v2 protocol acceptance.
# From Pgpool-II 4.8, it does not accept v2 frontend/backend protocol anymore.
# This test simply sends a v2 startup packet to pgpool using pgproto
# and see if pgpool rejects it.
# Note that this test requires version 4.8 or later Pgpool-II
# (including pgproto) and PostgreSQL 14 or later.
#
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PG_CTL=$PGBIN/pg_ctl
PGPROTO=$PGPOOL_INSTALL_DIR/bin/pgproto
export PGDATABASE=test
export PGUSER=$USER

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

echo -n "creating test environment..."
$PGPOOL_SETUP || exit 1
echo "done."
source ./bashrc.ports
./startall
wait_for_pgpool_startup

$PGPROTO -d $PGDATABASE -h localhost -p $PGPOOL_PORT --test-v2 > result.txt 2>&1
diff ../expected.txt result.txt
if [ $? = 0 ];then
    echo "test Ok."
    ./shutdownall
    exit 0
fi
echo "The expected result was different."
./shutdownall
exit 1
