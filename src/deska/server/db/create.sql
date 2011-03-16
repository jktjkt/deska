begin;
set role deska_team;
-- create schemas
\i create_schemas.sql

-- create version table and function
\i fn-api-changeset.sql

-- create functions
\i fn-api-schema.sql
\i fn-constraints.sql

-- add modules
\i modules/vendor.sql
\i modules/hardware.sql
\i modules/host.sql
\i modules/interface.sql

-- add generated files from modules
--\i gen_schema.sql
--\i usage_scenario.sql

SET search_path TO genproc,history,deska,production;

end;
