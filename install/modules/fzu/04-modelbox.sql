---
--- modelbox: specify common dimensions of a piece of iron which provides some space for accomodating other equipment
---
SET search_path TO production,deska;

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

    -- Which types of hardware we can accomodate?
    -- FIXME: maybe change this to a special formfactor_tag set, see the ML for
    -- discussion
    -- FIXME: how shall I name this constraint?
    -- accepts_inside identifier_set
    --    CONSTRAINT rset_foo_FIXME REFERENCES modelbox(uid) DEFERRABLE,

    note text
);

