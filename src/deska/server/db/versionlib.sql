-- switch to deska_dev SCHEMA
SET search_path TO deska,production;

-- vendors of hw -- versioning table
CREATE TABLE version (
	-- internal id
	id bigserial
		CONSTRAINT version_pk PRIMARY KEY,
	-- human readable id
	number int
		CONSTRAINT version_number_unique UNIQUE,
	created timestamp without time zone NOT NULL DEFAULT now(),
	note text
);
GRANT ALL ON version TO deska_team;

--
-- functions
--

--
-- fuction for create version - and return it's number
--
CREATE OR REPLACE FUNCTION add_version()
RETURNS integer
AS
$$
DECLARE ret integer;
BEGIN
	INSERT INTO version (note)
		VALUES ('');
	SELECT max(id) INTO ret FROM version;
	RETURN ret;
END
$$
LANGUAGE plpgsql;

