#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for SSL connection for: frontend <--> Pgpool-II and Pgpool-II and PostgreSQL.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PG_CTL=$PGBIN/pg_ctl
export PGDATABASE=test
SSL_KEY=server.key
SSL_CRT=server.crt

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment. Number of backend node is 1 is enough.
echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 1 || exit 1
echo "done."

# setup SSL key and crt file
cp -p ../$SSL_KEY etc/
chmod og-rwx etc/$SSL_KEY
cp -p ../$SSL_CRT etc/
cp -p ../$SSL_KEY data0/
chmod og-rwx data0/$SSL_KEY
cp -p ../$SSL_CRT data0/

# enable SSL support
dir=`pwd`

echo "ssl = on" >> etc/pgpool.conf
echo "ssl_key = '$dir/etc/$SSL_KEY'" >> etc/pgpool.conf
echo "ssl_cert = '$dir/etc/$SSL_CRT'" >> etc/pgpool.conf

echo "ssl = on" >> data0/postgresql.conf
echo "ssl_cert_file = '$SSL_CRT'" >> data0/postgresql.conf
echo "ssl_key_file = '$SSL_KEY'" >> data0/postgresql.conf

# backend must be connected via TCP/IP
echo "backend_hostname0 = 'localhost'" >> etc/pgpool.conf

# produce debug message since the only way to confirm the SSL
# connectins is being established is, look into the debug log.
echo "log_min_messages = debug5" >> etc/pgpool.conf

source ./bashrc.ports

./startall

export PGPORT=$PGPOOL_PORT

wait_for_pgpool_startup

# first, checking frontend<-->Pgpool-II...

$PSQL -h localhost test <<EOF | grep SSL
\conninfo
\q
EOF

if [ $? != 0 ];then
    echo "Checking SSL connection between frontend and Pgpool-II failed."
    ./shutdownall
    exit 1
fi

echo "Checking SSL connection between frontend and Pgpool-II was ok."

grep "client->server SSL response: S" log/pgpool.log >/dev/null
if [ $? != 0 ];then
    echo "Checking SSL connection between Pgpool-II and backend failed."
    ./shutdownall
    exit 1
fi

echo "Checking SSL connection between Pgpool-II and backend was ok."

./shutdownall
exit 0
