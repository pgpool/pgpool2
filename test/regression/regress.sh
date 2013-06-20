#
# pgpool-II regression test driver.
#
# usage: regress.sh [test_name]
dir=`pwd`
export TESTLIBS=$dir/libs.sh
export PGPOOL_SETUP=$HOME/bin/pgpool_setup
export PSQL=psql
log=$dir/log
fail=0
ok=0

rm -fr $log
mkdir $log

cd tests

if [ $# -eq 1 ];then
	dirs=`ls|grep $1`
else
	dirs=`ls`
fi

for i in $dirs
do
	cd $i
	echo -n "testing $i..."
	./test.sh > $log/$i 2>&1
	if [ $? = 0 ];then
		echo "ok."
		ok=`expr $ok + 1`
	else
		echo "failed."
		fail=`expr $fail + 1`
	fi

	cd ..

done

total=`expr $ok + $fail`

echo "out of $total ok:$ok failed:$fail"
