CREATE SCHEMA test;
SET search_path TO test, public, deska_dev;

/* ************************************************
*	 testing get_kindNames()		***
************************************************ */

CREATE TABLE deska_dev.testkindtab1(
	col1 INT,
	col2 NAME
);

CREATE TABLE deska_dev.testkindtab2(
	col1 INT
);


CREATE OR REPLACE FUNCTION test_get_kindNames1()
RETURNS BOOLEAN AS
$$
DECLARE
	r NAME;
	containstab1 BOOLEAN;
	containstab2 BOOLEAN;
	count INT;
BEGIN
	count := 0;
	containstab1 := false;
	containstab2 := false;
	FOR r IN SELECT get_kindNames() LOOP		
		CASE  
		WHEN r LIKE 'testkindtab1' THEN 
			containstab1 := true;
		WHEN r LIKE 'testkindtab2' THEN 
			containstab2 := true;
		ELSE
		END CASE;
		count := count + 1;		
	END LOOP;
	return containstab1 AND containstab2 AND (count >= 2);
END;
$$
LANGUAGE plpgsql;

select test_get_kindNames1();

DROP FUNCTION test_get_kindNames1();
DROP TABLE deska_dev.testkindtab1;
DROP TABLE deska_dev.testkindtab2;


/* ************************************************
*	 testing get_KindAttributes		***
************************************************ */

CREATE TABLE deska_dev.testTabAtt(nameAtt name, intAtt int);

CREATE TYPE KindAttributeDataType AS (attname name, typename name);

--tests basic funcionality, two attributes inside and theirs types
CREATE OR REPLACE FUNCTION test.test_get_KindAttributes1()
RETURNS name AS
$$
declare
containsName BOOLEAN;
containsInt BOOLEAN;
r KindAttributeDataType%ROWTYPE;
count INT;
begin
	count := 0;
	containsName := false;
	containsInt := false;

	FOR r IN  select attname, typename from get_kindAttributes('testtabatt') LOOP		
		CASE  
		WHEN r.attName LIKE 'nameatt' THEN 
			IF r.typename LIKE 'name' THEN
				containsName := true;
			ELSE
				return false;
			END IF;
		WHEN r.attName LIKE 'intatt' THEN 
			IF r.typename LIKE 'int4' THEN
				containsInt := true;
			ELSE
				return false;
			END IF;
		ELSE
			return false;
		END CASE;
		count := count + 1;		
	END LOOP;	
	
	return containsName AND containsInt AND (count = 2);
end;
$$
LANGUAGE plpgsql;

--not existing table
CREATE OR REPLACE FUNCTION test.test_get_KindAttributes2()
RETURNS name AS
$$
declare
r KindAttributeDataType%ROWTYPE;
count INT;
begin
	count := 0;
	FOR r IN  select attname, typename from get_kindAttributes('doesnotexist') LOOP		
		count := count + 1;		
	END LOOP;	
	
	return (count = 0);
end;
$$
LANGUAGE plpgsql;

select test.test_get_KindAttributes1();
select test.test_get_KindAttributes2();

drop function test.test_get_kindAttributes1();
drop function test.test_get_kindAttributes2();
drop type test.KindAttributeDataType;
drop table deska_dev.testTabAtt;



/* ************************************************
*	 testing get_primary_key_column		***
************************************************ */


CREATE TABLE deska_dev.testTabPK(
	col1 INT,
	pkcol INT PRIMARY KEY,
	col2 text
);

CREATE OR REPLACE FUNCTION test_get_primary_key_column1()
RETURNS BOOLEAN AS
$$
DECLARE
 tabname name;
BEGIN
	SELECT * INTO  tabname FROM get_primary_key_column('testtabpk');
	RETURN   tabname = 'pkcol';
END;
$$
LANGUAGE plpgsql;

select test_get_primary_key_column1();


DROP FUNCTION test_get_primary_key_column1();
DROP TABLE deska_dev.testTabPK;