#!/usr/bin/python2

class ConSet(dict):
	def __init__(self):
		dict.__init__(self)
	
	def __setitem__(self,name,att):
		if self.has_key(name):
			s = dict.__getitem__(self,name)
			s.add(att)
			dict.__setitem__(self,name,s)
		else:
			s = set()
			s.add(att)
			dict.__setitem__(self,name,s)
	

class Table:
	# template string for generate historic table
	hist_string = '''CREATE TABLE history.{tbl}_history (
	LIKE {tbl}
	-- include default values
	INCLUDING DEFAULTS
	-- include CHECK constrants !!! only check constraints in postgresql 9
	INCLUDING CONSTRAINTS
	-- INCLUDE INDEXES???
	,
	version int NOT NULL,
	dest_bit bit(1) NOT NULL DEFAULT B'0'
	{constraints}
);
'''
	# template string for set function's
	set_string = '''CREATE FUNCTION
	{tbl}_set_{colname}(IN name_ text,IN value {coltype})
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
	BEGIN
		SELECT my_version() INTO ver;
		UPDATE {tbl}_history SET {colname} = value, version = ver
			WHERE name = name_;
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql;

'''
	# template string for add function
	add_string = '''CREATE FUNCTION
	{tbl}_add(IN name_ text)
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
	BEGIN
		SELECT my_version() INTO ver;
		INSERT INTO {tbl}_history (name,version)
			VALUES (name_,ver);
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql;

'''
	# template string for del function
	del_string = '''CREATE FUNCTION
	{tbl}_del(IN name_ text)
	RETURNS integer
	AS
	$$
	DECLARE id bigint;
		ver bigint;
	BEGIN	
		SELECT my_version() INTO ver;
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
	{tbl}_commit()
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
	BEGIN
		SELECT my_version() INTO ver;
		UPDATE {tbl} as v SET name = new.name
			FROM {tbl}_history as new
				WHERE new.version = ver AND v.uid = new.uid;
		INSERT INTO {tbl} (uid,name)
			SELECT uid,name FROM {tbl}_history
				WHERE version = ver AND uid NOT IN ( SELECT uid FROM {tbl} );
		DELETE FROM {tbl}
			WHERE uid IN (SELECT uid FROM {tbl}_history
				WHERE version = ver AND dest_bit = '1');
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql;

'''
	def __init__(self,name):
		self.data = dict()
		self.col = dict()
		self.conset = ConSet()
		self.name= name

	def add_column(self,col_name,col_type):
		self.col[col_name] = col_type

	def add_key(self,con_name,att_name):
		self.conset[con_name] = att_name

	def gen_constraint(self,con):
		str = "CONSTRAINT history_{name} UNIQUE(".format(name = con)
		comma = False
		for att in self.conset[con]:
			if comma:
				str = str + "," + att
			else:
				comma = True
				str = str + att
		return str + ")"
	
	def gen_drop_notnull(self):
		nncol = self.col.copy()
		del nncol['uid']
		del nncol['name']
		drop = ""
		for col in nncol:
			drop = drop + "ALTER TABLE {name}_history ALTER {colname} DROP NOT NULL;\n".format(name = self.name, colname = col)
		return drop

	def gen_hist(self):
		constr = ""
		for con in self.conset:
			constr = constr + ",\n" + self.gen_constraint(con)
		drop = self.gen_drop_notnull()
		return self.hist_string.format(tbl = self.name, constraints = constr) + drop

	def gen_add(self):
		return self.add_string.format(tbl = self.name)

	def gen_del(self):
		return self.del_string.format(tbl = self.name)

	def gen_set(self,col_name):
		return self.set_string.format(tbl = self.name,colname = col_name, coltype = self.col[col_name])

	def gen_commit(self):
		#TODO if there is more columns...
		return self.commit_string.format(tbl = self.name)


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

