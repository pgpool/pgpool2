#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for that detach_false_primary could bring down all backends.
# See [pgpool-hackers: 4431] for more details.
#
# It is possible that all DB nodes go down if detach_false_primary is enabled.
# Story:
# There are 3 watchdog nodes pgpool0, pgpool1 and pgpool2.
# There are 2 DB nodes node0 and node1 (initially node 0 is primary).
# follow_primary_command is disabled.
# 1) Node 0 goes down at pgpool0 due to a network trouble. BUT actually
# node 0 is alive.
# 2) Node 0 goes down at pgpool1 due to a network trouble. BUT actually
# node 0 is alive.
# 3) Failover is triggered. Since pgpool0 and pgpool1 agree, node 0 is set to down.
# node 1 is promoted.
# 4) Before new status is synched with pgpool2, pgpool2's sr_check
# finds that there are two primary nodes due to
# #3. detach_false_primary is triggered and node 1 goes down.
# 5) Now all backends are in down status.

# wait for watchdog starting up by looking for "lifecheck started" in
# the pgpool.log.  argument: $log: absolute path to the pgpool.log.
function wait_for_watchdog_startup
{
    while :
    do
	grep "lifecheck started" $log >/dev/null
	if [ $? = 0 ];then
	    break;
	fi
	sleep 1
    done
}

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

# create 3 node pgpool with 2 backends.
$WATCHDOG_SETUP -wn 3 -n 2

# enable detach_false_primary and health_check_test. We need to
# disable follow_primary_command, othewise node 0 goes down by
# follow_primary_command and the test may not reveals the problem.
# Also we set sr_check_period to very short so that
# detach_false_primary is triggered before the new status is synched
# by watchdog leader.
for i in 0 1 2
do
    echo "detach_false_primary = on" >> pgpool$i/etc/pgpool.conf
    echo "health_check_test = on" >> pgpool$i/etc/pgpool.conf
    echo "follow_primary_command = ''" >> pgpool$i/etc/pgpool.conf
    echo "sr_check_period = 1"  >> pgpool$i/etc/pgpool.conf
done

cd pgpool0
source ./bashrc.ports
cd ..

./startall

echo -n "waiting for watchdog node 0 starting up... "
log=pgpool0/log/pgpool.log
wait_for_watchdog_startup $log
echo "done."

$PGPOOL_INSTALL_DIR/bin/pcp_watchdog_info -v -w -h localhost -p $PCP_PORT
$PGPOOL_INSTALL_DIR/bin/pcp_node_info -h localhost -p $PCP_PORT

# Let node 0 down at pgpool0
echo "0	down" > pgpool0/log/backend_down_request
# Let node 0 down at pgpool1
echo "0	down" > pgpool1/log/backend_down_request

# Wait up to 30 seconds before the problem (all nodes go down).
# Observe that pgpool1 and pgpool2 print:
# LOG:  pgpool_worker_child: invalid node found 1
# which means sr_check ran detach_false_primary but did not trigger failover:
# LOG:  do not detach invalid node 1 because I am not the leader or quorum does not exist
for t in {1..30}
do
    for i in 0 1 2
    do
	date
	echo "node info after failover at pgppol$i"
	cd pgpool$i
	source ./bashrc.ports
	cd ..
	$PGPOOL_INSTALL_DIR/bin/pcp_node_info -h localhost -p $PCP_PORT
    done
    # check whether all node down.
    n0=`$PGPOOL_INSTALL_DIR/bin/pcp_node_info -h localhost -p $PCP_PORT 0|awk '{print $5}'`
    n1=`$PGPOOL_INSTALL_DIR/bin/pcp_node_info -h localhost -p $PCP_PORT 1|awk '{print $5}'`
    if [ $n0 = "down" -a $n1 = "down" ];then
	echo "all nodes go down."
	./shutdownall
	exit 1
    fi
    sleep 1
done
echo "test succeeded."

./shutdownall

exit 0
