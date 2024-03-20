#!/usr/bin/env bash
#-------------------------------------------------------------------
# Test script for pgpool hang due to protocol violation.
# If frontend sends simple query before extended protocol ended, pgpool hangs.
# 
# Discussion:
# [pgpool-general: 8990] autosave=always jdbc option & it only sends query SAVEPOINT PGJDBC_AUTOSAVE and hangs
# [pgpool-hackers: 4427] Guard against ill mannered frontend
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PG_CTL=$PGBIN/pg_ctl
PGPROTO=$PGPOOL_INSTALL_DIR/bin/pgproto
export PGDATABASE=test

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

echo -n "creating test environment..."
$PGPOOL_SETUP || exit 1
echo "done."
echo "backend_weight1=0" >> etc/pgpool.conf
source ./bashrc.ports
./startall
wait_for_pgpool_startup

# test1:
# Wait for 1 seconds before pgproto ended.
# Usually 1 seconds should be enough to finish pgproto.
# If test suceeded, pgpool emits an error message:
# "FATAL:  simple query "SAVEPOINT PGJDBC_AUTOSAVE" arrived before ending an extended query message"
# grep command below should catch the message.
timeout 1 $PGPROTO -d $PGDATABASE -p $PGPOOL_PORT -f ../pgproto.data |& grep 'simple query "SAVEPOINT PGJDBC_AUTOSAVE" arrived '

if [ $? != 0 ];then
# timeout happened or pgproto returned non 0 status
    echo "test1 failed."
    ./shutdownall
    exit 1
fi

# test2:
# Check if reset queries can be executed even if extended query messages
# do not end.
timeout 1 $PGPROTO -d $PGDATABASE -p $PGPOOL_PORT -f ../pgproto2.data
if [ $? != 0 ];then
# timeout happened or pgproto returned non 0 status
    echo "test2 failed."
    ./shutdownall
    exit 1
fi

./shutdownall
exit 0
