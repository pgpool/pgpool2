#!/usr/bin/env bash

export PGPORT=11000
export PGDATABASE=test
export PGPOOL_INSTALL_DIR=$HOME/work/pgpool-II/current
#export PGPOOLDEBUG=true
PGPROTO=/usr/local/bin/pgproto

testdir=`pwd`/tests
expected=`pwd`/expected
results=`pwd`/results
rm -f $results/*

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

for i in $tests
do
    echo "====== $i ======="

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

    $PGPROTO -f $testdir/$i > $results/$i 2>&1
    cmp $expected/$i $results/$i >/dev/null 2>&1
    if [ $? != 0 ]
    then
	echo "$i differ"
	echo "=== $i ===" >> $diffs
	diff -N $expected/$i $results/$i >> $diffs
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
