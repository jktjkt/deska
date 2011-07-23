from deska import *

for (name, x) in host[host.name == 'a'].iteritems():
    print name, x.hardware, x.note
