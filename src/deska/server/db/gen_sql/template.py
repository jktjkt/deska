
class Template:
	table_query_str = "SELECT relname FROM get_table_info() WHERE attname LIKE 'template_%';"
	not_null_query_str = "SELECT n_constraints_on_table('%(tbl)s');"
	relations_str = "SELECT relation, attname, refkind FROM kindrelations_full_info('%s');"
	template_columns_str = "SELECT relname, attname FROM table_info_view WHERE attname LIKE 'template_%';"
	"""Query to get names of tables which are merged with this table."""


	template_str = '''
CREATE SEQUENCE production.%(tbl)s_template_uid;
CREATE TABLE production.%(tbl)s_template (
	LIKE production.%(tbl)s,
	CONSTRAINT pk_%(tbl)s_template PRIMARY KEY (uid),
	CONSTRAINT rtempl_%(tbl)s_templ FOREIGN KEY ("%(templ_col)s") REFERENCES %(tbl)s_template(uid) DEFERRABLE INITIALLY IMMEDIATE,
	CONSTRAINT "%(tbl)s_template with this name already exists" UNIQUE (name)
);
ALTER TABLE %(tbl)s_template ALTER COLUMN uid SET DEFAULT nextval('%(tbl)s_template_uid'::regclass);
ALTER TABLE %(tbl)s ADD CONSTRAINT rtempl_%(tbl)s FOREIGN KEY ("%(templ_col)s") REFERENCES %(tbl)s_template(uid) DEFERRABLE INITIALLY IMMEDIATE;

'''

	drop_column_str = "ALTER TABLE %s_template DROP COLUMN %s;"

	drop_not_null_str = "ALTER TABLE %s_template ALTER COLUMN %s DROP NOT NULL;"

	add_multiref_fk_str = '''ALTER TABLE %(tbl_name)s_template 
	ADD CONSTRAINT rset_%(tbl_name)s_fk_%(column)s FOREIGN KEY (%(column)s)
	REFERENCES %(reftbl_name)s (uid) DEFERRABLE INITIALLY IMMEDIATE;'''

	def __init__(self,db_connection):
		self.plpy = db_connection;
		self.plpy.execute("SET search_path TO deska,production, api")

		# init set of tables
		self.tables = set()

		# select all tables
		record = self.plpy.execute(self.table_query_str)
		for tbl in record:
			self.tables.add(tbl[0])

		self.not_null_dict = dict()
		for table_name in self.tables:
			not_null_record = self.plpy.execute(self.not_null_query_str % {'tbl': table_name})
			self.not_null_dict[table_name] = list()
			for row in not_null_record:
				if row[0] != 'uid' and row[0] != 'name':
					self.not_null_dict[table_name].append(row[0])

		#creates dict table name and table's column that refers to template table for this table
		record = self.plpy.execute(self.template_columns_str)
		self.template_columns = dict()
		for row in record:
			if row[0] not in self.template_columns:
				self.template_columns[row[0]] = row[1]
			else:
				raise ValueError, 'relation template for table %(tbl)s is badly defined, each kind can have at most one column with prefix template_'


	# generate sql for all tables
	def gen_templates(self,filename):
		self.sql = open(filename,'w')

		# print this to add proc into genproc schema
		self.sql.write("SET search_path TO production;\n")

		for tbl in self.tables:
			self.sql.write(self.gen_table_template(tbl))
			self.sql.write(self.gen_table_drop_not_null(tbl))
			self.gen_relation_modif(tbl)
		self.sql.close()
		return

	def gen_relation_modif(self, table_name):
		"""Generates sql code for drop column statemnets and adds refers to set relation to template tables.
		
		Drop column that is apart of the embed into relation and columns that are a part of some contains or containable relation.
		Adds foreign key constraint to columns that should refers to set. This ensures that the inner table for n:m relation - multirefence will be created for this template table.
		"""
		record = self.plpy.execute(self.relations_str % table_name)
		embed_column = ""
		contained_columns = list()
		refers_to_set = dict()
		for row in record:
			if row[0] == "EMBED_INTO":
				embed_column = row[1]
			elif row[0] == "CONTAINS" or row[0] == "CONTAINABLE":
				contained_columns.append(row[1])
			elif row[0] == "REFERS_TO_SET":
				refers_to_set[row[1]] = row[2]
		
		self.sql.write(self.gen_drop_embed_parent_column(table_name, embed_column))
		self.sql.write(self.gen_drop_contained_column(table_name, contained_columns))
		self.sql.write(self.gen_refers_to_set(table_name, refers_to_set))


	def gen_drop_embed_parent_column(self, table_name, embed_column):
		if embed_column <> "":
			return self.drop_column_str % (table_name, embed_column)
		else:
			return ""

	def gen_drop_contained_column(self, table_name, contained_columns):
		drop_contained_columns = ""
		for column in contained_columns:
			drop_contained_columns = drop_contained_columns + '\n' + self.drop_column_str % (table_name, column)
			
		return drop_contained_columns

	def gen_refers_to_set(self, table_name, refers_to_set):
		add_multiref_fk = ""
		for column in refers_to_set:
			reftbl = refers_to_set[column]
			add_multiref_fk = add_multiref_fk + '\n' + self.add_multiref_fk_str % {'tbl_name': table_name, 'column': column, 'reftbl_name': reftbl}
		
		return add_multiref_fk

	# generate sql for one table
	def gen_table_template(self,table_name):
		template_column = self.template_columns[table_name]
		return self.template_str % {'tbl': table_name, 'templ_col': template_column}

	def gen_table_drop_not_null(self, table_name):
		constraints = list(
			[self.drop_not_null_str % (tbl, col) for (tbl, col) in
				zip(([table_name] * len(self.not_null_dict[table_name])),
					self.not_null_dict[table_name])])
		return '\n'.join(constraints) + '\n'

