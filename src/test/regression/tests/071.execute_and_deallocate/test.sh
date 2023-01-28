#!/usr/bin/env bash
#-------------------------------------------------------------------
# Testing loadbalance failure using DEALLOCATE and EXECUTE command
# case with streaming replication mode.

source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
for mode in s r i n
do
    echo "=== starting test in \"$mode\" mode ==="
    if [ $mode = "n" ];then
	num_tests=5
    else
	num_tests=6
    fi
    success_count=0

    rm -fr $TESTDIR
    mkdir $TESTDIR
    cd $TESTDIR

    # create test environment
    echo -n "creating test environment..."
    $PGPOOL_SETUP -m $mode -n 2 || exit 1
    echo "done."

    source ./bashrc.ports

    export PGPORT=$PGPOOL_PORT

    # set backend_weight , loadbalance to standby only
    echo "backend_weight0 = 0" >> etc/pgpool.conf
    echo "backend_weight1 = 1" >> etc/pgpool.conf

    # start pgpool-II
    ./startall

    sleep 1

    # run test1 select query
    $PSQL -p 11000 test <<EOF
PREPARE test1 AS SELECT 1;
EXECUTE test1;
DEALLOCATE test1;
EOF

    # run test2 update query
    $PSQL -p 11000 test <<EOF
CREATE TABLE test_tbl(id int, name text);
PREPARE test2 AS UPDATE test_tbl SET id =2;
EXECUTE test2;
DEALLOCATE test2;
DEALLOCATE all;
EOF

    expect1=`fgrep "PREPARE test1" log/pgpool.log  | awk '{print substr($0, index($0, "DB node id:"),13)}'`
    expect2=`fgrep "PREPARE test2" log/pgpool.log  | awk '{print substr($0, index($0, "DB node id:"),13)}'`

    #test1 result
    echo -n "case 1: PREPARE and EXECUTE with SELECT query..."
    result=`fgrep "EXECUTE test1" log/pgpool.log  | awk '{print substr($0, index($0, "DB node id:"),13)}'`
    if [  "$expect1" = "$result" ]; then
        success_count=$(( success_count + 1 ))
	echo "ok."
    else
	echo "failed."
    fi

    echo -n "case 2: PREPARE and DEALLOCATE with SELECT query..."
    result=`fgrep "DEALLOCATE test1" log/pgpool.log  | awk '{print substr($0, index($0, "DB node id:"),13)}'`
    if [  "$expect1" = "$result" ]; then
        success_count=$(( success_count + 1 ))
	echo "ok."
    else
	echo "failed."
    fi

    #test2 result
    echo -n "case 3: PREPARE and EXECUTE with UPDATE query..."
    result=`fgrep "EXECUTE test2" log/pgpool.log  | awk '{print substr($0, index($0, "DB node id:"),13)}'`
    if [  "$expect2" = "$result" ]; then
        success_count=$(( success_count + 1 ))
	echo "ok."
    else
	echo "failed."
    fi

    echo -n "case 4: PREPARE and DEALLOCATE with UPDATE query..."
    result=`fgrep "DEALLOCATE test2" log/pgpool.log  | awk '{print substr($0, index($0, "DB node id:"),13)}'`
    if [  "$expect2" = "$result" ]; then
        success_count=$(( success_count + 1 ))
	echo "ok."
    else
	echo "failed."
    fi

    # DEALLOCATE all;
    echo -n "case 5: node0 DEALLOCATE all query..."
    grep -E "DB node id: 0 .*DEALLOCATE all" log/pgpool.log >/dev/null
    if [  $? -eq 0 ]; then
        success_count=$(( success_count + 1 ))
	echo "ok."
    else
	echo "failed."
    fi

    echo -n "case 6: node1 DEALLOCATE all query..."
    if [ $mode = "n" ];then
	echo "this test is not applied to mode \"$mode\" and skipped."
    else
	grep -E "DB node id: 1 .*DEALLOCATE all" log/pgpool.log >/dev/null
	if [  $? -eq 0 ]; then
            success_count=$(( success_count + 1 ))
	    echo "ok."
	else
	    echo "failed."
	fi
    fi

    echo "In mode \"$mode\" out of $success_count, $num_tests cases succeeded."

    ./shutdownall

    if [ $success_count -ne $num_tests ]; then
	echo "Some tests failed. Exiting..."
       exit 1
    fi

    cd ..
done

exit 0
