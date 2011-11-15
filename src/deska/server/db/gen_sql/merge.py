class Composition:
    comp_pairs_query_str = "SELECT relname, refrelname, attnames, refattnames FROM get_dependency_info() WHERE conname LIKE 'rcnta_%';"
    kind_attributes_query_str = "SELECT attname FROM kindAttributes('%(tbl)s')"

    add_constraint_str = '''
ALTER TABLE %(tbl)s ADD CONSTRAINT rcble_%(tbl)s_%(comp_tbl)s FOREIGN KEY (%(comp_tbl)s) REFERENCES %(comp_tbl)s(uid) DEFERRABLE INITIALLY IMMEDIATE;
'''

    trigger_link_comp_objects = '''
CREATE OR REPLACE FUNCTION history.%(tbl)s_%(comp_tbl)s_link_before()
RETURNS trigger
AS
$$
DECLARE
    refuid bigint;
BEGIN
    BEGIN
        SELECT %(comp_tbl)s_get_uid(NEW.name) INTO refuid;
    EXCEPTION
        WHEN SQLSTATE '70021' THEN
            RETURN NEW;
    END;

    IF refuid IS NOT NULL THEN
        NEW.%(comp_tbl)s := refuid;
    END IF;    
    RETURN NEW;
END
$$
LANGUAGE plpgsql;
CREATE TRIGGER trg_before_%(tbl)s_%(comp_tbl)s_link BEFORE INSERT ON %(tbl)s_history FOR EACH ROW WHEN (NEW.%(comp_tbl)s IS NULL) EXECUTE PROCEDURE %(tbl)s_%(comp_tbl)s_link_before();

CREATE OR REPLACE FUNCTION history.%(tbl)s_%(comp_tbl)s_link_after()
RETURNS trigger
AS
$$
DECLARE
    refuid bigint;
BEGIN
    BEGIN
        SELECT %(comp_tbl)s_get_uid(NEW.name) INTO refuid;
    EXCEPTION
        WHEN SQLSTATE '70021' THEN
            RETURN NEW;
    END;

    IF refuid IS NOT NULL THEN
        PERFORM inner_%(comp_tbl)s_set_%(tbl)s(NEW.name, NEW.name);
        NEW.%(comp_tbl)s = refuid;
    END IF;
    RETURN NEW;
END
$$
LANGUAGE plpgsql;
CREATE TRIGGER trg_after_%(tbl)s_%(comp_tbl)s_link AFTER INSERT ON %(tbl)s_history FOR EACH ROW EXECUTE PROCEDURE %(tbl)s_%(comp_tbl)s_link_after();
'''

    before_update_trigger_comp_obj_part = '''
    refuid = NULL;
    BEGIN
        SELECT %(comp_tbl)s_get_uid(NEW.name) INTO refuid;
    EXCEPTION
        WHEN SQLSTATE '70021' THEN
            --nothing
    END;

    NEW.%(comp_tbl)s = refuid;
'''

    after_update_trigger_comp_obj_part = '''
    refuid = NULL;
    BEGIN
        SELECT %(comp_tbl)s_get_uid(NEW.name) INTO refuid;
    EXCEPTION
        WHEN SQLSTATE '70021' THEN
            --nothing
    END;

    IF OLD.%(comp_tbl)s IS NOT NULL THEN
        PERFORM inner_%(comp_tbl)s_set_%(tbl)s(OLD.name, NULL);
    END IF;

    IF refuid IS NOT NULL THEN
        PERFORM inner_%(comp_tbl)s_set_%(tbl)s(NEW.name, NEW.name);
    END IF;
'''

    rename_trigger_link_comp_objects = '''
CREATE OR REPLACE FUNCTION history.%(tbl)s_link_before_update()
RETURNS trigger
AS
$$
DECLARE 
    refuid bigint;
BEGIN
    %(before_comp_obj_parts)s
    RETURN NEW;
END 
$$
LANGUAGE plpgsql;
CREATE TRIGGER trg_before_update_%(tbl)s_link BEFORE UPDATE ON %(tbl)s_history FOR EACH ROW WHEN (OLD.name <> NEW.name) EXECUTE PROCEDURE %(tbl)s_link_before_update();

CREATE OR REPLACE FUNCTION history.%(tbl)s_link_after_update()
RETURNS trigger
AS
$$
DECLARE 
    refuid bigint;
BEGIN
    %(after_comp_obj_parts)s    
    RETURN NEW;
END 
$$
LANGUAGE plpgsql;
CREATE TRIGGER trg_after_update_%(tbl)s_link AFTER UPDATE ON %(tbl)s_history FOR EACH ROW WHEN (OLD.name <> NEW.name) EXECUTE PROCEDURE %(tbl)s_link_after_update();

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
    def gen_composition(self,filename):
        name_split = filename.rsplit('/', 1)
        self.constraint_sql = open(name_split[0] + '/' + 'rel_' + name_split[1],'w')
        self.trigger_sql = open(name_split[0] + '/' + 'trg_' + name_split[1],'w')

        # print this to add proc into genproc schema
        self.constraint_sql.write("SET search_path TO production;\n")
        self.trigger_sql.write("SET search_path TO production, history, genproc;\n")

        self.composition_touples = dict()
        self.cble_touples = dict()
        record = self.plpy.execute(self.comp_pairs_query_str)
        for row in record:
            table = row[0]
            reftable = row[1]
            attnames = row[2]
            refattnames = row[3]
            self.check_comp_definition(table, reftable, attnames, refattnames)
            #we needs the oposite direction than the one that alredy exists
            self.constraint_sql.write(self.gen_comp_reference(reftable, table))
            self.trigger_sql.write(self.gen_link_trigger(table, reftable))
            
            if table not in self.composition_touples:
                self.composition_touples[table] = list()
            self.composition_touples[table].append(reftable)
            
            if reftable not in self.cble_touples:
                self.cble_touples[reftable] = list()
            self.cble_touples[reftable].append(table)

        self.constraint_sql.write(self.gen_add_check_constraint())
        self.trigger_sql.write(self.gen_rename_trigger())

        self.constraint_sql.close()
        self.trigger_sql.close()

    def gen_comp_reference(self, table, reftable):
        return self.add_constraint_str % {'tbl': table, 'comp_tbl': reftable}

    def gen_link_trigger(self, table, reftable):
        return self.trigger_link_comp_objects % {'tbl': table, 'comp_tbl': reftable} + '\n' + self.trigger_link_comp_objects % {'tbl': reftable, 'comp_tbl': table}
    
    def gen_rename_trigger(self):
        triggers = ""
        for table in self.composition_touples:
            compos_cols = self.composition_touples[table]
            before_trg_obj_part = ""
            after_trg_obj_part = ""
            for reftbl in compos_cols:
                before_trg_obj_part = before_trg_obj_part + self.before_update_trigger_comp_obj_part % {'tbl': table, 'comp_tbl': reftbl}
                after_trg_obj_part = after_trg_obj_part + self.after_update_trigger_comp_obj_part % {'tbl': table, 'comp_tbl': reftbl}
            triggers = triggers + self.rename_trigger_link_comp_objects % {'tbl': table, 'comp_tbl': reftbl, 'before_comp_obj_parts' : before_trg_obj_part, 'after_comp_obj_parts' : after_trg_obj_part}
            
        for table in self.cble_touples:
            before_trg_obj_part = ""
            after_trg_obj_part = ""
            cble_cols = self.cble_touples[table]
            for reftbl in cble_cols:
                before_trg_obj_part = before_trg_obj_part + self.before_update_trigger_comp_obj_part % {'tbl': table, 'comp_tbl': reftbl}
                after_trg_obj_part = after_trg_obj_part + self.after_update_trigger_comp_obj_part % {'tbl': table, 'comp_tbl': reftbl}
            triggers = triggers + self.rename_trigger_link_comp_objects % {'tbl': table, 'comp_tbl': reftbl, 'before_comp_obj_parts' : before_trg_obj_part, 'after_comp_obj_parts' : after_trg_obj_part}

        return triggers
        

    #generates check constraint and function that is used in this check contraint
    #check consraint checks whether there is not more than one not null column (that is in composition relation with this kind) in row (object)
    def gen_add_check_constraint(self):
        check_function_str = ""
        add_constr_string = ""
        for table in self.composition_touples:
            compos_cols = self.composition_touples[table]
            if len(compos_cols) > 1:
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
                add_constr_string = add_constr_string + self.add_check_composition_constraint % {'tbl': table, 'composition_columns': compos_cols_str}
        return check_function_str + add_constr_string

    def check_comp_definition(self, table, reftable, attname, refattname):
        #attnames, refattnames should have only one item
        if len(attname.split(',')) > 1 or len(refattname.split(',')) > 1:
            raise ValueError, 'relation composition is badly defined, too many columns in relation'

        #attname is the same as reftable
        if attname != reftable:
            note = ' attname ' + attname + ' reftable ' + reftable
            raise ValueError, 'relation composition is badly defined, name of referencing column should be the same as reftable' + note

        #refattname should be uid
        if refattname != 'uid':
            raise ValueError, 'relation composition is badly defined, referenced column should be uid column'

        #sets with atributes of composed kinds should be disjoint
        kindattributes1 = set()
        record = self.plpy.execute(self.kind_attributes_query_str % {'tbl': table})
        for row in record:
            kindattributes1.add(row[0])

        kindattributes2 = set()
        record = self.plpy.execute(self.kind_attributes_query_str % {'tbl': reftable})
        for row in record:
            kindattributes2.add(row[0])

        if len(kindattributes1 & kindattributes2):
            raise ValueError, ('Composition relation between "%s" and "%s" is badly defined, column sets of composed types should '
                               'be disjoint (got %s and %s)') % (table, reftable, repr(kindattributes1), repr(kindattributes2))
