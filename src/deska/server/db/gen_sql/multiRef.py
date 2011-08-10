class MultiRef:
    multiRef_str = "SELECT relname, refrelname, attnames, refattnames FROM get_dependency_info() WHERE conname LIKE 'rset_%';"
    coltype = "SELECT typename FROM kindAttributes('%(tbl)s') WHERE attname = '%(ref_tbl)s'"
    
    add_inner_table_str = '''CREATE TABLE deska.inner_%(tbl)s_%(ref_tbl)s_multiRef(
    %(tbl)s bigint
        CONSTRAINT inner_%(tbl)s_fk_%(ref_tbl)s REFERENCES %(tbl)s(uid) DEFERRABLE,
    %(ref_tbl)s bigint
        CONSTRAINT inner_%(ref_tbl)s_fk_%(tbl)s REFERENCES %(ref_tbl)s(uid) DEFERRABLE,
);
'''

    def __init__(self,db_connection):
        self.plpy = db_connection;
        self.plpy.execute("SET search_path TO deska, production, api")

    # generate sql for all tables
    def gen_multiRef(self,filename):
        self.sql = open(filename,'w')
        # print this to add proc into genproc schema
        self.sql.write("SET search_path TO production;\n")

        record = self.plpy.execute(self.multiRef_str)
        for row in record:
            table = row[0]
            reftable = row[1]
            attnames = row[2]
            refattnames = row[3]
            self.check_multiRef_definition(table, reftable, attnames, refattnames)
            #we needs the oposite direction than the one that alredy exists
            self.sql.write(self.gen_inner_table_references(table, reftable))

        self.sql.close()

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

