#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for watchdog
source $TESTLIBS
TESTDIR=master

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create master environment
echo -n "creating master pgpool..."
$PGPOOL_SETUP -m n -n 1 -p 11000|| exit 1
echo "done."

success_count=0
# copy the same configurations to standby
sdir=standby
rm -fr $sdir
mkdir $sdir
cp -r etc ../$sdir/

source ./bashrc.ports
cat ../master.conf >> etc/pgpool.conf

./startall
wait_for_pgpool_startup

cd ..

# create standby environment
cd $sdir
echo -n "creating standby pgpool..."
source ./bashrc.ports
cat ../standby.conf >> etc/pgpool.conf
# since we are using the same pgpool-II conf as of master. so change the pid file path in standby pgpool conf
echo "pid_file_name = '$PWD/pgpool2.pid'" >> etc/pgpool.conf
# start the stnadby pgpool-II by hand
$PGPOOL_INSTALL_DIR/bin/pgpool -D -n -f etc/pgpool.conf -F etc/pcp.conf -a etc/pool_hba.conf > log/pgpool.log 2>&1 &
wait_for_pgpool_startup
cd ..

# First test check if both pgpool-II have found their correct place in watchdog cluster.
echo "Waiting for the pgpool master..."
for i in 1 2 3 4 5 6 7 8 9 10
do
	RESULT=`grep "I am the cluster leader node. Starting escalation process" master/log/pgpool.log`
	if [ ! -z "$RESULT" ]; then
		success_count=$(( success_count + 1 ))
		echo "Master brought up successfully."
		break;
	fi
	echo "[check] $i times"
	sleep 2
done

# now check if standby has successfully joined connected to the master.
echo "Waiting for the pgpool master..."
for i in 1 2 3 4 5 6 7 8 9 10
do
	RESULT=`grep "successfully joined the watchdog cluster as standby node" standby/log/pgpool.log`
	if [ ! -z "$RESULT" ]; then
		success_count=$(( success_count + 1 ))
		echo "Standby successfully connected."
		break;
	fi
	echo "[check] $i times"
	sleep 2
done

# step 2 stop master pgpool and see if standby take over
$PGPOOL_INSTALL_DIR/bin/pgpool -f master/etc/pgpool.conf -m f stop

echo "Checking if the Standby pgpool-II detected the master shutdown..."
for i in 1 2 3 4 5 6 7 8 9 10
do
	RESULT=`grep " is shutting down" standby/log/pgpool.log`
	if [ ! -z "$RESULT" ]; then
		success_count=$(( success_count + 1 ))
		echo "Master shutdown detected."
		break;
	fi
	echo "[check] $i times"
	sleep 2
done

# Finally see if standby take over
$PGPOOL_INSTALL_DIR/bin/pgpool -f master/etc/pgpool.conf -m f stop

echo "Checking if the Standby pgpool-II takes over the master responsibility..."
for i in 1 2 3 4 5 6 7 8 9 10
do
	RESULT=`grep "I am the cluster leader node. Starting escalation process" standby/log/pgpool.log`
	if [ ! -z "$RESULT" ]; then
		success_count=$(( success_count + 1 ))
		echo "Standby successfully became the new master."
		break;
	fi
	echo "[check] $i times"
	sleep 2
done

$PGPOOL_INSTALL_DIR/bin/pgpool -f standby/etc/pgpool.conf -m f stop
cd master
./shutdownall

echo "$success_count out of 4 successfull";

if test $success_count -eq 4
then
    exit 0
fi

exit 1
