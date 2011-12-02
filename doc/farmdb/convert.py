import csv
import codecs
import socket
import struct

fd_vendors = {}
fd_networks = {}
fd_hardware = {}
fd_machines = {}
fd_interfaces = {}

class Struct(object):
    def __repr__(self):
        return "{%s}" % ", ".join("%s: %s" % (attr, self.__getattribute__(attr)) for attr in dir(self) if not attr.startswith("__"))

def unescape(x):
    if isinstance(x, str):
        x = x.lstrip()
        if x.startswith("N'") and x.endswith("'"):
            return x[2:][:-1]
        elif x == "NULL":
            return None
        elif x == "":
            return None
        else:
            return x
    elif isinstance(x, (list, tuple)):
        return [unescape(item) for item in x]
    else:
        return x

def getfile(name):
    #reader = csv.reader(file("%s.sql.csv" % name, "rb"))
    reader = csv.reader(preprocess_sql(name))
    first = True
    for row in reader:
        if first:
            first = False
            continue
        yield unescape(row)

def preprocess_sql(name):
    f = codecs.open("dbo.%s.Table.sql" % name, "rb", "utf-16")
    for line in f:
        if line.find("VALUES ") == -1:
            continue
        (garbage, line) = line.split("VALUES ")
        if line.startswith("("):
            line = line[1:]
        if line.endswith(")\r\n"):
            line = line[:-3]
        yield line

for (uid, name) in getfile("Vendors"):
    o = Struct()
    o.name = name
    fd_vendors[uid] = o

for row in getfile("Networks"):
    o = Struct()
    try:
        (uid, o.name, ip4, o.vlan, mask, o.note) = row
    except ValueError:
        print row
        raise
    numeric_addr = int(ip4, 16)
    if numeric_addr >= 0xffffffff:
        o.ip6 = numeric_addr
        o.mask6 = "pwn"
    else:
        print ip4, numeric_addr
        o.ip4 = socket.inet_ntoa(struct.pack("I", socket.htonl(numeric_addr)))
        o.mask4 = mask
    fd_networks[uid] = o

for row in getfile("Hardware"):
    o = Struct()
    try:
        (uid, o.vendorUid, o.vendorId, o.typeDesc, o.cpuDesc, o.cpuCount,
         o.cpuCoreCount, o.cpuHt, o.cpuPerf, o.ram, o.hddSize, o.hddDesc,
         o.weight, o.height, o.width, o.power, o.note, o.imageName) = row
    except ValueError:
        print row
        raise
    fd_hardware[uid] = o

for row in getfile("Machines"):
    o = Struct()
    try:
        (uid, o.parentMachineUid, o.serial, o.warrantyNo, o.invNo, o.hwUid,
         o.cpuHt, o.purchaseDate, o.warrantyEnd, o.kvmNo, o.kvmPos, o.rackNo,
         o.rackPos, o.rackHPos, o.os, o.note, o.obsolete) = row
    except ValueError:
        print row
        raise
    fd_machines[uid] = o

for row in getfile("Interfaces"):
    o = Struct()
    try:
        (uid,
         parentInterface, # appears unused
         o.machine, o.ip, o.mac, o.network, o.dns,
         o.switchNo, o.switchPos, o.note, o.pref) = row
    except ValueError:
        print row
        raise
    fd_interfaces[uid] = o


#histogram = {}
#for (k, v) in fd_interfaces.iteritems():
#    if histogram.has_key(v.parentInterface):
#        histogram[v.parentInterface] = histogram[v.parentInterface] + 1
#    else:
#        histogram[v.parentInterface] = 1
#print histogram

print "# dumping vendors"
for x in fd_vendors.itervalues():
    if x.name != "UNKNOWN":
        print "vendor %s\nend\n" % x.name
print
print "# dumping networks"
for x in fd_networks.itervalues():
    print "network %s" % x.name
    if dir(x).count("ip4"):
        print "  ip4 %s" % x.ip4
        print "  cidr4 %s" % x.mask4
    else:
        print "  ip6 %s" % x.ip6
        print "  cidr6 %s" % x.mask6
    print "  vlan %s" % x.vlan
    if x.note is not None:
        print "  note \"%s\"" % x.note
    print "end\n"
print
print "# dumping HW models"
for x in fd_hardware.itervalues():
    if x.vendorUid == "00000000-0000-0000-0000-000000000001":
        fullname = "_-%s" % x.typeDesc
    else:
        fullname = "%s-%s" % (fd_vendors[x.vendorUid].name, x.typeDesc)
    print "modelhardware %s" % fullname.replace(" ", "-")
    if x.cpuCount is not None:
        print "  cpu_sockets %s" % x.cpuCount
    if x.cpuCoreCount is not None:
        print "  cpu_cores %s" % x.cpuCoreCount
    if x.cpuDesc is not None:
        print "  cpu_type \"%s\"" % x.cpuDesc
    if x.cpuHt is not None:
        print "  cpu_ht %s" % x.cpuHt
    if x.cpuPerf is not None:
        print "  hepspec %s" % x.cpuPerf
    if x.vendorUid != "00000000-0000-0000-0000-000000000001":
        print "  vendor %s" % fd_vendors[x.vendorUid].name
    if x.ram is not None:
        print "  ram %s" % x.ram
    if x.hddSize is not None:
        print "  hdd_drive_capacity %s" % x.hddSize
    if x.hddDesc is not None:
        print "  hdd_note \"%s\"" % x.hddDesc
    if x.power is not None:
        print "  power_max %s" % x.power
    if x.imageName is not None:
        print "  # FIXME: imageName %s" % x.imageName
    if x.note is not None:
        print "  note_hardware \"%s\"" % x.note
    # FIXME: weight
    # FIXME: width, height into a modelbox
    print "end\n"
print

import pprint
#pprint.pprint(fd_hardware)
#for x in fd_vendors, fd_networks, fd_hardware, fd_machines, fd_interfaces:
#    pprint.pprint(x)
