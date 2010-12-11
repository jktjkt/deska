-- switch to deska_dev SCHEMA
SET search_path TO deska_dev,public;

DROP TABLE vendor_version;
DROP TABLE vendor;

--
-- functions
--
DROP FUNCTION vendor_add(IN id char(36), IN name text, IN ver int);
DROP FUNCTION vendor_commit(IN ver int);
