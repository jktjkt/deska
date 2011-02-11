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

-- current changesets
CREATE TABLE changeset (
	-- number - id version
	version bigint
		CONSTRAINT session_id_fk_version REFERENCES version(id),
	-- user - must be unique, use it for pk
	username text
		CONSTRAINT session_pk PRIMARY KEY
);



--
-- functions
--

--
-- fuction for create version - and return it's number
--
CREATE FUNCTION add_version()
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

--
-- start changeset
--
CREATE FUNCTION start_changeset()
RETURNS integer
AS
$$
DECLARE ver integer;
BEGIN
	SELECT add_version() INTO ver;
	INSERT INTO changeset (username,version)
		VALUES (current_user,ver);
	RETURN 1;
END
$$
LANGUAGE plpgsql;

--
-- get my version id
--
CREATE FUNCTION my_version()
RETURNS integer
AS
$$
DECLARE ver integer;
BEGIN
	SELECT version INTO ver FROM changeset
		WHERE username = current_user;
	RETURN ver;
END
$$
LANGUAGE plpgsql;
