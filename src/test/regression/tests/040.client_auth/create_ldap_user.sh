#! /usr/bin/bash
# Create input file for ldapadd command to stdout.
# Users' list must be provided from stdin.
if [ $# -ne 0 ];then
    echo "usage: $0"
    exit 1
fi

while read username
do
    cat <<EOF
dn: uid=$username,ou=people,dc=nodomain
cn: $username
sn: $username
objectClass: inetOrgPerson
objectClass: simpleSecurityObject
userPassword: $username

EOF
done
