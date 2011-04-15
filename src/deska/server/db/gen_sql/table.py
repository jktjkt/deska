#!/usr/bin/python2

from key_sets import PkSet,Fks
import constants

class Table(constants.Templates):
	def __init__(self,name):
		self.data = dict()
		self.col = dict()
		self.pkset = PkSet()
		self.fks = Fks()
		self.name= name

	def add_column(self,col_name,col_type):
		self.col[col_name] = col_type

	# add pk and unique
	def add_pk(self,con_name,att_name):
		self.pkset[con_name] = att_name

	# add fk 
	def add_fk(self,con_name,att_name,ref_table,ref_att):
		self.fks.add(con_name,att_name,ref_table,ref_att)

	def get_cols_reference_uid(self):
		return self.fks.references_uid()

	def get_col_embed_reference_uid(self):
		return self.fks.embed_col()

	def gen_assign(self,colname):
		return "{col} = new.{col}".format(col= colname)

	def gen_cols_assign(self):
		#TODO remove uid
		assign = map(self.gen_assign,self.col.keys())
		return ",".join(assign)

	def get_columns(self):
		# comma separated list of values
		return ",".join(self.col.keys())

	def gen_pk_constraint(self,con):
		# add version column into key constraint
		self.pkset[con] = "version"
		str = "CONSTRAINT history_{name} UNIQUE(".format(name = con)
		str = str + ",".join(self.pkset[con])
		return str + ") DEFERRABLE INITIALLY DEFERRED"

	def gen_drop_notnull(self):
		nncol = self.col.copy()
		del nncol['uid']
		del nncol['name']
		drop = ""
		for col in nncol:
			drop = drop + "ALTER TABLE {name}_history ALTER {colname} DROP NOT NULL;\n".format(name = self.name, colname = col)
		return drop
	
	def gen_fks(self):
		return self.fks.gen_fk_constraints().format(tbl = self.name)
	
	
	def gen_hist(self):
		constr = ""
		for con in self.pkset:
			constr = constr + ",\n" + self.gen_pk_constraint(con)
		drop = self.gen_drop_notnull()
		return self.hist_string.format(tbl = self.name, constraints = constr) + drop

	def gen_add(self):
		return self.add_string.format(tbl = self.name)

	def gen_add_embed(self, col_name, reftable):
		return self.add_embed_string.format(tbl = self.name, column = col_name, reftbl = reftable, delim = constants.DELIMITER)

	def gen_del(self):
		return self.del_string.format(tbl = self.name, delim = constants.DELIMITER)

	def gen_del_embed(self, col_name, reftable):
		return self.del_embed_string.format(tbl = self.name, column = col_name, reftbl = reftable, delim = constants.DELIMITER)

	def gen_set(self,col_name):
		return self.set_string.format(tbl = self.name,colname = col_name, coltype = self.col[col_name], columns = self.get_columns())

	def gen_set_ref_uid(self,col_name, reftable):
		return self.set_fk_uid_string.format(tbl = self.name, colname = col_name, coltype = self.col[col_name], reftbl = reftable, columns = self.get_columns())

	def gen_get_object_data(self):
		collist = self.col.copy()
		del collist['uid']
		del collist['name']
		if len(collist) == 0:
			return ""

		get_data_string = self.get_data_string
		embed_table = ""
		# replace uid of referenced object its name
		# old column : new column selector
		newcollist = dict()
		for refs in self.fks.att:
			tbl = self.fks.tbl[refs]
			if self.fks.ratt[refs] != list(['uid']):
				raise Exception("ref to not uid column")
			for col in self.fks.att[refs]:
				collist[col] = 'text'
				if "rembed_" in refs:
					# delete this col from output
					del collist[col]
					get_data_string = self.get_embed_data_string
					embed_table = tbl
				else:
					newcol = tbl + "_get_name(" + col + ") as " + col 
					newcollist[col] = newcol
		
		# create col: type dict
		coltypeslist = dict()
		for col in collist:
			coltypeslist[col] = " ".join([col,collist[col]])

		# prepare: values = keys
		keys = collist.keys()
		keys.sort()
		collist = dict(zip(keys,keys))
		# replace old cols with new 
		for col in newcollist:
			collist[col] = newcollist[col]

		coltypes = ",\n".join(coltypeslist.values())
		cols = ",".join(collist.values())
		type_def = self.get_data_type_string.format(tbl = self.name, columns = coltypes)
		cols_def = get_data_string.format(tbl = self.name, columns = cols, embedtbl = embed_table)
		return type_def + "\n" + cols_def

	def gen_get(self,col_name):
		return self.get_string.format(tbl = self.name,colname = col_name, coltype = self.col[col_name])

	def gen_get_name(self):
		return self.get_name_string.format(tbl = self.name)

	def gen_get_uid(self):
		return self.get_uid_string.format(tbl = self.name)
	
	def gen_get_uid_embed(self, refcolumn, reftable):
		return self.get_uid_embed_string.format(tbl = self.name, column = refcolumn, reftbl = reftable, delim = constants.DELIMITER)


	def gen_commit(self):
		#TODO if there is more columns...
		return self.commit_string.format(tbl = self.name, assign = self.gen_cols_assign(), columns = self.get_columns())

	def gen_names(self):
		return self.names_string.format(tbl = self.name)

	def gen_prev_changeset(self):
		return self.prev_changest_string.format(tbl = self.name)

	def gen_prev_changeset_by_name(self):
		return self.prev_changest_by_name_string.format(tbl = self.name)

	def gen_changeset_of_data_version(self):
		return self.changeset_of_data_version_string.format(tbl = self.name)

	def gen_changeset_of_data_version_by_name(self):
		return self.changeset_of_data_version_by_name_string.format(tbl = self.name)
		
	def gen_prev_changeset_by_name_embed(self, refcolumn, reftable):
		return self.prev_changest_by_name_embed_string.format(tbl = self.name, column = refcolumn ,reftbl = reftable)

	def gen_changeset_of_data_version_by_name_embed(self, refcolumn, reftable):
		return self.changeset_of_data_version_by_name_embed_string.format(tbl = self.name, column = refcolumn, reftbl = reftable)
