CREATE TEMP TABLE tmp (
  node_id text,
  hostname text,
  port text,
  status text,
  lb_weight text,
  role text,
  mode text);

INSERT INTO tmp VALUES
('0',:dir,'11002','2','0.500000','primary','s'),
('1',:dir,'11003','3','0.500000','standby','s'),
('0',:dir,'11002','2','0.500000','master','r'),
('1',:dir,'11003','3','0.500000','slave','r');

SELECT node_id,hostname,port,status,lb_weight,role
FROM tmp
WHERE mode = :mode
