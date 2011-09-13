'''Test that the server-side configuration generators work reasonably'''

import os
from apiUtils import *
import datetime
import deska

def writeGenerator(fname, script):
    fname = "%s/%s.py" % (os.environ["DESKA_CFGGEN_SCRIPTS"], fname)
    f = file(fname, "wb")
    f.write(script)
    f.close()
    os.chmod(fname, 0755)

def imperative(r):
    # create a sample script that simply dumps names of all hosts to a file
    writeGenerator("01",
"""#!/usr/bin/env python2
import deska

deska.init()

output = file("all-hosts", "wb")
for host in deska.host[deska.host.name != "404"]:
    output.write("%s\\n" % host)
""")

    r.c(startChangeset())
    for x in range(10):
        objname = "host%d" % x
        r.assertEqual(r.c(createObject("host", objname)), objname)

    coloredDiff = r.c(showConfigDiff())
    print coloredDiff

    # The format shall be "human readable", which basically means a colored diff, and the usual rule about an unsorted output
    # applies.  That's rather hard to check programatically.
    for x in range(10):
        r.assertNotEquals(coloredDiff.find("host%d" % x), -1)
    r.c(commitChangeset("objects set up"))
