begin;
-- create schemas
\i create_schemas.sql

-- create version table and function
\i versionlib.sql

-- create functions
\i fn-get-schema.sql

-- add modules
\i modules/vendor.sql

-- add generated files from modules
--\i gen_schema.sql
--\i usage_scenario.sql

SET search_path TO genproc,history,deska,production;

end;
