-- switch to deska_dev SCHEMA
SET search_path TO deska,versioning;

--
-- fuction for create changeset
--
CREATE FUNCTION create_changeset()
RETURNS integer
AS
$$
DECLARE max integer;
	parr integer;
BEGIN
	SELECT max(num) INTO max FROM version;
	IF NOT FOUND THEN
		-- not found parent revision
		RAISE SQLSTATE '10001' USING MESSAGE = 'No parent revision.';
	END IF;
	parr = num2id(max);
	INSERT INTO changeset (author,parentRevision,pid)
		VALUES (session_user,parr,pg_backend_pid());
	RETURN 1;
EXCEPTION
	WHEN unique_violation THEN
		-- already assigned changeset
		RAISE SQLSTATE '10002' USING MESSAGE = 'You have already assigned one changeset.';
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- fuction for commit changeset / create version and return human readable number
--
CREATE FUNCTION create_version(message_ text)
RETURNS integer
AS
$$
DECLARE ver integer;
	ret integer;
BEGIN
	SELECT get_current_changeset() INTO ver;
	-- FIXME: add commit message here
	INSERT INTO version (id,author)
		SELECT id,author FROM changeset
			WHERE id = ver;
	UPDATE version SET message = message_
		WHERE id = ver;
	SELECT num INTO ret FROM version
		WHERE id = ver;
	IF NOT FOUND THEN
		-- create version not successfull
		RAISE SQLSTATE '10004' USING MESSAGE = 'Error while creating revision.';
	END IF;
	PERFORM delete_changeset();
	RETURN ret;
	--FIXME: Exceptions
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
DECLARE ver bigint;
BEGIN
	ver = get_current_changeset();
	DELETE FROM changeset
		WHERE id = ver;
	RETURN 1;
	--FIXME: Exceptions
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- get my version id, this can return null
--
CREATE FUNCTION get_current_changeset_or_null()
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
-- get my version id
--
CREATE FUNCTION get_current_changeset()
RETURNS integer
AS
$$
DECLARE ver integer;
BEGIN
	SELECT id INTO ver FROM changeset
		WHERE pid = pg_backend_pid();
	IF NOT FOUND THEN
		-- no changeset assigned
		RAISE SQLSTATE '10003' USING MESSAGE = 'You do not have open any changeset.';
	END IF;
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
	IF NOT FOUND THEN
		-- not found parent revision
		RAISE SQLSTATE '10001' USING MESSAGE = 'No parent found.';
	END IF;
	RETURN par;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

-- API part here
SET search_path TO api,deska,versioning;
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
	SELECT get_current_changeset() INTO ver;
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
DECLARE chid bigint;
BEGIN
	chid = changeset2id(id_);
	UPDATE changeset SET status = 'INPROGRESS',
		pid = pg_backend_pid(), author = session_user
		WHERE id = chid; 
	IF NOT FOUND THEN
		-- no changeset of this name
		RAISE SQLSTATE '10006' USING MESSAGE = 'No changeset of this name.';
	END IF;
	RETURN 1;
EXCEPTION
        WHEN unique_violation THEN
	        -- already assigned changeset
		RAISE SQLSTATE '10002' USING MESSAGE = 'You have already assigned one changeset.';
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
		WHERE id = get_current_changeset(); 
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
	SELECT get_current_changeset() INTO ver;
	PERFORM delete_changeset();
	RETURN ver;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

CREATE TYPE changeset_type AS (
changeset text,
author text,
status changeset_status,
"parentRevision" text,
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
	RETURN QUERY SELECT id2changeset(id),author,status,num2revision(id2num(parentRevision)),timestamp,message FROM changeset;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- Commit changeset - just run genproc.commit
--
CREATE FUNCTION commitChangeset(commitMessage_ text)
RETURNS text
AS
$$
BEGIN
        RETURN num2revision(genproc.commit_all(commitMessage_));
END
$$
LANGUAGE plpgsql SECURITY DEFINER;
