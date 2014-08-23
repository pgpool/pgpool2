#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for database_redirect_preference_list and app_name_redirect_preference_list.
# requires pgbench.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PGBENCH=$PGBENCH_PATH

function getnode()
{
	grep $1 log/pgpool.log | grep SELECT | awk '{print $9}'
}

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment
echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 3 || exit 1
echo "done."

source ./bashrc.ports

echo "database_redirect_preference_list = 'postgres:primary,test:1,mydb[5-9]:2,test2:standby'" >> etc/pgpool.conf

./startall

export PGPORT=$PGPOOL_PORT
wait_for_pgpool_startup

createdb mydb6
createdb test2
$PGBENCH -i postgres

ok=yes

# should be redirect to primary (node 0)
$PSQL -c "SELECT 'test1'" postgres

test `getnode "test1"` -eq 0 || ok=ng

# should be redirect to node 1
$PSQL -c "SELECT 'test2'" test

test `getnode "test2"` -eq 1 || ok=ng

# should be redirect to node 2
$PSQL -c "SELECT 'test3'" mydb6

test `getnode "test3"` -eq 2 || ok=ng

# should be redirect to either node 1 or 2
$PSQL -c "SELECT 'test4'" test2

test `getnode "test4"` -eq 1 -o `getnode "test4"` -eq 2 || ok=ng

echo "app_name_redirect_preference_list = 'psql:primary,pgbench:standby'" >> etc/pgpool.conf

./pgpool_reload

wait_for_pgpool_startup

# should be redirect to node 0 because application name is psql
$PSQL -c "SELECT 'test5'" mydb6

test `getnode "test5"` -eq 0 || ok=ng

# should be redirect to either node 1 or 2
$PGBENCH -t 1 -f ../select.pgbench postgres

test `getnode "test6"` -eq 1 -o `getnode "test6"` -eq 2 || ok=ng

./shutdownall

if [ $ok = "yes" ];then
	exit 0
fi
exit 1
