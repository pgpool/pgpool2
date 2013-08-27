#! /bin/sh
# Recovery script for streaming replication.
# This script assumes that DB node 0 is primary, and 1 is standby.
#
datadir=$1
desthost=$2
destdir=$3
port=$4

psql -p $port -c "SELECT pg_start_backup('Streaming Replication', true)" postgres

rsync -C -a --delete -e ssh --exclude postgresql.conf --exclude postmaster.pid \
--exclude postmaster.opts --exclude pg_log --exclude pg_xlog \
--exclude recovery.conf $datadir/ $desthost:$destdir/

ssh -T $desthost mv $destdir/recovery.done $destdir/recovery.conf

psql -c "SELECT pg_stop_backup()" postgres
