#!/usr/bin/python2

class Table:
	# template uid sequence
	uidseq_string = '''CREATE SEQUENCE history.{name}_uid START 1;
'''
	# template string for generate historic table
	hist_string = uidseq_string + '''CREATE TABLE history.{name}_history (
	--LIKE {name},
	uid bigint NOT NULL default nextval('{name}_uid'),
	name TEXT NOT NULL,
	version int NOT NULL,
	dest_bit bit(1) NOT NULL DEFAULT B'0',
	CONSTRAINT {name}_history_pk PRIMARY KEY (uid,version)
	);'''
	# template string for set function's
	set_string = '''CREATE FUNCTION
	{name}_set_{colname}(IN id integer,IN value {coltype},IN ver integer)
	RETURNS integer
	AS
	$$
	BEGIN
		UPDATE {name}_history SET {colname} = value, version = ver
			WHERE uid = id;
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql;
	'''
	# template string for add function
	add_string = '''CREATE FUNCTION
	{name}_add(IN name_ text,IN ver integer)
	RETURNS integer
	AS
	$$
	BEGIN
		INSERT INTO {name}_history (name,version)
			VALUES (name_,ver);
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql;
	'''
	# template string for del function
	del_string = '''CREATE FUNCTION
	{name}_del(IN name_ text,IN ver integer)
	RETURNS integer
	AS
	$$
	DECLARE id bigint;
	BEGIN	
		SELECT max(uid) INTO id FROM vendor_history
			WHERE name = name_;
		INSERT INTO vendor_history (uid, name, version, dest_bit)
			VALUES (id, name_, ver, '1');
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql;
	'''
	# template string for commit function
	commit_string = '''CREATE FUNCTION
	{name}_commit(IN ver integer)
	RETURNS integer
	AS
	$$
	BEGIN
		UPDATE {name} as v SET name = new.name
			FROM {name}_history as new
				WHERE new.version = ver AND v.uid = new.uid;
		INSERT INTO {name} (uid,name)
			SELECT uid,name FROM {name}_history
				WHERE version = ver AND uid NOT IN ( SELECT uid FROM {name} );
		DELETE FROM {name}
			WHERE uid IN (SELECT uid FROM {name}_history
				WHERE version = ver AND dest_bit = '1');
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql;
	'''
	def __init__(self,name):
		self.data = dict()
		self.col = dict()
		self.name= name

	def add_column(self,col_name,col_type):
		self.col[col_name] = col_type

	def gen_hist(self):
		return self.hist_string.format(name = self.name)

	def gen_add(self):
		return self.add_string.format(name = self.name)

	def gen_del(self):
		return self.del_string.format(name = self.name)

	def gen_set(self,col_name):
		return self.set_string.format(name = self.name,colname = col_name, coltype = self.col[col_name])

	def gen_commit(self):
		#TODO if there is more columns...
		return self.commit_string.format(self.name)


class Api:
	# template string for set function's
	set_string = '''def {tbl}_set_{col}(objectName,value):
	return db.callproc("{tbl}_set_{col}",[objectName, value])
'''
	# template string for add function
	add_string = '''def {tbl}_add(objectName):
	return db.callproc("{tbl}_add",[objectName])
'''
	# template string for del function
	del_string = '''def {tbl}_del(objectName):
	return db.callproc("{tbl}_del",[objectName])
'''
	# template string for commit function
	commit_string = '''def {tbl}_commit(ver):
	return db.callproc("{tbl}_commit",[ver])
'''
	def __init__(self,name):
		self.data = dict()
		self.col = dict()
		self.name = name

	def add_column(self,col_name,col_type):
		self.col[col_name] = col_type

	def gen_hist(self):
		return self.hist_string.format(tbl = self.name)

	def gen_add(self):
		return self.add_string.format(tbl = self.name)

	def gen_del(self):
		return self.del_string.format(tbl = self.name)

	def gen_set(self,col_name):
		return self.set_string.format(tbl = self.name, col = col_name)

	def gen_commit(self):
		#TODO if there is more columns...
		return self.commit_string.format(tbl = self.name)

#
# just testing it
#vendor = Table('vendor')
#vendor.add_column('name','text')
#print vendor.gen_hist()
#print vendor.gen_set('name')
#print vendor.gen_add()
#print vendor.gen_del()
#print vendor.gen_commit()

