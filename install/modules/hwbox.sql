--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE hwbox_uid START 1;

-- boxes for hw
CREATE TABLE hwbox (
	-- nextval can be used - will aplly to history table
	uid bigint DEFAULT nextval('hwbox_uid')
		CONSTRAINT hwbox_pk PRIMARY KEY,
-- this column is required in all plugins
	name identifier
		CONSTRAINT "hwbox with this name already exists" UNIQUE NOT NULL,
	height int,
	width int,
	depth int,
	positions int NOT NULL,
	note text
);

