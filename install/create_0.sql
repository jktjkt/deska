BEGIN;
-- create schemas
\i create_schemas_0.sql

-- create version table and function
\i fn-api-changeset.sql

-- create functions for locking changeset during generating configuration
\i fn-changeset_locks.sql

-- create functions
\i fn-api-schema.sql


--creates functions - string operations
\i fn-strlib.sql

-- constraints functions
\i fn-constraints.sql

-- creates domains - deska data types
\i deska-types.sql

-- create revision functions
\i fn-revisions.sql

-- wrapper functions
\i fn-api-objects.sql

-- json functions
\i jsonObjects.sql
\i jsonVersioning.sql
\i jsonFunctions.sql
\i jsonData.sql
\i jsonMultipleData.sql

\i fn-refs-set.sql


-- user can use api fucntions
GRANT USAGE ON SCHEMA jsn TO deska_user;

-- use deska functions
GRANT USAGE ON SCHEMA deska TO deska_admin;
GRANT USAGE ON SCHEMA test TO deska_admin;
-- have rigths to all tables
GRANT ALL ON ALL TABLES IN SCHEMA deska TO deska_admin;
GRANT ALL ON ALL TABLES IN SCHEMA api TO deska_admin;
GRANT ALL ON ALL TABLES IN SCHEMA versioning TO deska_admin;

-- The restoringCommit DBAPI method really needs access to this one
GRANT USAGE ON SCHEMA api TO deska_admin;

END;
