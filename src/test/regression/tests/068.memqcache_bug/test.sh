#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for a memqcache bug reported in pgpool-general-jp:1441.
# (do_query() hangs in certain condition)
# requires Java PostgreSQL JDBC driver.

source $TESTLIBS
TESTDIR=testdir
export CLASSPATH=.:/usr/local/pgsql/share/postgresql-9.2-1003.jdbc4.jar

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment
echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 2 || exit 1
echo "done."

source ./bashrc.ports

export PGPORT=$PGPOOL_PORT

echo "memory_cache_enabled = on" >> etc/pgpool.conf
sh startall
wait_for_pgpool_startup

cd ..
psql test <<EOF
DROP TABLE IF EXISTS t1;
CREATE TABLE t1(i int);
EOF
javac Sample.java
java Sample	# hang here if the bug bites you...
cd $TESTDIR
sh shutdownall
exit 0

