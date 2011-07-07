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
	dns_name text,
	-- IP
	-- TODO unique constraint
	ip4 ipv4,
	ip6 ipv6,
	-- MAC
	-- TODO unique constraint
	mac macaddr,
	switch bigint
		CONSTRAINT interface_fk_switch REFERENCES switch(uid) DEFERRABLE,
	switch_pos int
		CONSTRAINT interface_switch_pos_positive
		CHECK (switch_pos > 0),
	note text,
	template bigint,
	CONSTRAINT interface_pk_namexhost UNIQUE (name,host)
);

-- function for trigger, checking ports number
CREATE FUNCTION switch_check()
RETURNS TRIGGER
AS
$$
BEGIN
	IF NEW.switch_pos > (SELECT ports FROM switch WHERE uid = NEW.switch) THEN
		RAISE EXCEPTION 'Switch does not have % ports!', NEW.switch_pos;
	END IF;
	RETURN NEW;
END
$$
LANGUAGE plpgsql;

CREATE TRIGGER switch_ports_check BEFORE INSERT OR UPDATE ON interface FOR EACH ROW
EXECUTE PROCEDURE switch_check(uid,switch) 
