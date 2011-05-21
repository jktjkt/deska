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
	# template string for set function's
	set_string = '''CREATE FUNCTION
	{tbl}_set_{colname}(IN name_ text,IN value text)
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
		rowuid bigint;
		tmp bigint;
	BEGIN
		SELECT get_current_changeset() INTO ver;
		SELECT {tbl}_get_uid(name_) INTO rowuid;
		IF NOT FOUND THEN
			RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '10021';
		END IF;
		-- try if there is already line for current version
		SELECT uid INTO tmp FROM {tbl}_history
			WHERE uid = rowuid AND version = ver;
		--object with given name was not modified in this version
		--we need to get its current data to this version
		IF NOT FOUND THEN
			INSERT INTO {tbl}_history ({columns},version)
				SELECT {columns},ver FROM {tbl}_history
					WHERE uid = rowuid AND version = {tbl}_prev_changeset(rowuid,parent(ver));
		END IF;
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
		SELECT {reftbl}_get_uid(value) INTO refuid;
		SELECT {tbl}_get_uid(name_) INTO rowuid;
		-- try if there is already line for current version
		SELECT uid INTO tmp FROM {tbl}_history
			WHERE uid = rowuid AND version = ver;
		IF NOT FOUND THEN
			INSERT INTO {tbl}_history ({columns},version)
				SELECT {columns},ver FROM {tbl}_history
					WHERE uid = rowuid AND version = {tbl}_prev_changeset(rowuid,parent(ver));
		END IF;
		UPDATE {tbl}_history SET {colname} = refuid, version = ver
			WHERE uid = rowuid AND version = ver;
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
	{tbl}_get_data(IN name_ text, from_version bigint = 0)
	RETURNS {tbl}_type
	AS
	$$
	DECLARE	ver bigint;
		obj_uid bigint;
		data {tbl}_type;
		deleted bit(1);
	BEGIN
		obj_uid = {tbl}_get_uid(name_);
		SELECT {tbl}_changeset_of_data_version(obj_uid,from_version) INTO ver;

		SELECT dest_bit INTO deleted FROM {tbl}_history
			WHERE uid = obj_uid AND version = ver;		

		IF deleted = B'1' THEN
			RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '10021';
		END IF;

		SELECT {columns} INTO data FROM {tbl}_history
			WHERE uid = obj_uid AND version = ver;
		
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
	DECLARE	ver bigint;
		parrent_uid bigint;
		parrent_name text;
		base_name text;
		obj_uid bigint;
		data {tbl}_type;
		deleted bit(1);
	BEGIN
		obj_uid = {tbl}_get_uid(name_);
		SELECT {tbl}_changeset_of_data_version(obj_uid,from_version) INTO ver;
		SELECT embed_name[1],embed_name[2] FROM embed_name(name_,'->') INTO parrent_name,base_name;
		--TODO get name of name of parent table that is tbl embed into
		SELECT {embedtbl}_get_uid(parrent_name) INTO parrent_uid;

		SELECT dest_bit INTO deleted FROM {tbl}_history
			WHERE name = base_name AND host = parrent_uid AND version = ver;

		IF deleted = B'1' THEN
			RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '10021';
		END IF;
		
		SELECT {columns} INTO data FROM {tbl}_history
			WHERE name = base_name AND host = parrent_uid AND version = ver;

		RETURN data;
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
		SELECT get_current_changeset() INTO ver;
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
	{tbl}_get_uid(IN name_ text, from_version bigint = 0)
	RETURNS bigint
	AS
	$$
	DECLARE
		changeset_id bigint;
		value bigint;
		deleted bit(1);
	BEGIN
	--from_version is user id of version
	--we need id of changeset
		
		--changeset_id id of changeset with last change of object with name name_
		changeset_id = {tbl}_changeset_of_data_version_by_name(name_, from_version);

		SELECT uid, dest_bit INTO value, deleted FROM {tbl}_history WHERE version = changeset_id AND name = name_;
		
		IF deleted = B'1' THEN
			RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '10021';
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
		ver bigint;
		value text;
		parent_changeset bigint;
		current_changeset bigint;
	BEGIN
		IF from_version = 0 THEN
			current_changeset = get_current_changeset_or_null();

			IF current_changeset IS NULL THEN
			--name from poduction
				SELECT name INTO value FROM {tbl} WHERE uid = {tbl}_uid;
				RETURN value;
			END IF;
			
			parent_changeset = parent(current_changeset);
			from_version = id2num(parent_changeset);
		ELSE
			current_changeset = NULL;
		END IF;

		SELECT name INTO value FROM {tbl}_history WHERE  uid = {tbl}_uid AND version = current_changeset AND dest_bit = '0';
		IF NOT FOUND THEN
			SELECT name INTO value
			FROM {tbl}_history h JOIN version v ON (h.uid = {tbl}_uid AND h.version = v.id AND h.version <= from_version) 
			WHERE v.id = (
					SELECT max(version) 
					FROM {tbl}_history h2 
						JOIN version v2 ON (h2.version <= from_version AND h2.version = v2.id AND h2.uid = h.uid)
				)
				AND dest_bit = '0';
		END IF;
		
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
			current_changeset = get_current_changeset_or_null();

			IF current_changeset IS NULL THEN
			--name from poduction
				SELECT name, {column} INTO local_name,{reftbl}_uid FROM {tbl} WHERE uid = {tbl}_uid;
			ELSE
				parent_changeset = parent(current_changeset);
				version_to_search = id2num(parent_changeset);
			END IF;
		ELSE
			current_changeset = NULL;
		END IF;

		IF local_name IS NULL THEN
			--get local name
			SELECT name, {column} INTO local_name, {reftbl}_uid FROM {tbl}_history WHERE  uid = {tbl}_uid AND version = current_changeset AND dest_bit = '0';
			IF NOT FOUND THEN
				SELECT name, {column} INTO local_name, {reftbl}_uid
				FROM {tbl}_history h JOIN version v ON (h.uid = {tbl}_uid AND h.version = v.id AND h.version <= version_to_search) 
				WHERE v.id = (
						SELECT max(version) 
						FROM {tbl}_history h2 
							JOIN version v2 ON (h2.version <= version_to_search AND h2.version = v2.id AND h2.uid = h.uid)
					)
					AND dest_bit = '0';
			END IF;
		END IF;
		
		raise notice 'local_name is %',local_name;
		
		
		rest_of_name = {reftbl}_get_name({reftbl}_uid, from_version);
		raise notice 'rest_of_name is %', rest_of_name;
		
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
		--finds uid of object which is thisone embed into
		SELECT {reftbl}_get_uid(rest_of_name, from_version) INTO {reftbl}_uid;

		--finds id of changeset where was object with full_name changed = object with local name + uid of object which is object embed into
		changeset_id = {tbl}_changeset_of_data_version_by_name({tbl}_name, {reftbl}_uid, from_version);
		
		SELECT uid, dest_bit INTO {tbl}_uid, deleted FROM {tbl}_history WHERE name = {tbl}_name AND {column} = {reftbl}_uid AND version = changeset_id;
		IF deleted = B'1' THEN
			RAISE 'No {tbl} with name % exist in this version', full_name USING ERRCODE = '10022';
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
		SELECT get_current_changeset() INTO ver;
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
	BEGIN
		SELECT embed_name[1],embed_name[2] FROM embed_name(full_name,'{delim}') INTO rest_of_name,{tbl}_name;
		SELECT {reftbl}_get_uid(rest_of_name) INTO {reftbl}_uid;
		SELECT get_current_changeset() INTO ver;
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
	DECLARE
		ver bigint;
		rowuid bigint;
		local_name text;
	BEGIN	
		SELECT get_current_changeset() INTO ver;
		SELECT {tbl}_get_uid(name_) INTO rowuid;
		-- try if there is already line for current version
		INSERT INTO {tbl}_history (uid, name, version, dest_bit)
			VALUES (rowuid, name_, ver, '1');
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	# template string for del function
	del_embed_string = '''CREATE FUNCTION
	{tbl}_del(IN full_name text)
	RETURNS integer
	AS
	$$
	DECLARE
		ver bigint;
		rowuid bigint;
		{reftbl}_uid bigint;
		{tbl}_name text;
		rest_of_name text;
	BEGIN	
		SELECT get_current_changeset() INTO ver;
		SELECT {tbl}_get_uid(full_name) INTO rowuid;
		SELECT embed_name[1],embed_name[2] FROM embed_name(full_name,'{delim}') INTO rest_of_name,{tbl}_name;
		-- try if there is already line for current version
		{reftbl}_uid = {reftbl}_get_uid(rest_of_name);
		INSERT INTO {tbl}_history (uid, name, {column}, version, dest_bit)
			VALUES (rowuid, {tbl}_name, {reftbl}_uid, ver, '1');
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
			current_changeset = get_current_changeset_or_null();

			IF current_changeset IS NULL THEN
			--names from poduction
				RETURN QUERY SELECT name FROM production.{tbl};
			ELSE
				parent_changeset = parent(current_changeset);
				from_version = id2num(parent_changeset);					
			END IF;
		ELSE
			current_changeset = NULL;
		END IF;

		RETURN QUERY SELECT name FROM {tbl}_history WHERE version = current_changeset AND dest_bit = '0'
		UNION
		SELECT name
		FROM {tbl}_history h JOIN version v ON (h.version = v.id and v.num <= from_version) 
		WHERE v.id = (
				SELECT max(version) 
				FROM {tbl}_history h2 
					JOIN version v2 ON (v2.num <= from_version AND h2.version = v2.id AND h2.uid = h.uid)
			)
			AND dest_bit = '0' AND h.uid NOT IN(
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
			current_changeset = get_current_changeset_or_null();

			IF current_changeset IS NULL THEN
			--name from poduction
				RETURN QUERY SELECT join_with_delim({reftbl}_get_name({column}, from_version), name, '{delim}') FROM {tbl};
			ELSE
				parent_changeset = parent(current_changeset);
				version_to_search = id2num(parent_changeset);
			END IF;
		ELSE
			current_changeset = NULL;
		END IF;

		RETURN QUERY SELECT join_with_delim({reftbl}_get_name({column}, from_version), name, '{delim}')  FROM {tbl}_history WHERE version = current_changeset AND dest_bit = '0'
		UNION
		SELECT join_with_delim({reftbl}_get_name({column}, from_version), name, '{delim}')
		FROM {tbl}_history h JOIN version v ON (h.version = v.id and v.num <= version_to_search) 
		WHERE v.id = (
				SELECT max(version) 
				FROM {tbl}_history h2 
					JOIN version v2 ON (v2.num <= version_to_search AND h2.version = v2.id AND h2.uid = h.uid)
			)
			AND dest_bit = '0' AND h.uid NOT IN(
				SELECT uid FROM {tbl}_history WHERE version = current_changeset
			);
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

#template for getting number of version, when was object modified before given version was commited
	changeset_of_data_version_string = '''CREATE FUNCTION 
	{tbl}_changeset_of_data_version(obj_uid bigint, from_version bigint = 0)
	RETURNS bigint
	AS
	$$
	DECLARE
		version_id bigint;
		changeset_id bigint;
		preceding_changeset bigint;
		tmp bigint;
	BEGIN
	--from_version is user id of version
	--we need id of changeset
		IF from_version = 0 THEN
			SELECT get_current_changeset_or_null() INTO changeset_id;
			--no opened changeset
			IF changeset_id IS NULL THEN
				--user wants newest data
				SELECT MAX(num) INTO version_id FROM version;
				SELECT ID INTO changeset_id FROM version WHERE num = version_id;
			ELSE
				--user wants data in his version, or earlier
				SELECT uid INTO tmp FROM {tbl}_history WHERE uid = obj_uid AND version = changeset_id;
				IF NOT FOUND THEN
					SELECT parentrevision INTO changeset_id FROM changeset WHERE id = changeset_id;
				ELSE
					RETURN changeset_id;
				END IF;		
			END IF;
		ELSE
			--user wants to see data valid in some commited version
			changeset_id  = num2id(from_version);
		END IF;

		SELECT {tbl}_prev_changeset(obj_uid,changeset_id) INTO preceding_changeset;
		
		RETURN preceding_changeset;
	END;
	$$
	LANGUAGE plpgsql;
	
'''

#template string for getting last id of changeset preceding changeset_id where object with uid was changed
	prev_changest_string='''CREATE FUNCTION 
	{tbl}_prev_changeset(obj_uid bigint, changeset_id bigint)
	RETURNS bigint
	AS
	$$
	DECLARE
		version_id bigint;
		--is result of function, last changeset where object with uid is changed
		last_changeset_id bigint;
		--last version where was the object with uid changed
		last_change_version bigint;
	BEGIN
		version_id = id2num(changeset_id);
	
		SELECT MAX(v.num) INTO last_change_version
			FROM {tbl}_history obj_history
				JOIN version v ON (obj_history.uid = obj_uid AND obj_history.version = v.id AND v.num <= version_id);
		IF last_change_version IS NULL THEN
			RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '10021';
		END IF;
			
		last_changeset_id  = num2id(last_change_version);

		RETURN last_changeset_id;
	END;
	$$
	LANGUAGE plpgsql;
	
'''

#template string for getting last id of changeset preceding changeset_id where object with name_ was changed
	prev_changest_by_name_string='''CREATE FUNCTION 
	{tbl}_prev_changeset_by_name(name_ text, changeset_id bigint)
	RETURNS bigint
	AS
	$$
	DECLARE
		version_id bigint;
		--is result of function, last changeset where object with uid is changed
		last_changeset_id bigint;
		--last version where was the object with uid changed
		last_change_version bigint;
	BEGIN
		version_id = id2num(changeset_id);
		SELECT MAX(v.num) INTO last_change_version
			FROM {tbl}_history obj_history
				JOIN version v ON (obj_history.name = name_ AND obj_history.version = v.id AND v.num <= version_id);
		IF last_change_version IS NULL THEN
			RAISE 'No {tbl} named %. Create it first.',name_ USING ERRCODE = '10021';
		END IF;
		last_changeset_id  = num2id(last_change_version);
		
		RETURN last_changeset_id;
	END;
	$$
	LANGUAGE plpgsql;
	
'''

	changeset_of_data_version_by_name_string = '''CREATE FUNCTION 
	{tbl}_changeset_of_data_version_by_name(name_ text, from_version bigint = 0)
	RETURNS bigint
	AS
	$$
	DECLARE
		version_id bigint;
		changeset_id bigint;
		preceding_changeset bigint;
		tmp bigint;
	BEGIN
	--from_version is user id of version
	--we need id of changeset
		IF from_version = 0 THEN
			SELECT get_current_changeset_or_null() INTO changeset_id;
			--no opened changeset
			IF changeset_id IS NULL THEN
				--user wants newest data
				SELECT MAX(num) INTO version_id FROM version;
				changeset_id = num2id(version_id);
			ELSE
				--user wants data in his version, or earlier
				SELECT uid INTO tmp FROM {tbl}_history WHERE name = name_ AND version = changeset_id;
				IF NOT FOUND THEN
					--object was not modified in changeset which user just works on
					SELECT parentrevision INTO changeset_id FROM changeset WHERE id = changeset_id;
				ELSE
					RETURN changeset_id;
				END IF;		
			END IF;
		ELSE
			--user wants to see data valid in some commited version
			changeset_id  = num2id(from_version);
		END IF;

		SELECT {tbl}_prev_changeset_by_name(name_,changeset_id) INTO preceding_changeset;
		
		RETURN preceding_changeset;
	END;
	$$
	LANGUAGE plpgsql;
	
'''
#template string for getting last id of changeset preceding changeset_id where object with name_ was changed
	prev_changest_by_name_embed_string='''CREATE FUNCTION 
	{tbl}_prev_changeset_by_name(name_ text, refuid bigint, changeset_id bigint)
	RETURNS bigint
	AS
	$$
	DECLARE
		version_id bigint;
		--is result of function, last changeset where object with uid is changed
		last_changeset_id bigint;
		--last version where was the object with uid changed
		last_change_version bigint;
	BEGIN
		version_id = id2num(changeset_id);
		
		SELECT MAX(v.num) INTO last_change_version
		FROM {tbl}_history obj_history
			JOIN version v ON (obj_history.name = name_ AND obj_history.version = v.id AND v.num <= version_id AND {column} = refuid);
		
		last_changeset_id  = num2id(last_change_version);
		
		RETURN last_changeset_id;
	END;
	$$
	LANGUAGE plpgsql;
	
'''

	changeset_of_data_version_by_name_embed_string = '''CREATE FUNCTION 
	{tbl}_changeset_of_data_version_by_name(name_ text, refuid bigint, from_version bigint = 0)
	RETURNS bigint
	AS
	$$
	DECLARE
		version_id bigint;
		changeset_id bigint;
		preceding_changeset bigint;
		tmp bigint;
	BEGIN
	--from_version is user id of version
	--we need id of changeset
		IF from_version = 0 THEN
			SELECT get_current_changeset_or_null() INTO changeset_id;
			--no opened changeset
			IF changeset_id IS NULL THEN
				--user wants newest data
				SELECT MAX(num) INTO version_id FROM version;
				changeset_id = num2id(version_id);
			ELSE
				--user wants data in his version, or earlier
				SELECT uid INTO tmp FROM {tbl}_history WHERE name = name_ AND {column} = refuid AND version = changeset_id;
				IF NOT FOUND THEN
					--object was not modified in changeset which user just works on
					SELECT parentrevision INTO changeset_id FROM changeset WHERE id = changeset_id;
				ELSE
					RETURN changeset_id;
				END IF;		
			END IF;
		ELSE
			--user wants to see data valid in some commited version
			changeset_id  = num2id(from_version);
		END IF;

		SELECT {tbl}_prev_changeset_by_name(name_, refuid, changeset_id) INTO preceding_changeset;
		
		RETURN preceding_changeset;
	END;
	$$
	LANGUAGE plpgsql;
	
'''
 #template for getting deleted objects between two versions
	diff_deleted_string = '''CREATE FUNCTION 
{tbl}_diff_deleted()
RETURNS SETOF text
AS
$$
BEGIN
	RETURN QUERY SELECT old_name FROM {tbl}_diff_data WHERE new_dest_bit = '1';
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
	diff_set_attribute_string = '''CREATE FUNCTION 
	{tbl}_diff_set_attributes(from_version bigint, to_version bigint)
	 RETURNS SETOF diff_set_attribute_type
	 AS
	 $$
	 DECLARE
		old_data {tbl}_history%rowtype;
		new_data {tbl}_history%rowtype;
		result diff_set_attribute_type;
	 BEGIN
		result.command = 'setAttribute';
		result.objkind = '{tbl}';
		FOR {old_new_obj_list} IN 
			SELECT {select_old_new_list}
			FROM {tbl}_diff_data
			WHERE new_name IS NOT NULL AND new_dest_bit = '0'
		LOOP				
			IF (old_data.name IS NOT NULL) AND (old_data.name <> new_data.name) THEN
					--first change is changed name
					result.objname = old_data.name;
					result.attribute = 'name';
					result.olddata = old_data.name;
					result.newdata = new_data.name;
					RETURN NEXT result;
			END IF;
			result.objname = new_data.name;
			{columns_changes}
		END LOOP;
	 END
	 $$
	 LANGUAGE plpgsql; 

'''

#template for function, which selects all data from kind table taht are present in version data_version
#is used in diff functions
	data_version_function_string = '''CREATE FUNCTION {tbl}_data_version(data_version bigint)
RETURNS SETOF {tbl}_history
AS
$$
BEGIN
	RETURN QUERY 
	SELECT h1.* 
	FROM {tbl}_history h1 
		JOIN version v1 ON (v1.id = h1.version)
		JOIN (	SELECT uid, max(num) AS maxnum 
				FROM {tbl}_history h JOIN version v ON (v.id = h.version )
				WHERE v.num <= data_version
				GROUP BY uid
			) vmax1 
		ON (h1.uid = vmax1.uid AND v1.num = vmax1.maxnum)
	WHERE dest_bit = '0';
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
	--needs better design
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
	changeset_var = get_current_changeset();
	from_version = id2num(parent(changeset_var));
	CREATE TEMP TABLE {tbl}_diff_data 
	AS  SELECT {diff_columns}
		FROM (SELECT * FROM {tbl}_history WHERE version = changeset_var) chv
			FULL OUTER JOIN {tbl}_data_version(from_version) dv ON (dv.uid = chv.uid);
END
$$
  LANGUAGE plpgsql;
  
'''

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