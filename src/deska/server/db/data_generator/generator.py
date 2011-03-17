#!/usr/bin/python2

import random, datetime
from randdom import Names, Macs, IPv4s, Dates

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

interface_add_template = '''SELECT interface_add('{interface}');
'''
interface_set_template = '''SELECT interface_set_hardware('{interface}','{host}');
SELECT interface_set_ip('{interface}','{ip}');
SELECT interface_set_mac('{interface}','{mac}');
'''

class Generator():

	vendor_add_template = "SELECT vendor_add('{0}');"
	hardware_add_template = "SELECT hardware_add('{0}');"
	hardware_set_template = '''SELECT hardware_set_vendor('{0}','{1}');
SELECT hardware_set_purchase('{0}','{2}');
SELECT hardware_set_warranty('{0}','{3}');
'''
	host_add_template = "SELECT host_add('{0}');"
	host_set_template = "SELECT host_set_hardware('{0}','{1}');"


	def __init__(self, count = 2):
		self.count = count
		self.data = list()

	def add_vendors(self,count = 0):
		if (count == 0):			
			count = self.count
		names = Names("names.txt")
		# gen set of N random (and unique) names
		self.vendor = names.rset(count) 
		str = map(self.vendor_add_template.format, self.vendor)
		self.data.extend(str)
		
	def add_hardwares(self,count = 0):
		if (count == 0):			
			count = self.count * 3
		names = Names("names.txt")
		# gen set of N random (and unique) names
		self.hardware = names.rset(count) 
		str = map(self.hardware_add_template.format, self.hardware)
		self.data.extend(str)
		
		#set part of hardware
		vendor = self.vendor.rlist(count)
		purchase_dates = Dates()
		# get list (not unique) of N dates
		purchase = purchase_dates.rlist(count)
		warranty_dates = Dates(2011,4)
		# get list (not unique) of N dates
		warranty = warranty_dates.rlist(count)
		str = map(self.hardware_set_template.format, self.hardware, vendor, purchase, warranty)
		self.data.extend(str)
		
	def add_hosts(self, count = 0):
		if (count == 0):			
			count = self.count * 4
		names = Names("names.txt")
		# gen set of N random (and unique) names
		self.host = names.rset(count) 
		str = map(self.host_add_template.format, self.host)
		self.data.extend(str)
		
		#set part of host
		hardware = self.hardware.rlist(count)
		str = map(self.host_set_template.format, self.host, hardware)
		self.data.extend(str)
		


generator = Generator(2)
generator.add_vendors()
generator.add_hardwares()
generator.add_hosts()

# debug
print generator.data
