#!/usr/bin/env bash
#-------------------------------------------------------------------
# Test script for pcp commands.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment.
echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 2 || exit 1
echo "done."

source ./bashrc.ports
export PGPORT=$PGPOOL_PORT

echo "=== test1: pcp_log_rotate"

BASEDIR=`pwd`
echo "logging_collector = on" >> etc/pgpool.conf
echo "log_directory = '$BASEDIR/log'" >> etc/pgpool.conf
echo "log_filename = 'pgpool.log'" >> etc/pgpool.conf

./startall
wait_for_pgpool_startup

mv log/pgpool.log log/pgpool.log.1

sleep 1
$PSQL -h localhost -c "SELECT 1" test > /dev/null 2>&1

grep "SELECT 1" log/pgpool.log.1 > /dev/null 2>&1
if [ $? != 0 ];then
    r1=fail
    echo "test1 failed before running pcp_log_rotate."
else
    $PGPOOL_INSTALL_DIR/bin/pcp_log_rotate -h localhost -w -p $PCP_PORT
    $PSQL -h localhost -c "SELECT 1" test > /dev/null 2>&1

    sleep 1
    grep "SELECT 1" log/pgpool.log > /dev/null 2>&1
    if [ $? = 0 ];then
        r1=ok
        echo "test1 ok"
    else
        echo "test1 failed."
    fi
fi

./shutdownall

if [ $r1 = ok ]; then
    echo "all test succeeded"
    exit 0
else
    echo "some tests failed"
    echo "test1: $r1"
    exit 1
fi
