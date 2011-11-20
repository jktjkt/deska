SET search_path TO deska,versioning;

-- get prefix and name from full name of embed object
CREATE OR REPLACE FUNCTION embed_name(name text,delimiter text)
RETURNS text[]
AS
$$
import Postgres

@pytypes
def main(name,delimiter):
	ret = list()
	str_array = name.split(delimiter)
	if (len(str_array) < 2):
		raise Postgres.ERROR('Name "{0}" is not fully qualified (does not contain "{1}").'.format(name,delimiter),code = 10123)

	ret.append(delimiter.join(str_array[:len(str_array)-1]))
	ret.append(str_array[len(str_array)-1])
	return ret
$$
LANGUAGE python;

CREATE OR REPLACE FUNCTION local_name_differs(name1 text, name2 text, delimiter text)
RETURNS boolean
AS
$$
DECLARE
	namearr1 text[];
	namearr2 text[];
	local_name1 text;
	local_name2 text;
BEGIN
	IF name1 = name2 THEN
		RETURN FALSE;
	END IF;
	
	namearr1 = embed_name(name1, delimiter);
	namearr2 = embed_name(name2, delimiter);
	local_name1 = namearr1[2];
	local_name2 = namearr2[2];

	IF local_name1 = local_name2 THEN 
		RETURN FALSE;
	ELSE
		RETURN TRUE;
	END IF;
END
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION join_with_delim(str1 text, str2 text, delimiter text)
RETURNS text
AS
$$
BEGIN
return str1 || delimiter || str2;
END
$$
LANGUAGE plpgsql;

CREATE TYPE int_name_type AS (
	name text,
	num int
);

CREATE OR REPLACE FUNCTION num_decorated_name(base_name text, count bigint)
RETURNS SETOF int_name_type
AS
$$
DECLARE
	i int;
	res int_name_type;
BEGIN
	FOR i IN 1..count LOOP
		res.name = base_name || '_' || i;
		res.num = i;
		RETURN NEXT res;
	END LOOP;
END;
$$
LANGUAGE plpgsql;
