#! /usr/bin/bash
# read users list from stdin and perform userdel command.
set -e
while read i
do
    /usr/sbin/userdel $i
done
