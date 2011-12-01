'''Test that the server-side configuration generators work reasonably'''

import os
from apiUtils import *
import datetime
import deska

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

    r.c(startChangeset())
    for x in range(10):
        objname = "host%d" % x
        r.assertEqual(r.c(createObject("host", objname)), objname)

    coloredDiff = r.c(showConfigDiff())
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
    rmGenerator("01")

def test_no_generators(r):
    """Running without any config generators shall fail horribly"""

    # Commit without calling the showConfigDiff() shall fail
    r.c(startChangeset())
    r.c(createObject("vendor", "dummy"))
    r.cfail(commitChangeset("added a dummy vendor"), exception=CfgGeneratingError())
    r.cvoid(abortCurrentChangeset())

    # Calling showConfigDiff() shall fail, too
    r.c(startChangeset())
    r.c(createObject("vendor", "dummy"))
    r.cfail(showConfigDiff(), exception=CfgGeneratingError())
    r.cfail(commitChangeset("added a dummy vendor"), exception=CfgGeneratingError())
    r.cvoid(abortCurrentChangeset())


def imperative(r):
    test_trivial(r)
    test_no_generators(r)
