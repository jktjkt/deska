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
	modelhardware bigint 
		CONSTRAINT hardware_fk_modelhardware REFERENCES modelhardware(uid) DEFERRABLE NOT NULL,
	purchase date NOT NULL,
	-- reference for the warranty information
	warranty_contact bigint
		CONSTRAINT hardware_fk_warranty_contract REFERENCES warranty_contact(uid) DEFERRABLE,
	-- box (contains)
	box bigint
		-- merge / contains
		--CONSTRAINT rmerge_hardware_fk_hwbox REFERENCES box(uid) DEFERRABLE,
		CONSTRAINT hardware_fk_hwbox REFERENCES box(uid) DEFERRABLE NOT NULL,

	-- Serial number one, in vendor's preferred form
	serial_1 text,
	-- Serial number two, in vendor's preferred form
	serial_2 text,

    -- FIXME: replace the warranty bits with a warranty_contract

	-- Warranty contract ID, in vendor's preferred form
	warranty_no text,
	-- Optional reference to a warranty vendor for contact information
	warranty_vendor bigint,
	-- Internal inventory number at FZU, format: dddddd-d
	inventory_no text,

	template_hardware bigint
);

