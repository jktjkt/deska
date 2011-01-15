#!/usr/bin/python2.4

class Table:
	hist_string = '''CREATE TABLE {0[name]}_history (
		LIKE {0[name]},
		version int NOT NULL,
		dest_bit bit(1) NOT NULL DEFAULT B'0',
		CONSTRAINT {0[name]}_history_pk PRIMARY KEY (uid,version)
		)'''
	set_string = '''CREATE FUNCITION
		{0[name]}_{1}_set(IN id integer,IN value {2})
		RETURNS interger
		AS
		$$
			UPDATE TABLE {0[name]}_history SET {1} = value
				WHERE uid = id
		$$
		LAGUAGE plpsql;
		'''
	def __init__(self,name):
		self.data = dict()
		self.col = dict()
		self.data['name'] = name

	def add_column(self,col_name,col_type):
		self.col[col_name] = col_type

	def gen_hist(self):
		return self.hist_string.format(self.data)

	def gen_add(self,col_name):
		return 'Not implemented yet'

	def gen_del(self,col_name):
		return 'Not implemented yet'

	def gen_set(self,col_name):
		return self.set_string.format(self.data,col_name,self.col[col_name])

# just testing it
vendor = Table('vendor')
vendor.add_column('name','text')
print vendor.gen_hist()
print vendor.gen_set('name')

