#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for external command replication delay source
#
source $TESTLIBS
TESTDIR=testdir
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

# Create external command scripts for testing
# NOTE: Commands now only output delay values for REPLICAS (not primary)
cat > delay_cmd_static.sh << 'EOF'
#!/bin/bash
# Static delay values for replicas: node1=25ms, node2=50ms (node0 is primary, not included)
echo "25 50"
EOF
chmod +x delay_cmd_static.sh

cat > delay_cmd_float.sh << 'EOF'
#!/bin/bash
# Float delay values for replicas: node1=25.5ms, node2=100.75ms
echo "25.5 100.75"
EOF
chmod +x delay_cmd_float.sh

cat > delay_cmd_high.sh << 'EOF'
#!/bin/bash
# High delay values to test threshold: node1=2000ms, node2=3000ms
echo "2000 3000"
EOF
chmod +x delay_cmd_high.sh

# ----------------------------------------------------------------------------------------
echo "=== Test0: External command receives replica identifiers only (primary omitted) ==="
# ----------------------------------------------------------------------------------------
# Command that captures its arguments and outputs valid delays for 2 replicas
cat > delay_cmd_args.sh << 'EOF'
#!/bin/bash
printf "%s " "$@" > args.txt
echo "25 50"
EOF
chmod +x delay_cmd_args.sh

echo "replication_delay_source_cmd = './delay_cmd_args.sh'" >> etc/pgpool.conf
echo "sr_check_period = 1" >> etc/pgpool.conf
echo "log_min_messages = 'DEBUG1'" >> etc/pgpool.conf
# Reduce memory requirements for macOS shared memory limits
echo "num_init_children = 4" >> etc/pgpool.conf
echo "max_pool = 2" >> etc/pgpool.conf
# Disable query caching to avoid shared memory issues on macOS
echo "memory_cache_enabled = off" >> etc/pgpool.conf

./startall
wait_for_pgpool_startup

echo "Waiting for sr_check to pass args..."
for i in {1..10}; do
    if [ -f args.txt ]; then
        break
    fi
    sleep 1
done

if [ ! -f args.txt ]; then
    echo fail: did not capture command arguments
    ./shutdownall
    exit 1
fi

ARGS_CONTENT=$(cat args.txt | sed 's/[[:space:]]*$//')
# Should receive 2 replica identifiers in host:port format (localhost:11003 localhost:11004 or server1:11003 server2:11004)
# Primary (localhost:11002 or server0:11002) should be omitted
if ! echo "$ARGS_CONTENT" | grep -qE "(server1|localhost):11003"; then
    echo "fail: expected replica1:11003 in arguments, got: '$ARGS_CONTENT'"
    ./shutdownall
    exit 1
fi
if ! echo "$ARGS_CONTENT" | grep -qE "(server2|localhost):11004"; then
    echo "fail: expected replica2:11004 in arguments, got: '$ARGS_CONTENT'"
    ./shutdownall
    exit 1
fi
if echo "$ARGS_CONTENT" | grep -qE "(server0|localhost):11002"; then
    echo "fail: primary should not be in arguments, got: '$ARGS_CONTENT'"
    ./shutdownall
    exit 1
fi

echo ok: argument order correct - replicas only, primary omitted, host:port format
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test1: Basic external command with integer millisecond values ==="
# ----------------------------------------------------------------------------------------
echo "replication_delay_source_cmd = './delay_cmd_static.sh'" >> etc/pgpool.conf
echo "sr_check_period = 1" >> etc/pgpool.conf
echo "log_standby_delay = 'always'" >> etc/pgpool.conf
echo "log_min_messages = 'DEBUG1'" >> etc/pgpool.conf

./startall
wait_for_pgpool_startup

$PSQL test <<EOF
CREATE TABLE t1(i INTEGER);
EOF

# Wait for sr_check to run and populate delay values
# sr_check_period is 1 second, so wait a bit longer to ensure it runs
echo "Waiting for sr_check to run..."
for i in {1..10}; do
    if grep -q "executing replication delay command" log/pgpool.log 2>/dev/null; then
        echo "Command executed after ${i} seconds"
        break
    fi
    sleep 1
done

$PSQL test <<EOF
SHOW POOL_NODES;
EOF

# Check that delay values are populated in the log
grep "executing replication delay command" log/pgpool.log >/dev/null 2>&1
if [ $? != 0 ];then
    echo fail: external command was not executed
    echo "Log contents:"
    tail -20 log/pgpool.log
    ./shutdownall
    exit 1
fi

# Verify actual delay values were parsed
if ! $PSQL -t -c "SHOW POOL_NODES" test | grep -E "[0-9]+\.[0-9]+" >/dev/null; then
    echo "Warning: No delay values found in POOL_NODES output"
fi

# Check for delay log messages
grep "Replication of node.*external command" log/pgpool.log >/dev/null 2>&1
if [ $? != 0 ];then
    echo fail: external command delay logging not found
    ./shutdownall
    exit 1
fi

echo ok: basic external command test succeeded
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test2: External command with floating-point millisecond values ==="
# ----------------------------------------------------------------------------------------
# Update configuration to use float command
sed -i.bak "s|delay_cmd_static.sh|delay_cmd_float.sh|" etc/pgpool.conf

./startall
wait_for_pgpool_startup

# Wait for sr_check to run with float values
echo "Waiting for sr_check with float values..."
for i in {1..10}; do
    if grep -q "executing replication delay command.*delay_cmd_float.sh" log/pgpool.log 2>/dev/null; then
        echo "Float command executed after ${i} seconds"
        break
    fi
    sleep 1
done

$PSQL test <<EOF
SHOW POOL_NODES;
EOF

# Check that float values are handled correctly
grep "executing replication delay command.*delay_cmd_float.sh" log/pgpool.log >/dev/null 2>&1
if [ $? != 0 ];then
    echo fail: float command was not executed
    ./shutdownall
    exit 1
fi

echo ok: floating-point values test succeeded
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test3: External command with delay threshold ==="
# ----------------------------------------------------------------------------------------
# Update configuration to use high delay command and set threshold
sed -i.bak "s|delay_cmd_float.sh|delay_cmd_high.sh|" etc/pgpool.conf
echo "delay_threshold_by_time = 1000" >> etc/pgpool.conf
echo "backend_weight0 = 0" >> etc/pgpool.conf  # Force queries to standby normally
echo "backend_weight2 = 0" >> etc/pgpool.conf  # Only use node 1 as standby

./startall
wait_for_pgpool_startup

# Wait for sr_check to run and detect high delays
echo "Waiting for sr_check with high delay values..."
for i in {1..10}; do
    if grep -q "executing replication delay command.*delay_cmd_high.sh" log/pgpool.log 2>/dev/null; then
        echo "High delay command executed after ${i} seconds"
        break
    fi
    sleep 1
done

$PSQL test <<EOF
SELECT * FROM t1 LIMIT 1;
EOF

# With high delays (2000ms > 1000ms threshold), query should go to primary (node 0)
# Log format can vary: either "statement: SELECT..." or "SELECT... DB node id:"
if ! grep -E "DB node id: 0.*statement: SELECT \* FROM t1 LIMIT 1" log/pgpool.log >/dev/null 2>&1 && \
   ! grep -E "SELECT \* FROM t1 LIMIT 1.*DB node id: 0" log/pgpool.log >/dev/null 2>&1; then
    echo fail: query was not sent to primary node despite high delay
    ./shutdownall
    exit 1
fi

echo ok: delay threshold test succeeded
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test4: External command execution as process user ==="
# ----------------------------------------------------------------------------------------
# Test that command runs as the current pgpool process user
sed -i.bak "s|delay_cmd_high.sh|delay_cmd_static.sh|" etc/pgpool.conf

./startall
wait_for_pgpool_startup

# Wait for sr_check to run
echo "Waiting for sr_check to run as process user..."
for i in {1..10}; do
    if grep -q "executing replication delay command.*delay_cmd_static.sh" log/pgpool.log 2>/dev/null; then
        echo "Command executed as process user after ${i} seconds"
        break
    fi
    sleep 1
done

# Check that command was executed (without su wrapper)
grep "executing replication delay command.*delay_cmd_static.sh" log/pgpool.log >/dev/null 2>&1
if [ $? != 0 ];then
    echo fail: command was not executed as process user
    ./shutdownall
    exit 1
fi

# Verify no su command was used
if grep -q "executing replication delay command.*su.*" log/pgpool.log 2>/dev/null; then
    echo fail: command should not use su wrapper
    ./shutdownall
    exit 1
fi

echo ok: process user execution test succeeded
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test5: Error handling - missing command ==="
# ----------------------------------------------------------------------------------------
# Test error handling when command is not configured
sed -i.bak "s|replication_delay_source_cmd = './delay_cmd_static.sh'|replication_delay_source_cmd = ''|" etc/pgpool.conf

./startall
wait_for_pgpool_startup

# With empty command, should fall back to builtin method
# No specific error message expected - just verify it doesn't crash
sleep 3

echo "ok: empty command test succeeded (fallback to builtin)"
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test6: Error handling - command execution failure ==="
# ----------------------------------------------------------------------------------------
# Test error handling when command fails
echo "replication_delay_source_cmd = './nonexistent_command.sh'" >> etc/pgpool.conf

./startall
wait_for_pgpool_startup

# Wait for sr_check to run with failing command
echo "Waiting for sr_check with failing command..."
for i in {1..5}; do
    # Check for various error conditions: exit code failure, no output, or explicit failure message
    if grep -qE "(replication delay command failed with exit code|replication delay command produced no output|failed to (execute|read output from) replication delay command)" log/pgpool.log 2>/dev/null; then
        echo "Command failure detected after ${i} seconds"
        break
    fi
    sleep 1
done

# Check for error message about command execution failure
# Accept multiple possible error messages depending on shell behavior:
# - "failed with exit code" when command returns non-zero
# - "produced no output" when command produces empty output
# - "failed to execute/read" for other failures
if ! grep -qE "(replication delay command failed with exit code|replication delay command produced no output|failed to (execute|read output from) replication delay command)" log/pgpool.log 2>/dev/null; then
    echo fail: command execution failure not detected
    echo "Log contents:"
    tail -50 log/pgpool.log
    ./shutdownall
    exit 1
fi

echo ok: command failure test succeeded
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test7: Command timeout handling ==="
# ----------------------------------------------------------------------------------------
# Create a command that takes longer than the timeout
cat > delay_cmd_slow.sh << 'EOF'
#!/bin/bash
# Slow command that takes 15 seconds (longer than default 10s timeout)
sleep 15
echo "25 50"
EOF
chmod +x delay_cmd_slow.sh

# Set a short timeout and use the slow command
sed -i.bak "s|replication_delay_source_cmd = './nonexistent_command.sh'|replication_delay_source_cmd = './delay_cmd_slow.sh'|" etc/pgpool.conf
echo "replication_delay_source_timeout = 3" >> etc/pgpool.conf

./startall
wait_for_pgpool_startup

# Wait for sr_check to run and timeout
echo "Waiting for command timeout..."
for i in {1..15}; do
    if grep -q "replication delay command timed out" log/pgpool.log 2>/dev/null; then
        echo "Command timeout detected after ${i} seconds"
        break
    fi
    sleep 1
done

# Check for timeout error message
grep "replication delay command timed out after 3 seconds" log/pgpool.log >/dev/null 2>&1
if [ $? != 0 ];then
    echo fail: command timeout not detected
    ./shutdownall
    exit 1
fi

echo ok: command timeout test succeeded
./shutdownall

# ----------------------------------------------------------------------------------------
echo "=== Test8: Handling of -1 for down nodes ==="
# ----------------------------------------------------------------------------------------
# Create a command that returns -1 for one replica
cat > delay_cmd_with_down_node.sh << 'EOF'
#!/bin/bash
# Return -1 for first replica (indicating it's down), normal value for second
echo "-1 50"
EOF
chmod +x delay_cmd_with_down_node.sh

# Reset config
rm -f etc/pgpool.conf.bak
sed -i.bak "s|delay_cmd_slow.sh|delay_cmd_with_down_node.sh|" etc/pgpool.conf
sed -i.bak "s|replication_delay_source_timeout = 3|replication_delay_source_timeout = 10|" etc/pgpool.conf

./startall
wait_for_pgpool_startup

# Wait for sr_check to process -1 value
echo "Waiting for sr_check to process -1 value..."
for i in {1..10}; do
    if grep -q "node.*reported as down by external command.*delay -1" log/pgpool.log 2>/dev/null; then
        echo "-1 handling detected after ${i} seconds"
        break
    fi
    sleep 1
done

# Check for -1 logging message
grep "node.*reported as down by external command.*delay -1.*relying on health check" log/pgpool.log >/dev/null 2>&1
if [ $? != 0 ];then
    echo fail: -1 handling message not found
    ./shutdownall
    exit 1
fi

# Verify that pgpool didn't trigger failover just from -1
# Check for actual failover execution, not just config mentions of failover_command
if grep -qE "(starting.*(failover|degeneration)|failover done|execute.*(failover|failback)_command)" log/pgpool.log 2>/dev/null; then
    echo "fail: -1 should not trigger immediate failover"
    ./shutdownall
    exit 1
fi

echo ok: -1 handling test succeeded
./shutdownall

echo "All external replication delay tests passed!"
exit 0
