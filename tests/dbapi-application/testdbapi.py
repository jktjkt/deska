import sys
import subprocess
import os
import select
import unittest
try:
    import json
except ImportError:
    import simplejson as json
import apiUtils

# enable "import libLowLevelPyDeska"
path_libLowLevelPyDeska = os.path.abspath(os.getcwd() + "../../..")
sys.path.append(path_libLowLevelPyDeska)
# enable "import deska"
path_deska = os.path.abspath(os.environ["DESKA_SOURCES"] + "/src/deska/LowLevelPyDeska")
sys.path.append(path_deska)

def deunicodeify(stuff):
    """Convert a dict or stuff like that into a dict with all strings changed into unicode"""
    # courtesy of http://stackoverflow.com/questions/1254454/fastest-way-to-convert-a-dicts-keys-values-from-unicode-to-str
    if isinstance(stuff, unicode):
        return stuff.encode("utf-8")
    elif isinstance(stuff, dict):
        return dict(map(deunicodeify, stuff.iteritems()))
    elif isinstance(stuff, (list, tuple)):
        return type(stuff)(map(deunicodeify, stuff))
    else:
        return stuff

class Connection(object):
    """Encapsulate communication to the Deska server over JSON"""

    counter = 0

    def __init__(self, cmd):
        self.cmd = cmd
        self.p = subprocess.Popen(self.cmd, stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        Connection.counter = Connection.counter + 1
        self.num = Connection.counter

    def runCommandStr(self, writeJson):
        """Handle string IO to the process"""
        self.p.stdin.write(writeJson)
        self.p.stdin.write("\n")
        self.p.stdin.flush()
        status = select.select((self.p.stdout, self.p.stderr), (), ())
        if (self.p.stderr in status[0]):
            err = os.read(self.p.stderr.fileno(), 65536)
            print err
        return self.p.stdout.readline()


class JsonApiTester(unittest.TestCase):
    """Verify that the interaction with the Deska server over JSON works as expected"""

    def setUp(self):
        """Start the process"""
        self.cmd = [SERVER_PATH, "-d", DBNAME, "-U", DBUSER, "--cfggen-backend",
                   CFGGEN_METHOD] + CFGGEN_EXTRA_OPTIONS
        self.conn = Connection(self.cmd)

    def imperativeCommands(self):
        """Run the test function which gets access to the whole environment"""
        imperative(self)

    def runJSON(self, cmd, conn):
        """Send a JSON string and return the parsed result"""
        if conn is None:
            conn = self.conn
        writeJson = json.dumps(cmd)
        print conn.num, writeJson
        readJson = conn.runCommandStr(writeJson)
        print conn.num, readJson
        sys.stdout.flush()
        return deunicodeify(json.loads(readJson))

    def c(self, command, conn=None):
        """Access the result of a command"""
        res = self.runJSON(command.command, conn)
        self.assertTrue("response" in res)
        self.assertTrue("tag" in res)
        self.assertTrue("dbException" not in res)
        self.assertTrue(command.name in res)
        return res[command.name]

    def cvoid(self, command, conn=None):
        """Execute a command, but scream loudly when no result is returned"""
        res = self.runJSON(command.command, conn)
        self.assertTrue("response" in res)
        self.assertTrue("tag" in res)
        self.assertTrue("dbException" not in res)
        self.assertTrue(command.name not in res)

    def cfail(self, command, exception=None, conn=None):
        """Make sure that the commands fails"""
        res = self.runJSON(command.command, conn)
        self.assertTrue("response" in res)
        self.assertTrue("tag" in res)
        self.assertTrue("dbException" in res)
        if exception is not None:
            self.assertEqual(res["dbException"], exception)

if __name__ == "__main__":
    # usage: testdbapi.py /path/to/deska_server.py testcase
    SERVER_PATH = sys.argv[1]
    DBNAME = os.environ["DESKA_DB"]
    DBUSER = os.environ["DESKA_USER"]
    TESTCASE = sys.argv[2]
    CFGGEN_EXTRA_OPTIONS = []
    try:
        CFGGEN_METHOD = os.environ["DESKA_CFGGEN_BACKEND"]
    except KeyError:
        CFGGEN_METHOD = "fake"
    if CFGGEN_METHOD == "git":
        CFGGEN_EXTRA_OPTIONS = ["--cfggen-script-path", os.environ["DESKA_CFGGEN_SCRIPTS"],
                                "--cfggen-git-repository", os.environ["DESKA_CFGGEN_GIT_PRIMARY_CLONE"],
                                "--cfggen-git-workdir", os.environ["DESKA_CFGGEN_GIT_WC"]]
    # massage the PYTHONPATH for the children
    if not os.environ.has_key("PYTHONPATH"):
        os.environ["PYTHONPATH"] = ":".join([path_libLowLevelPyDeska, path_deska])
    else:
        os.environ["PYTHONPATH"] = ":".join([os.environ["PYTHONPATH"], path_libLowLevelPyDeska, path_deska])
    module = __import__(TESTCASE)
    if "imperative" in dir(module):
        imperative = module.imperative
        JsonApiTester.testCase = JsonApiTester.imperativeCommands
        JsonApiTester.testCase.im_func.__doc__ = TESTCASE
    else:
        print "ERROR: No tests in the testcase"
        sys.exit(6)
    if module.__doc__ is not None:
        JsonApiTester.testCase.im_func.__doc__ = module.__doc__
    suite = unittest.TestLoader().loadTestsFromTestCase(JsonApiTester)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    sys.exit(not result.wasSuccessful())
