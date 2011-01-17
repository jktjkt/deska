CREATE SCHEMA pgunit AUTHORIZATION postgres;
--
-- Definition for function _exec (OID = 2868774) :
--
SET search_path = pgunit, pg_catalog;
SET check_function_bodies = false;
CREATE FUNCTION pgunit._exec (in_sql character varying) RETURNS void
AS
$body$
DECLARE
    s VARCHAR;
    in_sql_no_comments VARCHAR;
    func VARCHAR;
BEGIN
    func := 'pgunit_exec_tmp_' || md5(clock_timestamp()::VARCHAR || in_sql);
    in_sql_no_comments := trim(regexp_replace(
        in_sql,
        E' /\\*.*?\\*/ | --[^\\n]* ',
        '', 'mgx'
    ));
    s := 'CREATE FUNCTION '
        || func || '() RETURNS void AS $pgunit_exec_tmp$'
        || CASE WHEN upper(in_sql_no_comments) ~ E'^DECLARE\\s+'
                THEN in_sql
                ELSE 'BEGIN ' || in_sql || ' END;'
           END
        || '$pgunit_exec_tmp$ LANGUAGE ''plpgsql'' VOLATILE CALLED ON NULL INPUT SECURITY INVOKER';
    EXECUTE s;
    EXECUTE 'SELECT ' || func || '()';
    EXECUTE 'DROP FUNCTION ' || func || '()';
END;
$body$
    LANGUAGE plpgsql;
--
-- Definition for function fail (OID = 2868776) :
--
CREATE FUNCTION pgunit.fail (in_msg character varying) RETURNS void
AS
$body$
BEGIN
    RAISE EXCEPTION '%', in_msg;
END;
$body$
    LANGUAGE plpgsql;
--
-- Definition for function _fail_assert (OID = 2868777) :
--
CREATE FUNCTION pgunit._fail_assert (in_op character varying, in_exp anyelement, in_got anyelement) RETURNS void
AS
$body$
BEGIN
    PERFORM pgunit.fail(
        E'Failed asserting that two things are ' || in_op || E'.\n'
        || E'expected string <' || COALESCE(in_exp::VARCHAR, '<NULL>') || E'>\n'
        || E'got string      <' || COALESCE(in_got::VARCHAR, '<NULL>') || E'>'
    );
END;
$body$
    LANGUAGE plpgsql;
--
-- Definition for function _wildcard_match (OID = 2868778) :
--
CREATE FUNCTION pgunit._wildcard_match (in_wildcard character varying, in_str character varying) RETURNS boolean
AS
$body$
DECLARE
    i INTEGER;
    part VARCHAR;
BEGIN
    FOR part IN
        SELECT regexp_split_to_table(in_wildcard, E'\\s+')
    LOOP
        IF in_str LIKE part THEN RETURN TRUE; END IF;
    END LOOP;
    RETURN false;
END;
$body$
    LANGUAGE plpgsql;
--
-- Definition for type testfunc (OID = 2868783) :
--
CREATE TYPE pgunit.testfunc AS (
  container text,
  name text,
  setup text,
  func text,
  error text
);
--
-- Definition for function version (OID = 2868784) :
--
CREATE FUNCTION pgunit."version" () RETURNS character varying
AS
$body$
SELECT '2008-11-09'::VARCHAR;
$body$
    LANGUAGE sql;
--
-- Definition for function assert_same (OID = 2868789) :
--
CREATE FUNCTION pgunit.assert_same (in_expected anyelement, in_got anyelement, in_msg character varying) RETURNS void
AS
$body$
SELECT CASE WHEN $1 IS DISTINCT FROM $2
    THEN pgunit._fail_assert('same (' || $3 || ')', $1, $2)
    ELSE null
END
$body$
    LANGUAGE sql;
--
-- Definition for function testcase (OID = 2868792) :
--
CREATE FUNCTION pgunit.testcase (in_setup character varying, in_tests character varying[]) RETURNS testfunc[]
AS
$body$
DECLARE
    i INTEGER;
    tests pgunit.testfunc[];
BEGIN
    tests := '{}';
    FOR i IN array_lower(in_tests, 1) .. array_upper(in_tests, 1) BY 2 LOOP
        tests := tests || ROW(NULL, in_tests[i], in_setup, in_tests[i + 1], NULL)::pgunit.testfunc;
    END LOOP;
    RETURN tests;
END;
$body$
    LANGUAGE plpgsql;
--
-- Definition for function test_wildcard_match (OID = 2868804) :
--
CREATE FUNCTION pgunit.test_wildcard_match () RETURNS testfunc[]
AS
$body$
SELECT pgunit.testcase(
    $setUp$
        -- Nothing.
    $setUp$,
    ARRAY[
        'one value', $sql$
            PERFORM pgunit.assert_same(true, pgunit._wildcard_match('a%b', 'axb'));
        $sql$,

        'two values', $sql$
            PERFORM pgunit.assert_same(true, pgunit._wildcard_match('a%b zz%', 'zz123'));
        $sql$,

        'no matches', $sql$
            PERFORM pgunit.assert_same(false, pgunit._wildcard_match('a%b zz%', 'uuuu'));
        $sql$
    ]
);
$body$
    LANGUAGE sql;
--
-- Definition for function assert_same (OID = 2870068) :
--
CREATE FUNCTION pgunit.assert_same (in_expected anyelement, in_got anyelement) RETURNS void
AS
$body$
SELECT CASE WHEN $1 IS DISTINCT FROM $2
    THEN pgunit._fail_assert('same', $1, $2)
    ELSE null
END
$body$
    LANGUAGE sql;
--
-- Definition for function testcollect (OID = 2870173) :
--
CREATE FUNCTION pgunit.testcollect (in_wildcard character varying) RETURNS testfunc[]
AS
$body$
DECLARE
    fqname VARCHAR;
    tests pgunit.testfunc[];
    tmp pgunit.testfunc[];
    test pgunit.testfunc;
BEGIN
    tests := '{}';
    FOR fqname IN
        SELECT
            quote_ident(nspname) || '.' || quote_ident(proname)
        FROM
            pg_proc
            JOIN pg_namespace ON pg_namespace.oid = pronamespace
            JOIN pg_type ON pg_type.oid = prorettype
            JOIN pg_type e ON e.oid = pg_type.typelem
        WHERE
            proname like E'test\\_%'
            AND e.typname = 'testfunc'
            AND CASE WHEN COALESCE(in_wildcard, '') <> ''
                THEN
                    pgunit._wildcard_match(in_wildcard, proname::VARCHAR)
                    OR pgunit._wildcard_match(in_wildcard, nspname || '.' || proname)
                ELSE true
            END
        ORDER BY 1
    LOOP
        EXECUTE 'SELECT ' || fqname || '()' INTO tmp;
        FOR i IN array_lower(tmp, 1) .. array_upper(tmp, 1) LOOP
            test := tmp[i];
            test.container := fqname;
            tmp[i] := test;
        END LOOP;
        tests := tests || tmp;
    END LOOP;

    RETURN tests;
END;
$body$
    LANGUAGE plpgsql;
--
-- Definition for function test_assert_same (OID = 2874717) :
--
CREATE FUNCTION pgunit.test_assert_same () RETURNS testfunc[]
AS
$body$
SELECT pgunit.testcase(
    $setUp$
        -- Nothing.
    $setUp$,
    ARRAY[
        'matchedValues', $sql$
            PERFORM pgunit.assert_same('abc'::VARCHAR, 'abc');
        $sql$,

        'mismatchedValues', $sql$
            BEGIN
                PERFORM pgunit.assert_same('abc'::VARCHAR, 'def');
            EXCEPTION
                WHEN raise_exception THEN
            		PERFORM pgunit.assert_same(E'Failed asserting that two things are same.\nexpected string <abc>\ngot string      <def>', SQLERRM);
            END;
        $sql$
    ]
);
$body$
    LANGUAGE sql;
--
-- Definition for function _runwrapped (OID = 2885898) :
--
CREATE FUNCTION pgunit._runwrapped (in_code character varying) RETURNS character varying
AS
$body$
DECLARE
        result VARCHAR;
BEGIN
        -- Execute in a separated transaction, rolled back at the end.
        BEGIN
            result := NULL;
            PERFORM pgunit._exec(in_code);
            RAISE EXCEPTION 'ROLLBACK';
        EXCEPTION
            WHEN raise_exception THEN
                IF SQLERRM <> 'ROLLBACK' THEN
                    result := SQLERRM;
                END IF;
            WHEN others THEN result := SQLERRM;
        END;
        RETURN result;
END;
$body$
    LANGUAGE plpgsql;
--
-- Definition for function testrunner (OID = 2886439) :
--
CREATE FUNCTION pgunit.testrunner (in_wildcard character varying) RETURNS integer
AS
$body$
--
-- ATTENTION!
---
-- Unfortunctely we cannot wrap RAISE NOTICE with a function call,
-- because PostgreSQL adds "CONTEXT:" debug info to the output in case
-- of a notice is generated outside the calling routine.
--
-- Of course, we can say \set verbosity TERSE on the client, but I
-- want this behaviour to be default one...
--
DECLARE
    result VARCHAR;
    result_test VARCHAR;
    tests pgunit.testfunc[];
    errors pgunit.testfunc[];
    test pgunit.testfunc;
    all_start TIMESTAMP;
    all_dt INTERVAL;
    test_start timestamp;
    test_dt INTEGER;
    cont VARCHAR;
    cont_setup VARCHAR;
    failcnt INTEGER;
    testcnt INTEGER;

BEGIN
    SET client_min_messages TO 'NOTICE';
    RAISE NOTICE 'PGUnit % by Dmitry Koterov.', pgunit.version();
    RAISE NOTICE '';
    all_start := clock_timestamp();

    -- Collect all tests.
    tests := pgunit.testcollect(in_wildcard);

    -- First iteration level: unique containers.
    errors := '{}';
    FOR cont, cont_setup IN
        SELECT DISTINCT tests[i].container AS c, tests[i].setup AS s
        FROM generate_series(array_lower(tests, 1), array_upper(tests, 1)) AS s(i)
        ORDER BY c
    LOOP
        RAISE NOTICE '%', cont;
        result := NULL;

        -- Note that we CANNOT generate NOTICEs with no context litter from
        -- within a nested routine. :-(
        BEGIN
            -- Run the setUp block once per group. We cannot use _runwrapped()
            -- here, because we want the effect of running to be persistend
            -- within this transaction.
            IF COALESCE(cont_setup, '') <> '' THEN
                test_start := clock_timestamp();
      	        SET client_min_messages TO 'WARNING'; -- avoid possible function's notices
                PERFORM pgunit._exec(cont_setup);
                SET client_min_messages TO 'NOTICE';
                test_dt := extract(millisecond FROM date_trunc('millisecond', clock_timestamp() - test_start));
                RAISE NOTICE '  - OK % (% ms)', 'setUp', test_dt;
            END IF;

            -- Run all nested tests.
            FOR i IN array_lower(tests, 1) .. array_upper(tests, 1) LOOP
                test := tests[i];
                CONTINUE WHEN test.container <> cont;
                -- Run the test body within a nested transaction and rollback
                -- the result effect of that nested transaction only. So, we
                -- could use the same setUp effect with different non-intersected
                -- test functions!
                test_start := clock_timestamp();
                SET client_min_messages TO 'WARNING';
                result_test := pgunit._runwrapped(test.func);
                SET client_min_messages TO 'NOTICE';
                test_dt := extract(millisecond FROM date_trunc('millisecond', clock_timestamp() - test_start));
                IF result_test IS NULL THEN
                    RAISE NOTICE '  - OK % (% ms)', test.name, test_dt;
                ELSE
                    RAISE NOTICE '  ! FAIL % (% ms)', test.name, test_dt;
                    test.error := result_test;
                    errors := errors || test;
                END IF;
            END LOOP;

            -- All done, rollback the transaction.
            SET client_min_messages TO 'ERROR'; -- avoid "did not find subXID %u in MyProc" message
            RAISE EXCEPTION 'ROLLBACK';
        EXCEPTION
            WHEN raise_exception THEN
                IF SQLERRM <> 'ROLLBACK' THEN
                    result := SQLERRM;
                END IF;
            WHEN others THEN result := SQLERRM;
        END;
        SET client_min_messages TO 'NOTICE';

        -- We are here if setUp block is failed.
        IF result IS NOT NULL THEN
            test_dt := extract(millisecond FROM date_trunc('millisecond', clock_timestamp() - test_start));
            RAISE NOTICE '  ! FAIL % (% ms)', 'setUp', test_dt;
            test.container := cont;
            test.name := 'setUp';
            test.setup := cont_setup;
            test.func := NULL;
            test.error := result;
            errors := errors || test;
        END IF;
    END LOOP;

    -- Display errors one by one.
    all_dt := date_trunc('second', clock_timestamp() - all_start);
    failcnt := COALESCE(array_upper(errors, 1), 0);
    testcnt := COALESCE(array_upper(tests, 1), 0);
    IF testcnt > 0 THEN
        RAISE NOTICE '';
    END IF;
    RAISE NOTICE 'Time: %', all_dt;
    RAISE NOTICE '';
    IF failcnt > 0 THEN
        RAISE NOTICE 'There was % failure%', failcnt, CASE WHEN failcnt > 1 THEN 's' ELSE '' END;
        RAISE NOTICE '';
        FOR i IN array_lower(errors, 1) .. array_upper(errors, 1) LOOP
            test := errors[i];
            IF test.error IS NOT NULL THEN
                RAISE NOTICE '%) %(%)', i, test.name, test.container;
                RAISE NOTICE '%', regexp_replace(test.error, E'(?!\\A)^', 'NOTICE:  ', 'mg');
                RAISE NOTICE '';
            END IF;
        END LOOP;
        RAISE NOTICE 'FAILURES!';
        RAISE NOTICE 'Tests: %, Failures: %.', testcnt, failcnt;
    ELSE
        RAISE NOTICE 'OK (% tests)', testcnt;
    END IF;

    RETURN failcnt;
END;
$body$
    LANGUAGE plpgsql;
--
-- Definition for function test_sample (OID = 2901339) :
--
CREATE FUNCTION pgunit.test_sample () RETURNS testfunc[]
AS
$body$
SELECT pgunit.testcase(
    $setUp$
        -- setUp code is executed before ANY test function code (see below).
        -- Effect of this execution is persistent only during the code
        -- block execution and rolled back after the test is finished.
        CREATE TABLE tst(id INTEGER);
    $setUp$,
    ARRAY[
        -- This is a first test function code. Just check if we can insert
        -- into the table created in setUp.
        'first test: insert is okay', $sql$
            INSERT INTO tst VALUES(1);
            PERFORM pgunit.assert_same(1, (SELECT * FROM tst));
        $sql$,

        -- This is a second test function code.
        -- Illustrates that the effect of the first function is not visible.
        'second test: effect of previous function is not visible here', $sql$
            PERFORM pgunit.assert_same(NULL, (SELECT * FROM tst));
        $sql$,

        -- This is a third test function code. Illustrate that we may use DECLARE.
        'first test: you may use DECLARE in tests', $sql$
            DECLARE
                i INTEGER;
            BEGIN
                FOR i IN 1 .. 10 LOOP
                    INSERT INTO tst VALUES(i);
                    PERFORM pgunit.assert_same(i, (SELECT * FROM tst WHERE id = i));
                END LOOP;
            END;
        $sql$
    ]
);
$body$
    LANGUAGE sql;
--
-- Definition for function test_runtest (OID = 2902622) :
--
CREATE FUNCTION pgunit.test_runtest () RETURNS testfunc[]
AS
$body$
SELECT pgunit.testcase(
    $setUp$
        CREATE TEMPORARY SEQUENCE t2;
    $setUp$,
    ARRAY[
        'executed', $sql$
            DECLARE
                error VARCHAR;
            BEGIN
                error := pgunit._runwrapped('IF nextval(''t2'') > 0 THEN RETURN; END IF;');
                PERFORM pgunit.assert_same(1::BIGINT, (SELECT last_value FROM t2), 'calling func body');
                PERFORM pgunit.assert_same(NULL, error, 'returned error text');
            END;
        $sql$,

        'exception hook', $sql$
            DECLARE
                error VARCHAR;
            BEGIN
                error := pgunit._runwrapped('RAISE EXCEPTION ''test'';');
                PERFORM pgunit.assert_same('test', error);
            END;
        $sql$
    ]
);
$body$
    LANGUAGE sql;
--
-- Definition for function test_exec (OID = 2903048) :
--
CREATE FUNCTION pgunit.test_exec () RETURNS testfunc[]
AS
$body$
SELECT pgunit.testcase(
    $setUp$
        CREATE TEMPORARY TABLE exec(id INTEGER);
    $setUp$,
    ARRAY[
        'executed', $sql$
            PERFORM pgunit._exec('IF 1 = 1 THEN INSERT INTO exec VALUES(1); END IF;');
            PERFORM pgunit.assert_same(1, (SELECT * FROM exec));
        $sql$
    ]
);
$body$
    LANGUAGE sql;