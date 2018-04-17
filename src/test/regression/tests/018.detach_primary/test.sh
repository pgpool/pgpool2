#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for testing the feature of detach_false_primary.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PG_CTL=$PGBIN/pg_ctl
export PGDATABASE=test

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

version=`$PSQL --version|awk '{print $3}'`
result=`echo "$version >= 9.6"|bc`
if [ $result = 0 ];then
    echo "PostgreSQL version $version is 9.5 or before. Skipping test."
    exit 0
fi

# create test environment
echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 3 -s || exit 1
echo "done."

source ./bashrc.ports

echo "detach_false_primary=on" >> etc/pgpool.conf
echo "sr_check_period = 1" >> etc/pgpool.conf
./startall
export PGPORT=$PGPOOL_PORT
wait_for_pgpool_startup

# promote #3 node to create false primary
$PG_CTL -D data2 promote

sleep 10
wait_for_pgpool_startup
$PSQL -c "show pool_nodes" postgres > show_pool_nodes
primary_node=`grep primary show_pool_nodes|awk '{print $1}'`
if [ $primary_node != 0 ];then
    echo "primary node is not 0"
    ./shutdownall
    exit 1
fi

false_primary_node=`grep down show_pool_nodes|awk '{print $1}'`
if [ $false_primary_node != 2 ];then
    echo "false primary node is not 2"
    ./shutdownall
    exit 1
fi

./shutdownall

exit 0
