SET search_path TO deska,versioning;

--
-- get changeset identifier from its id
--
CREATE OR REPLACE FUNCTION id2changeset(id bigint)
RETURNS text
AS
$$

@pytypes
def main(id):
	return "tmp{id}".format(id = id)
$$
LANGUAGE python SECURITY DEFINER;

--
-- get revision identifier from its number
--
CREATE OR REPLACE FUNCTION num2revision(id bigint)
RETURNS text
AS
$$
@pytypes
def main(id):
	return "r{id}".format(id = id)
$$
LANGUAGE python SECURITY DEFINER;

--
-- get changeset id from its identificator
--
CREATE OR REPLACE FUNCTION changeset2id(ch text)
RETURNS bigint
AS
$$
DECLARE
    changeset_id bigint;
BEGIN
    IF ch IS NULL THEN
        RETURN 0;
    END IF;

    IF ch !~ E'^tmp\\d+$' THEN
        RAISE '% is not valid changeset id.', ch USING ERRCODE = '70011';
    END IF;
    
    changeset_id = CAST (substr(ch,4) as bigint);

    IF changeset_id <= 0 THEN
        RAISE '% is not valid changeset id.', ch USING ERRCODE = '70011';
    END IF;
        
    IF NOT EXISTS (SELECT * FROM changeset WHERE id = changeset_id) THEN
        RAISE 'Changeset % does not exist.', ch USING ERRCODE = '70014';
    END IF;
    
    RETURN changeset_id;
END;
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- get number from revision indentifier
--
CREATE OR REPLACE FUNCTION revision2num(rev text)
RETURNS bigint
AS
$$
DECLARE
	revnum bigint;
BEGIN
	IF rev IS NULL THEN
		RETURN 0;
	END IF;

	IF rev !~ E'^r\\d+$' THEN
		RAISE '% is not valid version id.', rev USING ERRCODE = '70012';
	END IF;
	
	revnum = CAST (substr(rev,2) as bigint);
	
	IF NOT EXISTS (SELECT * FROM version WHERE num = revnum) THEN
		RAISE 'Version % does not exist.', rev USING ERRCODE = '70013';
	END IF;
	
	RETURN revnum;
END;
$$
LANGUAGE plpgsql SECURITY DEFINER;

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
		RAISE 'Changeset with version % was not commited', id_ USING ERRCODE = '70013';
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
		RAISE 'Version with changeset num % was not commited', id_ USING ERRCODE = '70013';
	END IF;
	RETURN rev;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;
