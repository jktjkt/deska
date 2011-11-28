import csv

fd_vendors = {}
fd_networks = {}
fd_hardware = {}
fd_machines = {}

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
    o = Struct()
    try:
        (uid, o.name, o.ip, o.vlan, o.mask, o.note) = row
    except ValueError:
        print row
        raise
    fd_networks[uid] = o

for row in getfile("hardware"):
    o = Struct()
    try:
        (uid, o.vendorUid, o.vendorId, o.typeDesc, o.cpuDesc, o.cpuCount,
         o.cpuCoreCount, o.cpuHt, o.cpuPerf, o.ram, o.hddSize, o.hddDesc,
         o.weight, o.height, o.width, o.power, o.note, o.imageName) = row
    except ValueError:
        print row
        raise
    fd_hardware[uid] = o

for row in getfile("machines"):
    o = Struct()
    try:
        (uid, o.parentMachineUid, o.serial, o.warrantyNo, o.invNo, o.hwUid,
         o.cpuHt, o.purchaseDate, o.warrantyEnd, o.kvmNo, o.kvmPos, o.rackNo,
         o.rackPos, o.rackHPos, o.os, o.note, o.obsolete) = row
    except ValueError:
        print row
        raise
    fd_machines[uid] = o

import pprint
for x in fd_vendors, fd_networks, fd_hardware, fd_machines:
    pprint.pprint(x)
