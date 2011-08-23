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
		CONSTRAINT hardware_name_unique UNIQUE NOT NULL,
	-- TODO - better use uid
	vendor bigint 
		CONSTRAINT hardware_fk_vendor REFERENCES vendor(uid) DEFERRABLE,
	purchase date NOT NULL,
	warranty date NOT NULL,
	-- GB of RAM
	ram INT
		CONSTRAINT hardware_ram_positive
		CHECK (ram > 0),
	-- number of CPU's
	cpu_num INT
		CONSTRAINT hardware_cpu_num_positive
		CHECK (cpu_num > 0),
	-- add cpu type, when we have cpu type table
	host bigint,
	note_hardware text,
	template bigint
);

