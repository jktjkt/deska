\i util-include-path.sql

BEGIN;

CREATE TABLE test_interface(
	name text,
	step bigint,
	dest_bit bit(1)
);

CREATE FUNCTION test_undel_embed_one_changeset()
RETURNS SETOF TEXT
AS
$$
BEGIN
	DELETE FROM test_interface;
	INSERT INTO test_interface VALUES ('hosthwHP->eth0', 1, '1');
	INSERT INTO test_interface VALUES ('hosthwHP->eth0', 2, '0');

	PERFORM startChangeset();
	PERFORM vendor_add('HP');
	PERFORM hardware_add('hwHP');
	PERFORM hardware_set_vendor('hwHP','HP');
	PERFORM hardware_set_purchase('hwHP','2000-4-8');
	PERFORM hardware_set_warranty('hwHP','2014-7-25');
	PERFORM host_add('hosthwHP');
	PERFORM interface_add('hosthwHP->eth0');
	PERFORM interface_del('hosthwHP->eth0');

	PREPARE retnames AS SELECT interface_names();
	PREPARE deleted_names AS SELECT name FROM test_interface WHERE dest_bit = '1' AND step = 1;
	RETURN NEXT set_hasnt( 'retnames', 'deleted_names', 'deleted interface IS NOT present' );

	DEALLOCATE deleted_names;
	DEALLOCATE retnames;

	PERFORM interface_undel('hosthwHP->eth0');
	PREPARE retnames AS SELECT interface_names();
	PREPARE undeleted_names AS SELECT name FROM test_interface WHERE dest_bit = '0' AND step = 0;
	RETURN NEXT set_has( 'retnames', 'undeleted_names', 'undeleted interface IS present' );
	PERFORM commitChangeset('1');

	DEALLOCATE retnames;

	PREPARE retnames AS SELECT interface_names();
	RETURN NEXT set_has( 'retnames', 'undeleted_names', 'undeleted interface IS present after commit' );

	DEALLOCATE retnames;
	DEALLOCATE undeleted_names;
END
$$
LANGUAGE plpgsql;

--undel is called in another changeset then add
CREATE FUNCTION test_undel_embed_in_another_changeset()
RETURNS SETOF TEXT
AS
$$
BEGIN
	DELETE FROM test_interface;
	INSERT INTO test_interface VALUES ('hosthwHP2->eth0', 1, '1');
	INSERT INTO test_interface VALUES ('hosthwHP2->eth0', 2, '0');

	PERFORM startChangeset();
	PERFORM vendor_add('HP2');
	PERFORM hardware_add('hwHP2');
	PERFORM hardware_set_vendor('hwHP2','HP2');
	PERFORM hardware_set_purchase('hwHP2','2000-4-8');
	PERFORM hardware_set_warranty('hwHP2','2014-7-25');
	PERFORM host_add('hosthwHP2');
	PERFORM interface_add('hosthwHP2->eth0');
	PERFORM commitChangeset('1');

	PERFORM startChangeset();
	PERFORM interface_del('hosthwHP2->eth0');
	PREPARE retnames AS SELECT interface_names();
	PREPARE deleted_names AS SELECT name FROM test_interface WHERE dest_bit = '1' AND step = 1;
	RETURN NEXT set_hasnt( 'retnames', 'deleted_names', 'deleted interface IS NOT present' );

	DEALLOCATE deleted_names;
	DEALLOCATE retnames;

	PERFORM interface_undel('hosthwHP2->eth0');
	PREPARE retnames AS SELECT interface_names();
	PREPARE undeleted_names AS SELECT name FROM test_interface WHERE dest_bit = '0' AND step = 0;
	RETURN NEXT set_has( 'retnames', 'undeleted_names', 'undeleted interface IS present' );
	DEALLOCATE retnames;
	PERFORM commitChangeset('1');

	PREPARE retnames AS SELECT interface_names();
	RETURN NEXT set_has( 'retnames', 'undeleted_names', 'undeleted interface IS present after commit' );

	DEALLOCATE retnames;
	DEALLOCATE undeleted_names;

END
$$
LANGUAGE plpgsql;

--hardware part
CREATE TABLE test_hardware(
	name text,
	step bigint,
	dest_bit bit(1)
);

CREATE FUNCTION test_undel_one_changeset()
RETURNS SETOF TEXT
AS
$$
BEGIN
	DELETE FROM test_interface;
	INSERT INTO test_interface VALUES ('hw', 1, '1');
	INSERT INTO test_interface VALUES ('hw', 2, '0');

	PERFORM startChangeset();
	PERFORM vendor_add('vendor');
	PERFORM hardware_add('hw');
	PERFORM hardware_set_vendor('hw','vendor');
	PERFORM hardware_set_purchase('hw','2000-4-8');
	PERFORM hardware_set_warranty('hw','2014-7-25');

	PREPARE retnames AS SELECT hardware_names();
	PREPARE deleted_names AS SELECT name FROM test_hardware WHERE dest_bit = '1' AND step = 1;
	RETURN NEXT set_hasnt( 'retnames', 'deleted_names', 'deleted hardware IS NOT present' );

	DEALLOCATE deleted_names;
	DEALLOCATE retnames;

	PERFORM hardware_undel('hw');
	PREPARE retnames AS SELECT hardware_names();
	PREPARE undeleted_names AS SELECT name FROM test_hardware WHERE dest_bit = '0' AND step = 0;
	RETURN NEXT set_has( 'retnames', 'undeleted_names', 'undeleted hardware IS present' );
	PERFORM commitChangeset('1');

	DEALLOCATE retnames;

	PREPARE retnames AS SELECT hardware_names();
	RETURN NEXT set_has( 'retnames', 'undeleted_names', 'undeleted interface IS present after commit' );

	DEALLOCATE retnames;
	DEALLOCATE undeleted_names;
END
$$
LANGUAGE plpgsql;

--undel is called in another changeset then add
CREATE FUNCTION test_undel_in_another_changeset()
RETURNS SETOF TEXT
AS
$$
BEGIN
	DELETE FROM test_interface;
	INSERT INTO test_interface VALUES ('hw2', 1, '1');
	INSERT INTO test_interface VALUES ('hw2', 2, '0');

	PERFORM startChangeset();
	PERFORM vendor_add('vendor2');
	PERFORM hardware_add('hw2');
	PERFORM hardware_set_vendor('hw2','vendor2');
	PERFORM hardware_set_purchase('hw2','2000-4-8');
	PERFORM hardware_set_warranty('hw2','2014-7-25');
	PERFORM commitChangeset('1');

	PERFORM startChangeset();
	PERFORM hardware_del('hw2');
	PREPARE retnames AS SELECT hardware_names();
	PREPARE deleted_names AS SELECT name FROM test_hardware WHERE dest_bit = '1' AND step = 1;
	RETURN NEXT set_hasnt( 'retnames', 'deleted_names', 'deleted interface IS NOT present' );

	DEALLOCATE deleted_names;
	DEALLOCATE retnames;

	PERFORM hardware_undel('hw2');
	PREPARE retnames AS SELECT hardware_names();
	PREPARE undeleted_names AS SELECT name FROM test_hardware WHERE dest_bit = '0' AND step = 0;
	RETURN NEXT set_has( 'retnames', 'undeleted_names', 'undeleted interface IS present' );
	DEALLOCATE retnames;
	PERFORM commitChangeset('1');

	PREPARE retnames AS SELECT hardware_names();
	RETURN NEXT set_has( 'retnames', 'undeleted_names', 'undeleted interface IS present after commit' );

	DEALLOCATE retnames;
	DEALLOCATE undeleted_names;

END
$$
LANGUAGE plpgsql;

CREATE FUNCTION test_del_add_in_one_changeset()
RETURNS SETOF TEXT
AS
$$
BEGIN
	PERFORM startChangeset();
	PERFORM vendor_add('vendor2');
	PERFORM hardware_add('hw2');
	PERFORM hardware_set_vendor('hw2','vendor2');
	PERFORM hardware_set_purchase('hw2','2000-4-8');
	PERFORM hardware_set_warranty('hw2','2014-7-25');
	PERFORM hardware_del('hw2');
	PREPARE add_fc AS SELECT hardware_add('hw2');
	RETURN NEXT throws_ok('add_fc');
	PERFORM commitChangeset('1');

END
$$
LANGUAGE plpgsql;


SELECT * FROM runtests();

ROLLBACK; 
 
