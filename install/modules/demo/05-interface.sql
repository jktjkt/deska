--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE interface_uid START 1;

-- interfaces of host
CREATE TABLE interface (
	-- this column is required in all plugins
	uid bigint DEFAULT nextval('interface_uid')
		CONSTRAINT interface_pk PRIMARY KEY,
	-- this column is required in all plugins
	name identifier NOT NULL,
	-- host
	-- TODO better use uid
	host bigint
		CONSTRAINT rembed_interface_fk_host REFERENCES host(uid) DEFERRABLE,
	-- IP
	-- TODO unique constraint
	ip4 ipv4,
	ip6 ipv6,
	-- MAC
	-- TODO unique constraint
	mac macaddr,
	note text,
	template bigint,
	CONSTRAINT interface_pk_namexhost UNIQUE (name,host)
);