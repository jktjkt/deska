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
BEGIN
	--check if find not matching port in switch
        FOR port, ports_regexp IN SELECT interface.port, modelswitch.port_validity_regexp FROM interface
                JOIN switch ON (interface.switch = switch.uid)
                JOIN modelswitch ON (modelswitch.uid = switch.modelswitch)
                WHERE interface.port !~ modelswitch.port_validity_regexp
	LOOP
                RAISE EXCEPTION 'Switch port % does not match port_validity_regexp "%"!', port, ports_regexp;
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
