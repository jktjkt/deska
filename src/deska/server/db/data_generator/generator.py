#!/usr/bin/python2

import random, datetime

script_template = '''SET search_path TO genproc,history,deska,production;
BEGIN;
SELECT start_changeset();
{vendors_add}
{hardware_add}
{host_add}
{interface_add}
SELECT commit();
ROLLBACK;
 '''
 
vendor_add_template = '''SELECT vendor_add('{vendor}');
'''
 
hardware_add_template = '''SELECT hardware_add('{hardware}');
'''
hardware_set_template = '''SELECT hardware_set_vendor('{hardware}','{vendor}');
SELECT hardware_set_purchase('{hardware}','{purchase}');
SELECT hardware_set_warranty('{hardware}','{warranty}');
'''
host_add_template = '''SELECT host_add('{host}');
'''
host_set_template = '''SELECT host_set_hardware('{host}','{hardware}');
'''

interface_add_template = '''SELECT interface_add('{interface}');
'''
interface_set_template = '''SELECT interface_set_hardware('{interface}','{host}');
SELECT interface_set_ip('{interface}','{ip}');
SELECT interface_set_mac('{interface}','{mac}');
'''

def gen_ips(count):
	ips = list()
	bnum = count/1000 + 1
	bnum = min(bnum,255)
	bstart = random.randint(1,256 - bnum)
	cnum = bnum * 60
	cnum = min(cnum,255)
	cstart = random.randint(1,256 - cnum)
	dnum = cnum * 5
	dnum = min(dnum,255)
	if(count > bnum * cnum * dnum):
		raise Exception, "Too many interfacees to fill with ips."
	dstart = random.randint(1,256 - dnum)
	while (count > 0):				
		biter = 0		
		while(count > 0 and biter < bnum):
			citer = 0
			while (count > 0 and citer < cnum):
				diter = 0
				while (count > 0 and diter < dnum):
					d = dstart
					ip = "10." + str(bstart + biter) + "." + str(cstart + citer) + "." + str(dstart + diter)
					ips.append(ip)
					count = count - 1
					diter = diter + 1
				citer = citer + 1
		biter = biter + 1
	return ips

n = 20

random.seed()
count = random.randint(1,n)
vendors_string = ""
vendor_list = list()
while (count > 0):
	v = "vend" + str(count)
	vendor_list.append(v)
	vendors_string = vendors_string + vendor_add_template.format(vendor = v)
	count = count - 1
	
hardwares_string = ""
count = random.randint(n,3*n)
hardware_list = list()
while (count > 0):
	h = "hardw" + str(count)
	hardware_list.append(h)
	indexv = random.randint(0,len(vendor_list) - 1)
	v = vendor_list[indexv]
	delta = random.randint(0,900)
	purchase_date = datetime.date.today() - datetime.timedelta(days=delta)
	warranty_date = purchase_date + datetime.timedelta(days=600)
	hardwares_string = hardwares_string + hardware_add_template.format(hardware= h) + hardware_set_template.format(hardware = h, vendor = v, purchase = str(purchase_date), warranty = str(warranty_date))
	count = count - 1
	
count = random.randint(2*n,5*n)
hosts_string = ""
host_list = list()
while (count > 0):
	ho = "host" + str(count)
	host_list.append(ho)
	indexh = random.randint(0,len(hardware_list) - 1)
	hw = hardware_list[indexh]
	hosts_string = hosts_string + host_add_template.format(host = ho) + host_set_template.format(host = ho, hardware = hw)
	count = count - 1
	
maxinf = 5
hostnum = len(host_list)	
interfaces_string = ""
ipsnum = maxinf * hostnum	
ips = gen_ips(ipsnum)
while (hostnum > 0):
	enum = random.randint(1,maxinf)
	eiter = 0
	while (hostnum > 0 and eiter < enum):
		inf = "eth" + str(eiter)
		ipindex = random.randint(0, len(ips)-1)		
		ip_addr = ips[ipindex]
		ips.pop(ipindex)
		mac_addr = "01:23:45:67:89:ab"
		#interfaces_string = interface_add_template.format(interface = inf) + interface_set_template.format(interface = inf, host = host_list[hostnum - 1], ip = ip_addr, mac = mac_addr)
		interfaces_string = interfaces_string + interface_add_template.format(interface = inf) + interface_set_template.format(interface = inf, host = host_list[hostnum - 1], ip = ip_addr, mac = mac_addr)
		eiter = eiter + 1
	hostnum = hostnum - 1
		
result = script_template.format(vendors_add = vendors_string, hardware_add = hardwares_string, host_add = hosts_string, interface_add = interfaces_string)

f = open('fill.sql', 'w')
f.write(result)