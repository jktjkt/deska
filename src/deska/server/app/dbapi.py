import psycopg2
import datetime
import time
import logging
import json

class DB:
	methods = dict({
		"kindNames": ["tag"],
                "kindAttributes": ["tag", "kindName"],
                "kindRelations": ["tag", "kindName"],
                "kindInstances": ["tag", "kindName","revision"],
                #"kindInstances": ["tag", "kindName","revision","filter"],
                "deleteObject": ["tag", "kindName","objectName"],
                "restoreDeletedObject": ["tag", "kindName","objectName"],
                "createObject": ["tag", "kindName","objectName"],
                "renameObject": ["tag", "kindName","oldObjectName","newObjectName"],
                "setAttribute": ["tag", "kindName","objectName","attributeName","attributeData"],
                "startChangeset": ["tag"],
                "commitChangeset": ["tag", "commitMessage"],
                "rebaseChangeset": ["tag", "parentRevision"],
                "pendingChangesets": ["tag", "filter"],
                "resumeChangeset": ["tag", "changeset"],
                "detachFromCurrentChangeset": ["tag", "message"],
                "abortCurrentChangeset": ["tag"],
		"dataDifference": ["tag", "revisionA", "revisionB"],
		"dataDifferenceInTemporaryChangeset": ["tag"],
		#"resolvedDataDifference": ["tag", "revisionA", "revisionB"],
		#"resolvedDataDifferenceInTemporaryChangeset": ["tag", ],
		"objectData": ["tag", "kindName", "objectName","revision"],
		"objectData": ["tag", "kindName", "objectName","revision"],
		#"objectData": ["tag", "kindName", "objectName","revision", "filter"],
		"multipleObjectData": ["tag", "kindName", "revision"],
		#"multipleObjectData": ["tag", "kindName", "revision", "filter"],
		"listRevisions": ["tag", "filter"]
	})

	def __init__(self,**kwargs):
		try:
			self.db = psycopg2.connect(**kwargs);
			self.mark = self.db.cursor()
			self.mark.execute("SET search_path TO jsn,api,genproc,history,deska,versioning,production;")
			self.error = None
		except Exception, e:
			self.error = e

	def utf2str(self,data):
		'''Convert dict structure into str'''
		if data is None:
			return None
		elif type(data) == dict:
			'''We need to create json here'''
			newdict = dict()
			for key in data:
				newdict[str(key)] = self.utf2str(data[key])
			return newdict
		elif type(data) == list:
			return map(self.utf2str,data)
		else:
			return str(data)

	def errorJson(self,command,message):
		jsn = dict({"response": command,
			"dbException": {"type": "ServerError", "message": message}
		})
		return json.dumps(jsn)

	def run(self,name,args):
		logging.debug("start run method({n}, {a})".format(n = name, a = args))
		# test if connection is ok
		if self.error is not None:
			return self.errorJson(name,"No connection to DB")
		# copy needed args from command definition
		needed_args = self.methods[name][:]
		# have we the exact needed arguments
		if set(needed_args) != set(args.keys()):
			not_present = set(needed_args) - set(args.keys())
			# note that "filter" and "revision" are always optional
			if not_present <= set(["filter", "revision"]):
				if "filter" in not_present:
					args["filter"] = None
				if "revision" in not_present:
					args["revision"] = None
				logging.debug("{0} was not present, pass None arguments".format(not_present))
			else:
				return self.errorJson(name,"Missing arguments: {0}".format(list(not_present)))
		# sort args
		args = [args[i] for i in needed_args]
		# cast to string
		args = self.utf2str(args)

		for i in range(len(args)):
			if type(args[i]) == dict:
				args[i] = json.dumps(args[i])

		try:
			self.mark.callproc(name,args)
			data = self.mark.fetchall()[0][0]
		except Exception, e:
			logging.debug("Exception when call db function: {e})".format(e = e))
			self.db.commit()
			return self.errorJson(name,e.message.split("\n")[0])

		logging.debug("fetchall returning: {d})".format(d = data))
		self.db.commit()
		return data


