#!/usr/bin/python2

#delimiter string
DELIMITER = "->"

class Templates:
	# template string for generate historic table
	hist_string = '''CREATE TABLE history.%(tbl)s_history (
	LIKE %(tbl)s
	-- include default values
	INCLUDING DEFAULTS
	-- include CHECK constrants !!! only check constraints in postgresql 9
	INCLUDING CONSTRAINTS
	-- INCLUDE INDEXES???
	,
	version int NOT NULL,
	dest_bit bit(1) NOT NULL DEFAULT B'0'
	%(constraints)s
);
'''

	# template string for set functions
	set_string = '''CREATE FUNCTION
	%(tbl)s_set_%(colname)s(IN name_ text,IN value text)
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
		rowuid bigint;
		tmp bigint;
	BEGIN
		--for modifications we need to have opened changeset, this function raises exception in case we don't have
		SELECT get_current_changeset() INTO ver;
		SELECT %(tbl)s_get_uid(name_) INTO rowuid;
		--not found in case there is no object with name name_ in history
		IF NOT FOUND THEN
			RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
		END IF;
		-- try if there is already line for current version
		SELECT uid INTO tmp FROM %(tbl)s_history
			WHERE uid = rowuid AND version = ver;
		--object with given name was not modified in this version
		--we need to get its current data to this version
		IF NOT FOUND THEN
			INSERT INTO %(tbl)s_history (%(columns)s,version)
				SELECT %(columns)s,ver FROM %(tbl)s_data_version(id2num(parent(ver))) WHERE uid = rowuid;
		END IF;
		--set new value in %(colname)s column
		UPDATE %(tbl)s_history SET %(colname)s = CAST (value AS %(coltype)s), version = ver
			WHERE uid = rowuid AND version = ver;
		
		--flag is_generated set to false
		UPDATE changeset SET is_generated = FALSE WHERE id = ver;
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	#template for setting uid of referenced row that has name attribute value
	#value could be composed of names that are in embed into chain
	set_fk_uid_string = '''CREATE FUNCTION
	%(tbl)s_set_%(colname)s(IN name_ text,IN value text)
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
			SELECT %(reftbl)s_get_uid(value) INTO refuid;
		END IF;

		SELECT %(tbl)s_get_uid(name_) INTO rowuid;
		-- try if there is already line for current version
		SELECT uid INTO tmp FROM %(tbl)s_history
			WHERE uid = rowuid AND version = ver;
		--object with given name was not modified in this version
		--we need to get its current data to this version
		IF NOT FOUND THEN
			INSERT INTO %(tbl)s_history (%(columns)s,version)
				SELECT %(columns)s,ver FROM %(tbl)s_data_version(id2num(parent(ver))) WHERE uid = rowuid;
		END IF;
		--set column to refuid - uid of referenced object
		UPDATE %(tbl)s_history SET %(colname)s = refuid, version = ver
			WHERE uid = rowuid AND version = ver;
		
		--flag is_generated set to false
		UPDATE changeset SET is_generated = FALSE WHERE id = ver;
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	# template string for set functions for columns that reference set of identifiers
	set_refuid_set_string = '''CREATE FUNCTION
	%(tbl)s_set_%(colname)s(IN name_ text,IN value text[])
	RETURNS integer
	AS
	$$
	DECLARE
		ver bigint;
	BEGIN
		ver = get_current_changeset();
	
		--flag is_generated set to false
		UPDATE changeset SET is_generated = FALSE WHERE id = ver;
		
		BEGIN
			--row is inserted because of diff and changes between versions
			--this means object was modified
			INSERT INTO %(tbl)s_history (%(columns)s,version)
				SELECT %(columns)s,ver FROM %(tbl)s_data_version(id2num(parent(ver))) WHERE name = name_;
		EXCEPTION 
			WHEN unique_violation THEN
			-- do nothing
		END;
		RETURN genproc.inner_%(tbl)s_%(colname)s_multiref_set_%(colname)s(name_, value);
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	#template for generating function to add one item to set of identifiers
	refuid_set_insert_string = '''CREATE FUNCTION
	%(tbl)s_set_%(colname)s_insert(IN name_ identifier,IN value identifier)
	RETURNS integer
	AS
	$$
	DECLARE
		ver bigint;
	BEGIN
		ver = get_current_changeset();
	
		--flag is_generated set to false
		UPDATE changeset SET is_generated = FALSE WHERE id = ver;
		BEGIN
			--row is inserted because of diff and changes between versions
			--this means object was modified
			INSERT INTO %(tbl)s_history (%(columns)s,version)
				SELECT %(columns)s,ver FROM %(tbl)s_data_version(id2num(parent(ver))) WHERE name = name_;
		EXCEPTION WHEN unique_violation THEN
			-- do nothing
		END;
		RETURN genproc.inner_%(tbl)s_set_%(colname)s_insert(name_, value);
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	#template for generating function to remove one item from set of identifiers
	refuid_set_remove_string = '''CREATE FUNCTION
	%(tbl)s_set_%(colname)s_remove(IN name_ identifier,IN value identifier)
	RETURNS integer
	AS
	$$
	DECLARE
		ver bigint;
	BEGIN
		ver = get_current_changeset();
    
		--flag is_generated set to false
		UPDATE changeset SET is_generated = FALSE WHERE id = ver;
		BEGIN
			--row is inserted because of diff and changes between versions
			--this means object was modified
			INSERT INTO %(tbl)s_history (%(columns)s,version)
				SELECT %(columns)s,ver FROM %(tbl)s_data_version(id2num(parent(ver))) WHERE name = name_;
		EXCEPTION WHEN unique_violation THEN
			-- do nothing
		END;
		RETURN genproc.inner_%(tbl)s_set_%(colname)s_remove(name_, value);
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	# template string for set functions
	set_name_string = '''CREATE FUNCTION
	%(tbl)s_set_name(IN name_ text,IN new_name text)
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
		rowuid bigint;
		tmp bigint;
	BEGIN
		--for modifications we need to have opened changeset, this function raises exception in case we don't have
		SELECT get_current_changeset() INTO ver;
		SELECT %(tbl)s_get_uid(name_) INTO rowuid;
		--not found in case there is no object with name name_ in history
		IF NOT FOUND THEN
			RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
		END IF;
		-- try if there is already line for current version
		SELECT uid INTO tmp FROM %(tbl)s_history
			WHERE uid = rowuid AND version = ver;
		--object with given name was not modified in this version
		--we need to get its current data to this version
		IF NOT FOUND THEN
			INSERT INTO %(tbl)s_history (%(columns)s,version)
				SELECT %(columns)s,ver FROM %(tbl)s_data_version(id2num(parent(ver))) WHERE uid = rowuid;
		END IF;
		
		BEGIN
			UPDATE %(tbl)s_history SET name = new_name, version = ver
				WHERE uid = rowuid AND version = ver;
		EXCEPTION
			WHEN unique_violation THEN
				RAISE EXCEPTION 'object with name %% was deleted, ...', new_name USING ERRCODE = '70010';
		END;
		
		--flag is_generated set to false
		UPDATE changeset SET is_generated = FALSE WHERE id = ver;
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	set_name_embed_string = '''CREATE FUNCTION
	%(tbl)s_set_name(IN name_ text,IN new_name text)
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
		SELECT %(tbl)s_get_uid(name_) INTO rowuid;
		-- try if there is already line for current version
		SELECT uid INTO tmp FROM %(tbl)s_history
			WHERE uid = rowuid AND version = ver;
		--object with given name was not modified in this version
		--we need to get its current data to this version
		IF NOT FOUND THEN
			INSERT INTO %(tbl)s_history (%(columns)s,version)
				SELECT %(columns)s,ver FROM %(tbl)s_data_version(id2num(parent(ver))) WHERE uid = rowuid;
		END IF;
		--set column to refuid - uid of referenced object
		SELECT embed_name[1], embed_name[2] FROM embed_name(new_name,'%(delim)s') INTO refname, local_name;
		refuid = %(reftbl)s_get_uid(refname);
		BEGIN
			UPDATE %(tbl)s_history SET %(refcolumn)s = refuid, name = local_name, version = ver
				WHERE uid = rowuid AND version = ver;
		EXCEPTION
			WHEN unique_violation THEN
				RAISE EXCEPTION 'object with name %% was deleted, ...', new_name USING ERRCODE = '70010';
		END;
		--flag is_generated set to false
		UPDATE changeset SET is_generated = FALSE WHERE id = ver;
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	#template for data_type, given with columns - list of attributes' names and their types
	get_data_type_string='''CREATE TYPE %(tbl)s_type AS(
%(columns)s
);
'''
	# template string for get data functions
	# get data of object name_ in given version
	get_data_string = '''CREATE FUNCTION
	%(tbl)s_get_data(IN name_ text, from_version bigint = 0)
	RETURNS %(tbl)s_type
	AS
	$$
	DECLARE
		current_changeset bigint;
		data %(tbl)s_type;
		dbit bit(1);
	BEGIN
		IF from_version = 0 THEN
			current_changeset = get_current_changeset_or_null();
			IF current_changeset IS NULL THEN
				--user wants last data
				SELECT MAX(num) INTO  from_version FROM version;
			ELSE
				SELECT %(columns)s,dest_bit INTO %(data_columns)s, dbit FROM %(tbl)s_history WHERE name = name_ AND version = current_changeset;
				IF FOUND THEN
					--we have result and can return it
					IF dbit = '1' THEN
						RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
					END IF;
					RETURN data;
				END IF;
				--object name_ is not present in current changeset, we need look for it in parent revision or erlier
				from_version = id2num(parent(current_changeset));
			END IF;
		END IF;

		SELECT %(columns)s INTO %(data_columns)s FROM %(tbl)s_data_version(from_version)
			WHERE name = name_;
		IF NOT FOUND THEN
			RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
		END IF;

		RETURN data;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	# template string for get data functions with embed flag
	get_embed_data_string = '''CREATE FUNCTION
	%(tbl)s_get_data(IN name_ text, from_version bigint = 0)
	RETURNS %(tbl)s_type
	AS
	$$
	DECLARE
		current_changeset bigint;
		obj_uid bigint;
		data %(tbl)s_type;
		dbit bit(1);
	BEGIN
		obj_uid = %(tbl)s_get_uid(name_, from_version);
		IF obj_uid IS NULL THEN
			RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
		END IF;

		IF from_version = 0 THEN
			current_changeset = get_current_changeset_or_null();
			IF current_changeset IS NULL THEN
				--user wants current data from production
				--name in table is only local part of object, we should look for object by uid
				SELECT MAX(num) INTO from_version FROM version;
			ELSE
				--first we look for result in current changeset than in parent revision
				SELECT %(columns)s, dest_bit INTO %(data_columns)s, dbit FROM %(tbl)s_history WHERE uid = obj_uid AND version = current_changeset;
				IF FOUND THEN
					--we have result and can return it
					IF dbit = '1' THEN
						RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
					END IF;
					RETURN data;
				END IF;
				--object name_ is not present in current changeset, we need look for it in parent revision or erlier
				from_version = id2num(parent(current_changeset));
			END IF;
		END IF;

		SELECT %(columns)s INTO %(data_columns)s FROM %(tbl)s_data_version(from_version)
			WHERE uid = obj_uid;
		IF NOT FOUND THEN
			RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
		END IF;

		RETURN data;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	# template string for get functions for columns that reference set of identifiers
	get_refuid_set_string = '''CREATE FUNCTION
	%(tbl)s_get_%(colname)s(IN obj_uid bigint, from_version bigint = 0)
	RETURNS text[]
	AS
	$$
	BEGIN
		RETURN genproc.inner_%(tbl)s_%(refcol)s_multiref_get_set(obj_uid, from_version);
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	# template string for get resolved set functions for columns that reference set of identifiers
	get_resolved_refuid_set_string = '''CREATE FUNCTION
	%(tbl)s_get_resolved_%(colname)s(IN obj_uid bigint, from_version bigint = 0)
	RETURNS text[]
	AS
	$$
	DECLARE
		result text[];
	BEGIN
		result =  ARRAY(SELECT CAST(dv.name AS text) FROM genproc.inner_%(tbl)s_%(refcol)s_multiref_get_object_resolved_set(obj_uid, from_version) uids LEFT OUTER JOIN %(refcol)s_data_version(from_version) dv ON (uids = dv.uid));
		RETURN deska.ret_id_set(result);
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	#template string for get_uid functions (for name finds corresponding uid)
	#not for kind that are embed into another
	get_uid_string = '''CREATE FUNCTION
	%(tbl)s_get_uid(IN name_ text, from_version bigint = 0)
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
				SELECT uid INTO value FROM production.%(tbl)s WHERE name = name_;
				IF NOT FOUND THEN
					RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
				END IF;
				RETURN value;
			END IF;

			SELECT uid INTO value FROM %(tbl)s_history WHERE name = name_ AND version = changeset_id;
			IF FOUND THEN
				--we have result and can return it
				RETURN value;
			END IF;
			--object name_ is not present in current changeset, we need look for it in parent revision or erlier
			from_version = id2num(parent(changeset_id));
		END IF;

		SELECT uid INTO value FROM %(tbl)s_data_version(from_version) WHERE name = name_;
		IF NOT FOUND THEN
			RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
		END IF;
		RETURN value;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	#template string for get functions
	get_name_string = '''CREATE FUNCTION
	%(tbl)s_get_name(IN %(tbl)s_uid bigint, from_version bigint = 0)
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
				SELECT name INTO value FROM production.%(tbl)s WHERE uid = %(tbl)s_uid;
				RETURN value;
			END IF;

			SELECT name INTO value FROM %(tbl)s_history WHERE uid = %(tbl)s_uid AND version = current_changeset;
			IF FOUND THEN
				--we have result and can return it
				RETURN value;
			END IF;
			from_version = id2num(parent(current_changeset));
		END IF;

		SELECT name INTO value FROM %(tbl)s_data_version(from_version) WHERE uid = %(tbl)s_uid;
		RETURN value;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	#template string for get functions
	get_name_embed_string = '''CREATE FUNCTION
	%(tbl)s_get_name(IN %(tbl)s_uid bigint, from_version bigint = 0)
	RETURNS text
	AS
	$$
	DECLARE
		current_changeset bigint;
		local_name text = NULL;
		rest_of_name text;
		%(reftbl)s_uid bigint;
		--version from which we search for local name
		version_to_search bigint;
		value text;
	BEGIN
	--from_version we need unchanged for rest_of_name
		version_to_search = from_version;
		IF version_to_search = 0 THEN
			current_changeset = get_current_changeset_or_null();

			IF current_changeset IS NULL THEN
				SELECT join_with_delim(%(reftbl)s_get_name(%(column)s, from_version), name, '%(delim)s') INTO value FROM production.%(tbl)s
					WHERE uid = %(tbl)s_uid;
				IF NOT FOUND THEN
                    RETURN NULL;
				END IF;
				RETURN value;
			END IF;

			SELECT join_with_delim(%(reftbl)s_get_name(%(column)s, from_version), name, '%(delim)s') INTO value FROM %(tbl)s_history WHERE uid = %(tbl)s_uid AND version = current_changeset;
			IF FOUND THEN
				--we have result and can return it
				RETURN value;
			END IF;
			version_to_search = id2num(parent(current_changeset));
		END IF;

		SELECT name, %(column)s INTO local_name, %(reftbl)s_uid FROM %(tbl)s_data_version(version_to_search)
			WHERE uid = %(tbl)s_uid;

		--get name of referenced object
		rest_of_name = %(reftbl)s_get_name(%(reftbl)s_uid, from_version);
		--join local name of object and name of referenced object to full name of object
		RETURN join_with_delim(rest_of_name, local_name, '%(delim)s');
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	#template for function getting uid of object embed into another
	get_uid_embed_string = '''CREATE OR REPLACE FUNCTION %(tbl)s_get_uid(full_name text, from_version bigint = 0)
	RETURNS bigint
	AS
	$$
	DECLARE
		%(reftbl)s_uid bigint;
		rest_of_name text;
		%(tbl)s_name text;
		%(tbl)s_uid bigint;
		changeset_id bigint;
		deleted bit(1);
	BEGIN
		SELECT embed_name[1],embed_name[2] FROM embed_name(full_name,'%(delim)s') INTO rest_of_name,%(tbl)s_name;
		--finds uid of object which is this one embed into
		SELECT %(reftbl)s_get_uid(rest_of_name, from_version) INTO %(reftbl)s_uid;

		--finds id of changeset where was object with full_name changed = object with local name + uid of object which is object embed into
		--look for object in current_changeset
		IF from_version = 0 THEN
			changeset_id = get_current_changeset_or_null();
			IF changeset_id IS NULL THEN
				SELECT MAX(num) INTO from_version FROM version;
			ELSE
				SELECT uid INTO %(tbl)s_uid FROM %(tbl)s_history WHERE %(column)s = %(reftbl)s_uid AND name = %(tbl)s_name;
				IF NOT FOUND THEN
					--object with this name is not in current changeset, we should look for it in parent version or earlier
					from_version = id2num(parent(changeset_id));
				ELSE
					--we have result and can return it
					RETURN %(tbl)s_uid;
				END IF;
			END IF;
		END IF;

		SELECT uid INTO %(tbl)s_uid FROM %(tbl)s_data_version(from_version) WHERE name = %(tbl)s_name AND %(column)s = %(reftbl)s_uid;
		IF NOT FOUND THEN
			RAISE 'No %(tbl)s with name %% exist in this version', full_name USING ERRCODE = '70021';
		END IF;

		RETURN %(tbl)s_uid;
	END;
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	# template string for add function
	add_string = '''CREATE FUNCTION
	%(tbl)s_add(IN name_ text)
	RETURNS deska.identifier
	AS
	$$
	DECLARE	ver bigint;
		tmp bigint;
	BEGIN
		SELECT get_current_changeset() INTO ver;
		BEGIN
			SELECT uid INTO tmp FROM %(tbl)s_history WHERE version = ver AND name = name_ AND dest_bit = '1';
			IF FOUND THEN
				RAISE 'Object with name %% was deleted, ...', name_ USING ERRCODE = '70010';
			END IF;
			INSERT INTO %(tbl)s_history (name,version)
				VALUES (name_,ver);
		EXCEPTION
			WHEN check_violation THEN
				RAISE 'Object %% violates check constraint.', name_ USING ERRCODE = '70004';
		END;
		
		--flag is_generated set to false
		UPDATE changeset SET is_generated = FALSE WHERE id = ver;
		RETURN name_;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''
	# template string for add function
	add_embed_string = '''CREATE FUNCTION
	%(tbl)s_add(IN full_name text)
	RETURNS text
	AS
	$$
	DECLARE	ver bigint;
		%(reftbl)s_uid bigint;
		rest_of_name text;
		%(tbl)s_name text;
		tmp bigint;
		name_count bigint;
		new_name_num bigint;
	BEGIN
		SELECT embed_name[1],embed_name[2] FROM embed_name(full_name,'%(delim)s') INTO rest_of_name,%(tbl)s_name;
		SELECT %(reftbl)s_get_uid(rest_of_name) INTO %(reftbl)s_uid;
		
		--if name is not given then it would be generated
		--this works only for embed objects
		IF %(tbl)s_name = '' THEN
			SELECT COUNT(*) + 1 INTO name_count FROM %(tbl)s_data_version() WHERE %(column)s = %(reftbl)s_uid;
			SELECT MIN(generated.num) INTO new_name_num
			FROM
				(SELECT name, num FROM deska.num_decorated_name('%(tbl)s',name_count)) generated
				LEFT OUTER JOIN
				(SELECT name FROM %(tbl)s_data_version() WHERE %(column)s = %(reftbl)s_uid) tab_name
				ON (generated.name = tab_name.name)
			WHERE tab_name.name IS NULL;
			%(tbl)s_name = '%(tbl)s_' || new_name_num;
			full_name = full_name || '%(tbl)s_' || new_name_num;
		END IF;
		
		SELECT get_current_changeset() INTO ver;
		SELECT uid INTO tmp FROM %(tbl)s_history WHERE version = ver AND %(column)s = %(reftbl)s_uid AND name = %(tbl)s_name AND dest_bit = '1';
		IF FOUND THEN
			RAISE EXCEPTION 'object with name %% was deleted, ...', full_name USING ERRCODE = '70010';
		END IF;
		INSERT INTO %(tbl)s_history(name, %(column)s, version) VALUES (%(tbl)s_name, %(reftbl)s_uid, ver);
		
		--flag is_generated set to false
		UPDATE changeset SET is_generated = FALSE WHERE id = ver;
		RETURN full_name;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	undel_string = '''CREATE FUNCTION
	%(tbl)s_undel(name_ text)
	RETURNS integer
	AS
	$$
	DECLARE
		current_changeset bigint;
		rowuid bigint;
	BEGIN
		current_changeset = get_current_changeset();
		rowuid = %(tbl)s_get_uid(name_);
		UPDATE %(tbl)s_history SET dest_bit = '0' WHERE version = current_changeset AND uid = rowuid;
		IF NOT FOUND THEN
			RAISE EXCEPTION 'to undel %% should be the object deleted in opened changeset', name_;
		END IF;

		--flag is_generated set to false
		UPDATE changeset SET is_generated = FALSE WHERE id = current_changeset;
		RETURN 1;
	END;
	$$
	LANGUAGE plpgsql;

'''

	# template string for del function
	del_string = '''CREATE FUNCTION
	%(tbl)s_del(IN name_ text)
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
		rowuid = %(tbl)s_get_uid(name_);
		UPDATE %(tbl)s_history SET dest_bit = '1' WHERE uid = rowuid AND version = ver;
		IF NOT FOUND THEN
			--is not in current changeset, we should insert it
			INSERT INTO %(tbl)s_history (%(columns)s, version, dest_bit)
				SELECT %(columns)s, ver, '1' FROM %(tbl)s_data_version(id2num(parent(ver))) WHERE uid = rowuid;
		END IF;
		
		--flag is_generated set to false
		UPDATE changeset SET is_generated = FALSE WHERE id = ver;
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	# template string for commit function
	commit_string = '''CREATE FUNCTION
	%(tbl)s_commit()
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
	BEGIN
		SELECT get_current_changeset() INTO ver;
		UPDATE %(tbl)s as tbl SET %(assign)s
			FROM %(tbl)s_history as new
				WHERE new.version = ver AND tbl.uid = new.uid AND dest_bit = '0';
		INSERT INTO %(tbl)s (%(columns)s)
			SELECT %(columns)s FROM %(tbl)s_history
				WHERE version = ver AND uid NOT IN ( SELECT uid FROM %(tbl)s ) AND dest_bit = '0';
		DELETE FROM %(tbl)s
			WHERE uid IN (SELECT uid FROM %(tbl)s_history
				WHERE version = ver AND dest_bit = '1');
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	# template string for commit function
	commit_templated_string = '''CREATE FUNCTION
	%(tbl)s_commit()
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
	BEGIN
		CREATE TEMP TABLE temp_%(tbl)s_current_changeset AS
			WITH RECURSIVE resolved_data AS (
			SELECT %(columns)s, %(template_column)s,name,uid,version,dest_bit,%(template_column)s as orig_template
			FROM %(tbl)s_history
			WHERE version = get_current_changeset()
			UNION ALL
			SELECT
				%(rd_dv_coalesce)s
				dv.%(template_column)s AS %(template_column)s, rd.name AS name, rd.uid AS uid , rd.version AS version, rd.dest_bit AS dest_bit, rd.orig_template AS orig_template
			FROM %(template_tbl)s_data_version() dv, resolved_data rd
			WHERE dv.uid = rd.%(template_column)s
			)
			SELECT %(columns_except_template)s, version,dest_bit, orig_template AS %(template_column)s
			FROM resolved_data WHERE %(template_column)s IS NULL;

		SELECT get_current_changeset() INTO ver;
		UPDATE %(tbl)s AS tbl SET %(assign)s
			FROM temp_%(tbl)s_current_changeset as new
				WHERE tbl.uid = new.uid AND dest_bit = '0';
		INSERT INTO %(tbl)s (%(columns)s,name,uid,%(template_column)s)
			SELECT %(columns)s,name,uid,%(template_column)s FROM temp_%(tbl)s_current_changeset
				WHERE uid NOT IN ( SELECT uid FROM %(tbl)s ) AND dest_bit = '0';
		DELETE FROM %(tbl)s
			WHERE uid IN (SELECT uid FROM temp_%(tbl)s_current_changeset
				WHERE version = ver AND dest_bit = '1');

		DROP TABLE temp_%(tbl)s_current_changeset;
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	# template string for commit function (for templates of some kind)
	# in addition to another commits modification of template affectes contain of templated objects by it
	#tbl is here table that is templated by template_table for which is this commit function
	commit_kind_template_string = '''CREATE FUNCTION
	%(tbl)s_template_commit()
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
	BEGIN
		SELECT get_current_changeset() INTO ver;
		CREATE TEMP TABLE temp_%(tbl)s_template_data AS SELECT * FROM %(tbl)s_template_data_version();

		--resolved data that are new in current changeset
		CREATE TEMP TABLE temp_%(tbl)s_template_current_changeset AS
			WITH RECURSIVE resolved_data AS (
			SELECT %(columns)s, %(template_column)s,name,uid,version,dest_bit,%(template_column)s as orig_template
			FROM %(tbl)s_template_history
			WHERE version = ver
			UNION ALL
			SELECT
				%(rd_dv_coalesce)s
				dv.%(template_column)s AS %(template_column)s, rd.name AS name, rd.uid AS uid , rd.version AS version, rd.dest_bit AS dest_bit, rd.orig_template AS orig_template
			FROM temp_%(tbl)s_template_data dv, resolved_data rd
			WHERE dv.uid = rd.%(template_column)s
			)
			SELECT %(columns_except_template)s, version,dest_bit, orig_template AS %(template_column)s
			FROM resolved_data WHERE %(template_column)s IS NULL;

		UPDATE %(tbl)s_template AS tbl SET %(assign)s
			FROM temp_%(tbl)s_template_current_changeset as new
				WHERE tbl.uid = new.uid AND dest_bit = '0';
		INSERT INTO %(tbl)s_template (%(columns)s,name,uid,%(template_column)s)
			SELECT %(columns)s,name,uid,%(template_column)s FROM temp_%(tbl)s_template_current_changeset
				WHERE uid NOT IN ( SELECT uid FROM %(tbl)s_template ) AND dest_bit = '0';
		DELETE FROM %(tbl)s_template
			WHERE uid IN (SELECT uid FROM temp_%(tbl)s_template_current_changeset
				WHERE version = ver AND dest_bit = '1');

		--create temp table with uids of affected tbl_templates
		--it is needed to update templates and table in production that are templated by some affected template
		CREATE TEMP TABLE affected_templates AS (
			WITH RECURSIVE resolved_data AS (
				SELECT uid FROM %(tbl)s_template_history WHERE version = ver
				UNION ALL
				SELECT dv.uid FROM temp_%(tbl)s_template_data dv, resolved_data rd WHERE dv.%(template_column)s = rd.uid
			)
			SELECT uid
			FROM resolved_data
		);

		--resolved data for all templates that where affected by some change of template and wasn't changed in current changeset
		--templates changed in current changeset was already updated
		CREATE TEMP TABLE temp_affected_%(tbl)s_template_data AS
			WITH RECURSIVE resolved_data AS (
			SELECT %(columns)s, %(template_column)s,name,uid,version,dest_bit,%(template_column)s as orig_template
			FROM temp_%(tbl)s_template_data
			WHERE  version <> ver
				AND %(template_column)s IN (SELECT uid FROM affected_templates)
			UNION ALL
			SELECT
				%(rd_dv_coalesce)s
				dv.%(template_column)s AS %(template_column)s, rd.name AS name, rd.uid AS uid , rd.version AS version, rd.dest_bit AS dest_bit, rd.orig_template AS orig_template
			FROM temp_%(tbl)s_template_data dv, resolved_data rd
			WHERE dv.uid = rd.%(template_column)s
			)
			SELECT %(columns_except_template)s, version,dest_bit, orig_template AS %(template_column)s
			FROM resolved_data WHERE %(template_column)s IS NULL;

		--object which is not modified in currentchangeset (is not updated by tbl_commit) and is templated by modified template, should be updated now
		--update production.tbl_template
		UPDATE %(tbl)s_template AS tbl SET %(assign)s
			FROM temp_affected_%(tbl)s_template_data as new
				WHERE tbl.uid = new.uid AND dest_bit = '0';

		--update production.%(tbl)s as tbl set att = new.att ... from resolved_data
		CREATE TEMP TABLE temp_%(tbl)s_data AS SELECT * FROM %(tbl)s_data_version();
		CREATE TEMP TABLE temp_affected_%(tbl)s_data AS
			WITH RECURSIVE resolved_data AS (
			SELECT %(columns)s, %(template_column)s,name,uid,version,dest_bit,%(template_column)s as orig_template
			FROM temp_%(tbl)s_data
			WHERE %(template_column)s IN (SELECT uid FROM affected_templates)
			UNION ALL
			SELECT
				%(rd_dv_coalesce)s
				dv.%(template_column)s AS %(template_column)s, rd.name AS name, rd.uid AS uid , rd.version AS version, rd.dest_bit AS dest_bit, rd.orig_template AS orig_template
			FROM temp_%(tbl)s_template_data dv, resolved_data rd
			WHERE dv.uid = rd.%(template_column)s
			)
			SELECT %(columns_except_template)s, version,dest_bit, orig_template AS %(template_column)s
			FROM resolved_data WHERE %(template_column)s IS NULL;

		UPDATE %(tbl)s AS tbl SET %(assign)s
			FROM temp_affected_%(tbl)s_data as new
				WHERE tbl.uid = new.uid;

		DROP TABLE temp_%(tbl)s_template_current_changeset;
		DROP TABLE affected_templates;
		DROP TABLE temp_affected_%(tbl)s_data;
		DROP TABLE temp_affected_%(tbl)s_template_data;
		DROP TABLE temp_%(tbl)s_template_data;
		DROP TABLE temp_%(tbl)s_data;
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	# template string for names
	names_string = '''CREATE FUNCTION
	%(tbl)s_names(from_version bigint = 0)
	RETURNS SETOF identifier
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
				RETURN QUERY SELECT name FROM production.%(tbl)s;
			ELSE
				--for opened changeset set from_version to its parent
				parent_changeset = parent(current_changeset);
				from_version = id2num(parent_changeset);
			END IF;
		ELSE
			current_changeset = NULL;
		END IF;

		--returns union of names in current changeset and names present in from_version (for opened changeset parent revision)
		RETURN QUERY SELECT name FROM %(tbl)s_history WHERE version = current_changeset AND dest_bit = '0'
		UNION
		SELECT name
		FROM %(tbl)s_data_version(from_version)
		WHERE uid NOT IN(
				SELECT uid FROM %(tbl)s_history WHERE version = current_changeset
			);
	END;
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

	# template string for names
	names_embed_string = '''CREATE FUNCTION
	%(tbl)s_names(from_version bigint = 0)
	RETURNS SETOF text
	AS
	$$
	DECLARE
		ver bigint;
		local_name text = NULL;
		rest_of_name text;
		%(reftbl)s_uid bigint;
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
				RETURN QUERY SELECT join_with_delim(%(reftbl)s_get_name(%(column)s, from_version), name, '%(delim)s') FROM %(tbl)s;
			ELSE
				parent_changeset = parent(current_changeset);
				version_to_search = id2num(parent_changeset);
			END IF;
		ELSE
			current_changeset = NULL;
		END IF;

		--returns union of names in current changeset and names present in from_version (for opened changeset parent revision)
		RETURN QUERY SELECT %(tbl)s_get_name(uid) FROM %(tbl)s_history WHERE version = current_changeset AND dest_bit = '0'
		UNION
		SELECT %(tbl)s_get_name(uid, from_version)
		FROM %(tbl)s_data_version(version_to_search)
		WHERE uid NOT IN(
				SELECT uid FROM %(tbl)s_history WHERE version = current_changeset
			);
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

'''

#template for getting deleted objects between two versions
	diff_deleted_string = '''CREATE FUNCTION
%(tbl)s_diff_deleted()
RETURNS SETOF text
AS
$$
BEGIN
	--deleted were between two versions objects that have set dest_bit in new data
	RETURN QUERY SELECT cast(old_name as text) FROM %(tbl)s_diff_data WHERE new_dest_bit = '1' AND old_name IS NOT NULL;
END;
$$
LANGUAGE plpgsql;

'''

 #template for getting created objects between two versions
	diff_created_string = '''CREATE FUNCTION
%(tbl)s_diff_created()
RETURNS SETOF text
AS
$$
BEGIN
	--created were objects which are in new data and not deleted and are not in old data
	RETURN QUERY SELECT cast(new_name as text) FROM %(tbl)s_diff_data WHERE old_name IS NULL AND new_dest_bit = '0';
END;
$$
LANGUAGE plpgsql;

'''

#template for function that finds all rename changes
	diff_rename_string = '''CREATE FUNCTION %(tbl)s_diff_rename()
RETURNS SETOF deska.diff_rename_type
AS
$$
BEGIN
	RETURN QUERY SELECT cast(old_name as text), cast(new_name as text)
	FROM %(tbl)s_diff_data
	WHERE new_name IS NOT NULL AND new_dest_bit = '0' AND new_name <> old_name;
END;
$$
LANGUAGE plpgsql;
'''

#template for function that finds all rename changes
	diff_rename_embed_string = '''CREATE FUNCTION %(tbl)s_diff_rename()
RETURNS SETOF deska.diff_rename_type
AS
$$
BEGIN
	RETURN QUERY SELECT old_name, new_name
	FROM %(tbl)s_diff_data
	WHERE new_name IS NOT NULL AND new_dest_bit = '0' AND local_name_differs(new_name,old_name,'%(delim)s');
END;
$$
LANGUAGE plpgsql;
'''

#template for if constructs in diff_set_attribute
	one_column_change_string = '''
	 IF (old_data.%(column)s <> new_data.%(column)s) OR ((old_data.%(column)s IS NULL OR new_data.%(column)s IS NULL)
		  AND NOT(old_data.%(column)s IS NULL AND new_data.%(column)s IS NULL))
	 THEN
		  result.attribute = '%(column)s';
		  result.olddata = old_data.%(column)s;
		  result.newdata = new_data.%(column)s;
		  RETURN NEXT result;
	 END IF;

'''

#template for if constructs in diff_set_attribute, this version is for refuid columns
	one_column_change_ref_uid_string = '''
	 IF (old_data.%(column)s <> new_data.%(column)s) OR ((old_data.%(column)s IS NULL OR new_data.%(column)s IS NULL)
		  AND NOT(old_data.%(column)s IS NULL AND new_data.%(column)s IS NULL))
	 THEN
		  result.attribute = '%(column)s';
		  result.olddata = %(reftbl)s_get_name(old_data.%(column)s, from_version);
		  result.newdata = %(reftbl)s_get_name(new_data.%(column)s, to_version);
		  RETURN NEXT result;
	 END IF;

'''

#template for if constructs in diff_set_attribute, this version is for refuid columns
	one_column_change_ref_set_string = '''
	IF NOT inner_%(tbl)s_%(refcol)s_multiRef_sets_equal(newuid) 
	THEN
		result.attribute = '%(column)s';
		result.olddata = deska.ret_id_set(inner_%(tbl)s_%(refcol)s_multiRef_get_old_set(newuid));
		result.newdata = deska.ret_id_set(inner_%(tbl)s_%(refcol)s_multiRef_get_new_set(newuid));
		RETURN NEXT result;
	END IF;

'''

#template for getting created objects between two versions
#return type is defined in file diff.sql and created in create script
#parameters are necessary for get_name
	diff_set_attribute_string = '''CREATE FUNCTION
	%(tbl)s_diff_set_attributes(from_version bigint = 0, to_version bigint = 0)
	 RETURNS SETOF diff_set_attribute_type
	 AS
	 $$
	 DECLARE
		old_data %(tbl)s_diff_data_type;
		new_data %(tbl)s_diff_data_type;
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
		FOR %(old_new_obj_list)s IN
			SELECT %(select_old_new_list)s
			FROM %(tbl)s_diff_data
			WHERE new_name IS NOT NULL AND new_dest_bit = '0'
		LOOP
			--all changes are mentioned with new name
			result.objname = new_data.name;
			--check if the column was changed
			%(columns_changes)s
		END LOOP;
	 END
	 $$
	 LANGUAGE plpgsql;

'''

#diff set_attribute for attributes that refers to set o identifiers
	diff_refs_set_set_attribute_string = '''CREATE FUNCTION
	%(tbl)s_diff_refs_set_set_attributes(from_version bigint = 0, to_version bigint = 0)
	 RETURNS SETOF diff_refs_set_set_attribute_type
	 AS
	 $$
	 DECLARE
		%(set_variables)s
		newuid bigint;
		olduid bigint;
		newname text;
		oldname text;
		result diff_refs_set_set_attribute_type;
		current_changeset bigint;
	 BEGIN
		--sets from_version to parent revision, for diff in current changeset use
		IF from_version = 0 THEN
			--could raise exception, if you dont have opened changeset and you call this function for diff made in a current changeset
			current_changeset = get_current_changeset();
			from_version = id2num(parent(current_changeset));
		END IF;

		--for each row in diff_data lists modifications of each attribute
		FOR newuid, olduid, newname, oldname, %(old_new_obj_list)s IN
			SELECT new_uid, old_uid, new_name, old_name, %(select_old_new_list)s
			FROM %(tbl)s_diff_data
			WHERE new_name IS NOT NULL
		LOOP
			newuid = COALESCE(newuid, olduid);
			--all other changes are mentioned with new name
			result.objname = newname;
			--check if the column was changed
			%(columns_changes)s
		END LOOP;
	 END
	 $$
	 LANGUAGE plpgsql;

'''


#template for function, which selects all data from kind table taht are present in version data_version
#is used in diff functions
	data_version_function_string = '''CREATE FUNCTION %(tbl)s_data_version(data_version bigint = 0)
RETURNS SETOF %(tbl)s_history
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
	SELECT * FROM %(tbl)s_history
		WHERE version = changeset_id AND dest_bit = '0'
	UNION
	SELECT h1.* FROM %(tbl)s_history h1
		JOIN version v1 ON (v1.id = h1.version)
		JOIN (	SELECT uid, max(num) AS maxnum
				FROM %(tbl)s_history h JOIN version v ON (v.id = h.version )
				WHERE v.num <= data_version
				GROUP BY uid
			) vmax1
		ON (h1.uid = vmax1.uid AND v1.num = vmax1.maxnum)
	WHERE h1.dest_bit = '0' AND h1.uid NOT IN (
		SELECT uid FROM %(tbl)s_history
		WHERE version = changeset_id
	);
END
$$
LANGUAGE plpgsql;

'''

	data_changeset_function_string = '''CREATE FUNCTION %(tbl)s_data_changeset(changeset_id bigint = 0)
RETURNS SETOF %(tbl)s_history AS
$$
DECLARE
 parent_version bigint;
 to_version bigint;
BEGIN
	IF changeset_id = 0 THEN
		changeset_id = get_current_changeset();
	END IF;

	BEGIN
		parent_version = id2num(parent(changeset_id));
	EXCEPTION
		--has no parent in changeset table, changeset was already closed
		WHEN SQLSTATE '70001' THEN
			SELECT num INTO to_version FROM version WHERE id = changeset_id;
			RETURN QUERY SELECT * FROM %(tbl)s_data_version(to_version);
			RETURN;
	END;

	RETURN QUERY 
	SELECT par.* FROM %(tbl)s_data_version(parent_version) par
		LEFT OUTER JOIN %(tbl)s_history curr ON (par.uid = curr.uid AND curr.version = changeset_id)
	WHERE curr.uid IS NULL
	UNION
	SELECT * FROM %(tbl)s_history WHERE version = changeset_id;
END
$$
LANGUAGE plpgsql;

'''


#template for function which selects all changes of all objects, that where done between from_version and to_version versions
#is used in diff functions
	data_changes_function_string = '''CREATE FUNCTION %(tbl)s_changes_between_versions(from_version bigint, to_version bigint)
RETURNS SETOF %(tbl)s_history
AS
$$
BEGIN
	--for each object uid finds its last modification between versions from_version and to_version
	--joins it with history table of its kind to get object data in version to_version of all objects modified between from_version and to_version
	RETURN QUERY
	SELECT h1.*
	FROM %(tbl)s_history h1
		JOIN version v1 on (v1.id = h1.version)
		JOIN (	SELECT uid, max(num) as maxnum
				FROM %(tbl)s_history h join version v on (v.id = h.version )
				WHERE v.num <= to_version and v.num > from_version
				GROUP BY uid) vmax1
		ON(h1.uid = vmax1.uid and v1.num = vmax1.maxnum);
END
$$
LANGUAGE plpgsql;

'''

	diff_data_type_str = '''CREATE TYPE %(tbl)s_diff_data_type AS(
	name text,
	%(col_types)s,
	dest_bit bit(1)
);
'''

#template for function that returns resolved modifications between two versions
	data_resolved_changes_function_string = '''CREATE OR REPLACE FUNCTION genproc.%(tbl)s_resolved_changes_between_versions(from_version bigint, to_version bigint)
RETURNS SETOF %(tbl)s_diff_data_type AS
$$
DECLARE
	changeset_id bigint;
BEGIN
	changeset_id = get_current_changeset_or_null();
	IF from_version = 0 AND changeset_id IS NULL  THEN
		CREATE TEMP TABLE template_data_version AS SELECT * FROM production.%(templ_tbl)s;
	ELSE
		CREATE TEMP TABLE template_data_version AS SELECT * FROM %(templ_tbl)s_data_version(to_version);
	END IF;

	--for each object uid finds its last modification between versions from_version and to_version
	--joins it with history table of its kind to get object data in version to_version of all objects modified between from_version and to_version
	RETURN QUERY
	WITH RECURSIVE resolved_data AS (
		SELECT uid,name,%(columns_ex_templ)s, %(template_column)s, %(template_column)s as orig_template,dest_bit
		FROM %(tbl)s_changes_between_versions(from_version, to_version)
		UNION ALL
		SELECT
			rd.uid AS uid, rd.name AS name,
			%(rd_dv_coalesce)s,
			dv.%(template_column)s AS %(template_column)s, rd.orig_template AS orig_template,
			rd.dest_bit AS dest_bit
		FROM template_data_version dv, resolved_data rd
		WHERE dv.uid = rd.%(template_column)s
	)
	SELECT uid, name, %(columns_ex_templ)s,orig_template AS %(template_column)s, dest_bit
	FROM resolved_data WHERE %(template_column)s IS NULL;

	DROP TABLE template_data_version;
END;
$$
LANGUAGE plpgsql;

'''


#template for function that prepairs temp table for diff functions
	diff_init_function_string = '''CREATE FUNCTION %(tbl)s_init_diff(from_version bigint, to_version bigint)
RETURNS void
AS
$$
BEGIN
	--full outer join of data in from_version and list of changes made between from_version and to_version
	CREATE TEMP TABLE %(tbl)s_diff_data
	AS SELECT %(diff_columns)s
		FROM %(tbl)s_data_version(from_version) dv FULL OUTER JOIN %(tbl)s_changes_between_versions(from_version,to_version) chv ON (dv.uid = chv.uid);
	
	%(inner_tables_diff)s
END
$$
LANGUAGE plpgsql;

'''


#template for function that prepairs temp table for diff functions from resolved_data
	diff_init_resolved_function_string = '''CREATE FUNCTION %(tbl)s_init_resolved_diff(from_version bigint, to_version bigint)
RETURNS void
AS
$$
BEGIN
	--full outer join of data in from_version and list of changes made between from_version and to_version
	CREATE TEMP TABLE %(tbl)s_diff_data
	AS SELECT %(diff_columns)s
		FROM %(tbl)s_resolved_data(from_version) dv FULL OUTER JOIN %(tbl)s_resolved_data(to_version) chv ON (dv.uid = chv.uid);
		
	%(inner_tables_diff)s
END
$$
LANGUAGE plpgsql;

'''


#template for function that prepairs temp table for diff functions, which selects diffs between opened changeset and its parent
	diff_changeset_init_function_string = '''CREATE OR REPLACE FUNCTION %(tbl)s_init_ch_diff(changeset_id bigint = 0)
RETURNS void
AS
$$
DECLARE
	from_version bigint;
BEGIN
	IF changeset_id = 0 THEN
		--it's necessary to have opened changeset in witch we would like to see diff
		changeset_id = get_current_changeset();
	END IF;
	
	BEGIN
		from_version = id2num(parent(changeset_id));
	EXCEPTION
		--parent version was not found in table with opened changesets, given changeset was already closed
		WHEN SQLSTATE '70001' THEN
			SELECT num -1 INTO from_version FROM version WHERE id = changeset_id;
	END;
	--full outer join of data in parent revision and changes made in opened changeset
	CREATE TEMP TABLE %(tbl)s_diff_data
	AS  SELECT %(diff_columns)s
		FROM (SELECT * FROM %(tbl)s_history WHERE version = changeset_id) chv
			FULL OUTER JOIN %(tbl)s_data_version(from_version) dv ON (dv.uid = chv.uid);
	
	%(inner_tables_diff)s
END
$$
  LANGUAGE plpgsql;

'''

#template for function that prepairs temp table for diff functions, which selects diffs between changeset and its parent
	diff_changeset_init_resolved_function_string = '''CREATE OR REPLACE FUNCTION %(tbl)s_init_ch_resolved_diff(changeset_id bigint = 0)
RETURNS void
AS
$$
DECLARE
	from_version bigint;
	to_version bigint;
BEGIN
	--it's necessary to have opened changeset in witch we would like to see diff
	IF changeset_id = 0 THEN
		--it's necessary to have opened changeset in witch we would like to see diff
		changeset_id = get_current_changeset();
	END IF;
	
	BEGIN
		from_version = id2num(parent(changeset_id));
	EXCEPTION
		--parent version was not found in table with opened changesets, given changeset was already closed
		WHEN SQLSTATE '70001' THEN
			SELECT num INTO to_version FROM version WHERE id = changeset_id;
			from_version = to_version - 1;
			CREATE TEMP TABLE %(tbl)s_diff_data AS 
				SELECT %(diff_columns)s FROM %(tbl)s_resolved_data(to_version) chv
					LEFT OUTER JOIN %(tbl)s_resolved_data(from_version) dv ON (dv.uid = chv.uid);
			
			%(inner_tables_diff)s
			
			RETURN;
	END;
	
	--data valid in changeset_id are the newest data present in the from_version and in the changeset with the changeset_id
	CREATE TEMP TABLE local_template_data_version AS SELECT * FROM %(templ_tbl)s_data_changeset(changeset_id);
	
	CREATE TEMP TABLE current_changeset_resolved_data AS (
		WITH RECURSIVE resolved_data AS(
		--it is necessary to resolve all data in current version, resolved changes from changeset could not find inherited changes from templates
		SELECT uid, name, %(columns_ex_templ)s, %(template_column)s, %(template_column)s AS orig_template, dest_bit FROM %(tbl)s_data_changeset(changeset_id)
		UNION ALL
		SELECT
			rd.uid AS uid, rd.name AS name,
			%(rd_dv_coalesce)s,
			dv.%(template_column)s AS %(template_column)s, rd.orig_template AS orig_template,
			rd.dest_bit AS dest_bit
		FROM local_template_data_version dv, resolved_data rd
		WHERE dv.uid = rd.%(template_column)s
		)
		SELECT uid, name, %(columns_ex_templ)s,orig_template AS %(template_column)s, dest_bit
		FROM resolved_data WHERE %(template_column)s IS NULL
	);
	--full outer join of data in parent revision and changes made in opened changeset
	CREATE TEMP TABLE %(tbl)s_diff_data
	AS  SELECT %(diff_columns)s
		FROM current_changeset_resolved_data chv
			FULL OUTER JOIN %(tbl)s_resolved_data(from_version) dv ON (dv.uid = chv.uid);

	DROP TABLE current_changeset_resolved_data;
	DROP TABLE local_template_data_version;
	
	%(inner_tables_diff)s
END
$$
  LANGUAGE plpgsql;

'''


#template for terminating diff, drops temp table with data for diff functions
	diff_terminate_function_string = '''CREATE FUNCTION
%(tbl)s_terminate_diff()
RETURNS void
AS
$$
BEGIN
	DROP TABLE %(tbl)s_diff_data;
	%(inner_temrinate_diff)s
END;
$$
LANGUAGE plpgsql;

'''

	diff_terminate_refuid_set_function_string = '''CREATE FUNCTION
%(tbl)s_%(refcol)s_terminate_diff()
RETURNS void
AS
$$
BEGIN
	PERFORM inner_%(tbl)s_%(refcol)s_multiref_terminate_diff();
END;
$$
LANGUAGE plpgsql;

'''

	resolved_object_data_string = '''CREATE OR REPLACE FUNCTION %(tbl)s_resolved_object_data(name_ text, from_version bigint = 0)
RETURNS %(tbl)s_type
AS
$$
DECLARE
	data %(tbl)s_type;
	current_changeset bigint;
	dbit bit(1);
BEGIN
	IF from_version = 0 THEN
		current_changeset = get_current_changeset_or_null();
		IF current_changeset IS NULL THEN
			--user wants current data from production
			--select columns with resolved id_set and resolved refuid names
			SELECT %(columns_ex_templ_id_set_res_name)s, %(templ_tbl)s_get_name(%(template_column)s) INTO %(data_columns)s
			FROM production.%(tbl)s WHERE name = name_;
			IF NOT FOUND THEN
				RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
			END IF;
			RETURN data;
		END IF;
	END IF;

	CREATE TEMP TABLE template_data_version AS SELECT * FROM %(templ_tbl)s_data_version(from_version);

	WITH recursive resolved_data AS (
        --select uids for refuids and resolved id_set
        SELECT %(columns_ex_templ_id_set)s, %(template_column)s, %(template_column)s as orig_template
        FROM %(tbl)s_data_version(from_version)
        WHERE name = name_
        UNION ALL
        SELECT
			%(rd_dv_coalesce)s,
			dv.%(template_column)s, rd.orig_template
        FROM template_data_version dv, resolved_data rd
        WHERE dv.uid = rd.%(template_column)s
	)
	--select id_set from resolved_data and get name for refuid
	SELECT %(columns_ex_templ)s, %(templ_tbl)s_get_name(orig_template) AS %(template_column)s INTO %(data_columns)s
	FROM resolved_data WHERE %(template_column)s IS NULL;

	IF NOT FOUND THEN
		RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
	END IF;

	DROP TABLE template_data_version;

	RETURN data;
END
$$
LANGUAGE plpgsql;

'''

	resolved_object_data_embed_string = '''CREATE OR REPLACE FUNCTION %(tbl)s_resolved_object_data(name_ text, from_version bigint = 0)
RETURNS %(tbl)s_type
AS
$$
DECLARE
	data %(tbl)s_type;
	current_changeset bigint;
	obj_uid bigint;
BEGIN
	obj_uid = %(tbl)s_get_uid(name_, from_version);
	IF from_version = 0 THEN
		current_changeset = get_current_changeset_or_null();
		IF current_changeset IS NULL THEN
			--user wants current data from production
			SELECT %(columns_ex_templ)s, %(templ_tbl)s_get_name(%(template_column)s) INTO %(data_columns)s
			FROM production.%(tbl)s WHERE uid = obj_uid;
			IF NOT FOUND THEN
				RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
			END IF;
			RETURN data;
		END IF;
	END IF;

	CREATE TEMP TABLE template_data_version AS SELECT * FROM %(templ_tbl)s_data_version(from_version);

	WITH recursive resolved_data AS (
        SELECT %(columns)s, %(template_column)s, %(template_column)s as orig_template
        FROM %(tbl)s_data_version(from_version)
        WHERE uid = obj_uid
        UNION ALL
        SELECT
			%(rd_dv_coalesce)s,
			dv.%(template_column)s, rd.orig_template
        FROM template_data_version dv, resolved_data rd
        WHERE dv.uid = rd.%(template_column)s
	)
	SELECT %(columns_ex_templ)s, %(templ_tbl)s_get_name(orig_template) AS %(template_column)s INTO %(data_columns)s
	FROM resolved_data WHERE %(template_column)s IS NULL;

	IF NOT FOUND THEN
		RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
	END IF;

	DROP TABLE template_data_version;
	RETURN data;
END
$$
LANGUAGE plpgsql;


'''

	resolved_data_template_info_type_string = '''CREATE TYPE %(tbl)s_data_template_info_type AS(
	%(columns)s,
	%(templ_columns)s,
	%(template_column)s text
);
'''

	resolved_object_data_template_info_string = '''CREATE OR REPLACE FUNCTION %(tbl)s_resolved_object_data_template_info(name_ text, from_version bigint = 0)
RETURNS %(tbl)s_data_template_info_type
AS
$$
DECLARE
	data %(tbl)s_data_template_info_type;
BEGIN
	CREATE TEMP TABLE template_data_version AS SELECT * FROM %(templ_tbl)s_data_version(from_version);

	WITH recursive resolved_data AS (
        SELECT %(columns)s, %(case_columns)s, %(template_column)s, %(template_column)s as orig_template
        FROM %(tbl)s_data_version(from_version)
        WHERE name = name_
        UNION ALL
        SELECT
			%(rd_dv_coalesce)s,
			%(templ_case_columns)s,
			dv.%(template_column)s, rd.orig_template
        FROM template_data_version dv, resolved_data rd
        WHERE dv.uid = rd.%(template_column)s
	)
	SELECT %(columns_ex_templ)s, %(columns_templ)s, %(templ_tbl)s_get_name(orig_template) AS %(template_column)s INTO %(data_columns)s
	FROM resolved_data WHERE %(template_column)s IS NULL;

	IF NOT FOUND THEN
		RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
	END IF;

	DROP TABLE template_data_version;

	RETURN data;
END
$$
LANGUAGE plpgsql;

'''

	resolved_object_data_template_info_embed_string = '''CREATE OR REPLACE FUNCTION %(tbl)s_resolved_object_data_template_info(name_ text, from_version bigint = 0)
RETURNS %(tbl)s_data_template_info_type
AS
$$
DECLARE
	obj_uid bigint;
	data %(tbl)s_data_template_info_type;
BEGIN
	CREATE TEMP TABLE template_data_version AS SELECT * FROM %(templ_tbl)s_data_version(from_version);

	obj_uid = %(tbl)s_get_uid(name_, from_version);
	WITH recursive resolved_data AS (
        SELECT %(columns)s, %(case_columns)s, %(template_column)s, %(template_column)s as orig_template
        FROM %(tbl)s_data_version(from_version)
        WHERE uid = obj_uid
        UNION ALL
        SELECT
			%(rd_dv_coalesce)s,
			%(templ_case_columns)s,
			dv.%(template_column)s, rd.orig_template
        FROM template_data_version dv, resolved_data rd
        WHERE dv.uid = rd.%(template_column)s
	)
	SELECT %(columns_ex_templ)s, %(columns_templ)s, %(templ_tbl)s_get_name(orig_template) AS %(template_column)s INTO %(data_columns)s
	FROM resolved_data WHERE %(template_column)s IS NULL;

	IF NOT FOUND THEN
		RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
	END IF;

	DROP TABLE template_data_version;

	RETURN data;
END
$$
LANGUAGE plpgsql;

'''

	multiple_resolved_data_template_info_type_string = '''CREATE TYPE multiple_%(tbl)s_data_template_info_type AS(
	name identifier,
	uid bigint,
	%(columns)s,
	--columns for origin of data, templates' names
	%(templ_columns)s,
	%(template_column)s bigint,
	dest_bit bit(1)
);
'''

	multiple_resolved_data_type_string = '''CREATE TYPE multiple_%(tbl)s_data_type AS(
	name identifier,
	uid bigint,
	%(columns)s,
	%(template_column)s bigint,
	dest_bit bit(1)
);
'''

	resolved_data_template_info_string = '''CREATE OR REPLACE FUNCTION %(tbl)s_resolved_data_template_info(from_version bigint = 0)
RETURNS SETOF multiple_%(tbl)s_data_template_info_type
AS
$$
BEGIN
	CREATE TEMP TABLE template_data_version AS SELECT * FROM %(templ_tbl)s_data_version(from_version);

	RETURN QUERY WITH recursive resolved_data AS (
		SELECT uid,name,%(columns_ex_templ_id_set)s, %(case_columns)s, %(template_column)s, %(template_column)s as orig_template, dest_bit
		FROM %(tbl)s_data_version(from_version)
		UNION ALL
		SELECT
			rd.uid AS uid, rd.name AS name,
			%(rd_dv_coalesce)s,
			%(templ_case_columns)s,
			dv.%(template_column)s, rd.orig_template,
			rd.dest_bit
        FROM template_data_version dv, resolved_data rd
        WHERE dv.uid = rd.%(template_column)s
	)
	SELECT name, uid, %(columns_ex_templ)s, %(columns_templ)s, orig_template AS template, dest_bit
	FROM resolved_data WHERE %(template_column)s IS NULL;

	DROP TABLE template_data_version;
END
$$
LANGUAGE plpgsql;

'''

	ref_set_coal_string = '''--id is id of base table which is templated and refers to the set
CREATE OR REPLACE FUNCTION %(tbl)s_%(refcol)s_ref_set_coal(old_array text[], new_obj_id bigint, from_version bigint = 0)
RETURNS text[]
AS
$$
BEGIN
--new_obj_id is always template
	IF old_array IS NOT NULL THEN
		RETURN old_array;
	END IF;

	RETURN %(tbl_template)s_get_%(refcol)s(new_obj_id, from_version);
END;
$$
LANGUAGE plpgsql;

'''

	resolved_data_string = '''CREATE OR REPLACE FUNCTION %(tbl)s_resolved_data(from_version bigint = 0)
RETURNS SETOF multiple_%(tbl)s_data_type
AS
$$
DECLARE
	changeset_id integer;
BEGIN
	changeset_id = get_current_changeset_or_null();
	IF from_version = 0 AND changeset_id IS NULL  THEN
		RETURN QUERY SELECT name, uid, %(columns_ex_templ_res_id_set)s, %(template_column)s AS %(template_column)s, B'0'::bit(1) AS dest_bit FROM production.%(tbl)s;
	ELSE
		CREATE TEMP TABLE rd_template_data_version AS SELECT * FROM %(templ_tbl)s_data_version(from_version);

		RETURN QUERY WITH recursive resolved_data AS (
		--id_set in is get 
			SELECT uid,name,version,%(columns_ex_templ_id_set)s, %(template_column)s, %(template_column)s as orig_template, dest_bit
			FROM %(tbl)s_data_version(from_version)
			UNION ALL
			SELECT
				rd.uid AS uid, rd.name AS name, rd.version AS version,
				%(rd_dv_coalesce)s,
				dv.%(template_column)s, rd.orig_template,
				rd.dest_bit
			FROM rd_template_data_version dv, resolved_data rd
			WHERE dv.uid = rd.%(template_column)s
		)
		SELECT name, uid, %(columns_ex_templ)s, orig_template AS %(template_column)s, dest_bit
		FROM resolved_data WHERE %(template_column)s IS NULL;

		DROP TABLE rd_template_data_version;
	END IF;
END
$$
LANGUAGE plpgsql;

'''
 
