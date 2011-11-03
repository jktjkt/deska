class Merge:
    merge_pairs_query_str = "SELECT relname, refrelname, attnames, refattnames FROM get_dependency_info() WHERE conname LIKE 'rmerge_%';"
    kind_attributes_query_str = "SELECT attname FROM kindAttributes('%(tbl)s')"

    add_constraint_str = '''
ALTER TABLE %(tbl)s ADD CONSTRAINT rrmerge_%(tbl)s_%(merge_tbl)s FOREIGN KEY (%(merge_tbl)s) REFERENCES %(merge_tbl)s(uid) DEFERRABLE INITIALLY IMMEDIATE;
'''

    trigger_link_merged_objects = '''
CREATE OR REPLACE FUNCTION history.%(tbl)s_%(merge_tbl)s_link_before()
RETURNS trigger
AS
$$
DECLARE
    refuid bigint;
BEGIN
    BEGIN
        SELECT %(merge_tbl)s_get_uid(NEW.name) INTO refuid;
    EXCEPTION
        WHEN SQLSTATE '70021' THEN
            RETURN NEW;
    END;

    IF refuid IS NOT NULL THEN
        NEW.%(merge_tbl)s := refuid;
    END IF;
    RETURN NEW;
END
$$
LANGUAGE plpgsql;
CREATE TRIGGER trg_before_%(tbl)s_%(merge_tbl)s_link BEFORE INSERT ON %(tbl)s_history FOR EACH ROW EXECUTE PROCEDURE %(tbl)s_%(merge_tbl)s_link_before();

CREATE OR REPLACE FUNCTION history.%(tbl)s_%(merge_tbl)s_link_after()
RETURNS trigger
AS
$$
DECLARE
    refuid bigint;
BEGIN
    BEGIN
        SELECT %(merge_tbl)s_get_uid(NEW.name) INTO refuid;
    EXCEPTION
        WHEN SQLSTATE '70021' THEN
            RETURN NEW;
    END;

    IF refuid IS NOT NULL THEN
        PERFORM %(merge_tbl)s_set_%(tbl)s(NEW.name, NEW.name);
    END IF;
    RETURN NEW;
END
$$
LANGUAGE plpgsql;
CREATE TRIGGER trg_after_%(tbl)s_%(merge_tbl)s_link AFTER INSERT ON %(tbl)s_history FOR EACH ROW EXECUTE PROCEDURE %(tbl)s_%(merge_tbl)s_link_after();
'''

    nn_inc_count_str = '''
    IF %(column)s IS NOT NULL THEN
        count =  count + 1;
    END IF;
'''

    check_composition_function_str = '''CREATE FUNCTION %(tbl)s_composed_cols_count(%(composition_columns_params)s)
RETURNS int
AS
$$
DECLARE
    count int;
BEGIN
    count = 0;
    --includes list of ifs with columns which are in composition relation with this kind
    --if column is not null count is increased
    %(inc_count_if_col_nn)s
    
    RETURN count;
END
$$
LANGUAGE plpgsql;
'''

    add_check_composition_constraint = '''ALTER TABLE %(tbl)s ADD CONSTRAINT inner_check_%(tbl)s_composition CHECK (%(tbl)s_composed_cols_count(%(composition_columns)s) <= 1);
'''

    def __init__(self,db_connection):
        self.plpy = db_connection;
        self.plpy.execute("SET search_path TO deska, production, api")

    # generate sql for all tables
    def gen_merge(self,filename):
        name_split = filename.rsplit('/', 1)
        self.constraint_sql = open(name_split[0] + '/' + 'rel_' + name_split[1],'w')
        self.trigger_sql = open(name_split[0] + '/' + 'trg_' + name_split[1],'w')

        # print this to add proc into genproc schema
        self.constraint_sql.write("SET search_path TO production;\n")
        self.trigger_sql.write("SET search_path TO production, history, genproc;\n")

        self.composition_touples = dict()
        record = self.plpy.execute(self.merge_pairs_query_str)
        for row in record:
            table = row[0]
            reftable = row[1]
            attnames = row[2]
            refattnames = row[3]
            self.check_merge_definition(table, reftable, attnames, refattnames)
            #we needs the oposite direction than the one that alredy exists
            self.constraint_sql.write(self.gen_merge_reference(reftable, table))
            self.trigger_sql.write(self.gen_link_trigger(table, reftable))
            
            if table not in self.composition_touples:
                self.composition_touples[table] = list()
            self.composition_touples[table].append(reftable)
            
        self.constraint_sql.write(self.gen_add_check_constraint())

        self.constraint_sql.close()
        self.trigger_sql.close()

    def gen_merge_reference(self, table, reftable):
        return self.add_constraint_str % {'tbl': table, 'merge_tbl': reftable}

    def gen_link_trigger(self, table, reftable):
        return self.trigger_link_merged_objects % {'tbl': table, 'merge_tbl': reftable} + '\n' + self.trigger_link_merged_objects % {'tbl': reftable, 'merge_tbl': table}

    #generates check constraint and function that is used in this check contraint
    #check consraint checks whether there is not more than one not null column (that is in composition relation with this kind) in row (object)
    def gen_add_check_constraint(self):
        check_function_str = ""
        add_constr_string = ""
        for table in self.composition_touples:
            compos_cols = self.composition_touples[table]
            nn_col_str = ""
            #list of columns in composition relation with table and their type .. bigint
            compos_cols_types_list = list()
            for col in compos_cols:
                nn_col_str = nn_col_str + self.nn_inc_count_str % {'column': col}
                compos_cols_types_list.append("%(column)s bigint" % {'column': col})
            
            #is used for parameters in function to check them to be no more one of them not null
            compos_cols_types_str = ",".join(compos_cols_types_list)
            compos_cols_str = ",".join(compos_cols)
            check_function_str = check_function_str + self.check_composition_function_str % {'tbl': table, 'composition_columns_params': compos_cols_types_str, 'inc_count_if_col_nn': nn_col_str}
            add_constr_string = self.add_check_composition_constraint % {'tbl': table, 'composition_columns': compos_cols_str}
        return check_function_str + add_constr_string

    def check_merge_definition(self, table, reftable, attname, refattname):
        #attnames, refattnames should have only one item
        if len(attname.split(',')) > 1 or len(refattname.split(',')) > 1:
            raise ValueError, 'merge relation is badly defined, too many columns in relation'

        #attname is the same as reftable
        if attname != reftable:
            note = ' attname ' + attname + ' reftable ' + reftable
            raise ValueError, 'merge relation is badly defined, name of referencing column should be the same as reftable' + note

        #refattname should be uid
        if refattname != 'uid':
            raise ValueError, 'merge relation is badly defined, referenced column should be uid column'

        #sets with atributes of merged kinds should be disjoint
        kindattributes1 = set()
        record = self.plpy.execute(self.kind_attributes_query_str % {'tbl': table})
        for row in record:
            kindattributes1.add(row[0])

        kindattributes2 = set()
        record = self.plpy.execute(self.kind_attributes_query_str % {'tbl': reftable})
        for row in record:
            kindattributes2.add(row[0])

        if len(kindattributes1 & kindattributes2):
            raise ValueError, ('Merge relation between "%s" and "%s" is badly defined, column sets of merged types should '
                               'be disjoint (got %s and %s)') % (table, reftable, repr(kindattributes1), repr(kindattributes2))
