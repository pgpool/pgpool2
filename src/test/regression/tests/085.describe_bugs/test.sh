#!/usr/bin/env bash
#-------------------------------------------------------------------
# Test script for the cases for Describe message handling.
# Currently the following case is tested.
# Discussion: https://github.com/pgpool/pgpool2/issues/169
# Feel free to add more cases for Describe message.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PG_CTL=$PGBIN/pg_ctl
PGPROTO=$PGPOOL_INSTALL_DIR/bin/pgproto
export PGDATABASE=test
expected=../expected.txt
results=results.txt

for mode in s i r n
do
    rm -fr $TESTDIR
    mkdir $TESTDIR
    cd $TESTDIR

    echo -n "creating test environment..."
    $PGPOOL_SETUP -m $mode -n 1 || exit 1
    echo "done."
    source ./bashrc.ports
    ./startall
    wait_for_pgpool_startup

    # test1: non existing prepared statement case
    $PGPROTO -d $PGDATABASE -p $PGPOOL_PORT -f ../pgproto.data > results.txt 2>&1

    sed -e 's/L [0-9]*/L xxx/g' $expected > expected_tmp
    sed -e 's/L [0-9]*/L xxx/g' $results > results_tmp
    diff expected_tmp results_tmp
    if [ $? != 0 ];then
	echo "test1 failed in mode: $mode".
	./shutdownall
	exit 1
    fi
    ./shutdownall
    cd ..
done

exit 0
