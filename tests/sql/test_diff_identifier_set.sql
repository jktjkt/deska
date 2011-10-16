 
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
DECLARE
	version1 bigint;
	version2 bigint;
	version3 bigint;
	version4 bigint;
BEGIN
	INSERT INTO pgtap.test_host_service VALUES ('hpv1', NULL, 1);
	INSERT INTO pgtap.test_host_service VALUES ('hpv1', CAST(array[] AS text[]), 2);
	INSERT INTO pgtap.test_host_service VALUES ('hpv1', array['www','dhcp'], 31);
	INSERT INTO pgtap.test_host_service VALUES ('hpv1', array['dhcp'], 3);
	INSERT INTO pgtap.test_host_service VALUES ('hpv1', array['dhcp'], 4);
	INSERT INTO pgtap.test_host_service VALUES ('hpv2', array['www','dhcp'], 4);
	INSERT INTO pgtap.test_host_service VALUES ('hpv3', array['www'], 4);

	PERFORM startChangeset();
	PERFORM service_add('www');
	PERFORM service_add('ftp');
	PERFORM service_add('dhcp');
	PERFORM host_add('hpv1');

	PERFORM host_init_diff();
	PREPARE retservices AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes();
	RETURN NEXT is_empty( 'retservices', 'a new host was inserted, in inner table is no modification - current changeset' );
	DEALLOCATE retservices;
	PERFORM host_terminate_diff();

	version1 = revision2num(commitChangeset('1'));

	PERFORM startChangeset();
	PERFORM host_set_service('hpv1', cast(array[] as text[]));

	PERFORM host_init_diff();
	PREPARE expservices  AS SELECT v1.service AS olddata, v2.service AS newdata FROM (SELECT * FROM pgtap.test_host_service WHERE version = 1) v1 FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 2) v2 ON (v1.host = v2.host);
	PREPARE retservices AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes();
	RETURN NEXT results_eq( 'retservices', 'expservices', 'set hosts services from NULL to empty set' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_terminate_diff();

	version2 = revision2num(commitChangeset('2'));


	PERFORM host_init_diff(version1,version2);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version1,version2);
	PREPARE expservices  AS SELECT v1.service AS olddata, v2.service AS newdata FROM (SELECT * FROM pgtap.test_host_service WHERE version = 1) v1 FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 2) v2 ON (v1.host = v2.host);
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'set hosts services from NULL to empty set - after commit' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM startChangeset();
	PERFORM host_set_service_insert('hpv1', 'www');
	PERFORM host_set_service_insert('hpv1', 'dhcp');

	PERFORM host_init_diff();
	PREPARE expservices  AS SELECT v1.service AS olddata, v2.service AS newdata FROM (SELECT * FROM pgtap.test_host_service WHERE version = 2) v1 FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 31) v2 ON (v1.host = v2.host);
	PREPARE retservices AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes();
	RETURN NEXT results_eq( 'retservices', 'expservices', 'set hosts inserted services' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_terminate_diff();

	PERFORM host_set_service_remove('hpv1', 'www');

	PERFORM host_init_diff();
	PREPARE expservices  AS SELECT v1.service AS olddata, v2.service AS newdata FROM (SELECT * FROM pgtap.test_host_service WHERE version = 2) v1 FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 3) v2 ON (v1.host = v2.host);
	PREPARE retservices AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes();
	RETURN NEXT results_eq( 'retservices', 'expservices', 'set hosts removed service - in current changeset' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_terminate_diff();

	version3 = revision2num(commitChangeset('3'));


	PERFORM host_init_diff(version1,version3);

	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version1,version3);
	PREPARE expservices  AS SELECT v1.service AS olddata, v2.service AS newdata FROM (SELECT * FROM pgtap.test_host_service WHERE version = 1) v1 FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 3) v2 ON (v1.host = v2.host);
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'set hosts services insert, remove - after commit' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;

	PERFORM host_terminate_diff();

	PERFORM startChangeset();
	PERFORM host_add('hpv2');
	PERFORM host_add('hpv3');
	PERFORM host_set_service_insert('hpv2', 'www');
	PERFORM host_set_service_insert('hpv2', 'dhcp');
	PERFORM host_set_service_insert('hpv3', 'www');

	PERFORM host_init_diff();
	PREPARE expservices AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 3) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 4) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes() ORDER BY objname;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'more changes in more hosts - current changeset' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_terminate_diff();

	version4 = revision2num(commitChangeset('4'));


	PERFORM host_init_diff(version3,version4);
	PREPARE expservices AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 3) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 4) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL));
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version3,version4);
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT set_eq( 'retservices', 'expservices', 'more changes in more hosts - after commit' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM host_init_diff(version2,version4);
	PREPARE expservices AS 
		SELECT COALESCE(v1.host,v2.host), v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 2) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 4) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL));
	CREATE TEMPORARY TABLE versioned_data AS SELECT objname, olddata, newdata FROM host_diff_refs_set_set_attributes(version2,version4);
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT set_eq( 'retservices', 'expservices', 'changes between two versions which differs by more than one revision' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM host_init_diff(version1,version4);
	PREPARE expservices AS 
		SELECT COALESCE(v1.host,v2.host), v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 1) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 4) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL));
	CREATE TEMPORARY TABLE versioned_data AS SELECT objname, olddata, newdata FROM host_diff_refs_set_set_attributes(version2,version4);
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT set_eq( 'retservices', 'expservices', 'changes between two versions which differs by more than one revision' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();


END
$$
LANGUAGE plpgsql;


SELECT * FROM runtests();

ROLLBACK;
