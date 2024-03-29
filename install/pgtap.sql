-- This file defines pgTAP, a collection of functions for TAP-based unit
-- testing. It is distributed under the revised FreeBSD license. You can
-- find the original here:
--
-- http://github.com/theory/pgtap/raw/master/pgtap.sql.in
--
-- The home page for the pgTAP project is:
--
-- http://pgtap.org/

-- ## CREATE SCHEMA TAPSCHEMA;
-- ## SET search_path TO TAPSCHEMA, public;
CREATE SCHEMA pgtap AUTHORIZATION deska_admin;
SET search_path TO pgtap, public;

CREATE OR REPLACE FUNCTION pg_version()
RETURNS text AS 'SELECT current_setting(''server_version'')'
LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION pg_version_num()
RETURNS integer AS $$
    SELECT s.a[1]::int * 10000
           + COALESCE(substring(s.a[2] FROM '[[:digit:]]+')::int, 0) * 100
           + COALESCE(substring(s.a[3] FROM '[[:digit:]]+')::int, 0)
      FROM (
          SELECT string_to_array(current_setting('server_version'), '.') AS a
      ) AS s;
$$ LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION os_name()
RETURNS TEXT AS 'SELECT ''linux''::text;'
LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION pgtap_version()
RETURNS NUMERIC AS 'SELECT 0.25;'
LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION plan( integer )
RETURNS TEXT AS $$
DECLARE
    rcount INTEGER;
BEGIN
    BEGIN
        EXECUTE '
            CREATE TEMP SEQUENCE __tcache___id_seq;
            CREATE TEMP TABLE __tcache__ (
                id    INTEGER NOT NULL DEFAULT nextval(''__tcache___id_seq''),
                label TEXT    NOT NULL,
                value INTEGER NOT NULL,
                note  TEXT    NOT NULL DEFAULT ''''
            );
            CREATE UNIQUE INDEX __tcache___key ON __tcache__(id);
            GRANT ALL ON TABLE __tcache__ TO PUBLIC;
            GRANT ALL ON TABLE __tcache___id_seq TO PUBLIC;

            CREATE TEMP SEQUENCE __tresults___numb_seq;
            CREATE TEMP TABLE __tresults__ (
                numb   INTEGER NOT NULL DEFAULT nextval(''__tresults___numb_seq''),
                ok     BOOLEAN NOT NULL DEFAULT TRUE,
                aok    BOOLEAN NOT NULL DEFAULT TRUE,
                descr  TEXT    NOT NULL DEFAULT '''',
                type   TEXT    NOT NULL DEFAULT '''',
                reason TEXT    NOT NULL DEFAULT ''''
            );
            CREATE UNIQUE INDEX __tresults___key ON __tresults__(numb);
            GRANT ALL ON TABLE __tresults__ TO PUBLIC;
            GRANT ALL ON TABLE __tresults___numb_seq TO PUBLIC;
        ';

    EXCEPTION WHEN duplicate_table THEN
        -- Raise an exception if there's already a plan.
        EXECUTE 'SELECT TRUE FROM __tcache__ WHERE label = ''plan''';
      GET DIAGNOSTICS rcount = ROW_COUNT;
        IF rcount > 0 THEN
           RAISE EXCEPTION 'You tried to plan twice!';
        END IF;
    END;

    -- Save the plan and return.
    PERFORM _set('plan', $1 );
    RETURN '1..' || $1;
END;
$$ LANGUAGE plpgsql strict;

CREATE OR REPLACE FUNCTION no_plan()
RETURNS SETOF boolean AS $$
BEGIN
    PERFORM plan(0);
    RETURN;
END;
$$ LANGUAGE plpgsql strict;

CREATE OR REPLACE FUNCTION _get ( text )
RETURNS integer AS $$
DECLARE
    ret integer;
BEGIN
    EXECUTE 'SELECT value FROM __tcache__ WHERE label = ' || quote_literal($1) || ' LIMIT 1' INTO ret;
    RETURN ret;
END;
$$ LANGUAGE plpgsql strict;

CREATE OR REPLACE FUNCTION _get_latest ( text )
RETURNS integer[] AS $$
DECLARE
    ret integer[];
BEGIN
    EXECUTE 'SELECT ARRAY[ id, value] FROM __tcache__ WHERE label = ' ||
    quote_literal($1) || ' AND id = (SELECT MAX(id) FROM __tcache__ WHERE label = ' ||
    quote_literal($1) || ') LIMIT 1' INTO ret;
    RETURN ret;
END;
$$ LANGUAGE plpgsql strict;

CREATE OR REPLACE FUNCTION _get_latest ( text, integer )
RETURNS integer AS $$
DECLARE
    ret integer;
BEGIN
    EXECUTE 'SELECT MAX(id) FROM __tcache__ WHERE label = ' ||
    quote_literal($1) || ' AND value = ' || $2 INTO ret;
    RETURN ret;
END;
$$ LANGUAGE plpgsql strict;

CREATE OR REPLACE FUNCTION _get_note ( text )
RETURNS text AS $$
DECLARE
    ret text;
BEGIN
    EXECUTE 'SELECT note FROM __tcache__ WHERE label = ' || quote_literal($1) || ' LIMIT 1' INTO ret;
    RETURN ret;
END;
$$ LANGUAGE plpgsql strict;

CREATE OR REPLACE FUNCTION _get_note ( integer )
RETURNS text AS $$
DECLARE
    ret text;
BEGIN
    EXECUTE 'SELECT note FROM __tcache__ WHERE id = ' || $1 || ' LIMIT 1' INTO ret;
    RETURN ret;
END;
$$ LANGUAGE plpgsql strict;

CREATE OR REPLACE FUNCTION _set ( text, integer, text )
RETURNS integer AS $$
DECLARE
    rcount integer;
BEGIN
    EXECUTE 'UPDATE __tcache__ SET value = ' || $2
        || CASE WHEN $3 IS NULL THEN '' ELSE ', note = ' || quote_literal($3) END
        || ' WHERE label = ' || quote_literal($1);
    GET DIAGNOSTICS rcount = ROW_COUNT;
    IF rcount = 0 THEN
       RETURN _add( $1, $2, $3 );
    END IF;
    RETURN $2;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION _set ( text, integer )
RETURNS integer AS $$
    SELECT _set($1, $2, '')
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _set ( integer, integer )
RETURNS integer AS $$
BEGIN
    EXECUTE 'UPDATE __tcache__ SET value = ' || $2
        || ' WHERE id = ' || $1;
    RETURN $2;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION _add ( text, integer, text )
RETURNS integer AS $$
BEGIN
    EXECUTE 'INSERT INTO __tcache__ (label, value, note) values (' ||
    quote_literal($1) || ', ' || $2 || ', ' || quote_literal(COALESCE($3, '')) || ')';
    RETURN $2;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION _add ( text, integer )
RETURNS integer AS $$
    SELECT _add($1, $2, '')
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION add_result ( bool, bool, text, text, text )
RETURNS integer AS $$
BEGIN
    EXECUTE 'INSERT INTO __tresults__ ( ok, aok, descr, type, reason )
    VALUES( ' || $1 || ', '
              || $2 || ', '
              || quote_literal(COALESCE($3, '')) || ', '
              || quote_literal($4) || ', '
              || quote_literal($5) || ' )';
    RETURN currval('__tresults___numb_seq');
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION num_failed ()
RETURNS INTEGER AS $$
DECLARE
    ret integer;
BEGIN
    EXECUTE 'SELECT COUNT(*)::INTEGER FROM __tresults__ WHERE ok = FALSE' INTO ret;
    RETURN ret;
END;
$$ LANGUAGE plpgsql strict;

CREATE OR REPLACE FUNCTION _finish ( INTEGER, INTEGER, INTEGER)
RETURNS SETOF TEXT AS $$
DECLARE
    curr_test ALIAS FOR $1;
    exp_tests INTEGER := $2;
    num_faild ALIAS FOR $3;
    plural    CHAR;
BEGIN
    plural    := CASE exp_tests WHEN 1 THEN '' ELSE 's' END;

    IF curr_test IS NULL THEN
        RAISE EXCEPTION '# No tests run!';
    END IF;

    IF exp_tests = 0 OR exp_tests IS NULL THEN
         -- No plan. Output one now.
        exp_tests = curr_test;
        RETURN NEXT '1..' || exp_tests;
    END IF;

    IF curr_test <> exp_tests THEN
        RETURN NEXT diag(
            'Looks like you planned ' || exp_tests || ' test' ||
            plural || ' but ran ' || curr_test
        );
    ELSIF num_faild > 0 THEN
        RETURN NEXT diag(
            'Looks like you failed ' || num_faild || ' test' ||
            CASE num_faild WHEN 1 THEN '' ELSE 's' END
            || ' of ' || exp_tests
        );
    ELSE

    END IF;
    RETURN;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION finish ()
RETURNS SETOF TEXT AS $$
    SELECT * FROM _finish(
        _get('curr_test'),
        _get('plan'),
        num_failed()
    );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION diag ( msg text )
RETURNS TEXT AS $$
    SELECT '# ' || replace(
       replace(
            replace( $1, E'\r\n', E'\n# ' ),
            E'\n',
            E'\n# '
        ),
        E'\r',
        E'\n# '
    );
$$ LANGUAGE sql strict;

CREATE OR REPLACE FUNCTION diag ( msg anyelement )
RETURNS TEXT AS $$
    SELECT diag($1::text);
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION diag( VARIADIC text[] )
RETURNS TEXT AS $$
    SELECT diag(array_to_string($1, ''));
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION diag( VARIADIC anyarray )
RETURNS TEXT AS $$
    SELECT diag(array_to_string($1, ''));
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION ok ( boolean, text )
RETURNS TEXT AS $$
DECLARE
   aok      ALIAS FOR $1;
   descr    text := $2;
   test_num INTEGER;
   todo_why TEXT;
   ok       BOOL;
BEGIN
   todo_why := _todo();
   ok       := CASE
       WHEN aok = TRUE THEN aok
       WHEN todo_why IS NULL THEN COALESCE(aok, false)
       ELSE TRUE
    END;
    IF _get('plan') IS NULL THEN
        RAISE EXCEPTION 'You tried to run a test without a plan! Gotta have a plan';
    END IF;

    test_num := add_result(
        ok,
        COALESCE(aok, false),
        descr,
        CASE WHEN todo_why IS NULL THEN '' ELSE 'todo' END,
        COALESCE(todo_why, '')
    );

    RETURN (CASE aok WHEN TRUE THEN '' ELSE 'not ' END)
           || 'ok ' || _set( 'curr_test', test_num )
           || CASE descr WHEN '' THEN '' ELSE COALESCE( ' - ' || substr(diag( descr ), 3), '' ) END
           || COALESCE( ' ' || diag( 'TODO ' || todo_why ), '')
           || CASE aok WHEN TRUE THEN '' ELSE E'\n' ||
                diag('Failed ' ||
                CASE WHEN todo_why IS NULL THEN '' ELSE '(TODO) ' END ||
                'test ' || test_num ||
                CASE descr WHEN '' THEN '' ELSE COALESCE(': "' || descr || '"', '') END ) ||
                CASE WHEN aok IS NULL THEN E'\n' || diag('    (test result was NULL)') ELSE '' END
           END;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION ok ( boolean )
RETURNS TEXT AS $$
    SELECT ok( $1, NULL );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION is (anyelement, anyelement, text)
RETURNS TEXT AS $$
DECLARE
    result BOOLEAN;
    output TEXT;
BEGIN
    -- Would prefer $1 IS NOT DISTINCT FROM, but that's not supported by 8.1.
    result := NOT $1 IS DISTINCT FROM $2;
    output := ok( result, $3 );
    RETURN output || CASE result WHEN TRUE THEN '' ELSE E'\n' || diag(
           '        have: ' || CASE WHEN $1 IS NULL THEN 'NULL' ELSE $1::text END ||
        E'\n        want: ' || CASE WHEN $2 IS NULL THEN 'NULL' ELSE $2::text END
    ) END;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION is (anyelement, anyelement)
RETURNS TEXT AS $$
    SELECT is( $1, $2, NULL);
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION isnt (anyelement, anyelement, text)
RETURNS TEXT AS $$
DECLARE
    result BOOLEAN;
    output TEXT;
BEGIN
    result := $1 IS DISTINCT FROM $2;
    output := ok( result, $3 );
    RETURN output || CASE result WHEN TRUE THEN '' ELSE E'\n' || diag(
           '        have: ' || COALESCE( $1::text, 'NULL' ) ||
        E'\n        want: anything else'
    ) END;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION isnt (anyelement, anyelement)
RETURNS TEXT AS $$
    SELECT isnt( $1, $2, NULL);
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _alike ( BOOLEAN, ANYELEMENT, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    result ALIAS FOR $1;
    got    ALIAS FOR $2;
    rx     ALIAS FOR $3;
    descr  ALIAS FOR $4;
    output TEXT;
BEGIN
    output := ok( result, descr );
    RETURN output || CASE result WHEN TRUE THEN '' ELSE E'\n' || diag(
           '                  ' || COALESCE( quote_literal(got), 'NULL' ) ||
       E'\n   doesn''t match: ' || COALESCE( quote_literal(rx), 'NULL' )
    ) END;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION matches ( anyelement, text, text )
RETURNS TEXT AS $$
    SELECT _alike( $1 ~ $2, $1, $2, $3 );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION matches ( anyelement, text )
RETURNS TEXT AS $$
    SELECT _alike( $1 ~ $2, $1, $2, NULL );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION imatches ( anyelement, text, text )
RETURNS TEXT AS $$
    SELECT _alike( $1 ~* $2, $1, $2, $3 );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION imatches ( anyelement, text )
RETURNS TEXT AS $$
    SELECT _alike( $1 ~* $2, $1, $2, NULL );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION alike ( anyelement, text, text )
RETURNS TEXT AS $$
    SELECT _alike( $1 ~~ $2, $1, $2, $3 );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION alike ( anyelement, text )
RETURNS TEXT AS $$
    SELECT _alike( $1 ~~ $2, $1, $2, NULL );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION ialike ( anyelement, text, text )
RETURNS TEXT AS $$
    SELECT _alike( $1 ~~* $2, $1, $2, $3 );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION ialike ( anyelement, text )
RETURNS TEXT AS $$
    SELECT _alike( $1 ~~* $2, $1, $2, NULL );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _unalike ( BOOLEAN, ANYELEMENT, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    result ALIAS FOR $1;
    got    ALIAS FOR $2;
    rx     ALIAS FOR $3;
    descr  ALIAS FOR $4;
    output TEXT;
BEGIN
    output := ok( result, descr );
    RETURN output || CASE result WHEN TRUE THEN '' ELSE E'\n' || diag(
           '                  ' || COALESCE( quote_literal(got), 'NULL' ) ||
        E'\n         matches: ' || COALESCE( quote_literal(rx), 'NULL' )
    ) END;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION doesnt_match ( anyelement, text, text )
RETURNS TEXT AS $$
    SELECT _unalike( $1 !~ $2, $1, $2, $3 );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION doesnt_match ( anyelement, text )
RETURNS TEXT AS $$
    SELECT _unalike( $1 !~ $2, $1, $2, NULL );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION doesnt_imatch ( anyelement, text, text )
RETURNS TEXT AS $$
    SELECT _unalike( $1 !~* $2, $1, $2, $3 );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION doesnt_imatch ( anyelement, text )
RETURNS TEXT AS $$
    SELECT _unalike( $1 !~* $2, $1, $2, NULL );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION unalike ( anyelement, text, text )
RETURNS TEXT AS $$
    SELECT _unalike( $1 !~~ $2, $1, $2, $3 );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION unalike ( anyelement, text )
RETURNS TEXT AS $$
    SELECT _unalike( $1 !~~ $2, $1, $2, NULL );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION unialike ( anyelement, text, text )
RETURNS TEXT AS $$
    SELECT _unalike( $1 !~~* $2, $1, $2, $3 );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION unialike ( anyelement, text )
RETURNS TEXT AS $$
    SELECT _unalike( $1 !~~* $2, $1, $2, NULL );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION cmp_ok (anyelement, text, anyelement, text)
RETURNS TEXT AS $$
DECLARE
    have   ALIAS FOR $1;
    op     ALIAS FOR $2;
    want   ALIAS FOR $3;
    descr  ALIAS FOR $4;
    result BOOLEAN;
    output TEXT;
BEGIN
    EXECUTE 'SELECT ' ||
            COALESCE(quote_literal( have ), 'NULL') || '::' || pg_typeof(have) || ' '
            || op || ' ' ||
            COALESCE(quote_literal( want ), 'NULL') || '::' || pg_typeof(want)
       INTO result;
    output := ok( COALESCE(result, FALSE), descr );
    RETURN output || CASE result WHEN TRUE THEN '' ELSE E'\n' || diag(
           '    ' || COALESCE( quote_literal(have), 'NULL' ) ||
           E'\n        ' || op ||
           E'\n    ' || COALESCE( quote_literal(want), 'NULL' )
    ) END;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION cmp_ok (anyelement, text, anyelement)
RETURNS TEXT AS $$
    SELECT cmp_ok( $1, $2, $3, NULL );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION pass ( text )
RETURNS TEXT AS $$
    SELECT ok( TRUE, $1 );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION pass ()
RETURNS TEXT AS $$
    SELECT ok( TRUE, NULL );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION fail ( text )
RETURNS TEXT AS $$
    SELECT ok( FALSE, $1 );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION fail ()
RETURNS TEXT AS $$
    SELECT ok( FALSE, NULL );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION todo ( why text, how_many int )
RETURNS SETOF BOOLEAN AS $$
BEGIN
    PERFORM _add('todo', COALESCE(how_many, 1), COALESCE(why, ''));
    RETURN;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION todo ( how_many int, why text )
RETURNS SETOF BOOLEAN AS $$
BEGIN
    PERFORM _add('todo', COALESCE(how_many, 1), COALESCE(why, ''));
    RETURN;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION todo ( why text )
RETURNS SETOF BOOLEAN AS $$
BEGIN
    PERFORM _add('todo', 1, COALESCE(why, ''));
    RETURN;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION todo ( how_many int )
RETURNS SETOF BOOLEAN AS $$
BEGIN
    PERFORM _add('todo', COALESCE(how_many, 1), '');
    RETURN;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION todo_start (text)
RETURNS SETOF BOOLEAN AS $$
BEGIN
    PERFORM _add('todo', -1, COALESCE($1, ''));
    RETURN;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION todo_start ()
RETURNS SETOF BOOLEAN AS $$
BEGIN
    PERFORM _add('todo', -1, '');
    RETURN;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION in_todo ()
RETURNS BOOLEAN AS $$
DECLARE
    todos integer;
BEGIN
    todos := _get('todo');
    RETURN CASE WHEN todos IS NULL THEN FALSE ELSE TRUE END;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION todo_end ()
RETURNS SETOF BOOLEAN AS $$
DECLARE
    id integer;
BEGIN
    id := _get_latest( 'todo', -1 );
    IF id IS NULL THEN
        RAISE EXCEPTION 'todo_end() called without todo_start()';
    END IF;
    EXECUTE 'DELETE FROM __tcache__ WHERE id = ' || id;
    RETURN;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION _todo()
RETURNS TEXT AS $$
DECLARE
    todos INT[];
    note text;
BEGIN
    -- Get the latest id and value, because todo() might have been called
    -- again before the todos ran out for the first call to todo(). This
    -- allows them to nest.
    todos := _get_latest('todo');
    IF todos IS NULL THEN
        -- No todos.
        RETURN NULL;
    END IF;
    IF todos[2] = 0 THEN
        -- Todos depleted. Clean up.
        EXECUTE 'DELETE FROM __tcache__ WHERE id = ' || todos[1];
        RETURN NULL;
    END IF;
    -- Decrement the count of counted todos and return the reason.
    IF todos[2] <> -1 THEN
        PERFORM _set(todos[1], todos[2] - 1);
    END IF;
    note := _get_note(todos[1]);

    IF todos[2] = 1 THEN
        -- This was the last todo, so delete the record.
        EXECUTE 'DELETE FROM __tcache__ WHERE id = ' || todos[1];
    END IF;

    RETURN note;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION skip ( why text, how_many int )
RETURNS TEXT AS $$
DECLARE
    output TEXT[];
BEGIN
    output := '{}';
    FOR i IN 1..how_many LOOP
        output = array_append(output, ok( TRUE, 'SKIP: ' || COALESCE( why, '') ) );
    END LOOP;
    RETURN array_to_string(output, E'\n');
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION skip ( text )
RETURNS TEXT AS $$
    SELECT ok( TRUE, 'SKIP: ' || $1 );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION skip( int, text )
RETURNS TEXT AS 'SELECT skip($2, $1)'
LANGUAGE sql;

CREATE OR REPLACE FUNCTION skip( int )
RETURNS TEXT AS 'SELECT skip(NULL, $1)'
LANGUAGE sql;

CREATE OR REPLACE FUNCTION _query( TEXT )
RETURNS TEXT AS $$
    SELECT CASE
        WHEN $1 LIKE '"%' OR $1 !~ '[[:space:]]' THEN 'EXECUTE ' || $1
        ELSE $1
    END;
$$ LANGUAGE SQL;

-- throws_ok ( sql, errcode, errmsg, description )
CREATE OR REPLACE FUNCTION throws_ok ( TEXT, CHAR(5), TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    query     TEXT := _query($1);
    errcode   ALIAS FOR $2;
    errmsg    ALIAS FOR $3;
    desctext  ALIAS FOR $4;
    descr     TEXT;
BEGIN
    descr := COALESCE(
          desctext,
          'threw ' || errcode || ': ' || errmsg,
          'threw ' || errcode,
          'threw ' || errmsg,
          'threw an exception'
    );
    EXECUTE query;
    RETURN ok( FALSE, descr ) || E'\n' || diag(
           '      caught: no exception' ||
        E'\n      wanted: ' || COALESCE( errcode, 'an exception' )
    );
EXCEPTION WHEN OTHERS THEN
    IF (errcode IS NULL OR SQLSTATE = errcode)
        AND ( errmsg IS NULL OR SQLERRM = errmsg)
    THEN
        -- The expected errcode and/or message was thrown.
        RETURN ok( TRUE, descr );
    ELSE
        -- This was not the expected errcode or errmsg.
        RETURN ok( FALSE, descr ) || E'\n' || diag(
               '      caught: ' || SQLSTATE || ': ' || SQLERRM ||
            E'\n      wanted: ' || COALESCE( errcode, 'an exception' ) ||
            COALESCE( ': ' || errmsg, '')
        );
    END IF;
END;
$$ LANGUAGE plpgsql;

-- throws_ok ( sql, errcode, errmsg )
-- throws_ok ( sql, errmsg, description )
CREATE OR REPLACE FUNCTION throws_ok ( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
BEGIN
    IF octet_length($2) = 5 THEN
        RETURN throws_ok( $1, $2::char(5), $3, NULL );
    ELSE
        RETURN throws_ok( $1, NULL, $2, $3 );
    END IF;
END;
$$ LANGUAGE plpgsql;

-- throws_ok ( query, errcode )
-- throws_ok ( query, errmsg )
CREATE OR REPLACE FUNCTION throws_ok ( TEXT, TEXT )
RETURNS TEXT AS $$
BEGIN
    IF octet_length($2) = 5 THEN
        RETURN throws_ok( $1, $2::char(5), NULL, NULL );
    ELSE
        RETURN throws_ok( $1, NULL, $2, NULL );
    END IF;
END;
$$ LANGUAGE plpgsql;

-- throws_ok ( sql )
CREATE OR REPLACE FUNCTION throws_ok ( TEXT )
RETURNS TEXT AS $$
    SELECT throws_ok( $1, NULL, NULL, NULL );
$$ LANGUAGE SQL;

-- Magically cast integer error codes.
-- throws_ok ( sql, errcode, errmsg, description )
CREATE OR REPLACE FUNCTION throws_ok ( TEXT, int4, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT throws_ok( $1, $2::char(5), $3, $4 );
$$ LANGUAGE SQL;

-- throws_ok ( sql, errcode, errmsg )
CREATE OR REPLACE FUNCTION throws_ok ( TEXT, int4, TEXT )
RETURNS TEXT AS $$
    SELECT throws_ok( $1, $2::char(5), $3, NULL );
$$ LANGUAGE SQL;

-- throws_ok ( sql, errcode )
CREATE OR REPLACE FUNCTION throws_ok ( TEXT, int4 )
RETURNS TEXT AS $$
    SELECT throws_ok( $1, $2::char(5), NULL, NULL );
$$ LANGUAGE SQL;

-- lives_ok( sql, description )
CREATE OR REPLACE FUNCTION lives_ok ( TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    code  TEXT := _query($1);
    descr ALIAS FOR $2;
BEGIN
    EXECUTE code;
    RETURN ok( TRUE, descr );
EXCEPTION WHEN OTHERS THEN
    -- There should have been no exception.
    RETURN ok( FALSE, descr ) || E'\n' || diag(
           '        died: ' || SQLSTATE || ': ' || SQLERRM
    );
END;
$$ LANGUAGE plpgsql;

-- lives_ok( sql )
CREATE OR REPLACE FUNCTION lives_ok ( TEXT )
RETURNS TEXT AS $$
    SELECT lives_ok( $1, NULL );
$$ LANGUAGE SQL;

-- performs_ok ( sql, milliseconds, description )
CREATE OR REPLACE FUNCTION performs_ok ( TEXT, NUMERIC, TEXT )
RETURNS TEXT AS $$
DECLARE
    query     TEXT := _query($1);
    max_time  ALIAS FOR $2;
    descr     ALIAS FOR $3;
    starts_at TEXT;
    act_time  NUMERIC;
BEGIN
    starts_at := timeofday();
    EXECUTE query;
    act_time := extract( millisecond from timeofday()::timestamptz - starts_at::timestamptz);
    IF act_time < max_time THEN RETURN ok(TRUE, descr); END IF;
    RETURN ok( FALSE, descr ) || E'\n' || diag(
           '      runtime: ' || act_time || ' ms' ||
        E'\n      exceeds: ' || max_time || ' ms'
    );
END;
$$ LANGUAGE plpgsql;

-- performs_ok ( sql, milliseconds )
CREATE OR REPLACE FUNCTION performs_ok ( TEXT, NUMERIC )
RETURNS TEXT AS $$
    SELECT performs_ok(
        $1, $2, 'Should run in less than ' || $2 || ' ms'
    );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _rexists ( CHAR, NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT true
          FROM pg_catalog.pg_namespace n
          JOIN pg_catalog.pg_class c ON n.oid = c.relnamespace
         WHERE c.relkind = $1
           AND n.nspname = $2
           AND c.relname = $3
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _rexists ( CHAR, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT true
          FROM pg_catalog.pg_class c
         WHERE c.relkind = $1
           AND pg_catalog.pg_table_is_visible(c.oid)
           AND c.relname = $2
    );
$$ LANGUAGE SQL;

-- has_table( schema, table, description )
CREATE OR REPLACE FUNCTION has_table ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _rexists( 'r', $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- has_table( table, description )
CREATE OR REPLACE FUNCTION has_table ( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _rexists( 'r', $1 ), $2 );
$$ LANGUAGE SQL;

-- has_table( table )
CREATE OR REPLACE FUNCTION has_table ( NAME )
RETURNS TEXT AS $$
    SELECT has_table( $1, 'Table ' || quote_ident($1) || ' should exist' );
$$ LANGUAGE SQL;

-- hasnt_table( schema, table, description )
CREATE OR REPLACE FUNCTION hasnt_table ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _rexists( 'r', $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- hasnt_table( table, description )
CREATE OR REPLACE FUNCTION hasnt_table ( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _rexists( 'r', $1 ), $2 );
$$ LANGUAGE SQL;

-- hasnt_table( table )
CREATE OR REPLACE FUNCTION hasnt_table ( NAME )
RETURNS TEXT AS $$
    SELECT hasnt_table( $1, 'Table ' || quote_ident($1) || ' should not exist' );
$$ LANGUAGE SQL;

-- has_view( schema, view, description )
CREATE OR REPLACE FUNCTION has_view ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _rexists( 'v', $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- has_view( view, description )
CREATE OR REPLACE FUNCTION has_view ( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _rexists( 'v', $1 ), $2 );
$$ LANGUAGE SQL;

-- has_view( view )
CREATE OR REPLACE FUNCTION has_view ( NAME )
RETURNS TEXT AS $$
    SELECT has_view( $1, 'View ' || quote_ident($1) || ' should exist' );
$$ LANGUAGE SQL;

-- hasnt_view( schema, view, description )
CREATE OR REPLACE FUNCTION hasnt_view ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _rexists( 'v', $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- hasnt_view( view, description )
CREATE OR REPLACE FUNCTION hasnt_view ( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _rexists( 'v', $1 ), $2 );
$$ LANGUAGE SQL;

-- hasnt_view( view )
CREATE OR REPLACE FUNCTION hasnt_view ( NAME )
RETURNS TEXT AS $$
    SELECT hasnt_view( $1, 'View ' || quote_ident($1) || ' should not exist' );
$$ LANGUAGE SQL;

-- has_sequence( schema, sequence, description )
CREATE OR REPLACE FUNCTION has_sequence ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _rexists( 'S', $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- has_sequence( sequence, description )
CREATE OR REPLACE FUNCTION has_sequence ( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _rexists( 'S', $1 ), $2 );
$$ LANGUAGE SQL;

-- has_sequence( sequence )
CREATE OR REPLACE FUNCTION has_sequence ( NAME )
RETURNS TEXT AS $$
    SELECT has_sequence( $1, 'Sequence ' || quote_ident($1) || ' should exist' );
$$ LANGUAGE SQL;

-- hasnt_sequence( schema, sequence, description )
CREATE OR REPLACE FUNCTION hasnt_sequence ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _rexists( 'S', $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- hasnt_sequence( sequence, description )
CREATE OR REPLACE FUNCTION hasnt_sequence ( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _rexists( 'S', $1 ), $2 );
$$ LANGUAGE SQL;

-- hasnt_sequence( sequence )
CREATE OR REPLACE FUNCTION hasnt_sequence ( NAME )
RETURNS TEXT AS $$
    SELECT hasnt_sequence( $1, 'Sequence ' || quote_ident($1) || ' should not exist' );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _cexists ( NAME, NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT true
          FROM pg_catalog.pg_namespace n
          JOIN pg_catalog.pg_class c ON n.oid = c.relnamespace
          JOIN pg_catalog.pg_attribute a ON c.oid = a.attrelid
         WHERE n.nspname = $1
           AND c.relname = $2
           AND a.attnum > 0
           AND NOT a.attisdropped
           AND a.attname = $3
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _cexists ( NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT true
          FROM pg_catalog.pg_class c
          JOIN pg_catalog.pg_attribute a ON c.oid = a.attrelid
         WHERE c.relname = $1
           AND pg_catalog.pg_table_is_visible(c.oid)
           AND a.attnum > 0
           AND NOT a.attisdropped
           AND a.attname = $2
    );
$$ LANGUAGE SQL;

-- has_column( schema, table, column, description )
CREATE OR REPLACE FUNCTION has_column ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _cexists( $1, $2, $3 ), $4 );
$$ LANGUAGE SQL;

-- has_column( table, column, description )
CREATE OR REPLACE FUNCTION has_column ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _cexists( $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- has_column( table, column )
CREATE OR REPLACE FUNCTION has_column ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT has_column( $1, $2, 'Column ' || quote_ident($1) || '.' || quote_ident($2) || ' should exist' );
$$ LANGUAGE SQL;

-- hasnt_column( schema, table, column, description )
CREATE OR REPLACE FUNCTION hasnt_column ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _cexists( $1, $2, $3 ), $4 );
$$ LANGUAGE SQL;

-- hasnt_column( table, column, description )
CREATE OR REPLACE FUNCTION hasnt_column ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _cexists( $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- hasnt_column( table, column )
CREATE OR REPLACE FUNCTION hasnt_column ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT hasnt_column( $1, $2, 'Column ' || quote_ident($1) || '.' || quote_ident($2) || ' should not exist' );
$$ LANGUAGE SQL;

-- _col_is_null( schema, table, column, desc, null )
CREATE OR REPLACE FUNCTION _col_is_null ( NAME, NAME, NAME, TEXT, bool )
RETURNS TEXT AS $$
BEGIN
    IF NOT _cexists( $1, $2, $3 ) THEN
        RETURN fail( $4 ) || E'\n'
            || diag ('    Column ' || quote_ident($1) || '.' || quote_ident($2) || '.' || quote_ident($3) || ' does not exist' );
    END IF;
    RETURN ok(
        EXISTS(
            SELECT true
              FROM pg_catalog.pg_namespace n
              JOIN pg_catalog.pg_class c ON n.oid = c.relnamespace
              JOIN pg_catalog.pg_attribute a ON c.oid = a.attrelid
             WHERE n.nspname = $1
               AND c.relname = $2
               AND a.attnum  > 0
               AND NOT a.attisdropped
               AND a.attname    = $3
               AND a.attnotnull = $5
        ), $4
    );
END;
$$ LANGUAGE plpgsql;

-- _col_is_null( table, column, desc, null )
CREATE OR REPLACE FUNCTION _col_is_null ( NAME, NAME, TEXT, bool )
RETURNS TEXT AS $$
BEGIN
    IF NOT _cexists( $1, $2 ) THEN
        RETURN fail( $3 ) || E'\n'
            || diag ('    Column ' || quote_ident($1) || '.' || quote_ident($2) || ' does not exist' );
    END IF;
    RETURN ok(
        EXISTS(
            SELECT true
              FROM pg_catalog.pg_class c
              JOIN pg_catalog.pg_attribute a ON c.oid = a.attrelid
             WHERE pg_catalog.pg_table_is_visible(c.oid)
               AND c.relname = $1
               AND a.attnum > 0
               AND NOT a.attisdropped
               AND a.attname    = $2
               AND a.attnotnull = $4
        ), $3
    );
END;
$$ LANGUAGE plpgsql;

-- col_not_null( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_not_null ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT _col_is_null( $1, $2, $3, $4, true );
$$ LANGUAGE SQL;

-- col_not_null( table, column, description )
CREATE OR REPLACE FUNCTION col_not_null ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT _col_is_null( $1, $2, $3, true );
$$ LANGUAGE SQL;

-- col_not_null( table, column )
CREATE OR REPLACE FUNCTION col_not_null ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT _col_is_null( $1, $2, 'Column ' || quote_ident($1) || '.' || quote_ident($2) || ' should be NOT NULL', true );
$$ LANGUAGE SQL;

-- col_is_null( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_is_null ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT _col_is_null( $1, $2, $3, $4, false );
$$ LANGUAGE SQL;

-- col_is_null( schema, table, column )
CREATE OR REPLACE FUNCTION col_is_null ( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT _col_is_null( $1, $2, $3, false );
$$ LANGUAGE SQL;

-- col_is_null( table, column )
CREATE OR REPLACE FUNCTION col_is_null ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT _col_is_null( $1, $2, 'Column ' || quote_ident($1) || '.' || quote_ident($2) || ' should allow NULL', false );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION display_type ( OID, INTEGER )
RETURNS TEXT AS $$
    SELECT COALESCE(substring(
        pg_catalog.format_type($1, $2),
        '(("(?!")([^"]|"")+"|[^.]+)([(][^)]+[)])?)$'
    ), '')
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION display_type ( NAME, OID, INTEGER )
RETURNS TEXT AS $$
    SELECT CASE WHEN $1 IS NULL THEN '' ELSE quote_ident($1) || '.' END
        || display_type($2, $3)
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _get_col_type ( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT display_type(a.atttypid, a.atttypmod)
      FROM pg_catalog.pg_namespace n
      JOIN pg_catalog.pg_class c     ON n.oid = c.relnamespace
      JOIN pg_catalog.pg_attribute a ON c.oid = a.attrelid
     WHERE n.nspname = $1
       AND c.relname = $2
       AND a.attname = $3
       AND attnum    > 0
       AND NOT a.attisdropped
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _get_col_type ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT display_type(a.atttypid, a.atttypmod)
      FROM pg_catalog.pg_attribute a
      JOIN pg_catalog.pg_class c ON  a.attrelid = c.oid
     WHERE pg_table_is_visible(c.oid)
       AND c.relname = $1
       AND a.attname = $2
       AND attnum    > 0
       AND NOT a.attisdropped
       AND pg_type_is_visible(a.atttypid)
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _get_col_ns_type ( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT display_type(tn.nspname, a.atttypid, a.atttypmod)
      FROM pg_catalog.pg_namespace n
      JOIN pg_catalog.pg_class c      ON n.oid = c.relnamespace
      JOIN pg_catalog.pg_attribute a  ON c.oid = a.attrelid
      JOIN pg_catalog.pg_type t       ON a.atttypid = t.oid
      JOIN pg_catalog.pg_namespace tn ON t.typnamespace = tn.oid
     WHERE n.nspname = $1
       AND c.relname = $2
       AND a.attname = $3
       AND attnum    > 0
       AND NOT a.attisdropped
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _quote_ident_like(TEXT, TEXT)
RETURNS TEXT AS $$
DECLARE
    have    TEXT;
    pcision TEXT;
BEGIN
    -- Just return it if rhs isn't quoted.
    IF $2 !~ '"' THEN RETURN $1; END IF;

    pcision := substring($1 FROM '[(][^")]+[)]$');

    -- Just quote it if thre is no precision.
    if pcision IS NULL THEN RETURN quote_ident($1); END IF;

    -- Quote the non-precision part and concatenate with precision.
    RETURN quote_ident(substring($1 FROM char_length($1) - char_length(pcision)))
        || pcision;
END;
$$ LANGUAGE plpgsql;

-- col_type_is( schema, table, column, schema, type, description )
CREATE OR REPLACE FUNCTION col_type_is ( NAME, NAME, NAME, NAME, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    have_type TEXT := _get_col_ns_type($1, $2, $3);
    want_type TEXT;
BEGIN
    IF have_type IS NULL THEN
        RETURN fail( $6 ) || E'\n' || diag (
            '   Column ' || COALESCE(quote_ident($1) || '.', '')
            || quote_ident($2) || '.' || quote_ident($3) || ' does not exist'
        );
    END IF;

    want_type := quote_ident($4) || '.' || _quote_ident_like($5, have_type);
    IF have_type = want_type THEN
        -- We're good to go.
        RETURN ok( true, $6 );
    END IF;

    -- Wrong data type. tell 'em what we really got.
    RETURN ok( false, $6 ) || E'\n' || diag(
           '        have: ' || have_type ||
        E'\n        want: ' || want_type
    );
END;
$$ LANGUAGE plpgsql;

-- col_type_is( schema, table, column, schema, type )
CREATE OR REPLACE FUNCTION col_type_is ( NAME, NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_type_is( $1, $2, $3, $4, $5, 'Column ' || quote_ident($1) || '.' || quote_ident($2)
        || '.' || quote_ident($3) || ' should be type ' || quote_ident($4) || '.' || $5);
$$ LANGUAGE SQL;

-- col_type_is( schema, table, column, type, description )
CREATE OR REPLACE FUNCTION col_type_is ( NAME, NAME, NAME, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    have_type TEXT;
    want_type TEXT;
BEGIN
    -- Get the data type.
    IF $1 IS NULL THEN
        have_type := _get_col_type($2, $3);
    ELSE
        have_type := _get_col_type($1, $2, $3);
    END IF;

    IF have_type IS NULL THEN
        RETURN fail( $5 ) || E'\n' || diag (
            '   Column ' || COALESCE(quote_ident($1) || '.', '')
            || quote_ident($2) || '.' || quote_ident($3) || ' does not exist'
        );
    END IF;

    want_type := _quote_ident_like($4, have_type);
    IF have_type = want_type THEN
        -- We're good to go.
        RETURN ok( true, $5 );
    END IF;

    -- Wrong data type. tell 'em what we really got.
    RETURN ok( false, $5 ) || E'\n' || diag(
           '        have: ' || have_type ||
        E'\n        want: ' || want_type
    );
END;
$$ LANGUAGE plpgsql;

-- col_type_is( schema, table, column, type )
CREATE OR REPLACE FUNCTION col_type_is ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_type_is( $1, $2, $3, $4, 'Column ' || quote_ident($1) || '.' || quote_ident($2) || '.' || quote_ident($3) || ' should be type ' || $4 );
$$ LANGUAGE SQL;

-- col_type_is( table, column, type, description )
CREATE OR REPLACE FUNCTION col_type_is ( NAME, NAME, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT col_type_is( NULL, $1, $2, $3, $4 );
$$ LANGUAGE SQL;

-- col_type_is( table, column, type )
CREATE OR REPLACE FUNCTION col_type_is ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_type_is( $1, $2, $3, 'Column ' || quote_ident($1) || '.' || quote_ident($2) || ' should be type ' || $3 );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _has_def ( NAME, NAME, NAME )
RETURNS boolean AS $$
    SELECT a.atthasdef
      FROM pg_catalog.pg_namespace n
      JOIN pg_catalog.pg_class c ON n.oid = c.relnamespace
      JOIN pg_catalog.pg_attribute a ON c.oid = a.attrelid
     WHERE n.nspname = $1
       AND c.relname = $2
       AND a.attnum > 0
       AND NOT a.attisdropped
       AND a.attname = $3
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _has_def ( NAME, NAME )
RETURNS boolean AS $$
    SELECT a.atthasdef
      FROM pg_catalog.pg_class c
      JOIN pg_catalog.pg_attribute a ON c.oid = a.attrelid
     WHERE c.relname = $1
       AND a.attnum > 0
       AND NOT a.attisdropped
       AND a.attname = $2
$$ LANGUAGE sql;

-- col_has_default( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_has_default ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
BEGIN
    IF NOT _cexists( $1, $2, $3 ) THEN
        RETURN fail( $4 ) || E'\n'
            || diag ('    Column ' || quote_ident($1) || '.' || quote_ident($2) || '.' || quote_ident($3) || ' does not exist' );
    END IF;
    RETURN ok( _has_def( $1, $2, $3 ), $4 );
END
$$ LANGUAGE plpgsql;

-- col_has_default( table, column, description )
CREATE OR REPLACE FUNCTION col_has_default ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
BEGIN
    IF NOT _cexists( $1, $2 ) THEN
        RETURN fail( $3 ) || E'\n'
            || diag ('    Column ' || quote_ident($1) || '.' || quote_ident($2) || ' does not exist' );
    END IF;
    RETURN ok( _has_def( $1, $2 ), $3 );
END;
$$ LANGUAGE plpgsql;

-- col_has_default( table, column )
CREATE OR REPLACE FUNCTION col_has_default ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT col_has_default( $1, $2, 'Column ' || quote_ident($1) || '.' || quote_ident($2) || ' should have a default' );
$$ LANGUAGE SQL;

-- col_hasnt_default( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_hasnt_default ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
BEGIN
    IF NOT _cexists( $1, $2, $3 ) THEN
        RETURN fail( $4 ) || E'\n'
            || diag ('    Column ' || quote_ident($1) || '.' || quote_ident($2) || '.' || quote_ident($3) || ' does not exist' );
    END IF;
    RETURN ok( NOT _has_def( $1, $2, $3 ), $4 );
END;
$$ LANGUAGE plpgsql;

-- col_hasnt_default( table, column, description )
CREATE OR REPLACE FUNCTION col_hasnt_default ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
BEGIN
    IF NOT _cexists( $1, $2 ) THEN
        RETURN fail( $3 ) || E'\n'
            || diag ('    Column ' || quote_ident($1) || '.' || quote_ident($2) || ' does not exist' );
    END IF;
    RETURN ok( NOT _has_def( $1, $2 ), $3 );
END;
$$ LANGUAGE plpgsql;

-- col_hasnt_default( table, column )
CREATE OR REPLACE FUNCTION col_hasnt_default ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT col_hasnt_default( $1, $2, 'Column ' || quote_ident($1) || '.' || quote_ident($2) || ' should not have a default' );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _def_is( TEXT, TEXT, anyelement, TEXT )
RETURNS TEXT AS $$
DECLARE
    thing text;
BEGIN
    IF $1 ~ '^[^'']+[(]' THEN
        -- It's a functional default.
        RETURN is( $1, $3, $4 );
    END IF;

    EXECUTE 'SELECT is('
             || COALESCE($1, 'NULL' || '::' || $2) || '::' || $2 || ', '
             || COALESCE(quote_literal($3), 'NULL') || '::' || $2 || ', '
             || COALESCE(quote_literal($4), 'NULL')
    || ')' INTO thing;
    RETURN thing;
END;
$$ LANGUAGE plpgsql;

-- _cdi( schema, table, column, default, description )
CREATE OR REPLACE FUNCTION _cdi ( NAME, NAME, NAME, anyelement, TEXT )
RETURNS TEXT AS $$
BEGIN
    IF NOT _cexists( $1, $2, $3 ) THEN
        RETURN fail( $5 ) || E'\n'
            || diag ('    Column ' || quote_ident($1) || '.' || quote_ident($2) || '.' || quote_ident($3) || ' does not exist' );
    END IF;

    IF NOT _has_def( $1, $2, $3 ) THEN
        RETURN fail( $5 ) || E'\n'
            || diag ('    Column ' || quote_ident($1) || '.' || quote_ident($2) || '.' || quote_ident($3) || ' has no default' );
    END IF;

    RETURN _def_is(
        pg_catalog.pg_get_expr(d.adbin, d.adrelid),
        display_type(a.atttypid, a.atttypmod),
        $4, $5
    )
      FROM pg_catalog.pg_namespace n, pg_catalog.pg_class c, pg_catalog.pg_attribute a,
           pg_catalog.pg_attrdef d
     WHERE n.oid = c.relnamespace
       AND c.oid = a.attrelid
       AND a.atthasdef
       AND a.attrelid = d.adrelid
       AND a.attnum = d.adnum
       AND n.nspname = $1
       AND c.relname = $2
       AND a.attnum > 0
       AND NOT a.attisdropped
       AND a.attname = $3;
END;
$$ LANGUAGE plpgsql;

-- _cdi( table, column, default, description )
CREATE OR REPLACE FUNCTION _cdi ( NAME, NAME, anyelement, TEXT )
RETURNS TEXT AS $$
BEGIN
    IF NOT _cexists( $1, $2 ) THEN
        RETURN fail( $4 ) || E'\n'
            || diag ('    Column ' || quote_ident($1) || '.' || quote_ident($2) || ' does not exist' );
    END IF;

    IF NOT _has_def( $1, $2 ) THEN
        RETURN fail( $4 ) || E'\n'
            || diag ('    Column ' || quote_ident($1) || '.' || quote_ident($2) || ' has no default' );
    END IF;

    RETURN _def_is(
        pg_catalog.pg_get_expr(d.adbin, d.adrelid),
        display_type(a.atttypid, a.atttypmod),
        $3, $4
    )
      FROM pg_catalog.pg_class c, pg_catalog.pg_attribute a, pg_catalog.pg_attrdef d
     WHERE c.oid = a.attrelid
       AND pg_table_is_visible(c.oid)
       AND a.atthasdef
       AND a.attrelid = d.adrelid
       AND a.attnum = d.adnum
       AND c.relname = $1
       AND a.attnum > 0
       AND NOT a.attisdropped
       AND a.attname = $2;
END;
$$ LANGUAGE plpgsql;

-- _cdi( table, column, default )
CREATE OR REPLACE FUNCTION _cdi ( NAME, NAME, anyelement )
RETURNS TEXT AS $$
    SELECT col_default_is(
        $1, $2, $3,
        'Column ' || quote_ident($1) || '.' || quote_ident($2) || ' should default to '
        || COALESCE( quote_literal($3), 'NULL')
    );
$$ LANGUAGE sql;

-- col_default_is( schema, table, column, default, description )
CREATE OR REPLACE FUNCTION col_default_is ( NAME, NAME, NAME, anyelement, TEXT )
RETURNS TEXT AS $$
    SELECT _cdi( $1, $2, $3, $4, $5 );
$$ LANGUAGE sql;

-- col_default_is( schema, table, column, default, description )
CREATE OR REPLACE FUNCTION col_default_is ( NAME, NAME, NAME, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _cdi( $1, $2, $3, $4, $5 );
$$ LANGUAGE sql;

-- col_default_is( table, column, default, description )
CREATE OR REPLACE FUNCTION col_default_is ( NAME, NAME, anyelement, TEXT )
RETURNS TEXT AS $$
    SELECT _cdi( $1, $2, $3, $4 );
$$ LANGUAGE sql;

-- col_default_is( table, column, default, description )
CREATE OR REPLACE FUNCTION col_default_is ( NAME, NAME, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _cdi( $1, $2, $3, $4 );
$$ LANGUAGE sql;

-- col_default_is( table, column, default )
CREATE OR REPLACE FUNCTION col_default_is ( NAME, NAME, anyelement )
RETURNS TEXT AS $$
    SELECT _cdi( $1, $2, $3 );
$$ LANGUAGE sql;

-- col_default_is( table, column, default::text )
CREATE OR REPLACE FUNCTION col_default_is ( NAME, NAME, text )
RETURNS TEXT AS $$
    SELECT _cdi( $1, $2, $3 );
$$ LANGUAGE sql;

-- _hasc( schema, table, constraint_type )
CREATE OR REPLACE FUNCTION _hasc ( NAME, NAME, CHAR )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
            SELECT true
              FROM pg_catalog.pg_namespace n
              JOIN pg_catalog.pg_class c      ON c.relnamespace = n.oid
              JOIN pg_catalog.pg_constraint x ON c.oid = x.conrelid
             WHERE c.relhaspkey = true
               AND n.nspname = $1
               AND c.relname = $2
               AND x.contype = $3
    );
$$ LANGUAGE sql;

-- _hasc( table, constraint_type )
CREATE OR REPLACE FUNCTION _hasc ( NAME, CHAR )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
            SELECT true
              FROM pg_catalog.pg_class c
              JOIN pg_catalog.pg_constraint x ON c.oid = x.conrelid
             WHERE c.relhaspkey = true
               AND pg_table_is_visible(c.oid)
               AND c.relname = $1
               AND x.contype = $2
    );
$$ LANGUAGE sql;

-- has_pk( schema, table, description )
CREATE OR REPLACE FUNCTION has_pk ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _hasc( $1, $2, 'p' ), $3 );
$$ LANGUAGE sql;

-- has_pk( table, description )
CREATE OR REPLACE FUNCTION has_pk ( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _hasc( $1, 'p' ), $2 );
$$ LANGUAGE sql;

-- has_pk( table )
CREATE OR REPLACE FUNCTION has_pk ( NAME )
RETURNS TEXT AS $$
    SELECT has_pk( $1, 'Table ' || quote_ident($1) || ' should have a primary key' );
$$ LANGUAGE sql;

-- hasnt_pk( schema, table, description )
CREATE OR REPLACE FUNCTION hasnt_pk ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _hasc( $1, $2, 'p' ), $3 );
$$ LANGUAGE sql;

-- hasnt_pk( table, description )
CREATE OR REPLACE FUNCTION hasnt_pk ( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _hasc( $1, 'p' ), $2 );
$$ LANGUAGE sql;

-- hasnt_pk( table )
CREATE OR REPLACE FUNCTION hasnt_pk ( NAME )
RETURNS TEXT AS $$
    SELECT hasnt_pk( $1, 'Table ' || quote_ident($1) || ' should not have a primary key' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _ident_array_to_string( name[], text )
RETURNS text AS $$
    SELECT array_to_string(ARRAY(
        SELECT quote_ident($1[i])
          FROM generate_series(1, array_upper($1, 1)) s(i)
         ORDER BY i
    ), $2);
$$ LANGUAGE SQL immutable;

-- Borrowed from newsysviews: http://pgfoundry.org/projects/newsysviews/
CREATE OR REPLACE FUNCTION _pg_sv_column_array( OID, SMALLINT[] )
RETURNS NAME[] AS $$
    SELECT ARRAY(
        SELECT a.attname
          FROM pg_catalog.pg_attribute a
          JOIN generate_series(1, array_upper($2, 1)) s(i) ON a.attnum = $2[i]
         WHERE attrelid = $1
         ORDER BY i
    )
$$ LANGUAGE SQL stable;

-- Borrowed from newsysviews: http://pgfoundry.org/projects/newsysviews/
CREATE OR REPLACE FUNCTION _pg_sv_table_accessible( OID, OID )
RETURNS BOOLEAN AS $$
    SELECT CASE WHEN has_schema_privilege($1, 'USAGE') THEN (
                  has_table_privilege($2, 'SELECT')
               OR has_table_privilege($2, 'INSERT')
               or has_table_privilege($2, 'UPDATE')
               OR has_table_privilege($2, 'DELETE')
               OR has_table_privilege($2, 'RULE')
               OR has_table_privilege($2, 'REFERENCES')
               OR has_table_privilege($2, 'TRIGGER')
           ) ELSE FALSE
    END;
$$ LANGUAGE SQL immutable strict;

-- Borrowed from newsysviews: http://pgfoundry.org/projects/newsysviews/
CREATE OR REPLACE VIEW pg_all_foreign_keys
AS
  SELECT n1.nspname                                   AS fk_schema_name,
         c1.relname                                   AS fk_table_name,
         k1.conname                                   AS fk_constraint_name,
         c1.oid                                       AS fk_table_oid,
         _pg_sv_column_array(k1.conrelid,k1.conkey)   AS fk_columns,
         n2.nspname                                   AS pk_schema_name,
         c2.relname                                   AS pk_table_name,
         k2.conname                                   AS pk_constraint_name,
         c2.oid                                       AS pk_table_oid,
         ci.relname                                   AS pk_index_name,
         _pg_sv_column_array(k1.confrelid,k1.confkey) AS pk_columns,
         CASE k1.confmatchtype WHEN 'f' THEN 'FULL'
                               WHEN 'p' THEN 'PARTIAL'
                               WHEN 'u' THEN 'NONE'
                               else null
         END AS match_type,
         CASE k1.confdeltype WHEN 'a' THEN 'NO ACTION'
                             WHEN 'c' THEN 'CASCADE'
                             WHEN 'd' THEN 'SET DEFAULT'
                             WHEN 'n' THEN 'SET NULL'
                             WHEN 'r' THEN 'RESTRICT'
                             else null
         END AS on_delete,
         CASE k1.confupdtype WHEN 'a' THEN 'NO ACTION'
                             WHEN 'c' THEN 'CASCADE'
                             WHEN 'd' THEN 'SET DEFAULT'
                             WHEN 'n' THEN 'SET NULL'
                             WHEN 'r' THEN 'RESTRICT'
                             ELSE NULL
         END AS on_update,
         k1.condeferrable AS is_deferrable,
         k1.condeferred   AS is_deferred
    FROM pg_catalog.pg_constraint k1
    JOIN pg_catalog.pg_namespace n1 ON (n1.oid = k1.connamespace)
    JOIN pg_catalog.pg_class c1     ON (c1.oid = k1.conrelid)
    JOIN pg_catalog.pg_class c2     ON (c2.oid = k1.confrelid)
    JOIN pg_catalog.pg_namespace n2 ON (n2.oid = c2.relnamespace)
    JOIN pg_catalog.pg_depend d     ON (
                 d.classid = 'pg_constraint'::regclass
             AND d.objid = k1.oid
             AND d.objsubid = 0
             AND d.deptype = 'n'
             AND d.refclassid = 'pg_class'::regclass
             AND d.refobjsubid=0
         )
    JOIN pg_catalog.pg_class ci ON (ci.oid = d.refobjid AND ci.relkind = 'i')
    LEFT JOIN pg_depend d2      ON (
                 d2.classid = 'pg_class'::regclass
             AND d2.objid = ci.oid
             AND d2.objsubid = 0
             AND d2.deptype = 'i'
             AND d2.refclassid = 'pg_constraint'::regclass
             AND d2.refobjsubid = 0
         )
    LEFT JOIN pg_catalog.pg_constraint k2 ON (
                 k2.oid = d2.refobjid
             AND k2.contype IN ('p', 'u')
         )
   WHERE k1.conrelid != 0
     AND k1.confrelid != 0
     AND k1.contype = 'f'
     AND _pg_sv_table_accessible(n1.oid, c1.oid);

-- _keys( schema, table, constraint_type )
CREATE OR REPLACE FUNCTION _keys ( NAME, NAME, CHAR )
RETURNS SETOF NAME[] AS $$
    SELECT _pg_sv_column_array(x.conrelid,x.conkey)
      FROM pg_catalog.pg_namespace n
      JOIN pg_catalog.pg_class c       ON n.oid = c.relnamespace
      JOIN pg_catalog.pg_constraint x  ON c.oid = x.conrelid
     WHERE n.nspname = $1
       AND c.relname = $2
       AND x.contype = $3
$$ LANGUAGE sql;

-- _keys( table, constraint_type )
CREATE OR REPLACE FUNCTION _keys ( NAME, CHAR )
RETURNS SETOF NAME[] AS $$
    SELECT _pg_sv_column_array(x.conrelid,x.conkey)
      FROM pg_catalog.pg_class c
      JOIN pg_catalog.pg_constraint x  ON c.oid = x.conrelid
       AND c.relname = $1
       AND x.contype = $2
$$ LANGUAGE sql;

-- _ckeys( schema, table, constraint_type )
CREATE OR REPLACE FUNCTION _ckeys ( NAME, NAME, CHAR )
RETURNS NAME[] AS $$
    SELECT * FROM _keys($1, $2, $3) LIMIT 1;
$$ LANGUAGE sql;

-- _ckeys( table, constraint_type )
CREATE OR REPLACE FUNCTION _ckeys ( NAME, CHAR )
RETURNS NAME[] AS $$
    SELECT * FROM _keys($1, $2) LIMIT 1;
$$ LANGUAGE sql;

-- col_is_pk( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_is_pk ( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT is( _ckeys( $1, $2, 'p' ), $3, $4 );
$$ LANGUAGE sql;

-- col_is_pk( table, column, description )
CREATE OR REPLACE FUNCTION col_is_pk ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT is( _ckeys( $1, 'p' ), $2, $3 );
$$ LANGUAGE sql;

-- col_is_pk( table, column[] )
CREATE OR REPLACE FUNCTION col_is_pk ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT col_is_pk( $1, $2, 'Columns ' || quote_ident($1) || '(' || _ident_array_to_string($2, ', ') || ') should be a primary key' );
$$ LANGUAGE sql;

-- col_is_pk( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_is_pk ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_is_pk( $1, $2, ARRAY[$3], $4 );
$$ LANGUAGE sql;

-- col_is_pk( table, column, description )
CREATE OR REPLACE FUNCTION col_is_pk ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_is_pk( $1, ARRAY[$2], $3 );
$$ LANGUAGE sql;

-- col_is_pk( table, column )
CREATE OR REPLACE FUNCTION col_is_pk ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT col_is_pk( $1, $2, 'Column ' || quote_ident($1) || '(' || quote_ident($2) || ') should be a primary key' );
$$ LANGUAGE sql;

-- col_isnt_pk( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_isnt_pk ( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT isnt( _ckeys( $1, $2, 'p' ), $3, $4 );
$$ LANGUAGE sql;

-- col_isnt_pk( table, column, description )
CREATE OR REPLACE FUNCTION col_isnt_pk ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT isnt( _ckeys( $1, 'p' ), $2, $3 );
$$ LANGUAGE sql;

-- col_isnt_pk( table, column[] )
CREATE OR REPLACE FUNCTION col_isnt_pk ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT col_isnt_pk( $1, $2, 'Columns ' || quote_ident($1) || '(' || _ident_array_to_string($2, ', ') || ') should not be a primary key' );
$$ LANGUAGE sql;

-- col_isnt_pk( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_isnt_pk ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_isnt_pk( $1, $2, ARRAY[$3], $4 );
$$ LANGUAGE sql;

-- col_isnt_pk( table, column, description )
CREATE OR REPLACE FUNCTION col_isnt_pk ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_isnt_pk( $1, ARRAY[$2], $3 );
$$ LANGUAGE sql;

-- col_isnt_pk( table, column )
CREATE OR REPLACE FUNCTION col_isnt_pk ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT col_isnt_pk( $1, $2, 'Column ' || quote_ident($1) || '(' || quote_ident($2) || ') should not be a primary key' );
$$ LANGUAGE sql;

-- has_fk( schema, table, description )
CREATE OR REPLACE FUNCTION has_fk ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _hasc( $1, $2, 'f' ), $3 );
$$ LANGUAGE sql;

-- has_fk( table, description )
CREATE OR REPLACE FUNCTION has_fk ( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _hasc( $1, 'f' ), $2 );
$$ LANGUAGE sql;

-- has_fk( table )
CREATE OR REPLACE FUNCTION has_fk ( NAME )
RETURNS TEXT AS $$
    SELECT has_fk( $1, 'Table ' || quote_ident($1) || ' should have a foreign key constraint' );
$$ LANGUAGE sql;

-- hasnt_fk( schema, table, description )
CREATE OR REPLACE FUNCTION hasnt_fk ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _hasc( $1, $2, 'f' ), $3 );
$$ LANGUAGE sql;

-- hasnt_fk( table, description )
CREATE OR REPLACE FUNCTION hasnt_fk ( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _hasc( $1, 'f' ), $2 );
$$ LANGUAGE sql;

-- hasnt_fk( table )
CREATE OR REPLACE FUNCTION hasnt_fk ( NAME )
RETURNS TEXT AS $$
    SELECT hasnt_fk( $1, 'Table ' || quote_ident($1) || ' should not have a foreign key constraint' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _fkexists ( NAME, NAME, NAME[] )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT TRUE
           FROM pg_all_foreign_keys
          WHERE fk_schema_name    = $1
            AND quote_ident(fk_table_name)     = quote_ident($2)
            AND fk_columns = $3
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _fkexists ( NAME, NAME[] )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT TRUE
           FROM pg_all_foreign_keys
          WHERE quote_ident(fk_table_name)     = quote_ident($1)
            AND fk_columns = $2
    );
$$ LANGUAGE SQL;

-- col_is_fk( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_is_fk ( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
DECLARE
    names text[];
BEGIN
    IF _fkexists($1, $2, $3) THEN
        RETURN pass( $4 );
    END IF;

    -- Try to show the columns.
    SELECT ARRAY(
        SELECT _ident_array_to_string(fk_columns, ', ')
          FROM pg_all_foreign_keys
         WHERE fk_schema_name = $1
           AND fk_table_name  = $2
         ORDER BY fk_columns
    ) INTO names;

    IF names[1] IS NOT NULL THEN
        RETURN fail($4) || E'\n' || diag(
            '    Table ' || quote_ident($1) || '.' || quote_ident($2) || E' has foreign key constraints on these columns:\n        '
            ||  array_to_string( names, E'\n        ' )
        );
    END IF;

    -- No FKs in this table.
    RETURN fail($4) || E'\n' || diag(
        '    Table ' || quote_ident($1) || '.' || quote_ident($2) || ' has no foreign key columns'
    );
END;
$$ LANGUAGE plpgsql;

-- col_is_fk( table, column, description )
CREATE OR REPLACE FUNCTION col_is_fk ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
DECLARE
    names text[];
BEGIN
    IF _fkexists($1, $2) THEN
        RETURN pass( $3 );
    END IF;

    -- Try to show the columns.
    SELECT ARRAY(
        SELECT _ident_array_to_string(fk_columns, ', ')
          FROM pg_all_foreign_keys
         WHERE fk_table_name  = $1
         ORDER BY fk_columns
    ) INTO names;

    IF NAMES[1] IS NOT NULL THEN
        RETURN fail($3) || E'\n' || diag(
            '    Table ' || quote_ident($1) || E' has foreign key constraints on these columns:\n        '
            || array_to_string( names, E'\n        ' )
        );
    END IF;

    -- No FKs in this table.
    RETURN fail($3) || E'\n' || diag(
        '    Table ' || quote_ident($1) || ' has no foreign key columns'
    );
END;
$$ LANGUAGE plpgsql;

-- col_is_fk( table, column[] )
CREATE OR REPLACE FUNCTION col_is_fk ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT col_is_fk( $1, $2, 'Columns ' || quote_ident($1) || '(' || _ident_array_to_string($2, ', ') || ') should be a foreign key' );
$$ LANGUAGE sql;

-- col_is_fk( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_is_fk ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_is_fk( $1, $2, ARRAY[$3], $4 );
$$ LANGUAGE sql;

-- col_is_fk( table, column, description )
CREATE OR REPLACE FUNCTION col_is_fk ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_is_fk( $1, ARRAY[$2], $3 );
$$ LANGUAGE sql;

-- col_is_fk( table, column )
CREATE OR REPLACE FUNCTION col_is_fk ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT col_is_fk( $1, $2, 'Column ' || quote_ident($1) || '(' || quote_ident($2) || ') should be a foreign key' );
$$ LANGUAGE sql;

-- col_isnt_fk( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_isnt_fk ( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _fkexists( $1, $2, $3 ), $4 );
$$ LANGUAGE SQL;

-- col_isnt_fk( table, column, description )
CREATE OR REPLACE FUNCTION col_isnt_fk ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _fkexists( $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- col_isnt_fk( table, column[] )
CREATE OR REPLACE FUNCTION col_isnt_fk ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT col_isnt_fk( $1, $2, 'Columns ' || quote_ident($1) || '(' || _ident_array_to_string($2, ', ') || ') should not be a foreign key' );
$$ LANGUAGE sql;

-- col_isnt_fk( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_isnt_fk ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_isnt_fk( $1, $2, ARRAY[$3], $4 );
$$ LANGUAGE sql;

-- col_isnt_fk( table, column, description )
CREATE OR REPLACE FUNCTION col_isnt_fk ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_isnt_fk( $1, ARRAY[$2], $3 );
$$ LANGUAGE sql;

-- col_isnt_fk( table, column )
CREATE OR REPLACE FUNCTION col_isnt_fk ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT col_isnt_fk( $1, $2, 'Column ' || quote_ident($1) || '(' || quote_ident($2) || ') should not be a foreign key' );
$$ LANGUAGE sql;

-- has_unique( schema, table, description )
CREATE OR REPLACE FUNCTION has_unique ( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _hasc( $1, $2, 'u' ), $3 );
$$ LANGUAGE sql;

-- has_unique( table, description )
CREATE OR REPLACE FUNCTION has_unique ( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _hasc( $1, 'u' ), $2 );
$$ LANGUAGE sql;

-- has_unique( table )
CREATE OR REPLACE FUNCTION has_unique ( TEXT )
RETURNS TEXT AS $$
    SELECT has_unique( $1, 'Table ' || quote_ident($1) || ' should have a unique constraint' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _constraint ( NAME, NAME, CHAR, NAME[], TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    akey NAME[];
    keys TEXT[] := '{}';
    have TEXT;
BEGIN
    FOR akey IN SELECT * FROM _keys($1, $2, $3) LOOP
        IF akey = $4 THEN RETURN pass($5); END IF;
        keys = keys || akey::text;
    END LOOP;
    IF array_upper(keys, 0) = 1 THEN
        have := 'No ' || $6 || ' constriants';
    ELSE
        have := array_to_string(keys, E'\n              ');
    END IF;

    RETURN fail($5) || E'\n' || diag(
             '        have: ' || have
       || E'\n        want: ' || CASE WHEN $4 IS NULL THEN 'NULL' ELSE $4::text END
    );
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION _constraint ( NAME, CHAR, NAME[], TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    akey NAME[];
    keys TEXT[] := '{}';
    have TEXT;
BEGIN
    FOR akey IN SELECT * FROM _keys($1, $2) LOOP
        IF akey = $3 THEN RETURN pass($4); END IF;
        keys = keys || akey::text;
    END LOOP;
    IF array_upper(keys, 0) = 1 THEN
        have := 'No ' || $5 || ' constriants';
    ELSE
        have := array_to_string(keys, E'\n              ');
    END IF;

    RETURN fail($4) || E'\n' || diag(
             '        have: ' || have
       || E'\n        want: ' || CASE WHEN $3 IS NULL THEN 'NULL' ELSE $3::text END
    );
END;
$$ LANGUAGE plpgsql;

-- col_is_unique( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_is_unique ( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _constraint( $1, $2, 'u', $3, $4, 'unique' );
$$ LANGUAGE sql;

-- col_is_unique( table, column, description )
CREATE OR REPLACE FUNCTION col_is_unique ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _constraint( $1, 'u', $2, $3, 'unique' );
$$ LANGUAGE sql;

-- col_is_unique( table, column[] )
CREATE OR REPLACE FUNCTION col_is_unique ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT col_is_unique( $1, $2, 'Columns ' || quote_ident($1) || '(' || _ident_array_to_string($2, ', ') || ') should have a unique constraint' );
$$ LANGUAGE sql;

-- col_is_unique( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_is_unique ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_is_unique( $1, $2, ARRAY[$3], $4 );
$$ LANGUAGE sql;

-- col_is_unique( table, column, description )
CREATE OR REPLACE FUNCTION col_is_unique ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_is_unique( $1, ARRAY[$2], $3 );
$$ LANGUAGE sql;

-- col_is_unique( table, column )
CREATE OR REPLACE FUNCTION col_is_unique ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT col_is_unique( $1, $2, 'Column ' || quote_ident($1) || '(' || quote_ident($2) || ') should have a unique constraint' );
$$ LANGUAGE sql;

-- has_check( schema, table, description )
CREATE OR REPLACE FUNCTION has_check ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _hasc( $1, $2, 'c' ), $3 );
$$ LANGUAGE sql;

-- has_check( table, description )
CREATE OR REPLACE FUNCTION has_check ( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _hasc( $1, 'c' ), $2 );
$$ LANGUAGE sql;

-- has_check( table )
CREATE OR REPLACE FUNCTION has_check ( NAME )
RETURNS TEXT AS $$
    SELECT has_check( $1, 'Table ' || quote_ident($1) || ' should have a check constraint' );
$$ LANGUAGE sql;

-- col_has_check( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_has_check ( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _constraint( $1, $2, 'c', $3, $4, 'check' );
$$ LANGUAGE sql;

-- col_has_check( table, column, description )
CREATE OR REPLACE FUNCTION col_has_check ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _constraint( $1, 'c', $2, $3, 'check' );
$$ LANGUAGE sql;

-- col_has_check( table, column[] )
CREATE OR REPLACE FUNCTION col_has_check ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT col_has_check( $1, $2, 'Columns ' || quote_ident($1) || '(' || _ident_array_to_string($2, ', ') || ') should have a check constraint' );
$$ LANGUAGE sql;

-- col_has_check( schema, table, column, description )
CREATE OR REPLACE FUNCTION col_has_check ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_has_check( $1, $2, ARRAY[$3], $4 );
$$ LANGUAGE sql;

-- col_has_check( table, column, description )
CREATE OR REPLACE FUNCTION col_has_check ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT col_has_check( $1, ARRAY[$2], $3 );
$$ LANGUAGE sql;

-- col_has_check( table, column )
CREATE OR REPLACE FUNCTION col_has_check ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT col_has_check( $1, $2, 'Column ' || quote_ident($1) || '(' || quote_ident($2) || ') should have a check constraint' );
$$ LANGUAGE sql;

-- fk_ok( fk_schema, fk_table, fk_column[], pk_schema, pk_table, pk_column[], description )
CREATE OR REPLACE FUNCTION fk_ok ( NAME, NAME, NAME[], NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
DECLARE
    sch  name;
    tab  name;
    cols name[];
BEGIN
    SELECT pk_schema_name, pk_table_name, pk_columns
      FROM pg_all_foreign_keys
      WHERE fk_schema_name = $1
        AND fk_table_name  = $2
        AND fk_columns     = $3
      INTO sch, tab, cols;

    RETURN is(
        -- have
        quote_ident($1) || '.' || quote_ident($2) || '(' || _ident_array_to_string( $3, ', ' )
        || ') REFERENCES ' || COALESCE ( sch || '.' || tab || '(' || _ident_array_to_string( cols, ', ' ) || ')', 'NOTHING' ),
        -- want
        quote_ident($1) || '.' || quote_ident($2) || '(' || _ident_array_to_string( $3, ', ' )
        || ') REFERENCES ' ||
        $4 || '.' || $5 || '(' || _ident_array_to_string( $6, ', ' ) || ')',
        $7
    );
END;
$$ LANGUAGE plpgsql;

-- fk_ok( fk_table, fk_column[], pk_table, pk_column[], description )
CREATE OR REPLACE FUNCTION fk_ok ( NAME, NAME[], NAME, NAME[], TEXT )
RETURNS TEXT AS $$
DECLARE
    tab  name;
    cols name[];
BEGIN
    SELECT pk_table_name, pk_columns
      FROM pg_all_foreign_keys
      WHERE fk_table_name = $1
        AND fk_columns    = $2
      INTO tab, cols;

    RETURN is(
        -- have
        $1 || '(' || _ident_array_to_string( $2, ', ' )
        || ') REFERENCES ' || COALESCE( tab || '(' || _ident_array_to_string( cols, ', ' ) || ')', 'NOTHING'),
        -- want
        $1 || '(' || _ident_array_to_string( $2, ', ' )
        || ') REFERENCES ' ||
        $3 || '(' || _ident_array_to_string( $4, ', ' ) || ')',
        $5
    );
END;
$$ LANGUAGE plpgsql;

-- fk_ok( fk_schema, fk_table, fk_column[], fk_schema, pk_table, pk_column[] )
CREATE OR REPLACE FUNCTION fk_ok ( NAME, NAME, NAME[], NAME, NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT fk_ok( $1, $2, $3, $4, $5, $6,
        quote_ident($1) || '.' || quote_ident($2) || '(' || _ident_array_to_string( $3, ', ' )
        || ') should reference ' ||
        $4 || '.' || $5 || '(' || _ident_array_to_string( $6, ', ' ) || ')'
    );
$$ LANGUAGE sql;

-- fk_ok( fk_table, fk_column[], pk_table, pk_column[] )
CREATE OR REPLACE FUNCTION fk_ok ( NAME, NAME[], NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT fk_ok( $1, $2, $3, $4,
        $1 || '(' || _ident_array_to_string( $2, ', ' )
        || ') should reference ' ||
        $3 || '(' || _ident_array_to_string( $4, ', ' ) || ')'
    );
$$ LANGUAGE sql;

-- fk_ok( fk_schema, fk_table, fk_column, pk_schema, pk_table, pk_column, description )
CREATE OR REPLACE FUNCTION fk_ok ( NAME, NAME, NAME, NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT fk_ok( $1, $2, ARRAY[$3], $4, $5, ARRAY[$6], $7 );
$$ LANGUAGE sql;

-- fk_ok( fk_schema, fk_table, fk_column, pk_schema, pk_table, pk_column )
CREATE OR REPLACE FUNCTION fk_ok ( NAME, NAME, NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT fk_ok( $1, $2, ARRAY[$3], $4, $5, ARRAY[$6] );
$$ LANGUAGE sql;

-- fk_ok( fk_table, fk_column, pk_table, pk_column, description )
CREATE OR REPLACE FUNCTION fk_ok ( NAME, NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT fk_ok( $1, ARRAY[$2], $3, ARRAY[$4], $5 );
$$ LANGUAGE sql;

-- fk_ok( fk_table, fk_column, pk_table, pk_column )
CREATE OR REPLACE FUNCTION fk_ok ( NAME, NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT fk_ok( $1, ARRAY[$2], $3, ARRAY[$4] );
$$ LANGUAGE sql;

CREATE OR REPLACE VIEW tap_funky
 AS SELECT p.oid         AS oid,
           n.nspname     AS schema,
           p.proname     AS name,
           array_to_string(p.proargtypes::regtype[], ',') AS args,
           CASE p.proretset WHEN TRUE THEN 'setof ' ELSE '' END
             || p.prorettype::regtype AS returns,
           p.prolang     AS langoid,
           p.proisstrict AS is_strict,
           p.proisagg    AS is_agg,
           p.prosecdef   AS is_definer,
           p.proretset   AS returns_set,
           p.provolatile::char AS volatility,
           pg_catalog.pg_function_is_visible(p.oid) AS is_visible
      FROM pg_catalog.pg_proc p
      JOIN pg_catalog.pg_namespace n ON p.pronamespace = n.oid
;

CREATE OR REPLACE FUNCTION _got_func ( NAME, NAME, NAME[] )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT TRUE
          FROM tap_funky
         WHERE schema = $1
           AND name   = $2
           AND args   = array_to_string($3, ',')
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _got_func ( NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS( SELECT TRUE FROM tap_funky WHERE schema = $1 AND name = $2 );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _got_func ( NAME, NAME[] )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT TRUE
          FROM tap_funky
         WHERE name = $1
           AND args = array_to_string($2, ',')
           AND is_visible
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _got_func ( NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS( SELECT TRUE FROM tap_funky WHERE name = $1 AND is_visible);
$$ LANGUAGE SQL;

-- has_function( schema, function, args[], description )
CREATE OR REPLACE FUNCTION has_function ( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT ok( _got_func($1, $2, $3), $4 );
$$ LANGUAGE SQL;

-- has_function( schema, function, args[] )
CREATE OR REPLACE FUNCTION has_function( NAME, NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT ok(
        _got_func($1, $2, $3),
        'Function ' || quote_ident($1) || '.' || quote_ident($2) || '(' ||
        array_to_string($3, ', ') || ') should exist'
    );
$$ LANGUAGE sql;

-- has_function( schema, function, description )
CREATE OR REPLACE FUNCTION has_function ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _got_func($1, $2), $3 );
$$ LANGUAGE SQL;

-- has_function( schema, function )
CREATE OR REPLACE FUNCTION has_function( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
        _got_func($1, $2),
        'Function ' || quote_ident($1) || '.' || quote_ident($2) || '() should exist'
    );
$$ LANGUAGE sql;

-- has_function( function, args[], description )
CREATE OR REPLACE FUNCTION has_function ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT ok( _got_func($1, $2), $3 );
$$ LANGUAGE SQL;

-- has_function( function, args[] )
CREATE OR REPLACE FUNCTION has_function( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT ok(
        _got_func($1, $2),
        'Function ' || quote_ident($1) || '(' ||
        array_to_string($2, ', ') || ') should exist'
    );
$$ LANGUAGE sql;

-- has_function( function, description )
CREATE OR REPLACE FUNCTION has_function( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _got_func($1), $2 );
$$ LANGUAGE sql;

-- has_function( function )
CREATE OR REPLACE FUNCTION has_function( NAME )
RETURNS TEXT AS $$
    SELECT ok( _got_func($1), 'Function ' || quote_ident($1) || '() should exist' );
$$ LANGUAGE sql;

-- hasnt_function( schema, function, args[], description )
CREATE OR REPLACE FUNCTION hasnt_function ( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _got_func($1, $2, $3), $4 );
$$ LANGUAGE SQL;

-- hasnt_function( schema, function, args[] )
CREATE OR REPLACE FUNCTION hasnt_function( NAME, NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT ok(
        NOT _got_func($1, $2, $3),
        'Function ' || quote_ident($1) || '.' || quote_ident($2) || '(' ||
        array_to_string($3, ', ') || ') should not exist'
    );
$$ LANGUAGE sql;

-- hasnt_function( schema, function, description )
CREATE OR REPLACE FUNCTION hasnt_function ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _got_func($1, $2), $3 );
$$ LANGUAGE SQL;

-- hasnt_function( schema, function )
CREATE OR REPLACE FUNCTION hasnt_function( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
        NOT _got_func($1, $2),
        'Function ' || quote_ident($1) || '.' || quote_ident($2) || '() should not exist'
    );
$$ LANGUAGE sql;

-- hasnt_function( function, args[], description )
CREATE OR REPLACE FUNCTION hasnt_function ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _got_func($1, $2), $3 );
$$ LANGUAGE SQL;

-- hasnt_function( function, args[] )
CREATE OR REPLACE FUNCTION hasnt_function( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT ok(
        NOT _got_func($1, $2),
        'Function ' || quote_ident($1) || '(' ||
        array_to_string($2, ', ') || ') should not exist'
    );
$$ LANGUAGE sql;

-- hasnt_function( function, description )
CREATE OR REPLACE FUNCTION hasnt_function( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _got_func($1), $2 );
$$ LANGUAGE sql;

-- hasnt_function( function )
CREATE OR REPLACE FUNCTION hasnt_function( NAME )
RETURNS TEXT AS $$
    SELECT ok( NOT _got_func($1), 'Function ' || quote_ident($1) || '() should not exist' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _pg_sv_type_array( OID[] )
RETURNS NAME[] AS $$
    SELECT ARRAY(
        SELECT t.typname
          FROM pg_catalog.pg_type t
          JOIN generate_series(1, array_upper($1, 1)) s(i) ON t.oid = $1[i]
         ORDER BY i
    )
$$ LANGUAGE SQL stable;

-- can( schema, functions[], description )
CREATE OR REPLACE FUNCTION can ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
DECLARE
    missing text[];
BEGIN
    SELECT ARRAY(
        SELECT quote_ident($2[i])
          FROM generate_series(1, array_upper($2, 1)) s(i)
          LEFT JOIN tap_funky ON name = $2[i] AND schema = $1
         WHERE oid IS NULL
         GROUP BY $2[i], s.i
         ORDER BY MIN(s.i)
    ) INTO missing;
    IF missing[1] IS NULL THEN
        RETURN ok( true, $3 );
    END IF;
    RETURN ok( false, $3 ) || E'\n' || diag(
        '    ' || quote_ident($1) || '.' ||
        array_to_string( missing, E'() missing\n    ' || quote_ident($1) || '.') ||
        '() missing'
    );
END;
$$ LANGUAGE plpgsql;

-- can( schema, functions[] )
CREATE OR REPLACE FUNCTION can ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT can( $1, $2, 'Schema ' || quote_ident($1) || ' can' );
$$ LANGUAGE sql;

-- can( functions[], description )
CREATE OR REPLACE FUNCTION can ( NAME[], TEXT )
RETURNS TEXT AS $$
DECLARE
    missing text[];
BEGIN
    SELECT ARRAY(
        SELECT quote_ident($1[i])
          FROM generate_series(1, array_upper($1, 1)) s(i)
          LEFT JOIN pg_catalog.pg_proc p
            ON $1[i] = p.proname
           AND pg_catalog.pg_function_is_visible(p.oid)
         WHERE p.oid IS NULL
         ORDER BY s.i
    ) INTO missing;
    IF missing[1] IS NULL THEN
        RETURN ok( true, $2 );
    END IF;
    RETURN ok( false, $2 ) || E'\n' || diag(
        '    ' ||
        array_to_string( missing, E'() missing\n    ') ||
        '() missing'
    );
END;
$$ LANGUAGE plpgsql;

-- can( functions[] )
CREATE OR REPLACE FUNCTION can ( NAME[] )
RETURNS TEXT AS $$
    SELECT can( $1, 'Schema ' || _ident_array_to_string(current_schemas(true), ' or ') || ' can' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _ikeys( NAME, NAME, NAME)
RETURNS NAME[] AS $$
    SELECT ARRAY(
        SELECT a.attname
          FROM pg_catalog.pg_index x
          JOIN pg_catalog.pg_class ct    ON ct.oid = x.indrelid
          JOIN pg_catalog.pg_class ci    ON ci.oid = x.indexrelid
          JOIN pg_catalog.pg_namespace n ON n.oid = ct.relnamespace
          JOIN pg_catalog.pg_attribute a ON ct.oid = a.attrelid
          JOIN generate_series(0, current_setting('max_index_keys')::int - 1) s(i)
            ON a.attnum = x.indkey[s.i]
         WHERE ct.relname = $2
           AND ci.relname = $3
           AND n.nspname  = $1
         ORDER BY s.i
    );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _ikeys( NAME, NAME)
RETURNS NAME[] AS $$
    SELECT ARRAY(
        SELECT a.attname
          FROM pg_catalog.pg_index x
          JOIN pg_catalog.pg_class ct    ON ct.oid = x.indrelid
          JOIN pg_catalog.pg_class ci    ON ci.oid = x.indexrelid
          JOIN pg_catalog.pg_attribute a ON ct.oid = a.attrelid
          JOIN generate_series(0, current_setting('max_index_keys')::int - 1) s(i)
            ON a.attnum = x.indkey[s.i]
         WHERE ct.relname = $1
           AND ci.relname = $2
           AND pg_catalog.pg_table_is_visible(ct.oid)
         ORDER BY s.i
    );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _have_index( NAME, NAME, NAME)
RETURNS BOOLEAN AS $$
    SELECT EXISTS (
    SELECT TRUE
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ct    ON ct.oid = x.indrelid
      JOIN pg_catalog.pg_class ci    ON ci.oid = x.indexrelid
      JOIN pg_catalog.pg_namespace n ON n.oid = ct.relnamespace
     WHERE ct.relname = $2
       AND ci.relname = $3
       AND n.nspname  = $1
    );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _have_index( NAME, NAME)
RETURNS BOOLEAN AS $$
    SELECT EXISTS (
    SELECT TRUE
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ct    ON ct.oid = x.indrelid
      JOIN pg_catalog.pg_class ci    ON ci.oid = x.indexrelid
     WHERE ct.relname = $1
       AND ci.relname = $2
    );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _iexpr( NAME, NAME, NAME)
RETURNS TEXT AS $$
    SELECT pg_catalog.pg_get_expr( x.indexprs, ct.oid )
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ct    ON ct.oid = x.indrelid
      JOIN pg_catalog.pg_class ci    ON ci.oid = x.indexrelid
      JOIN pg_catalog.pg_namespace n ON n.oid = ct.relnamespace
     WHERE ct.relname = $2
       AND ci.relname = $3
       AND n.nspname  = $1
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _iexpr( NAME, NAME)
RETURNS TEXT AS $$
    SELECT pg_catalog.pg_get_expr( x.indexprs, ct.oid )
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ct ON ct.oid = x.indrelid
      JOIN pg_catalog.pg_class ci ON ci.oid = x.indexrelid
     WHERE ct.relname = $1
       AND ci.relname = $2
       AND pg_catalog.pg_table_is_visible(ct.oid)
$$ LANGUAGE sql;

-- has_index( schema, table, index, columns[], description )
CREATE OR REPLACE FUNCTION has_index ( NAME, NAME, NAME, NAME[], text )
RETURNS TEXT AS $$
DECLARE
     index_cols name[];
BEGIN
    index_cols := _ikeys($1, $2, $3 );

    IF index_cols IS NULL OR index_cols = '{}'::name[] THEN
        RETURN ok( false, $5 ) || E'\n'
            || diag( 'Index ' || quote_ident($3) || ' ON ' || quote_ident($1) || '.' || quote_ident($2) || ' not found');
    END IF;

    RETURN is(
        quote_ident($3) || ' ON ' || quote_ident($1) || '.' || quote_ident($2) || '(' || _ident_array_to_string( index_cols, ', ' ) || ')',
        quote_ident($3) || ' ON ' || quote_ident($1) || '.' || quote_ident($2) || '(' || _ident_array_to_string( $4, ', ' ) || ')',
        $5
    );
END;
$$ LANGUAGE plpgsql;

-- has_index( schema, table, index, columns[] )
CREATE OR REPLACE FUNCTION has_index ( NAME, NAME, NAME, NAME[] )
RETURNS TEXT AS $$
   SELECT has_index( $1, $2, $3, $4, 'Index ' || quote_ident($3) || ' should exist' );
$$ LANGUAGE sql;

-- has_index( schema, table, index, column/expression, description )
CREATE OR REPLACE FUNCTION has_index ( NAME, NAME, NAME, NAME, text )
RETURNS TEXT AS $$
DECLARE
    expr text;
BEGIN
    IF $4 NOT LIKE '%(%' THEN
        -- Not a functional index.
        RETURN has_index( $1, $2, $3, ARRAY[$4], $5 );
    END IF;

    -- Get the functional expression.
    expr := _iexpr($1, $2, $3);

    IF expr IS NULL THEN
        RETURN ok( false, $5 ) || E'\n'
            || diag( 'Index ' || quote_ident($3) || ' ON ' || quote_ident($1) || '.' || quote_ident($2) || ' not found');
    END IF;

    RETURN is(
        quote_ident($3) || ' ON ' || quote_ident($1) || '.' || quote_ident($2) || '(' || expr || ')',
        quote_ident($3) || ' ON ' || quote_ident($1) || '.' || quote_ident($2) || '(' || $4 || ')',
        $5
    );
END;
$$ LANGUAGE plpgsql;

-- has_index( schema, table, index, columns/expression )
CREATE OR REPLACE FUNCTION has_index ( NAME, NAME, NAME, NAME )
RETURNS TEXT AS $$
   SELECT has_index( $1, $2, $3, $4, 'Index ' || quote_ident($3) || ' should exist' );
$$ LANGUAGE sql;

-- has_index( table, index, columns[], description )
CREATE OR REPLACE FUNCTION has_index ( NAME, NAME, NAME[], text )
RETURNS TEXT AS $$
DECLARE
     index_cols name[];
BEGIN
    index_cols := _ikeys($1, $2 );

    IF index_cols IS NULL OR index_cols = '{}'::name[] THEN
        RETURN ok( false, $4 ) || E'\n'
            || diag( 'Index ' || quote_ident($2) || ' ON ' || quote_ident($1) || ' not found');
    END IF;

    RETURN is(
        quote_ident($2) || ' ON ' || quote_ident($1) || '(' || _ident_array_to_string( index_cols, ', ' ) || ')',
        quote_ident($2) || ' ON ' || quote_ident($1) || '(' || _ident_array_to_string( $3, ', ' ) || ')',
        $4
    );
END;
$$ LANGUAGE plpgsql;

-- has_index( table, index, columns[], description )
CREATE OR REPLACE FUNCTION has_index ( NAME, NAME, NAME[] )
RETURNS TEXT AS $$
   SELECT has_index( $1, $2, $3, 'Index ' || quote_ident($2) || ' should exist' );
$$ LANGUAGE sql;

-- _is_schema( schema )
CREATE OR REPLACE FUNCTION _is_schema( NAME )
returns boolean AS $$
    SELECT EXISTS(
        SELECT true
          FROM pg_catalog.pg_namespace
          WHERE nspname = $1
    );
$$ LANGUAGE sql;

-- has_index( table, index, column/expression, description )
-- has_index( schema, table, index, column/expression )
CREATE OR REPLACE FUNCTION has_index ( NAME, NAME, NAME, text )
RETURNS TEXT AS $$
DECLARE
    want_expr text;
    descr text;
    have_expr text;
    idx name;
    tab text;
BEGIN
    IF $3 NOT LIKE '%(%' THEN
        -- Not a functional index.
        IF _is_schema( $1 ) THEN
            -- Looking for schema.table index.
            RETURN ok ( _have_index( $1, $2, $3 ), $4);
        END IF;
        -- Looking for particular columns.
        RETURN has_index( $1, $2, ARRAY[$3], $4 );
    END IF;

    -- Get the functional expression.
    IF _is_schema( $1 ) THEN
        -- Looking for an index within a schema.
        have_expr := _iexpr($1, $2, $3);
        want_expr := $4;
        descr     := 'Index ' || quote_ident($3) || ' should exist';
        idx       := $3;
        tab       := quote_ident($1) || '.' || quote_ident($2);
    ELSE
        -- Looking for an index without a schema spec.
        have_expr := _iexpr($1, $2);
        want_expr := $3;
        descr     := $4;
        idx       := $2;
        tab       := quote_ident($1);
    END IF;

    IF have_expr IS NULL THEN
        RETURN ok( false, descr ) || E'\n'
            || diag( 'Index ' || idx || ' ON ' || tab || ' not found');
    END IF;

    RETURN is(
        quote_ident(idx) || ' ON ' || tab || '(' || have_expr || ')',
        quote_ident(idx) || ' ON ' || tab || '(' || want_expr || ')',
        descr
    );
END;
$$ LANGUAGE plpgsql;

-- has_index( table, index, column/expression )
-- has_index( schema, table, index )
CREATE OR REPLACE FUNCTION has_index ( NAME, NAME, NAME )
RETURNS TEXT AS $$
BEGIN
   IF _is_schema($1) THEN
       -- ( schema, table, index )
       RETURN ok( _have_index( $1, $2, $3 ), 'Index ' || quote_ident($3) || ' should exist' );
   ELSE
       -- ( table, index, column/expression )
       RETURN has_index( $1, $2, $3, 'Index ' || quote_ident($2) || ' should exist' );
   END IF;
END;
$$ LANGUAGE plpgsql;

-- has_index( table, index, description )
CREATE OR REPLACE FUNCTION has_index ( NAME, NAME, text )
RETURNS TEXT AS $$
    SELECT CASE WHEN $3 LIKE '%(%'
           THEN has_index( $1, $2, $3::name )
           ELSE ok( _have_index( $1, $2 ), $3 )
           END;
$$ LANGUAGE sql;

-- has_index( table, index )
CREATE OR REPLACE FUNCTION has_index ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok( _have_index( $1, $2 ), 'Index ' || quote_ident($2) || ' should exist' );
$$ LANGUAGE sql;

-- hasnt_index( schema, table, index, description )
CREATE OR REPLACE FUNCTION hasnt_index ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
BEGIN
    RETURN ok( NOT _have_index( $1, $2, $3 ), $4 );
END;
$$ LANGUAGE plpgSQL;

-- hasnt_index( schema, table, index )
CREATE OR REPLACE FUNCTION hasnt_index ( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
        NOT _have_index( $1, $2, $3 ),
        'Index ' || quote_ident($3) || ' should not exist'
    );
$$ LANGUAGE SQL;

-- hasnt_index( table, index, description )
CREATE OR REPLACE FUNCTION hasnt_index ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _have_index( $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- hasnt_index( table, index )
CREATE OR REPLACE FUNCTION hasnt_index ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
        NOT _have_index( $1, $2 ),
        'Index ' || quote_ident($2) || ' should not exist'
    );
$$ LANGUAGE SQL;

-- index_is_unique( schema, table, index, description )
CREATE OR REPLACE FUNCTION index_is_unique ( NAME, NAME, NAME, text )
RETURNS TEXT AS $$
DECLARE
    res boolean;
BEGIN
    SELECT x.indisunique
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ct    ON ct.oid = x.indrelid
      JOIN pg_catalog.pg_class ci    ON ci.oid = x.indexrelid
      JOIN pg_catalog.pg_namespace n ON n.oid = ct.relnamespace
     WHERE ct.relname = $2
       AND ci.relname = $3
       AND n.nspname  = $1
      INTO res;

      RETURN ok( COALESCE(res, false), $4 );
END;
$$ LANGUAGE plpgsql;

-- index_is_unique( schema, table, index )
CREATE OR REPLACE FUNCTION index_is_unique ( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT index_is_unique(
        $1, $2, $3,
        'Index ' || quote_ident($3) || ' should be unique'
    );
$$ LANGUAGE sql;

-- index_is_unique( table, index )
CREATE OR REPLACE FUNCTION index_is_unique ( NAME, NAME )
RETURNS TEXT AS $$
DECLARE
    res boolean;
BEGIN
    SELECT x.indisunique
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ct ON ct.oid = x.indrelid
      JOIN pg_catalog.pg_class ci ON ci.oid = x.indexrelid
     WHERE ct.relname = $1
       AND ci.relname = $2
       AND pg_catalog.pg_table_is_visible(ct.oid)
      INTO res;

      RETURN ok(
          COALESCE(res, false),
          'Index ' || quote_ident($2) || ' should be unique'
      );
END;
$$ LANGUAGE plpgsql;

-- index_is_unique( index )
CREATE OR REPLACE FUNCTION index_is_unique ( NAME )
RETURNS TEXT AS $$
DECLARE
    res boolean;
BEGIN
    SELECT x.indisunique
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ci ON ci.oid = x.indexrelid
      JOIN pg_catalog.pg_class ct ON ct.oid = x.indrelid
     WHERE ci.relname = $1
       AND pg_catalog.pg_table_is_visible(ct.oid)
      INTO res;

      RETURN ok(
          COALESCE(res, false),
          'Index ' || quote_ident($1) || ' should be unique'
      );
END;
$$ LANGUAGE plpgsql;

-- index_is_primary( schema, table, index, description )
CREATE OR REPLACE FUNCTION index_is_primary ( NAME, NAME, NAME, text )
RETURNS TEXT AS $$
DECLARE
    res boolean;
BEGIN
    SELECT x.indisprimary
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ct    ON ct.oid = x.indrelid
      JOIN pg_catalog.pg_class ci    ON ci.oid = x.indexrelid
      JOIN pg_catalog.pg_namespace n ON n.oid = ct.relnamespace
     WHERE ct.relname = $2
       AND ci.relname = $3
       AND n.nspname  = $1
      INTO res;

      RETURN ok( COALESCE(res, false), $4 );
END;
$$ LANGUAGE plpgsql;

-- index_is_primary( schema, table, index )
CREATE OR REPLACE FUNCTION index_is_primary ( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT index_is_primary(
        $1, $2, $3,
        'Index ' || quote_ident($3) || ' should be on a primary key'
    );
$$ LANGUAGE sql;

-- index_is_primary( table, index )
CREATE OR REPLACE FUNCTION index_is_primary ( NAME, NAME )
RETURNS TEXT AS $$
DECLARE
    res boolean;
BEGIN
    SELECT x.indisprimary
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ct ON ct.oid = x.indrelid
      JOIN pg_catalog.pg_class ci ON ci.oid = x.indexrelid
     WHERE ct.relname = $1
       AND ci.relname = $2
       AND pg_catalog.pg_table_is_visible(ct.oid)
     INTO res;

      RETURN ok(
          COALESCE(res, false),
          'Index ' || quote_ident($2) || ' should be on a primary key'
      );
END;
$$ LANGUAGE plpgsql;

-- index_is_primary( index )
CREATE OR REPLACE FUNCTION index_is_primary ( NAME )
RETURNS TEXT AS $$
DECLARE
    res boolean;
BEGIN
    SELECT x.indisprimary
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ci ON ci.oid = x.indexrelid
      JOIN pg_catalog.pg_class ct ON ct.oid = x.indrelid
     WHERE ci.relname = $1
       AND pg_catalog.pg_table_is_visible(ct.oid)
      INTO res;

      RETURN ok(
          COALESCE(res, false),
          'Index ' || quote_ident($1) || ' should be on a primary key'
      );
END;
$$ LANGUAGE plpgsql;

-- is_clustered( schema, table, index, description )
CREATE OR REPLACE FUNCTION is_clustered ( NAME, NAME, NAME, text )
RETURNS TEXT AS $$
DECLARE
    res boolean;
BEGIN
    SELECT x.indisclustered
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ct    ON ct.oid = x.indrelid
      JOIN pg_catalog.pg_class ci    ON ci.oid = x.indexrelid
      JOIN pg_catalog.pg_namespace n ON n.oid = ct.relnamespace
     WHERE ct.relname = $2
       AND ci.relname = $3
       AND n.nspname  = $1
      INTO res;

      RETURN ok( COALESCE(res, false), $4 );
END;
$$ LANGUAGE plpgsql;

-- is_clustered( schema, table, index )
CREATE OR REPLACE FUNCTION is_clustered ( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT is_clustered(
        $1, $2, $3,
        'Table ' || quote_ident($1) || '.' || quote_ident($2) ||
        ' should be clustered on index ' || quote_ident($3)
    );
$$ LANGUAGE sql;

-- is_clustered( table, index )
CREATE OR REPLACE FUNCTION is_clustered ( NAME, NAME )
RETURNS TEXT AS $$
DECLARE
    res boolean;
BEGIN
    SELECT x.indisclustered
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ct ON ct.oid = x.indrelid
      JOIN pg_catalog.pg_class ci ON ci.oid = x.indexrelid
     WHERE ct.relname = $1
       AND ci.relname = $2
      INTO res;

      RETURN ok(
          COALESCE(res, false),
          'Table ' || quote_ident($1) || ' should be clustered on index ' || quote_ident($2)
      );
END;
$$ LANGUAGE plpgsql;

-- is_clustered( index )
CREATE OR REPLACE FUNCTION is_clustered ( NAME )
RETURNS TEXT AS $$
DECLARE
    res boolean;
BEGIN
    SELECT x.indisclustered
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ci ON ci.oid = x.indexrelid
     WHERE ci.relname = $1
      INTO res;

      RETURN ok(
          COALESCE(res, false),
          'Table should be clustered on index ' || quote_ident($1)
      );
END;
$$ LANGUAGE plpgsql;

-- index_is_type( schema, table, index, type, description )
CREATE OR REPLACE FUNCTION index_is_type ( NAME, NAME, NAME, NAME, text )
RETURNS TEXT AS $$
DECLARE
    aname name;
BEGIN
    SELECT am.amname
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ct    ON ct.oid = x.indrelid
      JOIN pg_catalog.pg_class ci    ON ci.oid = x.indexrelid
      JOIN pg_catalog.pg_namespace n ON n.oid = ct.relnamespace
      JOIN pg_catalog.pg_am am       ON ci.relam = am.oid
     WHERE ct.relname = $2
       AND ci.relname = $3
       AND n.nspname  = $1
      INTO aname;

      return is( aname, $4, $5 );
END;
$$ LANGUAGE plpgsql;

-- index_is_type( schema, table, index, type )
CREATE OR REPLACE FUNCTION index_is_type ( NAME, NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT index_is_type(
        $1, $2, $3, $4,
        'Index ' || quote_ident($3) || ' should be a ' || quote_ident($4) || ' index'
    );
$$ LANGUAGE SQL;

-- index_is_type( table, index, type )
CREATE OR REPLACE FUNCTION index_is_type ( NAME, NAME, NAME )
RETURNS TEXT AS $$
DECLARE
    aname name;
BEGIN
    SELECT am.amname
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ct ON ct.oid = x.indrelid
      JOIN pg_catalog.pg_class ci ON ci.oid = x.indexrelid
      JOIN pg_catalog.pg_am am    ON ci.relam = am.oid
     WHERE ct.relname = $1
       AND ci.relname = $2
      INTO aname;

      return is(
          aname, $3,
          'Index ' || quote_ident($2) || ' should be a ' || quote_ident($3) || ' index'
      );
END;
$$ LANGUAGE plpgsql;

-- index_is_type( index, type )
CREATE OR REPLACE FUNCTION index_is_type ( NAME, NAME )
RETURNS TEXT AS $$
DECLARE
    aname name;
BEGIN
    SELECT am.amname
      FROM pg_catalog.pg_index x
      JOIN pg_catalog.pg_class ci ON ci.oid = x.indexrelid
      JOIN pg_catalog.pg_am am    ON ci.relam = am.oid
     WHERE ci.relname = $1
      INTO aname;

      return is(
          aname, $2,
          'Index ' || quote_ident($1) || ' should be a ' || quote_ident($2) || ' index'
      );
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION _trig ( NAME, NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT true
          FROM pg_catalog.pg_trigger t
          JOIN pg_catalog.pg_class c     ON c.oid = t.tgrelid
          JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
         WHERE n.nspname = $1
           AND c.relname = $2
           AND t.tgname  = $3
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _trig ( NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT true
          FROM pg_catalog.pg_trigger t
          JOIN pg_catalog.pg_class c     ON c.oid = t.tgrelid
         WHERE c.relname = $1
           AND t.tgname  = $2
    );
$$ LANGUAGE SQL;

-- has_trigger( schema, table, trigger, description )
CREATE OR REPLACE FUNCTION has_trigger ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _trig($1, $2, $3), $4);
$$ LANGUAGE SQL;

-- has_trigger( schema, table, trigger )
CREATE OR REPLACE FUNCTION has_trigger ( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT has_trigger(
        $1, $2, $3,
        'Table ' || quote_ident($1) || '.' || quote_ident($2) || ' should have trigger ' || quote_ident($3)
    );
$$ LANGUAGE sql;

-- has_trigger( table, trigger, description )
CREATE OR REPLACE FUNCTION has_trigger ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _trig($1, $2), $3);
$$ LANGUAGE sql;

-- has_trigger( table, trigger )
CREATE OR REPLACE FUNCTION has_trigger ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok( _trig($1, $2), 'Table ' || quote_ident($1) || ' should have trigger ' || quote_ident($2));
$$ LANGUAGE SQL;

-- hasnt_trigger( schema, table, trigger, description )
CREATE OR REPLACE FUNCTION hasnt_trigger ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _trig($1, $2, $3), $4);
$$ LANGUAGE SQL;

-- hasnt_trigger( schema, table, trigger )
CREATE OR REPLACE FUNCTION hasnt_trigger ( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
        NOT _trig($1, $2, $3),
        'Table ' || quote_ident($1) || '.' || quote_ident($2) || ' should not have trigger ' || quote_ident($3)
    );
$$ LANGUAGE sql;

-- hasnt_trigger( table, trigger, description )
CREATE OR REPLACE FUNCTION hasnt_trigger ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _trig($1, $2), $3);
$$ LANGUAGE sql;

-- hasnt_trigger( table, trigger )
CREATE OR REPLACE FUNCTION hasnt_trigger ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok( NOT _trig($1, $2), 'Table ' || quote_ident($1) || ' should not have trigger ' || quote_ident($2));
$$ LANGUAGE SQL;

-- trigger_is( schema, table, trigger, schema, function, description )
CREATE OR REPLACE FUNCTION trigger_is ( NAME, NAME, NAME, NAME, NAME, text )
RETURNS TEXT AS $$
DECLARE
    pname text;
BEGIN
    SELECT quote_ident(ni.nspname) || '.' || quote_ident(p.proname)
      FROM pg_catalog.pg_trigger t
      JOIN pg_catalog.pg_class ct     ON ct.oid = t.tgrelid
      JOIN pg_catalog.pg_namespace nt ON nt.oid = ct.relnamespace
      JOIN pg_catalog.pg_proc p       ON p.oid = t.tgfoid
      JOIN pg_catalog.pg_namespace ni ON ni.oid = p.pronamespace
     WHERE nt.nspname = $1
       AND ct.relname = $2
       AND t.tgname   = $3
      INTO pname;

    RETURN is( pname, quote_ident($4) || '.' || quote_ident($5), $6 );
END;
$$ LANGUAGE plpgsql;

-- trigger_is( schema, table, trigger, schema, function )
CREATE OR REPLACE FUNCTION trigger_is ( NAME, NAME, NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT trigger_is(
        $1, $2, $3, $4, $5,
        'Trigger ' || quote_ident($3) || ' should call ' || quote_ident($4) || '.' || quote_ident($5) || '()'
    );
$$ LANGUAGE sql;

-- trigger_is( table, trigger, function, description )
CREATE OR REPLACE FUNCTION trigger_is ( NAME, NAME, NAME, text )
RETURNS TEXT AS $$
DECLARE
    pname text;
BEGIN
    SELECT p.proname
      FROM pg_catalog.pg_trigger t
      JOIN pg_catalog.pg_class ct ON ct.oid = t.tgrelid
      JOIN pg_catalog.pg_proc p   ON p.oid = t.tgfoid
     WHERE ct.relname = $1
       AND t.tgname   = $2
       AND pg_catalog.pg_table_is_visible(ct.oid)
      INTO pname;

    RETURN is( pname, $3::text, $4 );
END;
$$ LANGUAGE plpgsql;

-- trigger_is( table, trigger, function )
CREATE OR REPLACE FUNCTION trigger_is ( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT trigger_is(
        $1, $2, $3,
        'Trigger ' || quote_ident($2) || ' should call ' || quote_ident($3) || '()'
    );
$$ LANGUAGE sql;

-- has_schema( schema, description )
CREATE OR REPLACE FUNCTION has_schema( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok(
        EXISTS(
            SELECT true
              FROM pg_catalog.pg_namespace
             WHERE nspname = $1
        ), $2
    );
$$ LANGUAGE sql;

-- has_schema( schema )
CREATE OR REPLACE FUNCTION has_schema( NAME )
RETURNS TEXT AS $$
    SELECT has_schema( $1, 'Schema ' || quote_ident($1) || ' should exist' );
$$ LANGUAGE sql;

-- hasnt_schema( schema, description )
CREATE OR REPLACE FUNCTION hasnt_schema( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok(
        NOT EXISTS(
            SELECT true
              FROM pg_catalog.pg_namespace
             WHERE nspname = $1
        ), $2
    );
$$ LANGUAGE sql;

-- hasnt_schema( schema )
CREATE OR REPLACE FUNCTION hasnt_schema( NAME )
RETURNS TEXT AS $$
    SELECT hasnt_schema( $1, 'Schema ' || quote_ident($1) || ' should not exist' );
$$ LANGUAGE sql;

-- has_tablespace( tablespace, location, description )
CREATE OR REPLACE FUNCTION has_tablespace( NAME, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT ok(
        EXISTS(
            SELECT true
              FROM pg_catalog.pg_tablespace
             WHERE spcname = $1
               AND spclocation = $2
        ), $3
    );
$$ LANGUAGE sql;

-- has_tablespace( tablespace, description )
CREATE OR REPLACE FUNCTION has_tablespace( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok(
        EXISTS(
            SELECT true
              FROM pg_catalog.pg_tablespace
             WHERE spcname = $1
        ), $2
    );
$$ LANGUAGE sql;

-- has_tablespace( tablespace )
CREATE OR REPLACE FUNCTION has_tablespace( NAME )
RETURNS TEXT AS $$
    SELECT has_tablespace( $1, 'Tablespace ' || quote_ident($1) || ' should exist' );
$$ LANGUAGE sql;

-- hasnt_tablespace( tablespace, description )
CREATE OR REPLACE FUNCTION hasnt_tablespace( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok(
        NOT EXISTS(
            SELECT true
              FROM pg_catalog.pg_tablespace
             WHERE spcname = $1
        ), $2
    );
$$ LANGUAGE sql;

-- hasnt_tablespace( tablespace )
CREATE OR REPLACE FUNCTION hasnt_tablespace( NAME )
RETURNS TEXT AS $$
    SELECT hasnt_tablespace( $1, 'Tablespace ' || quote_ident($1) || ' should not exist' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _has_type( NAME, NAME, CHAR[] )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT true
          FROM pg_catalog.pg_type t
          JOIN pg_catalog.pg_namespace n ON t.typnamespace = n.oid
         WHERE t.typisdefined
           AND n.nspname = $1
           AND t.typname = $2
           AND t.typtype = ANY( COALESCE($3, ARRAY['b', 'c', 'd', 'p', 'e']) )
    );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _has_type( NAME, CHAR[] )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT true
          FROM pg_catalog.pg_type t
         WHERE t.typisdefined
           AND pg_catalog.pg_type_is_visible(t.oid)
           AND t.typname = $1
           AND t.typtype = ANY( COALESCE($2, ARRAY['b', 'c', 'd', 'p', 'e']) )
    );
$$ LANGUAGE sql;

-- has_type( schema, type, description )
CREATE OR REPLACE FUNCTION has_type( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _has_type( $1, $2, NULL ), $3 );
$$ LANGUAGE sql;

-- has_type( schema, type )
CREATE OR REPLACE FUNCTION has_type( NAME, NAME )
RETURNS TEXT AS $$
    SELECT has_type( $1, $2, 'Type ' || quote_ident($1) || '.' || quote_ident($2) || ' should exist' );
$$ LANGUAGE sql;

-- has_type( type, description )
CREATE OR REPLACE FUNCTION has_type( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _has_type( $1, NULL ), $2 );
$$ LANGUAGE sql;

-- has_type( type )
CREATE OR REPLACE FUNCTION has_type( NAME )
RETURNS TEXT AS $$
    SELECT ok( _has_type( $1, NULL ), ('Type ' || quote_ident($1) || ' should exist')::text );
$$ LANGUAGE sql;

-- hasnt_type( schema, type, description )
CREATE OR REPLACE FUNCTION hasnt_type( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_type( $1, $2, NULL ), $3 );
$$ LANGUAGE sql;

-- hasnt_type( schema, type )
CREATE OR REPLACE FUNCTION hasnt_type( NAME, NAME )
RETURNS TEXT AS $$
    SELECT hasnt_type( $1, $2, 'Type ' || quote_ident($1) || '.' || quote_ident($2) || ' should not exist' );
$$ LANGUAGE sql;

-- hasnt_type( type, description )
CREATE OR REPLACE FUNCTION hasnt_type( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_type( $1, NULL ), $2 );
$$ LANGUAGE sql;

-- hasnt_type( type )
CREATE OR REPLACE FUNCTION hasnt_type( NAME )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_type( $1, NULL ), ('Type ' || quote_ident($1) || ' should not exist')::text );
$$ LANGUAGE sql;

-- has_domain( schema, domain, description )
CREATE OR REPLACE FUNCTION has_domain( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _has_type( $1, $2, ARRAY['d'] ), $3 );
$$ LANGUAGE sql;

-- has_domain( schema, domain )
CREATE OR REPLACE FUNCTION has_domain( NAME, NAME )
RETURNS TEXT AS $$
    SELECT has_domain( $1, $2, 'Domain ' || quote_ident($1) || '.' || quote_ident($2) || ' should exist' );
$$ LANGUAGE sql;

-- has_domain( domain, description )
CREATE OR REPLACE FUNCTION has_domain( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _has_type( $1, ARRAY['d'] ), $2 );
$$ LANGUAGE sql;

-- has_domain( domain )
CREATE OR REPLACE FUNCTION has_domain( NAME )
RETURNS TEXT AS $$
    SELECT ok( _has_type( $1, ARRAY['d'] ), ('Domain ' || quote_ident($1) || ' should exist')::text );
$$ LANGUAGE sql;

-- hasnt_domain( schema, domain, description )
CREATE OR REPLACE FUNCTION hasnt_domain( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_type( $1, $2, ARRAY['d'] ), $3 );
$$ LANGUAGE sql;

-- hasnt_domain( schema, domain )
CREATE OR REPLACE FUNCTION hasnt_domain( NAME, NAME )
RETURNS TEXT AS $$
    SELECT hasnt_domain( $1, $2, 'Domain ' || quote_ident($1) || '.' || quote_ident($2) || ' should not exist' );
$$ LANGUAGE sql;

-- hasnt_domain( domain, description )
CREATE OR REPLACE FUNCTION hasnt_domain( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_type( $1, ARRAY['d'] ), $2 );
$$ LANGUAGE sql;

-- hasnt_domain( domain )
CREATE OR REPLACE FUNCTION hasnt_domain( NAME )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_type( $1, ARRAY['d'] ), ('Domain ' || quote_ident($1) || ' should not exist')::text );
$$ LANGUAGE sql;

-- has_enum( schema, enum, description )
CREATE OR REPLACE FUNCTION has_enum( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _has_type( $1, $2, ARRAY['e'] ), $3 );
$$ LANGUAGE sql;

-- has_enum( schema, enum )
CREATE OR REPLACE FUNCTION has_enum( NAME, NAME )
RETURNS TEXT AS $$
    SELECT has_enum( $1, $2, 'Enum ' || quote_ident($1) || '.' || quote_ident($2) || ' should exist' );
$$ LANGUAGE sql;

-- has_enum( enum, description )
CREATE OR REPLACE FUNCTION has_enum( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _has_type( $1, ARRAY['e'] ), $2 );
$$ LANGUAGE sql;

-- has_enum( enum )
CREATE OR REPLACE FUNCTION has_enum( NAME )
RETURNS TEXT AS $$
    SELECT ok( _has_type( $1, ARRAY['e'] ), ('Enum ' || quote_ident($1) || ' should exist')::text );
$$ LANGUAGE sql;

-- hasnt_enum( schema, enum, description )
CREATE OR REPLACE FUNCTION hasnt_enum( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_type( $1, $2, ARRAY['e'] ), $3 );
$$ LANGUAGE sql;

-- hasnt_enum( schema, enum )
CREATE OR REPLACE FUNCTION hasnt_enum( NAME, NAME )
RETURNS TEXT AS $$
    SELECT hasnt_enum( $1, $2, 'Enum ' || quote_ident($1) || '.' || quote_ident($2) || ' should not exist' );
$$ LANGUAGE sql;

-- hasnt_enum( enum, description )
CREATE OR REPLACE FUNCTION hasnt_enum( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_type( $1, ARRAY['e'] ), $2 );
$$ LANGUAGE sql;

-- hasnt_enum( enum )
CREATE OR REPLACE FUNCTION hasnt_enum( NAME )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_type( $1, ARRAY['e'] ), ('Enum ' || quote_ident($1) || ' should not exist')::text );
$$ LANGUAGE sql;

-- enum_has_labels( schema, enum, labels, description )
CREATE OR REPLACE FUNCTION enum_has_labels( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT is(
        ARRAY(
            SELECT e.enumlabel
              FROM pg_catalog.pg_type t
              JOIN pg_catalog.pg_enum e      ON t.oid = e.enumtypid
              JOIN pg_catalog.pg_namespace n ON t.typnamespace = n.oid
              WHERE t.typisdefined
               AND n.nspname = $1
               AND t.typname = $2
               AND t.typtype = 'e'
             ORDER BY e.oid
        ),
        $3,
        $4
    );
$$ LANGUAGE sql;

-- enum_has_labels( schema, enum, labels )
CREATE OR REPLACE FUNCTION enum_has_labels( NAME, NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT enum_has_labels(
        $1, $2, $3,
        'Enum ' || quote_ident($1) || '.' || quote_ident($2) || ' should have labels (' || array_to_string( $3, ', ' ) || ')'
    );
$$ LANGUAGE sql;

-- enum_has_labels( enum, labels, description )
CREATE OR REPLACE FUNCTION enum_has_labels( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT is(
        ARRAY(
            SELECT e.enumlabel
              FROM pg_catalog.pg_type t
              JOIN pg_catalog.pg_enum e ON t.oid = e.enumtypid
              WHERE t.typisdefined
               AND pg_catalog.pg_type_is_visible(t.oid)
               AND t.typname = $1
               AND t.typtype = 'e'
             ORDER BY e.oid
        ),
        $2,
        $3
    );
$$ LANGUAGE sql;

-- enum_has_labels( enum, labels )
CREATE OR REPLACE FUNCTION enum_has_labels( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT enum_has_labels(
        $1, $2,
        'Enum ' || quote_ident($1) || ' should have labels (' || array_to_string( $2, ', ' ) || ')'
    );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _has_role( NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT true
          FROM pg_catalog.pg_roles
         WHERE rolname = $1
    );
$$ LANGUAGE sql STRICT;

-- has_role( role, description )
CREATE OR REPLACE FUNCTION has_role( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _has_role($1), $2 );
$$ LANGUAGE sql;

-- has_role( role )
CREATE OR REPLACE FUNCTION has_role( NAME )
RETURNS TEXT AS $$
    SELECT ok( _has_role($1), 'Role ' || quote_ident($1) || ' should exist' );
$$ LANGUAGE sql;

-- hasnt_role( role, description )
CREATE OR REPLACE FUNCTION hasnt_role( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_role($1), $2 );
$$ LANGUAGE sql;

-- hasnt_role( role )
CREATE OR REPLACE FUNCTION hasnt_role( NAME )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_role($1), 'Role ' || quote_ident($1) || ' should not exist' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _has_user( NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS( SELECT true FROM pg_catalog.pg_user WHERE usename = $1);
$$ LANGUAGE sql STRICT;

-- has_user( user, description )
CREATE OR REPLACE FUNCTION has_user( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _has_user($1), $2 );
$$ LANGUAGE sql;

-- has_user( user )
CREATE OR REPLACE FUNCTION has_user( NAME )
RETURNS TEXT AS $$
    SELECT ok( _has_user( $1 ), 'User ' || quote_ident($1) || ' should exist');
$$ LANGUAGE sql;

-- hasnt_user( user, description )
CREATE OR REPLACE FUNCTION hasnt_user( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_user($1), $2 );
$$ LANGUAGE sql;

-- hasnt_user( user )
CREATE OR REPLACE FUNCTION hasnt_user( NAME )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_user( $1 ), 'User ' || quote_ident($1) || ' should not exist');
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _is_super( NAME )
RETURNS BOOLEAN AS $$
    SELECT rolsuper
      FROM pg_catalog.pg_roles
     WHERE rolname = $1
$$ LANGUAGE sql STRICT;

-- is_superuser( user, description )
CREATE OR REPLACE FUNCTION is_superuser( NAME, TEXT )
RETURNS TEXT AS $$
DECLARE
    is_super boolean := _is_super($1);
BEGIN
    IF is_super IS NULL THEN
        RETURN fail( $2 ) || E'\n' || diag( '    User ' || quote_ident($1) || ' does not exist') ;
    END IF;
    RETURN ok( is_super, $2 );
END;
$$ LANGUAGE plpgsql;

-- is_superuser( user )
CREATE OR REPLACE FUNCTION is_superuser( NAME )
RETURNS TEXT AS $$
    SELECT is_superuser( $1, 'User ' || quote_ident($1) || ' should be a super user' );
$$ LANGUAGE sql;

-- isnt_superuser( user, description )
CREATE OR REPLACE FUNCTION isnt_superuser( NAME, TEXT )
RETURNS TEXT AS $$
DECLARE
    is_super boolean := _is_super($1);
BEGIN
    IF is_super IS NULL THEN
        RETURN fail( $2 ) || E'\n' || diag( '    User ' || quote_ident($1) || ' does not exist') ;
    END IF;
    RETURN ok( NOT is_super, $2 );
END;
$$ LANGUAGE plpgsql;

-- isnt_superuser( user )
CREATE OR REPLACE FUNCTION isnt_superuser( NAME )
RETURNS TEXT AS $$
    SELECT isnt_superuser( $1, 'User ' || quote_ident($1) || ' should not be a super user' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _has_group( NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS(
        SELECT true
          FROM pg_catalog.pg_group
         WHERE groname = $1
    );
$$ LANGUAGE sql STRICT;

-- has_group( group, description )
CREATE OR REPLACE FUNCTION has_group( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _has_group($1), $2 );
$$ LANGUAGE sql;

-- has_group( group )
CREATE OR REPLACE FUNCTION has_group( NAME )
RETURNS TEXT AS $$
    SELECT ok( _has_group($1), 'Group ' || quote_ident($1) || ' should exist' );
$$ LANGUAGE sql;

-- hasnt_group( group, description )
CREATE OR REPLACE FUNCTION hasnt_group( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_group($1), $2 );
$$ LANGUAGE sql;

-- hasnt_group( group )
CREATE OR REPLACE FUNCTION hasnt_group( NAME )
RETURNS TEXT AS $$
    SELECT ok( NOT _has_group($1), 'Group ' || quote_ident($1) || ' should not exist' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _grolist ( NAME )
RETURNS oid[] AS $$
    SELECT ARRAY(
        SELECT member
          FROM pg_catalog.pg_auth_members m
          JOIN pg_catalog.pg_roles r ON m.roleid = r.oid
         WHERE r.rolname =  $1
    );
$$ LANGUAGE sql;

-- is_member_of( group, user[], description )
CREATE OR REPLACE FUNCTION is_member_of( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
DECLARE
    missing text[];
BEGIN
    IF NOT _has_role($1) THEN
        RETURN fail( $3 ) || E'\n' || diag (
            '    Role ' || quote_ident($1) || ' does not exist'
        );
    END IF;

    SELECT ARRAY(
        SELECT quote_ident($2[i])
          FROM generate_series(1, array_upper($2, 1)) s(i)
          LEFT JOIN pg_catalog.pg_user ON usename = $2[i]
         WHERE usesysid IS NULL
            OR NOT usesysid = ANY ( _grolist($1) )
         ORDER BY s.i
    ) INTO missing;
    IF missing[1] IS NULL THEN
        RETURN ok( true, $3 );
    END IF;
    RETURN ok( false, $3 ) || E'\n' || diag(
        '    Users missing from the ' || quote_ident($1) || E' group:\n        ' ||
        array_to_string( missing, E'\n        ')
    );
END;
$$ LANGUAGE plpgsql;

-- is_member_of( group, user, description )
CREATE OR REPLACE FUNCTION is_member_of( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT is_member_of( $1, ARRAY[$2], $3 );
$$ LANGUAGE SQL;

-- is_member_of( group, user[] )
CREATE OR REPLACE FUNCTION is_member_of( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT is_member_of( $1, $2, 'Should have members of group ' || quote_ident($1) );
$$ LANGUAGE SQL;

-- is_member_of( group, user )
CREATE OR REPLACE FUNCTION is_member_of( NAME, NAME )
RETURNS TEXT AS $$
    SELECT is_member_of( $1, ARRAY[$2] );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _cmp_types(oid, name)
RETURNS BOOLEAN AS $$
DECLARE
    dtype TEXT := display_type($1, NULL);
BEGIN
    RETURN dtype = _quote_ident_like($2, dtype);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION _cast_exists ( NAME, NAME, NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS (
       SELECT TRUE
         FROM pg_catalog.pg_cast c
         JOIN pg_catalog.pg_proc p ON c.castfunc = p.oid
         JOIN pg_catalog.pg_namespace n ON p.pronamespace = n.oid
        WHERE _cmp_types(castsource, $1)
          AND _cmp_types(casttarget, $2)
          AND n.nspname   = $3
          AND p.proname   = $4
   );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _cast_exists ( NAME, NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS (
       SELECT TRUE
         FROM pg_catalog.pg_cast c
         JOIN pg_catalog.pg_proc p ON c.castfunc = p.oid
        WHERE _cmp_types(castsource, $1)
          AND _cmp_types(casttarget, $2)
          AND p.proname   = $3
   );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _cast_exists ( NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS (
       SELECT TRUE
         FROM pg_catalog.pg_cast c
        WHERE _cmp_types(castsource, $1)
          AND _cmp_types(casttarget, $2)
   );
$$ LANGUAGE SQL;

-- has_cast( source_type, target_type, schema, function, description )
CREATE OR REPLACE FUNCTION has_cast ( NAME, NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
   SELECT ok( _cast_exists( $1, $2, $3, $4 ), $5 );
$$ LANGUAGE SQL;

-- has_cast( source_type, target_type, schema, function )
CREATE OR REPLACE FUNCTION has_cast ( NAME, NAME, NAME, NAME )
RETURNS TEXT AS $$
   SELECT ok(
       _cast_exists( $1, $2, $3, $4 ),
        'Cast (' || quote_ident($1) || ' AS ' || quote_ident($2)
        || ') WITH FUNCTION ' || quote_ident($3)
        || '.' || quote_ident($4) || '() should exist'
    );
$$ LANGUAGE SQL;

-- has_cast( source_type, target_type, function, description )
CREATE OR REPLACE FUNCTION has_cast ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
   SELECT ok( _cast_exists( $1, $2, $3 ), $4 );
$$ LANGUAGE SQL;

-- has_cast( source_type, target_type, function )
CREATE OR REPLACE FUNCTION has_cast ( NAME, NAME, NAME )
RETURNS TEXT AS $$
   SELECT ok(
        _cast_exists( $1, $2, $3 ),
        'Cast (' || quote_ident($1) || ' AS ' || quote_ident($2)
        || ') WITH FUNCTION ' || quote_ident($3) || '() should exist'
    );
$$ LANGUAGE SQL;

-- has_cast( source_type, target_type, description )
CREATE OR REPLACE FUNCTION has_cast ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _cast_exists( $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- has_cast( source_type, target_type )
CREATE OR REPLACE FUNCTION has_cast ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
        _cast_exists( $1, $2 ),
        'Cast (' || quote_ident($1) || ' AS ' || quote_ident($2)
        || ') should exist'
    );
$$ LANGUAGE SQL;

-- hasnt_cast( source_type, target_type, schema, function, description )
CREATE OR REPLACE FUNCTION hasnt_cast ( NAME, NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
   SELECT ok( NOT _cast_exists( $1, $2, $3, $4 ), $5 );
$$ LANGUAGE SQL;

-- hasnt_cast( source_type, target_type, schema, function )
CREATE OR REPLACE FUNCTION hasnt_cast ( NAME, NAME, NAME, NAME )
RETURNS TEXT AS $$
   SELECT ok(
       NOT _cast_exists( $1, $2, $3, $4 ),
        'Cast (' || quote_ident($1) || ' AS ' || quote_ident($2)
        || ') WITH FUNCTION ' || quote_ident($3)
        || '.' || quote_ident($4) || '() should not exist'
    );
$$ LANGUAGE SQL;

-- hasnt_cast( source_type, target_type, function, description )
CREATE OR REPLACE FUNCTION hasnt_cast ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
   SELECT ok( NOT _cast_exists( $1, $2, $3 ), $4 );
$$ LANGUAGE SQL;

-- hasnt_cast( source_type, target_type, function )
CREATE OR REPLACE FUNCTION hasnt_cast ( NAME, NAME, NAME )
RETURNS TEXT AS $$
   SELECT ok(
        NOT _cast_exists( $1, $2, $3 ),
        'Cast (' || quote_ident($1) || ' AS ' || quote_ident($2)
        || ') WITH FUNCTION ' || quote_ident($3) || '() should not exist'
    );
$$ LANGUAGE SQL;

-- hasnt_cast( source_type, target_type, description )
CREATE OR REPLACE FUNCTION hasnt_cast ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _cast_exists( $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- hasnt_cast( source_type, target_type )
CREATE OR REPLACE FUNCTION hasnt_cast ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
        NOT _cast_exists( $1, $2 ),
        'Cast (' || quote_ident($1) || ' AS ' || quote_ident($2)
        || ') should not exist'
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _expand_context( char )
RETURNS text AS $$
   SELECT CASE $1
          WHEN 'i' THEN 'implicit'
          WHEN 'a' THEN 'assignment'
          WHEN 'e' THEN 'explicit'
          ELSE          'unknown' END
$$ LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION _get_context( NAME, NAME )
RETURNS "char" AS $$
   SELECT c.castcontext
     FROM pg_catalog.pg_cast c
    WHERE _cmp_types(castsource, $1)
      AND _cmp_types(casttarget, $2)
$$ LANGUAGE SQL;

-- cast_context_is( source_type, target_type, context, description )
CREATE OR REPLACE FUNCTION cast_context_is( NAME, NAME, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    want char = substring(LOWER($3) FROM 1 FOR 1);
    have char := _get_context($1, $2);
BEGIN
    IF have IS NOT NULL THEN
        RETURN is( _expand_context(have), _expand_context(want), $4 );
    END IF;

    RETURN ok( false, $4 ) || E'\n' || diag(
       '    Cast (' || quote_ident($1) || ' AS ' || quote_ident($2)
      || ') does not exist'
    );
END;
$$ LANGUAGE plpgsql;

-- cast_context_is( source_type, target_type, context )
CREATE OR REPLACE FUNCTION cast_context_is( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT cast_context_is(
        $1, $2, $3,
        'Cast (' || quote_ident($1) || ' AS ' || quote_ident($2)
        || ') context should be ' || _expand_context(substring(LOWER($3) FROM 1 FOR 1))
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _op_exists ( NAME, NAME, NAME, NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS (
       SELECT TRUE
         FROM pg_catalog.pg_operator o
         JOIN pg_catalog.pg_namespace n ON o.oprnamespace = n.oid
        WHERE n.nspname = $2
          AND o.oprname = $3
          AND CASE o.oprkind WHEN 'l' THEN $1 IS NULL
              ELSE _cmp_types(o.oprleft, $1) END
          AND CASE o.oprkind WHEN 'r' THEN $4 IS NULL
              ELSE _cmp_types(o.oprright, $4) END
          AND _cmp_types(o.oprresult, $5)
   );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _op_exists ( NAME, NAME, NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS (
       SELECT TRUE
         FROM pg_catalog.pg_operator o
        WHERE pg_catalog.pg_operator_is_visible(o.oid)
          AND o.oprname = $2
          AND CASE o.oprkind WHEN 'l' THEN $1 IS NULL
              ELSE _cmp_types(o.oprleft, $1) END
          AND CASE o.oprkind WHEN 'r' THEN $3 IS NULL
              ELSE _cmp_types(o.oprright, $3) END
          AND _cmp_types(o.oprresult, $4)
   );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _op_exists ( NAME, NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS (
       SELECT TRUE
         FROM pg_catalog.pg_operator o
        WHERE pg_catalog.pg_operator_is_visible(o.oid)
          AND o.oprname = $2
          AND CASE o.oprkind WHEN 'l' THEN $1 IS NULL
              ELSE _cmp_types(o.oprleft, $1) END
          AND CASE o.oprkind WHEN 'r' THEN $3 IS NULL
              ELSE _cmp_types(o.oprright, $3) END
   );
$$ LANGUAGE SQL;

-- has_operator( left_type, schema, name, right_type, return_type, description )
CREATE OR REPLACE FUNCTION has_operator ( NAME, NAME, NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _op_exists($1, $2, $3, $4, $5 ), $6 );
$$ LANGUAGE SQL;

-- has_operator( left_type, schema, name, right_type, return_type )
CREATE OR REPLACE FUNCTION has_operator ( NAME, NAME, NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
         _op_exists($1, $2, $3, $4, $5 ),
        'Operator ' || quote_ident($2) || '.' || $3 || '(' || $1 || ',' || $4
        || ') RETURNS ' || $5 || ' should exist'
    );
$$ LANGUAGE SQL;

-- has_operator( left_type, name, right_type, return_type, description )
CREATE OR REPLACE FUNCTION has_operator ( NAME, NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _op_exists($1, $2, $3, $4 ), $5 );
$$ LANGUAGE SQL;

-- has_operator( left_type, name, right_type, return_type )
CREATE OR REPLACE FUNCTION has_operator ( NAME, NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
         _op_exists($1, $2, $3, $4 ),
        'Operator ' ||  $2 || '(' || $1 || ',' || $3
        || ') RETURNS ' || $4 || ' should exist'
    );
$$ LANGUAGE SQL;

-- has_operator( left_type, name, right_type, description )
CREATE OR REPLACE FUNCTION has_operator ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _op_exists($1, $2, $3 ), $4 );
$$ LANGUAGE SQL;

-- has_operator( left_type, name, right_type )
CREATE OR REPLACE FUNCTION has_operator ( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
         _op_exists($1, $2, $3 ),
        'Operator ' ||  $2 || '(' || $1 || ',' || $3
        || ') should exist'
    );
$$ LANGUAGE SQL;

-- has_leftop( schema, name, right_type, return_type, description )
CREATE OR REPLACE FUNCTION has_leftop ( NAME, NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _op_exists(NULL, $1, $2, $3, $4), $5 );
$$ LANGUAGE SQL;

-- has_leftop( schema, name, right_type, return_type )
CREATE OR REPLACE FUNCTION has_leftop ( NAME, NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
         _op_exists(NULL, $1, $2, $3, $4 ),
        'Left operator ' || quote_ident($1) || '.' || $2 || '(NONE,'
        || $3 || ') RETURNS ' || $4 || ' should exist'
    );
$$ LANGUAGE SQL;

-- has_leftop( name, right_type, return_type, description )
CREATE OR REPLACE FUNCTION has_leftop ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _op_exists(NULL, $1, $2, $3), $4 );
$$ LANGUAGE SQL;

-- has_leftop( name, right_type, return_type )
CREATE OR REPLACE FUNCTION has_leftop ( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
         _op_exists(NULL, $1, $2, $3 ),
        'Left operator ' || $1 || '(NONE,' || $2 || ') RETURNS ' || $3 || ' should exist'
    );
$$ LANGUAGE SQL;

-- has_leftop( name, right_type, description )
CREATE OR REPLACE FUNCTION has_leftop ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _op_exists(NULL, $1, $2), $3 );
$$ LANGUAGE SQL;

-- has_leftop( name, right_type )
CREATE OR REPLACE FUNCTION has_leftop ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
         _op_exists(NULL, $1, $2 ),
        'Left operator ' || $1 || '(NONE,' || $2 || ') should exist'
    );
$$ LANGUAGE SQL;

-- has_rightop( left_type, schema, name, return_type, description )
CREATE OR REPLACE FUNCTION has_rightop ( NAME, NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _op_exists( $1, $2, $3, NULL, $4), $5 );
$$ LANGUAGE SQL;

-- has_rightop( left_type, schema, name, return_type )
CREATE OR REPLACE FUNCTION has_rightop ( NAME, NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
         _op_exists($1, $2, $3, NULL, $4 ),
        'Right operator ' || quote_ident($2) || '.' || $3 || '('
        || $1 || ',NONE) RETURNS ' || $4 || ' should exist'
    );
$$ LANGUAGE SQL;

-- has_rightop( left_type, name, return_type, description )
CREATE OR REPLACE FUNCTION has_rightop ( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _op_exists( $1, $2, NULL, $3), $4 );
$$ LANGUAGE SQL;

-- has_rightop( left_type, name, return_type )
CREATE OR REPLACE FUNCTION has_rightop ( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
         _op_exists($1, $2, NULL, $3 ),
        'Right operator ' || $2 || '('
        || $1 || ',NONE) RETURNS ' || $3 || ' should exist'
    );
$$ LANGUAGE SQL;

-- has_rightop( left_type, name, description )
CREATE OR REPLACE FUNCTION has_rightop ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _op_exists( $1, $2, NULL), $3 );
$$ LANGUAGE SQL;

-- has_rightop( left_type, name )
CREATE OR REPLACE FUNCTION has_rightop ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
         _op_exists($1, $2, NULL ),
        'Right operator ' || $2 || '(' || $1 || ',NONE) should exist'
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _are ( text, name[], name[], TEXT )
RETURNS TEXT AS $$
DECLARE
    what    ALIAS FOR $1;
    extras  ALIAS FOR $2;
    missing ALIAS FOR $3;
    descr   ALIAS FOR $4;
    msg     TEXT    := '';
    res     BOOLEAN := TRUE;
BEGIN
    IF extras[1] IS NOT NULL THEN
        res = FALSE;
        msg := E'\n' || diag(
            '    Extra ' || what || E':\n        '
            ||  _ident_array_to_string( extras, E'\n        ' )
        );
    END IF;
    IF missing[1] IS NOT NULL THEN
        res = FALSE;
        msg := msg || E'\n' || diag(
            '    Missing ' || what || E':\n        '
            ||  _ident_array_to_string( missing, E'\n        ' )
        );
    END IF;

    RETURN ok(res, descr) || msg;
END;
$$ LANGUAGE plpgsql;

-- tablespaces_are( tablespaces, description )
CREATE OR REPLACE FUNCTION tablespaces_are ( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'tablespaces',
        ARRAY(
            SELECT spcname
              FROM pg_catalog.pg_tablespace
            EXCEPT
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
        ),
        ARRAY(
            SELECT $1[i]
               FROM generate_series(1, array_upper($1, 1)) s(i)
            EXCEPT
            SELECT spcname
              FROM pg_catalog.pg_tablespace
        ),
        $2
    );
$$ LANGUAGE SQL;

-- tablespaces_are( tablespaces )
CREATE OR REPLACE FUNCTION tablespaces_are ( NAME[] )
RETURNS TEXT AS $$
    SELECT tablespaces_are( $1, 'There should be the correct tablespaces' );
$$ LANGUAGE SQL;

-- schemas_are( schemas, description )
CREATE OR REPLACE FUNCTION schemas_are ( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'schemas',
        ARRAY(
            SELECT nspname
              FROM pg_catalog.pg_namespace
             WHERE nspname NOT LIKE 'pg_%'
               AND nspname <> 'information_schema'
             EXCEPT
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
        ),
        ARRAY(
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
            EXCEPT
            SELECT nspname
              FROM pg_catalog.pg_namespace
             WHERE nspname NOT LIKE 'pg_%'
               AND nspname <> 'information_schema'
        ),
        $2
    );
$$ LANGUAGE SQL;

-- schemas_are( schemas )
CREATE OR REPLACE FUNCTION schemas_are ( NAME[] )
RETURNS TEXT AS $$
    SELECT schemas_are( $1, 'There should be the correct schemas' );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _extras ( CHAR, NAME, NAME[] )
RETURNS NAME[] AS $$
    SELECT ARRAY(
        SELECT c.relname
          FROM pg_catalog.pg_namespace n
          JOIN pg_catalog.pg_class c ON n.oid = c.relnamespace
         WHERE c.relkind = $1
           AND n.nspname = $2
           AND c.relname NOT IN('pg_all_foreign_keys', 'tap_funky', '__tresults___numb_seq', '__tcache___id_seq')
        EXCEPT
        SELECT $3[i]
          FROM generate_series(1, array_upper($3, 1)) s(i)
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _extras ( CHAR, NAME[] )
RETURNS NAME[] AS $$
    SELECT ARRAY(
        SELECT c.relname
          FROM pg_catalog.pg_namespace n
          JOIN pg_catalog.pg_class c ON n.oid = c.relnamespace
         WHERE pg_catalog.pg_table_is_visible(c.oid)
           AND n.nspname <> 'pg_catalog'
           AND c.relkind = $1
           AND c.relname NOT IN ('__tcache__', '__tresults__', 'pg_all_foreign_keys', 'tap_funky', '__tresults___numb_seq', '__tcache___id_seq')
        EXCEPT
        SELECT $2[i]
          FROM generate_series(1, array_upper($2, 1)) s(i)
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _missing ( CHAR, NAME, NAME[] )
RETURNS NAME[] AS $$
    SELECT ARRAY(
        SELECT $3[i]
          FROM generate_series(1, array_upper($3, 1)) s(i)
        EXCEPT
        SELECT c.relname
          FROM pg_catalog.pg_namespace n
          JOIN pg_catalog.pg_class c ON n.oid = c.relnamespace
         WHERE c.relkind = $1
           AND n.nspname = $2
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _missing ( CHAR, NAME[] )
RETURNS NAME[] AS $$
    SELECT ARRAY(
        SELECT $2[i]
          FROM generate_series(1, array_upper($2, 1)) s(i)
        EXCEPT
        SELECT c.relname
          FROM pg_catalog.pg_namespace n
          JOIN pg_catalog.pg_class c ON n.oid = c.relnamespace
         WHERE pg_catalog.pg_table_is_visible(c.oid)
           AND n.nspname NOT IN ('pg_catalog', 'information_schema')
           AND c.relkind = $1
    );
$$ LANGUAGE SQL;

-- tables_are( schema, tables, description )
CREATE OR REPLACE FUNCTION tables_are ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are( 'tables', _extras('r', $1, $2), _missing('r', $1, $2), $3);
$$ LANGUAGE SQL;

-- tables_are( tables, description )
CREATE OR REPLACE FUNCTION tables_are ( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are( 'tables', _extras('r', $1), _missing('r', $1), $2);
$$ LANGUAGE SQL;

-- tables_are( schema, tables )
CREATE OR REPLACE FUNCTION tables_are ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT _are(
        'tables', _extras('r', $1, $2), _missing('r', $1, $2),
        'Schema ' || quote_ident($1) || ' should have the correct tables'
    );
$$ LANGUAGE SQL;

-- tables_are( tables )
CREATE OR REPLACE FUNCTION tables_are ( NAME[] )
RETURNS TEXT AS $$
    SELECT _are(
        'tables', _extras('r', $1), _missing('r', $1),
        'Search path ' || pg_catalog.current_setting('search_path') || ' should have the correct tables'
    );
$$ LANGUAGE SQL;

-- views_are( schema, views, description )
CREATE OR REPLACE FUNCTION views_are ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are( 'views', _extras('v', $1, $2), _missing('v', $1, $2), $3);
$$ LANGUAGE SQL;

-- views_are( views, description )
CREATE OR REPLACE FUNCTION views_are ( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are( 'views', _extras('v', $1), _missing('v', $1), $2);
$$ LANGUAGE SQL;

-- views_are( schema, views )
CREATE OR REPLACE FUNCTION views_are ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT _are(
        'views', _extras('v', $1, $2), _missing('v', $1, $2),
        'Schema ' || quote_ident($1) || ' should have the correct views'
    );
$$ LANGUAGE SQL;

-- views_are( views )
CREATE OR REPLACE FUNCTION views_are ( NAME[] )
RETURNS TEXT AS $$
    SELECT _are(
        'views', _extras('v', $1), _missing('v', $1),
        'Search path ' || pg_catalog.current_setting('search_path') || ' should have the correct views'
    );
$$ LANGUAGE SQL;

-- sequences_are( schema, sequences, description )
CREATE OR REPLACE FUNCTION sequences_are ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are( 'sequences', _extras('S', $1, $2), _missing('S', $1, $2), $3);
$$ LANGUAGE SQL;

-- sequences_are( sequences, description )
CREATE OR REPLACE FUNCTION sequences_are ( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are( 'sequences', _extras('S', $1), _missing('S', $1), $2);
$$ LANGUAGE SQL;

-- sequences_are( schema, sequences )
CREATE OR REPLACE FUNCTION sequences_are ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT _are(
        'sequences', _extras('S', $1, $2), _missing('S', $1, $2),
        'Schema ' || quote_ident($1) || ' should have the correct sequences'
    );
$$ LANGUAGE SQL;

-- sequences_are( sequences )
CREATE OR REPLACE FUNCTION sequences_are ( NAME[] )
RETURNS TEXT AS $$
    SELECT _are(
        'sequences', _extras('S', $1), _missing('S', $1),
        'Search path ' || pg_catalog.current_setting('search_path') || ' should have the correct sequences'
    );
$$ LANGUAGE SQL;

-- functions_are( schema, functions[], description )
CREATE OR REPLACE FUNCTION functions_are ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'functions',
        ARRAY(
            SELECT name FROM tap_funky WHERE schema = $1
            EXCEPT
            SELECT $2[i]
              FROM generate_series(1, array_upper($2, 1)) s(i)
        ),
        ARRAY(
            SELECT $2[i]
               FROM generate_series(1, array_upper($2, 1)) s(i)
            EXCEPT
            SELECT name FROM tap_funky WHERE schema = $1
        ),
        $3
    );
$$ LANGUAGE SQL;

-- functions_are( schema, functions[] )
CREATE OR REPLACE FUNCTION functions_are ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT functions_are( $1, $2, 'Schema ' || quote_ident($1) || ' should have the correct functions' );
$$ LANGUAGE SQL;

-- functions_are( functions[], description )
CREATE OR REPLACE FUNCTION functions_are ( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'functions',
        ARRAY(
            SELECT name FROM tap_funky WHERE is_visible
            AND schema NOT IN ('pg_catalog', 'information_schema')
            EXCEPT
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
        ),
        ARRAY(
            SELECT $1[i]
               FROM generate_series(1, array_upper($1, 1)) s(i)
            EXCEPT
            SELECT name FROM tap_funky WHERE is_visible
            AND schema NOT IN ('pg_catalog', 'information_schema')
        ),
        $2
    );
$$ LANGUAGE SQL;

-- functions_are( functions[] )
CREATE OR REPLACE FUNCTION functions_are ( NAME[] )
RETURNS TEXT AS $$
    SELECT functions_are( $1, 'Search path ' || pg_catalog.current_setting('search_path') || ' should have the correct functions' );
$$ LANGUAGE SQL;

-- indexes_are( schema, table, indexes[], description )
CREATE OR REPLACE FUNCTION indexes_are( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'indexes',
        ARRAY(
            SELECT ci.relname
              FROM pg_catalog.pg_index x
              JOIN pg_catalog.pg_class ct    ON ct.oid = x.indrelid
              JOIN pg_catalog.pg_class ci    ON ci.oid = x.indexrelid
              JOIN pg_catalog.pg_namespace n ON n.oid = ct.relnamespace
             WHERE ct.relname = $2
               AND n.nspname  = $1
            EXCEPT
            SELECT $3[i]
              FROM generate_series(1, array_upper($3, 1)) s(i)
        ),
        ARRAY(
            SELECT $3[i]
              FROM generate_series(1, array_upper($3, 1)) s(i)
            EXCEPT
            SELECT ci.relname
              FROM pg_catalog.pg_index x
              JOIN pg_catalog.pg_class ct    ON ct.oid = x.indrelid
              JOIN pg_catalog.pg_class ci    ON ci.oid = x.indexrelid
              JOIN pg_catalog.pg_namespace n ON n.oid = ct.relnamespace
             WHERE ct.relname = $2
               AND n.nspname  = $1
        ),
        $4
    );
$$ LANGUAGE SQL;

-- indexes_are( schema, table, indexes[] )
CREATE OR REPLACE FUNCTION indexes_are( NAME, NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT indexes_are( $1, $2, $3, 'Table ' || quote_ident($1) || '.' || quote_ident($2) || ' should have the correct indexes' );
$$ LANGUAGE SQL;

-- indexes_are( table, indexes[], description )
CREATE OR REPLACE FUNCTION indexes_are( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'indexes',
        ARRAY(
            SELECT ci.relname
              FROM pg_catalog.pg_index x
              JOIN pg_catalog.pg_class ct ON ct.oid = x.indrelid
              JOIN pg_catalog.pg_class ci ON ci.oid = x.indexrelid
              JOIN pg_catalog.pg_namespace n ON n.oid = ct.relnamespace
             WHERE ct.relname = $1
               AND pg_catalog.pg_table_is_visible(ct.oid)
               AND n.nspname NOT IN ('pg_catalog', 'information_schema')
            EXCEPT
            SELECT $2[i]
              FROM generate_series(1, array_upper($2, 1)) s(i)
        ),
        ARRAY(
            SELECT $2[i]
              FROM generate_series(1, array_upper($2, 1)) s(i)
            EXCEPT
            SELECT ci.relname
              FROM pg_catalog.pg_index x
              JOIN pg_catalog.pg_class ct ON ct.oid = x.indrelid
              JOIN pg_catalog.pg_class ci ON ci.oid = x.indexrelid
              JOIN pg_catalog.pg_namespace n ON n.oid = ct.relnamespace
             WHERE ct.relname = $1
               AND pg_catalog.pg_table_is_visible(ct.oid)
               AND n.nspname NOT IN ('pg_catalog', 'information_schema')
        ),
        $3
    );
$$ LANGUAGE SQL;

-- indexes_are( table, indexes[] )
CREATE OR REPLACE FUNCTION indexes_are( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT indexes_are( $1, $2, 'Table ' || quote_ident($1) || ' should have the correct indexes' );
$$ LANGUAGE SQL;

-- users_are( users[], description )
CREATE OR REPLACE FUNCTION users_are( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'users',
        ARRAY(
            SELECT usename
              FROM pg_catalog.pg_user
            EXCEPT
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
        ),
        ARRAY(
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
            EXCEPT
            SELECT usename
              FROM pg_catalog.pg_user
        ),
        $2
    );
$$ LANGUAGE SQL;

-- users_are( users[] )
CREATE OR REPLACE FUNCTION users_are( NAME[] )
RETURNS TEXT AS $$
    SELECT users_are( $1, 'There should be the correct users' );
$$ LANGUAGE SQL;

-- groups_are( groups[], description )
CREATE OR REPLACE FUNCTION groups_are( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'groups',
        ARRAY(
            SELECT groname
              FROM pg_catalog.pg_group
            EXCEPT
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
        ),
        ARRAY(
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
            EXCEPT
            SELECT groname
              FROM pg_catalog.pg_group
        ),
        $2
    );
$$ LANGUAGE SQL;

-- groups_are( groups[] )
CREATE OR REPLACE FUNCTION groups_are( NAME[] )
RETURNS TEXT AS $$
    SELECT groups_are( $1, 'There should be the correct groups' );
$$ LANGUAGE SQL;

-- languages_are( languages[], description )
CREATE OR REPLACE FUNCTION languages_are( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'languages',
        ARRAY(
            SELECT lanname
              FROM pg_catalog.pg_language
             WHERE lanispl
            EXCEPT
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
        ),
        ARRAY(
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
            EXCEPT
            SELECT lanname
              FROM pg_catalog.pg_language
             WHERE lanispl
        ),
        $2
    );
$$ LANGUAGE SQL;

-- languages_are( languages[] )
CREATE OR REPLACE FUNCTION languages_are( NAME[] )
RETURNS TEXT AS $$
    SELECT languages_are( $1, 'There should be the correct procedural languages' );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _is_trusted( NAME )
RETURNS BOOLEAN AS $$
    SELECT lanpltrusted FROM pg_catalog.pg_language WHERE lanname = $1;
$$ LANGUAGE SQL;

-- has_language( language, description)
CREATE OR REPLACE FUNCTION has_language( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _is_trusted($1) IS NOT NULL, $2 );
$$ LANGUAGE SQL;

-- has_language( language )
CREATE OR REPLACE FUNCTION has_language( NAME )
RETURNS TEXT AS $$
    SELECT ok( _is_trusted($1) IS NOT NULL, 'Procedural language ' || quote_ident($1) || ' should exist' );
$$ LANGUAGE SQL;

-- hasnt_language( language, description)
CREATE OR REPLACE FUNCTION hasnt_language( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _is_trusted($1) IS NULL, $2 );
$$ LANGUAGE SQL;

-- hasnt_language( language )
CREATE OR REPLACE FUNCTION hasnt_language( NAME )
RETURNS TEXT AS $$
    SELECT ok( _is_trusted($1) IS NULL, 'Procedural language ' || quote_ident($1) || ' should not exist' );
$$ LANGUAGE SQL;

-- language_is_trusted( language, description )
CREATE OR REPLACE FUNCTION language_is_trusted( NAME, TEXT )
RETURNS TEXT AS $$
DECLARE
    is_trusted boolean := _is_trusted($1);
BEGIN
    IF is_trusted IS NULL THEN
        RETURN fail( $2 ) || E'\n' || diag( '    Procedural language ' || quote_ident($1) || ' does not exist') ;
    END IF;
    RETURN ok( is_trusted, $2 );
END;
$$ LANGUAGE plpgsql;

-- language_is_trusted( language )
CREATE OR REPLACE FUNCTION language_is_trusted( NAME )
RETURNS TEXT AS $$
    SELECT language_is_trusted($1, 'Procedural language ' || quote_ident($1) || ' should be trusted' );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _opc_exists( NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT EXISTS (
        SELECT TRUE
          FROM pg_catalog.pg_opclass oc
          JOIN pg_catalog.pg_namespace n ON oc.opcnamespace = n.oid
         WHERE n.nspname  = COALESCE($1, n.nspname)
           AND oc.opcname = $2
    );
$$ LANGUAGE SQL;

-- has_opclass( schema, name, description )
CREATE OR REPLACE FUNCTION has_opclass( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _opc_exists( $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- has_opclass( schema, name )
CREATE OR REPLACE FUNCTION has_opclass( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok( _opc_exists( $1, $2 ), 'Operator class ' || quote_ident($1) || '.' || quote_ident($2) || ' should exist' );
$$ LANGUAGE SQL;

-- has_opclass( name, description )
CREATE OR REPLACE FUNCTION has_opclass( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _opc_exists( NULL, $1 ), $2)
$$ LANGUAGE SQL;

-- has_opclass( name )
CREATE OR REPLACE FUNCTION has_opclass( NAME )
RETURNS TEXT AS $$
    SELECT ok( _opc_exists( NULL, $1 ), 'Operator class ' || quote_ident($1) || ' should exist' );
$$ LANGUAGE SQL;

-- hasnt_opclass( schema, name, description )
CREATE OR REPLACE FUNCTION hasnt_opclass( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _opc_exists( $1, $2 ), $3 );
$$ LANGUAGE SQL;

-- hasnt_opclass( schema, name )
CREATE OR REPLACE FUNCTION hasnt_opclass( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok( NOT _opc_exists( $1, $2 ), 'Operator class ' || quote_ident($1) || '.' || quote_ident($2) || ' should exist' );
$$ LANGUAGE SQL;

-- hasnt_opclass( name, description )
CREATE OR REPLACE FUNCTION hasnt_opclass( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( NOT _opc_exists( NULL, $1 ), $2)
$$ LANGUAGE SQL;

-- hasnt_opclass( name )
CREATE OR REPLACE FUNCTION hasnt_opclass( NAME )
RETURNS TEXT AS $$
    SELECT ok( NOT _opc_exists( NULL, $1 ), 'Operator class ' || quote_ident($1) || ' should exist' );
$$ LANGUAGE SQL;

-- opclasses_are( schema, opclasses[], description )
CREATE OR REPLACE FUNCTION opclasses_are ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'operator classes',
        ARRAY(
            SELECT oc.opcname
              FROM pg_catalog.pg_opclass oc
              JOIN pg_catalog.pg_namespace n ON oc.opcnamespace = n.oid
             WHERE n.nspname  = $1
            EXCEPT
            SELECT $2[i]
              FROM generate_series(1, array_upper($2, 1)) s(i)
        ),
        ARRAY(
            SELECT $2[i]
               FROM generate_series(1, array_upper($2, 1)) s(i)
            EXCEPT
            SELECT oc.opcname
              FROM pg_catalog.pg_opclass oc
              JOIN pg_catalog.pg_namespace n ON oc.opcnamespace = n.oid
             WHERE n.nspname  = $1
        ),
        $3
    );
$$ LANGUAGE SQL;

-- opclasses_are( schema, opclasses[] )
CREATE OR REPLACE FUNCTION opclasses_are ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT opclasses_are( $1, $2, 'Schema ' || quote_ident($1) || ' should have the correct operator classes' );
$$ LANGUAGE SQL;

-- opclasses_are( opclasses[], description )
CREATE OR REPLACE FUNCTION opclasses_are ( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'operator classes',
        ARRAY(
            SELECT oc.opcname
              FROM pg_catalog.pg_opclass oc
              JOIN pg_catalog.pg_namespace n ON oc.opcnamespace = n.oid
               AND n.nspname NOT IN ('pg_catalog', 'information_schema')
               AND pg_catalog.pg_opclass_is_visible(oc.oid)
            EXCEPT
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
        ),
        ARRAY(
            SELECT $1[i]
               FROM generate_series(1, array_upper($1, 1)) s(i)
            EXCEPT
            SELECT oc.opcname
              FROM pg_catalog.pg_opclass oc
              JOIN pg_catalog.pg_namespace n ON oc.opcnamespace = n.oid
               AND n.nspname NOT IN ('pg_catalog', 'information_schema')
               AND pg_catalog.pg_opclass_is_visible(oc.oid)
        ),
        $2
    );
$$ LANGUAGE SQL;

-- opclasses_are( opclasses[] )
CREATE OR REPLACE FUNCTION opclasses_are ( NAME[] )
RETURNS TEXT AS $$
    SELECT opclasses_are( $1, 'Search path ' || pg_catalog.current_setting('search_path') || ' should have the correct operator classes' );
$$ LANGUAGE SQL;

-- rules_are( schema, table, rules[], description )
CREATE OR REPLACE FUNCTION rules_are( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'rules',
        ARRAY(
            SELECT r.rulename
              FROM pg_catalog.pg_rewrite r
              JOIN pg_catalog.pg_class c     ON c.oid = r.ev_class
              JOIN pg_catalog.pg_namespace n ON c.relnamespace = n.oid
             WHERE c.relname = $2
               AND n.nspname = $1
            EXCEPT
            SELECT $3[i]
              FROM generate_series(1, array_upper($3, 1)) s(i)
        ),
        ARRAY(
            SELECT $3[i]
              FROM generate_series(1, array_upper($3, 1)) s(i)
            EXCEPT
            SELECT r.rulename
              FROM pg_catalog.pg_rewrite r
              JOIN pg_catalog.pg_class c     ON c.oid = r.ev_class
              JOIN pg_catalog.pg_namespace n ON c.relnamespace = n.oid
             WHERE c.relname = $2
               AND n.nspname = $1
        ),
        $4
    );
$$ LANGUAGE SQL;

-- rules_are( schema, table, rules[] )
CREATE OR REPLACE FUNCTION rules_are( NAME, NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT rules_are( $1, $2, $3, 'Relation ' || quote_ident($1) || '.' || quote_ident($2) || ' should have the correct rules' );
$$ LANGUAGE SQL;

-- rules_are( table, rules[], description )
CREATE OR REPLACE FUNCTION rules_are( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'rules',
        ARRAY(
            SELECT r.rulename
              FROM pg_catalog.pg_rewrite r
              JOIN pg_catalog.pg_class c     ON c.oid = r.ev_class
              JOIN pg_catalog.pg_namespace n ON c.relnamespace = n.oid
             WHERE c.relname = $1
               AND n.nspname NOT IN ('pg_catalog', 'information_schema')
               AND pg_catalog.pg_table_is_visible(c.oid)
            EXCEPT
            SELECT $2[i]
              FROM generate_series(1, array_upper($2, 1)) s(i)
        ),
        ARRAY(
            SELECT $2[i]
              FROM generate_series(1, array_upper($2, 1)) s(i)
            EXCEPT
            SELECT r.rulename
              FROM pg_catalog.pg_rewrite r
              JOIN pg_catalog.pg_class c     ON c.oid = r.ev_class
              JOIN pg_catalog.pg_namespace n ON c.relnamespace = n.oid
               AND c.relname = $1
               AND n.nspname NOT IN ('pg_catalog', 'information_schema')
               AND pg_catalog.pg_table_is_visible(c.oid)
        ),
        $3
    );
$$ LANGUAGE SQL;

-- rules_are( table, rules[] )
CREATE OR REPLACE FUNCTION rules_are( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT rules_are( $1, $2, 'Relation ' || quote_ident($1) || ' should have the correct rules' );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _is_instead( NAME, NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT r.is_instead
      FROM pg_catalog.pg_rewrite r
      JOIN pg_catalog.pg_class c     ON c.oid = r.ev_class
      JOIN pg_catalog.pg_namespace n ON c.relnamespace = n.oid
     WHERE r.rulename = $3
       AND c.relname  = $2
       AND n.nspname  = $1
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _is_instead( NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT r.is_instead
      FROM pg_catalog.pg_rewrite r
      JOIN pg_catalog.pg_class c     ON c.oid = r.ev_class
     WHERE r.rulename = $2
       AND c.relname  = $1
       AND pg_catalog.pg_table_is_visible(c.oid)
$$ LANGUAGE SQL;

-- has_rule( schema, table, rule, description )
CREATE OR REPLACE FUNCTION has_rule( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _is_instead($1, $2, $3) IS NOT NULL, $4 );
$$ LANGUAGE SQL;

-- has_rule( schema, table, rule )
CREATE OR REPLACE FUNCTION has_rule( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok( _is_instead($1, $2, $3) IS NOT NULL, 'Relation ' || quote_ident($1) || '.' || quote_ident($2) || ' should have rule ' || quote_ident($3) );
$$ LANGUAGE SQL;

-- has_rule( table, rule, description )
CREATE OR REPLACE FUNCTION has_rule( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _is_instead($1, $2) IS NOT NULL, $3 );
$$ LANGUAGE SQL;

-- has_rule( table, rule )
CREATE OR REPLACE FUNCTION has_rule( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok( _is_instead($1, $2) IS NOT NULL, 'Relation ' || quote_ident($1) || ' should have rule ' || quote_ident($2) );
$$ LANGUAGE SQL;

-- hasnt_rule( schema, table, rule, description )
CREATE OR REPLACE FUNCTION hasnt_rule( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _is_instead($1, $2, $3) IS NULL, $4 );
$$ LANGUAGE SQL;

-- hasnt_rule( schema, table, rule )
CREATE OR REPLACE FUNCTION hasnt_rule( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok( _is_instead($1, $2, $3) IS NULL, 'Relation ' || quote_ident($1) || '.' || quote_ident($2) || ' should not have rule ' || quote_ident($3) );
$$ LANGUAGE SQL;

-- hasnt_rule( table, rule, description )
CREATE OR REPLACE FUNCTION hasnt_rule( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT ok( _is_instead($1, $2) IS NULL, $3 );
$$ LANGUAGE SQL;

-- hasnt_rule( table, rule )
CREATE OR REPLACE FUNCTION hasnt_rule( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok( _is_instead($1, $2) IS NULL, 'Relation ' || quote_ident($1) || ' should not have rule ' || quote_ident($2) );
$$ LANGUAGE SQL;

-- rule_is_instead( schema, table, rule, description )
CREATE OR REPLACE FUNCTION rule_is_instead( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
DECLARE
    is_it boolean := _is_instead($1, $2, $3);
BEGIN
    IF is_it IS NOT NULL THEN RETURN ok( is_it, $4 ); END IF;
    RETURN ok( FALSE, $4 ) || E'\n' || diag(
        '    Rule ' || quote_ident($3) || ' does not exist'
    );
END;
$$ LANGUAGE plpgsql;

-- rule_is_instead( schema, table, rule )
CREATE OR REPLACE FUNCTION rule_is_instead( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT rule_is_instead( $1, $2, $3, 'Rule ' || quote_ident($3) || ' on relation ' || quote_ident($1) || '.' || quote_ident($2) || ' should be an INSTEAD rule' );
$$ LANGUAGE SQL;

-- rule_is_instead( table, rule, description )
CREATE OR REPLACE FUNCTION rule_is_instead( NAME, NAME, TEXT )
RETURNS TEXT AS $$
DECLARE
    is_it boolean := _is_instead($1, $2);
BEGIN
    IF is_it IS NOT NULL THEN RETURN ok( is_it, $3 ); END IF;
    RETURN ok( FALSE, $3 ) || E'\n' || diag(
        '    Rule ' || quote_ident($2) || ' does not exist'
    );
END;
$$ LANGUAGE plpgsql;

-- rule_is_instead( table, rule )
CREATE OR REPLACE FUNCTION rule_is_instead( NAME, NAME )
RETURNS TEXT AS $$
    SELECT rule_is_instead($1, $2, 'Rule ' || quote_ident($2) || ' on relation ' || quote_ident($1) || ' should be an INSTEAD rule' );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _expand_on( char )
RETURNS text AS $$
   SELECT CASE $1
          WHEN '1' THEN 'SELECT'
          WHEN '2' THEN 'UPDATE'
          WHEN '3' THEN 'INSERT'
          WHEN '4' THEN 'DELETE'
          ELSE          'UNKNOWN' END
$$ LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION _contract_on( TEXT )
RETURNS "char" AS $$
   SELECT CASE substring(LOWER($1) FROM 1 FOR 1)
          WHEN 's' THEN '1'::"char"
          WHEN 'u' THEN '2'::"char"
          WHEN 'i' THEN '3'::"char"
          WHEN 'd' THEN '4'::"char"
          ELSE          '0'::"char" END
$$ LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION _rule_on( NAME, NAME, NAME )
RETURNS "char" AS $$
    SELECT r.ev_type
      FROM pg_catalog.pg_rewrite r
      JOIN pg_catalog.pg_class c     ON c.oid = r.ev_class
      JOIN pg_catalog.pg_namespace n ON c.relnamespace = n.oid
     WHERE r.rulename = $3
       AND c.relname  = $2
       AND n.nspname  = $1
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _rule_on( NAME, NAME )
RETURNS "char" AS $$
    SELECT r.ev_type
      FROM pg_catalog.pg_rewrite r
      JOIN pg_catalog.pg_class c     ON c.oid = r.ev_class
     WHERE r.rulename = $2
       AND c.relname  = $1
$$ LANGUAGE SQL;

-- rule_is_on( schema, table, rule, event, description )
CREATE OR REPLACE FUNCTION rule_is_on( NAME, NAME, NAME, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    want char := _contract_on($4);
    have char := _rule_on($1, $2, $3);
BEGIN
    IF have IS NOT NULL THEN
        RETURN is( _expand_on(have), _expand_on(want), $5 );
    END IF;

    RETURN ok( false, $5 ) || E'\n' || diag(
        '    Rule ' || quote_ident($3) || ' does not exist on '
        || quote_ident($1) || '.' || quote_ident($2)
    );
END;
$$ LANGUAGE plpgsql;

-- rule_is_on( schema, table, rule, event )
CREATE OR REPLACE FUNCTION rule_is_on( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT rule_is_on(
        $1, $2, $3, $4,
        'Rule ' || quote_ident($3) || ' should be on ' || _expand_on(_contract_on($4)::char)
        || ' to ' || quote_ident($1) || '.' || quote_ident($2)
    );
$$ LANGUAGE SQL;

-- rule_is_on( table, rule, event, description )
CREATE OR REPLACE FUNCTION rule_is_on( NAME, NAME, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    want char := _contract_on($3);
    have char := _rule_on($1, $2);
BEGIN
    IF have IS NOT NULL THEN
        RETURN is( _expand_on(have), _expand_on(want), $4 );
    END IF;

    RETURN ok( false, $4 ) || E'\n' || diag(
        '    Rule ' || quote_ident($2) || ' does not exist on '
        || quote_ident($1)
    );
END;
$$ LANGUAGE plpgsql;

-- rule_is_on( table, rule, event )
CREATE OR REPLACE FUNCTION rule_is_on( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT rule_is_on(
        $1, $2, $3,
        'Rule ' || quote_ident($2) || ' should be on '
        || _expand_on(_contract_on($3)::char) || ' to ' || quote_ident($1)
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _nosuch( NAME, NAME, NAME[])
RETURNS TEXT AS $$
    SELECT E'\n' || diag(
        '    Function '
          || CASE WHEN $1 IS NOT NULL THEN quote_ident($1) || '.' ELSE '' END
          || quote_ident($2) || '('
          || array_to_string($3, ', ') || ') does not exist'
    );
$$ LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION _func_compare( NAME, NAME, NAME[], anyelement, anyelement, TEXT)
RETURNS TEXT AS $$
    SELECT CASE WHEN $4 IS NULL
      THEN ok( FALSE, $6 ) || _nosuch($1, $2, $3)
      ELSE is( $4, $5, $6 )
      END;
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _func_compare( NAME, NAME, NAME[], boolean, TEXT)
RETURNS TEXT AS $$
    SELECT CASE WHEN $4 IS NULL
      THEN ok( FALSE, $5 ) || _nosuch($1, $2, $3)
      ELSE ok( $4, $5 )
      END;
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _func_compare( NAME, NAME, anyelement, anyelement, TEXT)
RETURNS TEXT AS $$
    SELECT CASE WHEN $3 IS NULL
      THEN ok( FALSE, $5 ) || _nosuch($1, $2, '{}')
      ELSE is( $3, $4, $5 )
      END;
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _func_compare( NAME, NAME, boolean, TEXT)
RETURNS TEXT AS $$
    SELECT CASE WHEN $3 IS NULL
      THEN ok( FALSE, $4 ) || _nosuch($1, $2, '{}')
      ELSE ok( $3, $4 )
      END;
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _lang ( NAME, NAME, NAME[] )
RETURNS NAME AS $$
    SELECT l.lanname
      FROM tap_funky f
      JOIN pg_catalog.pg_language l ON f.langoid = l.oid
     WHERE f.schema = $1
       and f.name   = $2
       AND f.args   = array_to_string($3, ',')
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _lang ( NAME, NAME )
RETURNS NAME AS $$
    SELECT l.lanname
      FROM tap_funky f
      JOIN pg_catalog.pg_language l ON f.langoid = l.oid
     WHERE f.schema = $1
       and f.name   = $2
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _lang ( NAME, NAME[] )
RETURNS NAME AS $$
    SELECT l.lanname
      FROM tap_funky f
      JOIN pg_catalog.pg_language l ON f.langoid = l.oid
     WHERE f.name = $1
       AND f.args = array_to_string($2, ',')
       AND f.is_visible;
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _lang ( NAME )
RETURNS NAME AS $$
    SELECT l.lanname
      FROM tap_funky f
      JOIN pg_catalog.pg_language l ON f.langoid = l.oid
     WHERE f.name = $1
       AND f.is_visible;
$$ LANGUAGE SQL;

-- function_lang_is( schema, function, args[], language, description )
CREATE OR REPLACE FUNCTION function_lang_is( NAME, NAME, NAME[], NAME, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare($1, $2, $3, _lang($1, $2, $3), $4, $5 );
$$ LANGUAGE SQL;

-- function_lang_is( schema, function, args[], language )
CREATE OR REPLACE FUNCTION function_lang_is( NAME, NAME, NAME[], NAME )
RETURNS TEXT AS $$
    SELECT function_lang_is(
        $1, $2, $3, $4,
        'Function ' || quote_ident($1) || '.' || quote_ident($2) || '(' ||
        array_to_string($3, ', ') || ') should be written in ' || quote_ident($4)
    );
$$ LANGUAGE SQL;

-- function_lang_is( schema, function, language, description )
CREATE OR REPLACE FUNCTION function_lang_is( NAME, NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare($1, $2, _lang($1, $2), $3, $4 );
$$ LANGUAGE SQL;

-- function_lang_is( schema, function, language )
CREATE OR REPLACE FUNCTION function_lang_is( NAME, NAME, NAME )
RETURNS TEXT AS $$
    SELECT function_lang_is(
        $1, $2, $3,
        'Function ' || quote_ident($1) || '.' || quote_ident($2)
        || '() should be written in ' || quote_ident($3)
    );
$$ LANGUAGE SQL;

-- function_lang_is( function, args[], language, description )
CREATE OR REPLACE FUNCTION function_lang_is( NAME, NAME[], NAME, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare(NULL, $1, $2, _lang($1, $2), $3, $4 );
$$ LANGUAGE SQL;

-- function_lang_is( function, args[], language )
CREATE OR REPLACE FUNCTION function_lang_is( NAME, NAME[], NAME )
RETURNS TEXT AS $$
    SELECT function_lang_is(
        $1, $2, $3,
        'Function ' || quote_ident($1) || '(' ||
        array_to_string($2, ', ') || ') should be written in ' || quote_ident($3)
    );
$$ LANGUAGE SQL;

-- function_lang_is( function, language, description )
CREATE OR REPLACE FUNCTION function_lang_is( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare(NULL, $1, _lang($1), $2, $3 );
$$ LANGUAGE SQL;

-- function_lang_is( function, language )
CREATE OR REPLACE FUNCTION function_lang_is( NAME, NAME )
RETURNS TEXT AS $$
    SELECT function_lang_is(
        $1, $2,
        'Function ' || quote_ident($1)
        || '() should be written in ' || quote_ident($2)
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _returns ( NAME, NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT returns
      FROM tap_funky
     WHERE schema = $1
       AND name   = $2
       AND args   = array_to_string($3, ',')
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _returns ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT returns FROM tap_funky WHERE schema = $1 AND name = $2
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _returns ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT returns
      FROM tap_funky
     WHERE name = $1
       AND args = array_to_string($2, ',')
       AND is_visible;
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _returns ( NAME )
RETURNS TEXT AS $$
    SELECT returns FROM tap_funky WHERE name = $1 AND is_visible;
$$ LANGUAGE SQL;

-- function_returns( schema, function, args[], type, description )
CREATE OR REPLACE FUNCTION function_returns( NAME, NAME, NAME[], TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare($1, $2, $3, _returns($1, $2, $3), $4, $5 );
$$ LANGUAGE SQL;

-- function_returns( schema, function, args[], type )
CREATE OR REPLACE FUNCTION function_returns( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT function_returns(
        $1, $2, $3, $4,
        'Function ' || quote_ident($1) || '.' || quote_ident($2) || '(' ||
        array_to_string($3, ', ') || ') should return ' || $4
    );
$$ LANGUAGE SQL;

-- function_returns( schema, function, type, description )
CREATE OR REPLACE FUNCTION function_returns( NAME, NAME, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare($1, $2, _returns($1, $2), $3, $4 );
$$ LANGUAGE SQL;

-- function_returns( schema, function, type )
CREATE OR REPLACE FUNCTION function_returns( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT function_returns(
        $1, $2, $3,
        'Function ' || quote_ident($1) || '.' || quote_ident($2)
        || '() should return ' || $3
    );
$$ LANGUAGE SQL;

-- function_returns( function, args[], type, description )
CREATE OR REPLACE FUNCTION function_returns( NAME, NAME[], TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare(NULL, $1, $2, _returns($1, $2), $3, $4 );
$$ LANGUAGE SQL;

-- function_returns( function, args[], type )
CREATE OR REPLACE FUNCTION function_returns( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT function_returns(
        $1, $2, $3,
        'Function ' || quote_ident($1) || '(' ||
        array_to_string($2, ', ') || ') should return ' || $3
    );
$$ LANGUAGE SQL;

-- function_returns( function, type, description )
CREATE OR REPLACE FUNCTION function_returns( NAME, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare(NULL, $1, _returns($1), $2, $3 );
$$ LANGUAGE SQL;

-- function_returns( function, type )
CREATE OR REPLACE FUNCTION function_returns( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT function_returns(
        $1, $2,
        'Function ' || quote_ident($1) || '() should return ' || $2
    );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _definer ( NAME, NAME, NAME[] )
RETURNS BOOLEAN AS $$
    SELECT is_definer
      FROM tap_funky
     WHERE schema = $1
       AND name   = $2
       AND args   = array_to_string($3, ',')
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _definer ( NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT is_definer FROM tap_funky WHERE schema = $1 AND name = $2
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _definer ( NAME, NAME[] )
RETURNS BOOLEAN AS $$
    SELECT is_definer
      FROM tap_funky
     WHERE name = $1
       AND args = array_to_string($2, ',')
       AND is_visible;
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _definer ( NAME )
RETURNS BOOLEAN AS $$
    SELECT is_definer FROM tap_funky WHERE name = $1 AND is_visible;
$$ LANGUAGE SQL;

-- is_definer( schema, function, args[], description )
CREATE OR REPLACE FUNCTION is_definer ( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare($1, $2, $3, _definer($1, $2, $3), $4 );
$$ LANGUAGE SQL;

-- is_definer( schema, function, args[] )
CREATE OR REPLACE FUNCTION is_definer( NAME, NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT ok(
        _definer($1, $2, $3),
        'Function ' || quote_ident($1) || '.' || quote_ident($2) || '(' ||
        array_to_string($3, ', ') || ') should be security definer'
    );
$$ LANGUAGE sql;

-- is_definer( schema, function, description )
CREATE OR REPLACE FUNCTION is_definer ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare($1, $2, _definer($1, $2), $3 );
$$ LANGUAGE SQL;

-- is_definer( schema, function )
CREATE OR REPLACE FUNCTION is_definer( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
        _definer($1, $2),
        'Function ' || quote_ident($1) || '.' || quote_ident($2) || '() should be security definer'
    );
$$ LANGUAGE sql;

-- is_definer( function, args[], description )
CREATE OR REPLACE FUNCTION is_definer ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare(NULL, $1, $2, _definer($1, $2), $3 );
$$ LANGUAGE SQL;

-- is_definer( function, args[] )
CREATE OR REPLACE FUNCTION is_definer( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT ok(
        _definer($1, $2),
        'Function ' || quote_ident($1) || '(' ||
        array_to_string($2, ', ') || ') should be security definer'
    );
$$ LANGUAGE sql;

-- is_definer( function, description )
CREATE OR REPLACE FUNCTION is_definer( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare(NULL, $1, _definer($1), $2 );
$$ LANGUAGE sql;

-- is_definer( function )
CREATE OR REPLACE FUNCTION is_definer( NAME )
RETURNS TEXT AS $$
    SELECT ok( _definer($1), 'Function ' || quote_ident($1) || '() should be security definer' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _agg ( NAME, NAME, NAME[] )
RETURNS BOOLEAN AS $$
    SELECT is_agg
      FROM tap_funky
     WHERE schema = $1
       AND name   = $2
       AND args   = array_to_string($3, ',')
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _agg ( NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT is_agg FROM tap_funky WHERE schema = $1 AND name = $2
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _agg ( NAME, NAME[] )
RETURNS BOOLEAN AS $$
    SELECT is_agg
      FROM tap_funky
     WHERE name = $1
       AND args = array_to_string($2, ',')
       AND is_visible;
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _agg ( NAME )
RETURNS BOOLEAN AS $$
    SELECT is_agg FROM tap_funky WHERE name = $1 AND is_visible;
$$ LANGUAGE SQL;

-- is_aggregate( schema, function, args[], description )
CREATE OR REPLACE FUNCTION is_aggregate ( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare($1, $2, $3, _agg($1, $2, $3), $4 );
$$ LANGUAGE SQL;

-- is_aggregate( schema, function, args[] )
CREATE OR REPLACE FUNCTION is_aggregate( NAME, NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT ok(
        _agg($1, $2, $3),
        'Function ' || quote_ident($1) || '.' || quote_ident($2) || '(' ||
        array_to_string($3, ', ') || ') should be an aggregate function'
    );
$$ LANGUAGE sql;

-- is_aggregate( schema, function, description )
CREATE OR REPLACE FUNCTION is_aggregate ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare($1, $2, _agg($1, $2), $3 );
$$ LANGUAGE SQL;

-- is_aggregate( schema, function )
CREATE OR REPLACE FUNCTION is_aggregate( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
        _agg($1, $2),
        'Function ' || quote_ident($1) || '.' || quote_ident($2) || '() should be an aggregate function'
    );
$$ LANGUAGE sql;

-- is_aggregate( function, args[], description )
CREATE OR REPLACE FUNCTION is_aggregate ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare(NULL, $1, $2, _agg($1, $2), $3 );
$$ LANGUAGE SQL;

-- is_aggregate( function, args[] )
CREATE OR REPLACE FUNCTION is_aggregate( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT ok(
        _agg($1, $2),
        'Function ' || quote_ident($1) || '(' ||
        array_to_string($2, ', ') || ') should be an aggregate function'
    );
$$ LANGUAGE sql;

-- is_aggregate( function, description )
CREATE OR REPLACE FUNCTION is_aggregate( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare(NULL, $1, _agg($1), $2 );
$$ LANGUAGE sql;

-- is_aggregate( function )
CREATE OR REPLACE FUNCTION is_aggregate( NAME )
RETURNS TEXT AS $$
    SELECT ok( _agg($1), 'Function ' || quote_ident($1) || '() should be an aggregate function' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _strict ( NAME, NAME, NAME[] )
RETURNS BOOLEAN AS $$
    SELECT is_strict
      FROM tap_funky
     WHERE schema = $1
       AND name   = $2
       AND args   = array_to_string($3, ',')
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _strict ( NAME, NAME )
RETURNS BOOLEAN AS $$
    SELECT is_strict FROM tap_funky WHERE schema = $1 AND name = $2
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _strict ( NAME, NAME[] )
RETURNS BOOLEAN AS $$
    SELECT is_strict
      FROM tap_funky
     WHERE name = $1
       AND args = array_to_string($2, ',')
       AND is_visible;
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _strict ( NAME )
RETURNS BOOLEAN AS $$
    SELECT is_strict FROM tap_funky WHERE name = $1 AND is_visible;
$$ LANGUAGE SQL;

-- is_strict( schema, function, args[], description )
CREATE OR REPLACE FUNCTION is_strict ( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare($1, $2, $3, _strict($1, $2, $3), $4 );
$$ LANGUAGE SQL;

-- is_strict( schema, function, args[] )
CREATE OR REPLACE FUNCTION is_strict( NAME, NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT ok(
        _strict($1, $2, $3),
        'Function ' || quote_ident($1) || '.' || quote_ident($2) || '(' ||
        array_to_string($3, ', ') || ') should be strict'
    );
$$ LANGUAGE sql;

-- is_strict( schema, function, description )
CREATE OR REPLACE FUNCTION is_strict ( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare($1, $2, _strict($1, $2), $3 );
$$ LANGUAGE SQL;

-- is_strict( schema, function )
CREATE OR REPLACE FUNCTION is_strict( NAME, NAME )
RETURNS TEXT AS $$
    SELECT ok(
        _strict($1, $2),
        'Function ' || quote_ident($1) || '.' || quote_ident($2) || '() should be strict'
    );
$$ LANGUAGE sql;

-- is_strict( function, args[], description )
CREATE OR REPLACE FUNCTION is_strict ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare(NULL, $1, $2, _strict($1, $2), $3 );
$$ LANGUAGE SQL;

-- is_strict( function, args[] )
CREATE OR REPLACE FUNCTION is_strict( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT ok(
        _strict($1, $2),
        'Function ' || quote_ident($1) || '(' ||
        array_to_string($2, ', ') || ') should be strict'
    );
$$ LANGUAGE sql;

-- is_strict( function, description )
CREATE OR REPLACE FUNCTION is_strict( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare(NULL, $1, _strict($1), $2 );
$$ LANGUAGE sql;

-- is_strict( function )
CREATE OR REPLACE FUNCTION is_strict( NAME )
RETURNS TEXT AS $$
    SELECT ok( _strict($1), 'Function ' || quote_ident($1) || '() should be strict' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _expand_vol( char )
RETURNS TEXT AS $$
   SELECT CASE $1
          WHEN 'i' THEN 'IMMUTABLE'
          WHEN 's' THEN 'STABLE'
          WHEN 'v' THEN 'VOLATILE'
          ELSE          'UNKNOWN' END
$$ LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION _refine_vol( text )
RETURNS text AS $$
    SELECT _expand_vol(substring(LOWER($1) FROM 1 FOR 1)::char);
$$ LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION _vol ( NAME, NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT _expand_vol(volatility)
      FROM tap_funky f
     WHERE f.schema = $1
       and f.name   = $2
       AND f.args   = array_to_string($3, ',')
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _vol ( NAME, NAME )
RETURNS TEXT AS $$
    SELECT _expand_vol(volatility) FROM tap_funky f
     WHERE f.schema = $1 and f.name = $2
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _vol ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT _expand_vol(volatility)
      FROM tap_funky f
     WHERE f.name = $1
       AND f.args = array_to_string($2, ',')
       AND f.is_visible;
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _vol ( NAME )
RETURNS TEXT AS $$
    SELECT _expand_vol(volatility) FROM tap_funky f
     WHERE f.name = $1 AND f.is_visible;
$$ LANGUAGE SQL;

-- volatility_is( schema, function, args[], volatility, description )
CREATE OR REPLACE FUNCTION volatility_is( NAME, NAME, NAME[], TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare($1, $2, $3, _vol($1, $2, $3), _refine_vol($4), $5 );
$$ LANGUAGE SQL;

-- volatility_is( schema, function, args[], volatility )
CREATE OR REPLACE FUNCTION volatility_is( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT volatility_is(
        $1, $2, $3, $4,
        'Function ' || quote_ident($1) || '.' || quote_ident($2) || '(' ||
        array_to_string($3, ', ') || ') should be ' || _refine_vol($4)
    );
$$ LANGUAGE SQL;

-- volatility_is( schema, function, volatility, description )
CREATE OR REPLACE FUNCTION volatility_is( NAME, NAME, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare($1, $2, _vol($1, $2), _refine_vol($3), $4 );
$$ LANGUAGE SQL;

-- volatility_is( schema, function, volatility )
CREATE OR REPLACE FUNCTION volatility_is( NAME, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT volatility_is(
        $1, $2, $3,
        'Function ' || quote_ident($1) || '.' || quote_ident($2)
        || '() should be ' || _refine_vol($3)
    );
$$ LANGUAGE SQL;

-- volatility_is( function, args[], volatility, description )
CREATE OR REPLACE FUNCTION volatility_is( NAME, NAME[], TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare(NULL, $1, $2, _vol($1, $2), _refine_vol($3), $4 );
$$ LANGUAGE SQL;

-- volatility_is( function, args[], volatility )
CREATE OR REPLACE FUNCTION volatility_is( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT volatility_is(
        $1, $2, $3,
        'Function ' || quote_ident($1) || '(' ||
        array_to_string($2, ', ') || ') should be ' || _refine_vol($3)
    );
$$ LANGUAGE SQL;

-- volatility_is( function, volatility, description )
CREATE OR REPLACE FUNCTION volatility_is( NAME, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _func_compare(NULL, $1, _vol($1), _refine_vol($2), $3 );
$$ LANGUAGE SQL;

-- volatility_is( function, volatility )
CREATE OR REPLACE FUNCTION volatility_is( NAME, TEXT )
RETURNS TEXT AS $$
    SELECT volatility_is(
        $1, $2,
        'Function ' || quote_ident($1) || '() should be ' || _refine_vol($2)
    );
$$ LANGUAGE SQL;

-- check_test( test_output, pass, name, description, diag, match_diag )
CREATE OR REPLACE FUNCTION check_test( TEXT, BOOLEAN, TEXT, TEXT, TEXT, BOOLEAN )
RETURNS SETOF TEXT AS $$
DECLARE
    tnumb   INTEGER;
    aok     BOOLEAN;
    adescr  TEXT;
    res     BOOLEAN;
    descr   TEXT;
    adiag   TEXT;
    have    ALIAS FOR $1;
    eok     ALIAS FOR $2;
    name    ALIAS FOR $3;
    edescr  ALIAS FOR $4;
    ediag   ALIAS FOR $5;
    matchit ALIAS FOR $6;
BEGIN
    -- What test was it that just ran?
    tnumb := currval('__tresults___numb_seq');

    -- Fetch the results.
    EXECUTE 'SELECT aok, descr FROM __tresults__ WHERE numb = ' || tnumb
       INTO aok, adescr;

    -- Now delete those results.
    EXECUTE 'DELETE FROM __tresults__ WHERE numb = ' || tnumb;
    EXECUTE 'ALTER SEQUENCE __tresults___numb_seq RESTART WITH ' || tnumb;

    -- Set up the description.
    descr := coalesce( name || ' ', 'Test ' ) || 'should ';

    -- So, did the test pass?
    RETURN NEXT is(
        aok,
        eok,
        descr || CASE eok WHEN true then 'pass' ELSE 'fail' END
    );

    -- Was the description as expected?
    IF edescr IS NOT NULL THEN
        RETURN NEXT is(
            adescr,
            edescr,
            descr || 'have the proper description'
        );
    END IF;

    -- Were the diagnostics as expected?
    IF ediag IS NOT NULL THEN
        -- Remove ok and the test number.
        adiag := substring(
            have
            FROM CASE WHEN aok THEN 4 ELSE 9 END + char_length(tnumb::text)
        );

        -- Remove the description, if there is one.
        IF adescr <> '' THEN
            adiag := substring( adiag FROM 3 + char_length( diag( adescr ) ) );
        END IF;

        -- Remove failure message from ok().
        IF NOT aok THEN
           adiag := substring(
               adiag
               FROM 14 + char_length(tnumb::text)
                       + CASE adescr WHEN '' THEN 3 ELSE 3 + char_length( diag( adescr ) ) END
           );
        END IF;

        -- Remove the #s.
        adiag := replace( substring(adiag from 3), E'\n# ', E'\n' );

        -- Now compare the diagnostics.
        IF matchit THEN
            RETURN NEXT matches(
                adiag,
                ediag,
                descr || 'have the proper diagnostics'
            );
        ELSE
            RETURN NEXT is(
                adiag,
                ediag,
                descr || 'have the proper diagnostics'
            );
        END IF;
    END IF;

    -- And we're done
    RETURN;
END;
$$ LANGUAGE plpgsql;

-- check_test( test_output, pass, name, description, diag )
CREATE OR REPLACE FUNCTION check_test( TEXT, BOOLEAN, TEXT, TEXT, TEXT )
RETURNS SETOF TEXT AS $$
    SELECT * FROM check_test( $1, $2, $3, $4, $5, FALSE );
$$ LANGUAGE sql;

-- check_test( test_output, pass, name, description )
CREATE OR REPLACE FUNCTION check_test( TEXT, BOOLEAN, TEXT, TEXT )
RETURNS SETOF TEXT AS $$
    SELECT * FROM check_test( $1, $2, $3, $4, NULL, FALSE );
$$ LANGUAGE sql;

-- check_test( test_output, pass, name )
CREATE OR REPLACE FUNCTION check_test( TEXT, BOOLEAN, TEXT )
RETURNS SETOF TEXT AS $$
    SELECT * FROM check_test( $1, $2, $3, NULL, NULL, FALSE );
$$ LANGUAGE sql;

-- check_test( test_output, pass )
CREATE OR REPLACE FUNCTION check_test( TEXT, BOOLEAN )
RETURNS SETOF TEXT AS $$
    SELECT * FROM check_test( $1, $2, NULL, NULL, NULL, FALSE );
$$ LANGUAGE sql;


CREATE OR REPLACE FUNCTION findfuncs( NAME, TEXT )
RETURNS TEXT[] AS $$
    SELECT ARRAY(
        SELECT DISTINCT quote_ident(n.nspname) || '.' || quote_ident(p.proname) AS pname
          FROM pg_catalog.pg_proc p
          JOIN pg_catalog.pg_namespace n ON p.pronamespace = n.oid
         WHERE n.nspname = $1
           AND p.proname ~ $2
         ORDER BY pname
    );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION findfuncs( TEXT )
RETURNS TEXT[] AS $$
    SELECT ARRAY(
        SELECT DISTINCT quote_ident(n.nspname) || '.' || quote_ident(p.proname) AS pname
          FROM pg_catalog.pg_proc p
          JOIN pg_catalog.pg_namespace n ON p.pronamespace = n.oid
         WHERE pg_catalog.pg_function_is_visible(p.oid)
           AND p.proname ~ $1
         ORDER BY pname
    );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _runem( text[], boolean )
RETURNS SETOF TEXT AS $$
DECLARE
    tap    text;
    lbound int := array_lower($1, 1);
BEGIN
    IF lbound IS NULL THEN RETURN; END IF;
    FOR i IN lbound..array_upper($1, 1) LOOP
        -- Send the name of the function to diag if warranted.
        IF $2 THEN RETURN NEXT diag( $1[i] || '()' ); END IF;
        -- Execute the tap function and return its results.
        FOR tap IN EXECUTE 'SELECT * FROM ' || $1[i] || '()' LOOP
            RETURN NEXT tap;
        END LOOP;
    END LOOP;
    RETURN;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION _is_verbose()
RETURNS BOOLEAN AS $$
    SELECT current_setting('client_min_messages') NOT IN (
        'warning', 'error', 'fatal', 'panic'
    );
$$ LANGUAGE sql STABLE;

-- do_tap( schema, pattern )
CREATE OR REPLACE FUNCTION do_tap( name, text )
RETURNS SETOF TEXT AS $$
    SELECT * FROM _runem( findfuncs($1, $2), _is_verbose() );
$$ LANGUAGE sql;

-- do_tap( schema )
CREATE OR REPLACE FUNCTION do_tap( name )
RETURNS SETOF TEXT AS $$
    SELECT * FROM _runem( findfuncs($1, '^test'), _is_verbose() );
$$ LANGUAGE sql;

-- do_tap( pattern )
CREATE OR REPLACE FUNCTION do_tap( text )
RETURNS SETOF TEXT AS $$
    SELECT * FROM _runem( findfuncs($1), _is_verbose() );
$$ LANGUAGE sql;

-- do_tap()
CREATE OR REPLACE FUNCTION do_tap( )
RETURNS SETOF TEXT AS $$
    SELECT * FROM _runem( findfuncs('^test'), _is_verbose());
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _currtest()
RETURNS INTEGER AS $$
BEGIN
    RETURN currval('__tresults___numb_seq');
EXCEPTION
    WHEN object_not_in_prerequisite_state THEN RETURN 0;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION _cleanup()
RETURNS boolean AS $$
    DROP TABLE __tresults__;
    DROP SEQUENCE __tresults___numb_seq;
    DROP TABLE __tcache__;
    DROP SEQUENCE __tcache___id_seq;
    SELECT TRUE;
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _runner( text[], text[], text[], text[], text[] )
RETURNS SETOF TEXT AS $$
DECLARE
    startup  ALIAS FOR $1;
    shutdown ALIAS FOR $2;
    setup    ALIAS FOR $3;
    teardown ALIAS FOR $4;
    tests    ALIAS FOR $5;
    tap      text;
    verbos   boolean := _is_verbose(); -- verbose is a reserved word in 8.5.
    num_faild INTEGER := 0;
BEGIN
    BEGIN
        -- No plan support.
        PERFORM * FROM no_plan();
        FOR tap IN SELECT * FROM _runem(startup, false) LOOP RETURN NEXT tap; END LOOP;
    EXCEPTION
        -- Catch all exceptions and simply rethrow custom exceptions. This
        -- will roll back everything in the above block.
        WHEN raise_exception THEN
            RAISE EXCEPTION '%', SQLERRM;
    END;

    BEGIN
        FOR i IN 1..array_upper(tests, 1) LOOP
            BEGIN
                -- What test are we running?
                IF verbos THEN RETURN NEXT diag(tests[i] || '()'); END IF;

                -- Run the setup functions.
                FOR tap IN SELECT * FROM _runem(setup, false) LOOP RETURN NEXT tap; END LOOP;

                -- Run the actual test function.
                FOR tap IN EXECUTE 'SELECT * FROM ' || tests[i] || '()' LOOP
                    RETURN NEXT tap;
                END LOOP;

                -- Run the teardown functions.
                FOR tap IN SELECT * FROM _runem(teardown, false) LOOP RETURN NEXT tap; END LOOP;

                -- Remember how many failed and then roll back.
                num_faild := num_faild + num_failed();
                RAISE EXCEPTION '__TAP_ROLLBACK__';

            EXCEPTION WHEN raise_exception THEN
                IF SQLERRM <> '__TAP_ROLLBACK__' THEN
                    -- We didn't raise it, so propagate it.
                    RAISE EXCEPTION '%', SQLERRM;
                END IF;
            END;
        END LOOP;

        -- Run the shutdown functions.
        FOR tap IN SELECT * FROM _runem(shutdown, false) LOOP RETURN NEXT tap; END LOOP;

        -- Raise an exception to rollback any changes.
        RAISE EXCEPTION '__TAP_ROLLBACK__';
    EXCEPTION WHEN raise_exception THEN
        IF SQLERRM <> '__TAP_ROLLBACK__' THEN
            -- We didn't raise it, so propagate it.
            RAISE EXCEPTION '%', SQLERRM;
        END IF;
    END;
    -- Finish up.
    FOR tap IN SELECT * FROM _finish( currval('__tresults___numb_seq')::integer, 0, num_faild ) LOOP
        RETURN NEXT tap;
    END LOOP;

    -- Clean up and return.
    PERFORM _cleanup();
    RETURN;
END;
$$ LANGUAGE plpgsql;

-- runtests( schema, match )
CREATE OR REPLACE FUNCTION runtests( NAME, TEXT )
RETURNS SETOF TEXT AS $$
    SELECT * FROM _runner(
        findfuncs( $1, '^startup' ),
        findfuncs( $1, '^shutdown' ),
        findfuncs( $1, '^setup' ),
        findfuncs( $1, '^teardown' ),
        findfuncs( $1, $2 )
    );
$$ LANGUAGE sql;

-- runtests( schema )
CREATE OR REPLACE FUNCTION runtests( NAME )
RETURNS SETOF TEXT AS $$
    SELECT * FROM runtests( $1, '^test' );
$$ LANGUAGE sql;

-- runtests( match )
CREATE OR REPLACE FUNCTION runtests( TEXT )
RETURNS SETOF TEXT AS $$
    SELECT * FROM _runner(
        findfuncs( '^startup' ),
        findfuncs( '^shutdown' ),
        findfuncs( '^setup' ),
        findfuncs( '^teardown' ),
        findfuncs( $1 )
    );
$$ LANGUAGE sql;

-- runtests( )
CREATE OR REPLACE FUNCTION runtests( )
RETURNS SETOF TEXT AS $$
    SELECT * FROM runtests( '^test' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _temptable ( TEXT, TEXT )
RETURNS TEXT AS $$
BEGIN
    EXECUTE 'CREATE TEMP TABLE ' || $2 || ' AS ' || _query($1);
    return $2;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION _temptable ( anyarray, TEXT )
RETURNS TEXT AS $$
BEGIN
    CREATE TEMP TABLE _____coltmp___ AS
    SELECT $1[i]
    FROM generate_series(array_lower($1, 1), array_upper($1, 1)) s(i);
    EXECUTE 'ALTER TABLE _____coltmp___ RENAME TO ' || $2;
    return $2;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION _temptypes( TEXT )
RETURNS TEXT AS $$
    SELECT array_to_string(ARRAY(
        SELECT pg_catalog.format_type(a.atttypid, a.atttypmod)
          FROM pg_catalog.pg_attribute a
          JOIN pg_catalog.pg_class c ON a.attrelid = c.oid
         WHERE c.relname = $1
           AND c.relistemp
           AND attnum > 0
           AND NOT attisdropped
         ORDER BY attnum
    ), ',');
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _docomp( TEXT, TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    have    ALIAS FOR $1;
    want    ALIAS FOR $2;
    extras  TEXT[]  := '{}';
    missing TEXT[]  := '{}';
    res     BOOLEAN := TRUE;
    msg     TEXT    := '';
    rec     RECORD;
BEGIN
    BEGIN
        -- Find extra records.
        FOR rec in EXECUTE 'SELECT * FROM ' || have || ' EXCEPT ' || $4
                        || 'SELECT * FROM ' || want LOOP
            extras := extras || rec::text;
        END LOOP;

        -- Find missing records.
        FOR rec in EXECUTE 'SELECT * FROM ' || want || ' EXCEPT ' || $4
                        || 'SELECT * FROM ' || have LOOP
            missing := missing || rec::text;
        END LOOP;

        -- Drop the temporary tables.
        EXECUTE 'DROP TABLE ' || have;
        EXECUTE 'DROP TABLE ' || want;
    EXCEPTION WHEN syntax_error OR datatype_mismatch THEN
        msg := E'\n' || diag(
            E'    Columns differ between queries:\n'
            || '        have: (' || _temptypes(have) || E')\n'
            || '        want: (' || _temptypes(want) || ')'
        );
        EXECUTE 'DROP TABLE ' || have;
        EXECUTE 'DROP TABLE ' || want;
        RETURN ok(FALSE, $3) || msg;
    END;

    -- What extra records do we have?
    IF extras[1] IS NOT NULL THEN
        res := FALSE;
        msg := E'\n' || diag(
            E'    Extra records:\n        '
            ||  array_to_string( extras, E'\n        ' )
        );
    END IF;

    -- What missing records do we have?
    IF missing[1] IS NOT NULL THEN
        res := FALSE;
        msg := msg || E'\n' || diag(
            E'    Missing records:\n        '
            ||  array_to_string( missing, E'\n        ' )
        );
    END IF;

    RETURN ok(res, $3) || msg;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION _relcomp( TEXT, TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _docomp(
        _temptable( $1, '__taphave__' ),
        _temptable( $2, '__tapwant__' ),
        $3, $4
    );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _relcomp( TEXT, anyarray, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _docomp(
        _temptable( $1, '__taphave__' ),
        _temptable( $2, '__tapwant__' ),
        $3, $4
    );
$$ LANGUAGE sql;

-- set_eq( sql, sql, description )
CREATE OR REPLACE FUNCTION set_eq( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, $3, '' );
$$ LANGUAGE sql;

-- set_eq( sql, sql )
CREATE OR REPLACE FUNCTION set_eq( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, NULL::text, '' );
$$ LANGUAGE sql;

-- set_eq( sql, array, description )
CREATE OR REPLACE FUNCTION set_eq( TEXT, anyarray, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, $3, '' );
$$ LANGUAGE sql;

-- set_eq( sql, array )
CREATE OR REPLACE FUNCTION set_eq( TEXT, anyarray )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, NULL::text, '' );
$$ LANGUAGE sql;

-- bag_eq( sql, sql, description )
CREATE OR REPLACE FUNCTION bag_eq( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, $3, 'ALL ' );
$$ LANGUAGE sql;

-- bag_eq( sql, sql )
CREATE OR REPLACE FUNCTION bag_eq( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, NULL::text, 'ALL ' );
$$ LANGUAGE sql;

-- bag_eq( sql, array, description )
CREATE OR REPLACE FUNCTION bag_eq( TEXT, anyarray, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, $3, 'ALL ' );
$$ LANGUAGE sql;

-- bag_eq( sql, array )
CREATE OR REPLACE FUNCTION bag_eq( TEXT, anyarray )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, NULL::text, 'ALL ' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _do_ne( TEXT, TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    have    ALIAS FOR $1;
    want    ALIAS FOR $2;
    extras  TEXT[]  := '{}';
    missing TEXT[]  := '{}';
    res     BOOLEAN := TRUE;
    msg     TEXT    := '';
BEGIN
    BEGIN
        -- Find extra records.
        EXECUTE 'SELECT EXISTS ( '
             || '( SELECT * FROM ' || have || ' EXCEPT ' || $4
             || '  SELECT * FROM ' || want
             || ' ) UNION ( '
             || '  SELECT * FROM ' || want || ' EXCEPT ' || $4
             || '  SELECT * FROM ' || have
             || ' ) LIMIT 1 )' INTO res;

        -- Drop the temporary tables.
        EXECUTE 'DROP TABLE ' || have;
        EXECUTE 'DROP TABLE ' || want;
    EXCEPTION WHEN syntax_error OR datatype_mismatch THEN
        msg := E'\n' || diag(
            E'    Columns differ between queries:\n'
            || '        have: (' || _temptypes(have) || E')\n'
            || '        want: (' || _temptypes(want) || ')'
        );
        EXECUTE 'DROP TABLE ' || have;
        EXECUTE 'DROP TABLE ' || want;
        RETURN ok(FALSE, $3) || msg;
    END;

    -- Return the value from the query.
    RETURN ok(res, $3);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION _relne( TEXT, TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _do_ne(
        _temptable( $1, '__taphave__' ),
        _temptable( $2, '__tapwant__' ),
        $3, $4
    );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _relne( TEXT, anyarray, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _do_ne(
        _temptable( $1, '__taphave__' ),
        _temptable( $2, '__tapwant__' ),
        $3, $4
    );
$$ LANGUAGE sql;

-- set_ne( sql, sql, description )
CREATE OR REPLACE FUNCTION set_ne( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relne( $1, $2, $3, '' );
$$ LANGUAGE sql;

-- set_ne( sql, sql )
CREATE OR REPLACE FUNCTION set_ne( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relne( $1, $2, NULL::text, '' );
$$ LANGUAGE sql;

-- set_ne( sql, array, description )
CREATE OR REPLACE FUNCTION set_ne( TEXT, anyarray, TEXT )
RETURNS TEXT AS $$
    SELECT _relne( $1, $2, $3, '' );
$$ LANGUAGE sql;

-- set_ne( sql, array )
CREATE OR REPLACE FUNCTION set_ne( TEXT, anyarray )
RETURNS TEXT AS $$
    SELECT _relne( $1, $2, NULL::text, '' );
$$ LANGUAGE sql;

-- bag_ne( sql, sql, description )
CREATE OR REPLACE FUNCTION bag_ne( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relne( $1, $2, $3, 'ALL ' );
$$ LANGUAGE sql;

-- bag_ne( sql, sql )
CREATE OR REPLACE FUNCTION bag_ne( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relne( $1, $2, NULL::text, 'ALL ' );
$$ LANGUAGE sql;

-- bag_ne( sql, array, description )
CREATE OR REPLACE FUNCTION bag_ne( TEXT, anyarray, TEXT )
RETURNS TEXT AS $$
    SELECT _relne( $1, $2, $3, 'ALL ' );
$$ LANGUAGE sql;

-- bag_ne( sql, array )
CREATE OR REPLACE FUNCTION bag_ne( TEXT, anyarray )
RETURNS TEXT AS $$
    SELECT _relne( $1, $2, NULL::text, 'ALL ' );
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _relcomp( TEXT, TEXT, TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    have    TEXT    := _temptable( $1, '__taphave__' );
    want    TEXT    := _temptable( $2, '__tapwant__' );
    results TEXT[]  := '{}';
    res     BOOLEAN := TRUE;
    msg     TEXT    := '';
    rec     RECORD;
BEGIN
    BEGIN
        -- Find relevant records.
        FOR rec in EXECUTE 'SELECT * FROM ' || want || ' ' || $4
                       || ' SELECT * FROM ' || have LOOP
            results := results || rec::text;
        END LOOP;

        -- Drop the temporary tables.
        EXECUTE 'DROP TABLE ' || have;
        EXECUTE 'DROP TABLE ' || want;
    EXCEPTION WHEN syntax_error OR datatype_mismatch THEN
        msg := E'\n' || diag(
            E'    Columns differ between queries:\n'
            || '        have: (' || _temptypes(have) || E')\n'
            || '        want: (' || _temptypes(want) || ')'
        );
        EXECUTE 'DROP TABLE ' || have;
        EXECUTE 'DROP TABLE ' || want;
        RETURN ok(FALSE, $3) || msg;
    END;

    -- What records do we have?
    IF results[1] IS NOT NULL THEN
        res := FALSE;
        msg := msg || E'\n' || diag(
            '    ' || $5 || E' records:\n        '
            ||  array_to_string( results, E'\n        ' )
        );
    END IF;

    RETURN ok(res, $3) || msg;
END;
$$ LANGUAGE plpgsql;

-- set_has( sql, sql, description )
CREATE OR REPLACE FUNCTION set_has( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, $3, 'EXCEPT', 'Missing' );
$$ LANGUAGE sql;

-- set_has( sql, sql )
CREATE OR REPLACE FUNCTION set_has( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, NULL::TEXT, 'EXCEPT', 'Missing' );
$$ LANGUAGE sql;

-- bag_has( sql, sql, description )
CREATE OR REPLACE FUNCTION bag_has( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, $3, 'EXCEPT ALL', 'Missing' );
$$ LANGUAGE sql;

-- bag_has( sql, sql )
CREATE OR REPLACE FUNCTION bag_has( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, NULL::TEXT, 'EXCEPT ALL', 'Missing' );
$$ LANGUAGE sql;

-- set_hasnt( sql, sql, description )
CREATE OR REPLACE FUNCTION set_hasnt( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, $3, 'INTERSECT', 'Extra' );
$$ LANGUAGE sql;

-- set_hasnt( sql, sql )
CREATE OR REPLACE FUNCTION set_hasnt( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, NULL::TEXT, 'INTERSECT', 'Extra' );
$$ LANGUAGE sql;

-- bag_hasnt( sql, sql, description )
CREATE OR REPLACE FUNCTION bag_hasnt( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, $3, 'INTERSECT ALL', 'Extra' );
$$ LANGUAGE sql;

-- bag_hasnt( sql, sql )
CREATE OR REPLACE FUNCTION bag_hasnt( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT _relcomp( $1, $2, NULL::TEXT, 'INTERSECT ALL', 'Extra' );
$$ LANGUAGE sql;

-- results_eq( cursor, cursor, description )
CREATE OR REPLACE FUNCTION results_eq( refcursor, refcursor, text )
RETURNS TEXT AS $$
DECLARE
    have       ALIAS FOR $1;
    want       ALIAS FOR $2;
    have_rec   RECORD;
    want_rec   RECORD;
    have_found BOOLEAN;
    want_found BOOLEAN;
    rownum     INTEGER := 1;
BEGIN
    FETCH have INTO have_rec;
    have_found := FOUND;
    FETCH want INTO want_rec;
    want_found := FOUND;
    WHILE have_found OR want_found LOOP
        IF have_rec IS DISTINCT FROM want_rec OR have_found <> want_found THEN
            RETURN ok( false, $3 ) || E'\n' || diag(
                '    Results differ beginning at row ' || rownum || E':\n' ||
                '        have: ' || CASE WHEN have_found THEN have_rec::text ELSE 'NULL' END || E'\n' ||
                '        want: ' || CASE WHEN want_found THEN want_rec::text ELSE 'NULL' END
            );
        END IF;
        rownum = rownum + 1;
        FETCH have INTO have_rec;
        have_found := FOUND;
        FETCH want INTO want_rec;
        want_found := FOUND;
    END LOOP;

    RETURN ok( true, $3 );
EXCEPTION
    WHEN datatype_mismatch THEN
        RETURN ok( false, $3 ) || E'\n' || diag(
            E'    Columns differ between queries:\n' ||
            '        have: ' || CASE WHEN have_found THEN have_rec::text ELSE 'NULL' END || E'\n' ||
            '        want: ' || CASE WHEN want_found THEN want_rec::text ELSE 'NULL' END
        );
END;
$$ LANGUAGE plpgsql;

-- results_eq( cursor, cursor )
CREATE OR REPLACE FUNCTION results_eq( refcursor, refcursor )
RETURNS TEXT AS $$
    SELECT results_eq( $1, $2, NULL::text );
$$ LANGUAGE sql;

-- results_eq( sql, sql, description )
CREATE OR REPLACE FUNCTION results_eq( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    have REFCURSOR;
    want REFCURSOR;
    res  TEXT;
BEGIN
    OPEN have FOR EXECUTE _query($1);
    OPEN want FOR EXECUTE _query($2);
    res := results_eq(have, want, $3);
    CLOSE have;
    CLOSE want;
    RETURN res;
END;
$$ LANGUAGE plpgsql;

-- results_eq( sql, sql )
CREATE OR REPLACE FUNCTION results_eq( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT results_eq( $1, $2, NULL::text );
$$ LANGUAGE sql;

-- results_eq( sql, array, description )
CREATE OR REPLACE FUNCTION results_eq( TEXT, anyarray, TEXT )
RETURNS TEXT AS $$
DECLARE
    have REFCURSOR;
    want REFCURSOR;
    res  TEXT;
BEGIN
    OPEN have FOR EXECUTE _query($1);
    OPEN want FOR SELECT $2[i]
    FROM generate_series(array_lower($2, 1), array_upper($2, 1)) s(i);
    res := results_eq(have, want, $3);
    CLOSE have;
    CLOSE want;
    RETURN res;
END;
$$ LANGUAGE plpgsql;

-- results_eq( sql, array )
CREATE OR REPLACE FUNCTION results_eq( TEXT, anyarray )
RETURNS TEXT AS $$
    SELECT results_eq( $1, $2, NULL::text );
$$ LANGUAGE sql;

-- results_eq( sql, cursor, description )
CREATE OR REPLACE FUNCTION results_eq( TEXT, refcursor, TEXT )
RETURNS TEXT AS $$
DECLARE
    have REFCURSOR;
    res  TEXT;
BEGIN
    OPEN have FOR EXECUTE _query($1);
    res := results_eq(have, $2, $3);
    CLOSE have;
    RETURN res;
END;
$$ LANGUAGE plpgsql;

-- results_eq( sql, cursor )
CREATE OR REPLACE FUNCTION results_eq( TEXT, refcursor )
RETURNS TEXT AS $$
    SELECT results_eq( $1, $2, NULL::text );
$$ LANGUAGE sql;

-- results_eq( cursor, sql, description )
CREATE OR REPLACE FUNCTION results_eq( refcursor, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    want REFCURSOR;
    res  TEXT;
BEGIN
    OPEN want FOR EXECUTE _query($2);
    res := results_eq($1, want, $3);
    CLOSE want;
    RETURN res;
END;
$$ LANGUAGE plpgsql;

-- results_eq( cursor, sql )
CREATE OR REPLACE FUNCTION results_eq( refcursor, TEXT )
RETURNS TEXT AS $$
    SELECT results_eq( $1, $2, NULL::text );
$$ LANGUAGE sql;

-- results_eq( cursor, array, description )
CREATE OR REPLACE FUNCTION results_eq( refcursor, anyarray, TEXT )
RETURNS TEXT AS $$
DECLARE
    want REFCURSOR;
    res  TEXT;
BEGIN
    OPEN want FOR SELECT $2[i]
    FROM generate_series(array_lower($2, 1), array_upper($2, 1)) s(i);
    res := results_eq($1, want, $3);
    CLOSE want;
    RETURN res;
END;
$$ LANGUAGE plpgsql;

-- results_eq( cursor, array )
CREATE OR REPLACE FUNCTION results_eq( refcursor, anyarray )
RETURNS TEXT AS $$
    SELECT results_eq( $1, $2, NULL::text );
$$ LANGUAGE sql;

-- results_ne( cursor, cursor, description )
CREATE OR REPLACE FUNCTION results_ne( refcursor, refcursor, text )
RETURNS TEXT AS $$
DECLARE
    have       ALIAS FOR $1;
    want       ALIAS FOR $2;
    have_rec   RECORD;
    want_rec   RECORD;
    have_found BOOLEAN;
    want_found BOOLEAN;
BEGIN
    FETCH have INTO have_rec;
    have_found := FOUND;
    FETCH want INTO want_rec;
    want_found := FOUND;
    WHILE have_found OR want_found LOOP
        IF have_rec IS DISTINCT FROM want_rec OR have_found <> want_found THEN
            RETURN ok( true, $3 );
        ELSE
            FETCH have INTO have_rec;
            have_found := FOUND;
            FETCH want INTO want_rec;
            want_found := FOUND;
        END IF;
    END LOOP;
    RETURN ok( false, $3 );
EXCEPTION
    WHEN datatype_mismatch THEN
        RETURN ok( false, $3 ) || E'\n' || diag(
            E'    Columns differ between queries:\n' ||
            '        have: ' || CASE WHEN have_found THEN have_rec::text ELSE 'NULL' END || E'\n' ||
            '        want: ' || CASE WHEN want_found THEN want_rec::text ELSE 'NULL' END
        );
END;
$$ LANGUAGE plpgsql;

-- results_ne( cursor, cursor )
CREATE OR REPLACE FUNCTION results_ne( refcursor, refcursor )
RETURNS TEXT AS $$
    SELECT results_ne( $1, $2, NULL::text );
$$ LANGUAGE sql;

-- results_ne( sql, sql, description )
CREATE OR REPLACE FUNCTION results_ne( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    have REFCURSOR;
    want REFCURSOR;
    res  TEXT;
BEGIN
    OPEN have FOR EXECUTE _query($1);
    OPEN want FOR EXECUTE _query($2);
    res := results_ne(have, want, $3);
    CLOSE have;
    CLOSE want;
    RETURN res;
END;
$$ LANGUAGE plpgsql;

-- results_ne( sql, sql )
CREATE OR REPLACE FUNCTION results_ne( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT results_ne( $1, $2, NULL::text );
$$ LANGUAGE sql;

-- results_ne( sql, array, description )
CREATE OR REPLACE FUNCTION results_ne( TEXT, anyarray, TEXT )
RETURNS TEXT AS $$
DECLARE
    have REFCURSOR;
    want REFCURSOR;
    res  TEXT;
BEGIN
    OPEN have FOR EXECUTE _query($1);
    OPEN want FOR SELECT $2[i]
    FROM generate_series(array_lower($2, 1), array_upper($2, 1)) s(i);
    res := results_ne(have, want, $3);
    CLOSE have;
    CLOSE want;
    RETURN res;
END;
$$ LANGUAGE plpgsql;

-- results_ne( sql, array )
CREATE OR REPLACE FUNCTION results_ne( TEXT, anyarray )
RETURNS TEXT AS $$
    SELECT results_ne( $1, $2, NULL::text );
$$ LANGUAGE sql;

-- results_ne( sql, cursor, description )
CREATE OR REPLACE FUNCTION results_ne( TEXT, refcursor, TEXT )
RETURNS TEXT AS $$
DECLARE
    have REFCURSOR;
    res  TEXT;
BEGIN
    OPEN have FOR EXECUTE _query($1);
    res := results_ne(have, $2, $3);
    CLOSE have;
    RETURN res;
END;
$$ LANGUAGE plpgsql;

-- results_ne( sql, cursor )
CREATE OR REPLACE FUNCTION results_ne( TEXT, refcursor )
RETURNS TEXT AS $$
    SELECT results_ne( $1, $2, NULL::text );
$$ LANGUAGE sql;

-- results_ne( cursor, sql, description )
CREATE OR REPLACE FUNCTION results_ne( refcursor, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    want REFCURSOR;
    res  TEXT;
BEGIN
    OPEN want FOR EXECUTE _query($2);
    res := results_ne($1, want, $3);
    CLOSE want;
    RETURN res;
END;
$$ LANGUAGE plpgsql;

-- results_ne( cursor, sql )
CREATE OR REPLACE FUNCTION results_ne( refcursor, TEXT )
RETURNS TEXT AS $$
    SELECT results_ne( $1, $2, NULL::text );
$$ LANGUAGE sql;

-- results_ne( cursor, array, description )
CREATE OR REPLACE FUNCTION results_ne( refcursor, anyarray, TEXT )
RETURNS TEXT AS $$
DECLARE
    want REFCURSOR;
    res  TEXT;
BEGIN
    OPEN want FOR SELECT $2[i]
    FROM generate_series(array_lower($2, 1), array_upper($2, 1)) s(i);
    res := results_ne($1, want, $3);
    CLOSE want;
    RETURN res;
END;
$$ LANGUAGE plpgsql;

-- results_ne( cursor, array )
CREATE OR REPLACE FUNCTION results_ne( refcursor, anyarray )
RETURNS TEXT AS $$
    SELECT results_ne( $1, $2, NULL::text );
$$ LANGUAGE sql;

-- isa_ok( value, regtype, description )
CREATE OR REPLACE FUNCTION isa_ok( anyelement, regtype, TEXT )
RETURNS TEXT AS $$
DECLARE
    typeof regtype := pg_typeof($1);
BEGIN
    IF typeof = $2 THEN RETURN ok(true, $3 || ' isa ' || $2 ); END IF;
    RETURN ok(false, $3 || ' isa ' || $2 ) || E'\n' ||
        diag('    ' || $3 || ' isn''t a "' || $2 || '" it''s a "' || typeof || '"');
END;
$$ LANGUAGE plpgsql;

-- isa_ok( value, regtype )
CREATE OR REPLACE FUNCTION isa_ok( anyelement, regtype )
RETURNS TEXT AS $$
    SELECT isa_ok($1, $2, 'the value');
$$ LANGUAGE sql;

-- is_empty( sql, description )
CREATE OR REPLACE FUNCTION is_empty( TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    extras  TEXT[]  := '{}';
    res     BOOLEAN := TRUE;
    msg     TEXT    := '';
    rec     RECORD;
BEGIN
    -- Find extra records.
    FOR rec in EXECUTE _query($1) LOOP
        extras := extras || rec::text;
    END LOOP;

    -- What extra records do we have?
    IF extras[1] IS NOT NULL THEN
        res := FALSE;
        msg := E'\n' || diag(
            E'    Unexpected records:\n        '
            ||  array_to_string( extras, E'\n        ' )
        );
    END IF;

    RETURN ok(res, $2) || msg;
END;
$$ LANGUAGE plpgsql;

-- is_empty( sql )
CREATE OR REPLACE FUNCTION is_empty( TEXT )
RETURNS TEXT AS $$
    SELECT is_empty( $1, NULL );
$$ LANGUAGE sql;

-- collect_tap( tap, tap, tap )
CREATE OR REPLACE FUNCTION collect_tap( VARIADIC text[] )
RETURNS TEXT AS $$
    SELECT array_to_string($1, E'\n');
$$ LANGUAGE sql;

-- collect_tap( tap[] )
CREATE OR REPLACE FUNCTION collect_tap( VARCHAR[] )
RETURNS TEXT AS $$
    SELECT array_to_string($1, E'\n');
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _tlike ( BOOLEAN, TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT ok( $1, $4 ) || CASE WHEN $1 THEN '' ELSE E'\n' || diag(
           '   error message: ' || COALESCE( quote_literal($2), 'NULL' ) ||
       E'\n   doesn''t match: ' || COALESCE( quote_literal($3), 'NULL' )
    ) END;
$$ LANGUAGE sql;

-- throws_like ( sql, pattern, description )
CREATE OR REPLACE FUNCTION throws_like ( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
BEGIN
    EXECUTE _query($1);
    RETURN ok( FALSE, $3 ) || E'\n' || diag( '    no exception thrown' );
EXCEPTION WHEN OTHERS THEN
    return _tlike( SQLERRM ~~ $2, SQLERRM, $2, $3 );
END;
$$ LANGUAGE plpgsql;

-- throws_like ( sql, pattern )
CREATE OR REPLACE FUNCTION throws_like ( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT throws_like($1, $2, 'Should throw exception like ' || quote_literal($2) );
$$ LANGUAGE sql;

-- throws_ilike ( sql, pattern, description )
CREATE OR REPLACE FUNCTION throws_ilike ( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
BEGIN
    EXECUTE _query($1);
    RETURN ok( FALSE, $3 ) || E'\n' || diag( '    no exception thrown' );
EXCEPTION WHEN OTHERS THEN
    return _tlike( SQLERRM ~~* $2, SQLERRM, $2, $3 );
END;
$$ LANGUAGE plpgsql;

-- throws_ilike ( sql, pattern )
CREATE OR REPLACE FUNCTION throws_ilike ( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT throws_ilike($1, $2, 'Should throw exception like ' || quote_literal($2) );
$$ LANGUAGE sql;

-- throws_matching ( sql, pattern, description )
CREATE OR REPLACE FUNCTION throws_matching ( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
BEGIN
    EXECUTE _query($1);
    RETURN ok( FALSE, $3 ) || E'\n' || diag( '    no exception thrown' );
EXCEPTION WHEN OTHERS THEN
    return _tlike( SQLERRM ~ $2, SQLERRM, $2, $3 );
END;
$$ LANGUAGE plpgsql;

-- throws_matching ( sql, pattern )
CREATE OR REPLACE FUNCTION throws_matching ( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT throws_matching($1, $2, 'Should throw exception matching ' || quote_literal($2) );
$$ LANGUAGE sql;

-- throws_imatching ( sql, pattern, description )
CREATE OR REPLACE FUNCTION throws_imatching ( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
BEGIN
    EXECUTE _query($1);
    RETURN ok( FALSE, $3 ) || E'\n' || diag( '    no exception thrown' );
EXCEPTION WHEN OTHERS THEN
    return _tlike( SQLERRM ~* $2, SQLERRM, $2, $3 );
END;
$$ LANGUAGE plpgsql;

-- throws_imatching ( sql, pattern )
CREATE OR REPLACE FUNCTION throws_imatching ( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT throws_imatching($1, $2, 'Should throw exception matching ' || quote_literal($2) );
$$ LANGUAGE sql;

-- roles_are( roles[], description )
CREATE OR REPLACE FUNCTION roles_are( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'roles',
        ARRAY(
            SELECT rolname
              FROM pg_catalog.pg_roles
            EXCEPT
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
        ),
        ARRAY(
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
            EXCEPT
            SELECT rolname
              FROM pg_catalog.pg_roles
        ),
        $2
    );
$$ LANGUAGE SQL;

-- roles_are( roles[] )
CREATE OR REPLACE FUNCTION roles_are( NAME[] )
RETURNS TEXT AS $$
    SELECT roles_are( $1, 'There should be the correct roles' );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _types_are ( NAME, NAME[], TEXT, CHAR[] )
RETURNS TEXT AS $$
    SELECT _are(
        'types',
        ARRAY(
            SELECT t.typname
              FROM pg_catalog.pg_type t
              LEFT JOIN pg_catalog.pg_namespace n ON n.oid = t.typnamespace
             WHERE (
                     t.typrelid = 0
                 OR (SELECT c.relkind = 'c' FROM pg_catalog.pg_class c WHERE c.oid = t.typrelid)
             )
               AND NOT EXISTS(SELECT 1 FROM pg_catalog.pg_type el WHERE el.oid = t.typelem AND el.typarray = t.oid)
               AND n.nspname = $1
               AND t.typtype = ANY( COALESCE($4, ARRAY['b', 'c', 'd', 'p', 'e']) )
            EXCEPT
            SELECT $2[i]
              FROM generate_series(1, array_upper($2, 1)) s(i)
        ),
        ARRAY(
            SELECT $2[i]
               FROM generate_series(1, array_upper($2, 1)) s(i)
            EXCEPT
            SELECT t.typname
              FROM pg_catalog.pg_type t
              LEFT JOIN pg_catalog.pg_namespace n ON n.oid = t.typnamespace
             WHERE (
                     t.typrelid = 0
                 OR (SELECT c.relkind = 'c' FROM pg_catalog.pg_class c WHERE c.oid = t.typrelid)
             )
               AND NOT EXISTS(SELECT 1 FROM pg_catalog.pg_type el WHERE el.oid = t.typelem AND el.typarray = t.oid)
               AND n.nspname = $1
               AND t.typtype = ANY( COALESCE($4, ARRAY['b', 'c', 'd', 'p', 'e']) )
        ),
        $3
    );
$$ LANGUAGE SQL;

-- types_are( schema, types[], description )
CREATE OR REPLACE FUNCTION types_are ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _types_are( $1, $2, $3, NULL );
$$ LANGUAGE SQL;

-- types_are( schema, types[] )
CREATE OR REPLACE FUNCTION types_are ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT _types_are( $1, $2, 'Schema ' || quote_ident($1) || ' should have the correct types', NULL );
$$ LANGUAGE SQL;

-- types_are( types[], description )
CREATE OR REPLACE FUNCTION _types_are ( NAME[], TEXT, CHAR[] )
RETURNS TEXT AS $$
    SELECT _are(
        'types',
        ARRAY(
            SELECT t.typname
              FROM pg_catalog.pg_type t
              LEFT JOIN pg_catalog.pg_namespace n ON n.oid = t.typnamespace
             WHERE (
                     t.typrelid = 0
                 OR (SELECT c.relkind = 'c' FROM pg_catalog.pg_class c WHERE c.oid = t.typrelid)
             )
               AND NOT EXISTS(SELECT 1 FROM pg_catalog.pg_type el WHERE el.oid = t.typelem AND el.typarray = t.oid)
               AND n.nspname NOT IN ('pg_catalog', 'information_schema')
               AND pg_catalog.pg_type_is_visible(t.oid)
               AND t.typtype = ANY( COALESCE($3, ARRAY['b', 'c', 'd', 'p', 'e']) )
            EXCEPT
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
        ),
        ARRAY(
            SELECT $1[i]
               FROM generate_series(1, array_upper($1, 1)) s(i)
            EXCEPT
            SELECT t.typname
              FROM pg_catalog.pg_type t
              LEFT JOIN pg_catalog.pg_namespace n ON n.oid = t.typnamespace
             WHERE (
                     t.typrelid = 0
                 OR (SELECT c.relkind = 'c' FROM pg_catalog.pg_class c WHERE c.oid = t.typrelid)
             )
               AND NOT EXISTS(SELECT 1 FROM pg_catalog.pg_type el WHERE el.oid = t.typelem AND el.typarray = t.oid)
               AND n.nspname NOT IN ('pg_catalog', 'information_schema')
               AND pg_catalog.pg_type_is_visible(t.oid)
               AND t.typtype = ANY( COALESCE($3, ARRAY['b', 'c', 'd', 'p', 'e']) )
        ),
        $2
    );
$$ LANGUAGE SQL;


-- types_are( types[], description )
CREATE OR REPLACE FUNCTION types_are ( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _types_are( $1, $2, NULL );
$$ LANGUAGE SQL;

-- types_are( types[] )
CREATE OR REPLACE FUNCTION types_are ( NAME[] )
RETURNS TEXT AS $$
    SELECT _types_are( $1, 'Search path ' || pg_catalog.current_setting('search_path') || ' should have the correct types', NULL );
$$ LANGUAGE SQL;

-- domains_are( schema, domains[], description )
CREATE OR REPLACE FUNCTION domains_are ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _types_are( $1, $2, $3, ARRAY['d'] );
$$ LANGUAGE SQL;

-- domains_are( schema, domains[] )
CREATE OR REPLACE FUNCTION domains_are ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT _types_are( $1, $2, 'Schema ' || quote_ident($1) || ' should have the correct domains', ARRAY['d'] );
$$ LANGUAGE SQL;

-- domains_are( domains[], description )
CREATE OR REPLACE FUNCTION domains_are ( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _types_are( $1, $2, ARRAY['d'] );
$$ LANGUAGE SQL;

-- domains_are( domains[] )
CREATE OR REPLACE FUNCTION domains_are ( NAME[] )
RETURNS TEXT AS $$
    SELECT _types_are( $1, 'Search path ' || pg_catalog.current_setting('search_path') || ' should have the correct domains', ARRAY['d'] );
$$ LANGUAGE SQL;

-- enums_are( schema, enums[], description )
CREATE OR REPLACE FUNCTION enums_are ( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _types_are( $1, $2, $3, ARRAY['e'] );
$$ LANGUAGE SQL;

-- enums_are( schema, enums[] )
CREATE OR REPLACE FUNCTION enums_are ( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT _types_are( $1, $2, 'Schema ' || quote_ident($1) || ' should have the correct enums', ARRAY['e'] );
$$ LANGUAGE SQL;

-- enums_are( enums[], description )
CREATE OR REPLACE FUNCTION enums_are ( NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _types_are( $1, $2, ARRAY['e'] );
$$ LANGUAGE SQL;

-- enums_are( enums[] )
CREATE OR REPLACE FUNCTION enums_are ( NAME[] )
RETURNS TEXT AS $$
    SELECT _types_are( $1, 'Search path ' || pg_catalog.current_setting('search_path') || ' should have the correct enums', ARRAY['e'] );
$$ LANGUAGE SQL;

-- _dexists( schema, domain )
CREATE OR REPLACE FUNCTION _dexists ( NAME, NAME )
RETURNS BOOLEAN AS $$
   SELECT EXISTS(
       SELECT true
         FROM pg_catalog.pg_namespace n
         JOIN pg_catalog.pg_type t on n.oid = t.typnamespace
        WHERE n.nspname = $1
          AND t.typname = $2
   );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _dexists ( NAME )
RETURNS BOOLEAN AS $$
   SELECT EXISTS(
       SELECT true
         FROM pg_catalog.pg_type t
        WHERE t.typname = $1
          AND pg_catalog.pg_type_is_visible(t.oid)
   );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _get_dtype( NAME, TEXT, BOOLEAN )
RETURNS TEXT AS $$
    SELECT display_type(CASE WHEN $3 THEN tn.nspname ELSE NULL END, t.oid, t.typtypmod)
      FROM pg_catalog.pg_type d
      JOIN pg_catalog.pg_namespace dn ON d.typnamespace = dn.oid
      JOIN pg_catalog.pg_type t       ON d.typbasetype  = t.oid
      JOIN pg_catalog.pg_namespace tn ON d.typnamespace = tn.oid
     WHERE d.typisdefined
       AND dn.nspname = $1
       AND d.typname  = LOWER($2)
       AND d.typtype  = 'd'
$$ LANGUAGE sql;

CREATE OR REPLACE FUNCTION _get_dtype( NAME )
RETURNS TEXT AS $$
    SELECT display_type(t.oid, t.typtypmod)
      FROM pg_catalog.pg_type d
      JOIN pg_catalog.pg_type t  ON d.typbasetype  = t.oid
     WHERE d.typisdefined
       AND d.typname = LOWER($1)
       AND d.typtype = 'd'
$$ LANGUAGE sql;

-- domain_type_is( schema, domain, schema, type, description )
CREATE OR REPLACE FUNCTION domain_type_is( NAME, TEXT, NAME, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    actual_type TEXT := _get_dtype($1, $2, true);
BEGIN
    IF actual_type IS NULL THEN
        RETURN fail( $5 ) || E'\n' || diag (
            '   Domain ' || quote_ident($1) || '.' || $2
            || ' does not exist'
        );
    END IF;

    RETURN is( actual_type, quote_ident($3) || '.' || _quote_ident_like($4, actual_type), $5 );
END;
$$ LANGUAGE plpgsql;

-- domain_type_is( schema, domain, schema, type )
CREATE OR REPLACE FUNCTION domain_type_is( NAME, TEXT, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT domain_type_is(
        $1, $2, $3, $4,
        'Domain ' || quote_ident($1) || '.' || $2
        || ' should extend type ' || quote_ident($3) || '.' || $4
    );
$$ LANGUAGE SQL;

-- domain_type_is( schema, domain, type, description )
CREATE OR REPLACE FUNCTION domain_type_is( NAME, TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    actual_type TEXT := _get_dtype($1, $2, false);
BEGIN
    IF actual_type IS NULL THEN
        RETURN fail( $4 ) || E'\n' || diag (
            '   Domain ' || quote_ident($1) || '.' || $2
            || ' does not exist'
        );
    END IF;

    RETURN is( actual_type, _quote_ident_like($3, actual_type), $4 );
END;
$$ LANGUAGE plpgsql;

-- domain_type_is( schema, domain, type )
CREATE OR REPLACE FUNCTION domain_type_is( NAME, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT domain_type_is(
        $1, $2, $3,
        'Domain ' || quote_ident($1) || '.' || $2
        || ' should extend type ' || $3
    );
$$ LANGUAGE SQL;

-- domain_type_is( domain, type, description )
CREATE OR REPLACE FUNCTION domain_type_is( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    actual_type TEXT := _get_dtype($1);
BEGIN
    IF actual_type IS NULL THEN
        RETURN fail( $3 ) || E'\n' || diag (
            '   Domain ' ||  $1 || ' does not exist'
        );
    END IF;

    RETURN is( actual_type, _quote_ident_like($2, actual_type), $3 );
END;
$$ LANGUAGE plpgsql;

-- domain_type_is( domain, type )
CREATE OR REPLACE FUNCTION domain_type_is( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT domain_type_is(
        $1, $2,
        'Domain ' || $1 || ' should extend type ' || $2
    );
$$ LANGUAGE SQL;

-- domain_type_isnt( schema, domain, schema, type, description )
CREATE OR REPLACE FUNCTION domain_type_isnt( NAME, TEXT, NAME, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    actual_type TEXT := _get_dtype($1, $2, true);
BEGIN
    IF actual_type IS NULL THEN
        RETURN fail( $5 ) || E'\n' || diag (
            '   Domain ' || quote_ident($1) || '.' || $2
            || ' does not exist'
        );
    END IF;

    RETURN isnt( actual_type, quote_ident($3) || '.' || _quote_ident_like($4, actual_type), $5 );
END;
$$ LANGUAGE plpgsql;

-- domain_type_isnt( schema, domain, schema, type )
CREATE OR REPLACE FUNCTION domain_type_isnt( NAME, TEXT, NAME, TEXT )
RETURNS TEXT AS $$
    SELECT domain_type_isnt(
        $1, $2, $3, $4,
        'Domain ' || quote_ident($1) || '.' || $2
        || ' should not extend type ' || quote_ident($3) || '.' || $4
    );
$$ LANGUAGE SQL;

-- domain_type_isnt( schema, domain, type, description )
CREATE OR REPLACE FUNCTION domain_type_isnt( NAME, TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    actual_type TEXT := _get_dtype($1, $2, false);
BEGIN
    IF actual_type IS NULL THEN
        RETURN fail( $4 ) || E'\n' || diag (
            '   Domain ' || quote_ident($1) || '.' || $2
            || ' does not exist'
        );
    END IF;

    RETURN isnt( actual_type, _quote_ident_like($3, actual_type), $4 );
END;
$$ LANGUAGE plpgsql;

-- domain_type_isnt( schema, domain, type )
CREATE OR REPLACE FUNCTION domain_type_isnt( NAME, TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT domain_type_isnt(
        $1, $2, $3,
        'Domain ' || quote_ident($1) || '.' || $2
        || ' should not extend type ' || $3
    );
$$ LANGUAGE SQL;

-- domain_type_isnt( domain, type, description )
CREATE OR REPLACE FUNCTION domain_type_isnt( TEXT, TEXT, TEXT )
RETURNS TEXT AS $$
DECLARE
    actual_type TEXT := _get_dtype($1);
BEGIN
    IF actual_type IS NULL THEN
        RETURN fail( $3 ) || E'\n' || diag (
            '   Domain ' ||  $1 || ' does not exist'
        );
    END IF;

    RETURN isnt( actual_type, _quote_ident_like($2, actual_type), $3 );
END;
$$ LANGUAGE plpgsql;

-- domain_type_isnt( domain, type )
CREATE OR REPLACE FUNCTION domain_type_isnt( TEXT, TEXT )
RETURNS TEXT AS $$
    SELECT domain_type_isnt(
        $1, $2,
        'Domain ' || $1 || ' should not extend type ' || $2
    );
$$ LANGUAGE SQL;

-- row_eq( sql, record, description )
CREATE OR REPLACE FUNCTION row_eq( TEXT, anyelement, TEXT )
RETURNS TEXT AS $$
DECLARE
    rec    RECORD;
BEGIN
    EXECUTE _query($1) INTO rec;
    IF NOT rec IS DISTINCT FROM $2 THEN RETURN ok(true, $3); END IF;
    RETURN ok(false, $3 ) || E'\n' || diag(
           '        have: ' || CASE WHEN rec IS NULL THEN 'NULL' ELSE rec::text END ||
        E'\n        want: ' || CASE WHEN $2  IS NULL THEN 'NULL' ELSE $2::text  END
    );
END;
$$ LANGUAGE plpgsql;

-- row_eq( sql, record )
CREATE OR REPLACE FUNCTION row_eq( TEXT, anyelement )
RETURNS TEXT AS $$
    SELECT row_eq($1, $2, NULL );
$$ LANGUAGE sql;

-- triggers_are( schema, table, triggers[], description )
CREATE OR REPLACE FUNCTION triggers_are( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'triggers',
        ARRAY(
            SELECT t.tgname
              FROM pg_catalog.pg_trigger t
              JOIN pg_catalog.pg_class c     ON c.oid = t.tgrelid
              JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
             WHERE n.nspname = $1
               AND c.relname = $2
            EXCEPT
            SELECT $3[i]
              FROM generate_series(1, array_upper($3, 1)) s(i)
        ),
        ARRAY(
            SELECT $3[i]
              FROM generate_series(1, array_upper($3, 1)) s(i)
            EXCEPT
            SELECT t.tgname
              FROM pg_catalog.pg_trigger t
              JOIN pg_catalog.pg_class c     ON c.oid = t.tgrelid
              JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
             WHERE n.nspname = $1
               AND c.relname = $2
        ),
        $4
    );
$$ LANGUAGE SQL;

-- triggers_are( schema, table, triggers[] )
CREATE OR REPLACE FUNCTION triggers_are( NAME, NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT triggers_are( $1, $2, $3, 'Table ' || quote_ident($1) || '.' || quote_ident($2) || ' should have the correct triggers' );
$$ LANGUAGE SQL;

-- triggers_are( table, triggers[], description )
CREATE OR REPLACE FUNCTION triggers_are( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'triggers',
        ARRAY(
            SELECT t.tgname
              FROM pg_catalog.pg_trigger t
              JOIN pg_catalog.pg_class c ON c.oid = t.tgrelid
              JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
             WHERE c.relname = $1
               AND n.nspname NOT IN ('pg_catalog', 'information_schema')
            EXCEPT
            SELECT $2[i]
              FROM generate_series(1, array_upper($2, 1)) s(i)
        ),
        ARRAY(
            SELECT $2[i]
              FROM generate_series(1, array_upper($2, 1)) s(i)
            EXCEPT
            SELECT t.tgname
              FROM pg_catalog.pg_trigger t
              JOIN pg_catalog.pg_class c ON c.oid = t.tgrelid
              JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
               AND n.nspname NOT IN ('pg_catalog', 'information_schema')
        ),
        $3
    );
$$ LANGUAGE SQL;

-- triggers_are( table, triggers[] )
CREATE OR REPLACE FUNCTION triggers_are( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT triggers_are( $1, $2, 'Table ' || quote_ident($1) || ' should have the correct triggers' );
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION _areni ( text, text[], text[], TEXT )
RETURNS TEXT AS $$
DECLARE
    what    ALIAS FOR $1;
    extras  ALIAS FOR $2;
    missing ALIAS FOR $3;
    descr   ALIAS FOR $4;
    msg     TEXT    := '';
    res     BOOLEAN := TRUE;
BEGIN
    IF extras[1] IS NOT NULL THEN
        res = FALSE;
        msg := E'\n' || diag(
            '    Extra ' || what || E':\n        '
            ||  array_to_string( extras, E'\n        ' )
        );
    END IF;
    IF missing[1] IS NOT NULL THEN
        res = FALSE;
        msg := msg || E'\n' || diag(
            '    Missing ' || what || E':\n        '
            ||  array_to_string( missing, E'\n        ' )
        );
    END IF;

    RETURN ok(res, descr) || msg;
END;
$$ LANGUAGE plpgsql;


-- casts_are( casts[], description )
CREATE OR REPLACE FUNCTION casts_are ( TEXT[], TEXT )
RETURNS TEXT AS $$
    SELECT _areni(
        'casts',
        ARRAY(
            SELECT display_type(castsource, NULL) || ' AS ' || display_type(casttarget, NULL)
              FROM pg_catalog.pg_cast c
            EXCEPT
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
        ),
        ARRAY(
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
            EXCEPT
            SELECT display_type(castsource, NULL) || ' AS ' || display_type(casttarget, NULL)
              FROM pg_catalog.pg_cast c
        ),
        $2
    );
$$ LANGUAGE sql;

-- casts_are( casts[] )
CREATE OR REPLACE FUNCTION casts_are ( TEXT[] )
RETURNS TEXT AS $$
    SELECT casts_are( $1, 'There should be the correct casts');
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION display_oper ( NAME, OID )
RETURNS TEXT AS $$
    SELECT $1 || substring($2::regoperator::text, '[(][^)]+[)]$')
$$ LANGUAGE SQL;

-- operators_are( schema, operators[], description )
CREATE OR REPLACE FUNCTION operators_are( NAME, TEXT[], TEXT )
RETURNS TEXT AS $$
    SELECT _areni(
        'operators',
        ARRAY(
            SELECT display_oper(o.oprname, o.oid) || ' RETURNS ' || o.oprresult::regtype
              FROM pg_catalog.pg_operator o
              JOIN pg_catalog.pg_namespace n ON o.oprnamespace = n.oid
             WHERE n.nspname = $1
            EXCEPT
            SELECT $2[i]
              FROM generate_series(1, array_upper($2, 1)) s(i)
        ),
        ARRAY(
            SELECT $2[i]
              FROM generate_series(1, array_upper($2, 1)) s(i)
            EXCEPT
            SELECT display_oper(o.oprname, o.oid) || ' RETURNS ' || o.oprresult::regtype
              FROM pg_catalog.pg_operator o
              JOIN pg_catalog.pg_namespace n ON o.oprnamespace = n.oid
             WHERE n.nspname = $1
        ),
        $3
    );
$$ LANGUAGE SQL;

-- operators_are( schema, operators[] )
CREATE OR REPLACE FUNCTION operators_are ( NAME, TEXT[] )
RETURNS TEXT AS $$
    SELECT operators_are($1, $2, 'Schema ' || quote_ident($1) || ' should have the correct operators' );
$$ LANGUAGE SQL;

-- operators_are( operators[], description )
CREATE OR REPLACE FUNCTION operators_are( TEXT[], TEXT )
RETURNS TEXT AS $$
    SELECT _areni(
        'operators',
        ARRAY(
            SELECT display_oper(o.oprname, o.oid) || ' RETURNS ' || o.oprresult::regtype
              FROM pg_catalog.pg_operator o
              JOIN pg_catalog.pg_namespace n ON o.oprnamespace = n.oid
             WHERE pg_catalog.pg_operator_is_visible(o.oid)
               AND n.nspname NOT IN ('pg_catalog', 'information_schema')
            EXCEPT
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
        ),
        ARRAY(
            SELECT $1[i]
              FROM generate_series(1, array_upper($1, 1)) s(i)
            EXCEPT
            SELECT display_oper(o.oprname, o.oid) || ' RETURNS ' || o.oprresult::regtype
              FROM pg_catalog.pg_operator o
              JOIN pg_catalog.pg_namespace n ON o.oprnamespace = n.oid
             WHERE pg_catalog.pg_operator_is_visible(o.oid)
               AND n.nspname NOT IN ('pg_catalog', 'information_schema')
        ),
        $2
    );
$$ LANGUAGE SQL;

-- operators_are( operators[] )
CREATE OR REPLACE FUNCTION operators_are ( TEXT[] )
RETURNS TEXT AS $$
    SELECT operators_are($1, 'There should be the correct operators')
$$ LANGUAGE SQL;

-- columns_are( schema, table, columns[], description )
CREATE OR REPLACE FUNCTION columns_are( NAME, NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'columns',
        ARRAY(
            SELECT a.attname
              FROM pg_catalog.pg_namespace n
              JOIN pg_catalog.pg_class c ON n.oid = c.relnamespace
              JOIN pg_catalog.pg_attribute a ON c.oid = a.attrelid
             WHERE n.nspname = $1
               AND c.relname = $2
               AND a.attnum > 0
               AND NOT a.attisdropped
            EXCEPT
            SELECT $3[i]
              FROM generate_series(1, array_upper($3, 1)) s(i)
        ),
        ARRAY(
            SELECT $3[i]
              FROM generate_series(1, array_upper($3, 1)) s(i)
            EXCEPT
            SELECT a.attname
              FROM pg_catalog.pg_namespace n
              JOIN pg_catalog.pg_class c ON n.oid = c.relnamespace
              JOIN pg_catalog.pg_attribute a ON c.oid = a.attrelid
             WHERE n.nspname = $1
               AND c.relname = $2
               AND a.attnum > 0
               AND NOT a.attisdropped
        ),
        $4
    );
$$ LANGUAGE SQL;

-- columns_are( schema, table, columns[] )
CREATE OR REPLACE FUNCTION columns_are( NAME, NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT columns_are( $1, $2, $3, 'Table ' || quote_ident($1) || '.' || quote_ident($2) || ' should have the correct columns' );
$$ LANGUAGE SQL;

-- columns_are( table, columns[], description )
CREATE OR REPLACE FUNCTION columns_are( NAME, NAME[], TEXT )
RETURNS TEXT AS $$
    SELECT _are(
        'columns',
        ARRAY(
            SELECT a.attname
              FROM pg_catalog.pg_namespace n
              JOIN pg_catalog.pg_class c ON n.oid = c.relnamespace
              JOIN pg_catalog.pg_attribute a ON c.oid = a.attrelid
             WHERE n.nspname NOT IN ('pg_catalog', 'information_schema')
               AND pg_catalog.pg_table_is_visible(c.oid)
               AND c.relname = $1
               AND a.attnum > 0
               AND NOT a.attisdropped
            EXCEPT
            SELECT $2[i]
              FROM generate_series(1, array_upper($2, 1)) s(i)
        ),
        ARRAY(
            SELECT $2[i]
              FROM generate_series(1, array_upper($2, 1)) s(i)
            EXCEPT
            SELECT a.attname
              FROM pg_catalog.pg_namespace n
              JOIN pg_catalog.pg_class c ON n.oid = c.relnamespace
              JOIN pg_catalog.pg_attribute a ON c.oid = a.attrelid
             WHERE n.nspname NOT IN ('pg_catalog', 'information_schema')
               AND pg_catalog.pg_table_is_visible(c.oid)
               AND c.relname = $1
               AND a.attnum > 0
               AND NOT a.attisdropped
        ),
        $3
    );
$$ LANGUAGE SQL;

-- columns_are( table, columns[] )
CREATE OR REPLACE FUNCTION columns_are( NAME, NAME[] )
RETURNS TEXT AS $$
    SELECT columns_are( $1, $2, 'Table ' || quote_ident($1) || ' should have the correct columns' );
$$ LANGUAGE SQL;

