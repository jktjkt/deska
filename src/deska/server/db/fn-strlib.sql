SET search_path TO deska;

--find position of last delimiter in delimited string
CREATE OR REPLACE FUNCTION rposition(str text,delimiter text)
RETURNS int
AS
$$
	return str.rfind(delimiter)
$$
LANGUAGE plpythonu;
