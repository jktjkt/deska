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
	isVM bool NOT NULL DEFAULT FALSE,
	note text
);

-- add host-service relation
CREATE SEQUENCE hostservice_uid START 1;
CREATE TABLE hostservice (
	-- this column is required in all plugins
	uid bigint DEFAULT nextval('host_service_uid')
		CONSTRAINT hostservice_pk PRIMARY KEY,
	-- this table is not pluging and has no name

	host bigint
		CONSTRAINT hostservice_fk_host REFERENCES host(uid) DEFERRABLE,
	service bigint
		CONSTRAINT hostservice_fk_host REFERENCES service(uid) DEFERRABLE,

);

