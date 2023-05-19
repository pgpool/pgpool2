SELECT f1(1);	-- no load balance because volatile function
SELECT public.f2(1);	-- no load balance because volatile function
SELECT f3(1);	-- load balance because statble function
SELECT public.f4(1);	-- load balance because stable function
PREPARE p1 AS SELECT f1(1);	-- no load balance because volatile function
EXECUTE p1;	-- no load balance because volatile function
DEALLOCATE p1;	-- no load balance because volatile function
PREPARE p2 AS SELECT f3(1);	-- load balance because stable function
EXECUTE p2;	-- load balance because stable function
DEALLOCATE p2;	-- load balance because stable function
-- PREPARE in transaction test
BEGIN;
PREPARE p3 AS SELECT 1;	-- load balance
EXECUTE p3;	-- load balance
DEALLOCATE p3;	-- load balance
END;
-- PREPARE in writing transaction test
BEGIN;
PREPARE p3 AS SELECT 1;	-- load balance
SELECT f1(1);	-- no load balance. writing transaction is set
-- PREPARE is re-execute and EXECUTE no load balance in SL_MODE.
-- in other mode, load balance
EXECUTE p3;	
-- no load balance in SL_MODE.
-- in other mode, load balance
DEALLOCATE p3;	
END;
