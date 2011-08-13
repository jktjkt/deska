class MultiRef:
    multiRef_tables_str = "SELECT relname FROM get_table_info() WHERE typname = 'identifier_set';"
    multiRef_info_str = "SELECT attname, refkind, refattname FROM kindRelations_full_info('%(tbl)s') WHERE relation = 'REFERS_TO_SET';"
    coltype = "SELECT typename FROM kindAttributes('%(tbl)s') WHERE attname = '%(ref_tbl)s'"
    
    add_inner_table_str = '''CREATE TABLE deska.inner_%(tbl)s_%(ref_tbl)s_multiRef(
%(tbl)s bigint
    CONSTRAINT inner_%(tbl)s_fk_%(ref_tbl)s REFERENCES %(tbl)s(uid) DEFERRABLE,
%(ref_tbl)s bigint
    CONSTRAINT inner_%(ref_tbl)s_fk_%(tbl)s REFERENCES %(ref_tbl)s(uid) DEFERRABLE,
CONSTRAINT inner_%(tbl)s_%(ref_tbl)s_multiRef_pk PRIMARY KEY (%(tbl)s,%(ref_tbl)s)
);

'''

    hist_string = '''CREATE TABLE history.%(tbl)s_history (
    LIKE %(tbl)s
    -- include default values
    INCLUDING DEFAULTS,
    version int NOT NULL,
    CONSTRAINT %(tbl)s_pk PRIMARY KEY (%(tbl_name)s, %(ref_tbl_name)s, version)
);
'''

    add_item_str = '''  CREATE FUNCTION
genproc.inner_%(tbl)s_%(ref_tbl)s_add_item(IN %(tbl)s_name text, IN %(ref_tbl)s_name text)
RETURNS integer
AS
$$
DECLARE ver bigint;
    %(tbl)s_uid bigint;
BEGIN
    SELECT get_current_changeset() INTO ver;
    %(tbl)s_uid = %(tbl)s_get_uid(%(tbl)s_name);
    IF NOT FOUND THEN
        RAISE 'No %(tbl)s with name %% exist in this version', %(tbl)s USING ERRCODE = '70021';
    END IF;
    
    INSERT INTO inner_%(tbl)s_%(ref_tbl)s_multiRef_history (%(tbl)s, %(ref_tbl)s, version)
        VALUES (%(tbl)s_name, %(ref_tbl)s_name, ver);
    
    --flag is_generated set to false
    UPDATE changeset SET is_generated = FALSE WHERE id = ver;
    RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

'''

    # template string for set functions for columns that reference set of identifiers
    set_string = '''CREATE FUNCTION
genproc.inner_%(tbl)s_set_%(colname)s(IN name_ text,IN value text[])
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
    SELECT %(tbl)s_get_uid(name_) INTO rowuid;
    --not found in case there is no object with name name_ in history
    IF NOT FOUND THEN
        RAISE 'No %(tbl)s named %%. Create it first.',name_ USING ERRCODE = '70021';
    END IF;
    
    DELETE FROM inner_%(tbl)s_%(ref_tbl)s_multiRef_history WHERE version = ver AND %(tbl)s = rowuid;
    FOR pos IN 1..array_upper(value,1) LOOP
        refuid = %(ref_tbl)s_get_uid(value[pos]);
        IF refuid IS NULL THEN
            RAISE 'No %(ref_tbl)s named %%. Create it first.',value[pos] USING ERRCODE = '70021';
        END IF;

        INSERT INTO inner_%(tbl)s_%(ref_tbl)s_multiRef_history (%(tbl)s,%(ref_tbl)s,version) VALUES (rowuid, refuid, ver);
    END LOOP;
    
    RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

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
        INSERT INTO %(tbl)s (%(tbl_name)s, %(ref_tbl_name)s) SELECT %(tbl_name)s, %(ref_tbl_name)s FROM %(tbl)s_history WHERE version = ver AND %(tbl_name)s = %(tbl_name)s_uid;
    END LOOP;
    
    RETURN 1;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;
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
        self.fn_sql.write("SET search_path TO genproc, production;\n")

        record = self.plpy.execute(self.multiRef_tables_str)
        for row in record:
            table = row[0]
            tab_relation_rec = self.plpy.execute(self.multiRef_info_str % {'tbl': table})
            reftable = tab_relation_rec[0][1]
            attnames = tab_relation_rec[0][0]
            refattnames = tab_relation_rec[0][2]
            self.check_multiRef_definition(table, reftable, attnames, refattnames)
            #we needs the oposite direction than the one that alredy exists
            self.gen_tables(table, reftable)
            self.gen_functions(table, reftable, attnames, refattnames)

        self.tab_sql.close()
        self.fn_sql.close()

    def gen_inner_table_references(self, table, reftable):
        return self.add_inner_table_str % {'tbl': table, 'ref_tbl': reftable}

    def check_multiRef_definition(self, table, reftable, attname, refattname):
        #attnames, refattnames should have only one item
        if len(attname.split(',')) > 1 or len(refattname.split(',')) > 1:
            raise ValueError, 'multiRef relation is badly defined, too many columns in relation'

        #attname is the same as reftable
        if attname != reftable:
            raise ValueError, 'multiRef relation is badly defined, name of referencing column should be the same as reftable'

        #refattname should be uid
        if refattname != 'uid':
            raise ValueError, 'multiRef relation is badly defined, referenced column should be uid column'
        
        record = self.plpy.execute(self.coltype % {'tbl' : table, 'ref_tbl' : reftable})
        if record[0][0] != 'identifier_set':
            raise ValueError, 'multiRef relation is badly defined, referencing column should be of identifier_set type'

    def gen_inner_table_history(self, table):
        return table.gen_hist()

    def gen_tables(self, table, reftable):
        self.tab_sql.write(self.gen_inner_table_references(table, reftable))
        join_tab = "inner_%(tbl)s_%(ref_tbl)s_multiRef" % {'tbl': table, 'ref_tbl': reftable}
        self.tab_sql.write(self.hist_string % {'tbl': join_tab, 'tbl_name': table, 'ref_tbl_name': reftable})


    def gen_functions(self, table, reftable, attname, refattname):
        self.fn_sql.write(self.set_string % {'tbl': table, 'ref_tbl': reftable, 'colname': reftable})
        self.fn_sql.write(self.add_item_str % {'tbl': table, 'ref_tbl': reftable})
        join_tab = "inner_%(tbl)s_%(ref_tbl)s_multiRef" % {'tbl' : table, 'ref_tbl' : reftable}
        self.fn_sql.write(self.commit_str % {'tbl': join_tab, 'tbl_name' : table, 'ref_tbl_name': reftable})
        
        