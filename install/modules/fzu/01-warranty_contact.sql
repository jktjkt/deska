SET search_path TO production,deska;

CREATE SEQUENCE warranty_contact_uid START 1;

-- contact addresses for warranty service cases
CREATE TABLE warranty_contact (
	uid bigint DEFAULT nextval('warranty_contact_uid')
		CONSTRAINT warranty_contact_pk PRIMARY KEY,
	name identifier
		CONSTRAINT "warranty_contact with this name already exists" UNIQUE NOT NULL,
	phone text,
	supportmail text,
	supportnote text
);

