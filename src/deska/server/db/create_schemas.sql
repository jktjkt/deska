-- what schemas we are using

DROP SCHEMA production CASCADE;
DROP SCHEMA history CASCADE;
DROP SCHEMA genproc CASCADE;
DROP SCHEMA deska CASCADE;
--
-- schema for production tables
-- here place the modules
--
CREATE SCHEMA production AUTHORIZATION deska_team;

--
-- schema for history tables
--
CREATE SCHEMA history AUTHORIZATION deska_team;

--
-- schema for generated procedures
-- this is where generated things are placed
--
CREATE SCHEMA genproc AUTHORIZATION deska_team;

--
-- schema for other functions
-- here should be all hand writed functions, views, types etc.
--
CREATE SCHEMA deska AUTHORIZATION deska_team;
