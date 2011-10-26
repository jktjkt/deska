 
\i util-include-path.sql

BEGIN;

CREATE TABLE pgtap.test_host_service(
    host text,
    service text[],
    note text,
    service_templ text,
    note_templ text,
    template text,
    version bigint
);

CREATE TABLE pgtap.test_host_service_template(
    host text,
    service text[],
    note text,
    service_templ text,
    note_templ text,
    template text,
    version bigint
);

CREATE FUNCTION test_templated_identifier_set()
RETURNS SETOF text
AS
$$
DECLARE
    old_version bigint;
    old_version2 bigint;
BEGIN
    INSERT INTO test_host_service VALUES ('hpv1', array['www', 'dhcp'], 'note from host_templ', 'hpv1', 'host_templ', 'host_templ', 1);
    INSERT INTO test_host_service VALUES ('hpv2', array['ftp'], 'note from hpv2', 'host_templ', 'hpv2', 'host_templ', 1);
    INSERT INTO test_host_service_template VALUES ('host_templ', array['ftp'], 'note from host_templ', 'host_templ', 'host_templ',NULL, 1);

    INSERT INTO test_host_service VALUES ('hpv1', array['www', 'dhcp'], 'note from host_templ', 'hpv1', 'host_templ', 'host_templ', 2);
    INSERT INTO test_host_service VALUES ('hpv2', CAST(array[] AS text[]), 'note from hpv2', 'host_templ', 'hpv2', 'host_templ', 2);
    INSERT INTO test_host_service_template VALUES ('host_templ2', CAST(array[] AS text[]), 'note from host_templ', 'host_templ', 'host_templ', 'host_templ', 2);
    INSERT INTO test_host_service_template VALUES ('host_templ', CAST(array[] AS text[]), 'note from host_templ', 'host_templ', 'host_templ', NULL, 2);

    INSERT INTO test_host_service VALUES ('hpv1', array['www', 'dhcp'], 'note from host_templ', 'hpv1', 'host_templ', 'host_templ', 3);
    INSERT INTO test_host_service VALUES ('hpv2', array['ftp'], 'note from hpv2', 'host_templ', 'hpv2', 'host_templ', 3);
    INSERT INTO test_host_service VALUES ('hpv3', array['ftp'], 'note from host_templ', 'host_templ', 'host_templ', 'host_templ2', 3);
    INSERT INTO test_host_service_template VALUES ('host_templ2', array['ftp'], 'note from host_templ', 'host_templ', 'host_templ', 'host_templ', 3);
    INSERT INTO test_host_service_template VALUES ('host_templ', array['ftp'], 'note from host_templ', 'host_templ', 'host_templ', NULL, 3);

    INSERT INTO test_host_service VALUES ('hpv1', array['www', 'dhcp'], 'note from host_templ', 'hpv1', 'host_templ', 'host_templ', 4);
    INSERT INTO test_host_service VALUES ('hpv2', array['ftp'], 'note from hpv2', 'host_templ', 'hpv2', 'host_templ', 4);
    INSERT INTO test_host_service VALUES ('hpv3', array['ftp'], 'note from host_templ', 'host_templ', 'host_templ', 'host_templ2', 4);
    INSERT INTO test_host_service VALUES ('hpv4', array['ftp'], 'note from host_templ', 'host_templ', 'host_templ', 'host_templ3', 4);
    INSERT INTO test_host_service_template VALUES ('host_templ2', array['ftp'], 'note from host_templ', 'host_templ', 'host_templ', 'host_templ', 4);
    INSERT INTO test_host_service_template VALUES ('host_templ', array['ftp'], 'note from host_templ', 'host_templ', 'host_templ', NULL, 4);
    INSERT INTO test_host_service_template VALUES ('host_templ3', array['ftp'], 'note from host_templ', 'host_templ', 'host_templ', 'host_templ2', 4);

    INSERT INTO test_host_service VALUES ('hpv1', array['www', 'dhcp'], 'note from host_templ', 'hpv1', 'host_templ', 'host_templ', 5);
    INSERT INTO test_host_service VALUES ('hpv2', array['ftp'], 'note from hpv2', 'host_templ', 'hpv2', 'host_templ', 5);
    INSERT INTO test_host_service VALUES ('hpv3', array['dhcp', 'ftp'], 'note from host_templ', 'host_templ2', 'host_templ', 'host_templ2', 5);
    INSERT INTO test_host_service VALUES ('hpv4', array['dhcp', 'ftp'], 'note from host_templ', 'host_templ2', 'host_templ', 'host_templ3', 5);
    INSERT INTO test_host_service_template VALUES ('host_templ2', array['dhcp', 'ftp'], 'note from host_templ', 'host_templ2', 'host_templ', 'host_templ', 5);
    INSERT INTO test_host_service_template VALUES ('host_templ', array['ftp'], 'note from host_templ', 'host_templ', 'host_templ', NULL, 5);
    INSERT INTO test_host_service_template VALUES ('host_templ3', array['dhcp','ftp'], 'note from host_templ', 'host_templ2', 'host_templ', 'host_templ2', 5);

    INSERT INTO test_host_service VALUES ('hpv1', array['www', 'dhcp'], 'note from host_templ', 'hpv1', 'host_templ', 'host_templ', 6);
    INSERT INTO test_host_service VALUES ('hpv2', array['ftp'], 'note from hpv2', 'host_templ', 'hpv2', 'host_templ', 6);
    INSERT INTO test_host_service VALUES ('hpv3', array['dhcp', 'ftp'], 'note from host_templ', 'host_templ2', 'host_templ', 'host_templ2', 6);
    INSERT INTO test_host_service VALUES ('hpv4', array['dhcp'], 'note from host_templ', 'hpv4', 'host_templ', 'host_templ3', 6);
    INSERT INTO test_host_service_template VALUES ('host_templ2', array['dhcp', 'ftp'], 'note from host_templ', 'host_templ2', 'host_templ', 'host_templ', 6);
    INSERT INTO test_host_service_template VALUES ('host_templ', array['ftp'], 'note from host_templ', 'host_templ', 'host_templ', NULL, 6);
    INSERT INTO test_host_service_template VALUES ('host_templ3', array['dhcp','ftp'], 'note from host_templ', 'host_templ2', 'host_templ', 'host_templ2', 6);


    PERFORM startchangeset();
    PERFORM service_add('www');
    PERFORM service_add('dhcp');
    PERFORM service_add('ftp');
    PERFORM host_add('hpv1');
    PERFORM host_set_service_insert('hpv1', 'www');
    PERFORM host_set_service_insert('hpv1', 'dhcp');
    PERFORM host_template_add('host_templ');
    PERFORM host_template_set_note_host('host_templ', 'note from host_templ');
    PERFORM host_add('hpv2');
    PERFORM host_set_template_host('hpv2','host_templ');
    PERFORM host_set_template_host('hpv1','host_templ');
    PERFORM host_set_note_host('hpv2','note from hpv2');
    PERFORM host_template_set_service('host_templ',array['ftp']);

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 1 AND host = 'hpv1';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_resolved_object_data_template_info('hpv1');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok in opened changeset, host has own services' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 1 AND host = 'hpv2';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_resolved_object_data_template_info('hpv2');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok in opened changeset, host inherits services' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 1 AND host = 'host_templ';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_template_resolved_object_data_template_info('host_templ');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok in opened changeset, template has own services' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 1 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_resolved_data_template_info() ORDER BY name;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services of all hosts are ok in opened changeset' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 1 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_template_resolved_data_template_info() ORDER BY name;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services of all templates are ok in opened changeset' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    old_version = revision2num(commitchangeset('1'));

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 1 AND host = 'hpv1';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_resolved_object_data_template_info('hpv1');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok, host has own services - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 1 AND host = 'hpv2';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_resolved_object_data_template_info('hpv2');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok, host inherits services - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 1 AND host = 'host_templ';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_template_resolved_object_data_template_info('host_templ');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok, template has own services - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 1 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_resolved_data_template_info() ORDER BY name;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services of all hosts are ok - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 1 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_template_resolved_data_template_info() ORDER BY name;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services of all templates are ok - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;


    PERFORM startchangeset();
    PERFORM host_template_add('host_templ2');
    PERFORM host_template_set_template_host('host_templ2','host_templ');
    PERFORM host_template_set_service_remove('host_templ','ftp');

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 2 AND host = 'hpv2';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_resolved_object_data_template_info('hpv2');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok in opened changeset, host inherits services' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 2 AND host = 'host_templ2';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_template_resolved_object_data_template_info('host_templ2');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok in opened changeset, templates has own services' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 2 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_resolved_data_template_info();
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok in opened changeset, host has own services' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 2 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_template_resolved_data_template_info() ORDER BY name;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok in opened changeset, templates has own services - multiple data' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    old_version2 = revision2num(commitchangeset('2'));

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 2 AND host = 'hpv2';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_resolved_object_data_template_info('hpv2');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok after commit, host inherits services' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 1 AND host = 'hpv2';
    CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_resolved_object_data_template_info('hpv2',old_version);
    PREPARE retservices AS SELECT * FROM old_version_resolved_data;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok after commit, host inherits services, from older version' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;
    DROP TABLE old_version_resolved_data;


    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 2 AND host = 'host_templ2';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_template_resolved_object_data_template_info('host_templ2');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok after commit, template inherits services' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 2 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_resolved_data_template_info();
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok after commit - multiple data' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 2 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_template_resolved_data_template_info() ORDER BY name;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok after commit - multiple data' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;


    PERFORM startchangeset();
    PERFORM host_add('hpv3');
    PERFORM host_template_set_service_insert('host_templ','ftp');
    PERFORM host_set_template_host('hpv3','host_templ2');

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 3 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_resolved_data_template_info();
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok in opened changeset - multiple data' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 3 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_template_resolved_data_template_info() ORDER BY name;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templates services are ok in opened changeset - multiple data' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PERFORM commitchangeset('3');

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 3 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_resolved_data_template_info();
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok after commit - multiple data' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 3 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_template_resolved_data_template_info() ORDER BY name;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templates services are ok after commit - multiple data' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 2 ORDER BY host;
    CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_resolved_data_template_info(old_version2) ORDER BY name;
    PREPARE retservices AS SELECT * FROM old_version_resolved_data;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'host multiple data from older version' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;
    DROP TABLE old_version_resolved_data;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 2 ORDER BY host;
    CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_template_resolved_data_template_info(old_version2) ORDER BY name;
    PREPARE retservices AS SELECT * FROM old_version_resolved_data;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'host_template multiple data from older version' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;
    DROP TABLE old_version_resolved_data;

    PERFORM startchangeset();
    PERFORM host_template_add('host_templ3');
    PERFORM host_template_set_template_host('host_templ3','host_templ2');
    PERFORM host_add('hpv4');
    PERFORM host_set_template_host('hpv4','host_templ3');
    
    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 4 AND host = 'hpv4';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_resolved_object_data_template_info('hpv4');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok in opened changeset, host inherits services' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 4 AND host = 'host_templ3';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_template_resolved_object_data_template_info('host_templ3');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok in opened changeset, templates has own services' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 4 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_resolved_data_template_info();
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok in opened changeset - multiple data' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 4 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_template_resolved_data_template_info() ORDER BY name;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templates services are ok in opened changeset - multiple data' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PERFORM commitchangeset('4');

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 4 AND host = 'hpv4';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_resolved_object_data_template_info('hpv4');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok, host inherits services - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 4 AND host = 'host_templ3';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_template_resolved_object_data_template_info('host_templ3');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok, templates has own services - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 4 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_resolved_data_template_info();
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok - multiple data - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 4 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_template_resolved_data_template_info() ORDER BY name;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templates services are ok - multiple data - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;
    
    PERFORM startchangeset();
    PERFORM host_template_set_service_insert('host_templ2','dhcp');

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 5 AND host = 'hpv4';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_resolved_object_data_template_info('hpv4');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok, host inherits services - opened changeset' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 5 AND host = 'host_templ3';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_template_resolved_object_data_template_info('host_templ3');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok, templates has own services - opened changeset' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 5 AND host = 'host_templ2';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_template_resolved_object_data_template_info('host_templ2');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok, templates has own services - opened changeset' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 5 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_resolved_data_template_info();
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok - multiple data - opened changeset' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 5 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_template_resolved_data_template_info() ORDER BY name;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templates services are ok - multiple data - opened changeset' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PERFORM commitchangeset('5');

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 5 AND host = 'hpv4';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_resolved_object_data_template_info('hpv4');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok, host inherits services - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 5 AND host = 'host_templ3';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_template_resolved_object_data_template_info('host_templ3');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok, templates has own services - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 5 AND host = 'host_templ2';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_template_resolved_object_data_template_info('host_templ2');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok, templates has own services - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 5 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_resolved_data_template_info();
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templated services are ok - multiple data - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service_template WHERE version = 5 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_template_resolved_data_template_info() ORDER BY name;
    RETURN NEXT results_eq( 'retservices', 'expservices', 'templates services are ok - multiple data - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;


    PERFORM startchangeset();
    PERFORM host_set_service_remove('hpv4','ftp');

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 6 AND host = 'hpv4';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_resolved_object_data_template_info('hpv4');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'host mods service from template - opened changeset' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 6 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_resolved_data_template_info();
    RETURN NEXT results_eq( 'retservices', 'expservices', 'host mods service from template - multiple data - opened changeset' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PERFORM commitchangeset('6');

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 6 AND host = 'hpv4';
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, template_host FROM host_resolved_object_data_template_info('hpv4');
    RETURN NEXT results_eq( 'retservices', 'expservices', 'host mods service from template - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

    PREPARE expservices  AS SELECT service, note, service_templ, note_templ, template FROM pgtap.test_host_service WHERE version = 6 ORDER BY host;
    PREPARE retservices AS SELECT service, note_host, service_templ, note_host_templ, host_template_get_name(template_host) FROM host_resolved_data_template_info();
    RETURN NEXT results_eq( 'retservices', 'expservices', 'host mods service from template - multiple data - after commit' );
    DEALLOCATE expservices;
    DEALLOCATE retservices;

END
$$
LANGUAGE plpgsql;


SELECT * FROM runtests();

ROLLBACK;
