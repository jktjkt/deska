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
		SELECT my_version() INTO ver;
		SELECT {tbl}_get_uid(name_) INTO rowuid;
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
		SELECT my_version() INTO ver;
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
			RAISE EXCEPTION '{tbl} with name % does not exist',name_;
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
			RAISE EXCEPTION '{tbl} with name % does not exist',name_;
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
			RAISE EXCEPTION '{tbl} with name % does not exist in this version', name_;
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
			WHERE uid = {tbl}_uid;
		END IF;		
		RETURN value;
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
	BEGIN
		SELECT embed_name[1],embed_name[2] FROM embed_name(full_name,'{delim}') INTO rest_of_name,{tbl}_name;
		--finds uid of object which is thisone embed into
		SELECT {reftbl}_get_uid(rest_of_name, from_version) INTO {reftbl}_uid;
		--finds id of changeset where was object with full_name changed = object with local name + uid of object which is object embed into
		changeset_id = {tbl}_changeset_of_data_version_by_name({tbl}_name, {reftbl}_uid, from_version);
		SELECT uid INTO {tbl}_uid FROM {tbl}_history WHERE name = {tbl}_name AND {column} = {reftbl}_uid AND version = changeset_id;
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
	DECLARE
		ver bigint;
		rowuid bigint;
	BEGIN	
		SELECT my_version() INTO ver;
		SELECT {tbl}_get_uid(name_) INTO rowuid;
		-- try if there is already line for current version
		INSERT INTO {tbl}_history (uid, name, version, dest_bit)
			VALUES (rowuid, name_, ver, '1');
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
	RETURNS SETOF text
	AS
	$$
	BEGIN
		RETURN QUERY SELECT name FROM {tbl};
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
			SELECT my_version() INTO changeset_id;
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
		version_id = (changeset_id);
	
		SELECT MAX(v.num) INTO last_change_version
		FROM {tbl}_history obj_history
			JOIN version v ON (obj_history.uid = obj_uid AND obj_history.version = v.id AND v.num <= version_id);
			
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
			SELECT my_version() INTO changeset_id;
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
			SELECT my_version() INTO changeset_id;
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
