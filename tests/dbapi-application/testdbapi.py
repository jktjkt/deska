import sys
import subprocess
import os
import select
import unittest
import json
import apiUtils

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

def resolvePlaceholders(stuff):
    '''Replace placeholder values with real data valid at this point'''
    if isinstance(stuff, dict):
        return dict(map(resolvePlaceholders, stuff.iteritems()))
    elif isinstance(stuff, (list, tuple)):
        return type(stuff)(map(resolvePlaceholders, stuff))
    elif isinstance(stuff, apiUtils.Variable):
        return resolvePlaceholders(stuff.get())
    else:
        return stuff

class JsonApiTester(unittest.TestCase):
    """Verify that the interaction with the Deska server over JSON works as expected"""

    def setUp(self):
        """Start the process"""
        self.cmd = [SERVER_PATH, "-d", DBNAME, "-U", DBUSER]
        self.p = subprocess.Popen(self.cmd, stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    def testCommunication(self):
        """Iterate over the recorded communication and verify that we get the same results back"""
        for items in j:
            json.dump(items.command, self.p.stdin)
            self.p.stdin.write("\n")
            self.p.stdin.flush()
            status = select.select((self.p.stdout, self.p.stderr), (), ())
            if (self.p.stderr in status[0]):
                err = self.p.stderr.read()
                raise Exception(err)
            readJson = self.p.stdout.readline()
            print readJson
            sys.stdout.flush()
            output = json.loads(readJson)
            self.assertEqual(deunicodeify(output), items.response)

if __name__ == "__main__":
    # usage: testdbapi.py /path/to/deska_server.py testcase
    SERVER_PATH = sys.argv[1]
    DBNAME = os.environ["DESKA_DB"]
    DBUSER = os.environ["DESKA_USER"]
    TESTCASE = sys.argv[2]
    j = __import__(TESTCASE).j
    JsonApiTester.testCommunication.__func__.__doc__ = TESTCASE
    suite = unittest.TestLoader().loadTestsFromTestCase(JsonApiTester)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    sys.exit(not result.wasSuccessful())
