#!/usr/bin/python2.4
#
# Small script to show PostgreSQL and Psycopg2 together
#

import psycopg2
from table import Table,Api
  
class Plpy:
	def __init__(self):
		try:
			#conn = psycopg2.connect("dbname='deska_dev' user='kerpl' host='localhost' port=6666");
			conn = psycopg2.connect("dbname='deska_dev' user='deska' host='localhost' password='deska'");
			self.mark = conn.cursor()

		except:
			print "I am unable to connect to the database"

	def execute(self,statement):
		self.mark.execute(statement)
		try:
			return self.mark.fetchall()
		except:
			return ""

# connect to db
plpy = Plpy()

class Schema:
	table_str = "SELECT DISTINCT relname from deska.table_info_view"
	column_str = "SELECT attname,typname from deska.table_info_view where relname='{0}'"
	pk_str = "SELECT conname,attname FROM key_constraints_on_table('{0}')"
	fk_str = "SELECT conname,attname,reftabname,refattname FROM fk_constraints_on_table('{0}')"
	def __init__(self):
		plpy.execute("SET search_path TO deska,production")

		# init set of tables
		self.tables = set()

		# select all tables
		record = plpy.execute(self.table_str)
		for tbl in record[:]:
			self.tables.add(tbl[0])

		# print foreign keys at the end
		self.fks = ""

	# generate sql for all tables
	def gen_schema(self):
		self.sql = open('gen_schema.sql','w')
		self.py = open('db.py','w')

		# print this to add proc into genproc schema
		self.sql.write("SET search_path TO genproc,history,deska,production;")

		for tbl in self.tables:
			self.gen_for_table(tbl)

		# print fks at the end of generation
		self.sql.write(self.fks)

		self.py.close()
		self.sql.close()
		return 

	# generate sql for one table
	def gen_for_table(self,tbl):
		# select col info
		tables = plpy.execute(self.column_str.format(tbl))

		# create table obj
		table = Table(tbl)
		# create Api obj
		api = Api(tbl)
		
		# add columns
		for col in tables[:]:
			table.add_column(col[0],col[1])

		# add pk constraints
		constraints = plpy.execute(self.pk_str.format(tbl))
		for col in constraints[:]:
			table.add_pk(col[0],col[1])

		# add fk constraints
		constraints = plpy.execute(self.fk_str.format(tbl))
		for col in constraints[:]:
			table.add_fk(col[0],col[1],col[2],col[3])

		# generate sql
		self.sql.write(table.gen_hist())
		self.fks = self.fks + (table.gen_fks())
		for col in tables[:]:
			 self.sql.write(table.gen_set(col[0]))
			 self.py.write(api.gen_set(col[0]))
		self.sql.write(table.gen_add())
		self.py.write(api.gen_add())
		self.sql.write(table.gen_del())
		self.py.write(api.gen_del())
		self.sql.write(table.gen_commit())
		self.py.write(api.gen_commit())
		return 

# just testing it
schema = Schema()

#schema.gen_for_table('vendor')
schema.gen_schema()
