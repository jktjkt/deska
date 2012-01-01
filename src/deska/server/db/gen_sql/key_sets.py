#!/usr/bin/python2

import string

class PkSet(dict):
	'''class for storing primary key constraints'''
	def __init__(self):
		dict.__init__(self)

	def __setitem__(self,name,att):
		'''own set item - can insert more items into one key'''
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

class Fks:
	'''class for storing foreign key constraints'''
	def __init__(self):
		# attributes in key from local table
		self.att = PkSet()
		# attributes in key from referenced table
		self.ratt = PkSet()
		# dictionary of local table: referenced table
		self.tbl = dict()

	def add(self,name,att,table,ratt):
		'''insert tuple of table, local attribute, referenced table, referenced attribute'''
		self.att[name] = att
		self.tbl[name] = table
		self.ratt[name] = ratt

	def gen_fkcon(self,con):
		'''generate alter table for creating foreign key constraints
		add version column into key constraint'''

		s = "CONSTRAINT history_%(name)s FOREIGN KEY (%(att)s) REFERENCES %(rtbl)s_history(%(ratt)s) DEFERRABLE INITIALLY DEFERRED"
		atts = ",".join(self.att[con])
		ratts = ",".join(self.ratt[con])
		s = s % {'name': con, 'rtbl': self.tbl[con], 'att': atts, 'ratt': ratts}
		return "ALTER TABLE %(tbl)s_history ADD " + s

	def gen_fk_constraints(self):
		'''generate all constraints'''
		constr = ""
		for att in self.att:
			constr = constr + self.gen_fkcon(att) + ";\n"
		return constr

	def references_uid(self):
		'''needed for set function
		returns dictionary of collname (column that references uid in some table) : tablename (table that is referenced)'''
		result = dict()
		for conname in self.att:
			refattributes = self.ratt[conname]
			if ('uid' in refattributes):
				index = self.ratt[conname].index('uid')
				namecol = self.att[conname][index]
				result[namecol] = self.tbl[conname]
		return result

	def embed_col(self):
		'''return embed columns'''
		for conname in self.att:
			if (string.find(conname,'rembed_') == 0):
				return self.att[conname]
		return ""

