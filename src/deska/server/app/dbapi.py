import psycopg2
import datetime
import time
import logging
import json

class DB:
	methods = dict({
		"kindNames": [],
                "kindAttributes": ["kindName"],
                "kindRelations": ["kindName"],
                "kindInstances": ["kindName","revision"],
                #"kindInstances": ["kindName","revision","filter"],
                "deleteObject": ["kindName","objectName"],
                "restoreDeletedObject": ["kindName","objectName"],
                "createObject": ["kindName","objectName"],
                "renameObject": ["kindName","oldObjectName","newObjectName"],
                "setAttribute": ["kindName","objectName","attributeName","attributeData"],
                "startChangeset": [],
                "commitChangeset": ["commitMessage"],
                "rebaseChangeset": ["parentRevision"],
                "pendingChangesets": ["filter"],
                "resumeChangeset": ["changeset"],
                "detachFromCurrentChangeset": ["message"],
                "abortCurrentChangeset": [],
		"dataDifference": ["revisionA", "revisionB"],
		"dataDifferenceInTemporaryChangeset": [],
		#"resolvedDataDifference": ["revisionA", "revisionB"],
		#"resolvedDataDifferenceInTemporaryChangeset": [],
		"objectData": ["kindName", "objectName","revision"],
		"objectData": ["kindName", "objectName","revision"],
		#"objectData": ["kindName", "objectName","revision", "filter"],
		"multipleObjectData": ["kindName", "revision"],
		#"multipleObjectData": ["kindName", "revision", "filter"],
		"listRevisions": ["filter"]
	})

	def __init__(self,**kwargs):
		try:
			self.db = psycopg2.connect(**kwargs);
			self.mark = self.db.cursor()
			self.mark.execute("SET search_path TO jsn,api,genproc,history,deska,versioning,production;")
			# commit search_path
			self.db.commit()
			self.freeze = False
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
	
	def responseJson(self,command):
		jsn = dict({"response": command})
		return json.dumps(jsn)

	def freezeUnfreeze(self,name):
		if name == "freezeView":
			# set isolation level serializable, and read only transaction
			# FIXME: better solution needs psycopg2.4.2
			# self.db.set_session(SERIALIZABLE,True)
			self.db.set_isolation_level(2)
			self.db.commit()
			# commit and start new transaction with selected properties
			self.freeze = True
			return self.responseJson(name)
		elif name == "unFreezeView":
			# set isolation level readCommited
			# FIXME: better solution needs psycopg2.4.2
			#self.db.set_session(DEFAULT,False)
			self.db.set_isolation_level(1)
			self.db.commit()
			# commit and start new transaction with selected properties
			self.db.commit()
			self.freeze = False
			return self.responseJson(name)
		else:
			return self.errorJson(name,"Only freeze or unFreeze")

	def commit(self):
		if not self.freeze:
			self.db.commit()

	def run(self,name,args):
		logging.debug("start run method({n}, {a})".format(n = name, a = args))
		# test if connection is ok
		if self.error is not None:
			return self.errorJson(name,"No connection to DB")

		# this two spectial commands handle db transactions
		if name in set(["freezeView","unFreezeView"]):
			return self.freezeUnfreeze(name)

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
			self.commit()
			return self.errorJson(name,e.message.split("\n")[0])

		logging.debug("fetchall returning: {d})".format(d = data))
		self.commit()
		return data


