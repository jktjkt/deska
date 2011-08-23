--
-- every module must be place in schema production
--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE host_uid START 1;

-- vendors of hw
CREATE TABLE host (
	-- this column is required in all plugins
	uid bigint DEFAULT nextval('host_uid')
		CONSTRAINT host_pk PRIMARY KEY,
	-- this column is required in all plugins
	name identifier
		CONSTRAINT host_name_unique UNIQUE NOT NULL,
	-- hardwere where it runs
	-- TODO-virtual host
	-- TODO - better use uid
	hardware bigint
		CONSTRAINT rmerge_host_fk_hardware REFERENCES hardware(uid) DEFERRABLE,
	service identifier_set
		CONSTRAINT rset_host_fk_service REFERENCES service(uid),
	template bigint
	note_host text
);

