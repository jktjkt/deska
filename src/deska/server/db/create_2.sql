-- continue after generete and check gen_schema.sql

\i gen_schema.sql
GRANT ALL ON ALL TABLES IN SCHEMA history TO deska_admin;

-- grant user api
GRANT USAGE ON SCHEMA api To deska_user;

