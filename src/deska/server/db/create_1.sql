BEGIN;
-- create schemas
\i create_schemas_1.sql

-- create modules
\i modules/vendor.sql
\i modules/hardware.sql
\i modules/host.sql
\i modules/interface.sql

END;
