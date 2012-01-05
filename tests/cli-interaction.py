import sys
import subprocess
import select
import os
import unittest

# enable "import libLowLevelPyDeska"
path_libLowLevelPyDeska = os.path.abspath(os.getcwd() + "../../..")
sys.path.append(path_libLowLevelPyDeska)
# enable "import deska"
path_deska = os.path.abspath(os.environ["DESKA_SOURCES"] + "/src/deska/python")
sys.path.append(path_deska)
path_deska_server_bin = os.path.abspath(os.environ["DESKA_SOURCES"] +
                                       "/src/deska/server/app/deska-server")

class TestCliInteraction(unittest.TestCase):
    def setUp(self):
        DBNAME = os.environ["DESKA_DB"]
        DBUSER = os.environ["DESKA_USER"]
        TESTCASE = os.environ["DESKA_TESTCASE"]
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

        self.tdata = file("%s/%s" % (os.path.abspath(os.environ["DESKA_SOURCES"] + "/tests/cli-interaction/"), TESTCASE), "rb")
        cmd = ["../../deska-cli", "--DBConnection.Server", path_deska_server_bin, "--CLI.HistoryFilename", "/dev/null",
               "--execute", "-"]
        self.p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    def expect_stdin(self, expected_rdata):
        readSoFar = ""
        while True:
            status = select.select((self.p.stdout, self.p.stderr), (), (), 3)
            if (self.p.stderr in status[0]):
                err = os.read(self.p.stderr.fileno(), 65536)
                raise RuntimeError, "stderr: %s" % err
            elif status[0] == []:
                print "Read so far (%d): '%s'" % (len(readSoFar), readSoFar)
                print "Expected to read (%d): '%s'" % (len(expected_rdata), expected_rdata)
                raise RuntimeError, "tester: select() timed out, CLI stuck?"
            else:
                toRead = len(expected_rdata) - len(readSoFar)
                self.assertTrue(toRead > 0)
                readSoFar = readSoFar + os.read(self.p.stdout.fileno(), toRead)
                if len(readSoFar) == len(expected_rdata):
                    print "R %s" % readSoFar
                    self.assertEqual(readSoFar, expected_rdata)
                    break
                else:
                    continue

    def test_interaction(self):
        mode = "io"
        for line in self.tdata:
            if mode == "io":
                if line.startswith("#") or line == "\n":
                    continue
                elif line.startswith("w "):
                    print line
                    self.p.stdin.write(line[2:])
                    self.p.stdin.flush()
                elif line.startswith("r "):
                    self.expect_stdin(line[2:])
                elif line == "<dump>\n":
                    print "<dump>"
                    mode = "dump"
                    self.p.stdin.write("dump\n")
                    self.p.stdin.flush()
                else:
                    raise RuntimeError("Unexpected test data in IO mode: %s" % line)
            elif mode == "dump":
                if line == "</dump>\n":
                    print "</dump>"
                    mode = "io"
                else:
                    self.expect_stdin(line)
            else:
                r.assertFalse("Invalid state: %s" % mode)
        self.p.stdin.close()
        self.expect_stdin("All commands successfully executed.")
        print "Everything went OK"

if __name__ == "__main__":
    unittest.main()
