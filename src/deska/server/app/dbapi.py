import psycopg2
import datetime
import time
import logging
try:
    import json
except ImportError:
    import simplejson as json

class DB:
	methods = dict({
		"kindNames": ["tag"],
                "kindAttributes": ["tag", "kindName"],
                "kindRelations": ["tag", "kindName"],
                "kindInstances": ["tag", "kindName","revision","filter"],
                "deleteObject": ["tag", "kindName","objectName"],
                "restoreDeletedObject": ["tag", "kindName","objectName"],
                "createObject": ["tag", "kindName","objectName"],
                "renameObject": ["tag", "kindName","oldObjectName","newObjectName"],
                "setAttribute": ["tag", "kindName","objectName","attributeName","attributeData"],
                "startChangeset": ["tag"],
                "commitChangeset": ["tag", "commitMessage"],
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
		"multipleObjectData": ["tag", "kindName", "revision", "filter"],
		"listRevisions": ["tag", "filter"]
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

	def errorJson(self,command,tag,message):
		jsn = dict({"response": command,
			"dbException": {"type": "ServerError", "message": message}
		})
		if tag is not None:
			jsn["tag"] = tag
		return json.dumps(jsn)

	def responseJson(self,command,tag):
		jsn = dict({"response": command, "tag": tag})
		return json.dumps(jsn)

	def freezeUnfreeze(self,name,tag):
		if name == "freezeView":
			# set isolation level serializable, and read only transaction
			# FIXME: better solution needs psycopg2.4.2
			# self.db.set_session(SERIALIZABLE,True)
			self.db.set_isolation_level(2)
			self.db.commit()
			# commit and start new transaction with selected properties
			self.freeze = True
			return self.responseJson(name,tag)
		elif name == "unFreezeView":
			# set isolation level readCommited
			# FIXME: better solution needs psycopg2.4.2
			#self.db.set_session(DEFAULT,False)
			self.db.set_isolation_level(1)
			self.db.commit()
			# commit and start new transaction with selected properties
			self.db.commit()
			self.freeze = False
			return self.responseJson(name,tag)
		else:
				return self.errorJson(name,tag,"Only freeze or unFreeze")

	def applyBatchedChanges(self,changelist,tag):
		if type(changelist) != list:
			return self.errorJson(name,tag,"Modifications parameter must be list.")
		for command in changelist:
			if "command" not in command:
				return self.errorJson(name,tag,"Missing command.")
			name = command["command"]
			del command["command"]
			self.run(name,command)

		return self.responseJson("applyBatchedChanges",tag)

	def commit(self):
		if not self.freeze:
			self.db.commit()

	def run(self,name,args):
		logging.debug("start run method(%s, %s)" % (name, args))

		if "tag" not in args:
			return self.errorJson(name, None, "Missing 'tag'!")
		tag = args["tag"]

		# test if connection is ok
		if self.error is not None:
			return self.errorJson(name, tag, "No connection to DB")


		# this two spectial commands handle db transactions
		if name in set(["freezeView","unFreezeView"]):
			return self.freezeUnfreeze(name,tag)

		# applyBatchedChanges
		if name == "applyBatchedChanges":
			if "modifications" not in args:
				return self.errorJson(name, tag, "Missing 'modifications'!")
			return self.applyBatchedChanges(args["modifications"],tag)

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
				logging.debug("%s was not present, pass None arguments" % not_present)
			else:
				return self.errorJson(name,tag,"Missing arguments: %s" % list(not_present))
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
			logging.debug("Exception when call db function: %s)" % str(e))
			self.commit()
			return self.errorJson(name,tag,e.message.split("\n")[0])

		logging.debug("fetchall returning: %s)" % data)
		self.commit()
		return data


