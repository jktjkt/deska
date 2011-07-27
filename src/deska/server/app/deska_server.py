#!/usr/bin/python

from jsonparser import CommandParser
from dbapi import DB
import sys
import logging
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-d", "--database", dest="database", default="deska_dev",
                  help="Name of the database to use", metavar="DB")
parser.add_option("-U", "--username", dest="username",
                  help="User to connect to the DB", metavar="USER")
parser.add_option("--logfile", dest="logfile", default="deska_server.log",
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

dbargs = {}
if options.database:
    dbargs["database"] = options.database
if options.username:
    dbargs["user"] = options.username

db = DB(**dbargs)
logging.debug("conected to database")

while True:
	line = sys.stdin.readline()
	logging.debug("read data %s" % line)
	if not line:
		break
	jsn = CommandParser(line)
	fn = jsn.getfn()
	args = jsn.getargs()
	sys.stdout.write(db.run(fn,args) + "\n")
	sys.stdout.flush()

