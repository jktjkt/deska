--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE service_uid START 1;

-- services
CREATE TABLE service (
	-- this column is required in all plugins
	uid bigint DEFAULT nextval('service_uid')
		CONSTRAINT service_pk PRIMARY KEY,
	-- this column is required in all plugins
	name identifier
		CONSTRAINT "service of this name already exists" UNIQUE NOT NULL,
	isVM integer NOT NULL DEFAULT 0,
	note text
);
 
