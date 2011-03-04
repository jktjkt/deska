--
-- every module must be place in schema production
--
SET search_path TO production;

CREATE SEQUENCE interface_uid START 1;

-- interfaces of host
CREATE TABLE interface (
	-- this column is required in all plugins
	uid bigint default nextval('interface_uid')
		constraint interface_pk PRIMARY KEY,
	-- this column is required in all plugins
	name text NOT NULL,
	-- host
	host bigint
		CONSTRAINT interface_fk_host REFERENCES host(uid),
	-- IP
	-- TODO unique constraint
	ip inet,
	-- MAC
	-- TODO unique constraint
	mac macaddr,
	note text,
	CONSTRAINT interface_pk_namexhost UNIQUE (name,host)
);


