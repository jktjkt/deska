\i util-include-path.sql

BEGIN;

CREATE FUNCTION test_pass()
RETURNS SETOF text
AS
$$
BEGIN
	RETURN NEXT pass();
END
$$
LANGUAGE plpgsql;


SELECT * FROM runtests();

ROLLBACK;
