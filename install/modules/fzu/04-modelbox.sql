---
--- modelbox: specify common dimensions of a piece of iron which provides some space for accomodating other equipment
---
SET search_path TO production,deska;

CREATE SEQUENCE modelbox_uid START 1;

CREATE TABLE modelbox (
	uid bigint DEFAULT nextval('modelbox_uid')
		CONSTRAINT modelbox_pk PRIMARY KEY,
	name identifier
		CONSTRAINT "modelbox with this name already exists" UNIQUE NOT NULL,
	-- size
	sizeX int
		CONSTRAINT "sizex cannot be negative number" CHECK (sizeX >= 0),
	sizeY int
		CONSTRAINT "sizey cannot be negative number" CHECK (sizeX >= 0),
	sizeZ int
		CONSTRAINT "sizez cannot be negative number" CHECK (sizeX >= 0),
	-- inside dimensions
	insX int
		CONSTRAINT "insx cannot be negative number" CHECK (insX >= 0),
	insY int
		CONSTRAINT "insy cannot be negative number" CHECK (insY >= 0),
	insZ int
		CONSTRAINT "insz cannot be negative number" CHECK (insZ >= 0),
	note text
);

