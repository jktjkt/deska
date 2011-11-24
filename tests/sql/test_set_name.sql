BEGIN;

\i util-include-path.sql


CREATE TABLE test_hardware(
	name text,
	version int
);

CREATE FUNCTION test_set_name()
RETURNS SETOF text
AS
$$
BEGIN
	INSERT INTO test_hardware VALUES ('HPhw',1);
	INSERT INTO test_hardware VALUES ('HPhwNewName',2);

	PERFORM startchangeset();
	PERFORM vendor_add('HP');
	PERFORM hardware_add('HPhw');
	PERFORM hardware_set_vendor('HPhw', 'HP');
	PERFORM hardware_set_purchase('HPhw', '02-01-2010');
	PERFORM hardware_set_warranty('HPhw', '02-01-2014');
	
	PREPARE retnames AS SELECT hardware_names();
	PREPARE expnames AS SELECT name FROM test_hardware WHERE version = 1;
	RETURN NEXT set_has('retnames', 'expnames', 'returns good names in changeset');
	DEALLOCATE retnames;
	DEALLOCATE expnames;

	PERFORM hardware_set_name('HPhw', 'HPhwNewName');

	PREPARE retnames AS SELECT hardware_names();
	PREPARE expnames AS SELECT name FROM test_hardware WHERE version = 2;
	RETURN NEXT set_has('retnames', 'expnames', 'returns good after rename, names in changeset');
	PREPARE not_in_expnames AS SELECT name FROM test_hardware WHERE version = 1;
	RETURN NEXT set_hasnt('retnames', 'not_in_expnames', 'returns good after rename, names in changeset');
	
	DEALLOCATE retnames;
	DEALLOCATE expnames;
	DEALLOCATE not_in_expnames;

	PERFORM commitchangeset('');

	PREPARE retnames AS SELECT hardware_names();
	PREPARE expnames AS SELECT name FROM test_hardware WHERE version = 2;
	RETURN NEXT set_has('retnames', 'expnames', 'returns good names after commit');
	PREPARE not_in_expnames AS SELECT name FROM test_hardware WHERE version = 1;
	RETURN NEXT set_has('retnames', 'expnames', 'returns good after rename, names in changeset');

	DEALLOCATE retnames;
	DEALLOCATE expnames;
	DEALLOCATE not_in_expnames;
END
$$
LANGUAGE plpgsql;


CREATE TABLE test_interface(
	name text,
	version int
);


CREATE FUNCTION test_set_name_embed()
RETURNS SETOF text
AS
$$
BEGIN
	INSERT INTO test_interface VALUES ('hosthp1->eth0',1);
	INSERT INTO test_interface VALUES ('hosthp2->eth4',2);

	PERFORM startchangeset();
	PERFORM vendor_add('HP');
	PERFORM hardware_add('HPhw');
	PERFORM hardware_set_vendor('HPhw', 'HP');
	PERFORM hardware_set_purchase('HPhw', '02-01-2010');
	PERFORM hardware_set_warranty('HPhw', '02-01-2014');
	PERFORM host_add('hosthp1');
	RETURN NEXT throws_ilike('SELECT host_set_hardware(''hosthp1'',''HPhw'')', 'Column hardware in table host is read only.', 'Set on read-only column raises appropriate error.');
	PERFORM host_add('hosthp2');
	RETURN NEXT throws_ilike('SELECT  host_set_hardware(''hosthp2'',''HPhw'')', 'Column hardware in table host is read only.', 'Set on read-only column raises appropriate error.');
	PERFORM interface_add('hosthp1->eth0');
	
	PREPARE retnames AS SELECT interface_names();
	PREPARE expnames AS SELECT name FROM test_interface WHERE version = 1;
	RETURN NEXT set_has('retnames', 'expnames', 'returns good names in changeset');
	DEALLOCATE retnames;
	DEALLOCATE expnames;

	PERFORM interface_set_name('hosthp1->eth0', 'hosthp2->eth4');

	PREPARE retnames AS SELECT interface_names();
	PREPARE expnames AS SELECT name FROM test_interface WHERE version = 2;
	RETURN NEXT set_has('retnames', 'expnames', 'returns good after rename, names in changeset');
	PREPARE not_in_expnames AS SELECT name FROM test_interface WHERE version = 1;
	RETURN NEXT set_hasnt('retnames', 'not_in_expnames', 'returns good after rename, names in changeset');
	
	DEALLOCATE retnames;
	DEALLOCATE expnames;
	DEALLOCATE not_in_expnames;

	PERFORM commitchangeset('');

	PREPARE retnames AS SELECT interface_names();
	PREPARE expnames AS SELECT name FROM test_interface WHERE version = 2;
	RETURN NEXT set_has('retnames', 'expnames', 'returns good names after commit');
	PREPARE not_in_expnames AS SELECT name FROM test_interface WHERE version = 1;
	RETURN NEXT set_has('retnames', 'expnames', 'returns good after rename, names in changeset');

	DEALLOCATE retnames;
	DEALLOCATE expnames;
	DEALLOCATE not_in_expnames;

	DELETE FROM test_interface;
END
$$
LANGUAGE plpgsql;

--tries if set name works good, even if both part of embed_object's names are modified 
CREATE FUNCTION  test_set_both_part()
RETURNS SETOF text
AS
$$
BEGIN
	INSERT INTO test_interface VALUES ('hosthp1->eth0',1);
	INSERT INTO test_interface VALUES ('hosthp2->eth4',2);
	INSERT INTO test_interface VALUES ('hosthp1->eth0',3);

	PERFORM startchangeset();
	PERFORM host_add('hosthp1');
	PERFORM host_add('hosthp2');
	PERFORM interface_add('hosthp1->eth0');
	PERFORM commitchangeset('');

	PREPARE retnames AS SELECT interface_names();
	PREPARE expnames AS SELECT name FROM test_interface WHERE version = 1;
	RETURN NEXT set_has('retnames', 'expnames', 'returns good names in changeset');
	DEALLOCATE retnames;
	DEALLOCATE expnames;

	PERFORM startchangeset();
	PERFORM interface_set_name('hosthp1->eth0', 'hosthp1->eth4');
	PERFORM interface_set_host('hosthp1->eth4', 'hosthp2');
	PERFORM commitchangeset('');

	PREPARE retnames AS SELECT interface_names();
	PREPARE expnames AS SELECT name FROM test_interface WHERE version = 2;
	RETURN NEXT set_has('retnames', 'expnames', 'returns good names after renamening interface and setting another host');
	DEALLOCATE retnames;
	DEALLOCATE expnames;

	PERFORM startchangeset();
	PERFORM interface_set_host('hosthp2->eth4', 'hosthp1');
	PERFORM interface_set_name('hosthp1->eth4', 'hosthp1->eth0');
	PERFORM commitchangeset('');

	PREPARE retnames AS SELECT interface_names();
	PREPARE expnames AS SELECT name FROM test_interface WHERE version = 3;
	RETURN NEXT set_has('retnames', 'expnames', 'returns good names after renamening interface and setting another host');
	DEALLOCATE retnames;
	DEALLOCATE expnames;

	DELETE FROM test_interface;
END
$$
LANGUAGE plpgsql;

SELECT runtests();

ROLLBACK;
