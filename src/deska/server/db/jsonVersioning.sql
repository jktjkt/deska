SET search_path TO deska;

/** API function startChangeset
   * Call proper stored function
   */
CREATE OR REPLACE FUNCTION jsn.startChangeset(tag text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag):
	name = "startChangeset"
	jsn = dutil.jsn(name,tag)

	fname = 'deska.'+ name + "()"
	try:
		ver = dutil.fcall(fname)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	jsn[name] = str(ver)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

/** API function abortCurrentChangeset
   * Call proper stored function
   */
CREATE OR REPLACE FUNCTION jsn.abortCurrentChangeset(tag text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag):
	name = "abortCurrentChangeset"
	jsn = dutil.jsn(name,tag)

	fname = 'deska.'+ name + "()"
	try:
		ver = dutil.fcall(fname)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

/** API function resumeChangeset
   * Call proper stored function
   */
CREATE OR REPLACE FUNCTION jsn.resumeChangeset(tag text, revision text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,revision):
	name = "resumeChangeset"
	jsn = dutil.jsn(name,tag)

	fname = 'deska.'+ name + "(text)"
	try:
		ver = dutil.fcall(fname,revision)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

/** API function commitChangeset
   * Call proper stored function
   */
CREATE OR REPLACE FUNCTION jsn.commitChangeset(tag text, commitMessage text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,commitMessage):
	name = "commitChangeset"
	jsn = dutil.jsn(name,tag)

	fname = 'deska.'+ name + "(text)"
	try:
		ver = dutil.fcall(fname,commitMessage)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	jsn[name] = str(ver)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

/** API function restoringCommit
   * Call proper stored function
   */
CREATE OR REPLACE FUNCTION api.restoringCommit(tag text, commitMessage text, author text, timestamp_ timestamp without time zone)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,commitMessage,author,timestamp):
	name = "restoringCommit"
	jsn = dutil.jsn(name,tag)

	fname = 'deska.'+ name + "(text,text,timestamp)"
	try:
		ver = dutil.fcall(fname,commitMessage,author,timestamp)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	jsn[name] = str(ver)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

/** API function detachFromCurrentChangeset
   * Call proper stored function
   */
CREATE OR REPLACE FUNCTION jsn.detachFromCurrentChangeset(tag text, message text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,message):
	name = "detachFromCurrentChangeset"
	jsn = dutil.jsn(name,tag)

	fname = 'deska.'+ name + "(text)"
	try:
		ver = dutil.fcall(fname,message)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

/** API function pendingChangesets
   * Select data from changeset table
   */
CREATE OR REPLACE FUNCTION jsn.pendingChangesets(tag text, filter text = NULL)
RETURNS text
AS
$$
import dutil
from filter import Filter
import json

@pytypes
def main(tag,filter):
	name = "pendingChangesets"
	jsn = dutil.jsn(name,tag)

	try:
		filter = Filter(filter,1)
		where, values = filter.getWhere()
		select = "SELECT id2changeset(metadata.id),metadata.author,metadata.status::text,num2revision(id2num(metadata.parentRevision)),to_char(metadata.timestamp,'YYYY-MM-DD HH24:MI:SS') AS timestamp,metadata.message FROM changeset AS metadata " + filter.getJoin("metadata") + where + " GROUP BY metadata.id, metadata.author, metadata.status, metadata.parentRevision, metadata.timestamp, metadata.message ORDER BY max(metadata.id)"
	except dutil.DutilException as err:
		return err.json(name,jsn)

	try:
		colnames,data = dutil.getdata(select,*values)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	res = list()
	for line in data:
		line = dutil.pytypes(line)
		ver = dict()
		ver["changeset"] = line[0]
		ver["author"] = line[1]
		ver["status"] = line[2]
		ver["parentRevision"] = line[3]
		ver["timestamp"] = line[4]
		ver["message"] = line[5]
		res.append(ver)
	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

/** API function listRevisions
   * Select data from version table
   */
CREATE OR REPLACE FUNCTION jsn.listRevisions(tag text, filter text = NULL)
RETURNS text
AS
$$
import dutil
from filter import Filter
import json

@pytypes
def main(tag,filter):
	name = "listRevisions"
	jsn = dutil.jsn(name,tag)

	try:
		filter = Filter(filter,1)
		where, values = filter.getWhere()
		select = "SELECT num2revision(metadata.num),metadata.author,to_char(metadata.timestamp,'YYYY-MM-DD HH24:MI:SS') AS timestamp,metadata.message FROM version AS metadata " + filter.getJoin("metadata") + where + " GROUP BY metadata.num, metadata.author, metadata.timestamp, metadata.message ORDER BY max(metadata.num)"
	except dutil.DutilException as err:
		return err.json(name,jsn)
	
	try:
		colnames,data = dutil.getdata(select,*values)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	res = list()
	for line in data:
		line = dutil.pytypes(line)
		ver = dict()
		ver["revision"] = line[0]
		ver["author"] = line[1]
		ver["timestamp"] = line[2]
		ver["commitMessage"] = line[3]
		res.append(ver)

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

/** API function dataDifference
   * Call proper stored functions for diff for all kinds
   */
CREATE OR REPLACE FUNCTION jsn.dataDifference(tag text, a text, b text)
RETURNS text
AS
$$
import Postgres
import json
import dutil
from dutil import oneKindDiff

@pytypes
def main(tag,a,b):
	name = "dataDifference"
	diffname = "normal"
	jsn = dutil.jsn(name,tag)

	res = list()
	try:
		for kindName in dutil.generated.kinds():
			res.extend(oneKindDiff(kindName,diffname,a,b))
	except Postgres.Exception as dberr:
		err = dutil.DeskaException(dberr)
		return err.json(name,jsn)

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

/** API function dataDifferenceInTemporaryChangeset
   * Call proper stored functions for diff for all kinds
   */
CREATE OR REPLACE FUNCTION jsn.dataDifferenceInTemporaryChangeset(tag text, changeset text = NULL)
RETURNS text
AS
$$
import Postgres
import json
import dutil
from dutil import oneKindDiff

@pytypes
def main(tag,changeset):
	name = "dataDifferenceInTemporaryChangeset"
	diffname = "normal"
	jsn = dutil.jsn(name,tag)

	res = list()
	try:
		for kindName in dutil.generated.kinds():
			res.extend(oneKindDiff(kindName,diffname,changeset))
	except Postgres.Exception as dberr:
		err = dutil.DeskaException(dberr)
		return err.json(name,jsn)

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

/** API function resolvedDataDifference
   * Call proper stored functions for diff for all kinds
   */
CREATE OR REPLACE FUNCTION jsn.resolvedDataDifference(tag text, a text, b text)
RETURNS text
AS
$$
import Postgres
import json
import dutil
from dutil import oneKindDiff

@pytypes
def main(tag,a,b):
	name = "resolvedDataDifference"
	diffname = "resolved"
	jsn = dutil.jsn(name,tag)

	res = list()
	try:
		for kindName in dutil.generated.kinds():
			res.extend(oneKindDiff(kindName,diffname,a,b))
	except Postgres.Exception as dberr:
		err = dutil.DeskaException(dberr)
		return err.json(name,jsn)

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

/** API function resolvedDataDifferenceInTemporaryChangeset
   * Call proper stored functions for diff for all kinds
   */
CREATE OR REPLACE FUNCTION jsn.resolvedDataDifferenceInTemporaryChangeset(tag text, changeset text = NULL)
RETURNS text
AS
$$
import Postgres
import json
import dutil
from dutil import oneKindDiff

@pytypes
def main(tag,changeset):
	name = "resolvedDataDifferenceInTemporaryChangeset"
	diffname = "resolved"
	jsn = dutil.jsn(name,tag)

	res = list()
	try:
		for kindName in dutil.generated.kinds():
			res.extend(oneKindDiff(kindName,diffname,changeset))
	except Postgres.Exception as dberr:
		err = dutil.DeskaException(dberr)
		return err.json(name,jsn)

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

/** API function startChangeset
   * Call proper stored function
   */
CREATE OR REPLACE FUNCTION jsn.lockCurrentChangeset(tag text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag):
	name = "lockCurrentChangeset"
	jsn = dutil.jsn(name,tag)

	fname = 'deska.'+ name + "()"
	try:
		dutil.fcall(fname)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

/** API function startChangeset
   * Call proper stored function
   */
CREATE OR REPLACE FUNCTION jsn.unlockCurrentChangeset(tag text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag):
	name = "unlockCurrentChangeset"
	jsn = dutil.jsn(name,tag)

	fname = 'deska.'+ name + "()"
	try:
		dutil.fcall(fname)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.changesetHasFreshConfig(tag text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag):
	name = "changesetHasFreshConfig"
	jsn = dutil.jsn(name,tag)

	fname = 'deska.'+ name + "()"
	try:
		ver = dutil.fcall(fname)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	jsn[name] = dutil.pytypes([ver])[0]
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.markChangesetFresh(tag text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag):
	name = "markChangesetFresh"
	jsn = dutil.jsn(name,tag)

	fname = 'deska.'+ name + "()"
	try:
		dutil.fcall(fname)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.getCurrentChangeset(tag text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag):
	name = "getCurrentChangeset"
	jsn = dutil.jsn(name,tag)

	fname = 'deska.get_current_changeset()'
	try:
		ver = dutil.fcall(fname)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	jsn[name] = "tmp{0}".format(ver)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;
