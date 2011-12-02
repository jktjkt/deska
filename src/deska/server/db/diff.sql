CREATE TYPE deska.diff_set_attribute_type AS(
    "objname" text,
    attribute text,
    olddata text,
    newdata text
);

CREATE TYPE deska.diff_refs_set_set_attribute_type AS(
    "objname" deska.identifier,
    attribute text,
    olddata text[],
    newdata text[]
);

CREATE TYPE deska.diff_rename_type AS(
    oldname text,
    newname text
);
