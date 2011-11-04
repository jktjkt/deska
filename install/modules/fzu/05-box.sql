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
        IF NEW.box IS NOT NULL
	THEN
        -- FIXME: this is broken; there's no "box.box" at all. Also we have to
        -- use the CONTAINABLE relation, find our matching modelbox from there
        -- and work on that values.

		-- find parent inside dimensions
		SELECT insX,insY,insZ INTO parentX,parentY,parentZ FROM box JOIN modelbox ON (box.box = modelbox.uid)
			WHERE box.uid = NEW.box;
		-- position is inside the parent box
		IF NEW.x > parentX THEN
			RAISE EXCEPTION 'x is greater than parent insX: (% > %)', NEW.x, parentX;
		END IF;
		IF NEW.y > parentY THEN
			RAISE EXCEPTION 'y is greater than parent insY: (% > %)', NEW.y, parentY;
		END IF;
		IF NEW.z > parentZ THEN
			RAISE EXCEPTION 'z is greater than parent insZ: (% > %)', NEW.z, parentZ;
		END IF;
		nextX = parentX;
		nextY = parentY;
		nextZ = parentZ;
		prevX = 0;
		prevY = 0;
		prevZ = 0;
		-- find next position in parent box
		SELECT min(x) INTO nextX FROM box
			WHERE box = NEW.box AND
			x > NEW.x;
		SELECT min(y) INTO nextY FROM box
			WHERE box = NEW.box AND
			y > NEW.y;
		SELECT min(z) INTO nextZ FROM box
			WHERE box = NEW.box AND
			z > NEW.z;

		-- find previous position in parent box
		SELECT max(x) INTO prevX FROM box
			WHERE box = NEW.box AND
			x <= NEW.x;
		SELECT max(y) INTO prevY FROM box
			WHERE box = NEW.box AND
			y <= NEW.y;
		SELECT max(z) INTO prevZ FROM box
			WHERE box = NEW.box AND
			z <= NEW.z;

		-- find sizes
		SELECT modelbox.sizeX,modelbox.sizeY,modelbox.sizeZ INTO sizeX,sizeY,sizeZ FROM modelbox
			WHERE uid = NEW.modelbox;

		-- size is inside the parent box, and before next box
		IF (NEW.x + sizeX > nextX) OR (NEW.x <= prevX) THEN
			RAISE EXCEPTION 'There is not enough free space in box';
		END IF;
		IF (NEW.y + sizeY > nextY) OR (NEW.y <= prevY) THEN
			RAISE EXCEPTION 'There is not enough free space in box';
		END IF;
		IF (NEW.z + sizeZ > nextZ) OR (NEW.z <= prevZ) THEN
			RAISE EXCEPTION 'There is not enough free space in box';
		END IF;
	END IF;	
        RETURN NEW;
END
$$
LANGUAGE plpgsql;

CREATE TRIGGER box_trigger BEFORE INSERT OR UPDATE ON box FOR EACH ROW
        EXECUTE PROCEDURE box_check()
