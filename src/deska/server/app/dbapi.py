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
		"kindInstances": ["tag", "kindName", "revision", "filter"],
		"objectData": ["tag", "kindName", "objectName", "revision"],
		"multipleObjectData": ["tag", "kindName", "revision", "filter"],
		"resolvedObjectData": ["tag", "kindName", "objectName","revision"],
		#"multipleResolvedObjectData": ["tag", "kindName", "revision","filter"],
		#"resolvedObjectDataWithOrigin": ["tag", "kindName", "objectName","revision"],
		#"multipleResolvedObjectDataWithOrigin": ["tag", "kindName", "revision", "filter"],
		"deleteObject": ["tag", "kindName", "objectName"],
		"createObject": ["tag", "kindName", "objectName"],
		"restoreDeletedObject": ["tag", "kindName","objectName"],
		"renameObject": ["tag", "kindName", "oldObjectName", "newObjectName"],
		"setAttribute": ["tag", "kindName", "objectName", "attributeName", "attributeData"],
		"setAttributeInsert": ["tag", "kindName", "objectName", "attributeName", "attributeData"],
		"setAttributeRemove": ["tag", "kindName", "objectName", "attributeName", "attributeData"],
		# applyBatchedChanges is somewhere else
		"startChangeset": ["tag"],
		"commitChangeset": ["tag", "commitMessage"],
		"pendingChangesets": ["tag", "filter"],
		"resumeChangeset": ["tag", "changeset"],
		"detachFromCurrentChangeset": ["tag", "message"],
		"abortCurrentChangeset": ["tag"],
		# lockCurrentChangeset is special
		# unlockCurrentChangeset is special
		# freezeView is special
		# unfreezeView is special
		"listRevisions": ["tag", "filter"],
		"dataDifference": ["tag", "revisionA", "revisionB"],
		"dataDifferenceInTemporaryChangeset": ["tag", "changeset"],
		#"resolvedDataDifference": ["tag", "revisionA", "revisionB"],
		#"resolvedDataDifferenceInTemporaryChangeset": ["tag", "changeset"],
		# showConfigDiff is special
	})

	def __init__(self, dbOptions, cfggenBackend, cfggenOptions):
		try:
			self.db = psycopg2.connect(**dbOptions);
			self.mark = self.db.cursor()
			self.mark.execute("SET search_path TO jsn,api,genproc,history,deska,versioning,production;")
			# commit search_path
			self.db.commit()
			self.freeze = False
			self.error = None
			self.cfggenBackend = cfggenBackend
			self.cfggenOptions = cfggenOptions
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
		# end possibly running transaction
		self.endTransaction()
		for command in changelist:
			if "command" not in command:
				return self.errorJson(name,tag,"Missing command.")
			name = command["command"]
			del command["command"]
			try:
				# just run, no responce
				self.runDBFunction(name,command,tag)
			except Exception, e:
				# abort if error here
				self.db.rollback()
				return self.errorJson(name, tag, str(e))

		self.endTransaction()
		return self.responseJson("applyBatchedChanges",tag)

	def endTransaction(self):
		if not self.freeze:
			self.db.commit()

	def lockChangeset(self):
		# FIXME: implement me by calling a DB function
		pass

	def unlockChangeset(self):
		# FIXME: implement me by calling a DB function
		pass

	def changesetHasFreshConfig(self):
		# FIXME: implement me by calling a DB function
		return False

	def markChangesetFresh(self):
		# FIXME: implement me by calling a DB function
		pass

	def currentChangeset(self):
		# FIXME: get name of the current changeset from the DB
		return "unnamed"

	def initCfgGenerator(self):
		logging.debug("initCfgGenerator")
		# We really do want to re-init the config generator each and every time
		import sys
		import os
		oldpath = sys.path
		mypath = os.path.normpath(os.path.join(os.path.dirname(__file__), "../../LowLevelPyDeska/generators"))
		sys.path = [mypath] + sys.path
		logging.debug("sys.path prepended by %s" % mypath)
		if self.cfggenBackend == "git":
			logging.debug("Initializing git generator")
			import shutil
			from gitgenerator import GitGenerator
			repodir = self.cfggenOptions["cfggenGitRepo"]
			workdir = self.cfggenOptions["cfggenGitWorkdir"] + "/" + self.currentChangeset()
			if os.path.exists(workdir):
				# got to clean it up
				shutil.rmtree(workdir)
			scriptdir = self.cfggenOptions["cfggenScriptPath"]
			self.cfgGenerator = GitGenerator(repodir, workdir, scriptdir)
		elif self.cfggenBackend == "fake":
			from nullgenerator import NullGenerator
			self.cfgGenerator = NullGenerator(behavior=None)
			logging.debug("No generators configured, will silently do nothing upon request")
		else:
			# no configuration generator has been configured
			from nullgenerator import NullGenerator
			self.cfgGenerator = NullGenerator(behavior=NotImplementedError("Attempted to access configuration generators which haven't been configured yet"))
			logging.debug("No generators configured, will raise error upon use")
		sys.path = oldpath

	def cfgRegenerate(self):
		logging.debug("cfgRegenerate")
		logging.debug(" opening repository")
		self.cfgGenerator.openRepo()
		self.cfgGenerator.generate()

	def cfgPushToScm(self, message):
		self.cfgGenerator.apiSave(message)

	def cfgGetDiff(self):
		return self.cfgGenerator.diff()

	def showConfigDiff(self, name, tag, forceRegen):
		logging.debug("showConfigDiff")
		try:
			response = {"response": name, "tag": tag}
			self.lockChangeset()
			self.initCfgGenerator()
			if forceRegen or not self.changesetHasFreshConfig():
				logging.debug("about to regenerate config")
				self.cfgRegenerate()
				self.markChangesetFresh()
			response[name] = self.cfgGetDiff()
			self.unlockChangeset()
			return json.dumps(response)
		except Exception, e:
			return self.errorJson(name, tag, str(e))

	def commitConfig(self, name, args, tag):
		try:
			self.checkFunctionArguments(name, args, tag)
			self.lockChangeset()
			self.initCfgGenerator()
			if not self.changesetHasFreshConfig():
				self.cfgRegenerate()
				self.markChangesetFresh()
			self.cfgPushToScm(args["commitMessage"])
			res = self.standaloneRunDbFunction(name, args, tag)
			self.unlockChangeset()
			return res
		except Exception, e:
			return self.errorJson(name, tag, str(e))

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

		if name == "showConfigDiff":
			forceRegen = False
			if args.has_key("forceRegen") and args["forceRegen"] == True:
				forceRegen = True
			return self.showConfigDiff(name, tag, forceRegen)

		if name == "commitChangeset":
			# this one is special, it has to commit to the DB *and* push to SCM
			return self.commitConfig(name, args, tag)

		# applyBatchedChanges
		if name == "applyBatchedChanges":
			if "modifications" not in args:
				return self.errorJson(name, tag, "Missing 'modifications'!")
			return self.applyBatchedChanges(args["modifications"],tag)

		return self.standaloneRunDbFunction(name, args, tag)

	def standaloneRunDbFunction(self, name, args, tag):
		try:
			data = self.runDBFunction(name,args,tag)
			self.endTransaction()
			return data
		except Exception, e:
			self.endTransaction()
			return self.errorJson(name, tag, str(e))

	def checkFunctionArguments(self, name, args, tag):
		"""Check the signature of the function against our definition"""
		if name not in self.methods:
			raise Exception("'%s' is not a DBAPI method" % name)
		needed_args = self.methods[name]
		# Check that the caller has specified all of the required arguments
		if set(needed_args) != set(args.keys()):
			not_present = set(needed_args) - set(args.keys())
			extra_args = set(args.keys()) - set(needed_args)
			# note that "filter" and "revision" are always optional
			if not_present <= set(["filter", "revision"]):
				if "filter" in not_present:
					args["filter"] = None
				if "revision" in not_present:
					args["revision"] = None
				logging.debug("%s was not present, pass None arguments" % not_present)
			else:
				raise Exception("Missing arguments: %s" % not_present)
			if len(extra_args):
				raise Exception("Extra arguments: %s" % extra_args)

	def runDBFunction(self,name,args,tag):
		self.checkFunctionArguments(name, args, tag)
		needed_args = self.methods[name]

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
			raise Exception("Missing arguments: %s" % str(e).split("\n")[0])

		logging.debug("fetchall returning: %s)" % data)
		return data


