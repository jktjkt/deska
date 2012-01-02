SET search_path TO production,deska;

CREATE SEQUENCE interface_uid START 1;

-- network interfaces of host
CREATE TABLE interface (
	uid bigint DEFAULT nextval('interface_uid')
		CONSTRAINT interface_pk PRIMARY KEY,
	name identifier 
		CONSTRAINT "interface with this name already exists" NOT NULL,
	-- host
	host bigint
		CONSTRAINT rembed_interface_fk_host REFERENCES host(uid) DEFERRABLE,
	-- IP
	-- TODO unique constraint
	ip4 ipv4,
	network bigint
		CONSTRAINT rrefer_interface_fk_network REFERENCES network(uid) DEFERRABLE,
	ip6 ipv6,
	-- MAC address of an interface. It does *not* have to be unique.
	mac macaddr,
	switch bigint
		CONSTRAINT rrefer_interface_fk_switch REFERENCES switch(uid) DEFERRABLE,
	port text
		CONSTRAINT "interface port cannot be empty string"
		CHECK (port != ''),
	note text,
	CONSTRAINT "interface with this name already exists in this host" UNIQUE (name,host)
);

CREATE INDEX idx_interface_network ON interface(network);
CREATE INDEX idx_interface_switch ON interface(switch);

-- function for trigger, checking ports number and IPs
CREATE FUNCTION interface_check()
RETURNS TRIGGER
AS
$$
DECLARE net ipv4;
	ports_regexp text;
BEGIN
	SELECT set_masklen(ip4,cidr4) INTO net FROM network WHERE uid = NEW.network;
	IF NOT NEW.ip4 << net THEN
		RAISE EXCEPTION '% has ip % that is not valid in network %!', NEW.name, NEW.ip4, net;
	END IF;

	SELECT port_validity_regexp INTO ports_regexp FROM switch JOIN modelswitch ON (modelswitch.uid = switch.modelswitch) WHERE switch.uid = NEW.switch;
	IF NEW.port !~ ports_regexp THEN
		RAISE EXCEPTION 'Switch port % does not match port_validity_regexp "%"!', NEW.port, ports_regexp;
	END IF;
	RETURN NEW;
END
$$
LANGUAGE plpgsql;

CREATE TRIGGER switch_ports_check BEFORE INSERT OR UPDATE ON interface FOR EACH ROW
	EXECUTE PROCEDURE interface_check();

-- function checking ports number
CREATE FUNCTION ports_check()
RETURNS TRIGGER
AS
$$
DECLARE port text;
        ports_regexp text;
	interface bigint;
	modelswich text;
BEGIN
	--check if find not matching port in switch
        FOR port, ports_regexp, interface, modelswich IN
	SELECT interface.port, modelswitch.port_validity_regexp,interface.uid,modelswitch.name FROM interface
                JOIN switch ON (interface.switch = switch.uid)
                JOIN modelswitch ON (modelswitch.uid = switch.modelswitch)
                WHERE interface.port !~ modelswitch.port_validity_regexp
	LOOP
                RAISE EXCEPTION 'Switch port "%" of interface "%" does not match port_validity_regexp "%" of modelbox "%"!', port,interface_get_name(interface) ports_regexp, modelswich;
        END LOOP;
	RETURN NEW;
END
$$
LANGUAGE plpgsql;

-- check ports if modelswitch or switch changes
CREATE TRIGGER switch_ports_check_modelswitch AFTER INSERT OR UPDATE ON modelswitch FOR EACH ROW
	EXECUTE PROCEDURE ports_check();
CREATE TRIGGER switch_ports_check_switch AFTER INSERT OR UPDATE ON switch FOR EACH ROW
	EXECUTE PROCEDURE ports_check();

-- function checking ips in network
CREATE FUNCTION network_check()
RETURNS TRIGGER
AS
$$
DECLARE network text;
        ip ipv4;
        net ipv4;
        interface bigint;
BEGIN
	--check if find not matching ips in network
        FOR network, interface, ip, net IN
	SELECT network.name,interface.uid,interface.ip4,set_masklen(network.ip4,network.cidr4)  FROM interface
                JOIN network ON (interface.network = network.uid)
                WHERE NOT interface.ip4 << set_masklen(network.ip4,network.cidr4)
	LOOP
                RAISE EXCEPTION 'IPv4 "%" of interface "%" is not in network "%" (%)!', ip, interface_get_name(interface),network, net;
        END LOOP;
	RETURN NEW;
END
$$
LANGUAGE plpgsql;
-- check networks if network changes
--FIXME: uncomment after have valid data in fzu dump
--CREATE TRIGGER network_check_interface AFTER INSERT OR UPDATE ON network FOR EACH ROW
--	EXECUTE PROCEDURE network_check();
