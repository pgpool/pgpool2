#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for user_redirect_preference_list, database_redirect_preference_list and app_name_redirect_preference_list.
# requires pgbench.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
CREATEDB=$PGBIN/createdb
CREATEUSER=$PGBIN/createuser
PGBENCH=$PGBENCH_PATH

function getnode()
{
	grep $1 log/pgpool.log | grep SELECT | grep LOG | awk '{print $10}'
}

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment
echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 3 || exit 1
echo "done."

source ./bashrc.ports

# disable delay_threshold so that replication delay does not affect the tests.
echo "delay_threshold = 0" >> etc/pgpool.conf

ok=yes
#-------------------------------------------------------------------
# Test user_redirect_preference_list
#-------------------------------------------------------------------

echo "user_redirect_preference_list = 'user1:primary,user2:1,user3:standby,user4:primary(1.0),user5:standby(0.0),user[6-9]:2'" >> etc/pgpool.conf

./startall

export PGPORT=$PGPOOL_PORT
wait_for_pgpool_startup

$PSQL -c "show pool_nodes" postgres

$CREATEUSER user1
$CREATEUSER user2
$CREATEUSER user3
$CREATEUSER user4
$CREATEUSER user5
$CREATEUSER user6

# check to see if all databases have been replicated
for p in 3 4
do
    # set standby port
    myport=`expr $PGPOOL_PORT + $p`

	for r in 1 2 3 4 5
	do
		is_replicated=true
		for i in user1 user2 user3 user4 user5 user6
		do
			echo "try to connect to PostgreSQL using user $i:$myport"
			$PSQL -p $myport -U $i -c "select 1" test
			if [ $? != 0 ];then
				is_replicated=false
				break
			fi
		done
		if [ $is_replicated = "false" ];then
			sleep 1
		else
			break
		fi
	done
done

# Test1: should be redirect to primary (node 0)
$PSQL -U user1 -c "SELECT 'test1'" test
test `getnode "'test1'"` -eq 0 || ok=ng
echo $ok

# Test2: should be redirect to node 1
$PSQL -U user2 -c "SELECT 'test2'" test
test `getnode "'test2'"` -eq 1 || ok=ng
echo $ok

# Test3: should be redirect to either node 1 or 2 (standby)
$PSQL -U user3 -c "SELECT 'test3'" test
test `getnode "'test3'"` -eq 1 -o `getnode "test3"` -eq 2 || ok=ng
echo $ok

# Test4: should be redirect to primary (node 0)
$PSQL -U user4 -c "SELECT 'test4'" test
test `getnode "'test4'"` -eq 0 || ok=ng
echo $ok

# Test5: should be redirect to primary (node 0)
$PSQL -U user5 -c "SELECT 'test5'" test
test `getnode "'test5'"` -eq 0 || ok=ng
echo $ok

# Test6: should be redirect to node 2
$PSQL -U user6 -c "SELECT 'test6'" test
test `getnode "'test6'"` -eq 2 || ok=ng
echo $ok

#-------------------------------------------------------------------
# Test database_redirect_preference_list
#-------------------------------------------------------------------

echo "database_redirect_preference_list = 'postgres:primary,test:1,mydb[5-9]:2,test2:standby,test3:primary(0.0),test4:standby(0.0),test5:primary(1.0)'" >> etc/pgpool.conf

./pgpool_reload
sleep 10

wait_for_pgpool_startup

$CREATEDB mydb6
$CREATEDB test2
$CREATEDB test3
$CREATEDB test4
$CREATEDB test5

# check to see if all databases have been replicated
for p in 3 4
do
    # set standby port
    myport=`expr $PGPOOL_PORT + $p`

	for r in 1 2 3 4 5
	do

		is_replicated=true
		for i in mydb6 test2 test3 test4 test5
		do
			echo "try to connect to $i:$myport"
			$PSQL -p $myport -c "select 1" $i
			if [ $? != 0 ];then
				is_replicated=false
				break
			fi
		done
		if [ $is_replicated = "false" ];then
			sleep 1
		else
			break
		fi
	done
done

# Test7: should be redirect to primary (node 0)
$PSQL -c "SELECT 'test7'" postgres
test `getnode "'test7'"` -eq 0 || ok=ng
echo $ok

# Test8: should be redirect to node 1
$PSQL -c "SELECT 'test8'" test
test `getnode "'test8'"` -eq 1 || ok=ng
echo $ok

# Test9: should be redirect to node 2
$PSQL -c "SELECT 'test9'" mydb6
test `getnode "'test9'"` -eq 2 || ok=ng
echo $ok

# Test10: should be redirect to either node 1 or 2
$PSQL -c "SELECT 'test10'" test2
test `getnode "'test10'"` -eq 1 -o `getnode "test10"` -eq 2 || ok=ng
echo $ok

# Test11: should be redirect to either node 1 or 2
$PSQL -c "SELECT 'test11'" test3
test `getnode "'test11'"` -eq 1 -o `getnode "test11"` -eq 2 || ok=ng
echo $ok

# Test12: should be redirect to primary (node 0)
$PSQL -c "SELECT 'test12'" test4
test `getnode "'test12'"` -eq 0 || ok=ng
echo $ok

# Test13: should be redirect to primary (node 0)
$PSQL -c "SELECT 'test13'" test5
test `getnode "'test13'"` -eq 0 || ok=ng
echo $ok

# Test14: For example:
# if it matches both of user_redirect_preference_list and database_redirect_preference_list,
# should be redirect to node 1 because database name is test
# user_redirect_preference_list = 'user4:primary(1.0)'
# database_redirect_preference_list = 'test:1'
$PSQL -U user4 -c "SELECT 'test14'" test
test `getnode "'test14'"` -eq 1 || ok=ng

#-------------------------------------------------------------------
# Test app_name_redirect_preference_list
#-------------------------------------------------------------------
$PGBENCH -i postgres

echo "app_name_redirect_preference_list = 'psql:primary,pgbench:standby'" >> etc/pgpool.conf

./pgpool_reload
sleep 10

wait_for_pgpool_startup

# Test15: should be redirect to node 0 because application name is psql
$PSQL -c "SELECT 'test15'" mydb6
test `getnode "'test15'"` -eq 0 || ok=ng
echo $ok

# Test16: should be redirect to either node 1 or 2
$PGBENCH -t 1 -f ../select.pgbench postgres
test `getnode "'test16'"` -eq 1 -o `getnode "test16"` -eq 2 || ok=ng
echo $ok


echo "app_name_redirect_preference_list = 'psql:primary(0.0),pgbench:standby(1.0)'" >> etc/pgpool.conf

./pgpool_reload
sleep 10

wait_for_pgpool_startup

# Test17: should be redirect to either node 1 or 2
$PSQL -c "SELECT 'test17'" mydb6
test `getnode "'test17'"` -eq 1 -o `getnode "test17"` -eq 2 || ok=ng
echo $ok

# Test18: should be redirect to either node 1 or 2
$PGBENCH -t 1 -f ../select1.pgbench postgres
test `getnode "'test18'"` -eq 1 -o `getnode "test18"` -eq 2 || ok=ng
echo $ok

./shutdownall

if [ $ok = "yes" ];then
	exit 0
fi
exit 1
