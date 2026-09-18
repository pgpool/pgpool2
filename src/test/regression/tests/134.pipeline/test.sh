#!/usr/bin/env bash
#-------------------------------------------------------------------
# Test for a hang when a deferred constraint error is reported while
# processing Sync in extended-query pipeline mode.
#
source $TESTLIBS

TESTDIR=testdir
PGPROTO=$PGPOOL_INSTALL_DIR/bin/pgproto

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 1 || exit 1
echo "done."

source ./bashrc.ports

./startall
wait_for_pgpool_startup

timeout 1 $PGPROTO \
    -p $PGPOOL_PORT \
    -d test \
    -f ../pgproto.data \
    > result 2>&1

status=$?

if [ $status != 0 ]; then
    if [ $status = 124 ]; then
        echo "test failed: pgproto timed out"
    else
        echo "test failed: pgproto exited with status $status"
    fi

    cat result
    ./shutdownall
    exit 1
fi

#
# The deferred primary-key violation must be reported while processing
# Sync, and processing must continue through ReadyForQuery.
#
grep 'ErrorResponse(S ERROR V ERROR C 23505 ' result > /dev/null
if [ $? != 0 ]; then
    echo "test failed: expected deferred constraint error was not reported"
    cat result
    ./shutdownall
    exit 1
fi

grep 'ReadyForQuery(I)' result > /dev/null
if [ $? != 0 ]; then
    echo "test failed: ReadyForQuery was not received after the error"
    cat result
    ./shutdownall
    exit 1
fi

./shutdownall
exit 0