SET search_path TO api,genproc,history,deska,versioning,production;

drop function deska.jsnVersions();
CREATE OR REPLACE FUNCTION deska.jsnVersions()
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
