#!/usr/bin/python2

from key_sets import PkSet,Fks
import constants

class Table(constants.Templates):
	"""In the class Table are generated all the stored functions for the maintanance of this table.

	"""
	def __init__(self,name):
		"""Constructor of the class Table"""
		self.data = dict()
		self.col = dict()
		self.pkset = PkSet()
		self.fks = Fks()
		self.name= name

	def add_column(self,col_name,col_type):
		"""Adds the col_name column and its type to the dict with columns"""
		self.col[col_name] = col_type

	# add pk and unique
	def add_pk(self,con_name,att_name):
		"""Adds the constraints con_name to the PkSet structure pkset"""
		self.pkset[con_name] = att_name

	# add fk
	def add_fk(self,con_name,att_name,ref_table,ref_att):
		"""Adds the constraint con_name to the Fks structure fks."""
		self.fks.add(con_name,att_name,ref_table,ref_att)

	def get_cols_reference_uid(self):
		"""Gets the columns that references some uid column in some table"""
		return self.fks.references_uid()

	def get_col_embed_reference_uid(self):
		"""From those columns that references some uid column gets the column that references the uid with embed_into relation"""
		return self.fks.embed_col()

	def gen_assign(self,colname):
		"""Generates assignment for the colmn with colname."""
		return "%(col)s = new.%(col)s" % {'col': colname}

	def gen_cols_assign(self):
		"""Generates assignment of all columns in table."""
		#TODO remove uid
		assign = map(self.gen_assign,self.col.keys())
		return ",".join(assign)

	def get_columns(self):
		"""Returns comma separated list of columns."""
		# comma separated list of values
		return ",".join(self.col.keys())

	def gen_pk_constraint(self,con):
		"""Generates unique constraint for the history table, analogue of the unique contraint in the production."""
		# add version column into key constraint
		self.pkset[con] = "version"
		str = "CONSTRAINT \"%(name)s\" UNIQUE(" % {'name': con}
		str = str + ",".join(self.pkset[con])
		return str + ") DEFERRABLE INITIALLY IMMEDIATE"

	def gen_drop_notnull(self):
		"""Generates dropping of not null constraints. Attributes in row of history tables are filled one by one and rows could be leaky."""
		nncol = self.col.copy()
		if 'uid' in nncol:
			del nncol['uid']
		if 'name' in nncol:
			del nncol['name']
		drop = ""
		for col in nncol:
			drop = drop + "ALTER TABLE %(name)s_history ALTER %(colname)s DROP NOT NULL;\n" % {'name': self.name, 'colname': col}
		return drop

	def gen_fks(self):
		"""Generates foreign key constraints, analogue of foreign keys in production."""
		return self.fks.gen_fk_constraints() % {'tbl': self.name}

	def gen_hist(self):
		"""Generates the history table for this table.
		Generates create table like the table in the production, create unique constraints and drop of not null constraints.
		"""
		constr = ""
		for con in self.pkset:
			constr = constr + ",\n" + self.gen_pk_constraint(con)
		drop = self.gen_drop_notnull()
		return self.hist_string % {'tbl': self.name, 'constraints': constr} + drop

	def gen_add(self):
		"""Generates the stored procedure for adding new object into this table."""
		return self.add_string % {'tbl': self.name}

	def gen_add_embed(self, col_name, reftable):
		"""Generates the stored procedure for adding new object into this table.
		This function is used for tables that are embed into another one.
		"""
		return self.add_embed_string % {'tbl': self.name, 'column': col_name, 'reftbl': reftable, 'delim': constants.DELIMITER}

	def gen_del(self):
		"""Generates the stored procedure for deleting object from this table"""
		return self.del_string % {'tbl': self.name, 'columns': self.get_columns()}

	def gen_undel(self):
		"""Generates the stored procedure which anable restoring objects that were deleted in current changeset."""
		return self.undel_string % {'tbl': self.name}

	def gen_set(self,col_name):
		"""Generates the stored procedure that sets value of attribute named col_name."""
		return self.set_string % {'tbl': self.name, 'colname': col_name, 'coltype': self.col[col_name], 'columns': self.get_columns()}

	def gen_set_name_embed(self, col_name, reftable):
		"""Generates the set_name stored procedure for setting name of embed kinds."""
		return self.set_name_embed_string % {'tbl': self.name, 'refcolumn': col_name, 'columns': self.get_columns(), 'reftbl': reftable, 'delim': constants.DELIMITER}

	def gen_set_ref_uid(self,col_name, reftable):
		"""Generates the set_attribute stored procedure for those columns in table that references some uid column of some table."""
		return self.set_fk_uid_string % {'tbl': self.name, 'colname': col_name, 'coltype': self.col[col_name], 'reftbl': reftable, 'columns': self.get_columns()}
		
	def gen_set_refuid_set(self, reftable):
		"""Generates set function for columns that contains set of identifiers that references."""
		return self.set_refuid_set_string % {'tbl': self.name, 'ref_tbl': reftable, 'colname': reftable, 'columns': self.get_columns()}
		
	def gen_refuid_set_insert(self, reftable):
		"""Generates function to insert one item to set of identifiers."""
		return self.refuid_set_insert_string % {'tbl': self.name, 'ref_tbl': reftable, 'columns': self.get_columns()}

	def gen_refuid_set_remove(self, reftable):
		"""Generates function to insert one item to set of identifiers."""
		return self.refuid_set_remove_string % {'tbl': self.name, 'ref_tbl': reftable, 'columns': self.get_columns()}


	def gen_get_object_data(self):
		"""Generates get_object_data stored function that returns data stored in this table"""
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
				if col in self.refers_to_set:
					collist[col] = 'text[]'
				else:
					collist[col] = 'text'

		attributes = collist.keys()
		atttypes = collist.values()

		coltypes = ",\n".join(["%s %s" % (x, y) for (x, y) in zip(attributes, atttypes)])

		dcols = ",".join(["data.%s" % attr for attr in attributes])

		get_identifier_set_fns = ""
		for col in self.refers_to_set:
			get_identifier_set_fns = get_identifier_set_fns + self.get_refuid_set_string % {'tbl': self.name, 'colname': col, 'ref_tbl': col}

		for col in self.refuid_columns:
			if col in collist:
				pos = attributes.index(col)
				if col in self.refers_to_set:
					attributes[pos] = "%(tbl)s_get_%(refuid)s(uid) AS %(col)s" % \
						{'tbl': self.name, 'col': col, 'refuid': self.refuid_columns[col]}
				else:
					attributes[pos] = "%(refuid)s_get_name(%(col)s) AS %(col)s" % \
						{'col': col, 'refuid': self.refuid_columns[col]}

		cols = ",".join(attributes)
		type_def = self.get_data_type_string % {'tbl': self.name, 'columns': coltypes}
		cols_def = get_data_string % {'tbl': self.name, 'columns': cols, 'data_columns': dcols, 'embedtbl': self.embed_into}
		
		return get_identifier_set_fns + "\n" + type_def + "\n" + cols_def

	#generates function that returns all changes of columns between two versions
	def gen_diff_set_attribute(self):
		"""Generates function that returns all changes of all columns in this table between two versions."""
		#collist is list of columns
		collist = self.col.copy()
		#in addition to columns in production table we need although dest_bit column
		collist['dest_bit'] = 'bit(1)'

		#all attributes of old_data and new_data object (attributes are in collist), used in for clause
		#old_data.name, old_data.vendor, ..., new_data.name, new_data.vendor, ...
		old_new_attributes = ["old_data.%s" % col for col in collist]
		old_new_attributes.extend(["new_data.%s" % col for col in collist])
		old_new_attributes_string = ",".join(old_new_attributes)

		#data which we would like to select from diff_data table into old_data and new_data
		#old_name, old_vendor, ..., old_note, new_name, new_vendor, ...
		select_old_new_attributes = ["old_%s" % col for col in collist]
		select_old_new_attributes.extend(["new_%s" % col for col in collist])
		select_old_new_attributes_string = ",".join(select_old_new_attributes)

		#we dont want to check changes of name and dest_bit attributes
		del collist['uid']
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
						if col in self.refers_to_set:
							#if refers to set - sets should be compared
							cols_changes = cols_changes + self.one_column_change_ref_set_string % {'tbl': self.name, 'reftbl': tbl, 'column': col}
						else:
							cols_changes = cols_changes + self.one_column_change_ref_uid_string % {'reftbl': tbl, 'column': col}

		#for all remaining columns we generate if clause to find possible changes
		for col in collist:
			cols_changes = cols_changes + self.one_column_change_string % {'column': col}

		return self.diff_set_attribute_string % {'tbl': self.name, 'columns_changes': cols_changes, 'old_new_obj_list': old_new_attributes_string, 'select_old_new_list': select_old_new_attributes_string}

	#generates function which prepairs temp table with diff data
	#diff data table is used in diff_created data, diff_deleted and diff_set functions
	def gen_diff_init_function(self):
		"""Generates function which prepairs temp table with diff data for this table"""
		#dv.uid AS old_uid,dv.name AS old_name, dv.vendor AS old_vendor ..., chv.uid AS new_uid,chv.name AS new_name,chv.vendor AS new_vendor ...
		#with dv (diff version), chv(changes between versions) prefix
		collist = self.col.copy()
		collist['dest_bit'] = 'bit(1)'
		select_old_attributes = ["dv.%s AS old_%s" % (col, col) for col in collist]
		select_new_attributes = ["chv.%s AS new_%s" % (col, col) for col in collist]
		select_old_new_objects_attributes = ",".join(select_old_attributes) + "," + ",".join(select_new_attributes)
		
		inner_init_diff_str = "PERFORM %(tbl)s_%(ref_tbl)s_init_diff(from_version, to_version);"
		inner_init_diff = ""
		inner_init_diff_functions = ""
		for reftbl in self.refers_to_set:
			inner_init_diff_functions = inner_init_diff_functions + self.diff_init_refuid_set_string % {'tbl': self.name,'ref_tbl': reftbl}
			inner_init_diff = inner_init_diff + inner_init_diff_str % {'tbl': self.name, 'ref_tbl': reftbl}
		
		return  inner_init_diff_functions + \
				self.diff_init_function_string % {'tbl': self.name, 'diff_columns': select_old_new_objects_attributes, 'inner_tables_diff': inner_init_diff} + \
				self.diff_changeset_init_function_string % {'tbl': self.name, 'diff_columns': select_old_new_objects_attributes}

	#generates function terminate_diff which is oposite of init_diff
	#drops diff_data table
	def gen_diff_terminate_function(self):
		"""Generates function terminate_diff.
		Terminate_diff is oposite of init_diff."""
		
		inner_terminate_diff = ""
		inner_terminate_diff_str = "PERFORM %(tbl)s_%(ref_tbl)s_terminate_diff();"
		inner_diff_terminate_fn = ""
		for reftbl in self.refers_to_set:
			inner_diff_terminate_fn = inner_diff_terminate_fn + self.diff_terminate_refuid_set_function_string % {'tbl': self.name, 'ref_tbl': reftbl}
			inner_terminate_diff = inner_terminate_diff + inner_terminate_diff_str % {'tbl': self.name, 'ref_tbl': reftbl}
		
		return inner_diff_terminate_fn + self.diff_terminate_function_string % {'tbl': self.name, 'inner_temrinate_diff': inner_terminate_diff}

	def gen_get_name(self):
		"""Generates stored function that returns the name of the object with the given uid."""
		return self.get_name_string % {'tbl': self.name}

	def gen_get_name_embed(self, refcolumn, reftable):
		"""Generates stored function that returns the name of the object with the given uid.
		This function is used for the tables that are embed into another.
		"""
		return self.get_name_embed_string % {'tbl': self.name, 'column': refcolumn, 'reftbl': reftable, 'delim': constants.DELIMITER}

	def gen_get_uid(self):
		"""Generates stored function that returns the uid of the object with the given name."""
		return self.get_uid_string % {'tbl': self.name}

	def gen_get_uid_embed(self, refcolumn, reftable):
		"""Generates stored function that returns the uid of the object with the given name.
		This function is used for the tables that are embed into another.
		"""
		return self.get_uid_embed_string % {'tbl': self.name, 'column': refcolumn, 'reftbl': reftable, 'delim': constants.DELIMITER}

	def gen_commit_templated(self):
		"""gen_commit_templated generates commit function for templated tables.

		Differs from untemplated version by resolving data from templates and after that updating production.
		Commit of template modificationb should do changes in templated objects.
		"""
		collist = self.col.copy();
		
		if self.templates == "":
			templ_col = 'template_' + self.name
		else:
			templ_col = 'template_' + self.templates
		del collist[templ_col]
		
		cols_ex_templ = ','.join(collist)
		del collist['name']
		del collist['uid']


		rddvcols = collist.keys()
		rddv_list = list()
		for col in rddvcols:
			if (col == self.embed_into) or (col in self.merge_with):
				rddv_list.append( "rd." + col)
			else:
				rddv_list.append(("COALESCE(rd.%s, dv.%s) AS %s" % (col, col, col)))

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
			return self.commit_templated_string % {'tbl': self.name, 'template_tbl': templ_table, 'assign': self.gen_cols_assign(), 'columns': cols, 'rd_dv_coalesce': rddvcoal, 'columns_except_template': cols_ex_templ, 'template_column': templ_col}
		else:
			return self.commit_kind_template_string % {'tbl': self.templates, 'template_tbl': templ_table, 'assign': self.gen_cols_assign(), 'columns': cols, 'rd_dv_coalesce': rddvcoal, 'columns_except_template': cols_ex_templ, 'template_column': templ_col}

	def gen_commit(self):
		"""Generates commit function for this table.
		Commit function adds new data, deletes data that were deleted and updates modified data in the corresponding history table.
		"""
		#TODO if there is more columns...
		return self.commit_string % {'tbl': self.name, 'assign': self.gen_cols_assign(), 'columns': self.get_columns()}

	def gen_names(self):
		"""Generates names stpred function that returns list of names in this table."""
		return self.names_string % {'tbl': self.name}

	def gen_names_embed(self, refcolumn, reftable):
		"""Generates names stpred function that returns list of names in this table.
		This function is used for the tables that are embed into another.
		"""
		return self.names_embed_string % {'tbl': self.name, 'column': refcolumn, 'reftbl': reftable, 'delim': constants.DELIMITER}

	#generates function which finds all deleted objects in diff data table
	def gen_diff_deleted(self):
		"""Generates diff_deleted stored function that returns a list of names of objects that were deleted between two versions.
		This list of deleted data is found out from temp table prepared in the init_diff function,
		"""
		return self.diff_deleted_string % {'tbl': self.name}

	#generates function which finds all created objects in diff data table
	def gen_diff_created(self):
		"""Generates diff_created stored function that returns a list of objecta that where created between two versions
		This list of created objects is found out from temp table prepared in the init_diff function.
		"""
		return self.diff_created_string % {'tbl': self.name}

	def gen_data_version(self):
		"""Generates data_version stored function that returns data of all object in this table taht were in the table in the given version."""
		return self.data_version_function_string % {'tbl': self.name}

	def gen_data_changes(self):
		return self.data_changes_function_string % {'tbl': self.name}

	def gen_resolved_data(self):
		"""gen_resolved_data is function for generating %(tbl)s_resolved_data(name_ text, version bigint = 0)

		Function is called only for tables that could be templated by some template (has column template).
		"""

		cols_ex_template_dict = self.col.copy()
		del cols_ex_template_dict['uid']
		del cols_ex_template_dict['name']
		
		if self.templates == "":
			templ_col = 'template_' + self.name
		else:
			templ_col = 'template_' + self.templates			
		del cols_ex_template_dict[templ_col]

		#for multiple data we dont want to loos data about embed into columns and we would like to left uid columns unresolved
		multiple_ticols = ',\n'.join(
			["%s %s" % (k, v) for (k, v) in
				zip(cols_ex_template_dict.keys(), cols_ex_template_dict.values())])

		#list of columns of given kind
		collist = cols_ex_template_dict.keys()

		data_attributes = ['data.%s' % col for col in collist]
		dcols = ','.join(data_attributes) + ',' + 'data.template'

		#table tbl is templated by table tbl_template
		#table tbl_template is templated by tbl_template
		if self.name.endswith("_template"):
			templ_table = self.name
		else:
			templ_table = self.name + "_template"

		#in case of multiple data we would like to select all columns in table
		multiple_columns = ",".join(collist)
		#table which is thatone embed into
		if self.embed_into <> "" or len(self.merge_with) > 0:
			resolved_object_data_string = self.resolved_object_data_embed_string
			resolved_object_data_template_info_string = self.resolved_object_data_template_info_embed_string
			multiple_rd_dv_coalesce_list = list()
			for col in collist:
				if col == self.embed_into or col in self.merge_with:
					multiple_rd_dv_coalesce_list.append("rd.%s AS %s" % (col, col))
				else:
					multiple_rd_dv_coalesce_list.append(
						"COALESCE(rd.%s,dv.%s) AS %s" % (col, col, col))
			if self.embed_into <> "":
				collist.remove(self.embed_into)
				del cols_ex_template_dict[self.embed_into]
		else:
			resolved_object_data_string = self.resolved_object_data_string
			resolved_object_data_template_info_string = self.resolved_object_data_template_info_string
			multiple_rd_dv_coalesce_list = ["COALESCE(rd.%s,dv.%s) AS %s" % (x,x,x) for x in collist]

		multiple_rd_dv_coalesce = ",\n".join(multiple_rd_dv_coalesce_list)
		cols = ','.join(collist)
		columns_ex_template = list(collist)

		data_attributes = ['data.%s' % x for x in collist]
		#should be in right order at the last position
		dcols = ','.join(data_attributes) + ', data.' + templ_col

		# rd_dv_coalesce =coalesce(rd.vendor,dv.vendor),coalesce(rd.purchase,dv.purchase), ...
		templated_rddv_collist = list()
		for col in collist:
			if col in self.merge_with:
				templated_rddv_collist.append("rd." + col)
			else:
				templated_rddv_collist.append("COALESCE(rd.%s,dv.%s) AS %s" % (col,col,col))
		if len(templated_rddv_collist) > 0:
			rddvcoal = ',\n'.join(templated_rddv_collist)

		# replace uid of referenced object its name
		for col in self.refuid_columns:
			if col in collist:
				pos = collist.index(col)
				collist[pos] = "%s_get_name(%s) AS %s" % (col, col, col)
				cols_ex_template_dict[col] = "text"

		cols_ex_templ = ",".join(collist)

		ticols = ',\n'.join(["%s %s" % (k, v) for (k, v) in zip(cols_ex_template_dict.keys(), cols_ex_template_dict.values())])
		templ_cols = ',\n'.join(["%s_templ %s" % (k, d) for (k, d) in zip(cols_ex_template_dict.keys(), ['text']*len(cols_ex_template_dict))])
		case_col_string = '''
		CASE	WHEN %s IS NULL THEN NULL
			ELSE name
		END AS %s_templ'''
		case_cols = ','.join([case_col_string % (x,x) for x in cols_ex_template_dict.keys()])

		templ_case_cols_str = '''
		CASE	WHEN rd.%s_templ IS NOT NULL THEN rd.%s_templ
			WHEN rd.%s_templ IS NULL AND dv.%s IS NOT NULL THEN dv.name
			ELSE NULL
		END AS %s_templ'''
		templ_case_list = list()
		for col in cols_ex_template_dict.keys():
			if col == self.embed_into or col in self.merge_with:
				templ_case_list.append("rd.%s_templ AS %s_templ" % (col, col))
			else:
				templ_case_list.append(templ_case_cols_str % (col, col, col, col, col))
		templ_case_cols = ','.join(templ_case_list)

		templ_columns_list = ["%s_templ" % x for x in cols_ex_template_dict.keys()]
		cols_templ = ','.join(templ_columns_list)
		#SELECT %(columns_ex_templ)s, %(columns_templ)s, %(templ_tbl)s_get_name(orig_template) AS template INTO %(data_columns)s
		all_columns = columns_ex_template
		all_columns.extend(templ_columns_list)
		all_columns.append(templ_col)
		dticols = ','.join(["data.%s" % x for x in all_columns])

		templ_info_type = self.resolved_data_template_info_type_string % {'tbl': self.name, 'columns': ticols, 'templ_columns': templ_cols, 'template_column': templ_col}
		resolve_object_data_fce = resolved_object_data_string % {'tbl': self.name, 'columns': cols, 'columns_ex_templ': cols_ex_templ, 'rd_dv_coalesce': rddvcoal, 'templ_tbl': templ_table, 'data_columns': dcols, 'template_column': templ_col}
		resolve_data_template_info_fce = self.resolved_data_template_info_string % {'tbl': self.name, 'templ_tbl': templ_table, 'columns': multiple_columns, 'rd_dv_coalesce': multiple_rd_dv_coalesce, 'columns_ex_templ': multiple_columns, 'case_columns': case_cols, 'templ_case_columns': templ_case_cols, 'columns_templ': cols_templ, 'template_column': templ_col}
		resolve_object_data_template_info = resolved_object_data_template_info_string % {'tbl': self.name, 'templ_tbl': templ_table, 'columns': cols, 'rd_dv_coalesce': rddvcoal, 'columns_ex_templ': cols_ex_templ, 'case_columns': case_cols, 'templ_case_columns': templ_case_cols, 'columns_templ': cols_templ, 'data_columns': dticols, 'template_column': templ_col}
		multiple_object_data_templ_info_type = self.multiple_resolved_data_template_info_type_string % {'tbl': self.name, 'columns': multiple_ticols, 'templ_columns': templ_cols, 'template_column': templ_col}
		multiple_data_type = self.multiple_resolved_data_type_string % {'tbl': self.name, 'columns': multiple_ticols, 'template_column': templ_col}
		resolve_data_fce = self.resolved_data_string % {'tbl': self.name, 'templ_tbl': templ_table, 'columns': multiple_columns, 'rd_dv_coalesce': multiple_rd_dv_coalesce, 'columns_ex_templ': multiple_columns, 'template_column': templ_col}
		return  templ_info_type + '\n' + multiple_data_type + '\n' + multiple_object_data_templ_info_type + '\n' + resolve_object_data_fce  + '\n' + resolve_data_fce + '\n' + resolve_data_template_info_fce + '\n' + resolve_object_data_template_info


	def gen_resolved_data_diff(self):
		"""gen_resolved_data_diff is function for generating functions that init_resolved_diff function.

		Function is called only for tables that could be templated by some template (has column template).
		"""
		if self.name.endswith("_template"):
			templ_table = self.name
		else:
			templ_table = self.name + "_template"
		#	COALESCE(rd.warranty,dv.warranty) AS warranty, COALESCE(rd.purchase,dv.purchase) AS purchase,

		collist = self.col.keys()
		
		if self.templates == "":
			templ_col = 'template_' + self.name
		else:
			templ_col = 'template_' + self.templates
		collist.remove(templ_col)
		collist.remove('uid')
		collist.remove('name')

		if self.embed_into <> "" or len(self.merge_with) > 0:
			rd_dv_list = list()
			for col in collist:
				#column that refers to embed into parent and merge with columns are not present in template
				if (col == self.embed_into) or (col in self.merge_with):
					rd_dv_list.append("rd." + col)
				else:
					rd_dv_list.append("COALESCE(rd.%s,dv.%s) AS %s" % (col, col, col))
			rd_dv_coal = ',\n'.join(rd_dv_list)
		else:
			rd_dv_coal = ',\n'.join(["COALESCE(rd.%s,dv.%s) AS %s" % (x, x, x) for x in collist])

		columns = ','.join(collist)

		att_name_type = list()
		for col in collist:
			att_name_type.append("%s %s" % (col, self.col[col]))
		columns_types = ",\n".join(att_name_type)
		diff_type = self.diff_data_type_str % {'tbl': self.name, 'col_types': columns_types}
		changeses_function = self.data_resolved_changes_function_string % {'tbl': self.name, 'templ_tbl': templ_table, 'rd_dv_coalesce': rd_dv_coal, 'columns_ex_templ': columns, 'template_column': templ_col}

		#template, name must be present
		collist = self.col.keys()
		collist.remove('uid')
		select_new_attributes = ["chv.%s AS new_%s" % (x, x) for x in collist]
		select_new_attributes.append("chv.dest_bit AS new_dest_bit")
		#dest_bit from resolved data is allways 0
		select_old_attributes = ["dv.%s AS old_%s" % (x, x) for x in collist]
		select_old_attributes.append("CAST('0' AS bit(1)) AS old_dest_bit")
		select_old_new_objects_attributes = ",".join(select_old_attributes) + "," + ",".join(select_new_attributes)
		init_function = self.diff_init_resolved_function_string % {'tbl': self.name, 'diff_columns': select_old_new_objects_attributes, 'template_column': templ_col}
		current_changeset_diff = self.diff_changeset_init_resolved_function_string % {'tbl': self.name, 'diff_columns': select_old_new_objects_attributes, 'columns_ex_templ': columns, 'templ_tbl': templ_table, 'rd_dv_coalesce': rd_dv_coal, 'template_column': templ_col}
		return diff_type + '\n' + changeses_function + '\n' + init_function + '\n' + current_changeset_diff

