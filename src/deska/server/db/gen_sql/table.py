#!/usr/bin/python2

class Table:
	# template string for generate historic table
	hist_string = '''CREATE TABLE history.{0[name]}_history (
	LIKE {0[name]},
	version int NOT NULL,
	dest_bit bit(1) NOT NULL DEFAULT B'0',
	CONSTRAINT {0[name]}_history_pk PRIMARY KEY (uid,version)
	);'''
	# template string for set function's
	set_string = '''CREATE FUNCTION
	{0[name]}_set_{1}(IN id integer,IN value {2},IN ver integer)
	RETURNS integer
	AS
	$$
	BEGIN
		UPDATE {0[name]}_history SET {1} = value
			WHERE uid = id;
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql;
	'''
	# template string for add function
	add_string = '''CREATE FUNCTION
	{0[name]}_add(IN name_ text,IN ver integer)
	RETURNS integer
	AS
	$$
	BEGIN
		INSERT INTO {0[name]}_history (name,version)
			VALUES (name_,ver);
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql;
	'''
	# template string for del function
	del_string = '''CREATE FUNCTION
	{0[name]}_del(IN id integer,IN ver integer)
	RETURNS integer
	AS
	$$
	DECLARE name text;
	BEGIN	
		SELECT DISTINCT name INTO name FROM vendor_history
			WHERE uid = id;
		INSERT INTO vendor_history (uid, version, dest_bit)
			VALUES (id, name, ver, '1');
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql;
	'''
	# template string for commit function
	commit_string = '''CREATE FUNCTION
	{0[name]}_commit(IN ver integer)
	RETURNS integer
	AS
	$$
	BEGIN
		UPDATE {0[name]} as v SET name = new.name
			FROM {0[name]}_history as new
				WHERE version = ver AND v.uid = new.uid;
		INSERT INTO {0[name]} (uid,name)
			SELECT uid,name FROM {0[name]}_history
				WHERE version = ver AND uid NOT IN ( SELECT uid FROM {0[name]} );
		DELETE FROM {0[name]}
			WHERE uid IN (SELECT uid FROM {0[name]}_history
				WHERE version = ver AND dest_bit = '1');
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql;
	'''
	def __init__(self,name):
		self.data = dict()
		self.col = dict()
		self.data['name'] = name

	def add_column(self,col_name,col_type):
		self.col[col_name] = col_type

	def gen_hist(self):
		return self.hist_string.format(self.data)

	def gen_add(self):
		return self.add_string.format(self.data)

	def gen_del(self):
		return self.del_string.format(self.data)

	def gen_set(self,col_name):
		return self.set_string.format(self.data,col_name,self.col[col_name])

	def gen_commit(self):
		#TODO if there is more columns...
		return self.commit_string.format(self.data)

#
# just testing it
#vendor = Table('vendor')
#vendor.add_column('name','text')
#print vendor.gen_hist()
#print vendor.gen_set('name')
#print vendor.gen_add()
#print vendor.gen_del()
#print vendor.gen_commit()

