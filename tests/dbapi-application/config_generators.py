'''Test that the server-side configuration generators work reasonably'''

import os
import sys
from apiUtils import *
import datetime
import deska
import subprocess
import tempfile
import fcntl
import select
from testdbapi import Connection, json, deunicodeify

PATH_WC = os.environ["DESKA_CFGGEN_GIT_WC"]
PATH_SECOND = os.environ["DESKA_CFGGEN_GIT_SECOND"]

def helper_check_second_clone(r, contents):
    r.assertEqual(subprocess.call(["git", "pull"], cwd=PATH_SECOND), 0)
    myfiles = []
    for (root, dirs, files) in os.walk(PATH_SECOND):
        if ".git" in dirs:
            dirs.remove(".git")
        myfiles.extend([os.path.join(root[len(PATH_SECOND):], fname) for fname in files])
    r.assertEqual(contents, myfiles)

def helperScriptName(fname):
    return "%s/%s.py" % (os.environ["DESKA_CFGGEN_SCRIPTS"], fname)

def writeGenerator(fname, script):
    fname = helperScriptName(fname)
    f = file(fname, "wb")
    f.write(script)
    f.close()
    os.chmod(fname, 0755)

def rmGenerator(fname):
    os.unlink(helperScriptName(fname))

def test_trivial(r):
    """Create a sample script that simply dumps names of all hosts to a file"""
    writeGenerator("01",
"""#!/usr/bin/env python
import deska

deska.init()

output = file("all-hosts", "wb")
hosts = deska.host[deska.host.name != "404"]
for host in sorted(hosts):
    output.write("%s\\n" % host)
""")

    # Check that no config generators have ever run, or that they were correctly
    # cleaned already
    r.assertTrue(os.path.exists(PATH_WC))
    r.assertEqual(os.listdir(os.environ["DESKA_CFGGEN_GIT_WC"]), [])
    helper_check_second_clone(r, ["README"])

    changeset = r.c(startChangeset())
    for x in range(10):
        objname = "host%d" % x
        r.assertEqual(r.c(createObject("host", objname)), objname)

    # The cfggen for this changeset has not run yet
    mytarget = os.path.join(PATH_WC, changeset, "all-hosts")
    r.assertTrue(not os.path.exists(mytarget))

    coloredDiff = r.c(showConfigDiff())
    # Check the resulting directory structure
    r.assertEqual(os.listdir(PATH_WC), [changeset])
    r.assertTrue(os.path.exists(mytarget))
    r.assertEqual(file(mytarget, "rb").readlines(), ["host%d\n" % x for x in range(10)])
    helper_check_second_clone(r, ["README"])
    print coloredDiff

    expectedColoredDiff = '\x1b[1m--- /dev/null\x1b[m\n' + \
        '\x1b[1m+++ b/all-hosts\x1b[m\n' + \
        '\x1b[36m@@ -0,0 +1,10 @@\x1b[m\n' + \
        '\x1b[32m+\x1b[m\x1b[32mhost0\x1b[m\n' + \
        '\x1b[32m+\x1b[m\x1b[32mhost1\x1b[m\n' + \
        '\x1b[32m+\x1b[m\x1b[32mhost2\x1b[m\n' + \
        '\x1b[32m+\x1b[m\x1b[32mhost3\x1b[m\n' + \
        '\x1b[32m+\x1b[m\x1b[32mhost4\x1b[m\n' + \
        '\x1b[32m+\x1b[m\x1b[32mhost5\x1b[m\n' + \
        '\x1b[32m+\x1b[m\x1b[32mhost6\x1b[m\n' + \
        '\x1b[32m+\x1b[m\x1b[32mhost7\x1b[m\n' + \
        '\x1b[32m+\x1b[m\x1b[32mhost8\x1b[m\n' + \
        '\x1b[32m+\x1b[m\x1b[32mhost9\x1b[m'

    r.assertNotEquals(coloredDiff.find(expectedColoredDiff), -1)
    r.c(commitChangeset("objects set up"))
    helper_check_second_clone(r, ["README", "all-hosts"])

    # Now remove the generated file
    writeGenerator("01",
"""#!/usr/bin/env python
import os

os.unlink("all-hosts")
""")
    changeset = r.c(startChangeset())
    mytarget = os.path.join(PATH_WC, changeset, "all-hosts")
    r.assertTrue(not os.path.exists(mytarget))
    r.c(createObject("vendor", "rm-v-trivial"))
    r.c(commitChangeset("deleted conf files"))
    helper_check_second_clone(r, ["README"])

    rmGenerator("01")

def test_no_generators(r):
    """Running without any config generators shall fail horribly"""

    # Commit without calling the showConfigDiff() shall fail
    helper_check_second_clone(r, ["README"])
    changeset = r.c(startChangeset())
    r.c(createObject("vendor", "dummy"))
    r.cfail(commitChangeset("added a dummy vendor"), exception=CfgGeneratingError())
    helper_check_second_clone(r, ["README"])
    r.cvoid(abortCurrentChangeset())
    helper_check_second_clone(r, ["README"])

    # Calling showConfigDiff() shall fail, too
    changeset = r.c(startChangeset())
    helper_check_second_clone(r, ["README"])
    r.c(createObject("vendor", "dummy"))
    r.cfail(showConfigDiff(), exception=CfgGeneratingError())
    helper_check_second_clone(r, ["README"])
    r.cfail(commitChangeset("added a dummy vendor"), exception=CfgGeneratingError())
    helper_check_second_clone(r, ["README"])
    r.cvoid(abortCurrentChangeset())
    helper_check_second_clone(r, ["README"])

def test_no_output(r):
    """Config generators which do not produce any output"""
    writeGenerator("01",
"""#!/usr/bin/env python
import deska

deska.init()
""")
    changeset = r.c(startChangeset())
    r.c(createObject("vendor", "dummy01"))
    r.assertEqual(r.c(showConfigDiff()), "")
    r.c(commitChangeset("nothing"))
    # try it once again, now without the explicit showConfigDiff() call
    changeset = r.c(startChangeset())
    r.c(createObject("vendor", "dummy02"))
    r.c(commitChangeset("nothing"))
    rmGenerator("01")

def test_unchanged_output(r):
    """Test what happens if some commits do not generate any action"""
    writeGenerator("01",
"""#!/usr/bin/env python

f = file("blah", "wb")
f.write("pwnzor\\n")
""")
    changeset = r.c(startChangeset())
    helper_check_second_clone(r, ["README"])
    r.c(createObject("vendor", "dummy03"))
    r.c(commitChangeset("added file"))
    helper_check_second_clone(r, ["README", "blah"])
    mytarget = os.path.join(PATH_WC, changeset, "blah")
    r.assertEqual(file(mytarget, "rb").readlines(), ["pwnzor\n"])
    changeset = r.c(startChangeset())
    r.cvoid(deleteObject("vendor", "dummy03"))
    r.c(commitChangeset("no change"))
    helper_check_second_clone(r, ["README", "blah"])
    #mytarget = os.path.join(PATH_WC, changeset, "blah")
    r.assertEqual(file(mytarget, "rb").readlines(), ["pwnzor\n"])
    rmGenerator("01")

    writeGenerator("01",
"""#!/usr/bin/env python
import os

os.unlink("blah")
""")
    changeset = r.c(startChangeset())
    mytarget = os.path.join(PATH_WC, changeset, "blah")
    r.assertTrue(not os.path.exists(mytarget))
    r.c(createObject("vendor", "rm-v-unchanged"))
    r.c(commitChangeset("deleted conf files"))
    helper_check_second_clone(r, ["README"])

    rmGenerator("01")


def test_parallel_commit(r, shallCommit):
    """Test what happens when the config generators take a long time to complete

    This is going to be rather complicated. We want to have two parallel
    sessions running at the same time. The first of them will start a changeset,
    perform some changes and trigger the cfggen process, but this process will
    take "quite some time" to finish. While the generators are running, the
    parallel session attempts to steal the changeset. That shall fail.
    """

    lockA = tempfile.mkstemp()[1]
    lockDir = tempfile.mkdtemp()
    fifo = os.path.join(lockDir, "l")
    os.mkfifo(fifo)

    writeGenerator("sleeping-beauty",
"""#!/usr/bin/env python
import fcntl
import os
fA = file("%s", "ab")
fB = file("%s", "wb")
fB.write("x")
fB.flush()
fcntl.lockf(fA.fileno(), fcntl.LOCK_EX)
file("output", "wb").write("All done\\n")
""" % (lockA, fifo))

    fA = file(lockA, "ab")
    fcntl.lockf(fA.fileno(), fcntl.LOCK_EX)

    conn1 = Connection(r.cmd)
    conn2 = Connection(r.cmd)

    changeset = r.c(startChangeset(), conn=conn1)
    r.c(createObject("vendor", "dummy06"), conn=conn1)
    # We have to go the manual route here. We cannot block now, so we have to
    # feed the data directly.
    print >>sys.stderr, "*** Config generator will work in %s" % PATH_WC
    print >>sys.stderr, "*** Starting the config generator in the background"""
    conn1.p.stdin.write("{\"tag\":\"HHH\",\"command\":\"showConfigDiff\"}\r\n")
    conn1.p.stdin.flush()
    print >>sys.stderr, "*** Cfg generator started."""
    # At this point, the process associated with the conn1 shall begin executing
    # the generators.
    # Let's wait for the generator to come to the life
    print >>sys.stderr, "*** About to open a FIFO"
    fp = os.open(fifo, os.O_RDONLY)
    print >>sys.stderr, "*** FIFO opened, reading from there"
    r.assertEqual(os.read(fp, 1), "x")
    print >>sys.stderr, "*** Read data, which means that the cfg generator is alive and kicking"

    # At this point, the generator is blocking and we can do our best to beat
    # the hell out of the DB from a second connection.
    r.cfail(resumeChangeset(changeset), conn=conn2, exception=ChangesetLockingError())

    # Let's unblcok the generator and let it continue
    print >>sys.stderr, "*** Unblocking the cfg generator in conn1"
    fcntl.lockf(fA.fileno(), fcntl.LOCK_UN)

    # ...and process the command we sent in there
    print >>sys.stderr, "*** Waiting for the cfg generator to respond"""
    status = select.select((conn1.p.stdout, conn1.p.stderr), (), ())
    if (conn1.p.stderr in status[0]):
        err = os.read(conn1.p.stderr.fileno(), 65536)
        print >>sys.stderr, err
        r.assertTrue(False)
    cmdres = deunicodeify(json.loads(conn1.p.stdout.readline()))
    r.assertEqual(sorted(cmdres.keys()), sorted(['tag', 'response', 'showConfigDiff']))
    r.assertTrue(cmdres["showConfigDiff"].index("All done") != -1)
    rmGenerator("sleeping-beauty")


def imperative(r):
    test_trivial(r)
    test_no_generators(r)
    test_no_output(r)
    test_unchanged_output(r)
    test_parallel_commit(r)
