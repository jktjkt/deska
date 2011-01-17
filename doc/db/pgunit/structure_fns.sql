SET SEARCH_PATH TO pgunit, public;
DROP FUNCTION pgunit.test_sample();

CREATE FUNCTION pgunit.test_sample () RETURNS testfunc[]
AS
$body$
SELECT pgunit.testcase(
    $setUp$
        -- setUp code is executed before ANY test function code (see below).
        -- Effect of this execution is persistent only during the code
        -- block execution and rolled back after the test is finished.
        CREATE TABLE deska_dev.testTabPK(
	col1 INT,
	pkcol INT PRIMARY KEY,
	col2 text
	);
    $setUp$,
    ARRAY[
        -- This is a first test function code. Just check if we can insert
        -- into the table created in setUp.
        'first test: get_primary_key_column is okay', $sql$     
        DECLARE
	    pk NAME;
	BEGIN
	    SELECT deska_dev.get_primary_key_column('testtabpk') INTO pk;
	    PERFORM pgunit.assert_same('pkcol', pk);
	END;
	$sql$
    ]
);
$body$
    LANGUAGE sql;

select pgunit.testrunner(NULL);