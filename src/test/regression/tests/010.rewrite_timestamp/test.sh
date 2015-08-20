#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for unit test of rewriting timestamp qeries.
# requires Ruby

cd timestamp
make 
make test > result.txt 
cmp ../expected.txt result.txt
if [ $? != 0 ];then
	echo NG
	exit 1
fi
make clean
rm result.txt
cd ..

echo OK 
exit 0
