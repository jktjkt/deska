BEGIN;

\i util-include-path.sql


/* ************************************************
*	 testing add, set, resolve					***
************************************************ */
CREATE TABLE pgtap.test_hardware_template(
	name text,
	vendor text,
	purchase date,
	warranty date,
	ram int,
	cpu_num int,
	note text,
	template text,
	version bigint
);

CREATE TABLE pgtap.test_hardware(
	name text,
	vendor text,
	purchase date,
	warranty date,
	ram int,
	cpu_num int,
	note text,
	template text,
	version bigint
);

CREATE FUNCTION test_template()
RETURNS SETOF text
AS
$$
DECLARE
	old_version bigint;
BEGIN
	INSERT INTO pgtap.test_hardware_template (name, purchase, version) VALUES ('purchase_template', '2001-1-1',1);
	INSERT INTO pgtap.test_hardware_template (name, purchase, warranty, template, version) VALUES ('purchase_warranty_template', '2001-1-1', '2008-4-8', 'purchase_template',1);
	INSERT INTO pgtap.test_hardware_template (name, purchase, warranty, ram, cpu_num, template, version) VALUES ('ram_cpu_purchase_warranty_template', '2001-1-1', '2008-4-8', 4000, 4, 'purchase_warranty_template', 2);
	INSERT INTO pgtap.test_hardware_template (name, purchase, warranty, ram, cpu_num, note, template, version) VALUES ('template_all', '2001-1-1', '2008-4-8', 4000, 4, 'blabla note', 'ram_cpu_purchase_warranty_template', 3);
	INSERT INTO pgtap.test_hardware_template (name, purchase, warranty, template, version) VALUES ('purchase_warranty_template', '2001-4-8', '2008-4-8', 'purchase_template',4);
	INSERT INTO pgtap.test_hardware_template (name, purchase, warranty, ram, cpu_num, note, template, version) VALUES ('template_all', '2001-4-8', '2008-4-8', 4000, 4, 'blabla note', 'ram_cpu_purchase_warranty_template', 4);
	INSERT INTO pgtap.test_hardware (name, vendor, purchase, warranty, ram, cpu_num, note, template, version) VALUES ('templated_hardware_1', 'vendor_1', '2001-4-8', '2008-4-8', 4000, 4, 'blabla note', 'template_all', 5);
	INSERT INTO pgtap.test_hardware (name, vendor, purchase, warranty, ram, cpu_num, note, template, version) VALUES ('templated_hardware_2', 'vendor_2', '2010-12-12', '2016-12-12', 4000, 4, null, 'ram_cpu_purchase_warranty_template', 5);
	INSERT INTO pgtap.test_hardware (name, vendor, purchase, warranty, ram, cpu_num, note, template, version) VALUES ('templated_hardware_2', 'vendor_2', '2010-12-12', '2016-12-12', 4000, 4, 'note2', 'ram_cpu_purchase_warranty_template', 6);
	INSERT INTO pgtap.test_hardware_template (name, purchase, note, version) VALUES ('purchase_template', '2001-1-1', 'note2',6);
	INSERT INTO pgtap.test_hardware_template (name, purchase, warranty, note, template, version) VALUES ('purchase_warranty_template', '2001-4-8', '2008-4-8', 'note2', 'purchase_template',6);
	INSERT INTO pgtap.test_hardware_template (name, purchase, warranty, ram, cpu_num, note, template, version) VALUES ('ram_cpu_purchase_warranty_template', '2001-4-8', '2008-4-8', 4000, 4, 'note2', 'purchase_warranty_template', 6);
	INSERT INTO pgtap.test_hardware_template (name, purchase, warranty, ram, cpu_num, note, template, version) VALUES ('template_all', '2001-4-8', '2008-4-8', 4000, 4, 'blabla note', 'ram_cpu_purchase_warranty_template', 6);
	INSERT INTO pgtap.test_hardware (name, vendor, purchase, warranty, ram, cpu_num, note, template, version) VALUES ('templated_hardware_2', 'vendor_2', '2010-12-12', '2016-12-12', 4000, 4, 'note3', 'ram_cpu_purchase_warranty_template', 7);
	INSERT INTO pgtap.test_hardware_template (name, purchase, note, version) VALUES ('purchase_template', '2001-1-1', 'note3',7);
	INSERT INTO pgtap.test_hardware_template (name, purchase, warranty, note, template, version) VALUES ('purchase_warranty_template', '2001-4-8', '2008-4-8', 'note3', 'purchase_template',7);
	INSERT INTO pgtap.test_hardware_template (name, purchase, warranty, ram, cpu_num, note, template, version) VALUES ('ram_cpu_purchase_warranty_template', '2001-4-8', '2008-4-8', 4000, 4, 'note3', 'purchase_warranty_template', 7);
	
	PERFORM startChangeset();
	PERFORM hardware_template_add('purchase_template');
	PERFORM hardware_template_set_purchase('purchase_template','2001-1-1');
	PERFORM hardware_template_add('purchase_warranty_template');
	PERFORM hardware_template_set_warranty('purchase_warranty_template','2008-4-8');
	PERFORM hardware_template_set_template('purchase_warranty_template','purchase_template');
	PERFORM commitChangeset('1');

	PREPARE exptemplates  AS SELECT name FROM pgtap.test_hardware_template WHERE version = 1;
	PREPARE rettemplates AS SELECT * FROM hardware_template_names();
	RETURN NEXT set_has( 'rettemplates', 'exptemplates', 'created templates are present' );	
	DEALLOCATE exptemplates;
	DEALLOCATE rettemplates;

	PREPARE expresolved_data AS SELECT purchase,warranty,template FROM pgtap.test_hardware_template WHERE name = 'purchase_warranty_template' AND version = 1;
	PREPARE retresolved_data AS SELECT purchase,warranty,template FROM hardware_template_resolved_data('purchase_warranty_template');
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data are ok' );	
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;


	PERFORM startChangeset();
	PERFORM hardware_template_add('ram_cpu_purchase_warranty_template');
	PERFORM hardware_template_set_ram('ram_cpu_purchase_warranty_template','4000');
	PERFORM hardware_template_set_cpu_num('ram_cpu_purchase_warranty_template','4');
	PERFORM hardware_template_set_template('ram_cpu_purchase_warranty_template','purchase_warranty_template');
	PERFORM commitChangeset('1');

	PREPARE expresolved_data AS SELECT purchase,warranty,template FROM pgtap.test_hardware_template WHERE name = 'ram_cpu_purchase_warranty_template' AND version = 2;
	PREPARE retresolved_data AS SELECT purchase,warranty,template FROM hardware_template_resolved_data('ram_cpu_purchase_warranty_template');
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data are ok - 2 levels' );	
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	
	PERFORM startChangeset();
	PERFORM hardware_template_add('template_all');
	PERFORM hardware_template_set_note('template_all','blabla note');
	PERFORM hardware_template_set_template('template_all','ram_cpu_purchase_warranty_template');
	PERFORM commitChangeset('3');

	PREPARE expresolved_data AS SELECT purchase,warranty, ram, cpu_num, template FROM pgtap.test_hardware_template WHERE name = 'template_all' AND version = 3;
	PREPARE retresolved_data AS SELECT purchase,warranty, ram, cpu_num, template FROM hardware_template_resolved_data('template_all');
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data are ok - 3 levels' );	
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PERFORM startChangeset();
	PERFORM hardware_template_set_purchase('purchase_warranty_template','2001-4-8');
	PERFORM commitChangeset('3');

	PREPARE expresolved_data AS SELECT purchase,warranty, ram, cpu_num, template FROM pgtap.test_hardware_template WHERE name = 'purchase_warranty_template' AND version = 4;
	PREPARE retresolved_data AS SELECT purchase,warranty, ram, cpu_num, template FROM hardware_template_resolved_data('purchase_warranty_template');
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data are ok - overwritten attribute in lowest level' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PREPARE expresolved_data AS SELECT purchase,warranty, ram, cpu_num, note, template FROM pgtap.test_hardware_template WHERE name = 'template_all' AND version = 4;
	PREPARE retresolved_data AS SELECT purchase,warranty, ram, cpu_num, note, template FROM hardware_template_resolved_data('template_all');
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data are ok - overwritten attribute in 3rd level' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PERFORM startChangeset();
	PERFORM vendor_add('vendor_1');
	PERFORM hardware_add('templated_hardware_1');
	PERFORM hardware_set_vendor('templated_hardware_1','vendor_1');
	PERFORM hardware_set_template('templated_hardware_1','template_all');
	PERFORM vendor_add('vendor_2');
	PERFORM hardware_add('templated_hardware_2');
	PERFORM hardware_set_vendor('templated_hardware_2','vendor_2');
	PERFORM hardware_set_template('templated_hardware_2','ram_cpu_purchase_warranty_template');
	PERFORM hardware_set_purchase('templated_hardware_2','2010-12-12');
	PERFORM hardware_set_warranty('templated_hardware_2','2016-12-12');
	PERFORM commitChangeset('1');

	PREPARE expresolved_data AS SELECT vendor, purchase,warranty, ram, cpu_num, note, template FROM pgtap.test_hardware WHERE name = 'templated_hardware_1' AND version = 5;
	PREPARE retresolved_data AS SELECT vendor, purchase,warranty, ram, cpu_num, note, template FROM hardware_resolved_data('templated_hardware_1');
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data of templated hardware are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PREPARE expresolved_data AS SELECT vendor, purchase,warranty, ram, cpu_num, note, template FROM pgtap.test_hardware WHERE name = 'templated_hardware_2' AND version = 5;
	PREPARE retresolved_data AS SELECT vendor, purchase,warranty, ram, cpu_num, note, template FROM hardware_resolved_data('templated_hardware_2');
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data of templated hardware are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PERFORM startChangeset();
	PERFORM hardware_template_set_note('purchase_template','note2');
	old_version = revision2num(commitChangeset('1'));

	PREPARE expresolved_data AS SELECT vendor, purchase,warranty, ram, cpu_num, note, template FROM pgtap.test_hardware WHERE name = 'templated_hardware_2' AND version = 6;
	PREPARE retresolved_data AS SELECT vendor, purchase,warranty, ram, cpu_num, note, template FROM hardware_resolved_data('templated_hardware_2');
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data of templated hardware after change in template are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PREPARE expresolved_data AS SELECT purchase,note FROM pgtap.test_hardware_template WHERE name = 'purchase_template' AND version = 6;
	PREPARE retresolved_data AS SELECT purchase,note FROM hardware_template WHERE name = 'purchase_template';
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data of hardware_template after change in template are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PREPARE expresolved_data AS SELECT purchase,warranty, note, template FROM pgtap.test_hardware_template WHERE name = 'purchase_warranty_template' AND version = 6;
	PREPARE retresolved_data AS SELECT purchase,warranty, note, template FROM hardware_template_resolved_data('purchase_warranty_template');
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data of hardware_template after change in template are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PREPARE expresolved_data AS SELECT vendor, purchase,warranty, ram, cpu_num, note, template FROM pgtap.test_hardware_template WHERE name = 'ram_cpu_purchase_warranty_template' AND version = 6;
	PREPARE retresolved_data AS SELECT vendor, purchase,warranty, ram, cpu_num, note, template FROM hardware_template_resolved_data('ram_cpu_purchase_warranty_template');
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data of hardware_template after change in template are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PREPARE expresolved_data AS SELECT vendor, purchase,warranty, ram, cpu_num, note, template FROM pgtap.test_hardware_template WHERE name = 'template_all' AND version = 6;
	PREPARE retresolved_data AS SELECT vendor, purchase,warranty, ram, cpu_num, note, template FROM hardware_template_resolved_data('template_all');
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data of hardware_template after change in template are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PERFORM startChangeset();
	PERFORM hardware_template_set_note('purchase_template','note3');
	PERFORM commitChangeset('1');

	PREPARE expresolved_data AS SELECT vendor, purchase,warranty, ram, cpu_num, note, template FROM pgtap.test_hardware WHERE name = 'templated_hardware_2' AND version = 7;
	PREPARE retresolved_data AS SELECT vendor_get_name(vendor), purchase,warranty, ram, cpu_num, note, hardware_template_get_name(template) FROM production.hardware WHERE name = 'templated_hardware_2';
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved templated data of hardware in production after change in template are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PREPARE expresolved_data AS SELECT vendor, purchase,warranty, ram, cpu_num, note, template FROM pgtap.test_hardware WHERE name = 'templated_hardware_2' AND version = 6;
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT * FROM hardware_resolved_data('templated_hardware_2',old_version);
	PREPARE retresolved_data AS SELECT vendor, purchase,warranty, ram, cpu_num, note, template FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved templated data of hardware in different versions are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	DELETE FROM pgtap.test_hardware_template;
	DELETE FROM pgtap.test_hardware;

END
$$
LANGUAGE plpgsql;

CREATE FUNCTION test_set_template()
RETURNS SETOF text
AS
$$
BEGIN
	INSERT INTO pgtap.test_hardware (name, purchase, warranty, template, version) VALUES ('templated_hardware', '2001-1-1', '2011-1-1', 'first_template', 1);
	INSERT INTO pgtap.test_hardware (name, purchase, warranty, template, version) VALUES ('templated_hardware', '2002-2-2', '2012-2-2', 'second_template', 2);

	PERFORM startChangeset();
	PERFORM hardware_template_add('first_template');
	PERFORM hardware_template_set_purchase('first_template','2001-1-1');
	PERFORM hardware_template_set_warranty('first_template','2011-1-1');
	PERFORM hardware_template_add('second_template');
	PERFORM hardware_template_set_purchase('second_template','2002-2-2');
	PERFORM hardware_template_set_warranty('second_template','2012-2-2');
	PERFORM commitChangeset('1');

	PERFORM startChangeset();
	PERFORM hardware_add('templated_hardware');
	PERFORM hardware_set_template('templated_hardware','first_template');
	PERFORM commitChangeset('2');

	PREPARE expresolved_data AS SELECT purchase, warranty, template FROM pgtap.test_hardware WHERE name = 'templated_hardware' AND version = 1;
	PREPARE retresolved_data AS SELECT purchase, warranty, template FROM hardware_resolved_data('templated_hardware');
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data of hardware are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PERFORM startChangeset();
	PERFORM hardware_set_template('templated_hardware','second_template');
	PERFORM commitChangeset('3');

	PREPARE expresolved_data AS SELECT purchase, warranty, template FROM pgtap.test_hardware WHERE name = 'templated_hardware' AND version = 2;
	PREPARE retresolved_data AS SELECT purchase, warranty, template FROM hardware_resolved_data('templated_hardware');
	RETURN NEXT results_eq( 'expresolved_data', 'retresolved_data', 'resolved template data of hardware after changing template of hardware are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	DELETE FROM pgtap.test_hardware_template;
	DELETE FROM pgtap.test_hardware;
END
$$
LANGUAGE plpgsql;

SELECT * FROM runtests();

ROLLBACK; 
