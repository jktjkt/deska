--
-- every module must be place in schema production
--
SET search_path TO production;

-- interfaces of host
CREATE TABLE interface (
	-- this column is required in all plugins
	uid bigint
		constraint interface_pk PRIMARY KEY,
	-- this column is required in all plugins
	name text
		CONSTRAINT interface_name_unique UNIQUE NOT NULL,
	-- host
	host bigint
		CONSTRAINT interface_fk_host REFERENCES host(uid),
	-- IP
	-- TODO unique constraint
	ip inet,
	-- MAC
	-- TODO unique constraint
	mac macaddr,
	note text
);


