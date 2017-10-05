#!/usr/bin/env bash

timeout=300
export PGPORT=11000
export PGDATABASE=test
export PGPOOL_INSTALL_DIR=$HOME/work/pgpool-II/current
#export PGPOOLDEBUG=true
PGPROTO=/usr/local/bin/pgproto

testdir=`pwd`/tests
expected=`pwd`/expected
results=`pwd`/results
rm -f $results/*
mkdir $results

diffs=`pwd`/diffs
rm -f $diffs

tests=`(cd tests;ls)`
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
	timeoutcnt=`expr $timeout + 1`
    else
	cmp $expected/$i $results/$i >/dev/null 2>&1
	if [ $? != 0 ]
	then
	    echo "failed."
	    echo "=== $i ===" >> $diffs
	    diff -N $expected/$i $results/$i >> $diffs
	    failcnt=`expr $failcnt + 1`
	else
	    echo "ok."
	    okcnt=`expr $okcnt + 1`
	fi
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
