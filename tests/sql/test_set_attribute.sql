BEGIN;

\i util-include-path.sql

/* ************************************************
*	 testing set_attribute() + kind_names()		***
************************************************ */

CREATE TABLE set_atts_host(
	name text,
	note text,
	version int
);

--tests if attributes are set
--in set function is added row to object history, test if object is in kindinstances exactly ones

CREATE FUNCTION test_set_attribute_embed()
RETURNS SETOF text
AS
$$
BEGIN
	INSERT INTO set_atts_host(name, version) values('hpv1', 1);
	INSERT INTO set_atts_host(name, note, version) values('hpv1', 'host hpv1', 2);
	INSERT INTO set_atts_host(name, note, version) values('hpv2', 'host hpv2', 2);
	INSERT INTO set_atts_host(name, note, version) values('hpv2', 'host hpv2', 3);

	PERFORM startchangeset();
	PERFORM host_add('hpv1');
	PERFORM commitchangeset('hpv1 added');

	PREPARE retnames AS SELECT host_names();
	PREPARE expnames AS SELECT name FROM set_atts_host WHERE version = 1;
	RETURN NEXT set_eq( 'retnames', 'expnames', 'lists of hosts are same' );

	DEALLOCATE retnames;
	DEALLOCATE expnames;	

	PERFORM startchangeset();
	PERFORM host_set_host_note('hpv1', 'host hpv1');
	PERFORM host_add('hpv2');
	PERFORM host_set_host_note('hpv2', 'host hpv2');

	PREPARE retnames AS SELECT host_note FROM host_get_data('hpv1');
	PREPARE expnames AS SELECT note FROM set_atts_host WHERE name = 'hpv1' AND version = 2;
	RETURN NEXT set_eq( 'retnames', 'expnames', 'vendor_list is same' );	

	DEALLOCATE retnames;
	DEALLOCATE expnames;	

	PREPARE retnames AS SELECT host_names();
	PREPARE expnames AS SELECT name FROM set_atts_host WHERE version = 2;
	RETURN NEXT set_eq( 'retnames', 'expnames', 'lists of hosts are same in opened changeset' );

	PERFORM commitchangeset('hpv1 note set');


	DEALLOCATE retnames;
	DEALLOCATE expnames;	
	
	PREPARE retnotes AS SELECT host_note FROM host_get_data('hpv1');
	PREPARE expnotes AS SELECT note FROM set_atts_host WHERE name = 'hpv1' AND version = 2;
	RETURN NEXT set_eq( 'retnotes', 'expnotes', 'lists of hosts after commit are same' );	

	DEALLOCATE retnotes;
	DEALLOCATE expnotes;	


	PERFORM startchangeset();
	PERFORM host_del('hpv1');

	PREPARE retnames AS SELECT host_names();
	PREPARE expnames AS SELECT name FROM set_atts_host WHERE version = 3;
	RETURN NEXT set_eq( 'retnames', 'expnames', 'lists of hosts after commit are same' );	

	DEALLOCATE retnames;
	DEALLOCATE expnames;	

	PERFORM host_del('hpv2');

	PERFORM commitchangeset('del hosts');

END;
$$
language plpgsql;



/* ****************************************************
*	 testing set_attribute() + kind_names() embed	***
***************************************************** */

CREATE TABLE set_atts_interface(
	name text,
	note text,
	version int
);

CREATE FUNCTION test_set_attribute()
RETURNS SETOF text
AS
$$
BEGIN
	INSERT INTO set_atts_interface(name, version) values('hpv1->eth0', 1);
	INSERT INTO set_atts_interface(name, note, version) values('hpv1->eth0', 'interface hpv1->eth0', 2);
	INSERT INTO set_atts_interface(name, note, version) values('hpv1->eth1', 'interface hpv1->eth1', 2);
	INSERT INTO set_atts_interface(name, note, version) values('hpv1->eth1', 'interface hpv1->eth1', 3);

	PERFORM startchangeset();
	PERFORM host_add('hpv1');
	PERFORM interface_add('hpv1->eth0');
	PERFORM commitchangeset('hpv1->eth0 added');

	PREPARE retnames AS SELECT interface_names();
	PREPARE expnames AS SELECT name FROM set_atts_interface WHERE version = 1;
	RETURN NEXT set_eq( 'retnames', 'expnames', 'lists of hosts are same' );

	DEALLOCATE retnames;
	DEALLOCATE expnames;	

	PERFORM startchangeset();
	PERFORM interface_set_note('hpv1->eth0', 'interface hpv1->eth0');
	PERFORM interface_add('hpv1->eth1');
	PERFORM interface_set_note('hpv1->eth1', 'interface hpv1->eth1');

	PREPARE retnames AS SELECT note FROM interface_get_data('hpv1->eth0');
	PREPARE expnames AS SELECT note FROM set_atts_interface WHERE name = 'hpv1->eth0' AND version = 2;
	RETURN NEXT set_eq( 'retnames', 'expnames', 'data in opened changeset are ok' );	

	DEALLOCATE retnames;
	DEALLOCATE expnames;	

	PREPARE retnames AS SELECT interface_names();
	PREPARE expnames AS SELECT name FROM set_atts_interface WHERE version = 2;
	RETURN NEXT set_eq( 'retnames', 'expnames', 'lists of interaces are same in opened changeset' );

	PERFORM commitchangeset('hpv1->eth0,hpv1->eth1 note set');

	DEALLOCATE retnames;
	DEALLOCATE expnames;	
	
	PREPARE retnames AS SELECT note FROM interface_get_data('hpv1->eth0');
	PREPARE expnames AS SELECT note FROM set_atts_interface WHERE name = 'hpv1->eth0' AND version = 2;
	RETURN NEXT set_eq( 'retnames', 'expnames', 'lists of interfaces after commit are same' );	

	DEALLOCATE retnames;
	DEALLOCATE expnames;	

	PERFORM startchangeset();
	PERFORM interface_del('hpv1->eth0');

	PREPARE retnames AS SELECT interface_names();
	PREPARE expnames AS SELECT name FROM set_atts_interface WHERE version = 3;
	RETURN NEXT set_eq( 'retnames', 'expnames', 'lists of interfaces after commit are same' );	

	DEALLOCATE retnames;
	DEALLOCATE expnames;	

	PERFORM interface_del('hpv1->eth1');

	PERFORM commitchangeset('del interfaces');	

END;
$$
language plpgsql;

SELECT * FROM runtests();

ROLLBACK;
