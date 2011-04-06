BEGIN;

SET search_path TO api,genproc;

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
LANGUAGE plpythonu SECURITY DEFINER;


CREATE FUNCTION setAttribute(kindname text, objectname text, attributename text, value text)
RETURNS integer
AS
$$
plan = plpy.prepare("SELECT * from getfn('set',$1,$2)", [ "text", "text"])
res = plpy.execute(plan, [ kindname, attributename], 1)
fname = res[0]["getfn"]
plan = plpy.prepare("SELECT * from " + fname + "($1,$2)", [ "text", "text"])
res = plpy.execute(plan, [ objectname, value], 1)
return res[0][fname]
$$
LANGUAGE plpythonu SECURITY DEFINER;

CREATE FUNCTION removeAttribute(kindname text, objectname text, attributename text)
RETURNS integer
AS
$$
plan = plpy.prepare("SELECT * from getfn('rem',$1,$2)", [ "text", "text"])
res = plpy.execute(plan, [ kindname, attributename], 1)
fname = res[0]["getfn"]
plan = plpy.prepare("SELECT * from " + fname + "($1)", [ "text"])
res = plpy.execute(plan, [ objectname], 1)
return res[0][fname]
$$
LANGUAGE plpythonu SECURITY DEFINER;

CREATE FUNCTION changeObjectName(kindname text, oldname text, newname text)
RETURNS integer
AS
$$
plan = plpy.prepare("SELECT * from getfn('set',$1,$2)", [ "text", "text"])
res = plpy.execute(plan, [ kindname, "name"], 1)
fname = res[0]["getfn"]
plan = plpy.prepare("SELECT * from " + fname + "($1)", [ "text", "text"])
res = plpy.execute(plan, [ objectname], 1)
return res[0][fname]
$$
LANGUAGE plpythonu SECURITY DEFINER;

CREATE FUNCTION createObject(kindname text, objectname text)
RETURNS integer
AS
$$
plan = plpy.prepare("SELECT * from getfn('add',$1)", [ "text"])
res = plpy.execute(plan, [ kindname], 1)
fname = res[0]["getfn"]
plan = plpy.prepare("SELECT * from " + fname + "($1)", [ "text"])
res = plpy.execute(plan, [ objectname], 1)
return res[0][fname]
$$
LANGUAGE plpythonu SECURITY DEFINER;

CREATE FUNCTION deleteObject(kindname text, objectname text)
RETURNS integer
AS
$$
plan = plpy.prepare("SELECT * from getfn('del',$1)", [ "text"])
res = plpy.execute(plan, [ kindname], 1)
fname = res[0]["getfn"]
plan = plpy.prepare("SELECT * from " + fname + "($1)", [ "text"])
res = plpy.execute(plan, [ objectname], 1)
return res[0][fname]
$$
LANGUAGE plpythonu SECURITY DEFINER;

END;
