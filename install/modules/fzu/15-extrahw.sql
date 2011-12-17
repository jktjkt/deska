SET search_path TO production,deska;

CREATE SEQUENCE modelextrahw_uid START 1;
CREATE SEQUENCE extrahw_uid START 1;

CREATE TABLE modelextrahw (
    uid bigint DEFAULT nextval('modelextrahw_uid')
        CONSTRAINT modelextrahw_pk PRIMARY KEY,
    name identifier
        CONSTRAINT "modelextrahw with this name already exists" UNIQUE NOT NULL,
    vendor bigint
        CONSTRAINT modelextrahw_fk_vendor REFERENCES vendor(uid) DEFERRABLE,
    modelbox bigint
        CONSTRAINT modelextrahw_fk_modelbox REFERENCES modelbox(uid) DEFERRABLE
);

-- Extra equipment for which we don't have any specialized class
CREATE TABLE extrahw (
    uid bigint DEFAULT nextval('extrahw_uid')
        CONSTRAINT extrahw_pk PRIMARY KEY,
    name identifier
        CONSTRAINT "extrahw with this name already exists" UNIQUE NOT NULL,
    modelextrahw bigint NOT NULL
        CONSTRAINT extrahw_fk_modelextrahw REFERENCES modelextrahw(uid) DEFERRABLE,
    box bigint NOT NULL
        CONSTRAINT rconta_extrahw_fk_box REFERENCES box(uid) DEFERRABLE,
    purchase date,
    warranty_contract bigint
        CONSTRAINT switch_fk_warranty_contract REFERENCES warranty_contract(uid) DEFERRABLE,
    serial_1 text,
    serial_2 text,
    inventory_no text,
    note_extrahw text
);
