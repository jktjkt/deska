SET search_path TO deska,versioning;

CREATE FUNCTION deska.lockChangeset()
RETURNS void
AS
$$
BEGIN
	PERFORM pg_advisory_lock(id) FROM changeset WHERE id = get_current_changeset();
END;
$$
LANGUAGE plpgsql;

CREATE FUNCTION deska.lockIfDiffObsolete()
RETURNS boolean
AS
$$
DECLARE
	is_up_to_date boolean;
BEGIN
	SELECT is_generated INTO is_up_to_date FROM changeset WHERE id = get_current_changeset();
	PERFORM pg_advisory_lock(id) FROM changeset WHERE id = get_current_changeset();
	RETURN NOT is_up_to_date;
END;
$$
LANGUAGE plpgsql;

CREATE FUNCTION deska.releaseAndMarkAsOK()
RETURNS void
AS
$$
DECLARE
	is_locked boolean;
BEGIN
	UPDATE changeset SET is_generated = TRUE WHERE id = get_current_changeset();
	SELECT pg_advisory_unlock(id) INTO is_locked FROM changeset WHERE id = get_current_changeset();
	IF NOT is_locked THEN
		RAISE EXCEPTION 'current changeset is not locked';
	END IF;
END;
$$
LANGUAGE plpgsql;
