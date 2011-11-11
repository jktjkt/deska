\i util-include-path.sql

BEGIN;

CREATE TABLE pgtap.test_host_service(
	host text,
	service text[],
	template text,
	version bigint
);

CREATE TABLE pgtap.test_host_service_template(
	host text,
	service text[],
	template text,
	version bigint
);

CREATE FUNCTION test_resolved_diff()
RETURNS SETOF text
AS
$$
DECLARE
	base_version bigint;
	version1 bigint;
	version2 bigint;
	version3 bigint;
	version4 bigint;
	version5 bigint;
	version6 bigint;
BEGIN
	INSERT INTO test_host_service VALUES ('hpv1', array['www', 'dhcp'], 'host_templ', 1);
	INSERT INTO test_host_service VALUES ('hpv2', array['ftp'], 'host_templ', 1);
	INSERT INTO test_host_service_template VALUES ('host_templ', array['ftp'], NULL, 1);

	INSERT INTO test_host_service VALUES ('hpv1', array['www', 'dhcp'], 'host_templ', 2);
	INSERT INTO test_host_service VALUES ('hpv2', CAST(array[] AS text[]), 'host_templ', 2);
	INSERT INTO test_host_service_template VALUES ('host_templ2', CAST(array[] AS text[]), 'host_templ', 2);
	INSERT INTO test_host_service_template VALUES ('host_templ', CAST(array[] AS text[]), NULL, 2);

	INSERT INTO test_host_service VALUES ('hpv1', array['www', 'dhcp'], 'host_templ', 3);
	INSERT INTO test_host_service VALUES ('hpv2', array['ftp'], 'host_templ', 3);
	INSERT INTO test_host_service VALUES ('hpv3', array['ftp'], 'host_templ2', 3);
	INSERT INTO test_host_service_template VALUES ('host_templ2', array['ftp'], 'host_templ', 3);
	INSERT INTO test_host_service_template VALUES ('host_templ', array['ftp'], NULL, 3);

	INSERT INTO test_host_service VALUES ('hpv1', array['ftp'], 'host_templ', 4);
	INSERT INTO test_host_service VALUES ('hpv2', array['ftp'], 'host_templ', 4);
	INSERT INTO test_host_service VALUES ('hpv3', array['ftp'], 'host_templ2', 4);
	INSERT INTO test_host_service_template VALUES ('host_templ2', array['ftp'], 'host_templ', 4);
	INSERT INTO test_host_service_template VALUES ('host_templ', array['ftp'], NULL, 4);

	INSERT INTO test_host_service VALUES ('hpv1', array['ftp'], 'host_templ', 5);
	INSERT INTO test_host_service VALUES ('hpv2', array['ftp'], 'host_templ', 5);
	INSERT INTO test_host_service VALUES ('hpv3', array['dhcp'], 'host_templ2', 5);
	INSERT INTO test_host_service_template VALUES ('host_templ2', array['dhcp'], 'host_templ', 5);
	INSERT INTO test_host_service_template VALUES ('host_templ', array['ftp'], NULL, 5);

	INSERT INTO test_host_service VALUES ('hpv1', NULL, 'host_templ', 6);
	INSERT INTO test_host_service VALUES ('hpv2', NULL, 'host_templ', 6);
	INSERT INTO test_host_service VALUES ('hpv3', array['dhcp', 'ftp'], 'host_templ2', 6);
	INSERT INTO test_host_service_template VALUES ('host_templ2', array['dhcp'], 'host_templ', 6);
	INSERT INTO test_host_service_template VALUES ('host_templ', NULL, NULL, 6);



	PERFORM startchangeset();
	PERFORM service_add('www');
	PERFORM service_add('dhcp');
	PERFORM service_add('ftp');
	PERFORM host_add('hpv1');
	PERFORM host_set_service_insert('hpv1', 'www');
	PERFORM host_set_service_insert('hpv1', 'dhcp');
	PERFORM host_template_add('host_templ');
	PERFORM host_add('hpv2');
	PERFORM host_set_template_host('hpv2','host_templ');
	PERFORM host_set_template_host('hpv1','host_templ');
	PERFORM host_template_set_service('host_templ',array['ftp']);

	PERFORM host_init_ch_resolved_diff();
	PREPARE expservices  AS SELECT v1.service AS olddata, v2.service AS newdata FROM (SELECT * FROM pgtap.test_host_service WHERE version = 0) v1 FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 1) v2 ON (v1.host = v2.host);
	PREPARE retservices AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes();
	RETURN NEXT results_eq( 'retservices', 'expservices', 'add services to host, add service to template and assign the template to hosts - opened changeset' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_terminate_diff();

	PERFORM host_template_init_ch_resolved_diff();
	PREPARE expservices  AS SELECT v1.service AS olddata, v2.service AS newdata FROM (SELECT * FROM pgtap.test_host_service_template WHERE version = 0) v1 FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service_template WHERE version = 1) v2 ON (v1.host = v2.host);
	PREPARE retservices AS SELECT olddata, newdata FROM host_template_diff_refs_set_set_attributes();
	RETURN NEXT results_eq( 'retservices', 'expservices', 'add service to template - opened changeset' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_template_terminate_diff();

	base_version = id2num(parent(get_current_changeset()));
	version1 = revision2num(commitchangeset('1'));


	PERFORM host_init_resolved_diff(base_version, version1);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(base_version,version1);
	PREPARE expservices  AS SELECT v1.service AS olddata, v2.service AS newdata FROM (SELECT * FROM pgtap.test_host_service WHERE version = 0) v1 FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 1) v2 ON (v1.host = v2.host);
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'add services to host, add service to template and assign the template to hosts - after commit' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM startchangeset();
	PERFORM host_template_add('host_templ2');
	PERFORM host_template_set_template_host('host_templ2','host_templ');
	PERFORM host_template_set_service_remove('host_templ','ftp');

	PERFORM host_init_ch_resolved_diff();
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 1) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 2) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes() ORDER BY objname;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'add template and assign another one to it, change parametrs of parent template - opened changeset' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_terminate_diff();

	version2 = revision2num(commitchangeset('2'));

	PERFORM host_init_resolved_diff(version1, version2);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version1,version2);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 1) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 2) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'add template and assign another one to it, change parametrs of parent template - after commit' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM host_init_resolved_diff(base_version, version2);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(base_version,version2);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 0) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 2) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between base_version and version2' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM startchangeset();
	PERFORM host_add('hpv3');
	PERFORM host_template_set_service_insert('host_templ','ftp');
	PERFORM host_set_template_host('hpv3','host_templ2');

	PERFORM host_init_ch_resolved_diff();
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 2) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 3) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v2.host;
	PREPARE retservices AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes() ORDER BY objname;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'add host and template it by 2 levels of templates - opened changeset' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_terminate_diff();

	PERFORM host_template_init_ch_resolved_diff();
	PREPARE expservices  AS SELECT v1.service AS olddata, v2.service AS newdata FROM (SELECT * FROM pgtap.test_host_service_template WHERE version = 2) v1 FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service_template WHERE version = 3) v2 ON (v1.host = v2.host) ORDER BY v2.host;
	PREPARE retservices AS SELECT olddata, newdata FROM host_template_diff_refs_set_set_attributes() ORDER BY objname;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff of templates - opened changeset' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_template_terminate_diff();

	version3 = revision2num(commitchangeset('3'));

	PERFORM host_init_resolved_diff(base_version, version3);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(base_version,version3);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 0) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 3) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'add host and template it by 2 levels of templates - after commit' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM host_init_resolved_diff(version1, version3);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version1,version3);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 1) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 3) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version1 and version2' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM host_init_resolved_diff(version2, version3);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version2,version3);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 2) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 3) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version2 and version3' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM host_template_init_resolved_diff(base_version, version3);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_template_diff_refs_set_set_attributes(base_version,version3);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service_template WHERE version = 0) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service_template WHERE version = 3) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'templates - add host and template it by 2 levels of templates - after commit' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_template_terminate_diff();

	PERFORM host_template_init_resolved_diff(version1, version3);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_template_diff_refs_set_set_attributes(version1,version3);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service_template WHERE version = 1) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service_template WHERE version = 3) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'templates - diff between version1 and version3' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_template_terminate_diff();

	PERFORM host_template_init_resolved_diff(version2, version3);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_template_diff_refs_set_set_attributes(version2,version3);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service_template WHERE version = 2) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service_template WHERE version = 3) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'template - diff between version2 and version3' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_template_terminate_diff();
	

	PERFORM startchangeset();
	PERFORM host_set_service('hpv1', NULL);

	PERFORM host_init_ch_resolved_diff();
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 3) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 4) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes() ORDER BY objname;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'host service set to NULL ->inherit it from template - opened changeset' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_terminate_diff();

	version4 = revision2num(commitchangeset('4'));

	PERFORM host_init_resolved_diff(base_version, version4);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(base_version,version4);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 0) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 4) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'add host and template it by 2 levels of templates - after commit' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM host_init_resolved_diff(version1, version4);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version1,version4);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 1) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 4) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version1 and version4' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM host_init_resolved_diff(version2, version4);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version2,version4);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 2) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 4) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version2 and version4' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM host_init_resolved_diff(version3, version4);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version3,version4);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 3) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 4) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version3 and version4' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM startchangeset();
	PERFORM host_template_set_service('host_templ2', array['dhcp']);

	PERFORM host_init_ch_resolved_diff();
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 4) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 5) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes() ORDER BY objname;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'set service of templated service -> does not inherit it from template and rewrites data from parent template - opened changeset' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_terminate_diff();

	PERFORM host_template_init_ch_resolved_diff();
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service_template WHERE version = 4) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service_template WHERE version = 5) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT olddata, newdata FROM host_template_diff_refs_set_set_attributes() ORDER BY objname;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'set service of templated service -> does not inherit it from template and rewrites data from parent template - opened changeset' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_template_terminate_diff();

	version5 = revision2num(commitchangeset('5'));

	PERFORM host_init_resolved_diff(base_version, version5);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(base_version,version5);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 0) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 5) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'add host and template it by 2 levels of templates - after commit' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM host_init_resolved_diff(version1, version5);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version1,version5);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 1) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 5) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version1 and version5' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM host_init_resolved_diff(version2, version5);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version2,version5);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 2) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 5) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version2 and version5' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM host_init_resolved_diff(version3, version5);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version3,version5);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 3) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 5) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version3 and version5' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

	PERFORM host_init_resolved_diff(version4, version5);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version4,version5);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 4) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 5) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version4 and version5' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();


	PERFORM host_template_init_resolved_diff(base_version, version5);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_template_diff_refs_set_set_attributes(base_version,version5);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service_template WHERE version = 0) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service_template WHERE version = 5) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'add host and template it by 2 levels of templates - after commit' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_template_terminate_diff();

	PERFORM host_template_init_resolved_diff(version1, version5);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_template_diff_refs_set_set_attributes(version1,version5);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service_template WHERE version = 1) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service_template WHERE version = 5) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version1 and version5' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_template_terminate_diff();

	PERFORM host_template_init_resolved_diff(version2, version5);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_template_diff_refs_set_set_attributes(version2,version5);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service_template WHERE version = 2) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service_template WHERE version = 5) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version2 and version5' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_template_terminate_diff();

	PERFORM host_template_init_resolved_diff(version3, version5);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_template_diff_refs_set_set_attributes(version3,version5);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service_template WHERE version = 3) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service_template WHERE version = 5) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version3 and version5' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_template_terminate_diff();

	PERFORM host_template_init_resolved_diff(version4, version5);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_template_diff_refs_set_set_attributes(version4,version5);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service_template WHERE version = 4) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service_template WHERE version = 5) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version4 and version5' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_template_terminate_diff();

	PERFORM startchangeset();
	PERFORM host_set_service_insert('hpv3','ftp');
	PERFORM host_template_set_service('host_templ',NULL);

	PERFORM host_init_ch_resolved_diff();
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 5) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 6) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes() ORDER BY objname;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'template set service to null - opened changeset' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_terminate_diff();

	PERFORM host_template_init_ch_resolved_diff();
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service_template WHERE version = 5) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service_template WHERE version = 6) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT olddata, newdata FROM host_template_diff_refs_set_set_attributes() ORDER BY objname;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'template set service to null - opened changeset' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	PERFORM host_template_terminate_diff();

	version6 = revision2num(commitchangeset('6'));

	PERFORM host_init_resolved_diff(version5, version6);
	CREATE TEMPORARY TABLE versioned_data AS SELECT olddata, newdata FROM host_diff_refs_set_set_attributes(version5,version6);
	PREPARE expservices  AS 
		SELECT v1.service AS olddata, v2.service AS newdata 
		FROM (SELECT * FROM pgtap.test_host_service WHERE version = 5) v1 
			FULL OUTER JOIN (SELECT * FROM pgtap.test_host_service WHERE version = 6) v2 ON (v1.host = v2.host) 
		WHERE v1.service<>v2.service OR ((v1.service IS NULL AND v2.service IS NOT NULL) OR (v2.service IS NULL AND v1.service IS NOT NULL))
		ORDER BY v1.host;
	PREPARE retservices AS SELECT * FROM versioned_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'diff between version5 and version6' );
	DEALLOCATE retservices;
	DEALLOCATE expservices;
	DROP TABLE versioned_data;
	PERFORM host_terminate_diff();

END
$$
LANGUAGE plpgsql;


SELECT * FROM runtests();

ROLLBACK;
