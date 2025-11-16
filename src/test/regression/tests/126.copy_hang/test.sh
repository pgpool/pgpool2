#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for copy plus error case.
# It was reported that following sequece of copy command cause psql hang.
#
# CREATE TEMP TABLE vistest (a text);  
# COPY vistest FROM stdin CSV FREEZE;
# Enter data to be copied followed by a newline.
# End with a backslash and a period on a line by itself, or an EOF signal.
# >> p
# >> g
# >> \.
#
# In the normal case an error should be returned to psql.
#ERROR:  cannot perform COPY FREEZE because the table was not created or truncated in the current subtransaction

source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PGPROTO=$PGPOOL_INSTALL_DIR/bin/pgproto

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment
echo -n "creating test environment..."
$PGPOOL_SETUP -n 3 || exit 1
echo "done."

source ./bashrc.ports

export PGPORT=$PGPOOL_PORT

./startall
wait_for_pgpool_startup

# execute COPY

timeout 10 $PSQL test <<EOF
CREATE TEMP TABLE vistest (a text);  
COPY vistest FROM stdin CSV FREEZE;
p
g
\\.
\\q
EOF

if [ ! $? -eq 0 ];then
    ./shutdownall
    exit 1
fi

#
# Another COPY FROM STDIN hang case.
# commit ab091663b09ef8c2d0a1841921597948c597444e
# If Flush or Sync message is sent from frontend during COPY IN mode,
# pgpool hangs.
# In order to reproduce the problem, we use pgproto because psql
# cannot send Flush or Sync during COPY FROM STDIN

timeout 10 $PGPROTO -d test -f ../pgproto.data
if [ ! $? -eq 0 ];then
    ./shutdownall
    exit 1
fi

#
# Test case for COPY OUT in extended query protocol mode segfaults.
# since this creates temp table, prevent load balance
echo "backend_weight1 = 0" >> etc/pgpool.conf
echo "backend_weight2 = 0" >> etc/pgpool.conf
# reload pgpool.conf and wait until the effect is apparent
./pgpool_reload
sleep 1
# run test script
$PGPROTO -d test -f ../pgproto-copy-out.data > copy-out-result 2>&1
cmp ../copy-out-expected copy-out-result
if [ ! $? -eq 0 ];then
    ./shutdownall
    exit 1
fi
./shutdownall
