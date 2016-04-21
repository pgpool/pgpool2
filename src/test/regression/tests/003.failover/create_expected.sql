CREATE TEMP TABLE tmp (
  node_id text,
  hostname text,
  port text,
  status text,
  lb_weight text,
  role text,
  select_cnt text,
  load_balance_node text,
  mode text);

INSERT INTO tmp VALUES
('0',:dir,'11002','2','0.500000','primary','0','false','s'),
('1',:dir,'11003','3','0.500000','standby','0','false','s'),
('0',:dir,'11002','2','0.500000','master','0','false','r'),
('1',:dir,'11003','3','0.500000','slave','0','false','r');

SELECT node_id,hostname,port,status,lb_weight,role,select_cnt,load_balance_node
FROM tmp
WHERE mode = :mode
