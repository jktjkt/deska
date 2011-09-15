SET search_path TO production,deska;

CREATE SEQUENCE network_uid START 1;

-- networks
CREATE TABLE network (
	uid bigint DEFAULT nextval('network_uid')
		CONSTRAINT network_pk PRIMARY KEY,
	name identifier 
		CONSTRAINT "network with this name already exists" NOT NULL,
	-- the lowest IPv4 address on the network
	ip4 ipv4,
    -- number of bits in the netmask for IPv4 addresses
    cidr4 int,
	-- the lowest IPv6 address on the network
	ip6 ipv6,
    -- number of bits in the netmask for IPv6 addresses
    cidr6 int,
    -- the DNS domain name of this network
    dnsdomain text,
	note text
);
