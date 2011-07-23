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
	weight int,
	height int,
	width int,
	power int,
	note text,
	template bigint
);

