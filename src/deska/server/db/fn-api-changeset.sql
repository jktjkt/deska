-- switch to deska_dev SCHEMA
SET search_path TO deska;

CREATE SEQUENCE version_num;

CREATE TYPE changeset_status AS ENUM ( 'COMMITED', 'DETACHED', 'INPROGRESS' );

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
	status changeset_status NOT NULL DEFAULT 'INPROGRESS',
	created timestamp without time zone NOT NULL DEFAULT now(),
	note text
);
ALTER TABLE version ADD COLUMN
	-- revision of db, when new changset is started
	parrent int
		-- onlz for commited (with num assigned)
		CONSTRAINT version_parrent_uid REFERENCES version(num);

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
DECLARE parr integer;
BEGIN
	SELECT max(num) INTO parr FROM version;
	INSERT INTO version (note,username,parrent)
		VALUES ('',current_user,parr);
	-- FIXME: read it frpm sequence directly
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
			num = ret, status = 'COMMITED'
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
		WHERE version = my_version();
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

--
-- get id of parrent version
--
CREATE FUNCTION parrent(ver bigint)
RETURNS bigint
AS
$$
DECLARE parr bigint;
BEGIN
	SELECT p.id INTO parr FROM version p JOIN version v 
		ON v.id = ver AND v.parrent = p.num;
	RETURN parr;
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
CREATE FUNCTION resumeChangeset(id_ integer)
RETURNS integer
AS
$$
BEGIN
	UPDATE version SET status = 'INPROGRESS'
		WHERE id = id_; 
	INSERT INTO changeset (username,version,pid)
		VALUES (current_user,id_,pg_backend_pid());
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
	UPDATE version SET note = message, status = 'DETACHED'
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

CREATE TYPE changeset_info AS (
id bigint,
username text,
status changeset_status,
created timestamp,
note text
);

--
-- Return info about not commited changesets
--
CREATE FUNCTION pendingChangesets()
RETURNS SETOF changeset_info
AS
$$
BEGIN
	RETURN QUERY SELECT id,username,status,created,note FROM version
		WHERE status != 'COMMITED';
END
$$
LANGUAGE plpgsql SECURITY DEFINER;
