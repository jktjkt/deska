SET search_path TO production,deska;

CREATE SEQUENCE warranty_contract_uid START 1;

-- Warranty/support contracts
CREATE TABLE warranty_contract (
	uid bigint DEFAULT nextval('warranty_contract_uid')
		CONSTRAINT warranty_contract_pk PRIMARY KEY,
	name identifier
		CONSTRAINT "warranty_contract with this name already exists" UNIQUE NOT NULL,
    purchased date,
    expires date,
    contract_no text NOT NULL,
    vendor bigint CONSTRAINT warranty_contract_fk_vendor REFERENCES vendor(uid) DEFERRABLE,
	phone text,
	supportmail text,
	supportnote text
);

CREATE INDEX idx_warr_contr_vendor ON warranty_contract(vendor);

