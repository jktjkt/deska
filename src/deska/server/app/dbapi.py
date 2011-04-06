import psycopg2

conn = psycopg2.connect("dbname='deska_dev'");

class DB:
	methods = dict({
		"kindNames": [],
		"kindAttributes": ["kindName"],
		"kindRelations": ["kindName"],
		"kindInstances": ["kindName"],
		"deleteObject": ["kindName","objectName"],
		"createObject": ["kindName","objectName"],
		"renameObject": ["kindName","oldName","newName"],
		"removeAttribute": ["kindName","objectName","attributeName"],
		"setAttribute": ["kindName","objectName","attributeName","Value"],
		"startChangeset": [],
		"commitChangeset": [],
		#"rebaseChangeset": [],
		"resumeChangeset": ["revision"],
		"detachFromCurrentChangeset": ["message"],
		"abortCurrentChangeset": []
	})
	data_methods = dict({
		"objectData": ["kindName", "objectName"],
		#"pendingChangesets": [],
		#"resolvedObjectData": ["kindName", "objectName"],
		#"findOverriddenAttrs": ["kindName", "objectName", "attributeName"],
		#"findNonOverriddenAttrs": ["kindName", "objectName", "attributeName"],
	})
	def __init__(self):
		self.mark = conn.cursor()
		self.mark.execute("SET search_path TO api,genproc,history,deska,production;")

	# has api this method?
	def has(self,name):
		return self.methods.has_key(name) or self.data_methods.has_key(name)

	def run_method(self,name,args):
		# have we the exact needed arguments
		if set(self.methods[name]) != set(args.keys()):
			raise Exception("run_method: args are not good")
		# sort args
		args = [args[i] for i in self.methods[name]]
		# cast to string
		args = map(str,args)
		self.mark.callproc(name,args)
		self.res = self.mark.fetchall()
		return len(self.res)*len(self.res[0])

	def run_data_method(self,name,args):
		# this code is provisorium, rewrite before merge into master
		if set(self.data_methods[name]) != set(args.keys()):
			raise Exception("run_data_method: args are not good")
		# sort args
		args = [args[i] for i in self.data_methods[name]]
		# cast to string
		fname = args[0] + "_get_data"
		del args[0]
		args = map(str,args)
		self.mark.callproc(fname, args)
		self.res = self.mark.fetchall()
		return len(self.res)*len(self.res[0])

	def run(self,name,args):
		if self.methods.has_key(name):
			return self.run_method(name,args)
		elif self.data_methods.has_key(name):
			return self.run_data_method(name,args)
		else:
			raise "very bad assert here, not run this function without run has() before"

	def fetchall(self):
		res = list()
		args = list()
		# select attribute names
		for i in self.mark.description:
			args.append(i[0])
		for i in self.res:
			# for each line of result
			# add tuple argname:value into dictionary
			d = dict()
			map(d.__setitem__,args,list(i))
			res.append(d)

		return res

	def commit(self):
		conn.commit()









