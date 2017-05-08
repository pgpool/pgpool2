CREATE TEMP TABLE tmp (
  node_id text,
  hostname text,
  port text,
  status text,
  lb_weight text,
  role text,
  select_cnt text,
  mode text);

INSERT INTO tmp VALUES
('0',:dir,'11002','3','0.500000','standby','0','s'),
('1',:dir,'11003','2','0.500000','primary','0','s'),
('0',:dir,'11002','3','0.500000','slave','0','r'),
('1',:dir,'11003','2','0.500000','master','0','r');

SELECT node_id,hostname,port,status,lb_weight,role,select_cnt
FROM tmp
WHERE mode = :mode
