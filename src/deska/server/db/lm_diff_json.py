set search_path to deska, api, production, history, versioning, genproc;

drop function lm_diff_json(bigint,bigint);
CREATE OR REPLACE FUNCTION lm_diff_json(x bigint, y bigint)
RETURNS text
AS
$$
import Postgres
@pytypes
def main(x,y):
	plan = prepare("SELECT * FROM large_modul_init_diff($1,$2)")
	a = plan(x,y)
	plan = prepare("SELECT * FROM large_modul_diff_created()")
	a = plan()

	res = list()
	for line in a:
		base = dict()
		base["command"] = "createObject"
		base["kindName"] = "large_modul"
		base["objectName"] = line["large_modul_diff_created"]
		res.append(str(base))

	plan = prepare("SELECT * FROM large_modul_diff_set_attributes()")
	a = plan()

	for line in a:
		base = dict()
		base["command"] = "setAttribute"
		base["kindName"] = "large_modul"
		base["attributeName"] = line["attribute"]
		base["value"] = line["newdata"]
		base["objectName"] = line["objname"]
		res.append(str(base))


	plan = prepare("SELECT * FROM large_modul_diff_deleted()")
	a = plan()

	for line in a:
		base = dict()
		base["command"] = "deleteObject"
		base["kindName"] = "large_modul"
		base["objectName"] = line["large_modul_diff_deleted"]
		res.append(str(base))

	d = dict()
	d["command"] = "responce"
	d["data"] = res

$$
LANGUAGE python;
