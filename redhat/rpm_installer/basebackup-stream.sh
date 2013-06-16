#!/bin/sh
# Do base backup by rsync in streaming replication

master_dir=$1
dest_host=$2
dest_dir=$3

source /etc/pgpool-II/config_for_script

log=$PGPOOL_LOG_DIR/recovery.log

if [ $dest_host = $NODE0_HOST ]; then
  master_host=$NODE1_HOST
  master_port=$NODE1_PORT

elif [ $dest_host = $NODE1_HOST ]; then
  master_host=$NODE0_HOST
  master_port=$NODE0_PORT

else
  exit 1
fi

echo "----------------------------------------------------------------------" >> $log
date >> $log
echo "----------------------------------------------------------------------" >> $log
echo "" >> $log

# start base backup
echo "1. pg_start_backup" >> $log
$psql -p $master_port -U $PGSUPERUSER -c "SELECT pg_start_backup('Streaming Replication', true)" postgres

# rsync db cluster
echo "2. rsync: `whoami`@localhost:$master_dir -> $PGSUPERUSER@$dest_host:$dest_dir" >> $log
rsync -C -a -c --delete \
--exclude postmaster.pid --exclude postmaster.opts --exclude pg_log \
--exclude recovery.conf --exclude recovery.done --exclude pg_xlog/* \
$master_dir/ $PGSUPERUSER@$dest_host:$dest_dir/

# recovery.conf
echo "3. create recovery.conf" >> $log
cat > recovery.conf <<EOF
standby_mode             = 'on'
primary_conninfo         = 'host=$master_host port=$master_port user=$PGSUPERUSER'
recovery_target_timeline = 'latest'
EOF
scp recovery.conf $PGSUPERUSER@$dest_host:$dest_dir/
rm -f recovery.conf

# stop base backup
echo "4. pg_stop_backup" >> $log
$psql -p $master_port -U $PGSUPERUSER -c "SELECT pg_stop_backup()" postgres

echo "" >> $log
exit 0
