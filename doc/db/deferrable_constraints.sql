BEGIN;

SET search_path TO deska;

--creating of objetc
--tables that references to another table and tables that are referenced
--tuples are created in to different schemas

create table test(
	coli int primary key,
	colref  int
);

create table reftest(
	coli int primary key
);

create table public.test1(
	coli int primary key,
	colref  int
);

create table public.reftest1(
	coli int primary key
);
--deferrable constraints
alter table test add constraint test_fk_reftest foreign key (colref) references reftest(coli) DEFERRABLE;
alter table public.test1 add constraint public_test1_fk_reftest1 foreign key (colref) references public.reftest1(coli) DEFERRABLE;

--inserting into referencing tables
CREATE OR REPLACE FUNCTION test_insert()
RETURNS void
AS
$$
BEGIN
	INSERT INTO test(coli,colref) VALUES (1,1);
	INSERT INTO test(coli,colref) VALUES (2,1);
	INSERT INTO test(coli,colref) VALUES (3,2);
	INSERT INTO test(coli,colref) VALUES (4,3);
	INSERT INTO public.test1(coli,colref) VALUES (1,1);
END;
$$
LANGUAGE plpgsql;

--inserting into referenced tables
CREATE OR REPLACE FUNCTION testref_insert()
RETURNS void
AS
$$
BEGIN
	INSERT INTO reftest(coli) VALUES (1);
	INSERT INTO reftest(coli) VALUES (2);
	INSERT INTO reftest(coli) VALUES (3);
	INSERT INTO public.reftest1(coli) VALUES (1);	
END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION deferred_constr_test_raise()
RETURNS void
AS
$$
DECLARE
	path text;
BEGIN	
	SET search_path TO deska;
	SET CONSTRAINTS ALL DEFERRED;
	--at first we insert into referencing tables to raise foreign key violation in case of IMMEDIATE constraints
	PERFORM test_insert();
	PERFORM testref_insert();
	SET CONSTRAINTS ALL IMMEDIATE;
END;
$$
LANGUAGE plpgsql;

SELECT deferred_constr_test_raise();

--test of bases for commit
CREATE OR REPLACE FUNCTION deferred_test()
RETURNS text
AS
$$
DECLARE
	path text;
BEGIN	
	EXECUTE 'SHOW search_path' INTO path;
	SET search_path TO deska;
	SET CONSTRAINTS ALL DEFERRED;
	PERFORM test_insert();
	PERFORM testref_insert();
	EXECUTE 'SET search_path TO ' || path;
	RETURN path;
END;
$$
LANGUAGE plpgsql;

ROLLBACK;