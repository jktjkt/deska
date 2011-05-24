#!/usr/bin/python

from jsonparser import CommandParser
from dbapi import DB
import sys
import logging
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-d", "--database", dest="database", default="deska_dev",
                  help="Name of the database to use", metavar="DB")
parser.add_option("--logfile", dest="logfile",
                  help="File name of the debug log")
parser.add_option("--log-stderr", dest="log_stderr", action="store_true",
                  default=False, help="Log to standard error")

(options, args) = parser.parse_args()
if (options.log_stderr and options.logfile):
    # basicConfig() won't add duplicate loggers
    parser.error("Cannot log to both file and stderr -- too lazy")

if options.logfile:
    logging.basicConfig(filename = options.logfile, level=logging.DEBUG)
elif options.log_stderr:
    logging.basicConfig(stream = sys.stderr, level=logging.DEBUG)
else:
    logging.basicConfig(stream = sys.stderr, level=logging.CRITICAL)

logging.debug("starting deska server")

try:
	db = DB(options.database)
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

