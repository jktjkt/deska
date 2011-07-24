--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE network_uid START 1;

-- networks
CREATE TABLE network (
	-- this column is required in all plugins
	uid bigint DEFAULT nextval('network_uid')
		CONSTRAINT network_pk PRIMARY KEY,
	-- this column is required in all plugins
	name identifier 
		CONSTRAINT "network with this name already exists" NOT NULL,
	-- IP
	ip4 ipv4,
	--ip6 ipv6,
	note text
);
