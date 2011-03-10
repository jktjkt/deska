--
-- every module must be place in schema production
--
SET search_path TO production;

CREATE SEQUENCE interface_uid START 1;

-- interfaces of host
CREATE TABLE interface (
	-- this column is required in all plugins
	uid bigint DEFAULT nextval('interface_uid')
		CONSTRAINT interface_pk PRIMARY KEY,
	-- this column is required in all plugins
	name char(64) NOT NULL,
	-- host
	-- TODO better use uid
	host char(64)
		CONSTRAINT interface_fk_host REFERENCES host(name) DEFERRABLE,
	-- IP
	-- TODO unique constraint
	ip inet,
	-- MAC
	-- TODO unique constraint
	mac macaddr,
	note text,
	CONSTRAINT interface_pk_namexhost UNIQUE (name,host)
);


