
class Template:
	table_str = "SELECT relname FROM get_table_info() WHERE attname = 'template';"
	template_str = '''
CREATE SEQUENCE production.{tbl}_template_uid;
CREATE TABLE production.{tbl}_template (
	LIKE production.{tbl},
	CONSTRAINT pk_{tbl}_template PRIMARY KEY (uid),
	CONSTRAINT rtempl_templ FOREIGN KEY ("template") REFERENCES {tbl}_template(uid)
);
ALTER TABLE {tbl}_template ALTER COLUMN uid SET DEFAULT nextval('{tbl}_template_uid'::regclass);

'''

	def __init__(self,db_connection):
		self.plpy = db_connection;
		self.plpy.execute("SET search_path TO deska,production")

		# init set of tables
		self.tables = set()

		# select all tables
		record = self.plpy.execute(self.table_str)
		for tbl in record:
			self.tables.add(tbl[0])

	# generate sql for all tables
	def gen_templates(self,filename):
		self.sql = open(filename,'w')

		# print this to add proc into genproc schema
		self.sql.write("SET search_path TO production;\n")

		for tbl in self.tables:
			self.sql.write(self.gen_template(tbl))

		self.sql.close()
		return

	# generate sql for one table
	def gen_template(self,table_name):
		return self.template_str.format(tbl = table_name)
