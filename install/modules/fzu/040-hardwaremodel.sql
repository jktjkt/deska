--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE hardwaremodel_uid START 1;

-- models of hardware
CREATE TABLE hardwaremodel (
	-- this column is required in all plugins
	uid bigint DEFAULT nextval('hardwaremodel_uid')
		CONSTRAINT hardwaremodel_pk PRIMARY KEY,
	-- this column is required in all plugins
	name identifier
		CONSTRAINT "hardwaremodel with this name already exists" UNIQUE NOT NULL,
	-- TODO - better use uid
	vendor bigint 
		CONSTRAINT hardwaremodel_fk_vendor REFERENCES vendor(uid) DEFERRABLE,
	-- GB of RAM
	ram int
		CONSTRAINT "hardwaremodel ram should be positive number"
		CHECK (ram > 0),
	-- number of CPU's
	cpu_num int
		CONSTRAINT "hardwaremodel cpu_num should be positive number"
		CHECK (cpu_num > 0),
	-- cpu type
	cpu_type text,
	cpu_cores int
		CONSTRAINT "cpu_cores should be positive number"
		CHECK (cpu_cores > 0),
	cpu_ht bool,
	cpu_performance int,
	-- hdd size
	hdd_size int
		CONSTRAINT "hardwaremodel hdd_size should be positive number"
		CHECK (hdd_size > 0),
	power int,
	note text
);

