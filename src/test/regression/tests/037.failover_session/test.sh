#!/usr/bin/env bash
#-------------------------------------------------------------------
# Test script for session disconnection with failover.
# This test is for streaming replication mode only.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PG_CTL=$PGBIN/pg_ctl
PGBENCH=$PGBENCH_PATH
export PGDATABASE=test

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create streaming replication, 3-node test environment.
echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 3 -s || exit 1
echo "done."

source ./bashrc.ports

# PCP_PORT is defined in bashrc.ports
PCP_DETACH_NODE="$PGPOOL_INSTALL_DIR/bin/pcp_detach_node -w -h localhost -p $PCP_PORT 2"

# customize pgpool.conf. disable load balance to node 2.
# Also modify health check parameters so that health check detects
# backend down earlier.
cat >> etc/pgpool.conf <<EOF
backend_weight2 = 0
log_per_node_statement = off
log_error_verbosity = verbose
health_check_period0 = 1
health_check_max_retries0 = 0
health_check_period1 = 1
health_check_max_retries1 = 0
health_check_period2 = 1
health_check_max_retries2 = 0
EOF

./startall
export PGPORT=$PGPOOL_PORT
wait_for_pgpool_startup

$PGBENCH -i

echo "=== test1: backend_weight2 = 0 and pgbench without -C option"
# In this test we expect that shutdown of node 2 does not affect
# client sessions at all.

($PGBENCH -n -S -c 10 -T 5)&
sleep 1
$PG_CTL -D data2 stop
wait $!
if [ $? != 0 ];then
    echo "pgbench exited with error. test1 failed."
    r1=fail
else
    echo "pgbench suceeded. test1 ok."
    r1=ok
fi
# give pgpool chance to complete the failover
sleep 5
./shutdownall

echo "=== test2: backend_weight2 = 0 and pgbench with -C option"
# In this test we expect that shutdown of node 2 does not affect
# client sessions at all if pcp_detach_node is executed beforehand.

./startall
wait_for_pgpool_startup

($PGBENCH -n -S -C -c 10 -T 5)&
sleep 1
echo $PCP_DETACH_NODE
$PCP_DETACH_NODE
sleep 3
$PG_CTL -D data2 stop
wait $!
if [ $? != 0 ];then
    echo "pgbench exited with error. test2 failed."
    r2=fail
else
    echo "pgbench suceeded. test2 ok."
    r2=ok
fi

./shutdownall

echo "=== test3: load_balance_mode = off and pgbench without -C option"
# Same test as test1. The only the difference is load_balance_mode is
# off instead of backend_weitht2 = 0. To make sure that both have same
# effect against failover.

echo "backend_weight2 = 1" >> etc/pgpool.conf
echo "load_balance_mode = off" >> etc/pgpool.conf

./startall
wait_for_pgpool_startup

($PGBENCH -n -S -c 10 -T 5)&
sleep 1
#$PG_CTL -D data2 stop
$PCP_DETACH_NODE
wait $!
if [ $? != 0 ];then
    echo "pgbench exited with error. test3 failed."
    r3=fail
else
    echo "pgbench suceeded. test3 ok."
    r3=ok
fi
./shutdownall

echo "=== test4: load_balance_mode = off and pgbench with -C option"
# Same test as test3. The only the difference is load_balance_mode is
# off instead of backend_weitht2 = 0. To make sure that both have same
# effect against failover.

./startall
wait_for_pgpool_startup

($PGBENCH -n -S -C -c 10 -T 5)&
sleep 1
echo $PCP_DETACH_NODE
$PCP_DETACH_NODE
sleep 3
$PG_CTL -D data2 stop
wait $!
if [ $? != 0 ];then
    echo "pgbench exited with error. test4 failed."
    r4=fail
else
    echo "pgbench suceeded. test4 ok."
    r4=ok
fi

./shutdownall

if [ $r1 = ok -a $r2 = ok -a $r3 = ok -a $r4 = ok ]; then
    echo "all test succeeded"
    exit 0
else
    echo "some tests failed"
    echo "test1: $r1 test2: $r2 test3: $r3 test4: $r4"
    exit 1
fi
