#!/usr/bin/python2

#delimiter string
DELIMITER = "->"

class Templates:
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

	# template string for set functions
	set_string = '''CREATE FUNCTION
	{tbl}_set_{colname}(IN name_ text,IN value text)
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
		rowuid bigint;
		tmp bigint;
	BEGIN
		--for modifications we need to have opened changeset, this function raises exception in case we don't have
		SELECT get_current_changeset() INTO ver;
		SELECT {tbl}_get_uid(name_) INTO rowuid;
		--not found in case there is no object with name name_ in history
		IF NOT FOUND THEN
			RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '70021';
		END IF;
		-- try if there is already line for current version
		SELECT uid INTO tmp FROM {tbl}_history
			WHERE uid = rowuid AND version = ver;
		--object with given name was not modified in this version
		--we need to get its current data to this version
		IF NOT FOUND THEN
			INSERT INTO {tbl}_history ({columns},version)
				SELECT {columns},ver FROM {tbl}_data_version(id2num(parent(ver))) WHERE uid = rowuid;
		END IF;
		--set new value in {colname} column
		UPDATE {tbl}_history SET {colname} = CAST (value AS {coltype}), version = ver
			WHERE uid = rowuid AND version = ver;
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
		tmp bigint;
	BEGIN
		SELECT get_current_changeset() INTO ver;
		--value is name of object in reftable
		--we need to know uid of referenced object instead of its name
		IF value IS NULL THEN
			refuid = NULL;
		ELSE
			SELECT {reftbl}_get_uid(value) INTO refuid;
		END IF;
		
		SELECT {tbl}_get_uid(name_) INTO rowuid;
		-- try if there is already line for current version
		SELECT uid INTO tmp FROM {tbl}_history
			WHERE uid = rowuid AND version = ver;
		--object with given name was not modified in this version
		--we need to get its current data to this version			
		IF NOT FOUND THEN
			INSERT INTO {tbl}_history ({columns},version)
				SELECT {columns},ver FROM {tbl}_data_version(id2num(parent(ver))) WHERE uid = rowuid;
		END IF;
		--set column to refuid - uid of referenced object
		UPDATE {tbl}_history SET {colname} = refuid, version = ver
			WHERE uid = rowuid AND version = ver;
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	set_name_embed_string = '''CREATE FUNCTION
	{tbl}_set_name(IN name_ text,IN new_name text)
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
		refuid bigint;
		rowuid bigint;
		tmp bigint;
		refname text;
		local_name text;
	BEGIN
		SELECT get_current_changeset() INTO ver;
		--value is name of object in reftable
		--we need to know uid of referenced object instead of its name
		SELECT {tbl}_get_uid(name_) INTO rowuid;
		-- try if there is already line for current version
		SELECT uid INTO tmp FROM {tbl}_history
			WHERE uid = rowuid AND version = ver;
		--object with given name was not modified in this version
		--we need to get its current data to this version			
		IF NOT FOUND THEN
			INSERT INTO {tbl}_history ({columns},version)
				SELECT {columns},ver FROM {tbl}_data_version(id2num(parent(ver))) WHERE uid = rowuid;
		END IF;
		--set column to refuid - uid of referenced object
		SELECT embed_name[1], embed_name[2] FROM embed_name(new_name,'{delim}') INTO refname, local_name;
		refuid = {reftbl}_get_uid(refname);
		UPDATE {tbl}_history SET {refcolumn} = refuid, name = local_name, version = ver
			WHERE uid = rowuid AND version = ver;
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;
	
'''

	#template for data_type, given with columns - list of attributes' names and their types
	get_data_type_string='''CREATE TYPE {tbl}_type AS(
{columns}
);
'''
	# template string for get data functions
	# get data of object name_ in given version
	get_data_string = '''CREATE FUNCTION
	{tbl}_get_data(IN name_ text, from_version bigint = 0)
	RETURNS {tbl}_type
	AS
	$$
	DECLARE
		current_changeset bigint;
		data {tbl}_type;
	BEGIN
		IF from_version = 0 THEN
			current_changeset = get_current_changeset_or_null();
			IF current_changeset IS NULL THEN
				--user wants current data from production
				SELECT {columns} INTO data FROM production.{tbl} WHERE name = name_;
				IF NOT FOUND THEN
					RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '70021';
				END IF;
				RETURN data;
			END IF;
			
			SELECT {columns} INTO data FROM {tbl}_history WHERE name = name_ AND version = current_changeset;
			IF FOUND THEN
				--we have result and can return it
				RETURN data;
			END IF;
			--object name_ is not present in current changeset, we need look for it in parent revision or erlier
			from_version = id2num(parent(current_changeset));
		END IF;

		SELECT {columns} INTO data FROM {tbl}_data_version(from_version)
			WHERE name = name_;
		IF NOT FOUND THEN
			RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '70021';
		END IF;

		RETURN data;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	# template string for get data functions with embed flag
	get_embed_data_string = '''CREATE FUNCTION
	{tbl}_get_data(IN name_ text, from_version bigint = 0)
	RETURNS {tbl}_type
	AS
	$$
	DECLARE	
		current_changeset bigint;
		obj_uid bigint;
		data {tbl}_type;
	BEGIN
		obj_uid = {tbl}_get_uid(name_, from_version);
		IF obj_uid IS NULL THEN
			RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '70021';
		END IF;
		
		IF from_version = 0 THEN
			current_changeset = get_current_changeset_or_null();
			IF current_changeset IS NULL THEN
				--user wants current data from production
				--name in table is only local part of object, we should look for object by uid
				SELECT {columns} INTO data FROM production.{tbl} WHERE uid = obj_uid;
				IF NOT FOUND THEN
					RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '70021';
				END IF;				
				RETURN data;
			END IF;
			--first we look for result in current changeset than in parent revision
			SELECT {columns} INTO data FROM {tbl}_history WHERE uid = obj_uid AND version = current_changeset;
			IF FOUND THEN
				--we have result and can return it
				RETURN data;
			END IF;
			--object name_ is not present in current changeset, we need look for it in parent revision or erlier
			from_version = id2num(parent(current_changeset));
		END IF;
		
		SELECT {columns} INTO data FROM {tbl}_data_version(from_version)
			WHERE uid = obj_uid;
		IF NOT FOUND THEN
			RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '70021';
		END IF;

		RETURN data;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	#template string for get_uid functions (for name finds corresponding uid)
	#not for kind that are embed into another
	get_uid_string = '''CREATE FUNCTION
	{tbl}_get_uid(IN name_ text, from_version bigint = 0)
	RETURNS bigint
	AS
	$$
	DECLARE
		changeset_id bigint;
		value bigint;
	BEGIN
	--from_version is user id of version
	--we need id of changeset
		IF from_version = 0 THEN
			changeset_id = get_current_changeset_or_null();
			IF changeset_id IS NULL THEN
				--user wants current uid from production
				SELECT uid INTO value FROM production.{tbl} WHERE name = name_;
				IF NOT FOUND THEN
					RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '70021';
				END IF;
				RETURN value;
			END IF;
			
			SELECT uid INTO value FROM {tbl}_history WHERE name = name_ AND version = changeset_id;
			IF FOUND THEN
				--we have result and can return it
				RETURN value;
			END IF;
			--object name_ is not present in current changeset, we need look for it in parent revision or erlier
			from_version = id2num(parent(changeset_id));
		END IF;
		
		SELECT uid INTO value FROM {tbl}_data_version(from_version) WHERE name = name_;
		IF NOT FOUND THEN
			RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '70021';
		END IF;		
		RETURN value;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	#template string for get functions
	get_name_string = '''CREATE FUNCTION
	{tbl}_get_name(IN {tbl}_uid bigint, from_version bigint = 0)
	RETURNS text
	AS
	$$
	DECLARE
		value text;
		current_changeset bigint;
	BEGIN
		IF from_version = 0 THEN
			--user wants the most actual data
			current_changeset = get_current_changeset_or_null();
			IF current_changeset IS NULL THEN
				SELECT name INTO value FROM production.{tbl} WHERE uid = {tbl}_uid;
				RETURN value;
			END IF;
			
			SELECT name INTO value FROM {tbl}_history WHERE uid = {tbl}_uid AND version = current_changeset;
			IF FOUND THEN
				--we have result and can return it
				RETURN value;
			END IF;
			from_version = id2num(parent(current_changeset));
		END IF;

		SELECT name INTO value FROM {tbl}_data_version(from_version) WHERE uid = {tbl}_uid;
		RETURN value;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	#template string for get functions
	get_name_embed_string = '''CREATE FUNCTION
	{tbl}_get_name(IN {tbl}_uid bigint, from_version bigint = 0)
	RETURNS text
	AS
	$$
	DECLARE
		current_changeset bigint;
		local_name text = NULL;
		rest_of_name text;
		{reftbl}_uid bigint;
		--version from which we search for local name
		version_to_search bigint;
		value text;
	BEGIN
	--from_version we need unchanged for rest_of_name
		version_to_search = from_version;
		IF version_to_search = 0 THEN
			current_changeset = get_current_changeset_or_null();

			IF current_changeset IS NULL THEN
				SELECT join_with_delim({reftbl}_get_name({column}, from_version), name, '{delim}') INTO value FROM production.{tbl}
					WHERE uid = {tbl}_uid;
				IF NOT FOUND THEN
					RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '70021';
				END IF;
				RETURN value;
			END IF;
			
			SELECT join_with_delim({reftbl}_get_name({column}, from_version), name, '{delim}') INTO value FROM {tbl}_history WHERE uid = {tbl}_uid AND version = current_changeset;
			IF FOUND THEN
				--we have result and can return it
				RETURN value;
			END IF;
			version_to_search = id2num(parent(current_changeset));
		END IF;
			
		SELECT name, {column} INTO local_name, {reftbl}_uid FROM {tbl}_data_version(version_to_search)
			WHERE uid = {tbl}_uid;
		
		--get name of referenced object
		rest_of_name = {reftbl}_get_name({reftbl}_uid, from_version);
		--join local name of object and name of referenced object to full name of object
		RETURN join_with_delim(rest_of_name, local_name, '{delim}');
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	#template for function getting uid of object embed into another
	get_uid_embed_string = '''CREATE OR REPLACE FUNCTION {tbl}_get_uid(full_name text, from_version bigint = 0)
	RETURNS bigint
	AS
	$$
	DECLARE
		{reftbl}_uid bigint;
		rest_of_name text;
		{tbl}_name text;
		{tbl}_uid bigint;
		changeset_id bigint;
		deleted bit(1);
	BEGIN
		SELECT embed_name[1],embed_name[2] FROM embed_name(full_name,'{delim}') INTO rest_of_name,{tbl}_name;
		--finds uid of object which is this one embed into
		SELECT {reftbl}_get_uid(rest_of_name, from_version) INTO {reftbl}_uid;

		--finds id of changeset where was object with full_name changed = object with local name + uid of object which is object embed into
		--look for object in current_changeset
		IF from_version = 0 THEN
			changeset_id = get_current_changeset_or_null();
			IF changeset_id IS NULL THEN
				SELECT MAX(num) INTO from_version FROM version;
			ELSE
				SELECT uid INTO {tbl}_uid FROM {tbl}_history WHERE {column} = {reftbl}_uid AND name = {tbl}_name;
				IF NOT FOUND THEN
					--object with this name is not in current changeset, we should look for it in parent version or earlier
					from_version = id2num(parent(changeset_id));
				ELSE
					--we have result and can return it
					RETURN {tbl}_uid;
				END IF;
			END IF;
		END IF;
		
		SELECT uid INTO {tbl}_uid FROM {tbl}_data_version(from_version) WHERE name = {tbl}_name AND {column} = {reftbl}_uid;
		IF NOT FOUND THEN
			RAISE 'No {tbl} with name % exist in this version', full_name USING ERRCODE = '70022';
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
		tmp bigint;
	BEGIN
		SELECT get_current_changeset() INTO ver;
		SELECT uid INTO tmp FROM {tbl}_history WHERE version = ver AND name = name_ AND dest_bit = '1';
		IF FOUND THEN
			RAISE EXCEPTION 'object with name % was deleted, ...', name_;
		END IF;
		INSERT INTO {tbl}_history (name,version)
			VALUES (name_,ver);
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	# template string for add function
	add_embed_string = '''CREATE FUNCTION
	{tbl}_add(IN full_name text)
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
		{reftbl}_uid bigint;
		rest_of_name text;
		{tbl}_name text;
		tmp bigint;
	BEGIN
		SELECT embed_name[1],embed_name[2] FROM embed_name(full_name,'{delim}') INTO rest_of_name,{tbl}_name;
		SELECT {reftbl}_get_uid(rest_of_name) INTO {reftbl}_uid;
		SELECT get_current_changeset() INTO ver;
		SELECT uid INTO tmp FROM {tbl}_history WHERE version = ver AND uid = {reftbl}_uid AND dest_bit = '1';
		IF FOUND THEN
			RAISE EXCEPTION 'object with name % was deleted, ...', full_name;
		END IF;
		INSERT INTO {tbl}_history(name, {column}, version) VALUES ({tbl}_name, {reftbl}_uid, ver);
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	undel_string = '''CREATE FUNCTION
	{tbl}_undel(name_ text)
	RETURNS integer
	AS
	$$
	DECLARE
		current_changeset bigint;
		rowuid bigint;
	BEGIN
		current_changeset = get_current_changeset();
		rowuid = {tbl}_get_uid(name_);
		UPDATE {tbl}_history SET dest_bit = '0' WHERE version = current_changeset AND uid = rowuid;
		IF NOT FOUND THEN
			RAISE EXCEPTION 'to undel % should be the object deleted in opened changeset', name_;
		END IF;
		RETURN 1;
	END;
	$$
	LANGUAGE plpgsql;
	
'''

	# template string for del function
	del_string = '''CREATE FUNCTION
	{tbl}_del(IN name_ text)
	RETURNS integer
	AS
	$$
	DECLARE
		ver bigint;
		rowuid bigint;
		tmp bigint;
	BEGIN	
		SELECT get_current_changeset() INTO ver;
		-- try if there is already line for current version and update it
		rowuid = {tbl}_get_uid(name_);
		UPDATE {tbl}_history SET dest_bit = '1' WHERE uid = rowuid AND version = ver;
		IF NOT FOUND THEN
			--is not in current changeset, we should insert it
			INSERT INTO {tbl}_history ({columns}, version, dest_bit)
				SELECT {columns}, ver, '1' FROM {tbl}_data_version(id2num(parent(ver))) WHERE uid = rowuid;
		END IF;
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
		SELECT get_current_changeset() INTO ver;
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
	{tbl}_names(from_version bigint = 0)
	RETURNS SETOF text
	AS
	$$
	DECLARE
		parent_changeset bigint;
		current_changeset bigint;
	BEGIN
		IF from_version = 0 THEN
		-- from_version 0 means actual data for current changeset
			current_changeset = get_current_changeset_or_null();

			IF current_changeset IS NULL THEN
			--null current changeset means no opened changeset, will get data from production
			--names from poduction
				RETURN QUERY SELECT name FROM production.{tbl};
			ELSE
				--for opened changeset set from_version to its parent
				parent_changeset = parent(current_changeset);
				from_version = id2num(parent_changeset);
			END IF;
		ELSE
			current_changeset = NULL;
		END IF;

		--returns union of names in current changeset and names present in from_version (for opened changeset parent revision)
		RETURN QUERY SELECT name FROM {tbl}_history WHERE version = current_changeset AND dest_bit = '0'
		UNION
		SELECT name
		FROM {tbl}_data_version(from_version)
		WHERE uid NOT IN(
				SELECT uid FROM {tbl}_history WHERE version = current_changeset
			);
	END;
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	# template string for names
	names_embed_string = '''CREATE FUNCTION
	{tbl}_names(from_version bigint = 0)
	RETURNS SETOF text
	AS
	$$
	DECLARE
		ver bigint;
		local_name text = NULL;
		rest_of_name text;
		{reftbl}_uid bigint;
		parent_changeset bigint;
		current_changeset bigint;
		--version from which we search for local name
		version_to_search bigint;
	BEGIN
	--from_version we need unchanged for rest_of_name
		version_to_search = from_version;
		IF version_to_search = 0 THEN
		-- from_version 0 means actual data for current changeset
			current_changeset = get_current_changeset_or_null();

			IF current_changeset IS NULL THEN
			--name from production
				RETURN QUERY SELECT join_with_delim({reftbl}_get_name({column}, from_version), name, '{delim}') FROM {tbl};
			ELSE
				parent_changeset = parent(current_changeset);
				version_to_search = id2num(parent_changeset);
			END IF;
		ELSE
			current_changeset = NULL;
		END IF;

		--returns union of names in current changeset and names present in from_version (for opened changeset parent revision)
		RETURN QUERY SELECT {tbl}_get_name(uid) FROM {tbl}_history WHERE version = current_changeset AND dest_bit = '0'
		UNION
		SELECT {tbl}_get_name(uid, from_version)
		FROM {tbl}_data_version(version_to_search) 
		WHERE uid NOT IN(
				SELECT uid FROM {tbl}_history WHERE version = current_changeset
			);
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

#template for getting deleted objects between two versions
	diff_deleted_string = '''CREATE FUNCTION 
{tbl}_diff_deleted()
RETURNS SETOF text
AS
$$
BEGIN
	--deleted were between two versions objects that have set dest_bit in new data
	RETURN QUERY SELECT old_name FROM {tbl}_diff_data WHERE new_dest_bit = '1' AND old_dest_bit = '0';
END;
$$
LANGUAGE plpgsql;

'''

 #template for getting created objects between two versions
	diff_created_string = '''CREATE FUNCTION 
{tbl}_diff_created()
RETURNS SETOF text
AS
$$
BEGIN
	--created were objects which are in new data and not deleted and are not in old data
	RETURN QUERY SELECT new_name FROM {tbl}_diff_data WHERE old_name IS NULL AND new_dest_bit = '0';
END;
$$
LANGUAGE plpgsql;

'''

#template for if constructs in diff_set_attribute
	one_column_change_string = '''
	 IF (old_data.{column} <> new_data.{column}) OR ((old_data.{column} IS NULL OR new_data.{column} IS NULL) 
		  AND NOT(old_data.{column} IS NULL AND new_data.{column} IS NULL))
	 THEN
		  result.attribute = '{column}';
		  result.olddata = old_data.{column};
		  result.newdata = new_data.{column};
		  RETURN NEXT result;			
	 END IF;
	 
'''

#template for if constructs in diff_set_attribute, this version is for refuid columns
	one_column_change_ref_uid_string = '''
	 IF (old_data.{column} <> new_data.{column}) OR ((old_data.{column} IS NULL OR new_data.{column} IS NULL) 
		  AND NOT(old_data.{column} IS NULL AND new_data.{column} IS NULL))
	 THEN
		  result.attribute = '{column}';
		  result.olddata = {reftbl}_get_name(old_data.{column}, from_version);
		  result.newdata = {reftbl}_get_name(new_data.{column}, to_version);
		  RETURN NEXT result;			
	 END IF;
	 
'''


#template for getting created objects between two versions
#return type is defined in file diff.sql and created in create script
#parameters are necessary for get_name
	diff_set_attribute_string = '''CREATE FUNCTION 
	{tbl}_diff_set_attributes(from_version bigint = 0, to_version bigint = 0)
	 RETURNS SETOF diff_set_attribute_type
	 AS
	 $$
	 DECLARE
		old_data {tbl}_history%rowtype;
		new_data {tbl}_history%rowtype;
		result diff_set_attribute_type;
		current_changeset bigint;
	 BEGIN
		--sets from_version to parent revision, for diff in current changeset use
		IF from_version = 0 THEN
			--could raise exception, if you dont have opened changeset and you call this function for diff made in a current changeset 
			current_changeset = get_current_changeset();
			from_version = id2num(parent(current_changeset));
		END IF;
		
		--for each row in diff_data, which does not mean deletion (dest_bit='1'), lists modifications of each attribute
		FOR {old_new_obj_list} IN 
			SELECT {select_old_new_list}
			FROM {tbl}_diff_data
			WHERE new_name IS NOT NULL AND new_dest_bit = '0'
		LOOP				
			--if name was changed, then this modification would be mentioned with old object name
			--all other changes are mentioned with new name
			IF (old_data.name IS NOT NULL) AND (old_data.name <> new_data.name) THEN
					--first change is changed name
					result.objname = old_data.name;
					result.attribute = 'name';
					result.olddata = old_data.name;
					result.newdata = new_data.name;
					RETURN NEXT result;
			END IF;
			result.objname = new_data.name;
			--check if the column was changed
			{columns_changes}
		END LOOP;
	 END
	 $$
	 LANGUAGE plpgsql; 

'''

#template for function, which selects all data from kind table taht are present in version data_version
#is used in diff functions
	data_version_function_string = '''CREATE FUNCTION {tbl}_data_version(data_version bigint = 0)
RETURNS SETOF {tbl}_history
AS
$$
DECLARE
	changeset_id bigint;
BEGIN
	--for each object uid finds its last modification before data_version
	--joins it with history table of its kind to get object data in version data_version
	IF data_version = 0 THEN
		changeset_id = get_current_changeset_or_null();
		IF changeset_id IS NULL THEN
			SELECT MAX(num) INTO data_version FROM version;
		ELSE
			data_version = id2num(parent(changeset_id));
		END IF;
	END IF;
	
	RETURN QUERY 
	SELECT * FROM {tbl}_history
		WHERE version = changeset_id AND dest_bit = '0'
	UNION
	SELECT h1.* FROM {tbl}_history h1 
		JOIN version v1 ON (v1.id = h1.version)
		JOIN (	SELECT uid, max(num) AS maxnum 
				FROM {tbl}_history h JOIN version v ON (v.id = h.version )
				WHERE v.num <= data_version
				GROUP BY uid
			) vmax1
		ON (h1.uid = vmax1.uid AND v1.num = vmax1.maxnum)
	WHERE h1.dest_bit = '0' AND h1.uid NOT IN (
		SELECT uid FROM {tbl}_history
		WHERE version = changeset_id
	); 
END
$$
LANGUAGE plpgsql;

'''

#template for function which selects all changes of all objects, that where done between from_version and to_version versions
#is used in diff functions
	data_changes_function_string = '''CREATE FUNCTION {tbl}_changes_between_versions(from_version bigint, to_version bigint)
RETURNS SETOF {tbl}_history
AS
$$
BEGIN
	--for each object uid finds its last modification between versions from_version and to_version
	--joins it with history table of its kind to get object data in version to_version of all objects modified between from_version and to_version
	RETURN QUERY 
	SELECT h1.* 
	FROM {tbl}_history h1 
		JOIN version v1 on (v1.id = h1.version)
		JOIN (	SELECT uid, max(num) as maxnum 
				FROM {tbl}_history h join version v on (v.id = h.version )
				WHERE v.num <= to_version and v.num > from_version
				GROUP BY uid) vmax1 
		ON(h1.uid = vmax1.uid and v1.num = vmax1.maxnum);
END
$$
LANGUAGE plpgsql;

'''

#template for function that prepairs temp table for diff functions
	diff_init_function_string = '''CREATE FUNCTION {tbl}_init_diff(from_version bigint, to_version bigint)
RETURNS void
AS
$$
BEGIN
	--full outer join of data in from_version and list of changes made between from_version and to_version
	CREATE TEMP TABLE {tbl}_diff_data 
	AS SELECT {diff_columns}
		FROM {tbl}_data_version(from_version) dv FULL OUTER JOIN {tbl}_changes_between_versions(from_version,to_version) chv ON (dv.uid = chv.uid);
END
$$
LANGUAGE plpgsql;

'''

#template for function that prepairs temp table for diff functions, which selects diffs between opened changeset and its parent
	diff_changeset_init_function_string = '''CREATE OR REPLACE FUNCTION deska.{tbl}_init_diff()
RETURNS void
AS
$$
DECLARE
	changeset_var bigint;
	from_version bigint;	
BEGIN
	--it's necessary to have opened changeset in witch we would like to see diff
	changeset_var = get_current_changeset();
	from_version = id2num(parent(changeset_var));
	--full outer join of data in parent revision and changes made in opened changeset
	CREATE TEMP TABLE {tbl}_diff_data 
	AS  SELECT {diff_columns}
		FROM (SELECT * FROM {tbl}_history WHERE version = changeset_var) chv
			FULL OUTER JOIN {tbl}_data_version(from_version) dv ON (dv.uid = chv.uid);
END
$$
  LANGUAGE plpgsql;
  
'''

#template for terminating diff, drops temp table with data for diff functions
	diff_terminate_function_string = '''CREATE FUNCTION 
{tbl}_terminate_diff()
RETURNS void
AS
$$
BEGIN
	DROP TABLE {tbl}_diff_data;
END;
$$
LANGUAGE plpgsql;

'''

	resolved_data_string = '''CREATE OR REPLACE FUNCTION {tbl}_resolved_data(name_ text, from_version bigint = 0)
RETURNS {tbl}_type
AS
$$
DECLARE
	data {tbl}_type;
	current_changeset bigint;
BEGIN
	IF from_version = 0 THEN
		current_changeset = get_current_changeset_or_null();
		IF current_changeset IS NULL THEN
			--user wants current data from production
			SELECT {columns} INTO data 
			FROM production.{tbl} WHERE name = name_;
			IF NOT FOUND THEN
				RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '10021';
			END IF;
			RETURN data;
		END IF;
	END IF;

	WITH recursive resolved_data AS (
        SELECT {columns}, template as orig_template
        FROM {tbl}_data_version(from_version)
        WHERE name = name_
        UNION ALL
        SELECT
			{rd_dv_coalesce},
			dv.template, rd.orig_template
        FROM {templ_tbl}_data_version(from_version) dv, resolved_data rd 
        WHERE dv.uid = rd.template
	)
	SELECT {columns_ex_templ}, {templ_tbl}_get_name(orig_template) AS template INTO data
	FROM resolved_data WHERE template IS NULL;

	RETURN data;
END
$$
LANGUAGE plpgsql;

'''
