#!/usr/bin/python

from jsonparser import perform_io
from dbapi import DB
import os
import sys
import logging
from optparse import OptionParser
try:
    import json
except ImportError:
    import simplejson as json

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

# Make sure that Ctrl-C on the remote side won't ever propagate to us, so that
# we don't have to deal with KeyboardInterrupt exception
os.setsid()

cfggenOptions = {"cfggenScriptPath": options.cfggenScriptPath,
                 "cfggenGitRepo": options.cfggenGitRepo,
                 "cfggenGitWorkdir": options.cfggenGitWorkdir
                }
try:
    db = DB(dbOptions=dbargs, cfggenBackend=options.cfggenBackend, cfggenOptions=cfggenOptions)
except Exception, e:
    msg = "Cannot connect to database: %s" % e
    logging.error(msg)
    response = {"dbException": {"type": "ServerError", "message": msg}}
    sys.stdout.write(json.dumps(response))
    sys.stdout.write("\n")
    sys.stdout.flush()
    sys.exit(1)

logging.debug("connected to database")

while True:
    try:
        perform_io(db, sys.stdin, sys.stdout)
    except StopIteration:
        break
