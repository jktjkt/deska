--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE boxmodel_uid START 1;

-- boxmodels
CREATE TABLE boxmodel (
	-- nextval can be used - will aplly to history table
	uid bigint DEFAULT nextval('boxmodel_uid')
		CONSTRAINT boxmodel_pk PRIMARY KEY,
-- this column is required in all plugins
	name identifier
		CONSTRAINT "boxmodel with this name already exists" UNIQUE NOT NULL,
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

