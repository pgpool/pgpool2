#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for SSL connection upon reloading for: frontend <--> Pgpool-II and Pgpool-II and PostgreSQL.
#
source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PG_CTL=$PGBIN/pg_ctl
export PGDATABASE=test
SSL_KEY=server.key
SSL_CRT=server.crt

#-------------------------------------------
# Check psql output for \conninfo to see if SSL enabled
#-------------------------------------------
function check_ssl {
    $PSQL -h localhost test <<EOF > result
\conninfo
\q
EOF

    # PostgreSQL 18 or later prints tablular output for \conninfo.
    # For SSL, "SSL Connection | true (or false)"
    if [ $PGVERSION -ge 18 ];then
	grep "SSL Connection" result|grep true
    else
	grep SSL result
    fi
}

# ---------------------------------------------------------------
# Test ssl configuration param.
# params:
# $1: configuration name. e.g. ssl_cert
# $2: good value for the config
#
# This performs following tests:
# 1. Set bad value to the config and restart pgpool to make sure SSL connection does not establish.
# 2. Set good value to the config and reload to make sure SSL connection establishes.
# If test fails, exit with status 1
function test_ssl {
    # set bad value
    echo "$1 = 'bad_value'" >> etc/pgpool.conf
    # restart pgpool
    ./startall
    wait_for_pgpool_startup
    check_ssl
    if [ $? = 0 ];then
	echo "Checking SSL connection between frontend and Pgpool-II succeeded despite bad config value for $1"
	./shutdownall
	exit 1
    fi
    echo "Checking SSL connection between frontend and Pgpool-II failed due to bad config value for $1 as expected."

    # Make sure that SSL connection succeeds with good config value
    echo "$1 = '$2'" >> etc/pgpool.conf
    ./pgpool_reload
    sleep 1
    check_ssl
    if [ $? = 0 ];then
	echo "Checking SSL connection between frontend and Pgpool-II succeeded with good $1"
    else
	echo "Checking SSL connection between frontend and Pgpool-II failed with good $1"
	./shutdownall
	exit 1
    fi
    ./shutdownall
}

# main script starts here
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
echo "ssl_key = '$SSL_KEY'" >> etc/pgpool.conf
echo "ssl_cert = '$SSL_CRT'" >> etc/pgpool.conf
echo "ssl_prefer_server_ciphers = on" >> etc/pgpool.conf
echo "ssl_ciphers = 'EECDH:HIGH:MEDIUM:+3DES:!aNULL'" >> etc/pgpool.conf

echo "ssl = on" >> data0/postgresql.conf
echo "ssl_cert_file = '$SSL_CRT'" >> data0/postgresql.conf
echo "ssl_key_file = '$SSL_KEY'" >> data0/postgresql.conf

# backend must be connected via TCP/IP
echo "backend_hostname0 = 'localhost'" >> etc/pgpool.conf

# produce debug message since the only way to confirm the SSL
# connections is being established is, look into the debug log.
echo "log_min_messages = debug5" >> etc/pgpool.conf

# allow to access IPv6 localhost
echo "host    all             all             ::1/128                 trust" >> data0/pg_hba.conf

source ./bashrc.ports

./startall

export PGPORT=$PGPOOL_PORT

wait_for_pgpool_startup

# first, checking frontend<-->Pgpool-II...
check_ssl
if [ $? != 0 ];then
    echo "Checking SSL connection between frontend and Pgpool-II failed."
    ./shutdownall
    exit 1
fi
echo "Checking SSL connection between frontend and Pgpool-II was ok."

if [ $PGVERSION -ge 18 ];then
    grep "SSL Protocol" result|grep TLSv1.2
else
    grep SSL result |grep TLSv1.2
fi

# if SSl protocol version TLSv1.2
if [ $? = 0 ];then
    grep SSL result |grep ECDH

    if [ $? != 0 ];then
        echo "Checking SSL connection with ECDH between frontend and Pgpool-II failed."
        ./shutdownall
        exit 1
    fi
	echo "Checking SSL connection with ECDH between frontend and Pgpool-II was ok."
fi

grep "client->server SSL response: S" log/pgpool.log >/dev/null
if [ $? != 0 ];then
    echo "Checking SSL connection between Pgpool-II and backend failed."
    ./shutdownall
    exit 1
fi
echo "Checking SSL connection between Pgpool-II and backend was ok."

# So far SSL connection between clients and Pgpool-II, Pgpool-II and backend are ok.

./shutdownall

# ---------------------------------------------------------------
# Test SSL params

config_names[0]=ssl_cert
config_names[1]=ssl_ciphers
config_names[2]=ssl_crl_file
config_names[3]=ssl_dh_params_file	# ssl_dh_params_file can be a invalid file (fallback mechanism)
config_names[4]=ssl_ecdh_curve
config_names[5]=ssl_key
config_names[6]=ssl_passphrase_command	# cert does not require pass passphrase
config_names[7]=ssl_prefer_server_ciphers	# this affects server side ciphers

good_values[0]=server.crt
good_values[1]=HIGH:MEDIUM:+3DES:!aNULL
good_values[2]=""
good_values[3]=skip
good_values[4]=prime256v1
good_values[5]=server.key
good_values[6]=skip
good_values[7]=skip

for i in {0..7}
do
    echo "===== ${config_names[$i]} ====="
    if [ "${good_values[$i]}" = "skip" ];then
	echo "skip this test"
    else
	test_ssl ${config_names[$i]} ${good_values[$i]}
    fi
done

# ---------------------------------------------------------------
# Test 4: ssl_ca_cert swap – client cert rejected after CA rotation
#
# 1. Two independent self-signed CAs are generated (CA #1 / CA #2).
# 2. A client certificate is signed with CA #1.
# 3. pgpool starts trusting CA #1 with pool_hba.conf set to require
#    client-certificate authentication (cert method).
# 4. Verify the CA1-signed client cert is accepted.
# 5. Reload pgpool with ssl_ca_cert pointing at CA #2.
# 6. Verify the same client cert is now rejected because its issuer
#    (CA #1) is no longer trusted.
# ---------------------------------------------------------------
echo "===== ssl_ca_cert swap (client cert auth reload) ====="

CADIR=`pwd`/catest
mkdir -p "$CADIR"

openssl req -new -x509 -days 3650 -nodes \
    -subj "/CN=TestCA1" \
    -keyout "$CADIR/ca1.key" -out "$CADIR/ca1.crt" 2>/dev/null

openssl req -new -x509 -days 3650 -nodes \
    -subj "/CN=TestCA2" \
    -keyout "$CADIR/ca2.key" -out "$CADIR/ca2.crt" 2>/dev/null

openssl req -new -nodes \
    -subj "/CN=ssltest" \
    -keyout "$CADIR/client.key" -out "$CADIR/client.csr" 2>/dev/null
openssl x509 -req -days 3650 \
    -CA "$CADIR/ca1.crt" -CAkey "$CADIR/ca1.key" -CAcreateserial \
    -in "$CADIR/client.csr" -out "$CADIR/client.crt" 2>/dev/null
chmod 600 "$CADIR/client.key"

# Generate a dedicated server CA and a new backend cert (with SAN for localhost)
# signed by it.  This CA is stable across the two reload phases so pgpool can
# always verify the backend, while ca1/ca2 are swapped to control client-cert
# trust.  ca1.crt and ca2.crt are used only for client-certificate auth.
openssl req -new -x509 -days 3650 -nodes \
    -subj "/CN=TestServerCA" \
    -keyout "$CADIR/server_ca.key" -out "$CADIR/server_ca.crt" 2>/dev/null

openssl req -new -nodes \
    -subj "/CN=localhost" \
    -addext "subjectAltName=DNS:localhost,IP:127.0.0.1" \
    -keyout "$CADIR/backend.key" -out "$CADIR/backend.csr" 2>/dev/null
openssl x509 -req -days 3650 \
    -CA "$CADIR/server_ca.crt" -CAkey "$CADIR/server_ca.key" -CAcreateserial \
    -extfile <(echo "subjectAltName=DNS:localhost,IP:127.0.0.1") \
    -in "$CADIR/backend.csr" -out "$CADIR/backend.crt" 2>/dev/null
chmod 600 "$CADIR/backend.key"

# Replace the static server.crt/server.key with the new CA-signed cert so that
# both pgpool (frontend SSL) and PostgreSQL (backend SSL) use it.
cp "$CADIR/backend.crt" etc/$SSL_CRT
cp "$CADIR/backend.key" etc/$SSL_KEY
chmod og-rwx etc/$SSL_KEY
cp "$CADIR/backend.crt" data0/$SSL_CRT
cp "$CADIR/backend.key" data0/$SSL_KEY
chmod og-rwx data0/$SSL_KEY

# CA bundles: server_ca (verifies backend) + client CA (verifies client cert).
cat "$CADIR/server_ca.crt" "$CADIR/ca1.crt" > "$CADIR/combined_ca1.crt"
cat "$CADIR/server_ca.crt" "$CADIR/ca2.crt" > "$CADIR/combined_ca2.crt"

echo "ssl_ca_cert = '$CADIR/combined_ca1.crt'" >> etc/pgpool.conf
echo "enable_pool_hba = on" >> etc/pgpool.conf
cat >> etc/pool_hba.conf <<'HBA'
hostssl all ssltest 127.0.0.1/32 cert
hostssl all ssltest ::1/128 cert
HBA

./startall
wait_for_pgpool_startup

$PSQL -h localhost -p $PGPOOL_PORT test -c "CREATE ROLE ssltest LOGIN" 2>/dev/null ||:

PGSSLMODE=require PGSSLCERT="$CADIR/client.crt" PGSSLKEY="$CADIR/client.key" \
    PGSSLROOTCERT="$CADIR/server_ca.crt" $PSQL -h localhost -U ssltest test -c "SELECT 1" >/dev/null 2>&1
if [ $? = 0 ]; then
    echo "CA cert swap: CA1-signed client cert accepted before reload – ok."
else
    echo "CA cert swap: CA1-signed client cert rejected before reload – unexpected."
    ./shutdownall
    exit 1
fi

echo "ssl_ca_cert = '$CADIR/combined_ca2.crt'" >> etc/pgpool.conf
./pgpool_reload
sleep 1

PGSSLMODE=require PGSSLCERT="$CADIR/client.crt" PGSSLKEY="$CADIR/client.key" \
    PGSSLROOTCERT="$CADIR/server_ca.crt" $PSQL -h localhost -U ssltest test -c "SELECT 1" >/dev/null 2>&1
if [ $? != 0 ]; then
    echo "CA cert swap: CA1-signed client cert rejected after reload to CA2 – ok."
else
    echo "CA cert swap: CA1-signed client cert still accepted after CA2 reload – unexpected."
    ./shutdownall
    exit 1
fi

./shutdownall
exit 0
