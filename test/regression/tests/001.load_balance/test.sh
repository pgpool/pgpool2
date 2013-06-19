#! /bin/sh
#-------------------------------------------------------------------
# test script for load balancing.
#
source $TESTLIBS
TESTDIR=testdir

for mode in s r
do
	rm -fr $TESTDIR
	mkdir $TESTDIR
	cd $TESTDIR

# create test environment
	echo -n "creating test environment..."
	sh $PGPOOL_SETUP -m $mode -n 2 || exit 1
	echo "done."

	source ./bashrc.ports

	echo "backend_weight0 = 0" >> etc/pgpool.conf
	echo "backend_weight1 = 1" >> etc/pgpool.conf
	echo "black_function_list = 'f1'" >> etc/pgpool.conf

	./startall

	export PGPORT=$PGPOOL_PORT

	wait_for_pgpool_startup

	psql test <<EOF
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

	psql test <<EOF
SELECT f1(1);		-- this does load balance
EOF

# check if white function list worked
	fgrep "SELECT f1(1);" log/pgpool.log |grep "DB node id: 1">/dev/null 2>&1
	if [ $? != 0 ];then
	# expected result not found
		./shutdownall
		exit 1
	fi

	./shutdownall

	cd ..

done

exit 0
