SET search_path TO deska,api,genproc,history,versioning,production;

drop function jsnVersions();
CREATE OR REPLACE FUNCTION jsnVersions()
RETURNS text
AS
$$
import Postgres
@pytypes
def main():
	plan = prepare("SELECT num2revision(id) AS version,author,timestamp,message FROM version")
	a = plan()

	slist = list()
	for line in a:
		res = dict()
		res["author"] = line["author"]
		res["timestamp"] = line["timestamp"]
		res["message"] = line["message"]
		res["version"] = line["version"]
		slist.append(res)
	return slist
$$
LANGUAGE python;

drop function jsnKindInstances(text);
CREATE OR REPLACE FUNCTION jsnKindInstances(kindName text)
RETURNS text
AS
$$
import Postgres
@pytypes
def main(kindName):
	plan = prepare("SELECT * FROM kindInstances($1)")
	a = plan(kindName)

	return list(a)
$$
LANGUAGE python;

