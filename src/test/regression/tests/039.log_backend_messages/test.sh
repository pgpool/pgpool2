#!/usr/bin/env bash
#-------------------------------------------------------------------
# Test script for log_backend_messages
#
source $TESTLIBS
export PGDATABASE=test
TESTDIR=testdir
PSQL=$PGBIN/psql
PG_CTL=$PGBIN/pg_ctl
PGPROTO=$PGPOOL_INSTALL_DIR/bin/pgproto

# Loop test for streaming replication mode, snapshot isolation mode
# and raw mode
for mode in s i n
do
    rm -fr $TESTDIR
    mkdir $TESTDIR
    cd $TESTDIR

    echo -n "creating test environment..."
    $PGPOOL_SETUP -m $mode || exit 1
    echo "done."

    echo > result

    # We set backend_weight0 to 0 to send ready queries to backend 1.
    # We set client_min_messages to log so that log messages appear on
    # the client screen.
    # We set connection_cache to off so that each time client connects
    # to pgpool, it receives ready for query from backend.
        cat >> etc/pgpool.conf <<EOF
backend_weight0 = 0
client_min_messages = log
log_per_node_statement = off
connection_cache = off
EOF
    for option in none terse verbose
    do
	echo "==== mode: $mode option: $option ===" >> result

	cat >> etc/pgpool.conf <<EOF
log_backend_messages = $option
EOF
	source ./bashrc.ports
	./startall
	wait_for_pgpool_startup

	export PGPORT=$PGPOOL_PORT

	$PSQL <<EOF
DROP TABLE t1;
CREATE TABLE t1(i int);
INSERT INTO t1 SELECT generate_series(1,3);
EOF

	# Simple query protocol
	$PSQL >> result 2>&1 <<EOF
SELECT * FROM t1;
EOF

	# Extended query protocol
	$PGPROTO -d $PGDATABASE -p $PGPOOL_PORT -f ../pgproto.data |& sed 's/F pool_proto.*//' >> result 2>&1

	./shutdownall
    done

    diff -c result ../expected.$mode
    if [ $? != 0 ];then
	echo "test failed in mode: $mode"
	exit 1
    fi

    cd ..
done

exit 0
