import sys
import os
import psycopg2
import datetime
import time
import logging
import subprocess
import shutil
try:
    import json
except ImportError:
    import simplejson as json
from jsonparser import perform_io

class FreezingError(Exception):
    pass

class DB:
	methods = dict({
		"kindNames": ["tag"],
		"kindAttributes": ["tag", "kindName"],
		"kindRelations": ["tag", "kindName"],
		"kindInstances": ["tag", "kindName", "revision", "filter"],
		"objectData": ["tag", "kindName", "objectName", "revision"],
		"multipleObjectData": ["tag", "kindName", "revision", "filter"],
		"resolvedObjectData": ["tag", "kindName", "objectName","revision"],
		"multipleResolvedObjectData": ["tag", "kindName", "revision","filter"],
		"resolvedObjectDataWithOrigin": ["tag", "kindName", "objectName","revision"],
		"multipleResolvedObjectDataWithOrigin": ["tag", "kindName", "revision", "filter"],
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
		"restoringCommit": ["tag", "commitMessage", "author", "timestamp"],
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
		"resolvedDataDifference": ["tag", "revisionA", "revisionB"],
		"resolvedDataDifferenceInTemporaryChangeset": ["tag", "changeset"],
		# showConfigDiff is special
	})
	writeFunctions = ["startChangeset", "commitChangeset", "restoringCommit", "resumeChangeset", "abortCurrentChangeset",
		"detachFromCurrentChangeset", "lockCurrentChangeset", "unlockCurrentChangeset",
		"deleteObject", "createObject", "restoreDeletedObject", "renameObject", "setAttribute",
		"setAttributeInsert", "setAttributeRemove"]

	def __init__(self, dbOptions, cfggenBackend, cfggenOptions):
		self.db = psycopg2.connect(**dbOptions);
		self.mark = self.db.cursor()
		self.mark.execute("SET search_path TO jsn,api,genproc,history,deska,versioning,production;")
		# commit search_path
		self.db.commit()
		self.freeze = False
		self.cfggenBackend = cfggenBackend
		self.cfggenOptions = cfggenOptions

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

	def errorJson(self,command,tag,message,type="ServerError"):
		jsn = dict({"response": command,
			"dbException": {"type": type, "message": message}
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
			changeset = self.callProc("get_current_changeset_or_null",{})
			if changeset is not None:
				raise FreezingError("Cannot run freezeView, changeset tmp%s is attached." % changeset)
			# FIXME: better solution needs psycopg2.4.2
			# self.db.set_session(SERIALIZABLE,True)
			self.db.set_isolation_level(2)
			self.db.commit()
			# commit and start new transaction with selected properties
			self.freeze = True
			return self.responseJson(name,tag)
		elif name == "unFreezeView":
			if not self.freeze:
				raise FreezingError("Cannot call unFreezeView, view is not frozen")
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
			#add tag into the command for propper db call
			command["tag"] = tag
			try:
				# just run, no responce
				data = self.runDBFunction(name,command,tag)
			except Exception, e:
				# abort if error here
				self.db.rollback()
				return self.errorJson(name, tag, str(e))
			if "dbException" in data:
				'''abort db transaction if exception occured'''
				self.db.rollback()
				# return data with dbException
				return data


		self.endTransaction()
		return self.responseJson("applyBatchedChanges",tag)

	def endTransaction(self):
		if not self.freeze:
			self.db.commit()

	def lockCurrentChangeset(self):
		'''Lock changeset by db lock'''
		# This function is accessed from the DBAPI and also directly from the
		# Python code. The Python code expects to get an exception when locking
		# fails, while the DBAPI usage wants to get it conveyed through JSON.
		# This function shall therefore throw exceptions when it didn't get the
		# lock and the DBAPI wrapper shall convert it to a JSON-based error
		# report.
		# FIXME: make sure that this somehow raises an exception which will:
		#	1) cause commitConfig to fail,
		#	2) be always correctly reported as the ChangesetLockingError to the
		#	DBAPI when called from there
		self.callProc("lockCurrentChangeset",{})

	def unlockCurrentChangeset(self):
		'''Unlock changeset'''
		# FIXME: see lockCurrentChangeset for what needs to be done
		self.callProc("unlockCurrentChangeset",{})

	def changesetHasFreshConfig(self):
		# FIXME: implement me by calling a DB function
		#self.callProc("...NO FUNCTION NOW...",{})
		return False

	def markChangesetFresh(self):
		# FIXME: implement me by calling a DB function
		#self.callProc("releaseAndMarkAsOK",{})
		pass

	def currentChangeset(self):
		'''get name of the current changeset from the DB'''
		return "tmp" + str(self.callProc("get_current_changeset",{}))

	def initCfgGenerator(self):
		logging.debug("initCfgGenerator")
		if self.cfggenBackend == "git":
			logging.debug("Initializing git generator")
			from deska_server_utils.config_generators import GitGenerator
			repodir = self.cfggenOptions["cfggenGitRepo"]
			if repodir is None:
				raise ValueError, "cfggenGitRepo is None"
			workdir = self.cfggenOptions["cfggenGitWorkdir"]
			if workdir is None:
				raise ValueError, "cfggenGitWorkdir is None"
			workdir = workdir + "/" + self.currentChangeset()
			if os.path.exists(workdir):
				# got to clean it up
				shutil.rmtree(workdir)
			scriptdir = self.cfggenOptions["cfggenScriptPath"]
			self.cfgGenerator = GitGenerator(repodir, workdir, scriptdir)
		elif self.cfggenBackend == "fake":
			from deska_server_utils.config_generators import NullGenerator
			self.cfgGenerator = NullGenerator(behavior=None)
			logging.debug("No generators configured, will silently do nothing upon request")
		else:
			# no configuration generator has been configured
			from deska_server_utils.config_generators import NullGenerator
			self.cfgGenerator = NullGenerator(behavior=NotImplementedError("Attempted to access configuration generators which haven't been configured yet"))
			logging.debug("No generators configured, will raise error upon use")

	def cfgRegenerate(self):
		logging.debug("cfgRegenerate")
		logging.debug(" opening repository")
		self.cfgGenerator.openRepo()
		logging.debug(" calling cfgGenerator.generate")
		# The generators require a consistent state of the database. We are
		# supposed to be attached to an active changeset which is furthermore
		# locked. Let's hope this is really the case.
		self.cfgGenerator.generate(self)
		logging.debug("cfgRegenerate: done")


	def executeScript(self, script, workdir):
		# setup the environment and pipes for IO
		env = os.environ
		(remote_reading, writing) = os.pipe()
		(reading, remote_writing) = os.pipe()
		env["DESKA_VIA_FD_R"] = str(remote_reading)
		env["DESKA_VIA_FD_W"] = str(remote_writing)

		# launch the process
		proc = subprocess.Popen([script], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=workdir, env=env)

		# close our end of the pipes
		os.close(remote_reading)
		os.close(remote_writing)

		# prepare Python wrappers over the pipes
		rfile = os.fdopen(reading, "rb")
		wfile = os.fdopen(writing, "wb")
		# communicate
		while True:
			try:
				perform_io(self, rfile, wfile)
			except StopIteration:
				break
		# and wait for the corpse to appear
		(stdout, stderr) = proc.communicate()
		if proc.returncode:
			raise RuntimeError, "Child process %s exited with state %d.\nStdout: %s\n\nStderr: %s\n" % (script, proc.returncode, stdout, stderr)
		# that's all, baby!

	def cfgPushToScm(self, message):
		self.cfgGenerator.apiSave(message)

	def cfgGetDiff(self):
		return self.cfgGenerator.diff()

	def showConfigDiff(self, name, tag, forceRegen):
		logging.debug("showConfigDiff")
		response = {"response": name, "tag": tag}
		self.lockCurrentChangeset()
		self.initCfgGenerator()
		if forceRegen or not self.changesetHasFreshConfig():
			logging.debug("about to regenerate config")
			self.cfgRegenerate()
			self.markChangesetFresh()
		response[name] = self.cfgGetDiff()
		self.unlockCurrentChangeset()
		return json.dumps(response)

	def commitConfig(self, name, args, tag):
		self.checkFunctionArguments(name, args, tag)
		self.lockCurrentChangeset()
		self.initCfgGenerator()
		if not self.changesetHasFreshConfig():
			self.cfgRegenerate()
			self.markChangesetFresh()
		try:
			res = self.runDBFunction(name,args,tag)
			self.cfgPushToScm(args["commitMessage"])
		except Exception, e:
			'''Unexpected error in db or cfgPushToScm...'''
			self.db.rollback()
			self.unlockCurrentChangeset()
			return self.errorJson(name, tag, str(e))
		if "dbException" in res:
			'''Or regular error in db response'''
			self.db.rollback()
			self.unlockCurrentChangeset()
			return res
		self.db.commit()
		# The lock is still held even after a commit. No other sessions is
		# usually expected to try to obtain it, but it still won't hurt to
		# release the lock explicitly. However, the DB code tries to determine
		# the current changeset from the DB after we've commited the changeset,
		# and therefore correctly reports back that "there's no current
		# changeset". This unfortunately leads to leaving dangling locks behind.
		# A correct fix would be to explicitly unlock stuff from inside the DB
		# when commiting.
		return res

	def run(self,name,args):
		logging.debug("start run method(%s, %s)" % (name, args))

		if "tag" not in args:
			return self.errorJson(name, None, "Missing 'tag'!")
		tag = args["tag"]

		# this two spectial commands handle db transactions
		if name in set(["freezeView","unFreezeView"]):
			try:
				return self.freezeUnfreeze(name,tag)
			except FreezingError, e:
				return self.errorJson(name, tag, str(e), type="FreezingError")

		if name == "showConfigDiff":
			forceRegen = False
			if args.has_key("forceRegen") and args["forceRegen"] == True:
				forceRegen = True
			return self.showConfigDiff(name, tag, forceRegen)

		# applyBatchedChanges
		if name == "applyBatchedChanges":
			if "modifications" not in args:
				return self.errorJson(name, tag, "Missing 'modifications'!")
			return self.applyBatchedChanges(args["modifications"],tag)

		if name == "commitChangeset":
			# this one is special, it has to commit to the DB *and* push to SCM
			return self.commitConfig(name, args, tag)

		if name == "lockCurrentChangeset":
			self.lockCurrentChangeset()
			return self.responseJson(name,tag)
		elif name == "unlockCurrentChangeset":
			self.unlockCurrentChangeset()
			return self.responseJson(name,tag)

		return self.standaloneRunDbFunction(name, args, tag)

	def standaloneRunDbFunction(self, name, args, tag):
		'''Run stored procedure'''
		try:
			try:
				data = self.runDBFunction(name,args,tag)
			finally:
				self.endTransaction()
			return data
		except FreezingError, e:
			return self.errorJson(name, tag, str(e), type="FreezingError")
		except Exception, e:
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
		'''Check args, sort them and call db proc'''
		self.checkFunctionArguments(name, args, tag)
		needed_args = self.methods[name]

		# sort args
		args = [args[i] for i in needed_args]
		# cast to string
		args = self.utf2str(args)

		for i in range(len(args)):
			if type(args[i]) == dict:
				args[i] = json.dumps(args[i])
		return self.callProc(name,args)

	def callProc(self,name,args):
		'''Call the db function'''
		if self.freeze:
			'''check function is read only'''
			if name in self.writeFunctions:
				logging.debug("Exception when call db function in freeze view: %s" % name)
				raise FreezingError("Cannot run '%s' function, while you are in freeze (read only) mode." % name)
		try:
			self.mark.callproc(name,args)
			data = self.mark.fetchall()[0][0]
		except Exception, e:
			logging.debug("Exception when call db function: %s)" % str(e))
			raise

		logging.debug("fetchall returning: %s" % data)
		return data


