--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE hardware_uid START 1;

-- hardwares
CREATE TABLE hardware (
	-- this column is required in all plugins
	uid bigint DEFAULT nextval('hardware_uid')
		CONSTRAINT hardware_pk PRIMARY KEY,
	-- this column is required in all plugins
	name identifier
		CONSTRAINT "hardware with this name already exists" UNIQUE NOT NULL,
	-- model of hardware
	hardwaremodel bigint 
		CONSTRAINT hardware_fk_hardwaremodel REFERENCES hardwaremodel(uid) DEFERRABLE,
	purchase date NOT NULL,
	warranty date NOT NULL,
	box bigint
		CONSTRAINT hardware_fk_hwbox REFERENCES box(uid) DEFERRABLE,

	-- Serial number one, in vendor's preferred form
	serial_1 text,
	-- Serial number two, in vendor's preferred form
	serial_2 text,
	-- Warranty contract ID, in vendor's preferred form
	warranty_no text,
    -- Optional reference to a warranty vendor for contact information
    warranty_vendor bigint,
	-- Internal inventory number at FZU, format: dddddd-d
	inventory_no text,

	template bigint
);

