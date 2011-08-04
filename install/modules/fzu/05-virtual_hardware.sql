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
	hardwaremodel bigint 
		CONSTRAINT virtual_hardware_fk_hardwaremodel REFERENCES hardwaremodel(uid) DEFERRABLE,
	note text,
	template bigint
);

