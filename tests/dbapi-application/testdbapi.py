import sys
import subprocess
import unittest
import json

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

class JsonApiTester(unittest.TestCase):
    """Verify that the interaction with the Deska server over JSON works as expected"""

    def setUp(self):
        """Start the process"""
        self.cmd = [SERVER_PATH, "-d", DBNAME]
        self.p = subprocess.Popen(self.cmd, stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, stderr=sys.stderr)

    def testCommunication(self):
        """Iterate over the recorded communication and verify that we get the same results back"""
        for items in j:
            json.dump(items[0], self.p.stdin)
            self.p.stdin.write("\n")
            self.p.stdin.flush()
            output = json.loads(self.p.stdout.readline())
            self.assertEqual(deunicodeify(output), items[1])

if __name__ == "__main__":
    # usage: testdbapi.py /path/to/deska_server.py dbname testcase
    SERVER_PATH = sys.argv[1]
    DBNAME = sys.argv[2]
    TESTCASE = sys.argv[3]
    j = __import__(TESTCASE).j
    JsonApiTester.testCommunication.__func__.__doc__ = TESTCASE
    suite = unittest.TestLoader().loadTestsFromTestCase(JsonApiTester)
    unittest.TextTestRunner(verbosity=2).run(suite)
