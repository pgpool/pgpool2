#!/usr/bin/env bash
#-------------------------------------------------------------------
# test script for prefer_lower_delay_standby and standby delay.
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


# node 1,2 port number
PORT1=11003
PORT2=11004

# request replication pause and wait for confirmation.
# argument is a list of port numbers
# (currently only PORT1 is used)
function replay_pause
{
    for i in $1
    do
	echo ===$i===
	$PSQL -p $i test -c "$REPLAY_PAUSE"
    done

    for p in $1
    do
	for i in 1 2 3 4
	do
	    res=`$PSQL -p $p -q -t test -c "$REPLAY_STATE"|sed 's/ //'g`
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
    done
}

rm -fr $TESTDIR
mkdir $TESTDIR
cd $TESTDIR

# create test environment
echo -n "creating test environment..."
$PGPOOL_SETUP -m s -n 3 || exit 1
echo "done."
source ./bashrc.ports
export PGPORT=$PGPOOL_PORT

# The default wal_receiver_status_interval is 10 seconds, which is too
# slow to know the standby delay.
echo "wal_receiver_status_interval = 1s" >> data1/postgresql.conf
echo "wal_receiver_status_interval = 1s" >> data2/postgresql.conf

# Sleep time in seconds after pausing wal replay in case of
# delay_threshold_by_time.  By setting wal_receiver_status_interval to
# 1 second, we could set this as short as 3 seconds.
STIME=3

# ----------------------------------------------------------------------------------------
echo === Test1: delay_threshold with prefer_lower_delay_standby disabled. ===
# ----------------------------------------------------------------------------------------
echo "delay_threshold = 10" >> etc/pgpool.conf
echo "sr_check_period = 1" >> etc/pgpool.conf
echo "log_standby_delay = 'always'" >> etc/pgpool.conf
echo "log_min_messages = 'DEBUG1'" >> etc/pgpool.conf
# force load balance node to be 1.
echo "backend_weight0 = 0" >> etc/pgpool.conf
echo "backend_weight2 = 0" >> etc/pgpool.conf
./startall
wait_for_pgpool_startup

# Pause replay on node 1. Since prefer_lower_delay_standby is
# disabled, SELECT query should be sent to primary node.
replay_pause $PORT1

$PSQL test <<EOF
CREATE TABLE t1(i INTEGER);
CREATE TABLE t2(i INTEGER);
CREATE SEQUENCE myseq;
EOF

$PSQL test <<EOF
INSERT INTO t1 SELECT * FROM generate_series(1,100);
SELECT pg_sleep(4);
SHOW POOL_NODES;
SELECT * FROM t1 LIMIT 1;
EOF
fgrep "SELECT * FROM t1 LIMIT 1;" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
if [ $? != 0 ];then
    # expected result not found
    echo fail: query was not sent to primary node.
    ./shutdownall
    exit 1
fi
echo ok: testing delay_threshold with prefer_lower_delay_standby disabled succeeded.
echo resume streaming replication node 1
$PSQL -p $PORT1 test -c "$REPLAY_RESUME"
sleep 2
./shutdownall

# ----------------------------------------------------------------------------------------
echo === Test2: delay_threshold_by_time with prefer_lower_delay_standby disabled. ===
# ----------------------------------------------------------------------------------------
echo Start testing delay_threshold_by_time with prefer_lower_delay_standby disabled
echo "delay_threshold = 0" >> etc/pgpool.conf
echo "delay_threshold_by_time = 1" >> etc/pgpool.conf
./startall
wait_for_pgpool_startup
# pause replay on node 1
replay_pause $PORT1

$PSQL test <<EOF
INSERT INTO t1 SELECT * FROM generate_series(1,100);
EOF
sleep $STIME
$PSQL test <<EOF
SHOW POOL_NODES;
SELECT * FROM t1 LIMIT 1;
EOF
fgrep "SELECT * FROM t1 LIMIT 1;" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
if [ $? != 0 ];then
    # expected result not found
    echo fail: query was not sent to primary node.
    ./shutdownall
    exit 1
fi
echo ok: testing delay_threshold_by_time with prefer_lower_delay_standby disabled succeeded.
./shutdownall
# unforce load balance node to be 1.
echo "backend_weight0 = 1" >> etc/pgpool.conf
echo "backend_weight2 = 1" >> etc/pgpool.conf

# ----------------------------------------------------------------------------------------
echo === Test3: check to see if pgpool selects proper node for load balance ===
echo at the connection time with prefer_lower_delay_standby enabled.
# ----------------------------------------------------------------------------------------

# Redirect connection from app "psql" to node 1. This will make writing test easier.
echo "app_name_redirect_preference_list = 'psql:1'" >> etc/pgpool.conf
echo "prefer_lower_delay_standby = on" >> etc/pgpool.conf
./startall
wait_for_pgpool_startup

echo start: prefer_lower_delay_standby is on.
$PSQL test <<EOF
SHOW POOL_NODES;
EOF

# pause replay on node 1
replay_pause $PORT1

$PSQL test <<EOF
INSERT INTO t1 SELECT * FROM generate_series(1,1000);
EOF
sleep $STIME
$PSQL test <<EOF
SHOW POOL_NODES;
SELECT * FROM t1 LIMIT 1;
EOF

fgrep "SELECT * FROM t1 LIMIT 1;" log/pgpool.log |grep "DB node id: 2">/dev/null 2>&1
if [ $? != 0 ];then
    # expected result not found
    echo fail: query was not sent to node 2.
    ./shutdownall
    exit 1
fi

echo ok: query is sent to another standby node.

echo resume streaming replication node 1
$PSQL -p $PORT1 test -c "$REPLAY_RESUME"
sleep 2

# ----------------------------------------------------------------------------------------
echo === Test4: check to see if pgpool selects proper node for load balance ===
echo while in a session. For the test we use SELECT using write
echo function. It should be sent to primary node.
# see bug #798.
# https://www.pgpool.net/mantisbt/view.php?id=798
# ----------------------------------------------------------------------------------------

$PSQL test <<EOF
\! $PSQL -p $PORT1 test -c "$REPLAY_PAUSE"
SELECT pg_sleep(2);
INSERT INTO t1 SELECT * FROM generate_series(1,100);
EOF
sleep $STIME
$PSQL test <<EOF
SHOW POOL_NODES;
SELECT nextval('myseq');
EOF

fgrep "SELECT nextval('myseq');" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
if [ $? != 0 ];then
    # expected result not found
    echo fail: write query is not sent to primary node.
    ./shutdownall
    exit 1
fi

echo Test5: prefer_lower_delay_standby is off.

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
replay_pause $PORT1

$PSQL test <<EOF
INSERT INTO t2 SELECT * FROM generate_series(1,100);
EOF
sleep $STIME
$PSQL test <<EOF
SELECT * FROM t2 LIMIT 1;
EOF

fgrep "SELECT * FROM t2 LIMIT 1;" log/pgpool.log |grep "DB node id: 0">/dev/null 2>&1
if [ $? != 0 ];then
    # expected result not found
    echo fail: query was sent to standby node.
    ./shutdownall
    exit 1
fi

echo ok: prefer lower delay standby works.

./shutdownall

exit 0
