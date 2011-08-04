class Merge:
    merge_pairs_query_str = "SELECT relname, refrelname, attnames, refattnames FROM get_dependency_info() WHERE conname LIKE 'rmerge_%';"
    kind_attributes_query_str = "SELECT attname FROM kindAttributes('%(tbl)s')"
    
    add_constraint_str = '''
ALTER TABLE %(tbl)s ADD CONSTRAINT rmerge_%(tbl)s_%(merge_tbl)s FOREIGN KEY (%(merge_tbl)s) REFERENCES %(merge_tbl)s(uid) DEFERRABLE INITIALLY IMMEDIATE;
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

    def __init__(self,db_connection):
        self.plpy = db_connection;
        self.plpy.execute("SET search_path TO deska, production, api")                    

    # generate sql for all tables
    def gen_merge(self,filename):
        name_split = filename.rsplit('/', 1)
        self.constrain_sql = open(name_split[0] + '/' + 'rel_' + name_split[1],'w')
        self.trigger_sql = open(name_split[0] + '/' + 'trg_' + name_split[1],'w')

        # print this to add proc into genproc schema
        self.constrain_sql.write("SET search_path TO production;\n")
        self.trigger_sql.write("SET search_path TO production, history, genproc;\n")
        
        record = self.plpy.execute(self.merge_pairs_query_str)
        for row in record:
            table = row[0]
            reftable = row[1]
            attnames = row[2]
            refattnames = row[3]
            self.check_merge_definition(table, reftable, attnames, refattnames)
            #we needs the oposite direction than the one that alredy exists
            self.constrain_sql.write(self.gen_merge_reference(reftable, table))
            self.trigger_sql.write(self.gen_link_trigger(table, reftable))
        
        self.constrain_sql.close()
        self.trigger_sql.close()
    
    def gen_merge_reference(self, table, reftable):
        return self.add_constraint_str % {'tbl': table, 'merge_tbl': reftable}
    
    def gen_link_trigger(self, table, reftable):
        return self.trigger_link_merged_objects % {'tbl': table, 'merge_tbl': reftable} + '\n' + self.trigger_link_merged_objects % {'tbl': reftable, 'merge_tbl': table}
        
 
    def check_merge_definition(self, table, reftable, attname, refattname):
        #attnames, refattnames should have only one item
        if len(attname.split(',')) > 1 or len(refattname.split(',')) > 1:
            raise ValueError, 'merge relation is badly defined, too many columns in relation'

        #attname is the same as reftable
        if attname != reftable:
            raise ValueError, 'merge relation is badly defined, name of referencing column should be the same as reftable'

        #refattname should be uid
        if refattname != 'uid':
            raise ValueError, 'merge relation is badly defined, referenced column should be uid column'

        #sets with atributes of merged kinds should be disjoint
        kindattributes1 = set()
        record = self.plpy.execute(self.kind_attributes_query_str % {'tbl': table})
        for row in record:
            kindattributes1.add(row[0])

        kindattributes2 = set()
        for row in record:
            kindattributes2.add(row[0])

        if not kindattributes1.isdisjoint(kindattributes2):
            raise ValueError, 'merge relation is badly defined, column sets of merged types should be disjoint'



