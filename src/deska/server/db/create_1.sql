BEGIN;
-- create schemas
\i create_schemas.sql

-- create version table and function
\i fn-api-changeset.sql

-- create functions
\i fn-api-schema.sql

--creates functions - string operations
\i fn-strlib.sql

GRANT ALL ON ALL TABLES IN SCHEMA deska TO deska_admin;
-- constraints functions
\i fn-constraints.sql

-- add modules
\i modules/vendor.sql
\i modules/hardware.sql
\i modules/host.sql
\i modules/interface.sql

GRANT ALL ON ALL TABLES IN SCHEMA production TO deska_admin;

GRANT ALL ON ALL TABLES IN SCHEMA api TO deska_admin;

-- run this as db root

-- creates functions - string operations
\i fn-strlib.sql

-- create revision functions
\i fn-revisions.sql

-- wrapper functions
\i fn-api-objects.sql
END;
