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

#
# test for pcp_proc_info, pgpool_adm: pcp_proc_info
WHOAMI=`whoami`
./shutdownall
./startall
wait_for_pgpool_startup
$PSQL -a -h localhost -c "SELECT pg_sleep(1)" test &
$PSQL -h localhost test > result 2>&1 <<EOF
CREATE EXTENSION pgpool_adm;
SELECT database, status, case when client_host = '127.0.0.1' or client_host = '::1' then 'localhost' end, statement FROM pcp_proc_info
(host => '', port => $PCP_PORT, username => '$WHOAMI', password => '$WHOAMI')
WHERE connected = '1' AND backend_id = '0' AND statement = 'SELECT pg_sleep(1)'
EOF
if [ $? != 0 ];then
    r2=fail
    echo "test2 failed"
else
    cmp  ../expected result
    if [ $? != 0 ];then
	r2=fail
	echo "test2 failed"
    else
	r2=ok
    fi
fi
wait
./shutdownall

if [ $r1 = ok -a $r2 = ok ]; then
    echo "all test succeeded"
    exit 0
else
    echo "some tests failed"
    echo "test1: $r1"
    echo "test2: $r2"
    exit 1
fi
