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
	-- how many CPUs are assigned to this machine?
	vcpu_num int,
	-- how much RAM (in MB) to put in there?
	vram int,
    --virtual host is containable host
    host bigint
);

