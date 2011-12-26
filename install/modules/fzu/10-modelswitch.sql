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
	port_validity_regexp text
		CONSTRAINT "switch ports cannot be empty string"
		CHECK (port_validity_regexp != '')
);

CREATE INDEX idx_modelswitch_vendor ON modelswitch(vendor);
CREATE INDEX idx_modelswitch_modelbox ON modelswitch(modelbox);

