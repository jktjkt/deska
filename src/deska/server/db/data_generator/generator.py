#!/usr/bin/python2

import sys
import random, datetime
from randdom import Names, Macs, IPv4s, Dates, Numbers, Interfaces

script_template = '''SET search_path TO api,genproc,history,deska,versioning,production;
SELECT startChangeset();
{data}
SELECT commitChangeset('FINISH');
'''

class Generator():

	vendor_add_template = "SELECT vendor_add('{0}');"
	hardware_add_template = "SELECT hardware_add('{0}');"
	hardware_set_template = '''
SELECT hardware_set_vendor('{0}','{1}');
SELECT hardware_set_purchase('{0}','{2}');
SELECT hardware_set_warranty('{0}','{3}');
'''
	host_add_template = "SELECT host_add('{0}');"
	host_set_template = '''
SELECT host_set_hardware('{0}','{1}');'''
	interface_add_template = "SELECT interface_add('{0}');"
	interface_set_template = '''
SELECT interface_set_ip('{0}','{1}');
SELECT interface_set_mac('{0}','{2}');
'''
	commit_template = '''
SELECT commitChangeset('commit');
SELECT startChangeset();
'''
	def __init__(self, count = 2):
		self.count = count
		self.data = list()

	def add_vendors(self,count = 0):
		if (count == 0):			
			count = self.count
		names = Names("names.txt")
		names.extend(count / 5000)
		# gen set of N random (and unique) names
		self.vendor = names.rset(count) 
		str = map(self.vendor_add_template.format, self.vendor)
		self.data.extend(str)
		
	def add_hardwares(self,count = 0):
		if (count == 0):			
			count = self.count * 3
		names = Names("names.txt")
		names.extend(count / 5000)
		# gen set of N random (and unique) names
		self.hardware = names.rset(count) 
		str1 = map(self.hardware_add_template.format, self.hardware)
		
		#set part of hardware
		vendor = self.vendor.rlist(count)
		purchase_dates = Dates()
		# get list (not unique) of N dates
		purchase = purchase_dates.rlist(count)
		warranty_dates = Dates(2011,4)
		# get list (not unique) of N dates
		warranty = warranty_dates.rlist(count)
		str2 = map(self.hardware_set_template.format, self.hardware, vendor, purchase, warranty)
		str = list()
		for i in range(count):
			str.append(str1[i]+str2[i])

		self.data.extend(str)
		
	def add_hosts(self, count = 0):
		if (count == 0):			
			count = self.count * 4
		names = Names("names.txt")
		names.extend(count / 5000)
		# gen set of N random (and unique) names
		# gen set of N random (and unique) names
		self.host = names.rset(count) 
		str1 = map(self.host_add_template.format, self.host)
		
		#set part of host
		hardware = self.hardware.rlist(count)
		str2 = map(self.host_set_template.format, self.host, hardware)

		str = list()
		for i in range(count):
			str.append(str1[i]+str2[i])
		self.data.extend(str)
		
	def add_interfaces(self, count = 0):
		if (count == 0):			
			count = self.count * 6
		names = Interfaces(self.host)
		# gen set of N random (and unique) names
		self.interface = names.rset(count) 
		str1 = map(self.interface_add_template.format, self.interface)
		
		#set part of interface
		ips = IPv4s()
		ips.setBlock("A",10)
		ips.setBlock("B",0,256)
		ips.setBlock("C",0,256)
		ips.setBlock("D",0,256)
		ip = ips.rset(count)
		macs = Macs()
		mac = macs.rset(count)
		str2 = map(self.interface_set_template.format, self.interface, ip, mac)

		str = list()
		for i in range(count):
			str.append(str1[i]+str2[i])
		self.data.extend(str)
		
	def insert_commits(self, count = 0):
		if (count == 0):			
			count = len(self.data) / 10
		index = Numbers(len(generator.data)).rset(count)
		for i in index:
			self.data.insert(i,self.commit_template)

	def run(self):
		generator.add_vendors()
		generator.insert_commits()
		print script_template.format(data = "\n".join(generator.data))
		generator.data = list()
		generator.add_hardwares()
		generator.insert_commits()
		print script_template.format(data = "\n".join(generator.data))
		generator.data = list()
		generator.add_hosts()
		generator.insert_commits()
		print script_template.format(data = "\n".join(generator.data))
		generator.data = list()
		generator.add_interfaces()
		generator.insert_commits()
		print script_template.format(data = "\n".join(generator.data))

generator = Generator(int(sys.argv[1]))
generator.run()
