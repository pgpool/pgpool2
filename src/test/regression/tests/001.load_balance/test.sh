#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for load balancing.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql

# sleep time after reload in seconds
st=10

for mode in s r
do
	rm -fr $TESTDIR
	mkdir $TESTDIR
	cd $TESTDIR

# create test environment
	echo -n "creating test environment..."
	$PGPOOL_SETUP -m $mode -n 2 || exit 1
	echo "done."

	source ./bashrc.ports

	echo "backend_weight0 = 0" >> etc/pgpool.conf
	echo "backend_weight1 = 1" >> etc/pgpool.conf
	echo "black_function_list = 'f1'" >> etc/pgpool.conf

	./startall

	export PGPORT=$PGPOOL_PORT

	wait_for_pgpool_startup

	$PSQL test <<EOF
CREATE TABLE t1(i INTEGER);
CREATE FUNCTION f1(INTEGER) returns INTEGER AS 'SELECT \$1' LANGUAGE SQL;
SELECT * FROM t1;		-- this load balances
SELECT f1(1);		-- this does not load balance
EOF

# check if simple load balance worked
	fgrep "SELECT * FROM t1;" log/pgpool.log |grep "DB node id: 1">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		echo fail: select is sent to zero-weight node.
		./shutdownall
		exit 1
	fi
	echo ok: load balance works.

# check if black function list worked
	fgrep "SELECT f1(1);" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		echo fail: black function is sent to node 1.
		./shutdownall
		exit 1
	fi
	echo ok: black function list works.

	echo "white_function_list = 'f1'" >> etc/pgpool.conf
	echo "black_function_list = ''" >> etc/pgpool.conf

	./pgpool_reload
	sleep $t

	$PSQL test <<EOF
SELECT f1(1);		-- this does load balance
EOF

# check if white function list worked
	fgrep "SELECT f1(1);" log/pgpool.log |grep "DB node id: 1">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		echo fail: white function is sent to zero-weight node.
		./shutdownall
		exit 1
	fi
	echo ok: white function list works.

# in replication mode if load_balance_mode = off, SELECT query inside
# an explicit transaction should be sent to master only.
	if [ $mode = "r" ];then
		./shutdownall
		echo "load_balance_mode = off" >> etc/pgpool.conf
		./startall
		wait_for_pgpool_startup

		$PSQL test <<EOF
BEGIN;
SELECT 1;
END;
EOF

		ok=0
		fgrep "SELECT 1;" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
		if [ $? = 0 ];then
			fgrep "SELECT 1;" log/pgpool.log |grep "DB node id: 1">/dev/null 2>&1		
			if [ $? != 0 ];then
			# the SELECT should not be executed on node 1
				echo ok: select is sent to only master when not load-blanced.
				ok=1
			fi
		# the SELECT should be executed on node 0
		fi

# in replication mode if load_balance_mode = off, SELECT query
# including writing function should be sent to all the nodes.
# per [pgpool-general: 2221].
		echo "black_function_list = 'f1'" >> etc/pgpool.conf
		echo "white_function_list = ''" >> etc/pgpool.conf
		./pgpool_reload
		sleep $st
		$PSQL test <<EOF
SELECT f1(2);		-- this should be sent to all the nodes
EOF
		fgrep "SELECT f1(2);" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
		if [ $? = 0 ];then
			fgrep "SELECT f1(2);" log/pgpool.log |grep "DB node id: 1">/dev/null 2>&1		
			if [ $? = 0 ];then
			# the SELECT should be executed on node 0 & 1
				echo ok: black function is sent to all nodes.
				ok=`expr $ok + 1`
			fi
		# the SELECT should be executed on node 0
		fi

		if [ $ok != 2 ];then
		    exit 1;
		fi
	fi

	./shutdownall

	cd ..

done

exit 0
