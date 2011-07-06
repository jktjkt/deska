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
	INSERT INTO pgtap.test_interface_template (name, ip4, ip6, mac, note, template, version) VALUES ('ip4_ip6_template', '192.168.0.100', 'fec0::1', '01:23:45:67:89:ab', 'interface', 'inf_template_note_mac', 4);
	INSERT INTO pgtap.test_interface_template (name, ip4, ip6, mac, note, template, version) VALUES ('inf_template_all', '192.168.0.100', 'fec0::1', '01:23:45:67:89:ab', 'another note', 'ip4_ip6_template', 4);
	INSERT INTO pgtap.test_interface (name, host, mac, note, template, version) VALUES ('host1->eth2', 'host1', '01:23:45:67:89:ab', 'interface', 'inf_template_note_mac',5);
	INSERT INTO pgtap.test_interface (name, ip4, ip6, mac, note, template, version) VALUES ('host1->eth2', '192.168.0.100', 'fec0::1', '01:23:45:67:89:ab', 'another note', 'inf_template_all', 6);
	INSERT INTO pgtap.test_interface_template (name, note, version) VALUES ('inf_note_template', 'note',7);
	INSERT INTO pgtap.test_interface_template (name, ip4, ip6, mac, note, template, version) VALUES ('inf_template_all', '192.168.0.100', 'fec0::1', '01:23:45:67:89:ab', 'another note', 'ip4_ip6_template', 7);
	INSERT INTO pgtap.test_interface_template (name, ip4, ip6, mac, note, template, version) VALUES ('ip4_ip6_template', '192.168.0.100', 'fec0::1', '01:23:45:67:89:ab', 'note', 'inf_template_note_mac', 7);
	INSERT INTO pgtap.test_interface_template (name, mac, note, template, version) VALUES ('inf_template_note_mac', '01:23:45:67:89:ab', 'note', 'inf_note_template', 7);
	INSERT INTO pgtap.test_interface (name, ip4, ip6, mac, note, template, version) VALUES ('host1->eth2', '192.168.0.100', 'fec0::1', '01:23:45:67:89:ab', 'another note', 'inf_template_all', 7);

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
	PERFORM interface_template_add('inf_template_note_mac');
	PERFORM interface_template_set_mac('inf_template_note_mac','01:23:45:67:89:ab');
	PERFORM interface_template_set_template('inf_template_note_mac','inf_note_template');
	PERFORM commitchangeset('2');

	PREPARE expresolved_data AS SELECT mac, note, template FROM pgtap.test_interface_template WHERE name = 'inf_template_note_mac' AND version = 2;
	PREPARE retresolved_data AS SELECT mac, note, template FROM interface_template_resolved_data('inf_template_note_mac');
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

	PERFORM startchangeset();
	PERFORM interface_template_add('inf_template_all');
	PERFORM interface_template_add('ip4_ip6_template');
	PERFORM interface_template_set_ip4('ip4_ip6_template','192.168.0.100');
	PERFORM interface_template_set_ip6('ip4_ip6_template','fec0::1');
	PERFORM interface_template_set_template('ip4_ip6_template', 'inf_template_note_mac');
	PERFORM interface_template_set_note('inf_template_all', 'another note');
	PERFORM interface_template_set_template('inf_template_all', 'ip4_ip6_template');
	PERFORM commitchangeset('4');

	PREPARE expresolved_data AS SELECT ip4, ip6, mac, note, template FROM pgtap.test_interface_template WHERE name = 'ip4_ip6_template' AND version = 4;
	PREPARE retresolved_data AS SELECT ip4, ip6, mac, note, template FROM interface_template_resolved_data('ip4_ip6_template');
	RETURN NEXT results_eq( 'retresolved_data', 'expresolved_data', 'resolved template data are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PREPARE expresolved_data AS SELECT  ip4, ip6, mac, note, template FROM pgtap.test_interface_template WHERE name = 'inf_template_all' AND version = 4;
	PREPARE retresolved_data AS SELECT  ip4, ip6, mac, note, template FROM interface_template_resolved_data('inf_template_all');
	RETURN NEXT results_eq( 'retresolved_data', 'expresolved_data', 'resolved template data are ok - 3 levels' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PERFORM startchangeset();
	PERFORM interface_add('host1->eth2');
	PERFORM interface_set_template('host1->eth2','inf_template_note_mac');
	old_version = revision2num(commitchangeset('5'));

	PREPARE expresolved_data AS SELECT  mac, note, template FROM pgtap.test_interface WHERE name = 'host1->eth2' AND version = 5;
	PREPARE retresolved_data AS SELECT  mac, note, template FROM interface_resolved_data('host1->eth2');
	RETURN NEXT results_eq( 'retresolved_data', 'expresolved_data', 'resolved template data are ok - 3 levels' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PERFORM startchangeset();
	PERFORM interface_set_template('host1->eth2','inf_template_all');
	PERFORM commitchangeset('6');

	PREPARE expresolved_data AS SELECT  ip4, ip6, mac, note, template FROM pgtap.test_interface WHERE name = 'host1->eth2' AND version = 6;
	PREPARE retresolved_data AS SELECT  ip4, ip6, mac, note, template FROM interface_resolved_data('host1->eth2');
	RETURN NEXT results_eq( 'retresolved_data', 'expresolved_data', 'resolved object data are ok - 3 levels' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PREPARE expresolved_data AS SELECT  ip4, ip6, mac, note, template FROM pgtap.test_interface WHERE name = 'host1->eth2' AND version = 5;
	CREATE TEMPORARY TABLE old_version_resolved_data AS SELECT * FROM interface_resolved_data('host1->eth2',old_version);
	PREPARE retresolved_data AS SELECT  ip4, ip6, mac, note, template FROM old_version_resolved_data;
	RETURN NEXT results_eq( 'retresolved_data', 'expresolved_data', 'resolved object data in old version are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PERFORM startchangeset();
	PERFORM interface_template_set_note('inf_note_template', 'note');
	PERFORM commitchangeset('7');

	PREPARE expresolved_data AS SELECT  mac, note, template FROM pgtap.test_interface_template WHERE name = 'inf_template_note_mac' AND version = 7;
	PREPARE retresolved_data AS SELECT  mac, note, template FROM interface_template_resolved_data('inf_template_note_mac');
	RETURN NEXT results_eq( 'retresolved_data', 'expresolved_data', 'resolved template data after change in parent template' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

	PREPARE expresolved_data AS SELECT  name, note FROM pgtap.test_interface_template WHERE name IN ('inf_template_note_mac', 'inf_template_all', 'ip4_ip6_template', 'inf_note_template') AND version = 7 order by name;
	PREPARE retresolved_data AS SELECT  cast(name as text), note FROM production.interface_template WHERE name IN ('inf_template_note_mac', 'inf_template_all', 'ip4_ip6_template', 'inf_note_template') order by name;
	RETURN NEXT results_eq( 'retresolved_data', 'expresolved_data', 'data in production after modification in template are ok' );
	DEALLOCATE expresolved_data;
	DEALLOCATE retresolved_data;

END
$$
LANGUAGE plpgsql;

SELECT * FROM runtests();

ROLLBACK; 
