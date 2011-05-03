BEGIN;
-- create schemas
\i create_schemas_0.sql

-- create version table and function
\i fn-api-changeset.sql

-- create functions
\i fn-api-schema.sql

--creates functions - string operations
\i fn-strlib.sql

-- constraints functions
\i fn-constraints.sql

-- creates functions - string operations
\i fn-strlib.sql

-- create revision functions
\i fn-revisions.sql

-- wrapper functions
\i fn-api-objects.sql

-- user can use api fucntions
GRANT USAGE ON SCHEMA api TO deska_user;

-- admin can create schemas
GRANT CREATE ON DATABASE :dbname TO deska_admin;
-- use deska functions
GRANT USAGE ON SCHEMA deska TO deska_admin;
-- have rigths to all tables
GRANT ALL ON ALL TABLES IN SCHEMA deska TO deska_admin;
GRANT ALL ON ALL TABLES IN SCHEMA api TO deska_admin;
GRANT ALL ON ALL TABLES IN SCHEMA versioning TO deska_admin;

END;
