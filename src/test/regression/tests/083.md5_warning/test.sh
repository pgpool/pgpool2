#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for the message:
# WARNING:  authenticated with an MD5-encrypted password
# DETAIL:  MD5 password support is deprecated and will be removed in a future release of PostgreSQL.
# (PostgreSQL 19+)

# check PostgreSQL version
if [ "$PGVERSION" -lt 19 ];then
    echo "PostgreSQL $PGVERSION is not supported by the test".
    exit 0
fi

source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PG_CTL=$PGBIN/pg_ctl
export PGDATABASE=test
PG_MD5=$PGPOOL_INSTALL_DIR/bin/pg_md5
PGPOOL_CONF=etc/pgpool.conf

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# specify max protocol version to 3.0 to avoid protocol negotiation
# message which is only supported by Pgpool-II 4.7+
export PGMAXPROTOCOLVERSION=3.0

# create test environment. Number of backend node 1 is enough.
echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 1 || exit 1
echo "done."

dir=`pwd`
echo "enable_pool_hba = on" >> $PGPOOL_CONF

./startall

source ./bashrc.ports
export PGPORT=$PGPOOL_PORT
wait_for_pgpool_startup
echo PGPORT: $PGPORT
echo create MD5 password for "md5_user"
$PSQL -p 11002 test <<EOF
CREATE ROLE md5_user WITH LOGIN;
SET password_encryption = md5;
ALTER USER md5_user WITH ENCRYPTED PASSWORD 'md5_password';
EOF

$PG_MD5 -m -f $PGPOOL_CONF -u md5_user 'md5_password'

./pgpool_reload
sleep 1

echo "connection test with md5 user"
expected=$'WARNING:  authenticated with an MD5-encrypted password\nDETAIL:  MD5 password support is deprecated and will be removed in a future release of PostgreSQL.'

result=$(PGPASSWORD="md5_password" $PSQL -U md5_user -c "SELECT 1" test 2>&1 > /dev/null)
if [ $? != 0 ];then
    echo "psql failed"
    ./shutdownall
    exit 1
fi
./shutdownall
if [ "$expected" != "$result" ];then
    echo "result is not expected one: \"$result\""
    exit 1
fi
exit 0
