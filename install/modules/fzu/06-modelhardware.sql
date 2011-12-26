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
	-- vendor of the hardware
	vendor bigint 
		CONSTRAINT modelhardware_fk_vendor REFERENCES vendor(uid) DEFERRABLE,
	-- The "type of box" is specific to the model of hardware, which is why it
	-- goes here
	modelbox bigint 
		CONSTRAINT modelhardware_fk_modelbox REFERENCES modelbox(uid) DEFERRABLE,
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
	-- number of cpu cores
	cpu_cores int
		CONSTRAINT "cpu_cores should be positive number"
		CHECK (cpu_cores > 0),
	-- is the HypoerThreading on?
	cpu_ht bool,
	-- performance of the box
	hepspec real,
	-- size of one disk drive
	-- FIXME: add a constraint saying "either hdd_drive_capacity and
	-- hdd_drive_count are both non-NULL and positive, or the hdd_note is
	-- not-NULL.  All of these two posibilities shall be allowed also at the
	-- same time.
	hdd_drive_capacity int
		CONSTRAINT "modelhardware hdd_drive_capacity should be positive number"
		CHECK (hdd_drive_capacity > 0),
	-- number of spindles in the machine
	hdd_drive_count int,
		CONSTRAINT "modelhardware hdd_drive_count should be positive number"
		CHECK (hdd_drive_count > 0),
	-- an unspecified note for the disk
	hdd_note text,

	-- maximal power consumption of a box, in Watts
	power_max int,
	-- how many power supplies does it have?
	-- note that it is completely acceptable to have zero here (like for blades)
	power_supply_count int,

	weight int
		CONSTRAINT "modelhardware weight shall be non-negative number"
		CHECK (weight >= 0),

	note text
);

CREATE INDEX idx_modelhw_vendor ON modelhardware(vendor);
CREATE INDEX idx_modelhw_modelbox ON modelhardware(modelbox);
