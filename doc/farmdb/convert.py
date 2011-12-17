import csv
import codecs
import socket
import struct
import datetime
import sys
import ply.lex as lex

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

def ipify(ip):
    if ip is None:
        return ip
    if ip.startswith("0x"):
        ip = ip[2:]
    numeric_addr = int(ip, 16)
    if numeric_addr >= 0xffffffff:
        out = []
        for x in range(8):
            start = x*4
            end = start + 4
            out.append(int(ip[start:end], 16))
        return ":".join(hex(byte)[2:] for byte in out)
    else:
        # This is extremely unportable, as we rely on stuff like "host byte
        # order". We don't care. At all.
        return socket.inet_ntoa(struct.pack("I", socket.htonl(numeric_addr)))

class DboLexer:
    tokens = ('STR', 'COMMA', 'LPAREN', 'RPAREN', 'LEADER', 'NUM', 'HEX',
              'NULL', 'CRLF', 'CAST_DATETIME')

    t_LEADER = r"INSERT (.*) VALUES "
    def t_STR(self, t):
        r"(N')((''|[^'])*)(')"
        # yuck, that replace is evil
        t.value = t.lexer.lexmatch.group(3).replace("''", "'")
        # FIXME: this is a crude hack for the CLI
        t.value = t.value.replace("\"", "_").replace("\n", "_")
        return t
    def t_CAST_DATETIME(self, t):
        r"(CAST\(0x)(?P<datetime>[^\)]+)( DateTime\))"
        num_with_as = t.lexer.lexmatch.groupdict()["datetime"]
        if num_with_as.endswith(" AS "):
            num_with_as = num_with_as[:-4][:8]
            t.value = datetime.date(1900, 1, 1) + datetime.timedelta(days=int(num_with_as, 16))
        return t
    t_COMMA = ","
    t_LPAREN = "\("
    t_RPAREN = "\)"
    t_NUM = r"\d+"
    t_HEX = r"0x([0-9A-Fa-f])+"
    def t_NULL(self, t):
        "NULL"
        t.value = None
        return t
    t_CRLF = r"\r\n"

    t_ignore = " "

    def t_error(self, t):
        print "Invalid input: %s" % t
        sys.exit(1)

    def build(self, **kwargs):
        self.lexer = lex.lex(module=self, **kwargs)

    def parse(self, data):
        self.lexer.input(data)
        (S_BEGIN, S_FIELDS) = range(2)
        state = S_BEGIN
        line = []
        want_comma = False
        for tok in self.lexer:
            if tok.type == 'CRLF':
                continue
            if state == S_BEGIN:
                if tok.type == 'LPAREN':
                    state = S_FIELDS
                elif tok.type == 'LEADER':
                    continue
                else:
                    raise RuntimeError, "Unexpected token %s" % repr(tok)
            elif state == S_FIELDS:
                if tok.type == 'NUM' or tok.type == 'HEX' or tok.type == 'STR' or \
                   tok.type == 'NULL' or tok.type == "CAST_DATETIME":
                    if want_comma:
                        raise RuntimeError, "wanted comma: %s" % repr(tok)
                    line.append(tok.value)
                    want_comma = True
                elif tok.type == 'COMMA':
                    want_comma = False
                elif tok.type == 'RPAREN':
                    want_comma = False
                    state = S_BEGIN
                    yield line
                    line = []
                else:
                    raise RuntimeError, "Unexpected token %s" % repr(tok)

    def __init__(self):
        self.build()

def getfile(name):
    return (line for line in DboLexer().parse(open("dbo.%s.Table.sql" % name, "rb").read()))

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
    o.mac = o.mac.strip()
    if o.ip is not None:
        # FIXME: hardcoded stuff...
        if o.ip.startswith("172.16."):
            o.network = "6d31f21c-ceaf-4c22-9269-1730cd686456"
        elif o.ip.startswith("10.26.205."):
            o.network = "9751c60e-c319-45c5-a6c1-22fa7e63c4cb"
    if not len(o.mac):
        o.mac = None
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
        print "create vendor %s\n" % x.name
print
print "# dumping networks"
for x in fd_networks.itervalues():
    print "create network %s" % x.name
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
create formfactor rackmount

create modelbox 1u
modelbox 1u
  formfactor rackmount
end

create modelbox 2u
modelbox 2u
  formfactor rackmount
  height 2
end

create modelbox 3u
modelbox 3u
  formfactor rackmount
  height 3
end

create modelbox 4u
modelbox 4u
  formfactor rackmount
  height 4
end

"""

print """# Generic racks
create modelbox generic-rack
modelbox generic-rack
  internal_height 47
  internal_width 1
  internal_depth 1
  accepts_inside [rackmount]
end

"""

for rack in set([x.rackNo for x in fd_machines.itervalues() if x.rackNo is not None]):
    print "create box %s" % rack
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
    fullname = fullname.replace(" ", "-").replace("(", "").replace(")", "")
    # does not work: fullname.strip("-")
    if fullname.endswith("-"):
        fullname = fullname[:-1]
    out_assigned_modelhw[uid] = fullname
    # FIXME: "create" fails with duplicates
    #print "create modelhardware %s" % fullname
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
        myname = "FIXME_unknown"
        print "# FIXME: unknown HW; this would lead to a name clash:"
        print "# %s" % x
        continue
    if myname == "0":
        print "# FIXME: the following HW got created by a nasty UID"
        myname = uid
    if x.obsolete:
        myname += "-obsolete"
    myname = myname.replace(" ", "_").replace("/", "_").replace(".", "_")
    out_assigned_hardware[uid] = myname
    print "create hardware %s" % myname
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
    print "create host %s" % myname
    print "create box %s" % myname
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
    print "create interface %s->eth%d" % (hw, ifindex)
    print "interface %s->eth%d" % (hw, ifindex)
    try:
        print "  network %s" % fd_networks[x.network].name
    except KeyError:
        print "# FIXME: no such network: %s" % x.network
    if x.mac is not None:
        print "  mac %s" % x.mac
    if x.ip is not None:
        print "  ip4 %s" % x.ip
    print "end\nend\n"
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
