--
-- every module must be place in schema production
--
SET search_path TO production;

-- vendors of hw
CREATE TABLE host (
	-- this column is required in all plugins
	uid bigint
		constraint host_pk PRIMARY KEY,
	-- this column is required in all plugins
	name text
		CONSTRAINT host_name_unique UNIQUE NOT NULL,
	-- hardwere where it runs
	-- TODO-virtual host
	hardware bigint
		CONSTRAINT host_fk_hardware REFERENCES hardware(uid),
	note text
);


