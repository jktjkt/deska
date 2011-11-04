--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE host_uid START 1;

-- vendors of hw
CREATE TABLE host (
	-- this column is required in all plugins
	uid bigint DEFAULT nextval('host_uid')
		CONSTRAINT host_pk PRIMARY KEY,
	-- this column is required in all plugins
	name identifier
		CONSTRAINT "host with this name already exists" UNIQUE NOT NULL,
	-- hardware where it runs
	hardware bigint
		-- merge / contains
		--CONSTRAINT rmerge_host_fk_hardware REFERENCES hardware(uid) DEFERRABLE,
		CONSTRAINT host_fk_hardware REFERENCES hardware(uid) DEFERRABLE,
	virtual_hardware bigint
		-- merge / contains
		--CONSTRAINT rmerge_host_fk_virtual_hardware REFERENCES virtual_hardware(uid) DEFERRABLE,
		CONSTRAINT host_fk_virtual_hardware REFERENCES virtual_hardware(uid) DEFERRABLE,
	service identifier_set
		CONSTRAINT rset_host_fk_service REFERENCES service(uid) DEFERRABLE,
	note text
);

-- function for trigger, checking hardware/virtual_hardware consistency
CREATE FUNCTION host_runs_on_hw()
RETURNS TRIGGER
AS
$$
BEGIN
        IF NEW.hardware IS NULL AND NEW.virtual_hardware IS NULL THEN
                RAISE EXCEPTION 'Host % must run on either hardware or virtual_hardware', NEW.name;
        END IF;
        IF NEW.hardware IS NOT NULL AND NEW.virtual_hardware IS NOT NULL THEN
                RAISE EXCEPTION 'Host % cannot run on both hardware and virtual_hardware', NEW.name;
        END IF;
        RETURN NEW;
END
$$
LANGUAGE plpgsql;

CREATE TRIGGER host_hardware_check BEFORE INSERT OR UPDATE ON host FOR EACH ROW
EXECUTE PROCEDURE host_runs_on_hw()
