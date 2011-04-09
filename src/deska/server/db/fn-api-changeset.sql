-- switch to deska_dev SCHEMA
SET search_path TO deska;

CREATE SEQUENCE version_num;

CREATE TYPE changeset_status AS ENUM ( 'DETACHED', 'INPROGRESS' );

-- COMMITED versions
CREATE TABLE version(
	id bigint
		CONSTRAINT version_pk PRIMARY KEY,
	-- human readable id
	num int
		CONSTRAINT version_num_unique UNIQUE
			DEFAULT nextval('version_num'),
	-- who created
	author text,
	-- time of commit
	timestamp timestamp without time zone NOT NULL DEFAULT now(),
	-- commit message text
	message text
);

-- INPROGRESS and DETACHED changesets
CREATE TABLE changeset(
	-- internal id
	id bigserial
		CONSTRAINT changeset_pk PRIMARY KEY,
	author text NOT NULL,
	-- backend pid - like session id
	pid int
		CONSTRAINT changeset_pid_unique UNIQUE,
	-- revision of db, when new changset is started
	parentrevision bigint
		-- only for commited (with num assigned)
		CONSTRAINT changeset_parentrevision_uid REFERENCES version(id),
	status changeset_status NOT NULL DEFAULT 'INPROGRESS',
	-- time of creation
	timestamp timestamp without time zone NOT NULL DEFAULT now(),
	message text
		-- check message not null when detached
		CONSTRAINT changeset_message_not_null
			CHECK ((length(message) > 0)
				OR (status = 'INPROGRESS'))
);


--
-- functions
--

--
-- fuction for create changeset
--
CREATE FUNCTION create_changeset()
RETURNS integer
AS
$$
DECLARE parr integer;
	max integer;
BEGIN
	SELECT max(num) INTO max FROM version;
	SELECT id INTO parr FROM version
		WHERE max = num;
	INSERT INTO changeset (author,parentRevision,pid)
		VALUES (session_user,parr,pg_backend_pid());
	RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- fuction for commit changeset / create version and return human readable number
--
CREATE FUNCTION create_version()
RETURNS integer
AS
$$
DECLARE ver integer;
	ret integer;
BEGIN
	SELECT my_version() INTO ver;
	-- FIXME: add commit message here
	INSERT INTO version (id,author)
		SELECT id,author FROM changeset
			WHERE id = ver;
	SELECT num INTO ret FROM version
		WHERE id = ver;
	PERFORM delete_changeset();
	RETURN ret;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- delete changeset
--
CREATE FUNCTION delete_changeset()
RETURNS integer
AS
$$
BEGIN
	DELETE FROM changeset
		WHERE id = my_version();
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
	SELECT id INTO ver FROM changeset
		WHERE pid = pg_backend_pid();
	RETURN ver;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- get id of parent version
--
CREATE FUNCTION parent(ver bigint)
RETURNS bigint
AS
$$
DECLARE par bigint;
BEGIN
	SELECT parentrevision INTO par FROM changeset 
		WHERE id = ver;
	RETURN par;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

-- API part here
SET search_path TO api,deska;
--
-- start changeset
--
CREATE FUNCTION startChangeset()
RETURNS text
AS
$$
DECLARE ver integer;
BEGIN
	PERFORM create_changeset();
	SELECT my_version() INTO ver;
	RETURN id2changeset(ver);
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- resume to changeset
--
CREATE FUNCTION resumeChangeset(id_ text)
RETURNS integer
AS
$$
BEGIN
	UPDATE changeset SET status = 'INPROGRESS',
		pid = pg_backend_pid(), author = session_user
		WHERE id = changeset2id(id_); 
	RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- detach changeset
--
CREATE FUNCTION detachFromCurrentChangeset(message_ text)
RETURNS integer
AS
$$
BEGIN
	UPDATE changeset SET message = message_,
		status = 'DETACHED', pid = NULL
		WHERE id = my_version(); 
	RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- abort changeset, same as delete_changeset
--
CREATE FUNCTION abortCurrentChangeset()
RETURNS integer
AS
$$
DECLARE ver integer;
BEGIN
	SELECT my_version() INTO ver;
	PERFORM delete_changeset();
	RETURN ver;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

CREATE TYPE changeset_type AS (
changeset text,
author text,
status changeset_status,
parentrevision bigint,
timestamp timestamp,
message text
);
--
-- Return info about not commited changesets
--
CREATE FUNCTION pendingChangesets()
RETURNS SETOF changeset_type
AS
$$
BEGIN
	RETURN QUERY SELECT id2changeset(id),author,status,parentrevision,timestamp,message FROM changeset;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- Commit changeset - just run genproc.commit
--
CREATE FUNCTION commitChangeset()
RETURNS text
AS
$$
BEGIN
        RETURN num2revision(genproc.commit_all());
END
$$
LANGUAGE plpgsql SECURITY DEFINER;
