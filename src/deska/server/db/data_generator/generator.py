#!/usr/bin/python2

import random, datetime
from randdom import Names, Macs, IPv4s 

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

class Generator():

	vendor_add_template = "SELECT vendor_add('{0}');"
	vendor = set()
	hardware = set()
	
	def __init__(self, _count = 2):
		self.count = _count
		self.data = list()

	def add_vendors(self,c = 0):
		if (c is 0):			
			c = self.count
		names = Names("names.txt")
		# gen set of N random (and unique) names
		self.vendor = names.rset(c) 
		str = map(self.vendor_add_template.format, self.vendor)
		self.data.extend(str)


generator = Generator(5)
generator.add_vendors()

# debug
print generator.data
