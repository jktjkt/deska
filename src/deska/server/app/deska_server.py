#!/usr/bin/python

from apijson import Jsn
import sys
import logging
LOG_FILENAME = 'deska_server.log'
logging.basicConfig(filename=LOG_FILENAME,level=logging.DEBUG)


logging.debug("starting deska server")
while True:
	line = sys.stdin.readline()
	logging.debug("read data {d}".format(d = line))
	if not line:
		break
	jsn = Jsn(line)
	sys.stdout.write(jsn.process() + "\n")
	sys.stdout.flush()

