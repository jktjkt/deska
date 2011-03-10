BEGIN;

SET search_path TO genproc,history,production,deska;

CREATE FUNCTION getfn(fntype text, kindname text, attributename text = '')
RETURNS text
AS
$$
# strings
fn3_string = "{0}_{1}_{2}"
fn_string = "{0}_{1}"


if (fntype == "set") or  (fntype == "rem") :
	fname = fn3_string.format(kindname,fntype,attributename)
else:
	fname = fn_string.format(kindname,fntype)
return fname
$$
LANGUAGE plpythonu;

CREATE FUNCTION setAttribute(kindname text, objectname text, attributename text, value text)
	RETURNS text
AS
$$
plan = plpy.prepare("SELECT * from getfn('set',$1,$2)", [ "text", "text"])
res = plpy.execute(plan, [ kindname, attributename], 1)
fname = res[0]["getfn"]
plan = plpy.prepare("SELECT * from " + fname + "($1,$2)", [ "text", "text"])
res = plpy.execute(plan, [ objectname, value], 1)
return res[0][fname]
$$
LANGUAGE plpythonu;

END;
