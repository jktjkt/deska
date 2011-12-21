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
from deska_server_utils.config_generators import GeneratorError

class FreezingError(Exception):
    pass

class DeskaException(Exception):
	'''Exception class for deska exceptions from jsn'''
	def __init__(self, src):
		self.src = src

	def json(self,command,tag):
		jsn = {"response": command,
			"dbException": self.src,
			"tag": tag
		}
		return json.dumps(jsn)


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
		"lockCurrentChangeset": ["tag"],
		"unlockCurrentChangeset": ["tag"],
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
		v = psycopg2.__version__.split(" ")[0]
		splitted = [int (x) for x in v.split(".")]
		# Unfortunately, setuptools might not be available at this point, so we
		# cannot use their version_parse comparator
		if splitted[0] > 2:
			# we have no idea what to do here, so assume forward compatibility
			self.psycopg2_use_new_isolation = True
		elif splitted[0] < 2:
			raise RuntimeError, "psycopg2 version %s looks ancient, sorry" % v
		elif splitted[0] == 2 and splitted[1] < 4:
			self.psycopg2_use_new_isolation = False
		elif splitted[0] == 2 and splitted[1] == 4 and splitted[2] < 2:
			# The 2.4.1 fails when used in the old way
			self.psycopg2_use_new_isolation = False
		else:
			self.psycopg2_use_new_isolation = True
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

	def transaction_isolate(self):
		"""Set an isolation level between concurrent sessions"""
		# commit and start new transaction with selected properties
		if self.psycopg2_use_new_isolation:
			self.db.commit()
			self.db.set_session(isolation_level=psycopg2.extensions.ISOLATION_LEVEL_SERIALIZABLE, readonly=True)
		else:
			self.db.set_isolation_level(2)
			self.db.commit()

	def transaction_shared(self):
		"""Restore the transaction isolation to a default level"""
		# set isolation level readCommited
		if self.psycopg2_use_new_isolation:
			self.db.commit()
			self.db.set_session(isolation_level=psycopg2.extensions.ISOLATION_LEVEL_READ_COMMITTED, readonly=False)
		else:
			self.db.set_isolation_level(1)
			self.db.commit()


	def freezeUnfreeze(self,name,tag):
		if name == "freezeView":
			# set isolation level serializable, and read only transaction

			# try if there is changeset attached
			try:
				changeset = self.currentChangeset()
			except:
				changeset = None
			if changeset is not None:
				raise FreezingError("Cannot run freezeView, changeset tmp%s is attached." % changeset)
			self.transaction_isolate()
			self.freeze = True
			return self.responseJson(name,tag)
		elif name == "unFreezeView":
			if not self.freeze:
				raise FreezingError("Cannot call unFreezeView, view is not frozen")
			self.transaction_shared()
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

	def noJsonCallProc(self,name):
		'''Run normal json function, but this raises DeskaException and not returns json'''
		ret = json.loads(self.callProc(name,["TAG"]))
		if "dbException" in ret:
			raise DeskaException(ret["dbException"])
		if ret["response"] in ret:
			return ret[ret["response"]]
		pass

	def lockCurrentChangeset(self):
		'''Lock changeset by db lock'''
		self.noJsonCallProc("lockCurrentChangeset")
		pass

	def unlockCurrentChangeset(self):
		'''Unlock changeset'''
		self.noJsonCallProc("unlockCurrentChangeset")
		pass

	def changesetHasFreshConfig(self):
		'''return true if changeset has fresh configuration generated'''
		res = self.noJsonCallProc("changesetHasFreshConfig")
		logging.debug("changesetHasFreshConfig returns %s" % res)
		return res

	def markChangesetFresh(self):
		'''mark changeset as fresh (has fresh configuration generated'''
		self.noJsonCallProc("markChangesetFresh")
		logging.debug("markChangesetFresh")

	def currentChangeset(self):
		'''get name of the current changeset from the DB'''
		return self.noJsonCallProc("getCurrentChangeset")

	def initCfgGenerator(self):
		logging.debug("initCfgGenerator")
		if self.cfggenBackend == "git":
			logging.debug(" Initializing git generator")
			from deska_server_utils.config_generators import GitGenerator
			repodir = self.cfggenOptions["cfggenGitRepo"]
			if repodir is None:
				raise ValueError, "cfggenGitRepo is None"
			workdir = self.cfggenOptions["cfggenGitWorkdir"]
			if workdir is None:
				raise ValueError, "cfggenGitWorkdir is None"
			workdir = workdir + "/" + self.currentChangeset()
			scriptdir = self.cfggenOptions["cfggenScriptPath"]
			logging.debug(" cfggen: initializing Gitgenerator(%s, %s, %s)" % (repodir, workdir, scriptdir))
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
		startTime = time.time()
		env = os.environ
		(remote_reading, writing) = os.pipe()
		(reading, remote_writing) = os.pipe()
		env["DESKA_VIA_FD_R"] = str(remote_reading)
		env["DESKA_VIA_FD_W"] = str(remote_writing)

		# launch the process
		logging.debug("executeScript: starting %s (cwd=%s)" % (script, workdir))
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
				logging.debug("executeScript: iteration of the DBAPI loop")
				perform_io(self, rfile, wfile)
			except StopIteration:
				break
		# and wait for the corpse to appear
		(stdout, stderr) = proc.communicate()
		logging.debug("executeScript: time spent in executing: %ss" % (time.time() - startTime))
		if proc.returncode:
			raise RuntimeError, "Child process %s exited with state %d.\nStdout: %s\n\nStderr: %s\n" % (script, proc.returncode, stdout, stderr)
		# that's all, baby!

	def cfgPushToScm(self, message):
		self.cfgGenerator.apiSave(message)

	def cfgGetDiff(self):
		return self.cfgGenerator.diff()

	def showConfigDiff(self, name, tag, forceRegen):
		logging.debug("showConfigDiff forceRegen=%s", forceRegen)
		response = {"response": name, "tag": tag}
		try:
			try:
				self.lockCurrentChangeset()
				self.initCfgGenerator()
				if forceRegen or not self.changesetHasFreshConfig():
					logging.debug(" about to regenerate config")
					self.cfgRegenerate()
					self.markChangesetFresh()
				else:
					logging.debug(" configuration was fresh already")
				response[name] = self.cfgGetDiff()
			finally:
				self.unlockCurrentChangeset()
		except GeneratorError, e:
			return self.errorJson(name, tag, str(e), "CfgGeneratingError")
		return json.dumps(response)

	def commitConfig(self, name, args, tag):
		self.checkFunctionArguments(name, args, tag)
		self.lockCurrentChangeset()
		self.initCfgGenerator()
		try:
			if not self.changesetHasFreshConfig():
				self.cfgRegenerate()
				self.markChangesetFresh()
			res = self.runDBFunction(name,args,tag)
			self.cfgPushToScm(args["commitMessage"])
			self.cfgGenerator.nukeWorkDir()
		except GeneratorError, e:
			self.db.rollback()
			self.unlockCurrentChangeset()
			return self.errorJson(name, tag, str(e), "CfgGeneratingError")
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
			if args.has_key("forceRegenerate") and args["forceRegenerate"] == True:
				forceRegen = True
			try:
				return self.showConfigDiff(name, tag, forceRegen)
			except DeskaException, e:
				return e.json(name,tag)

		# applyBatchedChanges
		if name == "applyBatchedChanges":
			if "modifications" not in args:
				return self.errorJson(name, tag, "Missing 'modifications'!")
			return self.applyBatchedChanges(args["modifications"],tag)

		if name == "commitChangeset":
			# this one is special, it has to commit to the DB *and* push to SCM
			try:
				return self.commitConfig(name, args, tag)
			except DeskaException, e:
				return e.json(name,tag)

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


