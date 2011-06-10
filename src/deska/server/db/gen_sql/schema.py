from table import Table

class Schema:
	table_str = "SELECT DISTINCT relname from deska.table_info_view"
	column_str = "SELECT attname,typname from deska.table_info_view where relname='{0}'"
	pk_str = "SELECT conname,attname FROM key_constraints_on_table('{0}')"
	fk_str = "SELECT conname,attname,reftabname,refattname FROM fk_constraints_on_table('{0}')"
	templ_tables_str = "SELECT relname FROM get_table_info() WHERE attname = 'template';"
	embed_into_str = "SELECT refkind FROM kindRelations_full_info('{0}') WHERE relation = 'EMBED';"
	refuid_columns_str = "SELECT attname,tabname FROM cols_ref_uid('{0}');"
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
		{commit_tables}
		-- should we check constraint before version_commit?
		--SET CONSTRAINTS ALL IMMEDIATE;
		SELECT create_version(message) INTO rev;
		RETURN rev;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;
'''
	def __init__(self,db_connection):
		self.plpy = db_connection;
		self.plpy.execute("SET search_path TO deska,api,production")

		# init set of tables
		self.tables = set()

		# select all tables
		record = self.plpy.execute(self.table_str)
		for tbl in record[:]:
			self.tables.add(tbl[0])

		# print foreign keys at the end
		self.fks = ""
		record = self.plpy.execute(self.templ_tables_str)
		self.templated_tables = set()
		for tbl in record:
			self.templated_tables.add(tbl[0])


	# generate sql for all tables
	def gen_schema(self,filename):
		name_split = filename.rsplit('/', 1)
		self.table_sql = open(name_split[0] + '/' + 'create_tables2.sql','w')
		self.fn_sql = open(name_split[0] + '/' + 'fn_' + name_split[1],'w')

		# print this to add proc into genproc schema
		self.table_sql.write("SET search_path TO history,deska,versioning,production;\n")
		self.fn_sql.write("\\i create_schemas_2.sql\n")
		self.fn_sql.write("SET search_path TO genproc,history,deska,versioning,production;\n")

		for tbl in self.tables:
			self.gen_for_table(tbl)
			
		self.fn_sql.write(self.gen_commit())

		self.fn_sql.close()
		self.table_sql.close()
		return

	# generate sql for one table
	def gen_for_table(self,tbl):
		# select col info
		tables = self.plpy.execute(self.column_str.format(tbl))

		# create table obj
		table = Table(tbl)

		# add columns
		for col in tables[:]:
			table.add_column(col[0],col[1])

		# add pk constraints
		constraints = self.plpy.execute(self.pk_str.format(tbl))
		for col in constraints[:]:
			table.add_pk(col[0],col[1])

		# add fk constraints
		constraints = self.plpy.execute(self.fk_str.format(tbl))
		for col in constraints[:]:
			table.add_fk(col[0],col[1],col[2],col[3])

		embed_into_rec = self.plpy.execute(self.embed_into_str.format(tbl))
		table.embed_into = ""
		for row in embed_into_rec:
			table.embed_into = row[0]
			
		refuid_rec = self.plpy.execute(self.refuid_columns_str.format(tbl))
		table.refuid_columns = dict()
		for row in refuid_rec:
			table.refuid_columns[row[0]] = row[1]
		
		if tbl.endswith('_template'):
			templated_table = tbl.replace('_template','')
			refuid_rec = self.plpy.execute(self.refuid_columns_str.format(templated_table))
			for row in refuid_rec:
				if row[0] not in table.refuid_columns:
					table.refuid_columns[row[0]] = row[1]
		
		# generate sql
		self.table_sql.write(table.gen_hist())

		self.fks = self.fks + (table.gen_fks())
		#get dictionary of colname and reftable, which uid colname references
		#cols_ref_uid = table.get_cols_reference_uid()
		for col in tables[:]:
			if (col[0] in table.refuid_columns):
				reftable = table.refuid_columns[col[0]]
				#column that references uid has another set function(with finding corresponding uid)
				self.fn_sql.write(table.gen_set_ref_uid(col[0], reftable))
			elif (col[0] != 'name' and col[0]!='uid'):
				self.fn_sql.write(table.gen_set(col[0]))

			#get uid of that references uid should not return uid but name of according instance

		#get uid from embed object
		if (table.embed_into != ""):
			reftable = table.embed_into
			for k, v in table.refuid_columns.iteritems():
				if v == table.embed_into:
					embed_column = k
					break
			
			#adding full quolified name with _ delimiter
			self.fn_sql.write(table.gen_add_embed(embed_column,reftable))
			#get uid from embed object, again name have to be full
			self.fn_sql.write(table.gen_get_uid_embed(embed_column,reftable))
			self.fn_sql.write(table.gen_get_name_embed(embed_column,reftable))
			self.fn_sql.write(table.gen_names_embed(embed_column,reftable))
			self.fn_sql.write(table.gen_set_name_embed(embed_column, reftable))
		else:
			self.fn_sql.write(table.gen_add())
			self.fn_sql.write(table.gen_get_uid())
			self.fn_sql.write(table.gen_get_name())
			self.fn_sql.write(table.gen_names())
			self.fn_sql.write(table.gen_set('name'))
			
#TODO repair this part with, generating procedure for getting object data, in columns that referes to another kind is uid
#we need to return name of corresponding instance
		self.fn_sql.write(table.gen_del())
		self.fn_sql.write(table.gen_undel())
		self.fn_sql.write(table.gen_get_object_data())
		self.fn_sql.write(table.gen_diff_deleted())
		self.fn_sql.write(table.gen_diff_created())
		self.fn_sql.write(table.gen_diff_set_attribute())
		self.fn_sql.write(table.gen_diff_init_function())
		self.fn_sql.write(table.gen_diff_terminate_function())
		self.fn_sql.write(table.gen_data_version())
		self.fn_sql.write(table.gen_data_changes())
		
		#different generated functions for templated and not templated tables
		if tbl in self.templated_tables:
			self.fn_sql.write(table.gen_commit_templated())
			self.fn_sql.write(table.gen_resolved_data())
		else:
			self.fn_sql.write(table.gen_commit())
			
		return

	def gen_commit(self):
		commit_table_template = '''PERFORM {tbl}_commit();
		'''
		commit_tables=""
		for table in self.tables:
			commit_tables = commit_tables + commit_table_template.format(tbl = table)

		return self.commit_string.format(commit_tables = commit_tables)

	def pygen_commit(self):
		commit_str = '''def commit():
	return db.callproc("commit")
	'''
		return commit_str
		