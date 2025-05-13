#! /usr/bin/bash
# Create input file (newusers_input.txt) for newusers command.
# Users' list must be provided as $1.
if [ $# -ne 1 ];then
    echo "usage: $0 pam_users_list_file"
    exit 1
fi

USERS=newusers_input.txt

spec=$1

cp /dev/null $USERS
cat $spec|while read username
do
    echo "$username:$username::::/nonexistent:/usr/sbin/nologin" >> $USERS
done
