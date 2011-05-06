\i util-include-path.sql

BEGIN;

CREATE TABLE simple_add_set_hardware_names(
	 name text
);

CREATE TABLE simple_add_set_vendor_names(
	 name text
);

CREATE OR REPLACE FUNCTION pgtap.test_simple_add_set()
RETURNS SETOF TEXT AS
$$
DECLARE
BEGIN

	 PERFORM startChangeset();
	 PERFORM vendor_add('DELL');
	 PERFORM hardware_add('hwDELL');
	 PERFORM hardware_set_vendor('hwDELL','DELL');
	 PERFORM hardware_set_purchase('hwDELL','2000-4-8');
	 PERFORM hardware_set_warranty('hwDELL','2014-7-25');
	 PERFORM commitChangeset('add vendor dell, hardware hwdell');

	 PERFORM startChangeset();
	 PERFORM vendor_add('HP');
	 PERFORM hardware_add('hwHP');
	 PERFORM hardware_set_vendor('hwHP','HP');
	 PERFORM hardware_set_purchase('hwHP','2007-10-16');
	 PERFORM hardware_set_warranty('hwHP','2011-3-16');
	 PERFORM commitChangeset('add vendor HP, hardware hwhp');

	 INSERT INTO simple_add_set_vendor_names (name) VALUES ('DELL');
	 INSERT INTO simple_add_set_vendor_names (name) VALUES ('HP');

	 INSERT INTO simple_add_set_hardware_names (name) VALUES ('hwDELL');
	 INSERT INTO simple_add_set_hardware_names (name) VALUES ('hwHP');
	
	 PREPARE retnames AS SELECT vendor_names();
	 PREPARE expnames AS SELECT name FROM simple_add_set_vendor_names;
	 RETURN NEXT set_eq( 'retnames', 'expnames', 'added vendors are present' );

	 DEALLOCATE retnames;
	 DEALLOCATE expnames;

	 PREPARE retnames AS SELECT hardware_names();
	 PREPARE expnames AS SELECT name FROM simple_add_set_hardware_names;
	 RETURN NEXT set_eq( 'retnames', 'expnames', 'added hardware is present' );

	 DEALLOCATE retnames;
	 DEALLOCATE expnames;
END;
$$
LANGUAGE plpgsql;

CREATE TABLE simple_add_set_hardware_names_versions(
	 name text,
	 version bigint
);

CREATE TABLE simple_add_set_vendor_names_versions(
	 name text,
	 version bigint
);

CREATE OR REPLACE FUNCTION pgtap.test_simple_add_set_versions()
RETURNS SETOF TEXT AS
$$
DECLARE
	 changeset1_id bigint;
	 changeset2_id bigint;
	 version1 bigint;
	 version2 bigint;
	 pom text;
BEGIN

	 PERFORM startChangeset();
	 changeset1_id = get_current_changeset();
	 PERFORM vendor_add('DELL');
	 PERFORM hardware_add('hwDELL');
	 PERFORM hardware_set_vendor('hwDELL','DELL');
	 PERFORM hardware_set_purchase('hwDELL','2000-4-8');
	 PERFORM hardware_set_warranty('hwDELL','2014-7-25');
	 PERFORM commitChangeset('add vendor dell, hardware hwdell');

	 PERFORM startChangeset();
	 changeset2_id = get_current_changeset();
	 PERFORM vendor_add('HP');
	 PERFORM hardware_add('hwHP');
	 PERFORM hardware_set_vendor('hwHP','HP');
	 PERFORM hardware_set_purchase('hwHP','2007-10-16');
	 PERFORM hardware_set_warranty('hwHP','2011-3-16');
	 PERFORM commitChangeset('add vendor HP, hardware hwhp');

	 INSERT INTO simple_add_set_vendor_names_versions (name, version) VALUES ('DELL',1);
	 INSERT INTO simple_add_set_vendor_names_versions (name, version) VALUES ('HP',2);

	 INSERT INTO simple_add_set_hardware_names_versions (name, version) VALUES ('hwDELL',1);
	 INSERT INTO simple_add_set_hardware_names_versions (name, version) VALUES ('hwHP',2);
	
	 version1 = id2num(changeset1_id);
	 CREATE TEMPORARY TABLE version1_vendor_names(name) AS SELECT vendor_names(version1);
	 PREPARE retnames AS SELECT * FROM version1_vendor_names;
	 PREPARE expnames AS SELECT name FROM simple_add_set_vendor_names_versions WHERE version = 1;
	 RETURN NEXT results_eq( 'retnames', 'expnames', 'added vendors are present' );

	 DEALLOCATE retnames;
	 DEALLOCATE expnames;

	 CREATE TEMPORARY TABLE version1_hardware_names(name) AS SELECT hardware_names(version1);
	 PREPARE retnames AS SELECT * FROM version1_hardware_names;
	 PREPARE expnames AS SELECT name FROM simple_add_set_hardware_names_versions WHERE version = 1;
	 RETURN NEXT results_eq( 'retnames', 'expnames', 'added hardware is present' );

	 DEALLOCATE retnames;
	 DEALLOCATE expnames;

	 version2 = id2num(changeset2_id);
	 CREATE TEMPORARY TABLE version2_vendor_names(name) AS SELECT vendor_names(version2);
	 PREPARE retnames AS SELECT * FROM version2_vendor_names;
	 PREPARE expnames AS SELECT name FROM simple_add_set_vendor_names_versions WHERE version <= 2;
	 RETURN NEXT results_eq( 'retnames', 'expnames', 'added vendors are present' );

	 DEALLOCATE retnames;
	 DEALLOCATE expnames;

	 CREATE TEMPORARY TABLE version2_hardware_names(name) AS SELECT hardware_names(version2);
	 PREPARE retnames AS SELECT * FROM version2_hardware_names;
	 PREPARE expnames AS SELECT name FROM simple_add_set_hardware_names_versions WHERE version <= 2;
	 RETURN NEXT results_eq( 'retnames', 'expnames', 'added hardware is present' );

	 DEALLOCATE retnames;
	 DEALLOCATE expnames;
END;
$$
LANGUAGE plpgsql;


SELECT * FROM runtests();

ROLLBACK; 
