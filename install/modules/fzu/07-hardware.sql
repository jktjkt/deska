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
		CONSTRAINT rrefer_hardware_fk_modelhardware REFERENCES modelhardware(uid) DEFERRABLE NOT NULL,
	purchased date,
	-- reference for the warranty information
	warranty_contract bigint
		CONSTRAINT rrefer_hardware_fk_warranty_contract REFERENCES warranty_contract(uid) DEFERRABLE,
	-- box (contains)
	box bigint NOT NULL
		CONSTRAINT rconta_hardware_fk_hwbox REFERENCES box(uid) DEFERRABLE,

	--hardware is containable host
	host bigint,
	-- Serial number one, in vendor's preferred form
	serial_1 text,
	-- Serial number two, in vendor's preferred form
	serial_2 text,

	warranty_vendor bigint,
	-- Internal inventory number at FZU, format: dddddd-d
	inventory_no text,

	note_hardware text,

	template_hardware bigint
);

CREATE INDEX idx_hw_modelhardware ON hardware(modelhardware);
CREATE INDEX idx_hw_warranty_contract ON hardware(warranty_contract);
CREATE INDEX idx_hw_box ON hardware(box);
