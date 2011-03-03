--
-- This is example of deska plug-in
--
-- every module must be place in schema production
--
SET search_path TO production;

CREATE SEQUENCE vendor_uid START 1;

-- vendors of hw
CREATE TABLE vendor (
	-- nextval can be used - will aplly to history table
	uid bigint default nextval('vendor_uid')
		constraint vendor_pk PRIMARY KEY,
	-- this column is required in all plugins
	name text NOT NULL
);
GRANT ALL ON vendor TO deska_team;

