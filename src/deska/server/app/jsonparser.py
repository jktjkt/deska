try:
    import json
except ImportError:
    import simplejson as json

import logging

CMD = "command"
ERR = "dbException"

def perform_io(db, stdin, stdout):
	line = stdin.readline()
	logging.debug("read data %s" % line)
	if not line:
		raise StopIteration
	jsn = CommandParser(line)
	fn = jsn.getfn()
	args = jsn.getargs()
	stdout.write(db.run(fn,args) + "\n")
	stdout.flush()

class CommandParser:
	# dict of commands
	# create json object from string in data
	def __init__(self,data):
		logging.debug("loading data %s" % data)
		self.jsn = json.loads(data)
		# if it does not have CMD, error
		if CMD not in self.jsn:
			raise Exception("No CMD in json: %s" % self.jsn)

	def getfn(self):
		cmd = self.jsn[CMD]
		logging.debug("function name: " + cmd)
		return self.jsn[CMD]

	def getargs(self):
		args = self.jsn.copy()
		del args[CMD]
		logging.debug("arguments are %s" % args)
		return args

