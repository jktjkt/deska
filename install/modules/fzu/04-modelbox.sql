---
--- modelbox: specify common dimensions of a piece of iron which provides some space for accomodating other equipment
---
SET search_path TO production,deska;

CREATE SEQUENCE formfactor_uid START 1;
CREATE TABLE formfactor (
    uid bigint DEFAULT nextval('formfactor_uid')
        CONSTRAINT formfactor_pk PRIMARY KEY,
    name identifier
        CONSTRAINT "formfactor with this name already exists" UNIQUE NOT NULL
);

CREATE SEQUENCE modelbox_uid START 1;

CREATE TABLE modelbox (
    uid bigint DEFAULT nextval('modelbox_uid')
        CONSTRAINT modelbox_pk PRIMARY KEY,
    name identifier
        CONSTRAINT "modelbox with this name already exists" UNIQUE NOT NULL,
    -- size
    width int
        CONSTRAINT "width cannot be negative number" CHECK (width >= 0),
    height int
        CONSTRAINT "height cannot be negative number" CHECK (height >= 0),
    depth int
        CONSTRAINT "depth cannot be negative number" CHECK (depth >= 0),

    -- Regular expression for a list of valid bays in this
    -- rack/sleeve/enclosure/...
    bays_validity_regexp text,

    -- Outer form factor of this box
    formfactor bigint
        CONSTRAINT modelbox_fk_formfactor REFERENCES formfactor(uid) DEFERRABLE,

    -- Which types of hardware we can accomodate?
    accepts_inside identifier_set
       CONSTRAINT rset_modelbox_fk_formfactor REFERENCES formfactor(uid) DEFERRABLE,

    note text
);

