--
-- This is example of deska plug-in
--
-- 
--

-- vendors of hw
CREATE TABLE vendor (
	uid bigserial
		constraint vendor_pk PRIMARY KEY,
	-- this column is required in all plugins
	name text NOT NULL
);
GRANT ALL ON vendor TO deska_team;

