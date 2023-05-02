#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for load balancing.
#
source $TESTLIBS
TESTDIR=testdir
PG_CTL=$PGBIN/pg_ctl
PSQL="$PGBIN/psql -X "

version=`$PSQL --version|awk '{print $3}'`
major_version=`echo ${version%.*} | sed 's/\([0-9]*\).*/\1/'`

if [ $major_version -ge 10 ];then
	REPLAY_PAUSE="SELECT pg_wal_replay_pause();"
	REPLAY_RESUME="SELECT pg_wal_replay_resume();"
else
	REPLAY_PAUSE="SELECT pg_xlog_replay_pause();"
	REPLAY_RESUME="SELECT pg_xlog_replay_resume();"
fi

# PostgreSQL 13 or before does not have pg_get_wal_replay_pause_state().
# In these versions SELECT pg_wal_replay_pause() immediately pauses.
if [ $major_version -ge 14 ];then
	REPLAY_STATE="SELECT pg_get_wal_replay_pause_state()"
else
	REPLAY_STATE="SELECT 'paused'"
fi


# node 1 port number
PORT1=11003

# request replication pause and wait for confirmation
function replay_pause
{
    $PSQL -p $PORT1 test -c "$REPLAY_PAUSE"
    for i in 1 2 3 4
    do
	res=`$PSQL -p $PORT1 -q -t test -c "$REPLAY_STATE"|sed 's/ //'g`
	if [ "$res" = "paused" ];then
	    break;
	else
	    echo pause state: $res
	fi
	sleep 1
    done
    if [ "$res" != "paused" ];then
	echo replay pause failed.
	./shutdownall
	exit 1
    fi
}

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment
echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 3 || exit 1
echo "done."

source ./bashrc.ports
echo "app_name_redirect_preference_list = 'psql:1'" >> etc/pgpool.conf
echo "delay_threshold = 10" >> etc/pgpool.conf
echo "prefer_lower_delay_standby = on" >> etc/pgpool.conf
echo "sr_check_period = 3" >> etc/pgpool.conf

./startall

export PGPORT=$PGPOOL_PORT

wait_for_pgpool_startup

$PSQL test <<EOF
CREATE TABLE t1(i INTEGER);
CREATE TABLE t2(i INTEGER);
CREATE SEQUENCE myseq;
EOF

echo start: prefer_lower_delay_standby is on.

# check to see if pgpool selects proper node for load balance
# at the connection time

# pause replay on node 1
replay_pause

$PSQL test <<EOF
PGPOOL SET log_min_messages TO DEBUG1;
INSERT INTO t1 SELECT * FROM generate_series(1,100);
SELECT pg_sleep(4);
SELECT * FROM t1 LIMIT 1;
EOF

fgrep "SELECT * FROM t1 LIMIT 1;" log/pgpool.log |grep "DB node id: 2">/dev/null 2>&1
if [ $? != 0 ];then
    # expected result not found
    echo fail: query is sent to primary node.
    ./shutdownall
    exit 1
fi

echo ok: query is sent to another standby node.

echo resume streaming replication node 1
$PSQL -p $PORT1 test -c "$REPLAY_RESUME"
sleep 4

# check to see if pgpool selects proper node for load balance
# while in a session. For the test we use SELECT using write
# function. It should be sent to primary node.
# see bug #798.
# https://www.pgpool.net/mantisbt/view.php?id=798

$PSQL test <<EOF
PGPOOL SET log_min_messages TO DEBUG1;
\! $PSQL -p $PORT1 test -c "$REPLAY_PAUSE"
SELECT pg_sleep(4);
INSERT INTO t1 SELECT * FROM generate_series(1,100);
SELECT pg_sleep(4);
SELECT nextval('myseq');
EOF

fgrep "SELECT nextval('myseq');" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
if [ $? != 0 ];then
    # expected result not found
    echo fail: write query is not sent to primary node.
    ./shutdownall
    exit 1
fi

echo start: prefer_lower_delay_standby is off.

$PSQL -p $PORT1 test -c "$REPLAY_RESUME"

echo "prefer_lower_delay_standby = off" >> etc/pgpool.conf

$PGPOOL_INSTALL_DIR/bin/pcp_reload_config -w -h localhost -p $PCP_PORT

while :
do
    $PSQL test -c "PGPOOL SHOW prefer_lower_delay_standby" |grep off
    if [ $? = 0 ]; then
	break
    fi
    sleep 1
done

# pause replay on node 1
replay_pause

$PSQL test <<EOF
PGPOOL SET log_min_messages TO DEBUG1;
INSERT INTO t2 SELECT * FROM generate_series(1,100);
SELECT pg_sleep(4);
SELECT * FROM t2 LIMIT 1;
EOF

fgrep "SELECT * FROM t2 LIMIT 1;" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
if [ $? != 0 ];then
    # expected result not found
    echo fail: query is sent to standby node.
    ./shutdownall
    exit 1
fi

echo ok: prefer lower delay standby works.

./shutdownall

exit 0
