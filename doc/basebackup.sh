#! /bin/sh
# Recovery script for streaming replication.
# This script assumes followings:
#
# 1) Executed on the primary node by pgpool_recovery function.
# 2) Executed as "recovery_user" (usually PostgreSQL super user).
# 3) "postgres" database is available.
# 4) can connect to "postgres" database via UNIX domain socket without
#    password.
# 5) Password less access using ssh from the primary node to the
#    target node is possible.
# 6) Arguments for the scripts are:
#	$1: database cluster path on the primary node
#	$2: hostname or IP address to be recovered
#	$3: database cluster path on target node
#	$4: port number of of the primary database cluster
# 7) psql, rsync, ssh are available in the command search path.

datadir=$1
desthost=$2
destdir=$3
port=$4

psql -p $port -c "SELECT pg_start_backup('Streaming Replication', true)" postgres

rsync -C -a -c --delete -e ssh --exclude postgresql.conf --exclude postmaster.pid \
--exclude postmaster.opts --exclude pg_log --exclude pg_xlog \
--exclude recovery.conf $datadir/ $desthost:$destdir/

ssh -T $desthost mv $destdir/recovery.done $destdir/recovery.conf

psql -c "SELECT pg_stop_backup()" postgres
