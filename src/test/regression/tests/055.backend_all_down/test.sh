#!/usr/bin/env bash
#-------------------------------------------------------------------
# "pgpool -m f stop" hangs because unmanaged pgpool children remains. 
# Starting pgpool without starting PostgreSQL servers triggers the bug.
#
# Fixed in: http://git.postgresql.org/gitweb/?p=pgpool2.git;a=commit;h=fa6c3aa3233fd984a8b717f59e26b2ab9b7c8383
WHOAMI=`whoami`
source $TESTLIBS
TESTDIR=testdir

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment
echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 2 || exit 1
echo "done."

source ./bashrc.ports

export PGPORT=$PGPOOL_PORT

echo "LD_LIBRARY_PATH=/usr/local/pgsql/lib" > pgpool.sh
echo 'dir=`pwd`' >> pgpool.sh
grep pgpool.conf startall >> pgpool.sh

# start pgpool only. This causes immediate failover and may leave
# orphan pgpool children.
sh pgpool.sh

while true
do
	grep 'failover done. shutdown host /tmp(11003)' log/pgpool.log > /dev/null 2>&1
	if [ $? = 0 ];then
		break;
	fi
	sleep 1
done

# if that's the case, shutdownall will hang.
(./shutdownall)&
sleep 5
kill $!

if [ $? = 0 ];then
	./shutdownall
	exit 1
fi

./shutdownall
exit 0
