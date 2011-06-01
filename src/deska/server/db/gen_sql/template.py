
class Template:
	table_query_str = "SELECT relname FROM get_table_info() WHERE attname = 'template';"
	not_null_query_str = "SELECT n_constraints_on_table('{tbl}');"
	
	template_str = '''
CREATE SEQUENCE production.{tbl}_template_uid;
CREATE TABLE production.{tbl}_template (
	LIKE production.{tbl},
	CONSTRAINT pk_{tbl}_template PRIMARY KEY (uid),
	CONSTRAINT r{tbl}_templ FOREIGN KEY ("template") REFERENCES {tbl}_template(uid)
);
ALTER TABLE {tbl}_template ALTER COLUMN uid SET DEFAULT nextval('{tbl}_template_uid'::regclass);

'''
	drop_not_null_str = "ALTER TABLE {0}_template ALTER COLUMN {1} DROP NOT NULL;"

	def __init__(self,db_connection):
		self.plpy = db_connection;
		self.plpy.execute("SET search_path TO deska,production")

		# init set of tables
		self.tables = set()

		# select all tables
		record = self.plpy.execute(self.table_query_str)
		for tbl in record:
			self.tables.add(tbl[0])
		
		self.not_null_dict = dict()
		for table_name in self.tables:
			not_null_record = self.plpy.execute(self.not_null_query_str.format(tbl = table_name))
			self.not_null_dict[table_name] = list()
			for row in not_null_record:
				if row[0] != 'uid' and row[0] != 'name':
					self.not_null_dict[table_name].append(row[0])
					

	# generate sql for all tables
	def gen_templates(self,filename):
		self.sql = open(filename,'w')

		# print this to add proc into genproc schema
		self.sql.write("SET search_path TO production;\n")

		for tbl in self.tables:
			self.sql.write(self.gen_table_template(tbl))
			self.sql.write(self.gen_table_drop_not_null(tbl))

		self.sql.close()
		return

	# generate sql for one table
	def gen_table_template(self,table_name):
		return self.template_str.format(tbl = table_name)

	def gen_table_drop_not_null(self, table_name):
		constraints = list(map(self.drop_not_null_str.format,[table_name] * len(self.not_null_dict[table_name]),self.not_null_dict[table_name]))
		return '\n'.join(constraints) + '\n'
		