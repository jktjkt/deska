#!/usr/bin/python

from jsonparser import CommandParser
from dbapi import DB
import sys
import logging
LOG_FILENAME = 'deska_server.log'
logging.basicConfig(filename=LOG_FILENAME,level=logging.DEBUG)


logging.debug("starting deska server")

try:
	db = DB("deska_dev")
except Exception, e:
	print e
	sys.exit()
logging.debug("conected to database")

while True:
	line = sys.stdin.readline()
	logging.debug("read data {d}".format(d = line))
	if not line:
		break
	jsn = CommandParser(line)
	fn = jsn.getfn()
	args = jsn.getargs()
	sys.stdout.write(db.run(fn,args) + "\n")
	sys.stdout.flush()

