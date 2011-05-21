import psycopg2
import datetime
import time
import logging


LOG_FILENAME = 'deska_server.log'
logging.basicConfig(filename=LOG_FILENAME,level=logging.DEBUG)

class DB:
	methods = dict({
		"kindNames": [],
                "kindAttributes": ["kindName"],
                "kindRelations": ["kindName"],
                "kindInstances": ["kindName"],
                "deleteObject": ["kindName","objectName"],
                "createObject": ["kindName","objectName"],
                "renameObject": ["kindName","oldName","newName"],
                "setAttribute": ["kindName","objectName","attributeName","attributeData"],
                "startChangeset": [],
                "commitChangeset": ["commitMessage"],
                "rebaseChangeset": [],
                "pendingChangesets": [],
                "resumeChangeset": ["revision"],
                "detachFromCurrentChangeset": ["message"],
                "abortCurrentChangeset": [],
		"dataDifference": ["a", "b"],
		"objectData": ["kindName", "objectName"],
		"listVersions": []
	})

	def __init__(self,dbname):
		try:
			self.db = psycopg2.connect(database=dbname);
			self.mark = self.db.cursor()
			self.mark.execute("SET search_path TO jsn,api,genproc,history,deska,versioning,production;")
		except Exception, e:
			raise


	def run(self,name,args):
		logging.debug("start run method({n}, {a})".format(n = name, a = args))
		# copy needed args from command definition
		needed_args = self.methods[name][:]
		# have we the exact needed arguments
		if set(needed_args) != set(args.keys()):
			raise Exception("run_method: args are not good: {0} vs {1}".format(needed_args,args.keys()))
		# sort args
		args = [args[i] for i in needed_args]
		# cast to string
		args = map(str,args)
		try:
			self.mark.callproc(name,args)
			data = self.mark.fetchall()[0][0]
		except Exception, e:
			raise

		logging.debug("fetchall returning: {d})".format(d = data))
		self.db.commit()
		return data


