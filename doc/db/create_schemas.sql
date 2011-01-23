-- what schemas we are using

DROP SCHEMA production CASCADE;
DROP SCHEMA history CASCADE;
DROP SCHEMA deska CASCADE;
--
-- schema for production tables
-- here place the modules
--
CREATE SCHEMA production AUTHORIZATION deska_team;

--
-- schema for historic tables and procedures
-- this is where generated things are placed
--
CREATE SCHEMA history AUTHORIZATION deska_team;

--
-- schema for other functions
-- here should be all hand writed functions, views, types etc.
--
CREATE SCHEMA deska AUTHORIZATION deska_team;
