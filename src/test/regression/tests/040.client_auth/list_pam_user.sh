#! /usr/bin/bash
# List up user names used in the PAM authentication test.
spec=client_auth_2node.csv
IFS=","
cat $spec|while read line
do
    set $line
    if [ $1 != 'username' ];then
	username=$1
	pool_hba=$2
	pg_hba=$5
	if [ $pool_hba = "pam" -o $pg_hba = "pam" ];then
	    echo $username
	fi
    fi
done
