#! /usr/bin/bash
# read users list from stdin and perform ldapdelete command.
PASSWORD=ldapadmin
set -e
while read i
do
	ldapdelete -x -w $PASSWORD -D cn=admin,dc=nodomain uid=$i,ou=people,dc=nodomain
done
