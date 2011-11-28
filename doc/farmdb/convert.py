import csv

fd_vendors = {}
fd_networks = {}

class Struct(object):
    def __repr__(self):
        return "{%s}" % ", ".join("%s: %s" % (attr, self.__getattribute__(attr)) for attr in dir(self) if not attr.startswith("__"))

def unescape(x):
    if isinstance(x, str):
        if x.startswith("N'") and x.endswith("'"):
            return x[2:][:-1]
        elif x == "NULL":
            return None
        else:
            return x
    elif isinstance(x, (list, tuple)):
        return [unescape(item) for item in x]
    else:
        return x

def getfile(name):
    reader = csv.reader(file("%s.sql.csv" % name, "rb"))
    first = True
    for row in reader:
        if first:
            first = False
            continue
        yield unescape(row)

for (uid, name) in getfile("vendors"):
    o = Struct()
    o.name = name
    fd_vendors[uid] = o

for row in getfile("networks"):
    try:
        (uid, name, ip, vlan, mask, note) = row
    except ValueError:
        print row
        raise
    o = Struct()
    o.name = name
    o.ip = ip
    o.vlan = vlan
    o.mask = mask
    o.note = note
    fd_networks[uid] = o

import pprint
for x in fd_vendors, fd_networks:
    pprint.pprint(x)
