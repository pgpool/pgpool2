#!/usr/bin/env bash
#-------------------------------------------------------------------
# Test script for track table mutation global cold start
# on watchdog leader change.
#
# Uses $WATCHDOG_SETUP to create a 2-node watchdog cluster,
# then verifies that when the leader is stopped the new
# leader triggers a global cold start.
#-------------------------------------------------------------------
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
success_count=0

dir=`pwd`
rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# Create 2-node watchdog cluster
$WATCHDOG_SETUP -wn 2 || exit 1

# Ensure per-node scripts are executable
# (sed -i in watchdog_setup can strip permissions)
chmod 755 pgpool*/startall pgpool*/shutdownall

# Append track_table_mutation config to both nodes
for i in 0 1
do
	cat >> pgpool${i}/etc/pgpool.conf <<EOF
disable_load_balance_on_write = 'dml_adaptive_global'
track_table_mutation_cold_start_duration = 2000
enable_consensus_with_half_votes = on
log_min_messages = debug1
EOF
done

./startall
export PCPPASSFILE=$dir/$TESTDIR/pgpool0/pcppass

# Wait for watchdog lifecheck on node 0
echo -n "waiting for watchdog node 0 starting up..."
for i in 1 2 3 4 5 6 7 8 9 10
do
	grep "lifecheck started" \
		pgpool0/log/pgpool.log > /dev/null 2>&1
	if [ $? = 0 ]; then
		break
	fi
	sleep 2
done
echo "done."

# Test 1: Verify leader came up
echo "=== Test 1: Waiting for the pgpool leader... ==="
for i in 1 2 3 4 5 6 7 8 9 10
do
	grep "I am the cluster leader node" \
		pgpool0/log/pgpool.log > /dev/null 2>&1
	if [ $? = 0 ]; then
		success_count=$(( success_count + 1 ))
		echo "Test 1 PASSED: Leader brought up."
		break
	fi
	echo "[check] $i times"
	sleep 2
done

if [ $success_count -lt 1 ]; then
	echo "Test 1 FAILED: Leader did not start"
	./shutdownall
	exit 1
fi

# Test 2: Verify standby joined cluster
echo "=== Test 2: Waiting for standby to join... ==="
for i in 1 2 3 4 5 6 7 8 9 10
do
	grep "successfully joined the watchdog cluster" \
		pgpool1/log/pgpool.log > /dev/null 2>&1
	if [ $? = 0 ]; then
		success_count=$(( success_count + 1 ))
		echo "Test 2 PASSED: Standby joined."
		break
	fi
	echo "[check] $i times"
	sleep 2
done

if [ $success_count -lt 2 ]; then
	echo "Test 2 FAILED: Standby did not join"
	./shutdownall
	exit 1
fi

# Test 3: Verify track_table_mutation initialized
echo "=== Test 3: Verify feature initialized ==="
if grep -a "track_table_mutation: initialized" \
	pgpool0/log/pgpool.log > /dev/null 2>&1; then
	success_count=$(( success_count + 1 ))
	echo "Test 3 PASSED: Feature initialized."
else
	echo "Test 3 FAILED: Feature not initialized"
	./shutdownall
	exit 1
fi

# Test 4: Stop leader (pgpool0) to trigger failover
echo "=== Test 4: Stopping leader... ==="
cd pgpool0
source ./bashrc.ports
$PGPOOL_INSTALL_DIR/bin/pgpool \
	-f etc/pgpool.conf -m f stop
cd ..

echo "Checking standby detected shutdown..."
for i in 1 2 3 4 5 6 7 8 9 10
do
	grep -a "is shutting down" \
		pgpool1/log/pgpool.log > /dev/null 2>&1
	if [ $? = 0 ]; then
		success_count=$(( success_count + 1 ))
		echo "Test 4 PASSED: Shutdown detected."
		break
	fi
	echo "[check] $i times"
	sleep 2
done

if [ $success_count -lt 4 ]; then
	echo "Test 4 FAILED: Shutdown not detected"
	./shutdownall
	exit 1
fi

# Test 5: Verify standby became new leader
echo "=== Test 5: Checking standby takes over... ==="
for i in 1 2 3 4 5 6 7 8 9 10
do
	grep -a "I am the cluster leader node" \
		pgpool1/log/pgpool.log > /dev/null 2>&1
	if [ $? = 0 ]; then
		success_count=$(( success_count + 1 ))
		echo "Test 5 PASSED: Standby became leader."
		break
	fi
	echo "[check] $i times"
	sleep 2
done

if [ $success_count -lt 5 ]; then
	echo "Test 5 FAILED: Standby did not become leader"
	./shutdownall
	exit 1
fi

# Test 6: Verify global cold start was triggered
echo "=== Test 6: Checking global cold start... ==="
for i in 1 2 3 4 5 6 7 8 9 10
do
	grep -a "track_table_mutation: global cold start" \
		pgpool1/log/pgpool.log > /dev/null 2>&1
	if [ $? = 0 ]; then
		success_count=$(( success_count + 1 ))
		echo "Test 6 PASSED: Global cold start triggered."
		break
	fi
	echo "[check] $i times"
	sleep 2
done

# Cleanup
./shutdownall

echo ""
echo "$success_count out of 6 successful"

if test $success_count -eq 6
then
	echo "=== All Watchdog Tests PASSED ==="
	exit 0
fi

exit 1
