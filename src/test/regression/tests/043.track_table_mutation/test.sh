#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for track table mutation feature (in-memory table tracking).
# Tests routing of queries based on recently written tables.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PSQLOPTS="-a -q -X"
PGPOOLBIN=$PGPOOL_INSTALL_DIR/bin
export PGDATABASE=test

# Only run in streaming replication mode since that's the target use case
for mode in s
do
    rm -fr $TESTDIR
    mkdir $TESTDIR
    cd $TESTDIR

    # Create test environment with 2 nodes
    echo -n "creating test environment..."
    $PGPOOL_SETUP -m $mode -n 2 || exit 1
    echo "done."

    source ./bashrc.ports

    # Configure track table mutation feature via dml_adaptive_global
    echo "disable_load_balance_on_write = 'dml_adaptive_global'" >> etc/pgpool.conf
    echo "track_table_mutation_ttl_factor = 5.0" >> etc/pgpool.conf
    echo "track_table_mutation_cold_start_duration = 10000" >> etc/pgpool.conf

    # Enable load balancing explicitly
    echo "load_balance_mode = on" >> etc/pgpool.conf

    # Configure weights so we can distinguish routing
    # Backend 0 (primary) weight=0, Backend 1 (standby) weight=1
    # This means load balanced queries go to node 1 by default
    echo "backend_weight0 = 0" >> etc/pgpool.conf
    echo "backend_weight1 = 1" >> etc/pgpool.conf

    # Enable debug logging to see routing decisions
    echo "log_min_messages = debug1" >> etc/pgpool.conf

    ./startall

    export PGPORT=$PGPOOL_PORT
    export PGHOST=localhost

    wait_for_pgpool_startup

    # Create test tables
    $PSQL test <<EOF
CREATE TABLE t1(i INTEGER);
CREATE TABLE t2(i INTEGER);
CREATE TABLE t3(i INTEGER);
EOF

    echo "=== Test 1: Cold Start Routing ==="
    # During cold start, all queries should go to primary
    # Restart pgpool to trigger cold start
    ./shutdownall
    ./startall
    wait_for_pgpool_startup

    # Immediately query - should go to primary due to cold start
    $PSQL test -c "SELECT 'cold_start_test' as marker, * FROM t1;" > /dev/null 2>&1

    # Check log for cold start message (use -a to handle binary log files)
    if grep -a -q "could not load balance because of track table mutation cold start" log/pgpool.log; then
        echo "Test 1 PASSED: Cold start routing works"
    else
        echo "Test 1 FAILED: Cold start routing not detected"
        ./shutdownall
        exit 1
    fi

    echo "=== Test 2: Wait for cold start to end ==="
    # Wait for cold start period to end (10 seconds).
    # Use generous margin to avoid flakiness under load (e.g. full regression suite).
    sleep 12

    # Clear the log
    > log/pgpool.log

    # Now a clean table query should load balance (go to node 1)
    $PSQL test -c "SELECT 'after_cold_start' as marker, * FROM t3;" > /dev/null 2>&1

    # After cold start, queries to clean tables should load balance
    # Check that it did NOT get forced to primary due to track table mutation
    if grep -a -q "could not load balance because of track table mutation cold start" log/pgpool.log; then
        echo "Test 2 FAILED: Still in cold start after waiting"
        ./shutdownall
        exit 1
    fi
    echo "Test 2 PASSED: Cold start ended correctly"

    echo "=== Test 3: Write-then-Read Routing ==="
    # Clear the log
    > log/pgpool.log

    # Write to t1 and then read - use single connection to ensure same session
    $PSQL test <<EOF
INSERT INTO t1 VALUES (1);
SELECT 'write_read_test' as marker, * FROM t1;
EOF

    # Small delay to ensure log is flushed
    sleep 0.5

    # Check log for table staleness message
    if grep -a -q "could not load balance because table.*was recently written" log/pgpool.log; then
        echo "Test 3 PASSED: Write-then-read routing works"
    else
        echo "Test 3 FAILED: Table staleness not detected after write"
        # Show relevant log entries for debugging
        grep -a -i "track_table_mutation" log/pgpool.log | tail -20
        ./shutdownall
        exit 1
    fi

    echo "=== Test 4: Clean Table Still Load Balances ==="
    # Clear the log
    > log/pgpool.log

    # Read from t2 (never written to) - should load balance
    $PSQL test -c "SELECT 'clean_table_test' as marker, * FROM t2;" > /dev/null 2>&1

    # Should NOT see track table mutation blocking message for t2
    if grep -a -q "could not load balance because table.*t2.*was recently written" log/pgpool.log; then
        echo "Test 4 FAILED: Clean table incorrectly marked as stale"
        ./shutdownall
        exit 1
    fi
    echo "Test 4 PASSED: Clean tables still load balance"

    echo "=== Test 5: UPDATE Marks Table as Stale ==="
    # Clear the log
    > log/pgpool.log

    # Update t2 and then read - use single connection
    $PSQL test <<EOF
UPDATE t2 SET i = 999 WHERE i = 0;
SELECT 'update_test' as marker, * FROM t2;
EOF

    # Small delay to ensure log is flushed
    sleep 0.5

    if grep -a -q "could not load balance because table.*was recently written" log/pgpool.log; then
        echo "Test 5 PASSED: UPDATE marks table as stale"
    else
        echo "Test 5 FAILED: UPDATE did not mark table as stale"
        ./shutdownall
        exit 1
    fi

    echo "=== Test 6: DELETE Marks Table as Stale ==="
    # Clear the log
    > log/pgpool.log

    # Delete from t3 and then read - use single connection
    $PSQL test <<EOF
DELETE FROM t3 WHERE i = 0;
SELECT 'delete_test' as marker, * FROM t3;
EOF

    # Small delay to ensure log is flushed
    sleep 0.5

    if grep -a -q "could not load balance because table.*was recently written" log/pgpool.log; then
        echo "Test 6 PASSED: DELETE marks table as stale"
    else
        echo "Test 6 FAILED: DELETE did not mark table as stale"
        ./shutdownall
        exit 1
    fi

    echo "=== Test 7: TRUNCATE Marks Table as Stale ==="
    # Clear the log
    > log/pgpool.log

    # Create a fresh table for TRUNCATE test
    $PSQL test -c "CREATE TABLE t_truncate(i INTEGER);" > /dev/null 2>&1
    $PSQL test -c "INSERT INTO t_truncate VALUES (1), (2), (3);" > /dev/null 2>&1

    # Wait for any TTL to expire
    sleep 3

    # Clear the log again
    > log/pgpool.log

    # Truncate and then read - use single connection
    $PSQL test <<EOF
TRUNCATE t_truncate;
SELECT 'truncate_test' as marker, * FROM t_truncate;
EOF

    # Small delay to ensure log is flushed
    sleep 0.5

    if grep -a -q "could not load balance because table.*was recently written" log/pgpool.log; then
        echo "Test 7 PASSED: TRUNCATE marks table as stale"
    else
        echo "Test 7 FAILED: TRUNCATE did not mark table as stale"
        grep -a -i "track_table_mutation" log/pgpool.log | tail -20
        ./shutdownall
        exit 1
    fi

    echo "=== Test 8: WITH Clause (CTE with DELETE) Marks Table as Stale ==="
    # Clear the log
    > log/pgpool.log

    # Create a fresh table for WITH test
    $PSQL test -c "CREATE TABLE t_cte(i INTEGER);" > /dev/null 2>&1
    $PSQL test -c "INSERT INTO t_cte VALUES (1), (2), (3);" > /dev/null 2>&1

    # Wait for any TTL to expire
    sleep 3

    # Clear the log again
    > log/pgpool.log

    # Use WITH clause with DELETE, then read from the table
    $PSQL test <<EOF
WITH deleted AS (DELETE FROM t_cte WHERE i = 1 RETURNING *)
SELECT * FROM deleted;
SELECT 'cte_test' as marker, * FROM t_cte;
EOF

    # Small delay to ensure log is flushed
    sleep 0.5

    if grep -a -q "could not load balance because table.*was recently written" log/pgpool.log; then
        echo "Test 8 PASSED: WITH clause (CTE) marks table as stale"
    else
        echo "Test 8 FAILED: WITH clause (CTE) did not mark table as stale"
        grep -a -i "track_table_mutation" log/pgpool.log | tail -20
        ./shutdownall
        exit 1
    fi

    # Test 9: MERGE (PostgreSQL 15+ only)
    PG_MAJOR_VERSION=$($PSQL -t -c "SELECT substring(version() from 'PostgreSQL ([0-9]+)');" | tr -d ' ')
    if [ "$PG_MAJOR_VERSION" -ge 15 ] 2>/dev/null; then
        echo "=== Test 9: MERGE Marks Table as Stale (PostgreSQL $PG_MAJOR_VERSION) ==="
        # Clear the log
        > log/pgpool.log

        # Create tables for MERGE test
        $PSQL test -c "CREATE TABLE t_merge_target(id INTEGER PRIMARY KEY, val TEXT);" > /dev/null 2>&1
        $PSQL test -c "CREATE TABLE t_merge_source(id INTEGER, val TEXT);" > /dev/null 2>&1
        $PSQL test -c "INSERT INTO t_merge_target VALUES (1, 'old');" > /dev/null 2>&1
        $PSQL test -c "INSERT INTO t_merge_source VALUES (1, 'new'), (2, 'insert');" > /dev/null 2>&1

        # Wait for any TTL to expire
        sleep 3

        # Clear the log again
        > log/pgpool.log

        # Use MERGE, then read from the target table
        $PSQL test <<EOF
MERGE INTO t_merge_target t
USING t_merge_source s ON t.id = s.id
WHEN MATCHED THEN UPDATE SET val = s.val
WHEN NOT MATCHED THEN INSERT VALUES (s.id, s.val);
SELECT 'merge_test' as marker, * FROM t_merge_target;
EOF

        # Small delay to ensure log is flushed
        sleep 0.5

        if grep -a -q "could not load balance because table.*was recently written" log/pgpool.log; then
            echo "Test 9 PASSED: MERGE marks table as stale"
        else
            echo "Test 9 FAILED: MERGE did not mark table as stale"
            grep -a -i "track_table_mutation" log/pgpool.log | tail -20
            ./shutdownall
            exit 1
        fi
    else
        echo "=== Test 9: MERGE skipped (requires PostgreSQL 15+, have $PG_MAJOR_VERSION) ==="
    fi

    echo "=== Test 10: ROLLBACK Does NOT Mark Table as Stale ==="
    # Create a fresh table for rollback test
    $PSQL test -c "CREATE TABLE t_rollback(i INTEGER);" > /dev/null 2>&1

    # Wait for any TTL to expire
    sleep 3

    # Clear the log
    > log/pgpool.log

    # Write inside a transaction, then rollback
    $PSQL test <<EOF
BEGIN;
INSERT INTO t_rollback VALUES (1);
ROLLBACK;
SELECT 'rollback_test' as marker, * FROM t_rollback;
EOF

    # Small delay to ensure log is flushed
    sleep 0.5

    # Should NOT see t_rollback marked as stale since the write was rolled back
    if grep -a -q "could not load balance because table.*t_rollback.*was recently written" log/pgpool.log; then
        echo "Test 10 FAILED: Rolled-back write incorrectly marked table as stale"
        grep -a -i "track_table_mutation" log/pgpool.log | tail -20
        ./shutdownall
        exit 1
    fi
    echo "Test 10 PASSED: ROLLBACK does not mark table as stale"

    echo "=== Test 11: COMMIT Marks Table as Stale ==="
    # Create a fresh table for commit test
    $PSQL test -c "CREATE TABLE t_commit(i INTEGER);" > /dev/null 2>&1

    # Wait for any TTL to expire
    sleep 3

    # Clear the log
    > log/pgpool.log

    # Write inside a transaction, then commit, then read
    $PSQL test <<EOF
BEGIN;
INSERT INTO t_commit VALUES (1);
COMMIT;
SELECT 'commit_test' as marker, * FROM t_commit;
EOF

    # Small delay to ensure log is flushed
    sleep 0.5

    if grep -a -q "could not load balance because table.*was recently written" log/pgpool.log; then
        echo "Test 11 PASSED: COMMIT marks table as stale"
    else
        echo "Test 11 FAILED: Committed write did not mark table as stale"
        grep -a -i "track_table_mutation" log/pgpool.log | tail -20
        ./shutdownall
        exit 1
    fi

    echo ""
    echo "=== All Track Table Mutation Tests PASSED ==="

    ./shutdownall

    cd ..
done

exit 0
