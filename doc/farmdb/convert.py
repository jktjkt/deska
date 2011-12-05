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

out_assigned_modelhw = {}
out_assigned_hardware = {}

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
        return datetime.date(1900, 1, 1) + datetime.timedelta(days=offset)
    else:
        raise ValueError, "Malformed date: %s" % x

def ipify(ip):
    if ip is None:
        return ip
    if ip.startswith("0x"):
        ip = ip[2:]
    numeric_addr = int(ip, 16)
    if numeric_addr >= 0xffffffff:
        tupleA = socket.htonl(int(ip[2:10], 16))
        tupleB = socket.htonl(int(ip[10:18], 16))
        tupleC = socket.htonl(int(ip[18:26], 16))
        tupleD = socket.htonl(int(ip[26:34], 16))
        bytes = struct.pack("IIII", tupleA, tupleB, tupleC, tupleD)
        return socket.inet_ntop(socket.AF_INET6, bytes)
    else:
        # This is extremely unportable, as we rely on stuff like "host byte
        # order". We don't care. At all.
        return socket.inet_ntoa(struct.pack("I", socket.htonl(numeric_addr)))

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
    uid = uid.lower()
    fd_vendors[uid] = o

for row in getfile("Networks"):
    o = Struct()
    try:
        (uid, o.name, ip, o.vlan, mask, o.note) = row
    except ValueError:
        print row
        raise
    uid = uid.lower()
    ip = ipify(ip)
    if ip.find(":") == -1:
        o.ip4 = ip
        o.mask4 = mask
    else:
        o.ip6 = ip
        o.mask6 = mask
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
    uid = uid.lower()
    if o.vendorUid is not None:
        o.vendorUid = o.vendorUid.lower()
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
    uid = uid.lower()
    o.purchaseDate = dateify(o.purchaseDate)
    o.warrantyEnd = dateify(o.warrantyEnd)
    if o.parentMachineUid is not None:
        o.parentMachineUid = o.parentMachineUid.lower()
    if o.hwUid is not None:
        o.hwUid = o.hwUid.lower()
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
    uid = uid.lower()
    if o.machine is not None:
        o.machine = o.machine.lower()
    if map_ifaces.has_key(o.machine):
        map_ifaces[o.machine].append(uid)
    else:
        map_ifaces[o.machine] = [uid]
    o.ip = ipify(o.ip)
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

print """# Generic racks
modelbox generic-rack
  internal_height 47
  internal_width 1
  internal_depth 1
  accepts_inside [rackmount]
end

"""

for rack in set([x.rackNo for x in fd_machines.itervalues() if x.rackNo is not None]):
    print "box %s" % rack
    print "  direct_modelbox generic-rack"
    print "end"
print
print
print "# dumping HW models"
for (uid, x) in fd_hardware.iteritems():
    if x.vendorUid == "00000000-0000-0000-0000-000000000001":
        fullname = "_-%s" % x.typeDesc
    else:
        fullname = "%s-%s" % (fd_vendors[x.vendorUid].name, x.typeDesc)
    fullname = fullname.replace(" ", "-")
    out_assigned_modelhw[uid] = fullname
    print "modelhardware %s" % fullname
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
    #if x.imageName is not None:
    #    print "# FIXME: imageName %s" % x.imageName
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
    out_assigned_hardware[uid] = myname
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
    if x.hwUid is not None:
        if out_assigned_modelhw.has_key(x.hwUid):
            print "  modelhardware %s" % out_assigned_modelhw[x.hwUid]
        else:
            print "# FIXME: modelhardware not found: %s" % x.hwUid
    print "end"
    print "box %s" % myname
    if x.rackNo is not None:
        print "  inside %s" % x.rackNo
        if x.rackHPos is not None:
            print "  x %s" % x.rackHPos
        if x.rackPos is not None:
            print "  y %s" % x.rackPos
    elif x.rackPos is not None or x.rackHPos is not None:
        print "# FIXME: rackNo is null, but others are specified: %s %s" % (x.rackPos, x.rackHPos)
    else:
        print "# FIXME: famrdb does not contain any physical placement"
        pass
    print "end\n"

for (uid, x) in fd_interfaces.iteritems():
    try:
        hw = out_assigned_hardware[x.machine]
    except KeyError:
        print "# FIXME: iface %s has unknown HW %s" % (uid, x.machine)
        print
        continue
    ifindex = map_ifaces[x.machine].index(uid)
    print "interface %s->eth%d" % (hw, ifindex)
    try:
        print "  network %s" % fd_networks[x.network].name
    except KeyError:
        print "# FIXME: no such network: %s" % x.network
    print "  mac %s" % x.mac
    print "  ip4 %s" % x.ip
    print "end\n"
    # FIXME: more of them!

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
