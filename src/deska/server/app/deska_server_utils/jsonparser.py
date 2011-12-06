try:
    import json
except ImportError:
    import simplejson as json

import sys
import traceback
import time
import logging

CMD = "command"
ERR = "dbException"

last_t = None

def perform_io(db, stdin, stdout, ioTracer=None):
	try:
		line = stdin.readline()
		t = time.time()
		global last_t
		if ioTracer is not None:
			if last_t is not None:
				ioTracer.debug(t - last_t, extra={"deska_direction":"D"})
			ioTracer.debug(line.strip(), extra={"deska_direction":"R"})
		logging.debug("read data %s" % line)
		if not line:
			raise StopIteration
		jsn = CommandParser(line)
		fn = jsn.getfn()
		args = jsn.getargs()
		res = db.run(fn,args)
		stdout.write(res + "\n")
		stdout.flush()
		end = time.time()
		last_t = end
		if ioTracer is not None:
			ioTracer.debug(res, extra={"deska_direction":"W"})
			ioTracer.debug(end - t, extra={"deska_direction":"T"})
		logging.debug("perform_io: command took %ss" % (end - t))
	except StopIteration:
		raise
	except Exception, e:
		exc_type, exc_value, exc_traceback = sys.exc_info()
		jsonErr = { "dbException": {"type": "ServerError", "message": repr(
			traceback.format_exception(exc_type, exc_value, exc_traceback)
		) } }
		res = json.dumps(jsonErr)
		stdout.write(res + "\n")
		stdout.flush()
		end = time.time()
		last_t = end
		if ioTracer is not None:
			ioTracer.debug(res, extra={"deska_direction":"W"})
			ioTracer.debug(end - t, extra={"deska_direction":"T"})
		logging.debug("perform_io: command took %ss" % (end - t))
		raise

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

