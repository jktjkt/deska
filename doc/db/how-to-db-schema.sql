--
-- just notes how to get db schema
--

-- tables in our schema
select * from pg_tables where schemaname = 'deska_dev';

-- more info for one table, see column oid
select * from pg_class where relname = 'vendor';

-- attributes for the table vendor - oid
-- see the attnum column - user attributes(=columns) are the positive
select * from pg_attribute where attrelid = 24576;

-- all foreign keys
select * from pg_constraint where contype='f';

-- one constraint ...
select * from pg_constraint where conname='cpu_type_fkey' ;

-- table and column for this columns
select * from pg_attribute where attrelid = 24583 and attnum = 2

--
-- so now we only need to join it
-- this is only to see what we can get from the db ...
--
