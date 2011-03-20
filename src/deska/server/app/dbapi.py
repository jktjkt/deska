import psycopg2

class DB:
	methods = dict({
		"kindNames": [],
		"kindAttributes": ["kindName"],
		"kindRelations": ["kindName"],
		"kindInstances": ["kindName"],
		"objectData": ["kindName", "objectName"],
		"setAttribute": ["kindName","objectName","attributeName","Value"]
	
	})
	def __init__(self):
		print "conntect to db"
		conn = psycopg2.connect("dbname='deska_dev' user='deska' host='localhost' password='deska'");
		self.mark = conn.cursor()
		self.mark.execute("SET search_path TO api,genproc,history,deska,production;")

	# has api this method?
	def has(self,name):
		return self.methods.has_key(name)

	def run(self,name,args):
		# have we the exact needed arguments
		if set(self.methods[name]) != set(args.keys()):
			raise "args are not good"
		# sort args 
		args = [args[i] for i in self.methods[name]]
		# cast to string
		args = map(str,args)
		self.mark.callproc(name,args)
		print self.mark.statusmessage
		print self.mark.query
		self.res = self.mark.fetchall()
		return len(self.res)*len(self.res[0])
	
	def fetchall(self):
		print type(self.res)
		d = dict()
		for i in self.res:
			d[i[0]] = i[1]
		return d










