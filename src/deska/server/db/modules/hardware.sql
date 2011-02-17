--
-- every module must be place in schema production
--
SET search_path TO production;

-- vendors of hw
CREATE TABLE hardware (
	-- this column is required in all plugins
	uid bigint
		constraint hardware_pk PRIMARY KEY,
	-- this column is required in all plugins
	name text
		CONSTRAINT hardware_name_unique UNIQUE NOT NULL,
	vendor bigint
		REFERENCES vendor(uid)

);
GRANT ALL ON hardware TO deska_team;

