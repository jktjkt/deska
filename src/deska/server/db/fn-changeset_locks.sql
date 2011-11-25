SET search_path TO deska,versioning;

CREATE FUNCTION deska.lockChangeset()
RETURNS void
AS
$$
DECLARE
	is_locked boolean;
BEGIN
	SELECT pg_try_advisory_lock(id) INTO is_locked FROM changeset WHERE id = get_current_changeset();
	IF NOT is_locked THEN
		RAISE SQLSTATE '70015' USING MESSAGE = 'Current changeset can not be locked at the moment.';
	END IF;
END;
$$
LANGUAGE plpgsql;

CREATE FUNCTION deska.unlockChangeset()
RETURNS void
AS
$$
DECLARE
	is_locked boolean;
BEGIN
	SELECT pg_advisory_unlock(id) INTO is_locked FROM changeset WHERE id = get_current_changeset();
	IF NOT is_locked THEN
		RAISE SQLSTATE '70015' USING MESSAGE = 'Current changeset was not locked.';
	END IF;
END;
$$
LANGUAGE plpgsql;

CREATE FUNCTION deska.changesetHasFreshConfig()
RETURNS boolean
AS
$$
DECLARE
	is_up_to_date boolean;
BEGIN
	SELECT is_generated INTO is_up_to_date FROM changeset WHERE id = get_current_changeset();
	RETURN is_up_to_date;
END;
$$
LANGUAGE plpgsql;

--this function marks current changeset as having freshly generated configuration
CREATE FUNCTION deska.markChangesetFresh()
RETURNS void
AS
$$
BEGIN
	UPDATE changeset SET is_generated = TRUE WHERE id = get_current_changeset();
END;
$$
LANGUAGE plpgsql;
