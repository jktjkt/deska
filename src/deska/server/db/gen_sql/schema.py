from table import Table

"""@package Schema
This modul generates history, template tables and set, get functions for all tables in production.

Template tables are generated into schema production for those tables that have column template.
History tables are generated into schema history for all tables in production, even for template tables.
"""

class Schema:
	"""Schema class contains methods for generating template, history tables and stored procedures to maintain these tables.

    Template tables are generated into schema production for those tables that have column template.
    History tables are generated into schema history for all tables in production, even for template tables.
    Get, set, ... stored procedures are generated for all tables.
    """
	py_fn_str = """
def %(name)s(%(args)s):
	return %(result)s
"""
	table_str = "SELECT DISTINCT relname from deska.table_info_view"
	"""Query to get all tables in the schema production."""
	column_str = "SELECT attname,typname from deska.table_info_view where relname='%s'"
	"""Query to get all the columns and their types in the table"""
	att_type_str = "SELECT attname,typename from kindAttributes('%s')"
	"""Query to get all the columns and their types in the table"""
	pk_str = "SELECT conname,attname FROM key_constraints_on_table('%s')"
	"""Query to get the primary key constraint and all the columns relate to this constraint in the table"""
	fk_str = "SELECT conname,attname,reftabname,refattname FROM fk_constraints_on_table('%s')"
	"""Query to get for the table foreign key constraint, all the columns relate to this constraint and the name of referenced table."""
	templ_tables_str = "SELECT relname FROM get_table_info() WHERE attname LIKE 'template_%';"
	"""Query to get all tables that have attribute template.
	For these tables would be generated template table.
	"""
	templates_str = "SELECT * FROM get_templates_info() WHERE template <> kind;"
	"""Query to get all templates in the schema production, the template name and the name of table that is templated by this template"""
	embed_into_str = "SELECT attname FROM kindRelations_full_info('%s') WHERE relation = 'EMBED_INTO';"
	"""Query to get the name of table which is this table embed into """
	refuid_columns_str = "SELECT attname,tabname FROM cols_ref_uid('%s');"
	"""Query to get all refuid references in the table.
	Refuid reference is reference to some table's uid column.
	Gets name of column from which is uid referenced and the name of the table table that is referenced,
	"""
	composition_str = "SELECT refkind FROM kindRelations('%s') WHERE relation = 'CONTAINS' OR relation = 'CONTAINABLE';"
	"""Query to get names of tables which are in composition with this table."""
	refers_to_set_info_str = "SELECT attname, refkind, refattname FROM kindRelations_full_info('%(tbl)s') WHERE relation = 'REFERS_TO_SET'"
	"""Query to get info about all relations refers_to_set."""
	check_ondel_no_action_str = "SELECT kindname, refkind, attname FROM check_delete_no_action()"
	invalid_relations_str = "SELECT attname, refkind FROM kindRelations_full_info('%s') WHERE relation = 'INVALID';"
	"""Query to get foreign keys that are not deska relations """
	commit_string = '''
CREATE FUNCTION commit_all(message text)
	RETURNS bigint
	AS
	$$
	DECLARE rev bigint;
		last_rev bigint;
		parent bigint;
	BEGIN
		SELECT max(id) INTO last_rev FROM version;
		parent = parent(get_current_changeset());
		IF parent != last_rev THEN
			RAISE SQLSTATE '70007' USING MESSAGE = 'You must run rebase before commit.';
		END IF;
		SET CONSTRAINTS ALL DEFERRED;
		%(commit_tables)s
		-- should we check constraint before version_commit?
		--SET CONSTRAINTS ALL IMMEDIATE;
		SELECT create_version(message) INTO rev;

		SET CONSTRAINTS ALL IMMEDIATE;

		RETURN rev;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;
'''
	"""Template for the commit_all stored procedure.
	Commit_all stored procedure finds the parent revision of this revision and checks if the rebase is needed.
	Sets all the deferrable constraints to deferred (mostly foreign key constraints).
	Calls commit on all tables.
	Creates new version.
	"""

	def __init__(self,db_connection):
		"""The constructor for the Schema class."""
		self.plpy = db_connection;
		self.plpy.execute("SET search_path TO deska,api,production")

		# init set of tables
		self.tables = set()
		# structures for relations
		self.relFromCol = dict()
		self.relFromTbl = dict()
		self.relToTbl = dict()

		# dict of attributes dicts
		self.atts = dict()
		# dict of embeded
		self.embed = dict()
		self.embedNames = dict()
		# dict of composition
		self.composition = dict()
		self.containsNames = dict()
		self.containableNames = dict()
		# dict of templated tables
		#keys are template tables, values are tables templated by them
		self.template = dict()
		self.templateNames = dict()
		# dict for refs
		self.refs = dict()
		self.refNames = dict()

		# dict of tables that refers to some sets of uids
		self.refers_to_set = dict()
		# select all tables
		
		record = self.plpy.execute(self.check_ondel_no_action_str)
		for row in record:
			raise ValueError, "Foreign key constraint from %(tbl)s kind, %(att)s attribute to %(reftbl)s kind is badly defined, it should have set on delete no action." % {'tbl': row[0], 'att': row[1], 'reftbl': row[2]}
		
		record = self.plpy.execute(self.table_str)
		for tbl in record[:]:
			invalid_rec = self.plpy.execute(self.invalid_relations_str % tbl[0])
			#cheks if there is foreign key in schema that is not supported by deska
			for row in invalid_rec:
				print "WARNING: Foreign key constraint from %(tbl)s kind, %(att)s attribute to %(reftbl)s kind is not deska relation." % {'tbl': tbl[0], 'att': row[0], 'reftbl': row[1]}
			self.tables.add(tbl[0])

		# print foreign keys at the end
		self.fks = ""
		record = self.plpy.execute(self.templ_tables_str)
		self.templated_tables = set()
		for tbl in record:
			self.templated_tables.add(tbl[0])

		record = self.plpy.execute(self.templates_str)
		self.templates = dict()
		self.template_relations = dict()
		for row in record:
			#row[0] is template name
			#row[1] is table templated by template row[0]
			self.templates[row[0]] = row[1]
			self.template_relations[row[1]] = row[0]
			self.template_relations[row[0]] = row[0]


	# generate sql for all tables
	def gen_schema(self,filename,generated_filename):
		"""Generates sql files for creating history tables and stored procedures.

		For each table in variable table (tables in the schema production) is called function gen_for_table.
		Stored procedures for this table are generated into fn_sql file and the history table into table_sql file.
		After that is called function gen_commit that generates commit_all function for committing all modifications of all tables.
		"""

		name_split = filename.rsplit('/', 1)
		self.table_sql = open(name_split[0] + '/' + 'create_tables2.sql','w')
		self.fn_sql = open(name_split[0] + '/' + 'fn_' + name_split[1],'w')
		self.generated_py = open(generated_filename,'w')

		# print this to add proc into genproc schema
		self.table_sql.write("SET search_path TO history,deska,versioning,production;\n")
		self.fn_sql.write("\\i create_schemas_2.sql\n")
		self.fn_sql.write("SET search_path TO genproc,history,deska,versioning,production;\n")

		for tbl in self.tables:
			self.gen_for_table(tbl)

#TODO: multiref tables -history, special set, special get, fix normal get_object data
		#for tbl in self.inner_multiref_tables:
            #self.gen_for_inner_table(tbl)

		self.fn_sql.write(self.gen_commit())

		self.fn_sql.close()
		self.table_sql.close()
		refNamesCopy = self.refNames.copy()
		for tbl in self.templates:
			self.refs[tbl] = self.refs[self.templates[tbl]]
			for relName in refNamesCopy:
				if self.refNames[relName] == self.templates[tbl]:
					#FIXME: now we can overwrite another relation with same name, but this is wanted until we fix function for getting fks
					newRelName = relName + "_XXX"
					self.refNames[newRelName] = tbl
					self.relFromTbl[newRelName] = tbl
					self.relFromCol[newRelName] = self.relFromCol[relName]
					self.relToTbl[newRelName] = self.relToTbl[relName]


		# create some python helper functions
		self.generated_py.write(self.py_fn_str % {'name': "kinds", 'args': '', 'result': list(self.tables)})
		self.generated_py.write(self.py_fn_str % {'name': "atts", 'args': 'kind', 'result': str(self.atts) + "[kind]"})
		self.generated_py.write(self.py_fn_str % {'name': "refNames", 'args': '', 'result': str(self.refNames)})
		self.generated_py.write(self.py_fn_str % {'name': "containsNames", 'args': '', 'result': str(self.containsNames)})
		self.generated_py.write(self.py_fn_str % {'name': "containableNames", 'args': '', 'result': str(self.containableNames)})
		self.generated_py.write(self.py_fn_str % {'name': "embedNames", 'args': '', 'result': str(self.embedNames)})
		self.generated_py.write(self.py_fn_str % {'name': "templateNames", 'args': '', 'result': str(self.templateNames)})
		self.generated_py.write(self.py_fn_str % {'name': "relFromCol", 'args': 'relName', 'result': str(self.relFromCol) + "[relName]"})
		self.generated_py.write(self.py_fn_str % {'name': "relFromTbl", 'args': 'relName', 'result': str(self.relFromTbl) + "[relName]"})
		self.generated_py.write(self.py_fn_str % {'name': "relToTbl", 'args': 'relName', 'result': str(self.relToTbl) + "[relName]"})
		return

	# generate sql for one table
	def gen_for_table(self,tbl):
		"""Generates sql files for creating history tables and stored procedures.

		Calls functions for generating stored functions which differ for some cases. (embed tables, refuid columns)
		Stored functions means function for add object, set attribute, get attribute, get object data, get list of instances and functions for diffing of versions.
		For templated tables are generated stored functions to get resolved data or resolved data with its origin.
		"""
		# select col info
		columns = self.plpy.execute(self.column_str % tbl)
		att_types = self.plpy.execute(self.att_type_str % tbl)
		self.atts[tbl] = dict(att_types)
		self.refs[tbl] = list()

		# create table obj
		table = Table(tbl)

		# add columns
		for col in columns[:]:
			table.add_column(col[0],col[1])

		# add pk constraints
		constraints = self.plpy.execute(self.pk_str % tbl)
		for col in constraints[:]:
			table.add_pk(col[0],col[1])

		record = self.plpy.execute(self.refers_to_set_info_str % {'tbl': tbl})
		table.refers_to_set = list()
		if len(record) > 0:
			for row in record:
				table.refers_to_set.append(row[0])
			self.refers_to_set[tbl] = table.refers_to_set

		# add fk constraints
		fkconstraints = self.plpy.execute(self.fk_str % tbl)
		for col in fkconstraints[:]:
			table.add_fk(col[0],col[1],col[2],col[3])
			relName = col[0]
			if relName in self.relFromTbl:
				#FIXME: this should not happen, HOTFIX:
				relName = relName + "_XXX"
			self.relFromTbl[relName] = tbl
			self.relFromCol[relName] = col[1]
			self.relToTbl[relName] = col[2]

			# if there is a reference, change int for identifier'
			if col[1] not in table.refers_to_set:
				self.atts[tbl][col[1]] = 'identifier'
			prefix = col[0][0:7]
			if prefix == "rembed_":
				self.embed[tbl] = col[1]
				self.embedNames[relName] = tbl
			elif prefix == "rconta_":
				self.composition[tbl] = col[1]
				self.containsNames[relName] = tbl
			elif prefix == "rcoble_":
				self.containableNames[relName] = tbl
			elif prefix == "rtempl_":
				self.template[tbl] = col[2]
				self.templateNames[relName] = tbl
			else:
				self.refs[tbl].append(col[1])
				self.refNames[relName] = tbl

		embed_into_rec = self.plpy.execute(self.embed_into_str % tbl)
		table.embed_column = ""
		for row in embed_into_rec:
			table.embed_column = row[0]

		refuid_rec = self.plpy.execute(self.refuid_columns_str % tbl)
		table.refuid_columns = dict()
		for row in refuid_rec:
			table.refuid_columns[row[0]] = row[1]

		record = self.plpy.execute(self.composition_str % tbl)
		table.contains = list()
		for row in record:
			table.contains.append(row[0])

		if tbl in self.templates:
			templated_table = self.templates[tbl]
			refuid_rec = self.plpy.execute(self.refuid_columns_str % templated_table)
			for row in refuid_rec:
				if row[0] not in table.refuid_columns:
					table.refuid_columns[row[0]] = row[1]

		table.validate();

		# generate sql
		self.table_sql.write(table.gen_hist())

		self.fks = self.fks + (table.gen_fks())
		#get dictionary of colname and reftable, which uid colname references
		#cols_ref_uid = table.get_cols_reference_uid()
		for col in columns[:]:
			if (col[0] in table.refuid_columns):
				reftable = table.refuid_columns[col[0]]
				if col[0] in table.refers_to_set:
					self.fn_sql.write(table.gen_set_refuid_set(col[0], reftable))
					self.fn_sql.write(table.gen_refuid_set_insert(col[0], reftable))
					self.fn_sql.write(table.gen_refuid_set_remove(col[0], reftable))
				else:
					#column that references uid has another set function(with finding corresponding uid)
					self.fn_sql.write(table.gen_set_ref_uid(col[0], reftable))
			elif (col[0] != 'name' and col[0] != 'uid'):
				self.fn_sql.write(table.gen_set(col[0]))
			#get uid of that references uid should not return uid but name of according instance

		if table.embed_column <> "":
			#adding full quolified name with _ delimiter
			embed_column = table.embed_column
			reftable = table.refuid_columns[embed_column]
			self.fn_sql.write(table.gen_add_embed(embed_column,reftable))
			#get uid from embed object, again name have to be full
			self.fn_sql.write(table.gen_get_uid_embed(embed_column,reftable))
			self.fn_sql.write(table.gen_get_name_embed(embed_column,reftable))
			self.fn_sql.write(table.gen_names_embed(embed_column,reftable))
			self.fn_sql.write(table.gen_set_name_embed(embed_column, reftable))
			self.fn_sql.write(table.gen_get_name_changeset_embed(embed_column, reftable))
		else:
			self.fn_sql.write(table.gen_add())
			self.fn_sql.write(table.gen_get_uid())
			self.fn_sql.write(table.gen_get_name())
			self.fn_sql.write(table.gen_names())
			self.fn_sql.write(table.gen_set_name())
			self.fn_sql.write(table.gen_get_name_changeset())


		self.fn_sql.write(table.gen_del())
		self.fn_sql.write(table.gen_undel())
		self.fn_sql.write(table.gen_get_object_data())
		self.fn_sql.write(table.gen_diff_data_type())
		self.fn_sql.write(table.gen_diff_deleted())
		self.fn_sql.write(table.gen_diff_created())
		self.fn_sql.write(table.gen_diff_set_attribute())
		self.fn_sql.write(table.gen_diff_init_function())
		self.fn_sql.write(table.gen_diff_terminate_function())
		self.fn_sql.write(table.gen_data_version())
		self.fn_sql.write(table.gen_data_changes())

		#different generated functions for templated and not templated tables
		if tbl in self.templated_tables:
			if tbl in self.templates:
			#tbl is template
				table.templates = self.templates[tbl]
			else:
				table.templates = ""
			self.fn_sql.write(table.gen_resolved_data())
			self.fn_sql.write(table.gen_resolved_data_diff())
			self.fn_sql.write(table.gen_commit_templated())
			#for templated tables that has columns that refers to set of identifiers, works like coalesce
			if tbl in self.refers_to_set:
				self.fn_sql.write(table.gen_refs_set_coal())
		else:
			self.fn_sql.write(table.gen_commit())

		return

	def gen_commit(self):
		"""Generates stored procedure that executes commit on all tables.

		Generates PERFORM commit on each table and include it into commit_all function.
		"""
		commit_table_template = '''PERFORM %(tbl)s_commit();
		'''
		commit_tables=""
		tables_to_commit = self.tables.copy()
		#we need templates to be commited before templated tables by them
		for table in self.templates:
			commit_tables = commit_tables + commit_table_template % {'tbl': table}
			templated_table = self.templates[table]
			commit_tables = commit_tables + commit_table_template % {'tbl': templated_table}
			tables_to_commit.remove(table)
			tables_to_commit.remove(templated_table)

            
		for table in tables_to_commit:
			#here template means string which is formated not template of table
			commit_tables = commit_tables + commit_table_template % {'tbl': table}

		#commit of tables that are created in deska schema for inner propose, to maintain set of referenced identifier
		for table in self.refers_to_set:
			if table not in self.templates:
				for reftable in self.refers_to_set[table]:
					if table in self.template_relations:
						#table is templated but is not template
						template_table = self.template_relations[table]
						table_name = "inner_%(tbl)s_%(ref_tbl)s" % {'tbl' : template_table, 'ref_tbl' : reftable}
						commit_tables = commit_tables + commit_table_template % {'tbl': table_name}

					table_name = "inner_%(tbl)s_%(ref_tbl)s" % {'tbl' : table, 'ref_tbl' : reftable}
					commit_tables = commit_tables + commit_table_template % {'tbl': table_name}


		return self.commit_string % {'commit_tables': commit_tables}

	def pygen_commit(self):
		commit_str = '''def commit():
	return db.callproc("commit")
	'''
		return commit_str

