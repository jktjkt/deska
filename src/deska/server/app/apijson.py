import json
from dbapi import DB

CMD = "command"
RES = "response"

class Jsn:
	# dict of commands
	# create json object from string in data
	def __init__(self,data):
		#FIXME try and error reporting
		self.db = DB()
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
		if self.db.has(cmd):
			res = self.db.run(cmd,args)
			return res
		else:
			raise "no command named" + cmd
	
	def responce(self,res):
		if res > 1:
			data = self.db.fetchall()	
			self.jsn[self.cmd] = data
		elif res == 1:
			self.jsn["result"] = True
			if self.jsn.has_key(self.cmd):
				#remove unneded args
				del self.jsn[self.cmd]
		# write response instead of command
		del self.jsn[CMD]
		self.jsn[RES] = self.cmd
		return self.jsn	
		
	def process(self):
		res = self.command()
		jsn = self.responce(res)
		self.db.commit()
		return jsn

