import sys
import os
import logging
import errno
from optparse import OptionParser
try:
    import json
except ImportError:
    import simplejson as json
from deska_server_utils.jsonparser import perform_io
from deska_server_utils.dbapi import DB

class StreamLogger(object):
    """Fake the stream interface and write stuff into the logging module instead"""

    def write(self, s):
        logging.debug(s)

def run():
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
    # file the variables from environment
    if not parser.has_option("database") and os.environ.has_key("DESKA_DB"):
        options.database = os.environ["DESKA_DB"]
    if not parser.has_option("username") and os.environ.has_key("DESKA_USER"):
        options.username = os.environ["DESKA_USER"]
    if not parser.has_option("cfggenBackend") and os.environ.has_key("DESKA_CFGGEN_BACKEND"):
        options.cfggenBackend = os.environ["DESKA_CFGGEN_BACKEND"]
    if not parser.has_option("cfggenScriptPath") and os.environ.has_key("DESKA_CFGGEN_SCRIPTS"):
        options.cfggenScriptPath = os.environ["DESKA_CFGGEN_SCRIPTS"]
    if not parser.has_option("cfggenGitRepo") and os.environ.has_key("DESKA_CFGGEN_GIT_PRIMARY_CLONE"):
        options.cfggenGitRepo = os.environ["DESKA_CFGGEN_GIT_PRIMARY_CLONE"]
    if not parser.has_option("cfggenGitWorkdir") and os.environ.has_key("DESKA_CFGGEN_GIT_WC"):
        options.cfggenGitWorkdir = os.environ["DESKA_CFGGEN_GIT_WC"]
    if (options.log_stderr and options.logfile):
        # basicConfig() won't add duplicate loggers
        parser.error("Cannot log to both file and stderr -- too lazy")
    if options.cfggenBackend not in ("error", "fake", "git"):
        parser.error("Unsupported backend for configuration generators")

    logformat_pid = "%(levelname)s:%(name)s:%(process)s:%(message)s"
    if options.logfile:
        logging.basicConfig(filename = options.logfile, level=logging.DEBUG, format=logformat_pid)
    elif options.log_stderr:
        logging.basicConfig(stream = sys.stderr, level=logging.DEBUG, format=logformat_pid)
    else:
        logging.basicConfig(stream = sys.stderr, level=logging.CRITICAL, format=logformat_pid)

    logging.debug("starting deska server")

    dbargs = {}
    if options.database:
        dbargs["database"] = options.database
    if options.username:
        dbargs["user"] = options.username

    # Redirecting the stdout to stderr. This is required in order to support the
    # GIT_PYTHON_TRACE which blindly uses `print` for its debug output (and
    # clobbers our precious DBAPI communication that way).
    # Do NOT ever try to capture stderr this way, this will lead to infinite loops.
    orig_stdout = sys.stdout
    sys.stdout = StreamLogger()

    try:
        # Make sure that Ctrl-C on the remote side won't ever propagate to us, so that
        # we don't have to deal with KeyboardInterrupt exception
        os.setsid()
    except OSError, e:
        if e.errno == errno.EPERM:
            # we're already a session leader -> do nothing
            pass
        else:
            raise

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
        orig_stdout.write(json.dumps(response))
        orig_stdout.write("\n")
        orig_stdout.flush()
        sys.exit(1)

    logging.debug("connected to database")

    ioTracer = None
    if os.environ.has_key("DESKA_SERVER_IO_TRACE"):
        ioTracer = logging.getLogger("deska-io-tracer")
        ioTracer.propagate = False
        ioTracer.setLevel(logging.DEBUG)
        formatter = logging.Formatter("%(deska_direction)s%(message)s")
        handler = logging.FileHandler("deska_io_log", "w")
        handler.setFormatter(formatter)
        ioTracer.addHandler(handler)

    while True:
        try:
            perform_io(db, sys.stdin, orig_stdout, ioTracer=ioTracer)
        except StopIteration:
            break
