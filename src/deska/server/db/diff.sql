CREATE TYPE deska.diff_set_attribute_type AS(
    "objname" deska.identifier,
    attribute name,
    olddata text,
    newdata text
);

CREATE TYPE deska.diff_refs_set_set_attribute_type AS(
    "objname" deska.identifier,
    attribute name,
    olddata text[],
    newdata text[]
);
