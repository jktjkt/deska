import json
from dbapi import DB

db = DB()
CMD = "command"
RES = "response"

class Jsn:
	# dict of commands
	# create json object from string in data
	def __init__(self,data):
		#FIXME try and error reporting
		self.jsn = json.loads(data)
		# if it does not have CMD, error
		if CMD not in self.jsn:
			raise "No CMD in json: {jsn}".format(jsn = self.json)

	def command(self):
		cmd = self.jsn[CMD]
		self.cmd = cmd
		if self.jsn.has_key(cmd):
			args = self.jsn[cmd]
		else:
			args = dict()
		if db.has(cmd):
			res = db.run(cmd,args)
			return res
		else:
			raise "no command named" + cmd
	
	def responce(self,res):
		if res > 1:
			data = db.fetchall()	
			self.jsn[self.cmd] = data
		elif res == 1:
			self.jsn["result"] = True
		# write response instead of command
		del self.jsn[CMD]
		self.jsn[RES] = self.cmd
		return self.jsn	
		
	def process(self):
		res = self.command()
		return self.responce(res)


data = '{"command": "setAttribute", "setAttribute": {"kindName":"vendor", "objectName":"DELL", "attributeName": "name", "Value":"xxx"}}'
jsn = Jsn(data)
print jsn.process()

data = '{"command": "kindAttributes", "kindAttributes": { "kindName": "vendor"} }'
jsn = Jsn(data)
print jsn.process()
