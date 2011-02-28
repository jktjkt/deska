--
-- This is example of deska plug-in
--
-- every module must be place in schema production
--
SET search_path TO production;

-- vendors of hw
CREATE TABLE vendor (
	uid bigint
		constraint vendor_pk PRIMARY KEY,
	-- this column is required in all plugins
	name text UNIQUE NOT NULL
);
GRANT ALL ON vendor TO deska_team;

