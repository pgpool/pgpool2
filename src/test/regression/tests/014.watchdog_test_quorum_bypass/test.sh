#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for watchdog
#
# Please note that to successfully run the test, "HEALTHCHECK_DEBUG"
# must be defined before compiling main/health_check.c.
#
# test pcp_detach bypass failover_when_quorum_exists and failover_require_consensus
#
source $TESTLIBS
MASTER_DIR=master
num_tests=2
success_count=0
PSQL=$PGBIN/psql
PG_CTL=$PGBIN/pg_ctl

rm -fr $MASTER_DIR
rm -fr $STANDBY_DIR
rm -fr $STANDBY2_DIR

mkdir $MASTER_DIR
mkdir $STANDBY_DIR
mkdir $STANDBY2_DIR


# dir in master directory
cd $MASTER_DIR

# create master environment
echo -n "creating master pgpool and PostgreSQL clusters..."
$PGPOOL_SETUP -m s -n 2 -p 11000|| exit 1
echo "master setup done."


source ./bashrc.ports
cat ../master.conf >> etc/pgpool.conf

./startall
wait_for_pgpool_startup


# back to test root dir
cd ..


# First test check if pgpool-II became a master.
echo "Waiting for the pgpool master..."
for i in 1 2 3 4 5 6 7 8 9 10
do
	grep "I am the cluster leader node" $MASTER_DIR/log/pgpool.log > /dev/null 2>&1
	if [ $? = 0 ];then
		success_count=$(( success_count + 1 ))
		echo "Master brought up successfully."
		break;
	fi
	echo "[check] $i times"
	sleep 2
done

#export PCPPASSFILE=/home/usama/work/community/pgpool2/src/test/regression/tests/067.bug231/testdir/pcppass
$PGPOOL_INSTALL_DIR/bin/pcp_detach_node -w -h localhost -p $PCP_PORT 1 2>&1

wait_for_pgpool_startup

$PSQL -p 11000 -c "show pool_nodes" test|grep standby|grep down >/dev/null 2>&1
if [ $? = 0 ];then
    echo "Failover was successfuly executed"
	success_count=$(( success_count + 1 ))
fi

cd master
./shutdownall

echo "$success_count out of $num_tests successfull";

if test $success_count -eq $num_tests
then
    exit 0
fi

exit 1
