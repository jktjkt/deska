-- what schemas we are using

--
-- schema for production tables
-- here place the modules
--
CREATE SCHEMA production AUTHORIZATION deska_admin;

--
-- schema for history tables
--
CREATE SCHEMA history AUTHORIZATION deska_admin;

--
-- schema for generated procedures
-- this is where generated things are placed
--
CREATE SCHEMA genproc AUTHORIZATION deska_admin;

--
-- schema for other functions
-- here should be all hand writed functions, views, types etc.
--
CREATE SCHEMA deska AUTHORIZATION deska_admin;

--
-- schema for api functions
--
CREATE SCHEMA api AUTHORIZATION deska_team;
