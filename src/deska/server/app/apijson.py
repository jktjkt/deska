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
			raise Exception("No CMD in json: " + self.jsn)

	def command(self):
		cmd = self.jsn[CMD]
		self.cmd = cmd
		if self.jsn.has_key(cmd):
			args = self.jsn[cmd]
		else:
			args = dict()
		#if self.db.has(cmd):
		#is it good if we have exceptions? try better try:-)
		res = self.db.run(cmd,args)
		return res
	
	def responce(self,res):
		data = self.db.fetchall()
		if type(data) == int:
			self.jsn["result"] = data
		elif type(data) == bool:
			self.jsn["result"] = data
		else:
			self.jsn[self.cmd] = data
		# write response instead of command
		del self.jsn[CMD]
		self.jsn[RES] = self.cmd
		return self.jsn	
		
	def process(self):
		res = self.command()
		jsn = self.responce(res)
		self.db.commit()
		return json.dumps(jsn)

