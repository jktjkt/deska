BEGIN;

\i util-switch-user.sql

SET search_path TO pgtap, api, genproc, deska, production;


/* ************************************************
*	 testing kindNames()		***
************************************************ */

CREATE TABLE production.testkindtab1(
col1 int,
col2 name
);

CREATE TABLE production.testkindtab2(
col1 int
);

CREATE TABLE pgtap.results_kindName(
kindName name
);

INSERT INTO results_kindName VALUES ('testkindtab1');
INSERT INTO results_kindName VALUES ('testkindtab2');

CREATE OR REPLACE FUNCTION test_get_kindNames1()
RETURNS SETOF TEXT AS
$$
BEGIN
	PREPARE expkinds  AS SELECT kindName FROM pgtap.results_kindName;
	PREPARE retkinds AS SELECT * FROM api.kindNames();
	RETURN NEXT set_has( 'retkinds', 'expkinds', 'created kinds are present' );	
END;
$$
LANGUAGE plpgsql; 



/* ************************************************
*	 testing get_dependency_info()		***
************************************************ */

CREATE TABLE production.reftesttab(
uid int PRIMARY KEY, 
label name
);

CREATE TABLE production.btesttab(
label name,
uid int PRIMARY KEY,
refuid int REFERENCES reftesttab(uid)
);

CREATE TABLE pgtap.results_table_info(
relname name,
attnames text,
refrelname name,
refattnames text
);

INSERT INTO pgtap.results_table_info VALUES ('btesttab','refuid','reftesttab','uid');


CREATE OR REPLACE FUNCTION pgtap.test_get_dependency_info()
RETURNS SETOF TEXT AS
$$
DECLARE
  rel pgtap.results_table_info%ROWTYPE;
  exp results_table_info%ROWTYPE;
BEGIN
	FOR rel IN SELECT relname, attnames, refrelname, refattnames FROM pgtap.results_table_info LOOP	  				
		EXECUTE 'SELECT relname, attnames, refrelname, refattnames FROM deska.get_dependency_info() WHERE relname = $1 AND attnames LIKE $2 AND refrelname = $3'
		INTO exp
		USING rel.relname, rel.attnames, rel.refrelname;
		RETURN NEXT matches(rel.refattnames,exp.refattnames,'foreign key is ok');
	END LOOP;
END;
$$
LANGUAGE plpgsql;



/* ************************************************
*		 testing kindAttributes							***
************************************************ */

CREATE TABLE production.testkindatttab(
	intcol int,
	namecol name
);

CREATE TABLE pgtap.results_kindAttributes(
	attname name,
	typename name
);

INSERT INTO pgtap.results_kindAttributes VALUES ('intcol','int4');
INSERT INTO pgtap.results_kindAttributes VALUES ('namecol','name');

CREATE OR REPLACE FUNCTION pgtap.test_kindattributes()
RETURNS SETOF TEXT AS
$$
DECLARE
  rel pgtap.results_kindAttributes%ROWTYPE;
  exptype name;
BEGIN
	PREPARE expatts  AS SELECT attname FROM pgtap.results_kindAttributes;
	PREPARE retatts AS SELECT attname FROM api.kindattributes('testkindatttab');
	RETURN NEXT set_eq( 'retatts', 'expatts', 'all attributes are present' );	

	FOR rel IN SELECT attname, typename FROM results_kindAttributes LOOP	  				
		EXECUTE 'SELECT typename FROM api.kindattributes(''testkindatttab'') WHERE attname = $1'
		INTO exptype
		USING rel.attname;
		RETURN NEXT matches(rel.typename,exptype,'attribute type is ok');
	END LOOP;
END;
$$
LANGUAGE plpgsql;


/* ************************************************
*			 testing get_table_info				***
************************************************ */

CREATE TABLE production.tabinfo1(
	intcol int,
	namecol name
);

CREATE TABLE production.tabinfo2(
	textcol text,
	boolcol boolean,
	namecol name
);


CREATE TABLE pgtap.results_get_table_info(
	relname name,
	attname name,
	typename name
);

INSERT INTO pgtap.results_get_table_info VALUES ('tabinfo1','intcol','int4');
INSERT INTO pgtap.results_get_table_info VALUES ('tabinfo1','namecol','name');
INSERT INTO pgtap.results_get_table_info VALUES ('tabinfo2','textcol','text');
INSERT INTO pgtap.results_get_table_info VALUES ('tabinfo2','boolcol','bool');
INSERT INTO pgtap.results_get_table_info VALUES ('tabinfo2','namecol','name');

CREATE OR REPLACE FUNCTION pgtap.test_get_table_info()
RETURNS SETOF TEXT AS
$$
DECLARE
	attname name;
	rettype name;
	exptype name;
	tabname name;
BEGIN
	PREPARE expkindsti  AS SELECT relname FROM pgtap.results_get_table_info;
	PREPARE retkindsti AS SELECT relname FROM deska.get_table_info();
	RETURN NEXT set_has( 'retkindsti', 'expkindsti', 'all kinds/tables are present' );	

	FOR tabname IN SELECT DISTINCT relname FROM results_get_table_info LOOP
		EXECUTE 'CREATE TEMP TABLE tempretatts AS 
					SELECT attname FROM deska.get_table_info() WHERE relname = $1' USING tabname;

		EXECUTE 'CREATE TEMP TABLE tempexpatts AS
					SELECT attname FROM pgtap.results_get_table_info WHERE relname = $1' USING tabname;

		PREPARE expattsti AS SELECT attname FROM tempexpatts;
		PREPARE retattsti AS SELECT attname FROM tempretatts;
		RETURN NEXT set_eq( 'retattsti', 'expattsti', 'all attributes in this kind/table are present' );

		DROP TABLE tempexpatts;
		DROP TABLE tempretatts;

		DEALLOCATE expattsti;
		DEALLOCATE retattsti;
		
		FOR attname, exptype IN EXECUTE 'SELECT attname, typename FROM results_get_table_info WHERE relname = $1' USING tabname LOOP	  				
			EXECUTE 'SELECT typname FROM deska.get_table_info() WHERE relname = $1 AND attname = $2'
			INTO rettype
			USING tabname, attname;

			RETURN NEXT matches(rettype, exptype, 'attribute type is ok');
		END LOOP;

	END LOOP;

END;
$$
LANGUAGE plpgsql;

 SELECT * FROM runtests();

ROLLBACK; 