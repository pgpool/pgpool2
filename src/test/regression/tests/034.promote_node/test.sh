#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for pcp_promote_node.
#

#
# Test case 1: pcp_promote_node successes.
#
echo "=== test1: pcp_promote_node successes ==="
source $TESTLIBS
WHOAMI=`whoami`
TESTDIR=testdir

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment
echo -n "creating test environment..."
$PGPOOL_SETUP -n 4 || exit 1
echo "done."

source ./bashrc.ports
export PGPORT=$PGPOOL_PORT

./startall
wait_for_pgpool_startup

for i in 1 2 3 4 5 6 7 8 9 10
do
    $PGPOOL_INSTALL_DIR/bin/pcp_node_info -h localhost -w -p $PCP_PORT
    if [ $? = 0 ];then
	break;
    fi
    sleep 1
done

# promote node 3
$PGPOOL_INSTALL_DIR/bin/pcp_promote_node -h localhost -w -p $PCP_PORT -s 3
wait_for_pgpool_startup

# check to see if alll nodes are up
echo -n "starting to check follow primary results: "
date
cnt=120
while [ $cnt -gt 0 ]
do
    # check to see if primary is now node 3
    $PGBIN/psql -c "show pool_nodes" test|grep down
    if [ $? != 0 ];then
	node=`$PGBIN/psql -c "show pool_nodes" test|grep primary|awk '{print $1}'`
	if [ "$node" = 3 ];then
	    echo "test succeeded"
	    ./shutdownall
	    exit 0
	fi
    fi
    cnt=`expr $cnt - 1`
    echo "cnt: $cnt"
    sleep 1
done

if [ "$test1_succeeded" = "true" ]; then
	echo "test1 succeeded."
	./shutdownall
	cd ..
else
	echo "test1 failed: node 3 did not become primary or some nodes remained down."
	./shutdownall
	exit 1
fi

#
# Test case 2: pcp_promote_node when failover_command fails.
#
echo "=== test2: pcp_promote_node when failover_command fails ==="

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# Create test environment.
echo -n "creating test environment..."
$PGPOOL_SETUP -n 2 || exit 1
echo "done."

# Make failover_command fail after its normal processing.
echo "exit 1" >> etc/failover.sh

# Disable follow_primary_command to isolate the failover failure.
sed -i \
	"/^[[:space:]]*follow_primary_command[[:space:]]*=/c\
follow_primary_command = ''" \
	etc/pgpool.conf

source ./bashrc.ports
export PGPORT=$PGPOOL_PORT

./startall
wait_for_pgpool_startup

# Wait until the PCP interface becomes available.
pcp_ready=false
for i in 1 2 3 4 5 6 7 8 9 10
do
	$PGPOOL_INSTALL_DIR/bin/pcp_node_info \
		-h localhost -w -p $PCP_PORT > /dev/null 2>&1

	if [ $? -eq 0 ]; then
		pcp_ready=true
		break
	fi

	sleep 1
done

if [ "$pcp_ready" != "true" ]; then
	echo "test2 failed: PCP interface did not become available"
	./shutdownall
	exit 1
fi

# Promote node 1. failover.sh is expected to fail.
timeout 30s \
	$PGPOOL_INSTALL_DIR/bin/pcp_promote_node \
	-h localhost -w -p $PCP_PORT -s 1 \
	> result2 2>&1

if [ $? = 124 ]; then
	echo "test2 failed: pcp_promote_node timed out"
	./shutdownall
	exit 1
fi

cat result2

grep 'executing pcp_promote_node, execution of command failed' \
	result2 > /dev/null

if [ $? != 0 ]; then
	echo "test2 failed: pcp_promote_node did not report an error"
	./shutdownall
	exit 1
fi

grep 'command: .*failover.sh' result2 > /dev/null

if [ $? != 0 ]; then
	echo "test2 failed: pcp_promote_node did not report failover.sh"
	./shutdownall
	exit 1
fi

echo "test2 succeeded."
./shutdownall
cd ..

#
# Test case 3: pcp_promote_node when follow_primary_command fails.
#
echo "=== test3: pcp_promote_node when follow_primary_command fails."

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# Create test environment.
echo -n "creating test environment..."
$PGPOOL_SETUP -n 2 || exit 1
echo "done."

# Make follow_primary_command fail after its normal processing.
echo "exit 1" >> etc/follow_primary.sh

source ./bashrc.ports
export PGPORT=$PGPOOL_PORT

./startall
wait_for_pgpool_startup

# Wait until the PCP interface becomes available.
pcp_ready=false
for i in 1 2 3 4 5 6 7 8 9 10
do
	$PGPOOL_INSTALL_DIR/bin/pcp_node_info \
		-h localhost -w -p $PCP_PORT > /dev/null 2>&1

	if [ $? -eq 0 ]; then
		pcp_ready=true
		break
	fi

	sleep 1
done

if [ "$pcp_ready" != "true" ]; then
	echo "PCP interface did not become available"
	./shutdownall
	exit 1
fi

# Promote node 1. follow_primary.sh is expected to fail.
timeout 30s \
	$PGPOOL_INSTALL_DIR/bin/pcp_promote_node \
	-h localhost -w -p $PCP_PORT -s 1 \
	> result3 2>&1

if [ $? = 124 ]; then
	echo "test3 failed: pcp_promote_node timed out"
	./shutdownall
	exit 1
fi

cat result3

grep 'executing pcp_promote_node, execution of command failed' \
	result3 > /dev/null

if [ $? != 0 ]; then
	echo "test3 failed: pcp_promote_node did not report an error"
	./shutdownall
	exit 1
fi

grep 'command: .*follow_primary.sh' result3 > /dev/null

if [ $? != 0 ]; then
	echo "test3 failed: pcp_promote_node did not report follow_primary.sh"
	./shutdownall
	exit 1
fi

echo "test3 succeeded"
./shutdownall
exit 0