import psycopg2
import datetime
import logging


LOG_FILENAME = 'deska_server.log'
logging.basicConfig(filename=LOG_FILENAME,level=logging.DEBUG)

conn = psycopg2.connect("dbname='deska_dev'");

class Result:
	def __init__(self,db_mark):
		self.mark = db_mark
		self.res = db_mark.fetchall()
	def parse(self):
		return self.res[0][0]

class BoolResult(Result):
	def __init__(self,db_mark):
		self.mark = db_mark
		self.res = db_mark.fetchall()
	def parse(self):
		return bool(self.res[0][0])

class VectorResult(Result):
	def parse(self):
		vector = list()
		for i in self.res:
			vector.append(i[0])
		return vector 

class TupleResult(Result):
	def parse(self):
		res_tuple = dict()
		if not self.res:
			return res_tuple
                args = list()
                # select attribute names
                for i in self.mark.description:
                        args.append(i[0])
		# work with list instead of tuple
		res = list(self.res[0])
		map(res_tuple.__setitem__,args,list(res))
		return res_tuple

class KindAttributesResult(TupleResult):
	type_dict = ({
		"int8": "int",
		"int4": "int",
		"text": "string",
		"bpchar": "string",
		"date": "string",
		"macaddr": "string",
		"inet": "string"
	})
	def parse(self):
		res_tuple = dict()
		for i in self.res:
			# FIXME try if it is not in dict
			res_tuple[i[0]] = self.type_dict[i[1]]
		return res_tuple


class MatrixResult(Result):
	def parse(self):
                res = list()
                args = list()
                # select attribute names
                for i in self.mark.description:
                        args.append(i[0])
                for i in self.res:
                        # for each line of result
                        # add tuple argname:value into dictionary
                        line = dict()
			# work with list instead of tuple
			i = list(i)
			# FIXME: cli don't understand datetime
			for att in range(len(i)):
				if type(i[att]) == datetime.datetime:
					i[att] = str(i[att])
                        map(line.__setitem__,args,i)
			# FIXME: temporary workaround
			# If api doesnot take it
			# we have to fake it >:-)
			if line.has_key('relation'):
				del line['refattname']	
				del line['attname']	
				line['into'] = line['refkind']
				del line['refkind']
				if line['relation'] == 'INVALID':
					return list()
				if line['relation'] == 'EMBED':
					line['relation'] = 'EMBED_INTO'

                        res.append(line)
                return res
		

class DB:
	methods = dict({
		"kindNames": [VectorResult],
		"kindAttributes": [KindAttributesResult,"kindName"],
		"kindRelations": [MatrixResult,"kindName"],
		"kindInstances": [VectorResult,"kindName"],
		"deleteObject": [BoolResult,"kindName","objectName"],
		"createObject": [BoolResult,"kindName","objectName"],
		"renameObject": [BoolResult,"kindName","oldName","newName"],
		"removeAttribute": [BoolResult,"kindName","objectName","attributeName"],
		"setAttribute": [BoolResult,"kindName","objectName","attributeName","Value"],
		"startChangeset": [Result],
		"commitChangeset": [Result],
		#"rebaseChangeset": [Result],
		"resumeChangeset": [Result,"revision"],
		"detachFromCurrentChangeset": [Result,"message"],
		"abortCurrentChangeset": [Result],
		"pendingChangesets": [MatrixResult]
	})
	data_methods = dict({
		"objectData": [TupleResult,"kindName", "objectName"]
		#"resolvedObjectData": [MatrixResult,"kindName", "objectName"],
		#"findOverriddenAttrs": [MatrixResult,"kindName", "objectName", "attributeName"],
		#"findNonOverriddenAttrs": [MatrixResult,"kindName", "objectName", "attributeName"],
	})
	def __init__(self):
		self.mark = conn.cursor()
		self.mark.execute("SET search_path TO api,genproc,history,deska,production;")


	def run_method(self,name,args):
		logging.debug("start run_method({n}, {a})".format(n = name, a = args))
		# copy needed args from command definition
		needed_args = self.methods[name][:]
		self.fetch_class = needed_args[0]
		del needed_args[0]
		# have we the exact needed arguments
		if set(needed_args) != set(args.keys()):
			raise Exception("run_method: args are not good")
		# sort args
		args = [args[i] for i in needed_args]
		# cast to string
		args = map(str,args)
		self.mark.callproc(name,args)
		return 

	def run_data_method(self,name,args):
		logging.debug("start run_data_method({n}, {a})".format(n = name, a = args))
		needed_args = self.data_methods[name][:]
		self.fetch_class = needed_args[0]
		del needed_args[0]
		if set(needed_args) != set(args.keys()):
			raise Exception("run_data_method: args are not good")
		# sort args
		args = [args[i] for i in needed_args]

		# FIXME:
		# this code is provisorium, rewrite needed
		# this is similar to wrapper, get this into wrapper, or wrapper place here
		fname = args[0] + "_get_data"
		del args[0]
		# cast to string
		args = map(str,args)
		self.mark.callproc(fname, args)
		return 

	def run(self,name,args):
		if self.methods.has_key(name):
			return self.run_method(name,args)
		elif self.data_methods.has_key(name):
			return self.run_data_method(name,args)
		else:
			raise Exception("very bad assert here, not run this function without run has() before")
	
	def fetchall(self):
		cls = self.fetch_class(self.mark)
		data = cls.parse()
		logging.debug("fetchall returning: {d})".format(d = data))
		return data

	def commit(self):
		conn.commit()









