#!/usr/bin/python2

import random, datetime
class Names():
	def __init__(self,source):
		f = open(source,"r")
		self.data = list(f.read().split())
		print "{0} names read.".format(len(self.data))
		f.close()
	# random item
	def ritem(self):
		i = random.randint(0,len(self.data)-1)
		return self.data[i]
	
	# random set
	def rset(self,size):
		if size > len(self.data):
			raise "size larger then names in db"
		return random.sample(self.data, size)
	

class Numbers:
	def __init__(self, size, start = 0):
		self.end = start + size - 1
		self.start = start
	
	def ritem(self):
		return random.randint(self.start, self.end)
	
	def rset(self,size):
		return random.sample(range(self.start, self.end),size)

class Macs:
	def __init__(self):
		self.num = Numbers(256)

	def hextostr(self,hexstr):
		return hexstr.split('x')[1]
	
	def ritem(self):
		mac = list()
		for i in range(6):
			mac.append(self.num.ritem())
		mac = map(hex,mac)
		mac = map(self.hextostr,mac)
		return ":".join(mac)

class IPv4s:
	def __init__(self):
		self.data = dict()
	
	def setBlock(self,block,start,size = 1):
		self.data[block] = Numbers(size,start)

	def ritem(self):
		if len(self.data) != 4:
			raise "bed number of cidr blocks"
		numbers = map(self.data.__getitem__,["A", "B", "C", "D"])
		cidr = map(Numbers.ritem,numbers)
		strings = map(str,cidr)
		return ".".join(strings)

class Dates:
	def __init__(self, start = 2000, size = 10):
		self.Y = Numbers(size, start)
		self.M = Numbers(12, 2)
		# only 28 days...
		self.D = Numbers(28, 1)

	def ritem(self):
		return "{M}/{D}/{Y}".format(M = self.M.ritem(), D = self.D.ritem(), Y = self.Y.ritem())


