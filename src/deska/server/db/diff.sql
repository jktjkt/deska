SET search_path TO genproc;

CREATE TYPE genproc.diff_set_attribute_type AS(
    "objname" text,
    attribute name,
    olddata text,
    newdata text
);