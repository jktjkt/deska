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

create modelbox 5u
modelbox 5u
  formfactor rackmount
  height 5
end

create modelbox 8u
modelbox 8u
  formfactor rackmount
  height 8
end

create modelbox 9u
modelbox 9u
  formfactor rackmount
  height 9
end

create modelbox 10u
modelbox 10u
  formfactor rackmount
  height 10
end

create modelbox 11u
modelbox 11u
  formfactor rackmount
  height 11
end

create modelbox 29u
modelbox 29u
  formfactor rackmount
  height 29
end

create modelbox 40u
modelbox 40u
  formfactor rackmount
  height 40
end

# Generic racks

create formfactor rack

create modelbox generic-rack
modelbox generic-rack
  internal_height 48
  internal_width 1
  internal_depth 1
  accepts_inside [rackmount]
  formfactor rack
end

create modelbox serverovna
modelbox serverovna
   accepts_inside [rack]
end

box serverovna
  direct_modelbox serverovna
end

create formfactor idataplex-unit
create formfactor idataplex-sleeve

create modelbox idataplex-rack
modelbox idataplex-rack
    internal_width 2
    internal_height 48
    internal_depth 1
    accepts_inside [idataplex-sleeve]
    formfactor rack
end

create modelbox idataplex-sleeve
modelbox idataplex-chassis-2u
    formfactor idataplex-sleeve
    accepts_inside [idataplex-unit]
    width 1
    height 2
    depth 1
    internal_width 1
    internal_depth 1
    internal_height 2
    note "A sleeve for two iDataPlex units"
end

create modelbox idataplex-1u
modelbox idataplex-1u
    formfactor idataplex-unit
    width 1
    height 1
    depth 1
end

create modelbox idataplex-2u
modelbox idataplex-2u
    formfactor idataplex-unit
    width 1
    height 2
    depth 1
end

create formfactor sgi-twin

create modelbox sgi-twin-chassis
modelbox sgi-twin-chassis
    formfactor rackmount
    accepts_inside [sgi-twin]
    width 1
    height 1
    depth 1
    internal_width 2
    internal_depth 1
    internal_height 1
    note "A chassis for housing of two SGI twin nodes"
end

create modelbox sgi-twin
modelbox sgi-twin
    formfactor sgi-twin
    width 1
    height 1
    depth 1
end

create formfactor hp-blade-c

create modelbox hp-blade-c-chassis
modelbox hp-blade-c-chassis
    formfactor rackmount
    accepts_inside [hp-blade-c]
    width 1
    height 9
    depth 1
    internal_width 8
    internal_height 2
    internal_depth 1
    note "A C-class HP blade chassis"
end

create modelbox hp-blade-c
modelbox hp-blade-c
    formfactor hp-blade-c
    width 1
    height 1
    depth 1
end


create formfactor hp-blade-p

create modelbox hp-blade-p-chassis
modelbox hp-blade-p-chassis
    formfactor rackmount
    accepts_inside [hp-blade-p]
    width 1
    height 6
    depth 1
    internal_width 8
    internal_height 2
    internal_depth 1
    note "A P-class HP blade chassis"
end

create modelbox hp-blade-p-1u
modelbox hp-blade-p-1u
    formfactor hp-blade-p
    width 1
    height 1
    depth 1
end

create modelbox hp-blade-p-2u
modelbox hp-blade-p-2u
    formfactor hp-blade-p
    width 1
    height 2
    depth 1
end


# services and roles

create service wn
create service cluster_salix
create service cluster_saltix
create service cluster_golias
create service cluster_hypericum
create service cluster_iberis
create service cluster_ibis
create service cluster_ib
create service cluster_dorje

create service ha
create service dpmpool
create service torque
create service glite-ce
create service cream
create service dpm-head
create service nfs
create service sam
create service sekce
create service xrootd

create service cvmfs_wn
create service ui
create service xen_dom0
create service xen_paravirt
create service www
create service sbdii
create service ganglia_aggregator
create service dns
create service dhcpd
create service bond0_eth0_eth1

create service in_nagios

"""

for rack in set([x.rackNo for x in fd_machines.itervalues() if x.rackNo is not None]):
    print "create box %s" % rack
    print "box %s" % rack
    print "  inside serverovna"
    if rack in ("L10", "L11"):
        print "  direct_modelbox idataplex-rack"
    else:
        print "  direct_modelbox generic-rack"
    print "end"
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
    if fullname in out_assigned_modelhw.values():
        # try to avoid a collision
        candidate = "%s-%sGRAM" % (fullname, x.ram)
        if candidate in out_assigned_modelhw.values():
            fullname = "%s-%sdisk" % (fullname, x.hddSize)
            # ...and hope for the best
        else:
            fullname = candidate
    out_assigned_modelhw[uid] = fullname
    # FIXME: "create" fails with duplicates
    print "create modelhardware %s" % fullname
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
        if x.height in [str(y) for y in range(1,5) + [8, 9, 10, 11, 29, 40]]:
            print "  modelbox %su" % x.height
        else:
            print "# FIXME: weird height '%s' -> no modelbox" % x.height
    else:
        if fullname == "IBM-iDataPlex-dx360-M2-2U":
            print "  modelbox idataplex-2u"
        elif fullname.startswith("IBM-iDataPlex"):
            print "  modelbox idataplex-1u"
        elif fullname in ("SGI-Altix-XE310", "SGI-Altix-XE340"):
            print "  modelbox sgi-twin"
        elif fullname == "HP-BL20p":
            print "  modelbox hp-blade-p-2u"
        elif fullname.startswith("HP-BL35p"):
            print "  modelbox hp-blade-p-1u"
        elif fullname in ("HP-BL465c", "HP-BL460c", "HP-Bl460c"):
            print "  modelbox hp-blade-c"
        else:
            print "# FIXME: weird width '%s' -> no modelbox" % x.width
    print "end\n"
print
print

def cut_trailing(haystack, needles):
    for needle in needles:
        if haystack.endswith(needle):
            haystack = haystack[:-len(needle)]
    return haystack

def find_hostname_for_hw(uid, x):
    if map_ifaces.has_key(uid):
        names = [cut_trailing(fd_interfaces[y].dns, (".farm.particle.cz", ".fzu.cz", ".monitor"))
                for y in map_ifaces[uid] if fd_interfaces[y].dns is not None]
        names = list(set(names))
        if len(names):
            myname = "-".join(names)
        else:
            myname = "FIXME list of interfaces is useless"
    elif x.serial is not None:
        myname = x.serial
    elif out_assigned_modelhw.has_key(x.hwUid):
        # ...and hope for the best here...
        myname = out_assigned_modelhw[x.hwUid]
    else:
        myname = "FIXME_unknown"
        print "# FIXME: unknown HW; this would lead to a name clash:"
        print "# %s" % x
    if myname == "0":
        myname = uid
        print "# FIXME: will create HW with a nasty UID: %s" % myname
    if x.obsolete:
        myname += "-obsolete"
    myname = myname.replace(" ", "_").replace("/", "_").replace(".", "_")
    return myname

print """
create box hp-enc-c-1
box hp-enc-c-1
    direct_modelbox hp-blade-c-chassis
    inside R05
    y 24
    x 1
end

create box hp-enc-c-2
box hp-enc-c-2
    direct_modelbox hp-blade-c-chassis
    inside R05
    y 15
    x 1
end

create box hp-enc-p-1
box hp-enc-p-1
    direct_modelbox hp-blade-p-chassis
    inside L08
    x 1
    y 29
end

create box hp-enc-p-2
box hp-enc-p-2
    direct_modelbox hp-blade-p-chassis
    inside L08
    x 1
    y 23
end

create box hp-enc-p-3
box hp-enc-p-3
    direct_modelbox hp-blade-p-chassis
    inside L08
    x 1
    y 17
end


"""

created_twins = {}
obsolete_items = []

print "# dumping hardware"
for (uid, x) in fd_machines.iteritems():
    if uid == "9870fb90-bd67-4162-8801-0dc6e3a29fab":
        # wtf? "virtual blade"?
        continue
    myname = find_hostname_for_hw(uid, x)
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
        obsolete_items.append(("hardware", myname))
        obsolete_items.append(("box", myname))
        obsolete_items.append(("host", myname))
    if x.os is not None:
        print "# FIXME: os %s" % x.os
    my_modelhw = None
    if x.hwUid is not None:
        if out_assigned_modelhw.has_key(x.hwUid):
            my_modelhw = out_assigned_modelhw[x.hwUid]
            print "  modelhardware %s" % my_modelhw
        else:
            print "# FIXME: modelhardware not found: %s" % x.hwUid
    print "end"
    if my_modelhw.startswith("IBM-iDataPlex"):
        # a twin node; let's see if it's present already
        # These things are special, as the farmdb specifies their own slot for
        # each of them
        x.rackPos = int(x.rackPos)
        if int(x.rackHPos) == 1:
            # the left column
            if x.rackPos % 2:
                # an even one -> be special
                positions = [x.rackPos, x.rackPos + 1]
                rack_y = x.rackPos + 1
                sleeve_pos = 1
            else:
                positions = [x.rackPos, x.rackPos - 1]
                rack_y = x.rackPos
                sleeve_pos = 2
        else:
            # the right column
            if x.rackPos % 2:
                # an even one -> be special
                positions = [x.rackPos, x.rackPos - 1]
                rack_y = x.rackPos
                sleeve_pos = 1
            else:
                positions = [x.rackPos, x.rackPos + 1]
                rack_y = x.rackPos + 1
                sleeve_pos = 2

        candidates = [find_hostname_for_hw(k,v) for (k,v) in fd_machines.iteritems() if
                      v.hwUid == x.hwUid and v.rackNo == x.rackNo and
                      v.rackHPos == x.rackHPos and int(v.rackPos) in positions]
        if len(candidates) == 1:
            # looks like this one is alone in there
            boxname = "%s-sleeve" % candidates[0]
        else:
            boxname = "-".join(candidates)
        rack_x = x.rackHPos
        format = {"boxname": boxname, "rack": x.rackNo, "rack_x": rack_x,
                  "hostname": myname, "sleeve_pos": sleeve_pos, "rack_y": rack_y}
        if not created_twins.has_key(boxname):
            if x.obsolete is not None:
                obsolete_items.append(("box", boxname))
            created_twins[boxname] = True
            box_str = """create box %(boxname)s
box %(boxname)s
    direct_modelbox idataplex-chassis-2u
    inside %(rack)s
    x %(rack_x)s
    y %(rack_y)s
end
""" % format
        else:
            box_str = ""
        box_str = box_str + """create box %(hostname)s
box %(hostname)s
    inside %(boxname)s
    y %(sleeve_pos)s
end
""" % format
    elif my_modelhw.startswith("HP-B") and my_modelhw.endswith("c"):
        # The c-class blades
        enc_x = x.rackHPos
        if x.rackPos == "24":
            enclosure = "hp-enc-c-1"
            enc_y = 2
        elif x.rackPos == "20":
            enclosure = "hp-enc-c-1"
            enc_y = 1
        elif x.rackPos == "15":
            enclosure = "hp-enc-c-2"
            enc_y = 2
        elif x.rackPos == "11":
            enclosure = "hp-enc-c-2"
            enc_y = 1
        else:
            raise RuntimeError
        format = {"boxname": myname, "enclosure": enclosure, "enc_x": enc_x,
                  "enc_y": enc_y}
        box_str = """create box %(boxname)s
box %(boxname)s
    inside %(enclosure)s
    x %(enc_x)s
    y %(enc_y)s
end
""" % format
    elif my_modelhw.startswith("HP-BL"):
        # The p-class blades
        enc_x = x.rackHPos
        if x.rackPos == "29":
            enclosure = "hp-enc-p-1"
            enc_y = 2
        elif x.rackPos == "26":
            enclosure = "hp-enc-p-1"
            enc_y = 1
        elif x.rackPos == "23":
            enclosure = "hp-enc-p-2"
            enc_y = 2
        elif x.rackPos == "20":
            enclosure = "hp-enc-p-2"
            enc_y = 1
        elif x.rackPos == "17":
            enclosure = "hp-enc-p-3"
            enc_y = 2
        elif x.rackPos == "14":
            enclosure = "hp-enc-p-3"
            enc_y = 1
        else:
            raise RuntimeError, "Dunno how to convert %s" % x
        format = {"boxname": myname, "enclosure": enclosure, "enc_x": enc_x,
                  "enc_y": enc_y}
        box_str = """create box %(boxname)s
box %(boxname)s
    inside %(enclosure)s
    x %(enc_x)s
    y %(enc_y)s
end
""" % format


    elif my_modelhw in ("SGI-Altix-XE310", "SGI-Altix-XE340"):
        # the SGI twins
        candidates = [find_hostname_for_hw(k,v) for (k,v) in fd_machines.iteritems() if
                      v.hwUid == x.hwUid and v.rackNo == x.rackNo and v.rackPos == x.rackPos]
        if len(candidates) == 1:
            # looks like this one is alone in there
            boxname = "%s-sleeve" % candidates[0]
        else:
            boxname = "-".join(sorted(candidates))
        format = {"boxname": boxname, "rack": x.rackNo,"hostname": myname,
                  "sleeve_pos": x.rackHPos, "rack_y": x.rackPos}
        if not created_twins.has_key(boxname):
            if x.obsolete is not None:
                obsolete_items.append(("box", boxname))
            created_twins[boxname] = True
            box_str = """create box %(boxname)s
box %(boxname)s
    direct_modelbox sgi-twin-chassis
    inside %(rack)s
    x 1
    y %(rack_y)s
end
""" % format
        else:
            box_str = ""
        box_str = box_str + """create box %(hostname)s
box %(hostname)s
    inside %(boxname)s
    x %(sleeve_pos)s
    y 1
end
""" % format

    else:
        box_str = None
    print "create host %s" % myname
    services = []
    if myname == "golias100":
        services.append("dpm-head")
    elif myname.startswith("golias"):
        services.append("wn")
        services.append("cluster_golias")
    elif myname.startswith("iberis"):
        services.append("wn")
        services.append("cluster_iberis")
    elif myname.startswith("ibis"):
        services.append("wn")
        services.append("cluster_ibis")
    elif myname.startswith("ib"):
        services.append("wn")
        services.append("cluster_ib")
    elif myname.startswith("salix"):
        services.append("wn")
        services.append("cluster_salix")
    elif myname.startswith("saltix"):
        services.append("wn")
        services.append("cluster_saltix")
    elif myname.startswith("dpmpool") or myname == "se4":
        services.append("dpmpool")
    elif myname.startswith("xrootd"):
        services.append("xrootd")
    elif myname.startswith("storage"):
        services.append("nfs")
    elif myname.startswith("ha"):
        services.append("ha")
    elif myname in ("sam2", "sam3", "sam4"):
        services.append("sam")
    elif myname.startswith("ui"):
        services.append("ui")

    if myname in ("golias101", "golias169", "iberis01", "iberis03", "salix01",
                  "salix03", "saltix01", "saltix03", "ibis01", "ibis03", "ib01",
                  "ib03", "monitor", "hpv2", "skurut1-2.egee.cesnet.cz",
                  "skurut2-2", "dpm1", "ce2.egee.cesnet.cz"):
        services.append("ganglia_aggregator")

    if myname in ("netservice1", "netservice2"):
        services = services + ["dns", "dhcpd"]

    if myname in ("ha1", "ha2", "ha3", "sam3", "dpmpool4", "se4", "samson", "dalila"):
        services.append("bond0_eth0_eth1")

    if len(services):
        services.append("in_nagios")

    if len(services):
        print "host %s" % myname
        print "  service [%s]" % ", ".join(services)
        print "end"

    if box_str is not None:
        print box_str
    else:
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

# removing obsolete stuff"""

for (kind, name) in sorted(obsolete_items):
    print "delete %s %s" % (kind, name)

print """

delete box L50
delete box L51
delete box L53
delete box L61

@commit to r3
jkt
Removing obsolete items
2011-Dec-19 20:00:00.0
# commit end"""
