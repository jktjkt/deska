class MultiRef:
    multiRef_tables_str = "SELECT relname FROM get_table_info() WHERE typname = 'identifier_set';"
    multiRef_info_str = "SELECT attname, refkind, refattname FROM kindRelations_full_info('%(tbl)s') WHERE relation = 'REFERS_TO_SET';"
    tbl_name_template_str = "SELECT DISTINCT template, kind FROM get_templates_info();"
    template_column_str = "SELECT attname FROM kindRelations_full_info('%(tbl)s') WHERE relation = 'TEMPLATIZED';"
    
    add_inner_table_str = '''CREATE TABLE deska.inner_%(tbl)s_%(ref_col)s_multiRef(
%(tbl)s bigint
    CONSTRAINT inner_%(tbl)s_fk_%(ref_col)s REFERENCES %(tbl)s(uid) ON DELETE CASCADE DEFERRABLE INITIALLY IMMEDIATE,
%(ref_tbl)s bigint
    CONSTRAINT inner_%(ref_col)s_fk_%(tbl)s REFERENCES %(ref_tbl)s(uid) DEFERRABLE INITIALLY IMMEDIATE,
flag bit(1) DEFAULT '1', 
CONSTRAINT inner_%(tbl)s_%(ref_col)s_multiRef_unique UNIQUE (%(tbl)s,%(ref_tbl)s)
);

'''

    hist_string = '''CREATE TABLE history.%(tbl)s_history (
    LIKE %(tbl)s
    -- include default values
    INCLUDING DEFAULTS,
    version int NOT NULL,
    CONSTRAINT %(tbl)s_unique UNIQUE (%(tbl_name)s, %(ref_tbl_name)s, version)
);
'''

    add_item_str = '''  CREATE FUNCTION
genproc.inner_%(tbl_name)s_set_%(ref_col)s_insert(IN %(tbl_name)s_name text, IN %(ref_tbl_name)s_name text)
RETURNS integer
AS
$$
DECLARE ver bigint;
    %(tbl_name)s_uid bigint;
    %(ref_tbl_name)s_uid bigint;
BEGIN
    SELECT get_current_changeset() INTO ver;
    %(tbl_name)s_uid = %(tbl_name)s_get_uid(%(tbl_name)s_name);
    IF NOT FOUND THEN
        RAISE 'No %(tbl_name)s with name %% exist in this version', %(tbl_name)s USING ERRCODE = '70021';
    END IF;
    
    %(ref_tbl_name)s_uid = %(ref_tbl_name)s_get_uid(%(ref_tbl_name)s_name);
    IF NOT FOUND THEN
        RAISE 'No %(ref_tbl_name)s with name %% exist in this version', %(ref_tbl_name)s_name USING ERRCODE = '70021';
    END IF;
    
    BEGIN
        --if this set was modified in this changeset exception is thrown (all rows are already present)
        INSERT INTO %(tbl)s_history (%(tbl_name)s, %(ref_tbl_name)s, version) 
            SELECT %(tbl_name)s_uid AS %(tbl_name)s, %(tbl)s_get_object_resolved_set AS %(ref_tbl_name)s, ver AS version 
            FROM %(tbl)s_get_object_resolved_set(%(tbl_name)s_uid)
            WHERE %(tbl)s_get_object_resolved_set IS NOT NULL;
    EXCEPTION WHEN unique_violation THEN
        -- do nothing
    END;
    
    BEGIN
        DELETE FROM %(tbl)s_history WHERE version = ver AND %(tbl_name)s = %(tbl_name)s_uid AND %(ref_tbl_name)s IS NULL;
        INSERT INTO %(tbl)s_history (%(tbl_name)s, %(ref_tbl_name)s, version)
            VALUES (%(tbl_name)s_uid, %(ref_tbl_name)s_uid, ver);
    EXCEPTION WHEN unique_violation THEN
        -- do nothing
    END;
    --flag is_generated set to false
    UPDATE changeset SET is_generated = FALSE WHERE id = ver;
    RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

'''

    del_item_str = '''  CREATE FUNCTION
genproc.inner_%(tbl_name)s_set_%(ref_col)s_remove(IN %(tbl_name)s_name text, IN %(ref_tbl_name)s_name text)
RETURNS integer
AS
$$
DECLARE ver bigint;
    %(tbl_name)s_uid bigint;
    %(ref_tbl_name)s_uid bigint;
BEGIN
    SELECT get_current_changeset() INTO ver;
    %(tbl_name)s_uid = %(tbl_name)s_get_uid(%(tbl_name)s_name);
    IF NOT FOUND THEN
        RAISE 'No %(tbl_name)s with name %% exist in this version', %(tbl_name)s USING ERRCODE = '70021';
    END IF;
    
    %(ref_tbl_name)s_uid = %(ref_tbl_name)s_get_uid(%(ref_tbl_name)s_name);
    IF NOT FOUND THEN
        RAISE 'No %(ref_tbl_name)s with name %% exist in this version', %(ref_tbl_name)s_name USING ERRCODE = '70021';
    END IF;
    
    BEGIN
        INSERT INTO %(tbl)s_history (%(tbl_name)s, %(ref_tbl_name)s, version) 
            SELECT %(tbl_name)s_uid AS %(tbl_name)s, %(tbl)s_get_object_resolved_set AS %(ref_tbl_name)s, ver AS version 
            FROM %(tbl)s_get_object_resolved_set(%(tbl_name)s_uid)
            WHERE %(tbl)s_get_object_resolved_set IS NOT NULL;
    EXCEPTION WHEN unique_violation THEN
        -- do nothing
    END;
        
    DELETE FROM %(tbl)s_history WHERE %(tbl_name)s = %(tbl_name)s_uid AND %(ref_tbl_name)s = %(ref_tbl_name)s_uid  AND version = ver;

    IF NOT exists(SELECT * FROM %(tbl)s_history WHERE %(tbl_name)s = %(tbl_name)s_uid AND version = ver) THEN
        INSERT INTO %(tbl)s_history (%(tbl_name)s, %(ref_tbl_name)s, version) VALUES (%(tbl_name)s_uid, NULL, ver);
    END IF;

    --flag is_generated set to false
    UPDATE changeset SET is_generated = FALSE WHERE id = ver;
    RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

'''

    # template string for set functions for columns that reference set of identifiers
    set_string = '''CREATE FUNCTION
genproc.%(tbl)s_set_%(ref_col)s(IN name_ text,IN value text[])
RETURNS integer
AS
$$
DECLARE ver bigint;
    rowuid bigint;
    refuid bigint;
    pos bigint;
BEGIN
    --for modifications we need to have opened changeset, this function raises exception in case we don't have
    SELECT get_current_changeset() INTO ver;
    SELECT %(tbl_name)s_get_uid(name_) INTO rowuid;
    --not found in case there is no object with name name_ in history
    IF NOT FOUND THEN
        RAISE 'No %(tbl_name)s named %%. Create it first.',name_ USING ERRCODE = '70021';
    END IF;
    
    DELETE FROM %(tbl)s_history WHERE version = ver AND %(tbl_name)s = rowuid;

    --empty set of tuples (rowuid, refuid) represents null value, it is done by delete from ..
    IF value IS NULL THEN
        INSERT INTO %(tbl)s_history (%(tbl_name)s,flag,version) VALUES (rowuid, '0', ver);
        RETURN 1;
    END IF;
    
    --set {(rowuid, NULL)} represents empty set for object rowuid
    IF array_upper(value,1) IS NULL THEN
        INSERT INTO %(tbl)s_history (%(tbl_name)s,%(ref_tbl_name)s,version) VALUES (rowuid, NULL, ver);
        RETURN 1;
    END IF;

    FOR pos IN 1..array_upper(value,1) LOOP
        refuid = %(ref_tbl_name)s_get_uid(value[pos]);
        IF refuid IS NULL THEN
            RAISE 'No %(ref_tbl_name)s named %%. Create it first.',value[pos] USING ERRCODE = '70021';
        END IF;

        INSERT INTO %(tbl)s_history (%(tbl_name)s,%(ref_tbl_name)s,version) VALUES (rowuid, refuid, ver);
    END LOOP;
    
    RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

'''

    data_version_str = '''CREATE FUNCTION %(tbl)s_data_version(data_version bigint = 0)
RETURNS SETOF %(tbl)s_history
AS
$$
DECLARE
    changeset_id bigint;
BEGIN
    --for each object finds its last modification before data_version
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
        WHERE version = changeset_id AND flag = '1'
    UNION
    SELECT h1.* FROM %(tbl)s_history h1
        JOIN version v1 ON (v1.id = h1.version)
        JOIN (  SELECT %(tbl_name)s, max(num) AS maxnum
                FROM %(tbl)s_history h JOIN version v ON (v.id = h.version )
                WHERE v.num <= data_version
                GROUP BY %(tbl_name)s
            ) vmax1
        ON (h1.%(tbl_name)s = vmax1.%(tbl_name)s AND v1.num = vmax1.maxnum)
    WHERE flag = '1' AND h1.%(tbl_name)s NOT IN (
        SELECT %(tbl_name)s FROM %(tbl)s_history
        WHERE version = changeset_id
    );
END
$$
LANGUAGE plpgsql;

'''

    resolved_data = '''CREATE FUNCTION %(tbl)s_resolved_data_version(data_version bigint = 0)
RETURNS SETOF %(tbl)s
AS
$$
DECLARE
    changeset_id bigint;
BEGIN
    changeset_id = get_current_changeset_or_null();
    IF data_version = 0 AND changeset_id IS NULL THEN
        RETURN QUERY SELECT %(reftbl_name)s FROM %(tbl)s;
    ELSE
        CREATE TEMP TABLE template_data_version AS SELECT * FROM %(tbl_template_name)s_data_version(data_version);
        CREATE TEMP TABLE inner_template_data_version AS SELECT * FROM %(tbl_template)s_data_version(data_version);

        RETURN QUERY WITH RECURSIVE resolved_data AS(
        SELECT h.uid, s.%(tbl_name)s, s.%(reftbl_name)s, h.%(template_column)s
        FROM %(tbl_name)s_data_version(data_version) h
        LEFT OUTER JOIN %(tbl)s_data_version(data_version) s ON (s.%(tbl_name)s = h.uid)
        
        UNION ALL
        
        SELECT rd.uid AS uid, rd.%(tbl_name)s, s.%(reftbl_name)s AS %(reftbl_name)s, dv.%(template_column)s AS %(template_column)s, s.flag
        FROM template_data_version dv JOIN resolved_data rd ON (rd.%(template_column)s = dv.uid)
            LEFT OUTER JOIN inner_template_data_version s ON (rd.%(reftbl_name)s IS NULL AND (rd.flag = '0' OR rd.flag IS NULL) AND dv.uid = s.%(tbl_template_name)s)
        )
        SELECT uid AS %(tbl_name)s, %(reftbl_name)s AS %(reftbl_name)s FROM resolved_data WHERE (%(reftbl_name)s IS NOT NULL OR %(tbl_name)s IS NOT NULL);

        DROP TABLE template_data_version;
        DROP TABLE inner_template_data_version;        
    END IF;
END
$$
LANGUAGE plpgsql;

'''

#resolved changes between versions
    resolved_data_changes = '''CREATE FUNCTION %(tbl)s_resolved_changes_bv(from_version bigint, to_version bigint = 0)
RETURNS SETOF %(tbl)s
AS
$$
DECLARE
    changeset_id bigint;
BEGIN
    changeset_id = get_current_changeset_or_null();
    IF from_version = 0 AND changeset_id IS NULL THEN
        CREATE TEMP TABLE template_data_version AS SELECT * FROM production.%(tbl_template_name)s;
        CREATE TEMP TABLE inner_template_data_version AS SELECT * FROM %(tbl_template)s;
    ELSE
        CREATE TEMP TABLE template_data_version AS SELECT * FROM %(tbl_template_name)s_data_version(to_version);
        CREATE TEMP TABLE inner_template_data_version AS SELECT * FROM %(tbl_template)s_data_version(to_version);
    END IF;

    --for each object uid finds its last modification between versions from_version and to_version
    --joins it with history table of its kind to get object data in version to_version of all objects modified between from_version and to_version  
    RETURN QUERY WITH RECURSIVE resolved_data AS(
    SELECT h.uid, s.%(tbl_name)s, s.%(reftbl_name)s, h.%(template_column)s
    FROM %(tbl_name)s_changes_between_versions(from_version, to_version) h
    LEFT OUTER JOIN %(tbl)s_changes_between_versions(from_version, to_version) s ON (s.%(tbl_name)s = h.uid)
    WHERE s.flag = '1'
    
    UNION ALL
    
    SELECT rd.uid AS uid, rd.%(tbl_name)s, s.%(reftbl_name)s AS %(reftbl_name)s, dv.%(template_column)s AS %(template_column)s
    FROM template_data_version dv JOIN resolved_data rd ON (rd.%(template_column)s = dv.uid)
        LEFT OUTER JOIN inner_template_data_version s ON (rd.%(reftbl_name)s IS NULL AND flag = '1' AND dv.uid = s.%(tbl_template_name)s)
    )
    SELECT uid AS %(tbl_name)s, %(reftbl_name)s AS %(reftbl_name)s FROM resolved_data WHERE (%(reftbl_name)s IS NOT NULL OR %(tbl_name)s IS NOT NULL) AND %(template_column)s IS NULL;

    DROP TABLE template_data_version;
    DROP TABLE inner_template_data_version;
END
$$
LANGUAGE plpgsql;

'''

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
        JOIN (  SELECT uid, max(num) as maxnum
                FROM %(tbl)s_history h join version v on (v.id = h.version )
                WHERE v.num <= to_version and v.num > from_version
                GROUP BY uid) vmax1
        ON(h1.uid = vmax1.uid and v1.num = vmax1.maxnum);
END
$$
LANGUAGE plpgsql;

'''

    get_object_resolved_set = '''CREATE FUNCTION %(tbl)s_get_object_resolved_set(obj_uid bigint, data_version bigint = 0)
RETURNS SETOF bigint
AS
$$
DECLARE
    changeset_id bigint;
BEGIN
    changeset_id = get_current_changeset_or_null();
    IF data_version = 0 AND changeset_id IS NULL THEN
        RETURN QUERY SELECT %(reftbl_name)s FROM %(tbl)s WHERE %(tbl_name)s = obj_uid;
    ELSE
        CREATE TEMP TABLE template_data_version AS SELECT * FROM %(tbl_template_name)s_data_version(data_version);
        CREATE TEMP TABLE inner_template_data_version AS SELECT * FROM %(tbl_template)s_data_version(data_version);
  
        RETURN QUERY WITH RECURSIVE resolved_data AS(
        SELECT h.uid, s.%(tbl_name)s, s.%(reftbl_name)s, h.%(template_column)s, s.flag
        FROM %(tbl_name)s_data_version(data_version) h
        LEFT OUTER JOIN %(tbl)s_data_version(data_version) s ON (s.%(tbl_name)s = h.uid)
        WHERE h.uid = obj_uid
        
        UNION ALL
        
        SELECT rd.uid AS uid, rd.%(tbl_name)s, s.%(reftbl_name)s AS %(reftbl_name)s, dv.%(template_column)s AS %(template_column)s, s.flag
        FROM template_data_version dv JOIN resolved_data rd ON (rd.%(template_column)s = dv.uid)
            LEFT OUTER JOIN inner_template_data_version s ON (rd.%(reftbl_name)s IS NULL AND (rd.flag = '0' OR rd.flag IS NULL) AND dv.uid = s.%(tbl_template_name)s)
        )
        SELECT service AS %(reftbl_name)s FROM resolved_data WHERE flag = '1';

        DROP TABLE template_data_version;
        DROP TABLE inner_template_data_version;        
    END IF;
END
$$
LANGUAGE plpgsql;

'''

    commit_str = '''CREATE FUNCTION
%(tbl)s_commit()
RETURNS integer
AS
$$
DECLARE ver bigint;
    %(tbl_name)s_uid bigint;
BEGIN
    SELECT get_current_changeset() INTO ver;
    
    FOR %(tbl_name)s_uid IN SELECT DISTINCT %(tbl_name)s FROM %(tbl)s_history WHERE version = ver LOOP
        DELETE FROM %(tbl)s WHERE %(tbl_name)s = %(tbl_name)s_uid;
        INSERT INTO %(tbl)s (%(tbl_name)s, %(ref_tbl_name)s) SELECT %(tbl_name)s, %(ref_tbl_name)s FROM %(tbl)s_history WHERE version = ver AND %(tbl_name)s = %(tbl_name)s_uid AND flag = '1';
    END LOOP;
    
    RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;
'''

#tempalte for commit of templates of inner tables
#tbl=whole name of inner table, tbl_name=name of table that has identifier_set attribute, tbl_template=inner template table for which is fn made, template_column=column that references template table
    commit_template_str = '''CREATE OR REPLACE FUNCTION genproc.%(tbl_template)s_commit()
RETURNS integer 
AS
$$
DECLARE ver bigint;
    %(tbl_name)s_template_uid bigint;
    %(tbl_name)s_uid bigint;
    current_obj bigint;
BEGIN
    SELECT get_current_changeset() INTO ver;

    CREATE TEMP TABLE temp_%(tbl_template_name)s_data AS SELECT * FROM %(tbl_template_name)s_data_version();
    CREATE TEMP TABLE temp_%(tbl_name)s_data AS SELECT * FROM %(tbl_name)s_data_version();
    CREATE TEMP TABLE temp_inner_template_data AS SELECT * FROM %(tbl_template)s_data_version();
    CREATE TEMP TABLE temp_inner_data AS SELECT * FROM %(tbl)s_data_version();
    
--correction of templates' production   
--find all templates that were affected by some modification of template in current changeset
    CREATE TEMP TABLE affected_templates AS (
        WITH RECURSIVE resolved_data AS (
            (SELECT DISTINCT %(tbl_template_name)s as uid FROM %(tbl_template)s_history WHERE version = ver
            )
            UNION ALL
            SELECT dv.uid FROM temp_%(tbl_template_name)s_data dv, resolved_data rd WHERE dv.%(template_column)s = rd.uid
        )
        SELECT uid
        FROM resolved_data
    )
    UNION
    --newly created templates
    SELECT templ_history.uid FROM %(tbl_template_name)s_history templ_history LEFT OUTER JOIN %(tbl_template_name)s_data_version(id2num(parent(ver))) dv ON (templ_history.uid = dv.uid) WHERE templ_history.version = ver;

--delete from templated inner table which are templated by affected templates and has no own data
    DELETE FROM %(tbl_template)s WHERE %(tbl_template_name)s IN ( SELECT uid FROM affected_templates);

    INSERT INTO %(tbl_template)s (%(tbl_template_name)s, service)
        WITH RECURSIVE resolved_data AS(
            SELECT dv.uid AS uid, inner_templ.%(reftbl_name)s, inner_templ.flag, dv.%(template_column)s AS template
            FROM temp_%(tbl_template_name)s_data dv
            --join is faster then IN operator, we neeed to resolve only affected templates
                LEFT OUTER JOIN affected_templates affected ON (dv.uid = affected.uid)
            --we need to know with which template is object templated, we should join this data
                LEFT OUTER JOIN temp_inner_template_data inner_templ ON (inner_templ.%(tbl_template_name)s = dv.uid)
            WHERE affected.uid IS NOT NULL
                
            UNION ALL
            
            SELECT rd.uid AS uid, s.%(reftbl_name)s AS %(reftbl_name)s, s.flag, dv.%(template_column)s AS template
            FROM temp_%(tbl_template_name)s_data dv JOIN resolved_data rd ON (rd.template = dv.uid)
                LEFT OUTER JOIN temp_inner_template_data s ON (rd.%(reftbl_name)s IS NULL AND (rd.flag = '0' OR rd.flag IS NULL) AND dv.uid = s.%(tbl_template_name)s)
        )
        SELECT uid AS %(tbl_template_name)s, %(reftbl_name)s FROM resolved_data WHERE flag = '1';
    
    CREATE TEMP TABLE temp_affected_%(tbl_template_name)s_data AS
    WITH RECURSIVE resolved_data AS(
    SELECT tdata.%(tbl_template_name)s AS %(tbl_template_name)s, tdata.%(reftbl_name)s AS %(reftbl_name)s, tdata.flag, affected.uid AS template
    FROM affected_templates affected 
        LEFT OUTER JOIN temp_inner_template_data tdata ON (affected.uid = tdata.%(tbl_template_name)s)
    WHERE tdata.flag = '0' OR flag IS NULL
    
    UNION ALL
    
    SELECT rd.%(tbl_template_name)s AS %(tbl_template_name)s, s.%(reftbl_name)s AS %(reftbl_name)s, s.flag, dv.%(template_column)s AS template
    FROM temp_%(tbl_template_name)s_data dv JOIN resolved_data rd ON (rd.template = dv.uid)
        LEFT OUTER JOIN temp_inner_template_data s ON (rd.%(reftbl_name)s IS NULL AND s.flag = '1' AND dv.uid = s.%(tbl_template_name)s)
    )
    SELECT %(tbl_template_name)s AS %(tbl_template_name)s, %(reftbl_name)s AS %(reftbl_name)s, flag FROM resolved_data WHERE (%(reftbl_name)s IS NOT NULL OR %(tbl_template_name)s IS NOT NULL) AND template IS NULL;

    --correction of kind's production       
    --copy data with origin in inner table (not from template)

--objects that are templated by modified template and objects that where just added
    CREATE TEMP TABLE affected_objects AS (
        SELECT data.uid FROM temp_%(tbl_name)s_data data
            LEFT OUTER JOIN affected_templates templ ON (data.%(template_column)s = templ.uid)
            LEFT OUTER JOIN temp_inner_data idata ON (data.uid = idata.%(tbl_name)s)
        WHERE templ.uid IS NOT NULL AND idata.%(tbl_name)s IS NULL

        UNION

        SELECT h.uid FROM %(tbl_name)s_history h 
            LEFT OUTER JOIN (SELECT DISTINCT uid FROM %(tbl_name)s_history WHERE version = ver) inv ON (h.uid = inv.uid)
        WHERE inv.uid IS NOT NULL
        GROUP BY h.uid HAVING COUNT(*) = 1
    );

--delete all objects that has not its own data
    DELETE FROM %(tbl)s WHERE %(tbl_name)s IN ( SELECT affected.uid FROM affected_objects affected 
        LEFT OUTER JOIN temp_inner_data data ON (affected.uid = data.%(tbl_name)s) WHERE data.%(tbl_name)s IS NULL );

    --resolve inner data for all affected objects that has not its own data
    
    FOR %(tbl_name)s_uid IN (SELECT affected.uid FROM affected_objects affected 
        LEFT OUTER JOIN temp_inner_data data ON (affected.uid = data.%(tbl_name)s) WHERE data.%(tbl_name)s IS NULL) LOOP
        --templates are already resolved, we can use data from production.template
        SELECT %(template_column)s INTO %(tbl_name)s_template_uid FROM temp_%(tbl_name)s_data WHERE uid = %(tbl_name)s_uid;
        INSERT INTO %(tbl)s (%(tbl_name)s, %(reftbl_name)s)
                SELECT %(tbl_name)s_uid AS %(tbl_name)s, %(reftbl_name)s FROM %(tbl_template)s WHERE %(tbl_name)s_template = %(tbl_name)s_template_uid;
    END LOOP;

    DROP TABLE temp_%(tbl_name)s_template_data;
    DROP TABLE temp_%(tbl_name)s_data;
    DROP TABLE affected_templates;
    DROP TABLE affected_objects;
    DROP TABLE temp_inner_template_data;
    DROP TABLE temp_inner_data;
    DROP TABLE temp_affected_%(tbl_template_name)s_data;
    RETURN 1;
END
$$
LANGUAGE plpgsql;
 
'''

    diff_init_function_str = '''CREATE FUNCTION %(tbl)s_init_diff(from_version bigint, to_version bigint)
RETURNS void
AS
$$
BEGIN
    --full outer join of data in from_version and list of changes made between from_version and to_version
    CREATE TEMP TABLE %(tbl)s_diff_data
    AS SELECT inner1.%(tbl_name)s AS old_%(tbl_name)s, inner1.%(ref_tbl_name)s AS old_%(ref_tbl_name)s, inner1.flag AS old_flag, inner2.%(tbl_name)s AS new_%(tbl_name)s, inner2.%(ref_tbl_name)s AS new_%(ref_tbl_name)s, inner2.flag AS new_flag
    FROM %(tbl)s_data_version(from_version) inner1
    FULL OUTER JOIN %(tbl)s_data_version(to_version) inner2 ON (inner1.%(tbl_name)s = inner2.%(tbl_name)s AND inner1.%(ref_tbl_name)s = inner2.%(ref_tbl_name)s);
END
$$
LANGUAGE plpgsql;

'''

    diff_init_resolved_function_string = '''CREATE FUNCTION %(tbl)s_init_resolved_diff(from_version bigint, to_version bigint)
RETURNS void
AS
$$
BEGIN
    --full outer join of data in from_version and list of changes made between from_version and to_version
    CREATE TEMP TABLE %(tbl)s_diff_data
    AS SELECT inner1.%(tbl_name)s AS old_%(tbl_name)s, inner1.%(ref_tbl_name)s AS old_%(ref_tbl_name)s, inner1.flag AS old_flag, inner2.%(tbl_name)s AS new_%(tbl_name)s, inner2.%(ref_tbl_name)s AS new_%(ref_tbl_name)s, inner2.flag AS new_flag
    FROM %(tbl)s_resolved_data_version(from_version) inner1
    FULL OUTER JOIN %(tbl)s_resolved_data_version(to_version) inner2 ON (inner1.%(tbl_name)s = inner2.%(tbl_name)s AND inner1.%(ref_tbl_name)s = inner2.%(ref_tbl_name)s);
END
$$
LANGUAGE plpgsql;

'''



    diff_changeset_init_function_str = '''CREATE FUNCTION %(tbl)s_init_diff()
RETURNS void
AS
$$
DECLARE
    changeset_var bigint;
    from_version bigint;
BEGIN
    changeset_var = get_current_changeset();
    from_version = id2num(parent(changeset_var));
    CREATE TEMP TABLE %(tbl)s_diff_data
    AS  SELECT chv.%(tbl_name)s AS new_%(tbl_name)s, chv.%(ref_tbl_name)s AS new_%(ref_tbl_name)s, chv.flag AS new_flag, dv.%(tbl_name)s AS old_%(tbl_name)s, dv.%(ref_tbl_name)s AS old_%(ref_tbl_name)s, dv.flag AS old_flag
        FROM (SELECT * FROM %(tbl)s_history WHERE version = changeset_var) chv
            FULL OUTER JOIN %(tbl)s_data_version(from_version) dv ON (dv.%(tbl_name)s = chv.%(tbl_name)s AND dv.%(ref_tbl_name)s = chv.%(ref_tbl_name)s);
END
$$
LANGUAGE plpgsql;

'''

    diff_terminate_function_str = '''CREATE FUNCTION %(tbl)s_terminate_diff()
RETURNS void
AS
$$
BEGIN
    DROP TABLE %(tbl)s_diff_data;
END
$$
LANGUAGE plpgsql;

'''

    sets_equal_str = '''CREATE FUNCTION %(tbl)s_sets_equal(obj_uid bigint)
RETURNS boolean
AS
$$
DECLARE
    cnt int;
BEGIN
    SELECT COUNT(*) INTO cnt
    FROM %(tbl)s_diff_data
    WHERE (new_%(tbl_name)s = obj_uid OR old_%(tbl_name)s = obj_uid) AND (old_%(ref_tbl_name)s IS NULL OR new_%(ref_tbl_name)s IS NULL);
    IF cnt > 0 THEN
        RETURN false;
    ELSE
        RETURN true;
    END IF;
END;
$$
LANGUAGE plpgsql;

'''

    get_identifier_set = '''CREATE FUNCTION %(tbl)s_get_set(obj_uid bigint, from_version bigint = 0)
RETURNS text[]
AS
$$
DECLARE
    result text[];
BEGIN
--we need to get pure data (not reoslved) from version from_version, due to resolved data in production we need to get them from history table
    result = ARRAY(
        SELECT %(ref_tbl_name)s_get_name(%(ref_tbl_name)s) FROM %(tbl)s_data_version(from_version)
        WHERE %(tbl_name)s = obj_uid AND flag = '1'
    );
    RETURN deska.ret_id_set(result);
END;
$$
LANGUAGE plpgsql;
'''


    diff_get_old_set = '''CREATE FUNCTION %(tbl)s_get_old_set(obj_uid bigint)
RETURNS text[]
AS
$$
DECLARE
    changeset_id bigint;
BEGIN
    RETURN ARRAY(SELECT %(ref_tbl_name)s_get_name(old_%(ref_tbl_name)s) FROM %(tbl)s_diff_data WHERE old_%(tbl_name)s = obj_uid AND old_flag = '1');
END
$$
LANGUAGE plpgsql;

'''    

    diff_get_new_set = '''CREATE FUNCTION %(tbl)s_get_new_set(obj_uid bigint)
RETURNS text[]
AS
$$
DECLARE
    changeset_id bigint;
BEGIN
    RETURN ARRAY(SELECT %(ref_tbl_name)s_get_name(new_%(ref_tbl_name)s) FROM %(tbl)s_diff_data WHERE new_%(tbl_name)s = obj_uid AND new_flag = '1');
END
$$
LANGUAGE plpgsql;

'''    
    

    def __init__(self,db_connection):
        self.plpy = db_connection;
        self.plpy.execute("SET search_path TO deska, production, api")

    # generate sql for all tables
    def gen_multiRef(self,filename):
        name_split = filename.rsplit('/', 1)
        self.tab_sql = open(name_split[0] + '/' + 'tab_' + name_split[1],'w')
        self.fn_sql = open(name_split[0] + '/' + 'fn_' + name_split[1],'w')

        # print this to add proc into genproc schema
        self.tab_sql.write("SET search_path TO deska, production, history;\n")
        self.fn_sql.write("SET search_path TO genproc, deska, production, history;\n")

        record = self.plpy.execute(self.tbl_name_template_str)
        self.template_tables = dict()
        self.templated_tables = dict()
        for row in record:
            self.templated_tables[row[1]] = row[0]
            if row[0] != row[1]:
                self.template_tables[row[0]] = row[1]

        record = self.plpy.execute(self.multiRef_tables_str)
        for row in record:
            table = row[0]
            tab_relation_rec = self.plpy.execute(self.multiRef_info_str % {'tbl': table})
            if len(tab_relation_rec):
                reftable = tab_relation_rec[0][1]
                attname = tab_relation_rec[0][0]
                refattname = tab_relation_rec[0][2]
                self.check_multiRef_definition(table, reftable, attname, refattname)
                #we needs the oposite direction than the one that alredy exists
                self.gen_tables(table, reftable, attname)
                self.gen_functions(table, reftable, attname, refattname)

        self.tab_sql.close()
        self.fn_sql.close()

    def gen_inner_table_references(self, table, reftable, attname):
        return self.add_inner_table_str % {'tbl': table, 'ref_tbl': reftable, 'ref_col': attname}

    def check_multiRef_definition(self, table, reftable, attname, refattname):
        #attnames, refattnames should have only one item
        if len(attname.split(',')) > 1 or len(refattname.split(',')) > 1:
            raise ValueError, 'multiRef relation is badly defined, too many columns in relation'

        #refattname should be uid
        if refattname != 'uid':
            raise ValueError, 'multiRef relation is badly defined, referenced column should be uid column'
        
    def gen_inner_table_history(self, table):
        return table.gen_hist()

    def gen_tables(self, table, reftable, attname):
        self.tab_sql.write(self.gen_inner_table_references(table, reftable, attname))
        #inner table name is inner_tablename_idsetattname_multiref, where idset attname is name of column that references to set of identifiers
        join_tab = "inner_%(tbl)s_%(ref_col)s_multiRef" % {'tbl': table, 'ref_col': attname}
        self.tab_sql.write(self.hist_string % {'tbl': join_tab, 'tbl_name': table, 'ref_tbl_name': reftable, 'ref_col': attname})


    def gen_functions(self, table, reftable, attname, refattname):
        join_tab = "inner_%(tbl)s_%(ref_col)s_multiRef" % {'tbl' : table, 'ref_col' : attname}
        self.fn_sql.write(self.set_string % {'tbl': join_tab, 'tbl_name': table, 'ref_tbl_name': reftable, 'ref_col': attname})
        self.fn_sql.write(self.add_item_str % {'tbl': join_tab, 'tbl_name': table, 'ref_tbl_name': reftable, 'ref_col': attname})
        self.fn_sql.write(self.del_item_str % {'tbl': join_tab, 'tbl_name': table, 'ref_tbl_name': reftable, 'ref_col': attname})
        self.fn_sql.write(self.data_version_str % {'tbl': join_tab, 'tbl_name' : table})
        self.fn_sql.write(self.diff_init_function_str % {'tbl': join_tab, 'tbl_name' : table, 'ref_tbl_name': reftable})
        self.fn_sql.write(self.diff_changeset_init_function_str % {'tbl': join_tab, 'tbl_name' : table, 'ref_tbl_name': reftable})
        self.fn_sql.write(self.diff_terminate_function_str % {'tbl': join_tab, 'tbl_name' : table, 'ref_tbl_name': reftable})
        self.fn_sql.write(self.sets_equal_str % {'tbl': join_tab, 'tbl_name' : table, 'ref_tbl_name': reftable})
        self.fn_sql.write(self.get_identifier_set % {'tbl': join_tab, 'tbl_name' : table, 'ref_tbl_name': reftable})
        self.fn_sql.write(self.diff_get_old_set % {'tbl': join_tab, 'tbl_name' : table, 'ref_tbl_name': reftable})
        self.fn_sql.write(self.diff_get_new_set % {'tbl': join_tab, 'tbl_name' : table, 'ref_tbl_name': reftable})
        self.fn_sql.write(self.data_changes_function_string % {'tbl': join_tab})
        if table in self.template_tables:
            #this table is template of another table, join_base_tab is name of inner table that is templated
            base_table = self.template_tables[table]
            join_base_tab = "inner_%(tbl)s_%(ref_col)s_multiRef" % {'tbl' : base_table, 'ref_col' : attname}
            record = self.plpy.execute(self.template_column_str % {'tbl': table})
            if len(record) != 1:
                raise ValueError, 'template is badly defined'
            template_col = record[0][0]
            
            #table is template of another table
            #tbl=whole name of inner table, tbl_name=name of table that has identifier_set attribute, tbl_template=inner template table for which is fn made, template_column=column that references template table
            self.fn_sql.write(self.commit_template_str % {'tbl': join_base_tab, 'tbl_name' : base_table, 'tbl_template': join_tab, 'template_column': template_col, 'reftbl_name': reftable, 'tbl_template_name': table})
        else:
            self.fn_sql.write(self.commit_str % {'tbl': join_tab, 'tbl_name' : table, 'ref_tbl_name': reftable})
        
        
        if table in self.templated_tables:
            template_table = self.templated_tables[table]
            join_base_tab = "inner_%(tbl)s_%(ref_tbl)s_multiRef" % {'tbl' : table, 'ref_tbl' : reftable}
            join_template_tab = "inner_%(tbl)s_%(ref_tbl)s_multiRef" % {'tbl' : template_table, 'ref_tbl' : reftable}
            record = self.plpy.execute(self.template_column_str % {'tbl': table})
            if len(record) != 1:
                raise ValueError, 'template is badly defined'
            template_col = record[0][0]
            self.fn_sql.write(self.get_object_resolved_set % {'tbl': join_tab, 'tbl_name' : table, 'tbl_template_name': template_table, 'tbl_template': join_template_tab, 'template_column': template_col, 'reftbl_name': reftable})        
            self.fn_sql.write(self.diff_init_resolved_function_string % {'tbl': join_tab, 'tbl_name' : table, 'ref_tbl_name': reftable})
            self.fn_sql.write(self.resolved_data % {'tbl': join_tab, 'tbl_name' : table, 'tbl_template_name': template_table, 'tbl_template': join_template_tab, 'template_column': template_col, 'reftbl_name': reftable})
            self.fn_sql.write(self.resolved_data_changes % {'tbl': join_tab, 'tbl_name' : table, 'tbl_template_name': template_table, 'tbl_template': join_template_tab, 'template_column': template_col, 'reftbl_name': reftable})
        else:
            self.fn_sql.write(self.get_object_set_version % {'tbl': join_tab, 'tbl_name' : table, 'reftbl_name': reftable})   

