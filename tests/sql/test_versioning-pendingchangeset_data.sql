BEGIN;

\i util-include-path.sql

CREATE TABLE public.vendor_test_tmp1(
	name text,
	action char(1)
);

CREATE TABLE public.vendor_test_tmp2(
	name text,
	action char(1)
);

--it is expected that vendor with name DELL was Inserted in changeset tmp1
INSERT INTO public.vendor_test_tmp1 (name, action) VALUES ('DELL','I');
--it is expected that vendor with name HP was Inserted
INSERT INTO public.vendor_test_tmp2 (name, action) VALUES ('HP','I');
INSERT INTO public.vendor_test_tmp2 (name, action) VALUES ('IBM','I');

CREATE OR REPLACE FUNCTION pgtap.test_pending_changeset_data()
RETURNS SETOF TEXT AS
$$
DECLARE
	 first_changeset text;
BEGIN
	PERFORM startchangeset();
	first_changeset = id2changeset(my_version());
	PERFORM vendor_add('DELL');	
	PERFORM detachfromcurrentchangeset('vendor DELL added');
	

	PREPARE retnames AS SELECT vendor_names();
	PREPARE u1names AS SELECT name FROM public.vendor_test_tmp2 WHERE action='I';
	RETURN NEXT set_hasnt( 'retnames', 'u1names', 'uncommited data are NOT in production' );

	DEALLOCATE retnames;
	DEALLOCATE u1names;

	PERFORM startchangeset();

	PREPARE retnames AS SELECT vendor_names();
	PREPARE u1names AS SELECT name FROM public.vendor_test_tmp2 WHERE action='I';
	RETURN NEXT set_hasnt( 'retnames', 'u1names', 'uncommited data are NOT visible from another changeset' );

	DEALLOCATE retnames;
	DEALLOCATE u1names;

	
	BEGIN
		PERFORM vendor_del('DELL');
		RETURN NEXT fail('deleting nonexisting data - exception was expected');
		EXCEPTION
			WHEN OTHERS THEN           
				RETURN NEXT pass( 'expected exception was raised' );
	END;
	
	PERFORM vendor_add('HP');
	PERFORM vendor_add('IBM');
	PERFORM commitchangeset('add HP, IBM');

	
	PREPARE expnames  AS SELECT name FROM public.vendor_test_tmp2 WHERE action ='I';
	PREPARE exp_not_in_names AS SELECT name FROM public.vendor_test_tmp2 WHERE action ='D';
	PREPARE retnames AS SELECT vendor_names();
	RETURN NEXT set_has( 'retnames', 'expnames', 'inserted objects in tmp2 are present' );
	RETURN NEXT set_hasnt( 'retnames', 'exp_not_in_names', 'deleted objects in tmp2 are NOT present' );

	DEALLOCATE expnames;
	DEALLOCATE retnames;
	DEALLOCATE exp_not_in_names;

	--resume tmp1
	PERFORM resumechangeset(first_changeset);
	-- tmp1 version_commit
	PERFORM commitchangeset('add DELL');

	PREPARE retnames AS SELECT vendor_names();
	PREPARE expnames  AS 
		  SELECT name FROM public.vendor_test_tmp1 WHERE action ='I' 
		  UNION 
		  SELECT name FROM public.vendor_test_tmp2 WHERE action ='I';
	PREPARE exp_not_in_names AS
		  SELECT name FROM public.vendor_test_tmp1 WHERE action ='D' 
		  UNION 
		  SELECT name FROM public.vendor_test_tmp2 WHERE action ='D';
-- 	PREPARE retnames AS SELECT vendor_names();
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
