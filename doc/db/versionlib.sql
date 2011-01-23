-- switch to deska_dev SCHEMA
SET search_path TO deska,production;

DROP TABLE version;

-- vendors of hw -- versioning table
CREATE TABLE version (
	-- internal id
	id int
		CONSTRAINT version_pk PRIMARY KEY,
	-- human readable id
	number int
		CONSTRAINT version_number_unique UNIQUE,
	created timestamp  NOT NULL
);
GRANT ALL ON version TO deska_team;

--
-- functions
--

