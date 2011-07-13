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

		if self.embed_into <> "":
			get_data_string = self.get_embed_data_string
			del collist[self.embed_into]
		else:
			get_data_string = self.get_data_string

		#replace uid of referenced table object by its name
		for col in self.refuid_columns:
			if col in collist:
				collist[col] = 'text'

		attributes = collist.keys()
		atttypes = collist.values()

		coltypes = ",\n".join(map("{0} {1}".format,attributes, atttypes))

		dattributes = map("data.{0}".format,attributes)
		dcols = ",".join(dattributes)

		for col in self.refuid_columns:
			if col in collist:
				pos = attributes.index(col)
				attributes[pos] = "{1}_get_name({0}) AS {0}".format(col, self.refuid_columns[col])
		
		cols = ",".join(attributes)
		type_def = self.get_data_type_string.format(tbl = self.name, columns = coltypes)
		cols_def = get_data_string.format(tbl = self.name, columns = cols, data_columns = dcols, embedtbl = self.embed_into)
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
		"""gen_commit_templated generates commit function for templated tables.
		
		Differs from untemplated version by resolving data from templates and after that updating production.
		Commit of template modificationb should do changes in templated objects.
		"""
		collist = self.col.copy();
		
		del collist['template']
		cols_ex_templ = ','.join(collist)
		del collist['name']
		del collist['uid']


		rddvcols = collist.keys()
		rddv_list = list()
		for col in rddvcols:
			if col == self.embed_into:
				rddv_list.append( "rd." + self.embed_into)
			else:
				rddv_list.append(("COALESCE(rd.{0},dv.{0}) AS {0}".format(col)))
				
		if len(rddv_list) > 0:
			rddvcoal = ',\n'.join(rddv_list) + ','
	
		cols = ','.join(collist)
		
		#table tbl is templated by table tbl_template
		#table tbl_template is templated by tbl_template
		if self.name.endswith("_template"):
			templ_table = self.name
		else:
			templ_table = self.name + "_template"
			
		commit_templated_string = self.commit_templated_string
		if self.templates is "":
		#table is template, modification should be propagated to templated kind
			return self.commit_templated_string.format(tbl = self.name, template_tbl = templ_table, assign = self.gen_cols_assign(), columns = cols, rd_dv_coalesce = rddvcoal, columns_except_template = cols_ex_templ)
		else:
			return self.commit_kind_template_string.format(tbl = self.templates, template_tbl = templ_table, assign = self.gen_cols_assign(), columns = cols, rd_dv_coalesce = rddvcoal, columns_except_template = cols_ex_templ)

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
		
		cols_ex_template_dict = self.col.copy()
		del cols_ex_template_dict['uid']
		del cols_ex_template_dict['name']
		del cols_ex_template_dict['template']

		#for multiple data we dont want to loos data about embed into columns and we would like to left uid columns unresolved
		multiple_ticols = ',\n'.join(list(map("{0} {1}".format,cols_ex_template_dict.keys(),cols_ex_template_dict.values())))
		
		#list of columns of given kind
		collist = cols_ex_template_dict.keys()
		
		#table tbl is templated by table tbl_template
		#table tbl_template is templated by tbl_template
		if self.name.endswith("_template"):
			templ_table = self.name
		else:
			templ_table = self.name + "_template"
		
		#in case of multiple data we would like to select all columns in table
		multiple_columns = ",".join(collist)
		#table which is thatone embed into
		if self.embed_into <> "":
			resolved_object_data_string = self.resolved_object_data_embed_string
			resolved_object_data_template_info_string = self.resolved_object_data_template_info_embed_string
			multiple_rd_dv_coalesce_list = list()
			for col in collist:
				if col == self.embed_into:
					multiple_rd_dv_coalesce_list.append("rd.{0} AS {0}".format(col))
				else:
					multiple_rd_dv_coalesce_list.append("COALESCE(rd.{0},dv.{0}) AS {0}".format(col))
			collist.remove(self.embed_into)
			del cols_ex_template_dict[self.embed_into]
		else:
			resolved_object_data_string = self.resolved_object_data_string
			resolved_object_data_template_info_string = self.resolved_object_data_template_info_string
			multiple_rd_dv_coalesce_list = list(map("COALESCE(rd.{0},dv.{0}) AS {0}".format,collist))
			
		multiple_rd_dv_coalesce = ",".join(multiple_rd_dv_coalesce_list)
		cols = ','.join(collist)
		columns_ex_template = list(collist)
		
		data_attributes = map('data.{0}'.format, collist)
		#should be in right order at the last position
		dcols = ','.join(data_attributes) + ', data.template'

		# rd_dv_coalesce =coalesce(rd.vendor,dv.vendor),coalesce(rd.purchase,dv.purchase), ...
		if len(collist) > 0:
			rddvcoal = ',\n'.join(list(map("COALESCE(rd.{0},dv.{0}) AS {0}".format,collist)))
		
		# replace uid of referenced object its name
		for col in self.refuid_columns:
			if col in collist:
				pos = collist.index(col)
				collist[pos] = "{0}_get_name({0}) AS {0}".format(col)
				cols_ex_template_dict[col] = "text"
		
		cols_ex_templ = ",".join(collist)
		
		ticols = ',\n'.join(list(map("{0} {1}".format,cols_ex_template_dict.keys(),cols_ex_template_dict.values())))
		templ_cols = ',\n'.join(list(map("{0}_templ {1}".format,cols_ex_template_dict.keys(),['text']*len(cols_ex_template_dict))))
		case_col_string = '''
		CASE	WHEN {0} IS NULL THEN NULL 
			ELSE name
		END AS {0}_templ'''
		case_cols = ','.join(list(map(case_col_string.format,cols_ex_template_dict.keys())))
	
		templ_case_cols_str = '''
		CASE	WHEN rd.{0}_templ IS NOT NULL THEN rd.{0}_templ
			WHEN rd.{0}_templ IS NULL AND dv.{0} IS NOT NULL THEN dv.name
			ELSE NULL
		END AS {0}_templ'''		
		templ_case_list = list()
		for col in cols_ex_template_dict.keys():
			if col == self.embed_into:
				templ_case_list.append("rd.{0}_templ AS {0}_templ".format(col))
			else:
				templ_case_list.append(templ_case_cols_str.format(col))
		templ_case_cols = ','.join(templ_case_list)
		
		templ_columns_list = list(map("{0}_templ".format, cols_ex_template_dict.keys()))
		cols_templ = ','.join(templ_columns_list)
		#SELECT {columns_ex_templ}, {columns_templ}, {templ_tbl}_get_name(orig_template) AS template INTO {data_columns}
		all_columns = columns_ex_template
		all_columns.extend(templ_columns_list)
		all_columns.append('template')
		dticols = ','.join(list(map("data.{0}".format,all_columns)))
		
		templ_info_type = self.resolved_data_template_info_type_string.format(tbl = self.name, columns = ticols, templ_columns = templ_cols )
		resolve_object_data_fce = resolved_object_data_string.format(tbl = self.name, columns = cols, columns_ex_templ = cols_ex_templ, rd_dv_coalesce = rddvcoal, templ_tbl = templ_table, data_columns = dcols)
		resolve_data_template_info_fce = self.resolved_data_template_info_string.format(tbl = self.name, templ_tbl = templ_table, columns = multiple_columns, rd_dv_coalesce = multiple_rd_dv_coalesce, columns_ex_templ = multiple_columns, case_columns = case_cols, templ_case_columns = templ_case_cols, columns_templ = cols_templ)
		resolve_object_data_template_info = resolved_object_data_template_info_string.format(tbl = self.name, templ_tbl = templ_table, columns = cols, rd_dv_coalesce = rddvcoal, columns_ex_templ = cols_ex_templ, case_columns = case_cols, templ_case_columns = templ_case_cols, columns_templ = cols_templ, data_columns = dticols)
		multiple_object_data_templ_info_type = self.multiple_resolved_data_template_info_type_string.format(tbl = self.name, columns = multiple_ticols, templ_columns = templ_cols )
		multiple_data_type = self.multiple_resolved_data_type_string.format(tbl = self.name, columns = multiple_ticols)
		resolve_data_fce = self.resolved_data_string.format(tbl = self.name, templ_tbl = templ_table, columns = multiple_columns, rd_dv_coalesce = multiple_rd_dv_coalesce, columns_ex_templ = multiple_columns)
		return  templ_info_type + '\n' + multiple_data_type + '\n' + multiple_object_data_templ_info_type + '\n' + resolve_object_data_fce  + '\n' + resolve_data_fce + '\n' + resolve_data_template_info_fce + '\n' + resolve_object_data_template_info
		
			
	def gen_resolved_data_diff(self):
		if self.name.endswith("_template"):
			templ_table = self.name
		else:
			templ_table = self.name + "_template"
		#	COALESCE(rd.warranty,dv.warranty) AS warranty, COALESCE(rd.purchase,dv.purchase) AS purchase, 

		collist = self.col.keys()
		collist.remove('template')
		collist.remove('uid')
		collist.remove('name')

		if self.embed_into <> "":
			rd_dv_list = list()
			for col in collist:
				if col == self.embed_into:
					rd_dv_list.append("rd." + col)
				else:
					rd_dv_list.append("COALESCE(rd.{0},dv.{0}) AS {0}".format(col))
			rd_dv_coal = ',\n'.join(rd_dv_list)
		else:
			rd_dv_coal = ',\n'.join(list(map("COALESCE(rd.{0},dv.{0}) AS {0}".format,collist)))
		
		columns = ','.join(collist)
		
		att_name_type = list()
		for col in collist:
			att_name_type.append("{0} {1}".format(col, self.col[col]))
		columns_types = ",\n".join(att_name_type)
		diff_type = self.diff_data_type_str.format(tbl = self.name, col_types = columns_types)
		changeses_function = self.data_resolved_changes_function_string.format(tbl = self.name, templ_tbl = templ_table, rd_dv_coalesce = rd_dv_coal, columns_ex_templ = columns)

		#template, name must be present
		collist = self.col.keys()
		collist.remove('uid')
		select_new_attributes = list(map("chv.{0} AS new_{0}".format,collist))
		select_new_attributes.append("chv.dest_bit AS new_dest_bit")
		#dest_bit from resolved data is allways 0
		select_old_attributes = list(map("dv.{0} AS old_{0}".format,collist))
		select_old_attributes.append("CAST('0' AS bit(1)) AS old_dest_bit")
		select_old_new_objects_attributes = ",".join(select_old_attributes) + "," + ",".join(select_new_attributes)
		init_function = self.diff_init_resolved_function_string.format(tbl = self.name, diff_columns = select_old_new_objects_attributes)
		current_changeset_diff = self.diff_changeset_init_resolved_function_string.format(tbl = self.name, diff_columns = select_old_new_objects_attributes, columns_ex_templ = columns, templ_tbl = templ_table, rd_dv_coalesce = rd_dv_coal)
		return diff_type + '\n' + changeses_function + '\n' + init_function + '\n' + current_changeset_diff