#! /bin/sh
# clean up test results
dir=`pwd`
export TESTLIBS=$dir/libs.sh
export PGPOOL_SETUP=$HOME/bin/pgpool_setup
log=$dir/log

rm -fr $log
rm -fr $dir/temp

cd tests
dirs=`ls`
for i in $dirs
do
	cd $i
	rm -fr testdir *~
	cd ..
done

rm -fr $dir/tests/004.watchdog/master
rm -fr $dir/tests/004.watchdog/standby
