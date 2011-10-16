 
\i util-include-path.sql

BEGIN;

CREATE TABLE pgtap.test_host_service(
	host text,
	service text[],
	version bigint
);

CREATE FUNCTION test_set()
RETURNS SETOF text
AS
$$
BEGIN
	INSERT INTO pgtap.test_host_service VALUES ('host1', array['www', 'ftp'], 1);
	INSERT INTO pgtap.test_host_service VALUES ('host2', array['www', 'dhcp'], 21);
	INSERT INTO pgtap.test_host_service VALUES ('host2', array['www'], 2);
	INSERT INTO pgtap.test_host_service VALUES ('host1', array['www', 'ftp', 'dhcp'], 3);

	PERFORM startChangeset();
	PERFORM service_add('www');
	PERFORM service_add('ftp');
	PERFORM commitChangeset('2');

	PERFORM startChangeset();
	PERFORM host_add('host1');
	PERFORM host_set_service('host1', ARRAY['www', 'ftp']);

	PREPARE expservices  AS SELECT service FROM pgtap.test_host_service WHERE version = 1 AND host = 'host1';
	PREPARE retservices AS SELECT service FROM host_get_data('host1');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'set services are ok in opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM commitChangeset('3');

	PREPARE expservices  AS SELECT service FROM pgtap.test_host_service WHERE version = 1 AND host = 'host1';
	PREPARE retservices AS SELECT service FROM host_get_data('host1');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'set services are ok after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM startChangeset();
	PERFORM host_add('host2');
	PERFORM service_add('dhcp');
	PERFORM host_set_service('host2', array['www','dhcp']);

	PREPARE expservices  AS SELECT service FROM pgtap.test_host_service WHERE version = 21 AND host = 'host2';
	PREPARE retservices AS SELECT service FROM host_get_data('host2');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'set services are ok in opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM host_set_service('host2', array['www']);

	PREPARE expservices  AS SELECT service FROM pgtap.test_host_service WHERE version = 2 AND host = 'host2';
	PREPARE retservices AS SELECT service FROM host_get_data('host2');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'set services are ok in opened changeset after second change' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM commitChangeset('4');

	PERFORM startChangeset();
	PERFORM host_set_service('host1', ARRAY['www', 'ftp', 'dhcp']);
	PREPARE expservices  AS SELECT service FROM pgtap.test_host_service WHERE version = 3 AND host = 'host1';
	PREPARE retservices AS SELECT service FROM host_get_data('host1');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'set services are ok in opened changeset after changes in different changesets' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	PERFORM commitChangeset('5');

	PREPARE expservices  AS SELECT service FROM pgtap.test_host_service WHERE version = 3 AND host = 'host1';
	PREPARE retservices AS SELECT service FROM host_get_data('host1');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'set services are ok in opened changeset after changes in different changesets' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

END
$$
LANGUAGE plpgsql;



CREATE TABLE pgtap.test_host_service_2(
	host text,
	service text[],
	version bigint
);

CREATE FUNCTION test_set_insert_remove()
RETURNS SETOF text
AS
$$
BEGIN
	INSERT INTO pgtap.test_host_service_2 VALUES ('h1', NULL, 0);
	INSERT INTO pgtap.test_host_service_2 VALUES ('h1', array['s1'], 1);
	INSERT INTO pgtap.test_host_service_2 VALUES ('h1', cast(array[] as text[]), 2);
	INSERT INTO pgtap.test_host_service_2 VALUES ('h1', array['s1', 's2', 's3'], 3);

	PERFORM startChangeset();
	PERFORM host_add('h1');

	PREPARE expservices  AS SELECT service FROM pgtap.test_host_service_2 WHERE version = 0 AND host = 'h1';
	PREPARE retservices AS SELECT service FROM host_get_data('h1');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'service default is null' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM service_add('s1');
	PERFORM service_add('s2');
	PERFORM host_set_service_insert('h1', 's1');

	PREPARE expservices  AS SELECT service FROM pgtap.test_host_service_2 WHERE version = 1 AND host = 'h1';
	PREPARE retservices AS SELECT service FROM host_get_data('h1');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'inserted services are ok in opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM commitChangeset('2');

	PREPARE expservices  AS SELECT service FROM pgtap.test_host_service_2 WHERE version = 1 AND host = 'h1';
	PREPARE retservices AS SELECT service FROM host_get_data('h1');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'inserted services are ok after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM startChangeset();
	PERFORM host_set_service_remove('h1', 's1');

	PREPARE expservices  AS SELECT service FROM pgtap.test_host_service_2 WHERE version = 2 AND host = 'h1';
	PREPARE retservices AS SELECT service FROM host_get_data('h1');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'removed services are ok in opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM commitChangeset('3');

	PREPARE expservices  AS SELECT service FROM pgtap.test_host_service_2 WHERE version = 2 AND host = 'h1';
	PREPARE retservices AS SELECT service FROM host_get_data('h1');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'removed services are ok after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM startChangeset();
	PERFORM service_add('s3');
	PERFORM service_add('s4');
	PERFORM host_set_service_insert('h1', 's1');
	PERFORM host_set_service_insert('h1', 's4');
	PERFORM host_set_service_insert('h1', 's2');
	PERFORM host_set_service_insert('h1', 's3');
	PERFORM host_set_service_remove('h1', 's4');
	PERFORM host_set_service_remove('h1', 's3');
	PERFORM host_set_service_insert('h1', 's3');

	PREPARE expservices  AS SELECT service FROM pgtap.test_host_service_2 WHERE version = 3 AND host = 'h1';
	PREPARE retservices AS SELECT service FROM host_get_data('h1');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'inserted services are ok in opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	
	PERFORM commitChangeset('4');

	PREPARE expservices  AS SELECT service FROM pgtap.test_host_service_2 WHERE version = 3 AND host = 'h1';
	PREPARE retservices AS SELECT service FROM host_get_data('h1');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'inserted services are ok in opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

END
$$
LANGUAGE plpgsql;


SELECT * FROM runtests();

ROLLBACK;
