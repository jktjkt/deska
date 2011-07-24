--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE virtual_hardware_uid START 1;

-- virtual hardware
CREATE TABLE virtual_hardware (
	-- this column is required in all plugins
	uid bigint DEFAULT nextval('virtual_hardware_uid')
		CONSTRAINT virtual_hardware_pk PRIMARY KEY,
	-- this column is required in all plugins
	name identifier
		CONSTRAINT "virtual_hardware with this name already exists" UNIQUE NOT NULL,
	-- TODO - make this sense?
	vendor bigint 
		CONSTRAINT hardware_fk_vendor REFERENCES vendor(uid) DEFERRABLE,
	-- GB of RAM
	ram int
		CONSTRAINT "virtual_hardware ram should be positive number"
		CHECK (ram > 0),
	-- number of CPU's
	cpu_num int
		CONSTRAINT "virtual_hardware cpu_num should be positive number"
		CHECK (cpu_num > 0),
	-- cpu type TODO make this sense?
	cpu_type bigint
		CONSTRAINT virtual_hardware_fk_cpu_type REFERENCES cpu_type(uid) DEFERRABLE,
	-- hdd size
	hdd_size int
		CONSTRAINT "virtual_hardware hdd_size should be positive number"
		CHECK (hdd_size > 0),
	note text,
	template bigint
);

