-- continue after generete and check gen_schema.sql
BEGIN;

\i gen_schema.sql
GRANT ALL ON ALL TABLES IN SCHEMA history TO deska_admin;
GRANT ALL ON ALL TABLES IN SCHEMA production TO deska_admin;

-- create first empty initial revision 0
SET search_path TO api,deska,genproc,history,production;
SELECT startChangeset();
SELECT commitChangeset('Initial revision');

END;

