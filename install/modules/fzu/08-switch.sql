--
-- This is example of deska plug-in
--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE switch_uid START 1;

-- vendors of hw
CREATE TABLE switch (
	-- nextval can be used - will aplly to history table
	uid bigint DEFAULT nextval('switch_uid')
		CONSTRAINT switch_pk PRIMARY KEY,
-- this column is required in all plugins
	name identifier
		CONSTRAINT "switch with this name already exists" UNIQUE NOT NULL,
	ports int
		CONSTRAINT "swhitch ports should be positive number"
		CHECK (ports > 0)
);
