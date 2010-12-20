-- switch to deska_dev SCHEMA
SET search_path TO deska_dev,public;

DROP TABLE version;

-- vendors of hw -- versioning table
CREATE TABLE version (
	-- internal id
	id int
		CONSTRAINT version_pk PRIMARY KEY,
	-- human readable id
	number int
		CONSTRAINT version_number_unique UNIQUE,
	name text
);
GRANT ALL ON version TO deska_team;

--
-- functions
--

-- commit
CREATE or REPLACE FUNCTION 
commit(IN name text)
RETURNS int
as
$$
	DECLARE ver int;
BEGIN	
	ver = 1;
	RETURN 1;
END
$$
language plpgsql;

-- create record
CREATE or REPLACE FUNCTION 
create_record(IN tbl text, IN attr text, IN value text, IN ver int)
RETURNS int
AS
$$
--	DECLARE ver int;
BEGIN	
	plan = plpy.prepare("INSERT INTO $1 ($2,version) VALUES ($3,$4)", [ "text", "text", "text", "int" ];
	rv = plpy.execute(plan, tbl, attr, value, ver);
	return rv;
END
$$
language plpythonu;

