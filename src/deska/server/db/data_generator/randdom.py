#!/usr/bin/python2

import random, datetime
from operator import mul

random.seed(777)

class rlist(list):
	# get random item from list
	def ritem(self):
		i = random.randint(0,len(self)-1)
		return list.__getitem__(self,i)

	# get set of random items
	def rset(self,size):
		if size > len(self):
			raise "size larger then items in rlist"
		return rlist(random.sample(self, size))
	
	# get list of random items
	def rlist(self,size):
		ret = rlist()
		for i in range(0,size):
			ret.append(self.ritem())
		return ret
	
	def extend(self,boost):
		if boost < 2:
			return
		l = list()
		for item in self.data:
			for i in range(boost):
				l.append(item + "_" + str(i))
		self.data = l 


class Names(rlist):
	def __init__(self,source):
		f = open(source,"r")
		self.data = list(f.read().split())
		f.close()
	
	def __len__(self):
		return len(self.data)
	
	# random item
	def ritem(self):
		i = random.randint(0,len(self.data)-1)
		return self.data[i]
	
	# random set
	def rset(self,size):
		if size > len(self.data):
			raise "size larger then names in db"
		return rlist(random.sample(self.data, size))
	

class Numbers(rlist):
	def __init__(self, size, start = 0):
		self.end = start + size - 1
		self.start = start
		self.size = size
	
	def __len__(self):
		return self.size
	
	def ritem(self):
		return random.randint(self.start, self.end)
	
	def rset(self,size):
		return rlist(random.sample(range(self.start, self.end),size))

class Macs(rlist):
	def __init__(self):
		self.num = Numbers(256)

	def __len__(self):
		return 256**6

	def hextostr(self,hexstr):
		return hexstr.split('x')[1]
	
	def ritem(self):
		mac = list()
		for i in range(6):
			mac.append(self.num.ritem())
		mac = map(hex,mac)
		mac = map(self.hextostr,mac)
		return ":".join(mac)
	
	def rset(self,size):
		s = set(self.rlist(size))
		while len(s) < size:
			print len(s)
			s.add(self.ritem())
		return rlist(s)


class IPv4s(rlist):
	def __init__(self):
		self.data = dict()
	
	def setBlock(self,block,start,size = 1):
		self.data[block] = Numbers(size,start)

	def __len__(self):
		if len(self.data) != 4:
			raise "bed number of cidr blocks"
		numbers = map(self.data.__getitem__,["A", "B", "C", "D"])
		numbers = map(Numbers.__len__,numbers)
		return reduce(mul,numbers)

	def ritem(self):
		if len(self.data) != 4:
			raise "bed number of cidr blocks"
		numbers = map(self.data.__getitem__,["A", "B", "C", "D"])
		cidr = map(Numbers.ritem,numbers)
		strings = map(str,cidr)
		return ".".join(strings)
	
	def rset(self,size):
		s = set(self.rlist(size))
		while len(s) < size:
			s.add(self.ritem())
		return rlist(s)


class Dates(rlist):
	def __init__(self, start = 2000, size = 10):
		self.Y = Numbers(size, start)
		self.M = Numbers(12, 1)
		# only 28 days...
		self.D = Numbers(28, 1)

	def ritem(self):
		return "{D}/{M}/{Y}".format(M = self.M.ritem(), D = self.D.ritem(), Y = self.Y.ritem())

class Interfaces(rlist):
	def __init__(self,names,num = 4):
		self.names = names
		self.num = Numbers(num)

	def ritem(self):
		return "{name}->eth{num}".format(name = self.names.ritem(), num = self.num.ritem())

	def rset(self,size):
		s = set(self.rlist(size))
		while len(s) < size:
			s.add(self.ritem())
		return rlist(s)

