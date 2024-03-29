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

CREATE FUNCTION test_mod_templated_identifier_set()
RETURNS SETOF text
AS
$$
DECLARE
	old_version bigint;
	old_version2 bigint;
BEGIN
	INSERT INTO test_host_service VALUES ('hpv1', CAST(array[] AS text[]), 'host_templ', 1);
	INSERT INTO test_host_service VALUES ('hpv2', array['www','dhcp'], 'host_templ', 1);
	INSERT INTO test_host_service VALUES ('hpv3', array['www'], 'host_templ', 1);
	INSERT INTO test_host_service VALUES ('hpv4', array['www'], 'host_templ', 1);
	INSERT INTO test_host_service VALUES ('hpv1', CAST(array[] AS text[]), 'host_templ', 2);
	INSERT INTO test_host_service VALUES ('hpv2', array['www','dhcp'], 'host_templ', 2);
	INSERT INTO test_host_service VALUES ('hpv3', array['www', 'dhcp'], 'host_templ', 2);
	INSERT INTO test_host_service VALUES ('hpv4', CAST(array[] AS text[]), 'host_templ', 2);
	INSERT INTO test_host_service VALUES ('hpv1', CAST(array[] AS text[]), 'host_templ', 3);
	INSERT INTO test_host_service VALUES ('hpv2', array['www','dhcp'], 'host_templ', 3);
	INSERT INTO test_host_service VALUES ('hpv3', array['www'], 'host_templ', 3);
	INSERT INTO test_host_service VALUES ('hpv4', CAST(array[] AS text[]), 'host_templ', 3);

	INSERT INTO test_host_service_template VALUES ('host_templ', array['www'], NULL, 1);

	PERFORM startchangeset();
	PERFORM service_add('www');
	PERFORM service_add('dhcp');
	PERFORM host_template_add('host_templ');
	PERFORM host_template_set_service('host_templ',ARRAY['www']);
	PERFORM host_add('hpv1');
	PERFORM host_add('hpv2');
	PERFORM host_set_template_host('hpv1','host_templ');
	PERFORM host_set_template_host('hpv2','host_templ');
	PERFORM host_set_service_remove('hpv1', 'www');
	PERFORM host_set_service_insert('hpv2', 'dhcp');
	PERFORM host_add('hpv3');
	PERFORM host_add('hpv4');
	PERFORM host_set_template_host('hpv3','host_templ');
	PERFORM host_set_template_host('hpv4','host_templ');

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 1 AND host = 'hpv1';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('hpv1');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after remove service from object that is templated - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 1 AND host = 'hpv2';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('hpv2');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after remove service from object that is templated - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service_template WHERE version = 1 AND host = 'host_templ';
	PREPARE retservices AS SELECT service, template_host FROM host_template_resolved_object_data('host_templ');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'template services are ok - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 1 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data() ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'all templated services of all hosts are ok in opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service_template WHERE version = 1 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_template_resolved_data() ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'all templated services of all hosts templates are ok in opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	old_version = revision2num(commitchangeset('1'));

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 1 AND host = 'hpv1';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('hpv1');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after remove service from object that is templated - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 1 AND host = 'hpv2';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('hpv2');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after remove service from object that is templated - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service_template WHERE version = 1 AND host = 'host_templ';
	PREPARE retservices AS SELECT service, template_host FROM host_template_resolved_object_data('host_templ');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'template services are ok - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 1 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data() ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'all templated services of all hosts are ok in opened changeset - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service_template WHERE version = 1 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_template_resolved_data() ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'all templated services of all hosts templates are ok in opened changeset - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM startchangeset();
	PERFORM host_set_service_insert('hpv3','dhcp');
	PERFORM host_set_service_remove('hpv4','www');

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 2 AND host = 'hpv3';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('hpv3');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after remove service from object that is templated in another changeset - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 2 AND host = 'hpv4';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('hpv4');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after remove service from object that is templated in another changeset - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 2 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data() ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'all templated services of all hosts are ok in another changeset - in opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	old_version2 = revision2num(commitchangeset('2'));

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 2 AND host = 'hpv3';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('hpv3');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after remove service from object that is templated in another changeset - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 2 AND host = 'hpv4';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('hpv4');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after remove service from object that is templated in another changeset - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 2 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data() ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'all templated services of all hosts are ok in another changeset - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM startchangeset();
	PERFORM host_set_service('hpv3',NULL);

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 3 AND host = 'hpv3';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('hpv3');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'service was set to null, object again inherits services from template - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 3 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data() ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'resolved templated services are ok - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM commitchangeset('3');

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 3 AND host = 'hpv3';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('hpv3');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'service was set to null, object again inherits services from template - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 3 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data() ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'resolved templated services are ok - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 2 AND host = 'hpv3';
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, template_host FROM host_resolved_object_data('hpv3',old_version2);
	PREPARE retservices AS SELECT * FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'select from older version' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	DROP TABLE old_version_resolved_data;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 2 AND host = 'hpv4';
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, template_host FROM host_resolved_object_data('hpv4',old_version2);
	PREPARE retservices AS SELECT * FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'select from older version' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	DROP TABLE old_version_resolved_data;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 2 ORDER BY host;
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data(old_version2);
	PREPARE retservices AS SELECT * FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'select from older version' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	DROP TABLE old_version_resolved_data;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 1 AND host = 'hpv1';
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, template_host FROM host_resolved_object_data('hpv1',old_version);
	PREPARE retservices AS SELECT * FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'select from older version' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	DROP TABLE old_version_resolved_data;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 1 AND host = 'hpv2';
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, template_host FROM host_resolved_object_data('hpv2',old_version2);
	PREPARE retservices AS SELECT * FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'select from older version' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	DROP TABLE old_version_resolved_data;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service_template WHERE version = 1 AND host = 'host_templ';
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, template_host FROM host_template_resolved_object_data('host_templ',old_version);
	PREPARE retservices AS SELECT * FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'select from older version' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	DROP TABLE old_version_resolved_data;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service WHERE version = 1 ORDER BY host;
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data(old_version) ORDER BY name;
	PREPARE retservices AS SELECT * FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'select from older version' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	DROP TABLE old_version_resolved_data;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test_host_service_template WHERE version = 1 ORDER BY host;
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, host_template_get_name(template_host) FROM host_template_resolved_data(old_version) ORDER BY name;
	PREPARE retservices AS SELECT * FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'select from older version' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	DROP TABLE old_version_resolved_data;

END
$$
LANGUAGE plpgsql;

CREATE TABLE pgtap.test2_host_service(
	host text,
	service text[],
	template text,
	version bigint
);

CREATE TABLE pgtap.test2_host_service_template(
	host text,
	service text[],
	template text,
	version bigint
);


CREATE FUNCTION test_mod_templated_identifier_set_templates()
RETURNS SETOF text
AS
$$
DECLARE
	old_version bigint;
	old_version2 bigint;
BEGIN
	INSERT INTO test2_host_service VALUES ('host2', CAST(ARRAY[] AS text[]), 'host_templ22', 1);
	INSERT INTO test2_host_service VALUES ('host3', ARRAY['service1','service2'], 'host_templ23', 1);
	INSERT INTO test2_host_service VALUES ('host4', ARRAY['service1','service3'], 'host_templ24', 1);
	INSERT INTO test2_host_service VALUES ('host5', CAST(ARRAY[] AS text[]), 'host_templ25', 1);

	INSERT INTO test2_host_service VALUES ('host2', ARRAY['service1'], 'host_templ22', 2);
	INSERT INTO test2_host_service VALUES ('host3', ARRAY['service1','service2'], 'host_templ23', 2);
	INSERT INTO test2_host_service VALUES ('host4', ARRAY['service1','service3'], 'host_templ24', 2);
	INSERT INTO test2_host_service VALUES ('host5', ARRAY['service3'], 'host_templ25', 2);
	INSERT INTO test2_host_service VALUES ('host6', CAST(ARRAY[] AS text[]), 'host_templ26', 2);

	INSERT INTO test2_host_service VALUES ('host2', ARRAY['service1'], 'host_templ22', 3);
	INSERT INTO test2_host_service VALUES ('host3', ARRAY['service1','service2'], 'host_templ23', 3);
	INSERT INTO test2_host_service VALUES ('host4', ARRAY['service1'], 'host_templ24', 3);
	INSERT INTO test2_host_service VALUES ('host5', ARRAY['service3'], 'host_templ25', 3);
	INSERT INTO test2_host_service VALUES ('host6', ARRAY['service1'], 'host_templ26', 3);

	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ21', ARRAY['service1'], NULL, 1);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ22', CAST(ARRAY[] AS text[]), 'host_templ21', 1);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ23', ARRAY['service1','service2'], 'host_templ21', 1);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ24', ARRAY['service1'], 'host_templ21', 1);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ25', CAST(ARRAY[] AS text[]), 'host_templ22', 1);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ26', ARRAY['service1'], 'host_templ21', 1);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ27', ARRAY['service1'], 'host_templ21', 1);

	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ21', ARRAY['service1'], NULL, 2);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ22', CAST(ARRAY[] AS text[]), 'host_templ21', 2);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ23', ARRAY['service1','service2'], 'host_templ21', 2);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ24', ARRAY['service1'], 'host_templ21', 2);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ25', ARRAY['service3'], 'host_templ22', 2);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ26', CAST(ARRAY[] AS text[]), 'host_templ21', 2);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ27', ARRAY['service1', 'service2'], 'host_templ21', 2);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ28', ARRAY['service3'], 'host_templ25', 2);

	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ21', ARRAY['service1'], NULL, 3);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ22', CAST(ARRAY[] AS text[]), 'host_templ21', 3);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ23', ARRAY['service1','service2'], 'host_templ21', 3);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ24', ARRAY['service1'], 'host_templ21', 3);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ25', ARRAY['service3'], 'host_templ22', 3);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ26', ARRAY['service1'], 'host_templ21', 3);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ27', ARRAY['service1', 'service2'], 'host_templ21', 3);
	INSERT INTO pgtap.test2_host_service_template VALUES ('host_templ28', ARRAY['service3'], 'host_templ25', 3);


	PERFORM startchangeset();
	PERFORM service_add('service1');
	PERFORM service_add('service2');
	PERFORM service_add('service3');
	PERFORM host_template_add('host_templ21');
	PERFORM host_template_add('host_templ22');
	PERFORM host_template_add('host_templ23');
	PERFORM host_template_add('host_templ24');
	PERFORM host_template_set_service('host_templ21',ARRAY['service1']);
	PERFORM host_template_set_template_host('host_templ22','host_templ21');
	PERFORM host_template_set_template_host('host_templ23','host_templ21');
	PERFORM host_template_set_template_host('host_templ24','host_templ21');
	PERFORM host_add('host2');
	PERFORM host_add('host3');
	PERFORM host_add('host4');
	PERFORM host_set_template_host('host2','host_templ22');
	PERFORM host_set_template_host('host3','host_templ23');
	PERFORM host_set_template_host('host4','host_templ24');
	PERFORM host_template_set_service_remove('host_templ22', 'service1');
	PERFORM host_template_set_service_insert('host_templ23', 'service2');
	PERFORM host_set_service_insert('host4', 'service3');
	PERFORM host_template_add('host_templ25');
	PERFORM host_template_add('host_templ26');
	PERFORM host_template_add('host_templ27');
	PERFORM host_template_set_template_host('host_templ25','host_templ22');
	PERFORM host_template_set_template_host('host_templ26','host_templ21');
	PERFORM host_template_set_template_host('host_templ27','host_templ21');
	PERFORM host_add('host5');
	PERFORM host_set_template_host('host5','host_templ25');

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 1 AND host = 'host4';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host4');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after 2 levels inheritance - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 1 AND host = 'host5';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host5');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after 2 levels inheritance - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 1 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data() WHERE name IN ('host2','host3','host4','host5') ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services in all hosts are ok after 2 levels inheritance - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service_template WHERE version = 1 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_template_resolved_data() WHERE name IN ('host_templ21','host_templ22','host_templ23','host_templ24','host_templ25','host_templ26','host_templ27') ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services in all templates are ok after 2 levels inheritance - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	old_version = revision2num(commitchangeset('1'));

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 1 AND host = 'host4';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host4');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after 2 levels inheritance - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 1 AND host = 'host5';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host5');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after 2 levels inheritance - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 1 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data() WHERE name IN ('host2','host3','host4','host5') ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services in all hosts are ok after 2 levels inheritance - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service_template WHERE version = 1 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_template_resolved_data() WHERE name IN ('host_templ21','host_templ22','host_templ23','host_templ24','host_templ25','host_templ26','host_templ27') ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services in all templates are ok after 2 levels inheritance - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM startchangeset();
	PERFORM host_template_set_service_remove('host_templ26', 'service1');
	PERFORM host_template_set_service_insert('host_templ27', 'service2');
	PERFORM host_template_add('host_templ28');
	PERFORM host_template_set_template_host('host_templ28','host_templ25');
	PERFORM host_template_set_service_insert('host_templ25', 'service3');
	PERFORM host_add('host6');
	PERFORM host_set_template_host('host6', 'host_templ26');
	PERFORM host_set_service_insert('host2', 'service1');

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 2 AND host = 'host2';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host2');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'modified services are ok after 2 levels inheritance - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 2 AND host = 'host5';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host5');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after 2 levels inheritance, modified template - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 2 AND host = 'host6';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host6');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after 3 levels inheritance - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 2 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data() WHERE name IN ('host2','host3','host4','host5','host6') ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services in all hosts are ok after 2,3 levels inheritance - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service_template WHERE version = 2 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_template_resolved_data() WHERE name IN ('host_templ21','host_templ22','host_templ23','host_templ24','host_templ25','host_templ26','host_templ27','host_templ28') ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services in all templates are ok after 2,3 levels inheritance - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	old_version2 = revision2num(commitchangeset('2'));

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 2 AND host = 'host2';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host2');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'modified services are ok after 2 levels inheritance - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 2 AND host = 'host5';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host5');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after 2 levels inheritance, modified template - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 2 AND host = 'host6';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host6');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after 3 levels inheritance - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 2 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data() WHERE name IN ('host2','host3','host4','host5','host6') ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services in all hosts are ok after 2,3 levels inheritance - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service_template WHERE version = 2 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_template_resolved_data() WHERE name IN ('host_templ21','host_templ22','host_templ23','host_templ24','host_templ25','host_templ26','host_templ27','host_templ28') ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services in all templates are ok after 2,3 levels inheritance - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 1 ORDER BY host;
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data(old_version) WHERE name IN ('host2','host3','host4','host5') ORDER BY name;
	PREPARE retservices AS SELECT * FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'host multiple data from older version' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	DROP TABLE old_version_resolved_data;


	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service_template WHERE version = 1 ORDER BY host;
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, host_template_get_name(template_host) FROM host_template_resolved_data(old_version) WHERE name IN ('host_templ21','host_templ22','host_templ23','host_templ24','host_templ25','host_templ26','host_templ27','host_templ28') ORDER BY name;
	PREPARE retservices AS SELECT * FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'host_template multiple data from older version' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	DROP TABLE old_version_resolved_data;

	PERFORM startchangeset();
	PERFORM host_set_service('host4',NULL);
	PERFORM host_template_set_service('host_templ26',NULL);


	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 3 AND host = 'host4';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host4');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'modified services are ok after 2 levels inheritance - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 3 AND host = 'host6';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host6');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after 3 levels inheritance - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 3 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data() WHERE name IN ('host2','host3','host4','host5','host6') ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services in all hosts are ok after 2,3 levels inheritance - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service_template WHERE version = 3 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_template_resolved_data() WHERE name IN ('host_templ21','host_templ22','host_templ23','host_templ24','host_templ25','host_templ26','host_templ27','host_templ28') ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services in all templates are ok after 2,3 levels inheritance - opened changeset' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PERFORM commitchangeset('3');

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 3 AND host = 'host4';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host4');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'modified services are ok after 2 levels inheritance - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 3 AND host = 'host6';
	PREPARE retservices AS SELECT service, template_host FROM host_resolved_object_data('host6');
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services are ok after 2 levels inheritance, modified template - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 3 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data() WHERE name IN ('host2','host3','host4','host5','host6') ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services in all hosts are ok after 2,3 levels inheritance - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service_template WHERE version = 3 ORDER BY host;
	PREPARE retservices AS SELECT service, host_template_get_name(template_host) FROM host_template_resolved_data() WHERE name IN ('host_templ21','host_templ22','host_templ23','host_templ24','host_templ25','host_templ26','host_templ27','host_templ28') ORDER BY name;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'services in all templates are ok after 2,3 levels inheritance - after commit' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;

	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service WHERE version = 2 ORDER BY host;
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, host_template_get_name(template_host) FROM host_resolved_data(old_version2) WHERE name IN ('host2','host3','host4','host5','host6') ORDER BY name;
	PREPARE retservices AS SELECT * FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'host multiple data from older version' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	DROP TABLE old_version_resolved_data;


	PREPARE expservices  AS SELECT service, template FROM pgtap.test2_host_service_template WHERE version = 2 ORDER BY host;
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, host_template_get_name(template_host) FROM host_template_resolved_data(old_version2) WHERE name IN ('host_templ21','host_templ22','host_templ23','host_templ24','host_templ25','host_templ26','host_templ27','host_templ28') ORDER BY name;
	PREPARE retservices AS SELECT * FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'retservices', 'expservices', 'host_template multiple data from older version' );
	DEALLOCATE expservices;
	DEALLOCATE retservices;
	DROP TABLE old_version_resolved_data;

END
$$
LANGUAGE plpgsql;

SELECT * FROM runtests();

ROLLBACK;
