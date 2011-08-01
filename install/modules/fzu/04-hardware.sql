--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE hardware_uid START 1;

-- vendors of hw
CREATE TABLE hardware (
	-- this column is required in all plugins
	uid bigint DEFAULT nextval('hardware_uid')
		CONSTRAINT hardware_pk PRIMARY KEY,
	-- this column is required in all plugins
	name identifier
		CONSTRAINT "hardware with this name already exists" UNIQUE NOT NULL,
	-- TODO - better use uid
	vendor bigint 
		CONSTRAINT hardware_fk_vendor REFERENCES vendor(uid) DEFERRABLE,
	purchase date NOT NULL,
	warranty date NOT NULL,
	-- GB of RAM
	ram int
		CONSTRAINT "hardware ram should be positive number"
		CHECK (ram > 0),
	-- number of CPU's
	cpu_num int
		CONSTRAINT "hardware cpu_num should be positive number"
		CHECK (cpu_num > 0),
	-- cpu type
	cpu_type bigint
		CONSTRAINT hardware_fk_cpu_type REFERENCES cpu_type(uid) DEFERRABLE,
	-- hdd size
	hdd_size int
		CONSTRAINT "hardware hdd_size should be positive number"
		CHECK (hdd_size > 0),
	hwbox bigint
		CONSTRAINT hardware_fk_hwbox REFERENCES hwbox(uid) DEFERRABLE,
	position int
		CONSTRAINT "hardware position in rack should be positive number"
		CHECK (hdd_size > 0),
	power int,
	note text,

	-- Serial number one, in vendor's preferred form
	serial_1 text,
	-- Serial number two, in vendor's preferred form
	serial_2 text,
	-- Warranty contract ID, in vendor's preferred form
	warranty_no text,
	-- Internal inventory number at FZU, format: dddddd-d
	inventory_no text,

	template bigint
);

-- function for trigger, checking position number
CREATE FUNCTION hardware_check()
RETURNS TRIGGER
AS
$$
BEGIN
        IF NEW.position > (SELECT positions FROM hwbox WHERE uid = NEW.hwbox) THEN
                RAISE EXCEPTION 'HWBox does not have % ports!', NEW.position;
        END IF;
        RETURN NEW;
END
$$
LANGUAGE plpgsql;

CREATE TRIGGER hardware_trigger BEFORE INSERT OR UPDATE ON hardware FOR EACH ROW
        EXECUTE PROCEDURE hardware_check()

