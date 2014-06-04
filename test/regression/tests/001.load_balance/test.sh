#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for load balancing.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql

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

# check if simle load balance worked
	fgrep "SELECT * FROM t1;" log/pgpool.log |grep "DB node id: 1">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		./shutdownall
		exit 1
	fi

# check if black function list worked
	fgrep "SELECT f1(1);" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		./shutdownall
		exit 1
	fi

	echo "white_function_list = 'f1'" >> etc/pgpool.conf
	echo "black_function_list = ''" >> etc/pgpool.conf

	./pgpool_reload

	$PSQL test <<EOF
SELECT f1(1);		-- this does load balance
EOF

# check if white function list worked
	fgrep "SELECT f1(1);" log/pgpool.log |grep "DB node id: 1">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		./shutdownall
		exit 1
	fi

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
		$PSQL test <<EOF
SELECT f1(2);		-- this should be sent to all the nodes
EOF
		fgrep "SELECT f1(2);" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
		if [ $? = 0 ];then
			fgrep "SELECT f1(2);" log/pgpool.log |grep "DB node id: 1">/dev/null 2>&1		
			if [ $? = 0 ];then
			# the SELECT should be executed on node 0 & 1
				ok=`expr $ok + 1`
			fi
		# the SELECT should be executed on node 0
		fi

	fi

	./shutdownall

	if [ $ok != 2 ];then
		exit 1;
	fi

	cd ..

done

exit 0
