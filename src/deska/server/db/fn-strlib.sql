SET search_path TO deska;

-- get prefix and name from full name of embed object
CREATE OR REPLACE FUNCTION embed_name(str text,delimiter text)
RETURNS text[]
AS
$$
ret = list()
str_array = str.split(delimiter)
if (len(str_array) < 2):
	raise 'this object is embed into another, this name is not fully qualifide';
ret.append(delimiter.join(str_array[:len(str_array)-1]))
ret.append(str_array[len(str_array)-1])
return ret
$$
LANGUAGE plpythonu;
