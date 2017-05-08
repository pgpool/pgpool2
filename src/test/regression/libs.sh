#-------------------------------------------
# wait for pgpool comes up
#-------------------------------------------
function wait_for_pgpool_startup {
	timeout=20

	while [ $timeout -gt  0 ]
	do
		$PGBIN/psql -p $PGPOOL_PORT -c "show pool_nodes" test >/dev/null 2>&1
		if [ $? = 0 ];then
			break;
		fi
		timeout=`expr $timeout - 1`
		sleep 1
	done
}

#-------------------------------------------
# wait for primary/master failover done
#-------------------------------------------
function wait_for_failover_done {
	timeout=20

	while [ $timeout -gt  0 ]
	do
		$PGBIN/psql -p $PGPOOL_PORT -c "show pool_nodes" test >/dev/null 2>&1
		if [ $? = 0 ];then
		    $PGBIN/psql -p $PGPOOL_PORT -c "show pool_nodes" test |egrep -i "primary|master">/dev/null 2>&1
		    if [ $? = 0 ];then
			break;
		    fi
		fi
		timeout=`expr $timeout - 1`
		echo "timeout: $timeout"
		sleep 1
	done
}
