SET search_path TO deska;

-- get prefix and name from full name of embed object
CREATE OR REPLACE FUNCTION embed_name(name text,delimiter text)
RETURNS text[]
AS
$$
ret = list()
str_array = name.split(delimiter)
if (len(str_array) < 2):
	raise Exception('Name "{0}" is not fully qualifide (does not containt "{1}").'.format(name,delimiter))
ret.append(delimiter.join(str_array[:len(str_array)-1]))
ret.append(str_array[len(str_array)-1])
return ret
$$
LANGUAGE plpythonu;

CREATE OR REPLACE FUNCTION join_with_delim(str1 text, str2 text, delimiter text)
RETURNS text
AS
$$
BEGIN
return str1 || delimiter || str2;
END
$$
LANGUAGE plpgsql;
