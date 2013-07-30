#! /bin/sh
#-------------------------------------------------------------------
# test script for watchdog
source $TESTLIBS
TESTDIR=master

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create master environment
echo -n "creating master pgpool..."
sh $PGPOOL_SETUP -m s -n 2 -p 11000|| exit 1
echo "done."
cat ../master.conf >> etc/pgpool.conf

./startall
wait_for_pgpool_startup

cd ..

# create standby environment
sdir=standby
rm -fr $sdir
mkdir $sdir
cd $sdir
echo -n "creating standby pgpool..."
sh $PGPOOL_SETUP -m s -n 2 -p 11100|| exit 1
echo "done."
cat ../standby.conf >> etc/pgpool.conf
egrep 'backend_data_directory0|backend_data_directory1|failover_command|follow_master_command' ../$TESTDIR/etc/pgpool.conf >> etc/pgpool.conf
./startall
wait_for_pgpool_startup
cd ..

# stop master pgpool and see if standby take over
$PGPOOL_INSTALL_DIR/bin/pgpool -f master/etc/pgpool.conf -m f stop

sleep 1

RESULT=`grep "wd_escalation: escalated to master pgpool successfully" standby/log/pgpool.log`

cd master
./shutdownall
cd ../standby
./shutdownall

if [ -z "$RESULT" ]; then
    exit 1
fi

exit 0
