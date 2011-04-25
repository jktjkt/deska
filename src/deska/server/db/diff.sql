SET search_path TO genproc;

CREATE TYPE diff_set_attribute_type AS(
	kind name,
	name text,
	attribute name,
	old_data text,
	new_data text
);
