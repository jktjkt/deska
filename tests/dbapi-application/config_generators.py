'''Test that the server-side configuration generators work reasonably'''

import os
from apiUtils import *
import datetime
import deska
import subprocess

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


def imperative(r):
    test_trivial(r)
    test_no_generators(r)
