BEGIN;
-- create schemas
\i create_schemas_0.sql

-- create version table and function
\i fn-api-changeset.sql

-- create functions
\i fn-api-schema.sql

--creates functions - string operations
\i fn-strlib.sql

GRANT ALL ON ALL TABLES IN SCHEMA deska TO deska_admin;
-- constraints functions
\i fn-constraints.sql

GRANT ALL ON ALL TABLES IN SCHEMA api TO deska_admin;

-- creates functions - string operations
\i fn-strlib.sql

-- create revision functions
\i fn-revisions.sql

-- wrapper functions
\i fn-api-objects.sql
END;
