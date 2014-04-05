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
