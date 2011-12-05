import csv
import codecs
import socket
import struct
import datetime

fd_vendors = {}
fd_networks = {}
fd_hardware = {}
fd_machines = {}
fd_interfaces = {}

map_ifaces = {}

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

def dateify(x):
    prefix = "CAST(0x"
    if x is None:
        return None
    elif x.startswith(prefix):
        x = x[len(prefix):][:8]
        offset = int(x, 16)
        return datetime.date(1970, 1, 1) + datetime.timedelta(days=offset)
    else:
        raise ValueError, "Malformed date: %s" % x


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
        (uid, o.name, ip, o.vlan, mask, o.note) = row
    except ValueError:
        print row
        raise
    numeric_addr = int(ip, 16)
    if numeric_addr >= 0xffffffff:
        tupleA = socket.htonl(int(ip[2:10], 16))
        tupleB = socket.htonl(int(ip[10:18], 16))
        tupleC = socket.htonl(int(ip[18:26], 16))
        tupleD = socket.htonl(int(ip[26:34], 16))
        bytes = struct.pack("IIII", tupleA, tupleB, tupleC, tupleD)
        o.ip6 = socket.inet_ntop(socket.AF_INET6, bytes)
        o.mask6 = mask
    else:
        # This is extremely unportable, as we rely on stuff like "host byte
        # order". We don't care. At all.
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
    o.purchaseDate = dateify(o.purchaseDate)
    o.warrantyEnd = dateify(o.warrantyEnd)
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
    if map_ifaces.has_key(o.machine):
        map_ifaces[o.machine].append(uid)
    else:
        map_ifaces[o.machine] = [uid]
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
print """# artificial: the boxmodels and form factors
formfactor rackmount
end

modelbox 1u
  formfactor rackmount
end

modelbox 2u
  formfactor rackmount
  height 2
end

modelbox 3u
  formfactor rackmount
  height 3
end
modelbox 4u
  formfactor rackmount
  height 4
end

"""
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
        if x.cpuHt:
            print "  cpu_ht true"
        else:
            print "  cpu_ht false"
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
        print "# FIXME: imageName %s" % x.imageName
    if x.note is not None:
        print "  note \"%s\"" % x.note
    if x.weight is not None:
        print "  weight %s" % x.weight
    if x.width == "100":
        if x.height in [str(y) for y in range(1,5)]:
            print "  modelbox %su" % x.height
        else:
            print "# FIXME: weird height '%s' -> no modelbox" % x.height
    else:
        print "# FIXME: weird width '%s' -> no modelbox" % x.width
    print "end\n"
print
print
print """# adding a fake modelhardware
modelhardware FAKE-REMOVE end

"""
print "# dumping hardware"
for (uid, x) in fd_machines.iteritems():
    if map_ifaces.has_key(uid):
        names = [fd_interfaces[y].dns for y in map_ifaces[uid] if
                 fd_interfaces[y].dns is not None]
        if len(names):
            myname = "-".join(names)
        else:
            myname = "FIXME list of interfaces is useless"
    elif x.serial is not None:
        myname = x.serial
    else:
        myname = "FIXME unknown"
    myname = myname.replace(" ", "_").replace("/", "_").replace(".", "_")
    print "box %s end" % myname
    print "hardware %s" % myname
    if x.serial is not None:
        print "  serial_1 \"%s\"" % x.serial
    if x.warrantyNo is not None:
        print "# FIXME  warrantyNo %s" % x.warrantyNo
    if x.warrantyEnd is not None:
        print "# FIXME  warrantyExpires %s" % x.warrantyEnd
    if x.invNo is not None:
        print "  inventory_no '%s'" % x.invNo
    if x.cpuHt is not None:
        print "# FIXME: cpu_ht at the individual level: %s" % x.cpuHt
    if x.purchaseDate is not None:
        print "  purchased %s" % x.purchaseDate
    if x.kvmNo is not None:
        print "# FIXME: kvmNo %s" % x.kvmNo
    if x.kvmPos is not None:
        print "# FIXME: kvmPos %s" % x.kvmPos
    if x.note is not None:
        print "  note_hardware '%s'" % x.note
    if x.obsolete is not None:
        print "# FIXME obsolete: %s" % x.obsolete
    if x.os is not None:
        print "# FIXME: os %s" % x.os
    # FIXME: rack no, pos, hpos
    print "  modelhardware FAKE-REMOVE"
    print "end\n"
print
print """@commit to r2
jkt
Initial import
2011-Dec-02 18:19:44.929512
#commit end
"""

import pprint
#pprint.pprint(fd_hardware)
#for x in fd_vendors, fd_networks, fd_hardware, fd_machines, fd_interfaces:
#    pprint.pprint(x)
