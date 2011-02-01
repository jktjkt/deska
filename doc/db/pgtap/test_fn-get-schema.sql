BEGIN;

set search_path to pgtap, public, deska_dev;

/* ************************************************
*	 testing get_kindNames()		***
************************************************ */

CREATE TABLE deska_dev.testkindtab1(
	col1 int,
	col2 name
);

CREATE TABLE deska_dev.testkindtab2(
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
	PREPARE retkinds AS SELECT * FROM deska_dev.get_kindNames();
	RETURN NEXT set_has( 'retkinds', 'expkinds', 'created kinds are present' );	
END;
$$
LANGUAGE plpgsql; 


/* ************************************************
*	 testing get_dependency_info()		***
************************************************ */

CREATE TABLE reftesttab(
	uid int PRIMARY KEY, 
	label name
);

CREATE TABLE btesttab(
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
	FOR rel IN SELECT relname, attnames, refrelname, refattnames FROM results_table_info LOOP	  				
		EXECUTE 'SELECT relname, attnames, refrelname, refattnames FROM deska_dev.get_dependency_info() WHERE relname = $1 AND attnames LIKE $2 AND refrelname = $3'
		INTO exp
		USING rel.relname, rel.attnames, rel.refrelname;
		RETURN NEXT matches(rel.refattnames,exp.refattnames,'foreign key is ok');
	END LOOP;
END;
$$
LANGUAGE plpgsql;


/* ************************************************
*		 testing get_kind_attributes			***
************************************************ */

CREATE TABLE deska_dev.testkindatttab(
	intcol int,
	namecol name
);

CREATE TABLE pgtap.results_kindAttributes(
	attname name,
	typename name
);

INSERT INTO pgtap.results_kindAttributes VALUES ('intcol','int4');
INSERT INTO pgtap.results_kindAttributes VALUES ('namecol','name');

CREATE OR REPLACE FUNCTION pgtap.test_get_kind_attributes()
RETURNS SETOF TEXT AS
$$
DECLARE
  rel pgtap.results_kindAttributes%ROWTYPE;
  exptype name;
BEGIN
	PREPARE expatts  AS SELECT attname FROM pgtap.results_kindAttributes;
	PREPARE retatts AS SELECT attname FROM deska_dev.get_kind_attributes('testkindatttab');
	RETURN NEXT set_eq( 'retatts', 'expatts', 'all attributes are present' );	

	FOR rel IN SELECT attname, typename FROM results_kindAttributes LOOP	  				
		EXECUTE 'SELECT typename FROM deska_dev.get_kind_attributes(''testkindatttab'') WHERE attname = $1'
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
	PREPARE retkindsti AS SELECT relname FROM deska_dev.get_table_info();
	RETURN NEXT set_has( 'retkindsti', 'expkindsti', 'all kinds/tables are present' );	

	FOR tabname IN SELECT DISTINCT relname FROM results_get_table_info LOOP
		EXECUTE 'CREATE TEMP TABLE tempretatts AS 
					SELECT attname FROM deska_dev.get_table_info() WHERE relname = $1' USING tabname;

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
			EXECUTE 'SELECT typname FROM deska_dev.get_table_info() WHERE relname = $1 AND attname = $2'
			INTO rettype
			USING tabname, attname;

			RETURN NEXT matches(rettype, exptype, 'attribute type is ok');
		END LOOP;

	END LOOP;

END;
$$
LANGUAGE plpgsql;



/* ************************************************
*			 testing get_relations				***
************************************************ */
CREATE TABLE pgtap.results_get_relations(
relname name,
relkind text,
attname	text,
refkind	name,
refattname	text
);


INSERT INTO pgtap.results_get_relations VALUES ('mergedkind','MERGE','mergecol','kind2','model');
INSERT INTO pgtap.results_get_relations VALUES ('templatedkind','TEMPLATE','templatecol','kind2','model');
INSERT INTO pgtap.results_get_relations VALUES ('embedkind','EMBED','embedcol','kind2','model');
INSERT INTO pgtap.results_get_relations VALUES ('badkind','INVALID','col','kind2','model');


--kind2 is referenced table
CREATE TABLE deska_dev.kind2(
model name primary key
);

CREATE TABLE deska_dev.mergedkind(
kindname name, 
mergecol name,
CONSTRAINT rmerge_mergedkind_kind2 FOREIGN KEY (mergecol) REFERENCES deska_dev.kind2(model)
);

CREATE TABLE deska_dev.templatedkind(
kindname name, 
templatecol name,
CONSTRAINT rtempl_templatedkind_kind2 FOREIGN KEY (templatecol) REFERENCES deska_dev.kind2(model)
);

CREATE TABLE deska_dev.embedkind(
kindname name, 
embedcol name,
CONSTRAINT rembed_embedkind_kind2 FOREIGN KEY (embedcol) REFERENCES deska_dev.kind2(model)
);

--table with invalid relation = reference
CREATE TABLE deska_dev.badkind(
kindname name, 
col name,
CONSTRAINT rbad_kind1_kind2 FOREIGN KEY (col) REFERENCES deska_dev.kind2(model)
);

CREATE OR REPLACE FUNCTION pgtap.test_get_relations_merge()
RETURNS SETOF TEXT AS
$$
DECLARE
	exprelation pgtap.results_get_relations%ROWTYPE;
	retrelation deska_dev.kind_relation;
BEGIN
	FOR exprelation IN SELECT relname,relkind,attname,refkind,refattname FROM pgtap.results_get_relations LOOP
		EXECUTE 'SELECT relkind, attname, refkind, refattname FROM deska_dev.get_relations($1) WHERE attname LIKE $2'
		INTO retrelation
		USING exprelation.relname, exprelation.attname;				
		
		RETURN NEXT matches( retrelation.relkind, exprelation.relkind, 'relation type is OK' );
		RETURN NEXT matches( retrelation.refkind, exprelation.refkind, 'referenced kind is OK' );
		RETURN NEXT matches( retrelation.refattname, exprelation.refattname, 'referenced columns are OK' );
	END LOOP;
END;
$$
LANGUAGE plpgsql;


DELETE FROM pgtap.results_get_relations;

INSERT INTO pgtap.results_get_relations VALUES ('kind1','MERGE','related','kind2','model');
INSERT INTO pgtap.results_get_relations VALUES ('kind1','TEMPLATE','template','kind1','kindname');


CREATE TABLE deska_dev.kind1(
kindname name UNIQUE, 
related name,
template name,
CONSTRAINT rmerge_kind1_kind2 FOREIGN KEY (related) REFERENCES deska_dev.kind2(model),
CONSTRAINT rtempl_kind1_kind2 FOREIGN KEY (template) REFERENCES deska_dev.kind1(kindname)
);

CREATE OR REPLACE FUNCTION pgtap.test_get_relations2()
RETURNS SETOF TEXT AS
$$
DECLARE
	exprelation pgtap.results_get_relations%ROWTYPE;
	retrelation deska_dev.kind_relation;
BEGIN
    FOR exprelation IN SELECT relname,relkind,attname,refkind,refattname FROM pgtap.results_get_relations LOOP
		EXECUTE 'SELECT relkind, attname, refkind, refattname FROM deska_dev.get_relations($1) WHERE attname LIKE $2'
		INTO retrelation
		USING exprelation.relname, exprelation.attname;				
		
		RETURN NEXT matches( retrelation.relkind, exprelation.relkind, 'relation type is OK' );
		RETURN NEXT matches( retrelation.refkind, exprelation.refkind, 'referenced kind is OK' );
		RETURN NEXT matches( retrelation.refattname, exprelation.refattname, 'referenced columns are OK' );
	END LOOP;

END;
$$
LANGUAGE plpgsql;

SELECT * FROM runtests();

ROLLBACK;