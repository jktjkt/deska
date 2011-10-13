--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE modelhardware_uid START 1;

-- models of hardware
CREATE TABLE modelhardware (
	-- this column is required in all plugins
	uid bigint DEFAULT nextval('modelhardware_uid')
		CONSTRAINT modelhardware_pk PRIMARY KEY,
	-- this column is required in all plugins
	name identifier
		CONSTRAINT "modelhardware with this name already exists" UNIQUE NOT NULL,
	-- TODO - better use uid
	vendor bigint 
		CONSTRAINT modelhardware_fk_vendor REFERENCES vendor(uid) DEFERRABLE,
	-- MB of RAM
	ram int
		CONSTRAINT "modelhardware ram should be positive number"
		CHECK (ram > 0),
	-- number of CPU's
	cpu_sockets int
		CONSTRAINT "modelhardware cpu_sockets should be positive number"
		CHECK (cpu_sockets > 0),
	-- cpu type
	cpu_type text,
	cpu_physicalcores int
		CONSTRAINT "cpu_physicalcores should be positive number"
		CHECK (cpu_physicalcores > 0),
	cpu_ht bool,
    -- performance of the box
	hepspec real,
	-- size of one disk drive
	hdd_drive_capacity int
		CONSTRAINT "modelhardware hdd_drive_capacity should be positive number"
		CHECK (hdd_drive_capacity > 0),
    -- number of spindles in the machine
    hdd_drive_count int,
    -- an unspecified note for the disk
    hdd_note text,

    -- maximal power consumption of a box
	power_max int,
    -- how many power supplies does it have?
    -- note that it is completely acceptable to have zero here (like for blades)
    power_supply_count int,

	note text
);

