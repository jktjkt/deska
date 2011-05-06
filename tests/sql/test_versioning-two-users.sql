BEGIN;

\set ECHO
\set QUIET 1

\i util-include-path.sql

CREATE USER u1;
GRANT deska_user TO u1;

CREATE USER u2;
GRANT deska_user TO u2;


--action is indicator of operation done with object
--I object was inserted
--D object was deleted
CREATE TABLE public.vendor_test(
	name text,
	action char(1),
	username char(2)
);

CREATE OR REPLACE FUNCTION pgtap.test_vendor_versioning1()
RETURNS SETOF TEXT AS
$$
DECLARE
BEGIN
	SET ROLE TO u1;

	--u1 start changeset
	PERFORM startchangeset();


	-- u1 add vendor DELL
	PERFORM vendor_add('DELL');

--it is expected that vendor with name DELL was Inserted by user u1
	INSERT INTO public.vendor_test (name, action, username) VALUES ('DELL','I','u1');

	SET ROLE TO u2;

	PREPARE retnames AS SELECT vendor_names();
	PREPARE u1names AS SELECT name FROM public.vendor_test WHERE action='I' AND username='u1';
	RETURN NEXT set_hasnt( 'retnames', 'u1names', 'uncommited data are NOT in production' );

	DEALLOCATE retnames;
	DEALLOCATE u1names;


	PERFORM startchangeset();
	
	BEGIN
		PERFORM vendor_del('DELL');
		RETURN NEXT fail('deleting nonexisting data - exception was expected');
		EXCEPTION
			WHEN OTHERS THEN           
				RETURN NEXT pass( 'expected exception was raised' );
	END;
	
	PERFORM vendor_add('HP');
	PERFORM vendor_add('IBM');
	PERFORM vendor_commit();
	PERFORM version_commit();

	--it is expected that vendor with name HP was Inserted
	INSERT INTO public.vendor_test (name, action, username) VALUES ('HP','I','u2');
	INSERT INTO public.vendor_test (name, action, username) VALUES ('IBM','I','u2');

	PREPARE expnames  AS SELECT name FROM public.vendor_test WHERE action ='I' AND username='u2';
	PREPARE exp_not_in_names AS SELECT name FROM public.vendor_test WHERE action ='D' AND username='u2';
	PREPARE retnames AS SELECT vendor_names();
	RETURN NEXT set_has( 'retnames', 'expnames', 'inserted objects by u2 are present' );
	RETURN NEXT set_hasnt( 'retnames', 'exp_not_in_names', 'deleted objects by u2 are NOT present' );

	DEALLOCATE expnames;
	DEALLOCATE retnames;
	DEALLOCATE exp_not_in_names;


	SET ROLE TO u1;

	-- u1 vendor commit
	PERFORM vendor_commit();

	-- u1 version_commit
	PERFORM version_commit();

	PREPARE expnames  AS SELECT name FROM public.vendor_test WHERE action ='I';
	PREPARE exp_not_in_names AS SELECT name FROM public.vendor_test WHERE action ='D';
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
