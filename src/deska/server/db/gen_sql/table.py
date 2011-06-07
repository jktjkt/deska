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
		return self.del_string.format(tbl = self.name, columns = self.get_columns())
		
	def gen_undel(self):
		return self.undel_string.format(tbl = self.name)
		
	def gen_set(self,col_name):
		return self.set_string.format(tbl = self.name,colname = col_name, coltype = self.col[col_name], columns = self.get_columns())
		
	def gen_set_name_embed(self, col_name, reftable):
		return self.set_name_embed_string.format(tbl = self.name, refcolumn = col_name, columns = self.get_columns(), reftbl = reftable, delim = constants.DELIMITER)

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
	
	#generates function that returns all changes of columns between two versions
	def gen_diff_set_attribute(self):
		#collist is list of columns
		collist = self.col.copy()
		del collist['uid']
		#in addition to columns in production table we need although dest_bit column
		collist['dest_bit'] = 'bit(1)'

		#all attributes of old_data and new_data object (attributes are in collist), used in for clause
		#old_data.name, old_data.vendor, ..., new_data.name, new_data.vendor, ...
		old_new_attributes = list(map("old_data.{0}".format,collist))
		old_new_attributes.extend(list(map("new_data.{0}".format,collist)))
		old_new_attributes_string = ",".join(old_new_attributes)
		
		#data which we would like to select from diff_data table into old_data and new_data
		#old_name, old_vendor, ..., old_note, new_name, new_vendor, ...
		select_old_new_attributes = list(map("old_{0}".format,collist))
		select_old_new_attributes.extend(list(map("new_{0}".format,collist)))
		select_old_new_attributes_string = ",".join(select_old_new_attributes)
		
		#we dont want to check changes of name and dest_bit attributes
		del collist['name']
		del collist['dest_bit']

		#for columns which are references to uid of another object
		#we would like to know change of referenced name not change of referenced uid
		refuid_collist = list()
		cols_changes = ""
		#we would like to find all columns that references uid of some kind
		#we find them in foreign keys
		for refs in self.fks.att:
			tbl = self.fks.tbl[refs]
			#value of every column that refer to uid should be replaced by name of object with given uid
			for i in range(len(self.fks.att[refs])):
				if self.fks.ratt[refs][i] == 'uid':
					col = self.fks.att[refs][i]
					if col in collist:
						del collist[col]
						#columns that references uid
						cols_changes = cols_changes + self.one_column_change_ref_uid_string.format(reftbl = tbl, column = col)
		
		#for all remaining columns we generate if clause to find possible changes
		for col in collist:
			cols_changes = cols_changes + self.one_column_change_string.format(column = col)

		return self.diff_set_attribute_string.format(tbl = self.name, columns_changes = cols_changes, old_new_obj_list = old_new_attributes_string, select_old_new_list = select_old_new_attributes_string)

	#generates function which prepairs temp table with diff data
	#diff data table is used in diff_created data, diff_deleted and diff_set functions
	def gen_diff_init_function(self):
		#dv.uid AS old_uid,dv.name AS old_name, dv.vendor AS old_vendor ..., chv.uid AS new_uid,chv.name AS new_name,chv.vendor AS new_vendor ...
		#with dv (diff version), chv(changes between versions) prefix
		collist = self.col.copy()
		collist['dest_bit'] = 'bit(1)'
		select_old_attributes = list(map("dv.{0} AS old_{0}".format,collist))
		select_new_attributes = list(map("chv.{0} AS new_{0}".format,collist))
		select_old_new_objects_attributes = ",".join(select_old_attributes) + "," + ",".join(select_new_attributes)
		return self.diff_init_function_string.format(tbl = self.name, diff_columns = select_old_new_objects_attributes) + self.diff_changeset_init_function_string.format(tbl = self.name, diff_columns = select_old_new_objects_attributes)
		
	#generates function terminate_diff which is oposite of init_diff
	#drops diff_data table
	def gen_diff_terminate_function(self):
		return self.diff_terminate_function_string.format(tbl = self.name)

	def gen_get_name(self):
		return self.get_name_string.format(tbl = self.name)

	def gen_get_name_embed(self, refcolumn, reftable):
		return self.get_name_embed_string.format(tbl = self.name, column = refcolumn, reftbl = reftable, delim = constants.DELIMITER)

	def gen_get_uid(self):
		return self.get_uid_string.format(tbl = self.name)
	
	def gen_get_uid_embed(self, refcolumn, reftable):
		return self.get_uid_embed_string.format(tbl = self.name, column = refcolumn, reftbl = reftable, delim = constants.DELIMITER)

	def gen_commit_templated(self):
		#TODO if there is more columns...
		collist = self.col.copy();
		
		del collist['template']
		cols_ex_templ = ','.join(collist)
		del collist['name']
		del collist['uid']
		
		rddvcoal = ""
		if len(collist) > 0:
			rddvcoal = ',\n'.join(list(map("COALESCE(rd.{0},dv.{0}) AS {0}".format,collist))) + ','
	
		cols = ','.join(collist)
		
		if self.name.endswith("_template"):
			templ_table = self.name
		else:
			templ_table = self.name + "_template"
			
		return self.commit_templated_string.format(tbl = self.name, template_tbl = templ_table, assign = self.gen_cols_assign(), columns = cols, rd_dv_coalesce = rddvcoal, columns_except_template = cols_ex_templ)

	def gen_commit(self):
		#TODO if there is more columns...
		return self.commit_string.format(tbl = self.name, assign = self.gen_cols_assign(), columns = self.get_columns())

	def gen_names(self):
		return self.names_string.format(tbl = self.name)

	def gen_names_embed(self, refcolumn, reftable):
		return self.names_embed_string.format(tbl = self.name, column = refcolumn, reftbl = reftable, delim = constants.DELIMITER)

	#generates function which finds all deleted objects in diff data table
	def gen_diff_deleted(self):
		return self.diff_deleted_string.format(tbl = self.name)
	
	#generates function which finds all created objects in diff data table
	def gen_diff_created(self):
		return self.diff_created_string.format(tbl = self.name)
		
	def gen_data_version(self):
		return self.data_version_function_string.format(tbl = self.name)
	
	def gen_data_changes(self):
		return self.data_changes_function_string.format(tbl = self.name)

	def gen_resolved_data(self):
		"""gen_resolved_data is function for generating {tbl}_resolved_data(name_ text, version bigint = 0)
			
		Function is called only for tables that could be templated by some template (has column template).
		"""
		collist = self.col.keys()
		collist.remove('uid')
		collist.remove('name')
		cols = ','.join(collist)
		
		if self.name.endswith("_template"):
			templ_table = self.name
		else:
			templ_table = self.name + "_template"
		
		#table which is thatone embed into
		if self.embed_into <> "":
			resolved_data_string = self.resolved_data_embed_string
		else:
			resolved_data_string = self.resolved_data_string

		collist.remove('template')

		# rd_dv_coalesce =coalesce(rd.vendor,dv.vendor),coalesce(rd.purchase,dv.purchase), ...
		rddvcoal = ','.join(list(map("COALESCE(rd.{0},dv.{0})".format,collist)))
		# replace uid of referenced object its name
		# old column : new column selector
		for col in self.refuid_columns:
			if col in collist:
				pos = collist.index(col)
				collist[pos] = "{0}_get_name({0}) AS {0}".format(col)
		
		cols_ex_templ = ",".join(collist)
			
		return resolved_data_string.format(tbl = self.name, columns = cols, columns_ex_templ = cols_ex_templ, rd_dv_coalesce = rddvcoal, templ_tbl = templ_table)
		