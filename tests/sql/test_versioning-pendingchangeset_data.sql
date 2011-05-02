BEGIN;

SET search_path TO pgtap, api, genproc, deska, history, production;

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
	first_changeset = id2changeset(get_current_changeset());
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
	 --HERE SHOULD BE REBASE
--	PERFORM commitchangeset('add DELL');
/*
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
	DEALLOCATE exp_not_in_names;*/

END;
$$
LANGUAGE plpgsql; 


SELECT * FROM runtests();

ROLLBACK;