SET search_path TO deska,versioning;

CREATE FUNCTION deska.lockChangeset(changeset_id bigint)
RETURNS void
AS
$$
BEGIN
	PERFORM pg_advisory_lock(id) FROM changeset WHERE id = changeset_id;
END;
$$
LANGUAGE plpgsql;

CREATE FUNCTION deska.lockIfDiffObsolete(changeset_id bigint)
RETURNS boolean
AS
$$
DECLARE
	is_up_to_date boolean;
BEGIN
	SELECT is_generated INTO is_up_to_date FROM changeset WHERE id = changeset_id;
	PERFORM pg_advisory_lock(id) FROM changeset WHERE id = changeset_id;
	RETURN NOT is_up_to_date;
END;
$$
LANGUAGE plpgsql;

CREATE FUNCTION deska.releaseAndMarkAsOK(changeset_id bigint)
RETURNS void
AS
$$
DECLARE
	is_locked boolean;
BEGIN
	UPDATE changeset SET is_generated = TRUE WHERE id = changeset_id;
	SELECT pg_advisory_unlock(id) INTO is_locked FROM changeset WHERE id = changeset_id;
	IF NOT is_locked THEN
		RAISE EXCEPTION 'changeset % is not locked', changeset_id;
	END IF;
END;
$$
LANGUAGE plpgsql;
