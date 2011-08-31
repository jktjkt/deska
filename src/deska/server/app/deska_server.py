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

parser.add_option("--cfggen-backend", dest="cfggenBackend", metavar="METHOD",
                  default="error", help=
                      "Configuration method to use: error (throw errors upon "
                      "using the configuration generators), fake (just do "
                      "nothing, but do not throw errors) or git (work with a "
                      "git repository)")
parser.add_option("--cfggen-script-path", dest="cfggenScriptPath", default=None,
                  metavar="PATH", help="Path to use when looking for the actual "
                      "executables used for generating configuration")
parser.add_option("--cfggen-git-repository", dest="cfggenGitRepo", default=None,
                  metavar="PATH", help="Path of a git repository to use. Git "
                      "will call `git push` from this repository, so this cannot "
                      "be the master repo.")
parser.add_option("--cfggen-git-workdir", dest="cfggenGitWorkdir", default=None,
                  metavar="PATH", help="Path to use for storing git working "
                    "directories for each changeset")

(options, args) = parser.parse_args()
if (options.log_stderr and options.logfile):
    # basicConfig() won't add duplicate loggers
    parser.error("Cannot log to both file and stderr -- too lazy")
if options.cfggenBackend not in ("error", "fake", "git"):
    parser.error("Unsupported backend for configuration generators")

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

cfggenOptions = {"cfggenScriptPath": options.cfggenScriptPath,
                 "cfggenGitRepo": options.cfggenGitRepo,
                 "cfggenGitWorkdir": options.cfggenGitWorkdir
                }

db = DB(dbOptions=dbargs, cfggenBackend=options.cfggenBackend, cfggenOptions=cfggenOptions)
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

