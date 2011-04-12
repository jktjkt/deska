#!/usr/bin/python2

import string

# class for storing primary key constraints
class PkSet(dict):
	def __init__(self):
		dict.__init__(self)
	
	# own set item - can insert more items into one key
	def __setitem__(self,name,att):
		if self.has_key(name):
			# if key already exist, append value into item list
			s = dict.__getitem__(self,name)
			s.append(att)
			dict.__setitem__(self,name,s)
		else:
			# if key does'n exist, insert list with one item
			s = list()
			s.append(att)
			dict.__setitem__(self,name,s)

# class for storing foreign key constraints
class Fks():
	def __init__(self):
		# attributes in key from local table
		self.att = PkSet()
		# attributes in key from referenced table
		self.ratt = PkSet()
		# dictionary of local table: referenced table
		self.tbl = dict()
	
	# insert tuple of table, local attribute, referenced table, referenced attribute
	def add(self,name,att,table,ratt):
		self.att[name] = att
		self.tbl[name] = table
		self.ratt[name] = ratt
	
	# generate alter table for creating foreign key constraints
	def gen_fkcon(self,con):
		# add version column into key constraint

		# workround for add fks to objects from earlier changesets
		#self.att[con] = "version"
		#self.ratt[con] = "version"

		str = "CONSTRAINT history_{name} FOREIGN KEY ({att}) REFERENCES {rtbl}_history({ratt}) DEFERRABLE INITIALLY DEFERRED"
		atts = ",".join(self.att[con])
		ratts = ",".join(self.ratt[con])
		str = str.format(name = con,rtbl = self.tbl[con],att = atts, ratt = ratts)
		return "ALTER TABLE {tbl}_history ADD " + str
	
	# generate all constraints
	def gen_fk_constraints(self):
		constr = ""
		for att in self.att:
			constr = constr + self.gen_fkcon(att) + ";\n" 
		return constr
			
	#needed for set function
	#returns dictionary of collname (column that references uid in some table) : tablename (table that is referenced)
	def references_uid(self):
		result = dict()
		for conname in self.att:
			refattributes = self.ratt[conname]
			if ('uid' in refattributes):
				index = self.ratt[conname].index('uid')
				namecol = self.att[conname][index]
				result[namecol] = self.tbl[conname]
		return result

	# FIXME: add comment
	def embed_col(self):
		for conname in self.att:
			if (string.find(conname,'rembed_') == 0):
				return self.att[conname]
		return ""

