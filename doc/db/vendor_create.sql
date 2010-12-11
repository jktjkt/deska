-- switch to deska_dev SCHEMA
SET search_path TO deska_dev,public;

-- vendors of hw
CREATE TABLE vendor (
	uid char(36)
		constraint vendor_pk PRIMARY KEY,
	name text NOT NULL
);
GRANT ALL ON vendor TO deska_team;

-- vendors of hw -- versioning table
CREATE TABLE vendor_version (
	uid char(36) NOT NULL,
	name text NOT NULL,
	version int NOT NULL
);
GRANT ALL ON vendor_version TO deska_team;

--
-- functions
--

-- add
CREATE or REPLACE FUNCTION 
vendor_add(IN id char(36), IN name text, IN ver int)
RETURNS int
AS
$$
--	DECLARE ver int;
BEGIN	
	INSERT INTO vendor_version (uid, name, version)
		VALUES (id, name, ver);
	RETURN 1;
END
$$
language plpgsql;

-- commit
CREATE or REPLACE FUNCTION
vendor_commit(IN ver int)
RETURNS int
AS
$$
BEGIN
	UPDATE vendor as v SET name = new.name
		FROM vendor_version as new
			WHERE version = ver AND v.uid = new.uid;
	INSERT INTO vendor (uid,name)
		SELECT uid,name FROM vendor_version
			WHERE version = ver AND uid NOT IN ( SELECT uid FROM vendor );
	RETURN 1;
END
$$
language plpgsql;

