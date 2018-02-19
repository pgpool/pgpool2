#!/usr/bin/env bash

timeout=30
export PGPORT=11000
export PGDATABASE=test
export PGPOOL_INSTALL_DIR=$HOME/work/pgpool-II/current
#export PGPOOLDEBUG=true
PGPROTO=/usr/local/bin/pgproto

testdir=`pwd`/tests
expected=`pwd`/expected
extra_scripts=`pwd`/extra_scripts
export PGPOOLLOG=`pwd`/testdata/log/pgpool.log
results=`pwd`/results
rm -f $results/*
test ! -d $results && mkdir $results

diffs=`pwd`/diffs
rm -f $diffs

if [ $# -gt 0 ];then
    tests=`(cd tests;ls |grep $1)`
else
    tests=`(cd tests;ls)`
fi
rm -fr testdata
mkdir testdata
cd testdata
echo -n "creating test database..."
pgpool_setup > /dev/null 2>&1
echo "done."
cp etc/pgpool.conf pgpool.conf.back

okcnt=0
failcnt=0
timeoutcnt=0

for i in $tests
do
    echo -n "testing $i ... "

    # check if modification to pgpool.conf specified.
    d=/tmp/diff$$
    grep '^##' $testdir/$i > $d
    if [ -s $d ]
    then
	sed -e 's/^##//' $d >> etc/pgpool.conf
    fi
    rm -f $d

    ./startall >/dev/null 2>&1

    while :
    do
	psql -c "select 1" test >/dev/null 2>&1
	if [ $? = 0 ]
	then
	    break
	fi
	sleep 1
    done

    timeout $timeout $PGPROTO -f $testdir/$i > $results/$i 2>&1
    if [ $? = 124 ]
    then
	echo "timeout."
	timeoutcnt=`expr $timeoutcnt + 1`
    else
	sed -e 's/L [0-9]* R/L xxx R/' $expected/$i > expected_tmp
	sed -e 's/L [0-9]* R/L xxx R/' $results/$i > results_tmp
	cmp expected_tmp results_tmp >/dev/null 2>&1
	if [ $? != 0 ]
	then
	    echo "failed."
	    echo "=== $i ===" >> $diffs
	    diff -N $expected/$i $results/$i >> $diffs
	    failcnt=`expr $failcnt + 1`
	else
	    extra_fail=0
	    # excute extra scripts if exists.
	    if [ -x $extra_scripts/$i ]
	    then
		$extra_scripts/$i > $results/$i.extra 2>&1

		if [ $? != 0 ]
		then
		    echo "extra test failed."
		    extra_fail=1
		    failcnt=`expr $failcnt + 1`
		fi
	    fi

	    if [ $extra_fail = 0 ]
	    then
		echo "ok."
		okcnt=`expr $okcnt + 1`
	    fi
	fi
	rm expected_tmp results_tmp
    fi
    grep pool_check_pending_message_and_reply log/pgpool.log
    ./shutdownall >/dev/null 2>&1
    cp pgpool.conf.back etc/pgpool.conf 
    process=`ps x|grep pgpool|grep idle`
    if [ ! -z $process ]
    then
	echo "Some process remains. Aborting tests"
	exit 1
    fi

done

total=`expr $okcnt + $failcnt + $timeoutcnt`
echo "out of $total ok: $okcnt failed: $failcnt timeout: $timeoutcnt."
