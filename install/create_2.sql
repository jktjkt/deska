-- continue after generete and check gen_schema.sql
BEGIN;
--structures for diffs
\i diff.sql

\i fn_gen_schema.sql
GRANT ALL ON ALL TABLES IN SCHEMA history TO deska_admin;
GRANT ALL ON ALL TABLES IN SCHEMA production TO deska_admin;
GRANT ALL ON ALL TABLES IN SCHEMA genproc TO deska_admin;

END;

