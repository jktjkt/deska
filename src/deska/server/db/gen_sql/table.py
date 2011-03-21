#!/usr/bin/python2

import string

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
			
	#needed for set functiom
	#returns map of collname (column that references uid in some table) and tablename (table that is referenced)
	def references_uid(self):
		result = dict()
		for conname in self.att:
			refattributes = self.ratt[conname]
			if ('uid' in refattributes):
				index = self.ratt[conname].index('uid')
				namecol = self.att[conname][index]
				result[namecol] = self.tbl[conname]
		return result

	def embed_col(self):
		for conname in self.att:
			if (string.find(conname,'rembed_') == 0):
				return self.att[conname]
		return ""


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
		rowuid bigint;
	BEGIN
		SELECT my_version() INTO ver;
		SELECT {tbl}_get_uid(name_) INTO rowuid;
		UPDATE {tbl}_history SET {colname} = CAST (value AS {coltype}), version = ver
			WHERE uid = rowuid AND version = ver;
		--TODO if there is nothing in current version???
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	# template string for set function's
	set_name_string = '''CREATE FUNCTION
	{tbl}_set_name(IN {tbl}_uid bigint,IN name_ text)
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
	BEGIN
		SELECT my_version() INTO ver;
		UPDATE {tbl}_history SET name = name_, version = ver
			WHERE uid = {tbl}_uid AND version = ver;
		--TODO if there is nothing in current version???
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	#template for setting uid of referenced row that has name attribute value
	#value could be composed of names that are in embed into chain
	set_fk_uid_string = '''CREATE FUNCTION
	{tbl}_set_{colname}(IN name_ text,IN value text)
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
		refuid bigint;
		rowuid bigint;
	BEGIN
		SELECT my_version() INTO ver;
		SELECT {reftbl}_get_uid(value) INTO refuid;
		SELECT {tbl}_get_uid(name_) INTO rowuid;
		UPDATE {tbl}_history SET {colname} = refuid, version = ver
			WHERE uid = rowuid AND version = ver;
		--TODO if there is nothing in current version???
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	get_data_type_string='''CREATE TYPE {tbl}_type AS(
{columns}
);
'''
	# template string for get data functions
	get_data_string = '''CREATE FUNCTION
	{tbl}_get_data(IN name_ text)
	RETURNS SETOF {tbl}_type
	AS
	$$
	DECLARE	ver bigint;
	BEGIN
		SELECT my_version() INTO ver;
		RETURN QUERY 
			SELECT {columns} FROM {tbl}_history
			WHERE name = name_ AND version = ver
			UNION
			SELECT {columns} FROM {tbl}
			WHERE name = name_ AND name NOT IN 
				(SELECT name FROM {tbl}_history
				WHERE name = name_ AND version = ver);
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	#template string for get functions
	get_string = '''CREATE FUNCTION
	{tbl}_get_{colname}(IN name_ text)
	RETURNS {coltype}
	AS
	$$
	DECLARE
		ver bigint;
		value {coltype};
	BEGIN
		SELECT my_version() INTO ver;
		SELECT {colname} INTO value
			FROM {tbl}_history
			WHERE name = name_ AND version = ver;
		--if the value isn't in current version then it should be found in production
		IF NOT FOUND THEN
			SELECT {colname} INTO value
			FROM {tbl}
			WHERE name = name_;
		END IF;		
		RETURN value;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	#template string for get_uid functions (for name finds corresponding uid)
	#not for kind that are embed into another
	get_uid_string = '''CREATE FUNCTION
	{tbl}_get_uid(IN name_ text)
	RETURNS bigint
	AS
	$$
	DECLARE
		ver bigint;
		value bigint;
	BEGIN
		SELECT my_version() INTO ver;
		SELECT uid INTO value
			FROM {tbl}_history
			WHERE name = name_ AND version = ver;
		--if the value isn't in current version then it should be found in production
		IF NOT FOUND THEN
			SELECT uid INTO value
			FROM {tbl}
			WHERE name = name_;
		END IF;		
		RETURN value;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	#template string for get functions
	get_name_string = '''CREATE FUNCTION
	{tbl}_get_name(IN {tbl}_uid bigint)
	RETURNS text
	AS
	$$
	DECLARE
		ver bigint;
		value text;
	BEGIN
		SELECT my_version() INTO ver;
		SELECT name INTO value
			FROM {tbl}_history
			WHERE uid = {tbl}_uid AND version = ver;
		--if the value isn't in current version then it should be found in production
		IF NOT FOUND THEN
			SELECT name INTO value
			FROM {tbl}
			WHERE uid = {tbl}_name;
		END IF;		
		RETURN value;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	#template for function getting uid of object embed into another
	get_uid_embed_string = '''CREATE OR REPLACE FUNCTION {tbl}_get_uid(full_name text)
	RETURNS bigint
	AS
	$$
	DECLARE
		{reftbl}_uid bigint;
		rest_of_name text;
		pos integer;
		{tbl}_name char(64);
		{tbl}_uid bigint;
	BEGIN
		pos = position('_' IN full_name);
		IF pos = 0 THEN
			RAISE EXCEPTION 'this object is embed into another, this name is not fully qualifide';
		END IF;
		{tbl}_name = substr(full_name,1,pos - 1);
		rest_of_name = substr(full_name,pos + 1);
		SELECT {reftbl}_get_uid(rest_of_name) INTO {reftbl}_uid;
		SELECT uid INTO {tbl}_uid FROM {tbl}_history WHERE name = {tbl}_name AND {column} = {reftbl}_uid;
		IF NOT FOUND THEN
			SELECT uid INTO {tbl}_uid
			FROM {tbl}
			WHERE name = {tbl}_name AND {column} = {reftbl}_uid;
		END IF;		
		RETURN {tbl}_uid;
	END;
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
	# template string for add function
	add_embed_string = '''CREATE FUNCTION
	{tbl}_add(IN fullname text)
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
		{reftbl}_uid bigint;
		rest_of_name text;
		pos integer;
		{tbl}_name char(64);
	BEGIN
		pos = position('_' IN fullname);
		IF pos = 0 THEN
			RAISE EXCEPTION 'this object is embed into another, this name is not fully qualifide';
		END IF;
		{tbl}_name = substr(fullname,1,pos - 1);
		rest_of_name = substr(fullname,pos + 1);
		SELECT {reftbl}_get_uid(rest_of_name) INTO {reftbl}_uid;
		SELECT my_version() INTO ver;
		INSERT INTO {tbl}_history(name, {column}, version) VALUES ({tbl}_name, {reftbl}_uid, ver);
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
	# template string for names
	names_string = '''CREATE FUNCTION
	{tbl}_names()
	RETURNS SETOF char(64) --FIXME this should be set dynamic
	AS
	$$
	BEGIN
		RETURN QUERY SELECT name FROM {tbl};
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

	def get_cols_reference_uid(self):
		return self.fks.references_uid()

	def get_col_embed_reference_uid(self):
		return self.fks.embed_col()

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

	def gen_add_embed(self, col_name, reftable):
		return self.add_embed_string.format(tbl = self.name, column = col_name, reftbl = reftable)

	def gen_del(self):
		return self.del_string.format(tbl = self.name)

	def gen_set(self,col_name):
		return self.set_string.format(tbl = self.name,colname = col_name, coltype = self.col[col_name])

	def gen_set_ref_uid(self,col_name, reftable):
		return self.set_fk_uid_string.format(tbl = self.name, colname = col_name, coltype = self.col[col_name], reftbl = reftable)

	def gen_set_name(self):
		return self.set_name_string.format(tbl = self.name)

	def gen_get_object_data(self):
		collist = self.col.copy()
		del collist['uid']
		del collist['name']
		if len(collist) == 0:
			return ""
		cols = ",".join(collist)
		coltype_string = "{colname} {coltype}"
		coltypeslist = list()
		for col in collist:
			coltypeslist.append(coltype_string.format(colname = col, coltype = collist[col]))
		coltypes = ",\n".join(coltypeslist)
		return self.get_data_type_string.format(tbl = self.name, columns = coltypes) + "\n" + self.get_data_string.format(tbl = self.name, columns = cols)

	def gen_get(self,col_name):
		return self.get_string.format(tbl = self.name,colname = col_name, coltype = self.col[col_name])

	def gen_get_name(self):
		return self.get_name_string.format(tbl = self.name)

	def gen_get_uid(self):
		return self.get_uid_string.format(tbl = self.name)
	
	def gen_get_uid_embed(self, refcolumn, reftable):
		return self.get_uid_embed_string.format(tbl = self.name, column = refcolumn, reftbl = reftable)


	def gen_commit(self):
		#TODO if there is more columns...
		return self.commit_string.format(tbl = self.name, assign = self.gen_cols_assign(), columns = self.get_columns())

	def gen_names(self):
		return self.names_string.format(tbl = self.name)


class Api:
	# template string for set function's
	set_string = '''def {tbl}_set_{col}(objectName,value):
	return db.callproc("{tbl}_set_{col}",[objectName, value])
'''
	# template string for get functions
	get_string = '''def {tbl}_get_{col}(objectName):
	return db.callproc("{tbl}_get_{col}",[objectName])
'''
	# template string for get functions
	get_data_string = '''def {tbl}_get_data(objectName):
	return db.callproc("{tbl}_get_data",[objectName])
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

	def gen_get_object_data(self):
		return self.get_data_string.format(tbl = self.name)

	def gen_get(self,col_name):
		return self.get_string.format(tbl = self.name, col = col_name)

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

