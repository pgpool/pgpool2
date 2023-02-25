#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for load balancing.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PSQLOPTS="-a -q -X"
PGPOOLBIN=$PGPOOL_INSTALL_DIR/bin
export PGDATABASE=test

# sleep time after reload in seconds
st=10

# function to check the result
# argument is test case number.
function check_result
{
    diff -c ../expected/expected$1$suffix result$1

    if [ $? = 0 ];then
	echo "test$1 succeeded."
    else
	echo "test$1 failed."
	./shutdownall
	exit 1
    fi
}

# main test script

for mode in s r i
do
    rm -fr $TESTDIR
    mkdir $TESTDIR
    cd $TESTDIR

    # create test environment
    echo -n "creating test environment..."
    $PGPOOL_SETUP -m $mode -n 2 || exit 1
    echo "done."

    source ./bashrc.ports

	# set expected file suffix
	if [ $mode = 's' ];then
	    suffix='-s'
	else
	    suffix='-r'
	fi

	echo "=== test1 started ==="

	echo "backend_weight0 = 0" >> etc/pgpool.conf
	echo "backend_weight1 = 1" >> etc/pgpool.conf
	echo "write_function_list = 'f1,public.f2'" >> etc/pgpool.conf
	echo "notice_per_node_statement = on" >> etc/pgpool.conf

	./startall

	export PGPORT=$PGPOOL_PORT

	wait_for_pgpool_startup

	$PSQL $PSQLOPTS > result1 2>&1 <<EOF
CREATE TABLE t1(i INTEGER);
CREATE TABLE t2(i INTEGER);
CREATE FUNCTION f1(INTEGER) returns INTEGER AS 'SELECT \$1' LANGUAGE SQL;
CREATE FUNCTION f2(INTEGER) returns INTEGER AS 'SELECT \$1' LANGUAGE SQL;
SELECT * FROM t1;		-- this load balances
SELECT f1(1);			-- this does not load balance
SELECT public.f2(1);	-- this does not load balance
EOF
	check_result 1

	echo "=== test2 started ==="
	# check if read_only function list works
	echo "read_only_function_list = 'f1,public.f2'" >> etc/pgpool.conf
	echo "write_function_list = ''" >> etc/pgpool.conf

	./pgpool_reload
	sleep $st

	$PSQL $PSQLOPTS > result2 2>&1 <<EOF
SELECT f1(1);			-- this does load balance
SELECT public.f2(1);	-- this does load balance
EOF
	check_result 2

	echo "=== test3 started ==="
	# check if primary routing query pattern list worked
	./shutdownall
	echo "primary_routing_query_pattern_list = 'SELECT \'a\'\;;SELECT 1\;;SELECT \'\;\'\;;SELECT \* FROM t1\;;^.*t2.*\;$;^.*f1.*$'" >> etc/pgpool.conf
	./startall
	wait_for_pgpool_startup

	$PSQL $PSQLOPTS > result3 2>&1 <<EOF
SELECT * FROM t1;
SELECT 'a';
SELECT 1;
SELECT ';';
SELECT * FROM t2;
SELECT f1(1);
EOF
	# If streaming replication mode, all queries are sent to primary node only.
	# If query match both primary_routing_query_pattern_list and read_only_function_list,
	# read_only_function_list will be ignored, and query is sent to primary node only.
	#
	# If replication node, all queries are load-balanced.
	check_result 3

	echo "=== test4 started ==="
	# check if statement level load balance works
	./shutdownall
	echo "read_only_function_list = ''" >> etc/pgpool.conf
	echo "write_function_list = ''" >> etc/pgpool.conf
	echo "statement_level_load_balance = on" >> etc/pgpool.conf
	echo "log_min_messages = debug1" >> etc/pgpool.conf

	./startall
	sleep $st

	$PSQL test <<EOF
SELECT 3333;
SELECT 4444;
EOF

	n=`grep "selecting load balance node" log/pgpool.log | wc -l`
	if [ $n != 3 ]; then
	# expected result not found
	    echo "fail: statement level load balance doesn't work."
	    ./shutdownall
	    exit 1
	fi
	echo ok: statement level load balance works.

# in replication mode if load_balance_mode = off, SELECT query inside
# an explicit transaction should be sent to main node only.
	if [ $mode = "r" ];then
	    ./shutdownall
	    echo "load_balance_mode = off" >> etc/pgpool.conf
	    ./startall
	    wait_for_pgpool_startup

	    $PSQL $PSQLOPTS > result4 2>&1 <<EOF
BEGIN;
SELECT 1;
END;
EOF
	    check_result 4

	    echo "=== test5 started ==="

# in replication mode if load_balance_mode = off, SELECT query
# including writing function should be sent to all the nodes.
# per [pgpool-general: 2221].
	    echo "write_function_list = 'f1'" >> etc/pgpool.conf
	    echo "read_only_function_list = ''" >> etc/pgpool.conf
	    ./pgpool_reload
	    sleep $st
	    $PSQL $PSQLOPTS > result5 2>&1 <<EOF
SELECT f1(2);		-- this should be sent to all the nodes
EOF
	    check_result 5
	fi

	echo "=== test6 started ==="
# -------------------------------------------------------------------------------
# check the case when write_function_list and read_only_function_list are both empty.
# In this case pg_proc.provolatile is checked. If it is 'v' (volatile), then the
# function is regarded doing writes.
# Since f1() and f2() were declared without volatility property, they are regarded
# as volatile functions.
# -------------------------------------------------------------------------------
	echo "load_balance_mode = on" >> etc/pgpool.conf
	echo "write_function_list = ''" >> etc/pgpool.conf
	echo "read_only_function_list = ''" >> etc/pgpool.conf
	./pgpool_reload
	sleep $st

	$PSQL $PSQLOPTS > result6 2>&1 <<EOF
SELECT f1(1);
SELECT public.f2(1);
EOF
	check_result 6 $PSQLVERSION

	echo "=== test7 started ==="
# -------------------------------------------------------------------------------
# multi statement queries
# -------------------------------------------------------------------------------
	echo "statement_level_load_balance = off" >> etc/pgpool.conf
	# XXX primary_routing_query_pattern_list does not allow to overwritten.
	# So following does not work.
	#echo "primary_routing_query_pattern_list = ''" >> etc/pgpool.conf
	sed -i '/^primary_routing_query_pattern_list/d' etc/pgpool.conf
	#echo "log_min_messages = debug5" >> etc/pgpool.conf

	./shutdownall
	./startall
	wait_for_pgpool_startup
	$PSQL -c "SHOW POOL_NODES;" test

	$PSQL $PSQLOPTS < ../sql/7.sql > result7 2>&1
	check_result 7

	echo "=== test8 started ==="
# -------------------------------------------------------------------------------
# multi statement queries (swapping primary and standby)
# -------------------------------------------------------------------------------
	if [ $mode = 's' ];then
	    echo $PGPOOLBIN/pcp_promote_node -w -p $PCP_PORT --switchover 1
	    $PGPOOLBIN/pcp_promote_node -w -p $PCP_PORT --switchover 1
	    while :
	    do
		wait_for_pgpool_startup
		$PSQL -c "SHOW POOL_NODES;" test | grep down
		if [ $? != 0 ];then
		    break
		fi
		sleep 1
	    done

	    # Swap the weights. Now backend 0 is the load balance node
	    echo "backend_weight0 = 1" >> etc/pgpool.conf
	    echo "backend_weight1 = 0" >> etc/pgpool.conf
	    ./shutdownall
	    ./startall
	    wait_for_pgpool_startup
	    $PSQL $PSQLOPTS < ../sql/7.sql > result8 2>&1
	    check_result 8
	fi
	./shutdownall

	cd ..
done

exit 0
