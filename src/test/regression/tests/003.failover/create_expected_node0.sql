CREATE TEMP TABLE tmp (
  node_id text,
  hostname text,
  port text,
  status text,
  lb_weight text,
  role text,
  select_cnt text,
  load_balance_node text,
  replication_delay text,
  replication_state text,
  replication_sync_state text,
  last_status_change text,
  mode text);

INSERT INTO tmp VALUES
('0',:dir,'11002','down','0.500000','standby','0','false','0','','','XXXX-XX-XX XX:XX:XX','s'),
('1',:dir,'11003','up','0.500000','primary','0','false','0','','','XXXX-XX-XX XX:XX:XX','s'),
('0',:dir,'11002','down','0.500000','replica','0','false','0','','','XXXX-XX-XX XX:XX:XX','r'),
('1',:dir,'11003','up','0.500000','main','0','false','0','','','XXXX-XX-XX XX:XX:XX','r');

SELECT node_id,hostname,port,status,lb_weight,role,select_cnt,load_balance_node,replication_delay,replication_state, replication_sync_state, last_status_change
FROM tmp
WHERE mode = :mode
