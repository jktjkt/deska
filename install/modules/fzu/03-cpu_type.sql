SET search_path TO production,deska;

CREATE SEQUENCE cpu_type_uid START 1;

-- cpu type
CREATE TABLE cpu_type (
	-- nextval can be used - will aplly to history table
	uid bigint DEFAULT nextval('cpu_type_uid')
		CONSTRAINT cpu_type_pk PRIMARY KEY,
-- this column is required in all plugins
	name identifier
		CONSTRAINT "cpu_type with this name already exists" UNIQUE NOT NULL,
	cores int
		CONSTRAINT "cpu_type cores should be positive number"
		CHECK (cores > 0),
	ht boolean,
	performance int
);

