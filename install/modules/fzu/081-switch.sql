SET search_path TO production,deska;

CREATE SEQUENCE switch_uid START 1;

-- Ethernet switches
CREATE TABLE switch (
	uid bigint DEFAULT nextval('switch_uid')
		CONSTRAINT switch_pk PRIMARY KEY,
	name identifier
		CONSTRAINT "switch with this name already exists" UNIQUE NOT NULL,
    model bigint NOT NULL, -- FIXME: add a reference

    purchase date,
    warranty date,
    serial_1 text,
    serial_2 text,
    warranty_no text,
    warranty_vendor bigint, -- FIXME: add a reference
    inventory_no text,

    inside bigint
        CONSTRAINT modelswitch_fk_box REFERENCES box(uid) DEFERRABLE
    
);

