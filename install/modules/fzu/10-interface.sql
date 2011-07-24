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
	name identifier 
		CONSTRAINT "interface with this name already exists" NOT NULL,
	-- host
	-- TODO better use uid
	host bigint
		CONSTRAINT rembed_interface_fk_host REFERENCES host(uid) DEFERRABLE,
	dns_name text,
	-- IP
	-- TODO unique constraint
	ip4 ipv4,
	network bigint
		CONSTRAINT interface_fk_network REFERENCES network(uid) DEFERRABLE,
	--ip6 ipv6,
	-- MAC
	-- TODO unique constraint
	mac macaddr,
	switch bigint
		CONSTRAINT interface_fk_switch REFERENCES switch(uid) DEFERRABLE,
	switch_pos int
		CONSTRAINT "interface switch_pos should be positive number"
		CHECK (switch_pos > 0),
	note text,
	template bigint,
	CONSTRAINT "interface with this name already exists in this host" UNIQUE (name,host)
);

-- function for trigger, checking ports number and IPs
CREATE FUNCTION interface_check()
RETURNS TRIGGER
AS
$$
DECLARE net ipv4;
BEGIN
	SELECT ip4 INTO net FROM network WHERE uid = NEW.network;
	IF NEW.ip4 << net THEN
		RAISE EXCEPTION 'IPv4 %  is not valid in network %!', NEW.ip4, ip4;
	END IF;
	IF NEW.switch_pos > (SELECT ports FROM switch WHERE uid = NEW.switch) THEN
		RAISE EXCEPTION 'Switch does not have % ports!', NEW.switch_pos;
	END IF;
	RETURN NEW;
END
$$
LANGUAGE plpgsql;

CREATE TRIGGER switch_ports_check BEFORE INSERT OR UPDATE ON interface FOR EACH ROW
	EXECUTE PROCEDURE interface_check();

