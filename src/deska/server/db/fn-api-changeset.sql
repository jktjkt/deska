-- switch to deska_dev SCHEMA
SET search_path TO deska;

CREATE SEQUENCE version_num;

-- vendors of hw -- versioning table
CREATE TABLE version (
	-- internal id
	id bigserial
		CONSTRAINT version_pk PRIMARY KEY,
	-- human readable id
	num int
		CONSTRAINT version_numberunique UNIQUE,
	-- who a whet created
	username text,
	created timestamp without time zone NOT NULL DEFAULT now(),
	note text
);

-- current changesets
CREATE TABLE changeset (
	-- number - id version
	version bigint
		CONSTRAINT changeset_id_fk_version REFERENCES version(id),
	-- user - must be unique, use it for pk
	username text NOT NULL,
	-- backend pid - like session id
	pid int
		CONSTRAINT changeset_pid_pk PRIMARY KEY
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
DECLARE ver integer;
BEGIN
	INSERT INTO version (note,username)
		VALUES ('',current_user);
	SELECT max(id) INTO ver FROM version;
	RETURN ver;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

-- fuction for commit version and return human readable number
--
CREATE FUNCTION version_commit()
RETURNS integer
AS
$$
DECLARE ver integer;
	ret integer;
BEGIN
	SELECT my_version() INTO ver;
	SELECT nextval('version_num') INTO ret;
	UPDATE version SET username = current_user,
			num = ret
		WHERE id = ver; 
	PERFORM close_changeset();
	RETURN ret;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- close changeset
--
CREATE FUNCTION close_changeset()
RETURNS integer
AS
$$
BEGIN
	DELETE FROM changeset
		WHERE username = current_user;
	RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

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
		WHERE username = current_user AND pid = pg_backend_pid();
	RETURN ver;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

-- API part here
SET search_path TO api,deska;
--
-- start changeset
--
CREATE FUNCTION startChangeset()
RETURNS integer
AS
$$
DECLARE ver integer;
BEGIN
	SELECT add_version() INTO ver;
	INSERT INTO changeset (username,version,pid)
		VALUES (current_user,ver,pg_backend_pid());
	RETURN ver;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- resume to changeset
--
CREATE FUNCTION resumeChangeset(id integer)
RETURNS integer
AS
$$
BEGIN
	INSERT INTO changeset (username,version,pid)
		VALUES (current_user,id,pg_backend_pid());
	RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- detach changeset, same as close_changeset
--
CREATE FUNCTION detachFromCurrentChangeset(message text)
RETURNS integer
AS
$$
BEGIN
	UPDATE version SET note = message
		WHERE id = my_version(); 
	PERFORM close_changeset();
	RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- abort changeset, same as close_changeset
--
CREATE FUNCTION abortCurrentChangeset()
RETURNS integer
AS
$$
DECLARE ver integer;
BEGIN
	-- we need to keep version id and then delete it in this order (due to fk)
	SELECT my_version() INTO ver;
	DELETE FROM changeset
		WHERE version = ver;
	DELETE FROM version
		WHERE id = ver;
	RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;
