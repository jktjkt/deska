BEGIN;

\i util-include-path.sql


/* ************************************************
*	 testing add, set, resolve					***
************************************************ */
CREATE TABLE pgtap.test_interface_template(
	name text,
	ip4 ipv4,
	ip6 ipv6,
	mac macaddr,
	note text,
	template text,
	version bigint
);

CREATE TABLE pgtap.test_interface(
	name text,
	host text,
	ip4 ipv4,
	ip6 ipv6,
	mac macaddr,
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
	INSERT INTO pgtap.test_interface_template (name, note, version) VALUES ('inf_note_template', 'interface',1);
	INSERT INTO pgtap.test_interface (name, host, note, template, version) VALUES ('host1->eth0', 'host1', 'interface', 'inf_note_template',1);
	INSERT INTO pgtap.test_interface_template (name, mac, note, template, version) VALUES ('inf_template_note_mac', '01:23:45:67:89:ab', 'interface', 'inf_note_template', 2);
	INSERT INTO pgtap.test_interface (name, host, mac, note, template, version) VALUES ('host1->eth1', 'host1', '01:23:45:67:89:ab', 'interface', 'inf_template_note_mac',3);

	PERFORM startChangeset();
	PERFORM host_add('host1');
	PERFORM host_add('host2');

	PERFORM interface_add('host1->eth0');
	PERFORM interface_template_add('inf_note_template');
	PERFORM interface_template_set_note('inf_note_template','interface');
	PERFORM interface_set_template('host1->eth0','inf_note_template');
	PERFORM commitchangeset('1');

	PREPARE exptemplates  AS SELECT name FROM pgtap.test_interface_template WHERE version = 1;
	PREPARE rettemplates AS SELECT * FROM interface_template_names();
	RETURN NEXT set_has( 'rettemplates', 'exptemplates', 'created templates are present' );	
	DEALLOCATE exptemplates;
	DEALLOCATE rettemplates;

	PREPARE expresolved_data AS SELECT note FROM pgtap.test_interface_template WHERE name = 'inf_note_template' AND version = 1;
	PREPARE retresolved_data AS SELECT note FROM interface_template_resolved_data('inf_note_template');
	RETURN NEXT results_eq( 'retresolved_data', 'expresolved_data', 'resolved template data are ok' );	
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PREPARE expresolved_data AS SELECT host, note, template FROM pgtap.test_interface WHERE name = 'host1->eth0' AND version = 1;
	PREPARE retresolved_data AS SELECT host, note, template FROM interface_resolved_data('host1->eth0');
	RETURN NEXT results_eq( 'retresolved_data', 'expresolved_data', 'resolved object data are ok' );	
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PERFORM startchangeset();
	PERFORM interface_template_add('inf_template_note_host');

	PREPARE my_thrower AS SELECT interface_template_set_host('inf_template_note_host','nonexisting');
	RETURN NEXT throws_ok('my_thrower');

	PERFORM interface_template_set_mac('inf_template_note_mac','01:23:45:67:89:ab');
	PERFORM interface_template_set_template('inf_template_note_host','inf_note_template');
	PERFORM commitchangeset('2');

	PREPARE expresolved_data AS SELECT host, note, template FROM pgtap.test_interface_template WHERE name = 'inf_template_note_mac' AND version = 2;
	PREPARE retresolved_data AS SELECT host, note, template FROM interface_template_resolved_data('inf_template_note_mac');
	RETURN NEXT results_eq( 'retresolved_data', 'expresolved_data', 'resolved template data are ok' );	
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PERFORM startchangeset();
	PERFORM interface_add('host1->eth1');
	PERFORM interface_set_template('host1->eth1','inf_template_note_mac');
	PERFORM commitchangeset('3');

	PREPARE expresolved_data AS SELECT host, note, template FROM pgtap.test_interface WHERE name = 'host1->eth1' AND version = 3;
	PREPARE retresolved_data AS SELECT host, note, template FROM interface_resolved_data('host1->eth1');
	RETURN NEXT results_eq( 'retresolved_data', 'expresolved_data', 'resolved object data are ok - 2 levels' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	

END
$$
LANGUAGE plpgsql;


/* ************************************************
*	 testing  replacing template with another	***
************************************************ */


SELECT * FROM runtests();

ROLLBACK; 
