#! /bin/sh
#-------------------------------------------------------------------
# test script for bug#61
# Child process xxxx was terminated by segmentation fault
#
# Fixed in: http://git.postgresql.org/gitweb/?p=pgpool2.git;a=commit;h=d493dd2b7d2065fb554654b137ecd587564f0043
source $TESTLIBS
TESTDIR=testdir

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment
echo -n "creating test environment..."
sh $PGPOOL_SETUP -m s -n 1 || exit 1
echo "done."

source ./bashrc.ports

export PGPORT=$PGPOOL_PORT

./startall
wait_for_pgpool_startup

pgbench -i test

EXPECT='psql: FATAL: no PostgreSQL user name specified in startup packet'
RESULT=`$PSQL -p $PGPOOL_PORT -U '' test 2>&1`

if [ $RESULT -ne '' $EXPECT ]; then
	./shutdownall
    exit 1
fi

./shutdownall
exit 0
