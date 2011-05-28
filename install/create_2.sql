-- continue after generete and check gen_schema.sql
-- FIXME: temporary turn of transaction, we need to split tables and functions
-- to remove this fix
--BEGIN;

--
-- schema for generated procedures
-- this is where generated things are placed
--
CREATE SCHEMA genproc AUTHORIZATION deska_admin;


--structures for diffs
\i diff.sql

\i gen_schema.sql
GRANT ALL ON ALL TABLES IN SCHEMA history TO deska_admin;
GRANT ALL ON ALL TABLES IN SCHEMA production TO deska_admin;

--END;

