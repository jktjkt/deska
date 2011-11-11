--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE box_uid START 1;

-- boxes for hw
CREATE TABLE box (
	-- nextval can be used - will aplly to history table
	uid bigint DEFAULT nextval('box_uid')
		CONSTRAINT box_pk PRIMARY KEY,
-- this column is required in all plugins
	name identifier
		CONSTRAINT "box with this name already exists" UNIQUE NOT NULL,
	-- The "box model" is defined by the kinds into which we are contained, not
	-- at this level.

	-- Our "parent"
	inside bigint
		CONSTRAINT box_fk_box REFERENCES box(uid) DEFERRABLE,
	-- position
	x int
		CONSTRAINT "'x' cannot be negative number" CHECK (x >= 0),
	y int
		CONSTRAINT "'y' cannot be negative number" CHECK (y >= 0),
	z int
		CONSTRAINT "'z' cannot be negative number" CHECK (z >= 0),
	note text
);

-- function for trigger, checking position number
CREATE FUNCTION box_check()
RETURNS TRIGGER
AS
$$
DECLARE parentX int;
	parentY int;
	parentZ int;
	nextX int;
	nextY int;
	nextZ int;
	prevX int;
	prevY int;
	prevZ int;
	sizeX int;
	sizeY int;
	sizeZ int;
BEGIN
        -- FIXME: this is broken; there's no "box.box" at all. Also we have to
        -- use the CONTAINABLE relation, find our matching modelbox from there
        -- and work on that values.

		-- FIXME: also change this to support the inner_bay_regexp
        RETURN NEW;
END
$$
LANGUAGE plpgsql;

-- CREATE TRIGGER box_trigger BEFORE INSERT OR UPDATE ON box FOR EACH ROW
--        EXECUTE PROCEDURE box_check()
