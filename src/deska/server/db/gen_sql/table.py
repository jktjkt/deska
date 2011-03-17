#!/usr/bin/python2

class PkSet(dict):
	def __init__(self):
		dict.__init__(self)
	
	def __setitem__(self,name,att):
		if self.has_key(name):
			s = dict.__getitem__(self,name)
			s.append(att)
			dict.__setitem__(self,name,s)
		else:
			s = list()
			s.append(att)
			dict.__setitem__(self,name,s)

# foreign keys
class Fks():
	def __init__(self):
		self.att = PkSet()
		self.ratt = PkSet()
		self.tbl = dict()
	
	def add(self,name,att,table,ratt):
		self.att[name] = att
		self.tbl[name] = table
		self.ratt[name] = ratt
	
	def gen_fkcon(self,con):
		# add version column into key constraint
		# workround for add fks to objects from earlier changesets
		#self.att[con] = "version"
		#self.ratt[con] = "version"
		str = "CONSTRAINT history_{name} FOREIGN KEY ({att}) REFERENCES {rtbl}_history({ratt})"
		atts = ",".join(self.att[con])
		ratts = ",".join(self.ratt[con])
		str = str.format(name = con,rtbl = self.tbl[con],att = atts, ratt = ratts)
		return "ALTER TABLE {tbl}_history ADD " + str
	
	def gen_fk_constraints(self):
		constr = ""
		for att in self.att:
			constr = constr + self.gen_fkcon(att) + ";\n" 
		return constr
			
		

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
	{tbl}_set_{colname}(IN name_ text,IN value text)
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
	BEGIN
		SELECT my_version() INTO ver;
		UPDATE {tbl}_history SET {colname} = CAST (value AS {coltype}), version = ver
			WHERE name = name_ AND version = ver;
		--TODO if there is nothing in current version???
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

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
	LANGUAGE plpgsql SECURITY DEFINER;

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
		SELECT max(uid) INTO id FROM {tbl}_history
			WHERE name = name_;
		INSERT INTO {tbl}_history (uid, name, version, dest_bit)
			VALUES (id, name_, ver, '1');
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

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
		UPDATE {tbl} as tbl SET {assign}
			FROM {tbl}_history as new
				WHERE new.version = ver AND tbl.uid = new.uid AND dest_bit = '0';
		INSERT INTO {tbl} ({columns})
			SELECT {columns} FROM {tbl}_history
				WHERE version = ver AND uid NOT IN ( SELECT uid FROM {tbl} ) AND dest_bit = '0';
		DELETE FROM {tbl}
			WHERE uid IN (SELECT uid FROM {tbl}_history
				WHERE version = ver AND dest_bit = '1');
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	def __init__(self,name):
		self.data = dict()
		self.col = dict()
		self.pkset = PkSet()
		self.fks = Fks()
		self.name= name

	def add_column(self,col_name,col_type):
		self.col[col_name] = col_type

	# add pk and unique
	def add_pk(self,con_name,att_name):
		self.pkset[con_name] = att_name

	# add fk 
	def add_fk(self,con_name,att_name,ref_table,ref_att):
		self.fks.add(con_name,att_name,ref_table,ref_att)

	def gen_assign(self,colname):
		return "{col} = new.{col}".format(col= colname)

	def gen_cols_assign(self):
		#TODO remove uid
		assign = map(self.gen_assign,self.col.keys())
		return ",".join(assign)

	def get_columns(self):
		# comma separated list of values
		return ",".join(self.col.keys())

	def gen_pk_constraint(self,con):
		# add version column into key constraint
		self.pkset[con] = "version"
		str = "CONSTRAINT history_{name} UNIQUE(".format(name = con)
		str = str + ",".join(self.pkset[con])
		return str + ")"

	def gen_drop_notnull(self):
		nncol = self.col.copy()
		del nncol['uid']
		del nncol['name']
		drop = ""
		for col in nncol:
			drop = drop + "ALTER TABLE {name}_history ALTER {colname} DROP NOT NULL;\n".format(name = self.name, colname = col)
		return drop
	
	def gen_fks(self):
		return self.fks.gen_fk_constraints().format(tbl = self.name)
	
	
	def gen_hist(self):
		constr = ""
		for con in self.pkset:
			constr = constr + ",\n" + self.gen_pk_constraint(con)
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
		return self.commit_string.format(tbl = self.name, assign = self.gen_cols_assign(), columns = self.get_columns())


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

