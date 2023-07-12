#!/usr/bin/env bash
#-------------------------------------------------------------------
# Test script for multi statement query including PREPARE *and* bind is used later on.
# Discussion: [pgpool-general: 8870] Prepared statements over pgpool ?
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PG_CTL=$PGBIN/pg_ctl
PGPROTO=$PGPOOL_INSTALL_DIR/bin/pgproto
export PGDATABASE=test

#for mode in s
for mode in s i r n
do
    rm -fr $TESTDIR
    mkdir $TESTDIR
    cd $TESTDIR

    echo -n "creating test environment..."
    $PGPOOL_SETUP -m $mode || exit 1
    echo "done."
    source ./bashrc.ports
    ./startall
    wait_for_pgpool_startup

    $PGPROTO -d $PGDATABASE -p $PGPOOL_PORT -f ../pgproto.data > results.txt 2>&1
    cmp ../expected.txt results.txt

    if [ $? != 0 ];then
	echo "test failed in mode: $mode".
	./shutdownall
	exit 1
    fi
    ./shutdownall
    cd ..
done

exit 0
