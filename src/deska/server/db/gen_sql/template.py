
class Template:
	"""This class contains all functions needed for the generation of the template table.
	These template tables are generated for tables that have a attribute with prefix "template_" and could be templated.
	"""
	table_query_str = "SELECT relname, typname FROM get_table_info() WHERE attname LIKE 'template_%';"
	"""Query to get names of tables that would be templated.
	For those tables we will generate tbl_template from which they will inherit values.
	"""
	not_null_query_str = "SELECT n_constraints_on_table('%(tbl)s');"
	"""Query to get names of attributes of table tbl, which have not null constraint.
	We will drop these constraints. In the template tables we allow null values.
	"""
	relations_str = "SELECT relation, attname, refkind FROM kindrelations_full_info('%s');"
	"""Query to get information about relations of the table.
	We will generate template tables for identifiers sets' inner tables. Drop columns which could not be templated - columns which are in contains, containable or embed into relation.
	"""
	template_columns_str = "SELECT relname, attname FROM table_info_view WHERE attname LIKE 'template_%';"
	"""Query to get name of attribute with "tempalte_" prefix for each table that has it.
	We will drop these constraints. In template table we allow null values.
	"""
	table_columns_str = "SELECT attname FROM kindattributes('%(tbl)s') WHERE attname NOT LIKE 'template_%%';"
	"""Query to get all attributes of the given templated table.
	They are used to find out if there is at least one attribute that could be templated and could inherit value from template.
	"""
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
	"""Create table string and do appropriate modifications.
	We will create template table for tables that have to be templated. Template tables would be like the table that would be templated by them.
	Constraint with rtempl_ prefix will be added to template table. This constraint will refer to uid of the same table. It ensures that template would be templated to.
	"""
	drop_column_str = "ALTER TABLE %s_template DROP COLUMN %s;"
	"""Drop column string.
	"""

	drop_not_null_str = "ALTER TABLE %s_template ALTER COLUMN %s DROP NOT NULL;"
	"""Drop not null constraint.
	"""

	add_multiref_fk_str = '''ALTER TABLE %(tbl_name)s_template
	ADD CONSTRAINT rset_%(tbl_name)s_fk_%(column)s FOREIGN KEY (%(column)s)
	REFERENCES %(reftbl_name)s (uid) DEFERRABLE INITIALLY IMMEDIATE;'''
	"""Add rset_ constraint to columns that refers to identifier set.
	"""

	cycle_check_constraint_str = '''CREATE FUNCTION %(tbl)s_template_not_in_cycle(obj_uid bigint, templ_uid bigint)
RETURNS BOOLEAN
AS
$$
BEGIN
CREATE TEMP TABLE temp_templates AS SELECT uid, %(templ_col)s AS templ FROM %(tbl)s_template_data_version();

IF obj_uid IN (
	WITH recursive rdata AS(
		SELECT templ_uid AS uid
		UNION
		SELECT templ AS uid FROM temp_templates tab JOIN rdata rd ON (tab.uid = rd.uid)
		WHERE templ IS NOT NULL
	)
	SELECT uid FROM rdata
)
THEN
	DROP TABLE temp_templates;
	RAISE EXCEPTION '%(tbl)s templates are in cycle';
	--RAISE EXCEPTION '%(tbl)s templates are in cycle' USING ERRCODE = '70016';
	RETURN FALSE;
ELSE
	DROP TABLE temp_templates;
	RETURN TRUE;
END IF;

END;
$$
LANGUAGE plpgsql;

'''

	def __init__(self,db_connection):
		self.plpy = db_connection;
		self.plpy.execute("SET search_path TO deska,production, api")

		# init set of tables
		self.tables = set()

		# select all tables
		record = self.plpy.execute(self.table_query_str)
		for row in record:
			self.tables.add(row[0])
			if row[1] not in ['bigint', 'int8']:
				raise ValueError, ('relation template for table %(tbl)s is badly defined, template column should have type bigint' % {'tbl': row[0]})                                      

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
				raise ValueError, ('relation template for table %(tbl)s is badly defined, each kind can have at most one column with prefix template_' % {'tbl': row[0]})


	# generate sql for all tables
	def gen_templates(self,filename):
		"""Generates create table statement for template table and modification needed for having well defined template table.
		Template table will be created like table that will be templated by it. So created template table will have than dropped not null constraints and dropped columns that won't be in template table.
		"""
		self.sql = open(filename,'w')

		# print this to add proc into genproc schema
		self.sql.write("SET search_path TO production;\n")

		for tbl in self.tables:
			self.sql.write(self.gen_table_template(tbl))
			self.sql.write(self.gen_table_drop_not_null(tbl))
			self.sql.write(self.add_check_cycle(tbl, self.template_columns[tbl]))
			self.gen_relation_modif(tbl)
		self.sql.close()
		return

	def gen_relation_modif(self, table_name):
		"""Generates sql code for drop column statements and adds refers to set relation to template tables.

		Drop column that is apart of the embed into relation and columns that are a part of some contains or containable relation.
		Adds foreign key constraint to columns that should refers to set. This ensures that the inner table for n:m relation - multirefence will be created for this template table.
		"""
		record = self.plpy.execute(self.relations_str % table_name)
		embed_column = ""
		contained_columns = list()
		relation_columns = list()
		refers_to_set = dict()
		for row in record:
			if row[0] == "EMBED_INTO":
				embed_column = row[1]
				relation_columns.append(row[1])
			elif row[0] == "CONTAINS" or row[0] == "CONTAINABLE":
				contained_columns.append(row[1])
				relation_columns.append(row[1])
			elif row[0] == "REFERS_TO_SET":
				refers_to_set[row[1]] = row[2]
				relation_columns.append(row[1])

		columns = list()
		record = self.plpy.execute(self.table_columns_str % {'tbl': table_name})
		for row in record:
			if row[0] not in relation_columns:
				columns.append(row[0])
		if len(columns) == 0:
			raise ValueError, ('relation template for table %(tbl)s is badly defined, there is no attribute to template in kind %(tbl)s' % {'tbl': table_name})

		self.sql.write(self.gen_drop_embed_parent_column(table_name, embed_column))
		self.sql.write(self.gen_drop_contained_column(table_name, contained_columns))
		self.sql.write(self.gen_refers_to_set(table_name, refers_to_set))


	def gen_drop_embed_parent_column(self, table_name, embed_column):
		"""The column that refers to another table with embed into relation is not templated"""
		if embed_column <> "":
			return self.drop_column_str % (table_name, embed_column)
		else:
			return ""

	def gen_drop_contained_column(self, table_name, contained_columns):
		"""Not null constraints inherited from the table that thisone templates are dropped, compulsory columns need not to be defined in template table."""
		drop_contained_columns = ""
		for column in contained_columns:
			drop_contained_columns = drop_contained_columns + '\n' + self.drop_column_str % (table_name, column)

		return drop_contained_columns

	def gen_refers_to_set(self, table_name, refers_to_set):
		"""Create templates for identifier set inner tables. Values that are inserted into attrributes of identifier_set type could be defined by template too."""
		add_multiref_fk = ""
		for column in refers_to_set:
			reftbl = refers_to_set[column]
			add_multiref_fk = add_multiref_fk + '\n' + self.add_multiref_fk_str % {'tbl_name': table_name, 'column': column, 'reftbl_name': reftbl}

		return add_multiref_fk

	# generate sql for one table
	def gen_table_template(self,table_name):
		"""Create template table."""
		template_column = self.template_columns[table_name]
		return self.template_str % {'tbl': table_name, 'templ_col': template_column}

	def gen_table_drop_not_null(self, table_name):
		"""Generates drop not null columns statements"""
		constraints = list(
			[self.drop_not_null_str % (tbl, col) for (tbl, col) in
				zip(([table_name] * len(self.not_null_dict[table_name])),
					self.not_null_dict[table_name])])
		return '\n'.join(constraints) + '\n'

	def add_check_cycle(self, table_name, template_column):
		"""Generates check constriant function for finding a cycle in templates"""
		return self.cycle_check_constraint_str % {'tbl': table_name, 'templ_col': template_column}
		