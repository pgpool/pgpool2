#! /bin/sh
# Execute command by failover.
# special values:  %d = node id
#                  %h = host name
#                  %p = port number
#                  %D = database cluster path
#                  %m = new master node id
#                  %M = old master node id
#                  %H = new master node host name
#                  %P = old primary node id
#                  %% = '%' character
#                  %R = new master database cluster path
#                  %% = '%' character

source /etc/pgpool-II/config_for_script

log=$PGPOOL_LOG_DIR/failover.log

failed_node_id=$1
failed_host=$2
failed_port=$3
failed_dir=$4
new_master_id=$5
old_master_id=$6
new_master_host=$7
old_primary_node_id=$8
new_master_port=$9
new_master_dir=${10}

echo "----------------------------------------------------------------------" >> $log
date >> $log
echo "----------------------------------------------------------------------" >> $log
echo "" >> $log

echo "
[ node which failed ]
failed_node_id         $failed_node_id
failed_host            $failed_host
failed_port            $failed_port
failed_dir             $failed_dir

[ before failover ]
old_primary_node_id    $old_primary_node_id
old_master_id          $old_master_id

[ after faiover ]
new_master_id          $new_master_id
new_master_host        $new_master_host
new_master_port        $new_master_port
new_master_dir         $new_master_dir
" >> $log

# Do promote only when the primary node failes
if [ $failed_node_id = $old_primary_node_id ]; then
    echo "The primary node (node $old_primary_node_id) dies." >> $log
    echo "Node $new_master_id takes over the primary." >> $log

    ssh $PGSUPERUSER@$new_master_host -T "$pg_ctl -D $new_master_dir promote" >> $log

else
    echo "Node $failed_node_id which is not the primary dies. This script doesnt't anything."
fi

echo "" >> $log
