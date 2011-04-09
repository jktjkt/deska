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

