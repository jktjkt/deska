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

