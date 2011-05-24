import json

import logging

CMD = "command"
ERR = "dbException"

class CommandParser:
	# dict of commands
	# create json object from string in data
	def __init__(self,data):
		logging.debug("loading data {d}".format(d = data))
		self.jsn = json.loads(data)
		# if it does not have CMD, error
		if CMD not in self.jsn:
			raise Exception("No CMD in json: " + self.jsn)

	def getfn(self):
		cmd = self.jsn[CMD]
		logging.debug("function name: " + cmd)
		return self.jsn[CMD]

	def getargs(self):
		args = self.jsn.copy()
		del args[CMD]
		logging.debug("arguments are {a}".format(a = args))
		return args

