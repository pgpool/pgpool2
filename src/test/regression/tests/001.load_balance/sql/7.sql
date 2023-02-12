-- ordinary read only SELECT: load balance expected
SELECT 1;
-- multi-statement query including BEGIN
BEGIN\;SELECT 1;
-- ordinary read only SELECT: load balance expected
SELECT 2;
-- tx started by multi-statement query ends
END;
-- multi-statement query including BEGIN
BEGIN\;SELECT 1;
-- SAVEPOINT
SAVEPOINT a;
-- PREPARE
PREPARE foo AS SELECT 2;
-- EXECUTE
EXECUTE foo;
-- DEALLOCATE
DEALLOCATE foo;
-- ROLLBACK TO
ROLLBACK TO a;
-- tx started by multi-statement query ends
END;
-- multi-statement query including BEGIN and ROLLBACK
BEGIN\;SELECT 1\;ROLLBACK;
-- ordinary read only SELECT: load balance expected
SELECT 1;
-- multi-statement query including BEGIN and invalid query
BEGIN\;SELECT 1\;FOO;
-- ordinary read only SELECT: load balance expected
SELECT 1;
