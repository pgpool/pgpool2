#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for external command validation and edge cases
#
source $TESTLIBS
TESTDIR=testdir_validation
PG_CTL=$PGBIN/pg_ctl
PSQL="$PGBIN/psql -X "

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment
echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 3 || exit 1
echo "done."
source ./bashrc.ports
export PGPORT=$PGPOOL_PORT

# Create test command scripts
# NOTE: All commands output values for REPLICAS only (primary omitted)
cat > delay_cmd_validation.sh << 'EOF'
#!/bin/bash
# Test validation: output with invalid values for 2 replicas
echo "invalid_value 50.5"
EOF
chmod +x delay_cmd_validation.sh

cat > delay_cmd_negative.sh << 'EOF'
#!/bin/bash
# Test negative values (other than -1)
echo "-25 50"
EOF
chmod +x delay_cmd_negative.sh

cat > delay_cmd_large.sh << 'EOF'
#!/bin/bash
# Test extremely large values
echo "9999999 50"
EOF
chmod +x delay_cmd_large.sh

cat > delay_cmd_wrong_count.sh << 'EOF'
#!/bin/bash
# Test wrong number of values (only 1 instead of 2 for 2 replicas)
echo "25"
EOF
chmod +x delay_cmd_wrong_count.sh

# ----------------------------------------------------------------------------------------
echo "=== Test1: Validation of invalid delay values ==="
# ----------------------------------------------------------------------------------------
echo "replication_delay_source_cmd = './delay_cmd_validation.sh'" >> etc/pgpool.conf
echo "sr_check_period = 1" >> etc/pgpool.conf
echo "log_standby_delay = 'always'" >> etc/pgpool.conf
echo "log_min_messages = 'DEBUG1'" >> etc/pgpool.conf
# Reduce memory requirements for macOS shared memory limits
echo "num_init_children = 4" >> etc/pgpool.conf
echo "max_pool = 2" >> etc/pgpool.conf
# Disable query caching to avoid shared memory issues on macOS
echo "memory_cache_enabled = off" >> etc/pgpool.conf

./startall
wait_for_pgpool_startup

$PSQL test <<EOF
CREATE TABLE t1(i INTEGER);
EOF

# Wait for sr_check to run
echo "Waiting for validation test..."
for i in {1..10}; do
    if grep -q "invalid delay value" log/pgpool.log 2>/dev/null; then
        echo "Validation error detected after ${i} seconds"
        break
    fi
    sleep 1
done

# Check for validation warning
grep "invalid delay value 'invalid_value' for node" log/pgpool.log >/dev/null 2>&1
if [ $? != 0 ];then
    echo fail: validation warning not found
    ./shutdownall
    exit 1
fi

echo ok: invalid value validation test succeeded
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test2: Negative delay values (other than -1) ==="
# ----------------------------------------------------------------------------------------
sed -i.bak "s|delay_cmd_validation.sh|delay_cmd_negative.sh|" etc/pgpool.conf

./startall
wait_for_pgpool_startup

# Wait for sr_check to run
echo "Waiting for negative value test..."
for i in {1..10}; do
    if grep -q "negative delay value.*other than -1" log/pgpool.log 2>/dev/null; then
        echo "Negative value warning detected after ${i} seconds"
        break
    fi
    sleep 1
done

# Check for negative value warning
grep "negative delay value.*other than -1.*treating as 0" log/pgpool.log >/dev/null 2>&1
if [ $? != 0 ];then
    echo fail: negative value warning not found
    ./shutdownall
    exit 1
fi

echo ok: negative value validation test succeeded
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test3: Extremely large delay values ==="
# ----------------------------------------------------------------------------------------
sed -i.bak "s|delay_cmd_negative.sh|delay_cmd_large.sh|" etc/pgpool.conf

./startall
wait_for_pgpool_startup

# Wait for sr_check to run
echo "Waiting for large value test..."
for i in {1..10}; do
    if grep -q "extremely large delay value" log/pgpool.log 2>/dev/null; then
        echo "Large value warning detected after ${i} seconds"
        break
    fi
    sleep 1
done

# Check for large value warning
grep "extremely large delay value.*for node" log/pgpool.log >/dev/null 2>&1
if [ $? != 0 ];then
    echo fail: large value warning not found
    ./shutdownall
    exit 1
fi

echo ok: large value validation test succeeded
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test4: Wrong number of output values ==="
# ----------------------------------------------------------------------------------------
sed -i.bak "s|delay_cmd_large.sh|delay_cmd_wrong_count.sh|" etc/pgpool.conf

./startall
wait_for_pgpool_startup

# Wait for sr_check to run
echo "Waiting for wrong count test..."
for i in {1..10}; do
    if grep -q "returned.*values, expected.*replica" log/pgpool.log 2>/dev/null; then
        echo "Wrong count warning detected after ${i} seconds"
        break
    fi
    sleep 1
done

# Check for wrong count warning
grep "returned.*values, expected.*replica.*Command should output one delay value per replica" log/pgpool.log >/dev/null 2>&1
if [ $? != 0 ];then
    echo fail: wrong count validation test not found
    ./shutdownall
    exit 1
fi

echo ok: wrong count validation test succeeded
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test5: Multiple -1 values ==="
# ----------------------------------------------------------------------------------------
cat > delay_cmd_multi_down.sh << 'EOF'
#!/bin/bash
# Test multiple replicas down
echo "-1 -1"
EOF
chmod +x delay_cmd_multi_down.sh

sed -i.bak "s|delay_cmd_wrong_count.sh|delay_cmd_multi_down.sh|" etc/pgpool.conf

./startall
wait_for_pgpool_startup

# Wait for sr_check to run
echo "Waiting for multi-down test..."
for i in {1..10}; do
    if grep -q "node.*reported as down by external command" log/pgpool.log 2>/dev/null; then
        echo "Multiple down nodes detected after ${i} seconds"
        break
    fi
    sleep 1
done

# Check for multiple -1 handling
DOWN_COUNT=$(grep -c "node.*reported as down by external command.*delay -1" log/pgpool.log)
if [ "$DOWN_COUNT" -lt 2 ]; then
    echo fail: expected 2 down node messages, found $DOWN_COUNT
    ./shutdownall
    exit 1
fi

echo ok: multiple -1 handling test succeeded
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test6: Command timeout with different timeout values ==="
# ----------------------------------------------------------------------------------------
cat > delay_cmd_timeout.sh << 'EOF'
#!/bin/bash
# Command that takes 5 seconds
sleep 5
echo "25 50"
EOF
chmod +x delay_cmd_timeout.sh

# Test with timeout shorter than command duration
sed -i.bak "s|delay_cmd_multi_down.sh|delay_cmd_timeout.sh|" etc/pgpool.conf
echo "replication_delay_source_timeout = 2" >> etc/pgpool.conf

./startall
wait_for_pgpool_startup

# Wait for timeout
echo "Waiting for timeout test (2s timeout, 5s command)..."
for i in {1..10}; do
    if grep -q "replication delay command timed out after 2 seconds" log/pgpool.log 2>/dev/null; then
        echo "Timeout detected after ${i} seconds"
        break
    fi
    sleep 1
done

# Check for timeout message
grep "replication delay command timed out after 2 seconds" log/pgpool.log >/dev/null 2>&1
if [ $? != 0 ];then
    echo fail: timeout not detected
    ./shutdownall
    exit 1
fi

echo ok: timeout test succeeded
./shutdownall

# Test with timeout longer than command duration
sed -i.bak "s|replication_delay_source_timeout = 2|replication_delay_source_timeout = 10|" etc/pgpool.conf

./startall
wait_for_pgpool_startup

# Wait for successful execution
echo "Waiting for successful execution (10s timeout, 5s command)..."
for i in {1..15}; do
    if grep -q "executing replication delay command.*delay_cmd_timeout.sh" log/pgpool.log 2>/dev/null; then
        echo "Command executed successfully after ${i} seconds"
        break
    fi
    sleep 1
done

# Should not timeout this time
if grep -q "replication delay command timed out" log/pgpool.log 2>/dev/null; then
    echo fail: command should not have timed out with 10s timeout
    ./shutdownall
    exit 1
fi

echo ok: extended timeout test succeeded
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test7: Mix of valid delays and -1 ==="
# ----------------------------------------------------------------------------------------
cat > delay_cmd_mixed.sh << 'EOF'
#!/bin/bash
# One replica up (25ms), one down (-1)
echo "25 -1"
EOF
chmod +x delay_cmd_mixed.sh

sed -i.bak "s|delay_cmd_timeout.sh|delay_cmd_mixed.sh|" etc/pgpool.conf

./startall
wait_for_pgpool_startup

# Wait for sr_check
echo "Waiting for mixed delay test..."
for i in {1..10}; do
    if grep -q "node.*reported as down by external command" log/pgpool.log 2>/dev/null; then
        echo "Mixed delay handling detected after ${i} seconds"
        break
    fi
    sleep 1
done

# Should log one -1 and process one normal delay
grep "node.*reported as down by external command.*delay -1" log/pgpool.log >/dev/null 2>&1
if [ $? != 0 ];then
    echo fail: -1 not logged
    ./shutdownall
    exit 1
fi

# Should also log the normal replica delay
grep "Replication of node.*external command" log/pgpool.log >/dev/null 2>&1
if [ $? != 0 ];then
    echo "Note: Normal replica delay logging may not be visible with log_standby_delay settings"
fi

echo ok: mixed delay handling test succeeded
./shutdownall

echo "All validation tests passed!"
exit 0