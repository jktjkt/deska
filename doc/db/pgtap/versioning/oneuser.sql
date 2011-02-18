BEGIN;

SET search_path TO pgtap,genproc,history,deska,production;

--action is indicator of operation done with object
--I object was inserted
--D object was deleted
CREATE TABLE pgtap.vendor_test1(
	name text,
	action char(1)
);

	--it is expected that vendor with name DELL was Inserted	
	INSERT INTO pgtap.vendor_test1 (name, action) VALUES ('DELL','I');


CREATE TABLE pgtap.vendor_test2(
	name text,
	action char(1)
);


	--it is expected that vendor with name DELL was Deleted                                                              
	INSERT INTO pgtap.vendor_test2 (name, action) VALUES ('DELL','D');
	--it is expected that vendor with name HP was Inserted
	INSERT INTO pgtap.vendor_test2 (name, action) VALUES ('HP','I');
	INSERT INTO pgtap.vendor_test2 (name, action) VALUES ('IBM','I');

CREATE OR REPLACE FUNCTION pgtap.test_vendor_versioning1()
RETURNS SETOF TEXT AS
$$
DECLARE
BEGIN
	
	PERFORM start_changeset();

	-- some changes follows
	PERFORM vendor_add('DELL');

	-- commit
	PERFORM vendor_commit();

	-- end session, part of vendor commit? of vendor commit part of this?
	PERFORM version_commit();
	
	PREPARE expnames  AS SELECT name FROM pgtap.vendor_test1 WHERE action ='I';
	PREPARE retnames AS SELECT name FROM production.vendor;
	RETURN NEXT set_has( 'retnames', 'expnames', 'all names are present' );

	DEALLOCATE expnames;
	DEALLOCATE retnames;

	PERFORM start_changeset();
	PERFORM vendor_del('DELL');
	PERFORM vendor_add('HP');
	PERFORM vendor_add('IBM');
	PERFORM vendor_commit();
	PERFORM version_commit();



	PREPARE expnames  AS SELECT name FROM pgtap.vendor_test2 WHERE action ='I';
	PREPARE exp_not_in_names AS SELECT name FROM pgtap.vendor_test2 WHERE action ='D';
	PREPARE retnames AS SELECT name FROM production.vendor;
	RETURN NEXT set_has( 'retnames', 'expnames', 'inserted objects are present' );
	RETURN NEXT set_hasnt( 'retnames', 'exp_not_in_names', 'deleted objects are NOT present' );

	DEALLOCATE expnames;
	DEALLOCATE retnames;
	DEALLOCATE exp_not_in_names;
END;
$$
LANGUAGE plpgsql;
 

SELECT * FROM runtests();

ROLLBACK;