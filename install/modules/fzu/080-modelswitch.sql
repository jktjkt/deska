SET search_path TO production,deska;

CREATE SEQUENCE modelswitch_uid START 1;

-- models of Ethernet switches
CREATE TABLE modelswitch (
	uid bigint DEFAULT nextval('modelswitch_uid')
		CONSTRAINT modelswitch_pk PRIMARY KEY,
	name identifier
		CONSTRAINT "modelswitch with this name already exists" UNIQUE NOT NULL,
	vendor bigint
		CONSTRAINT modelswitch_fk_vendor REFERENCES vendor(uid) DEFERRABLE,
	modelbox bigint
		CONSTRAINT modelswitch_fk_modelbox REFERENCES modelbox(uid) DEFERRABLE,
	--FIXME: TODO port_validity_regexp
	ports text NOT NULL
		CONSTRAINT "switch ports cannot be empty string"
		CHECK (char_length(ports) > 0)
);

