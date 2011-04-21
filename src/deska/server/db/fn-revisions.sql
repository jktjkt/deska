SET search_path TO deska;

--
-- get changeset identifier from its id
--
CREATE OR REPLACE FUNCTION id2changeset(id bigint)
RETURNS text
AS
$$
return "tmp{id}".format(id = id)
$$
LANGUAGE plpythonu SECURITY DEFINER;

--
-- get revision identifier from its number
--
CREATE OR REPLACE FUNCTION num2revision(id bigint)
RETURNS text
AS
$$
return "r{id}".format(id = id)
$$
LANGUAGE plpythonu SECURITY DEFINER;

--
-- get changeset id from its identificator
--
CREATE OR REPLACE FUNCTION changeset2id(rev text)
RETURNS bigint
AS
$$
#FIXME: match "tmp"
if len(rev) < 4:
	plpy.SPIError(55555)
	plpy.error(10005)
	return 0

return rev[3:len(rev)]
$$
LANGUAGE plpythonu SECURITY DEFINER;

--
-- get number from revision indentifier
--
CREATE OR REPLACE FUNCTION revision2num(rev text)
RETURNS bigint
AS
$$
#FIXME: match "r"
return rev[1:len(rev)]
$$
LANGUAGE plpythonu SECURITY DEFINER;

--
-- get number from revision id
--
CREATE OR REPLACE FUNCTION id2num(id_ bigint)
RETURNS bigint
AS
$$
DECLARE rev bigint;
BEGIN
	SELECT num INTO rev FROM version
		WHERE id = id_;
	IF NOT FOUND THEN
		RAISE EXCEPTION 'changeset with version % was not commited', id_;
	END IF;
	RETURN rev;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- get id from revision number
--
CREATE OR REPLACE FUNCTION num2id(num_ bigint)
RETURNS bigint
AS
$$
DECLARE rev bigint;
BEGIN
	SELECT id INTO rev FROM version
		WHERE num = num_;
	IF NOT FOUND THEN
		RAISE EXCEPTION 'version with changeset num % was not commited', num_;
	END IF;
	RETURN rev;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;
