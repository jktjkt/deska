SET search_path TO production,deska;

CREATE SEQUENCE switch_uid START 1;

-- Ethernet switches
CREATE TABLE switch (
	uid bigint DEFAULT nextval('switch_uid')
		CONSTRAINT switch_pk PRIMARY KEY,
	name identifier
		CONSTRAINT "switch with this name already exists" UNIQUE NOT NULL,
	modelswitch bigint NOT NULL
		CONSTRAINT switch_fk_modelswitch REFERENCES modelswitch(uid) DEFERRABLE,
	-- contains box
	box bigint NOT NULL
		-- merge / contains
		--CONSTRAINT rmerge_switch_fk_box REFERENCES box(uid) DEFERRABLE,
		CONSTRAINT switch_fk_box REFERENCES box(uid) DEFERRABLE,
	purchase date,
	-- reference for the warranty information
	warranty_contact bigint
		CONSTRAINT switch_fk_warranty_contract REFERENCES warranty_contact(uid) DEFERRABLE,
	serial_1 text,
	serial_2 text,
	warranty_no text,
	--warranty_vendor bigint, -- FIXME: add a reference
	inventory_no text
);

