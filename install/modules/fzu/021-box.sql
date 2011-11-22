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
	-- box model
	modelbox bigint
		CONSTRAINT box_fk_boxmode REFERENCES modelbox(uid) DEFERRABLE,
	-- box can be another whbox
	parent bigint
		CONSTRAINT box_fk_box REFERENCES box(uid) DEFERRABLE,
	-- position
	posX int
		CONSTRAINT "posx cannot be negative number" CHECK (posX >= 0),
	posY int
		CONSTRAINT "posy cannot be negative number" CHECK (posY >= 0),
	posZ int
		CONSTRAINT "posz cannot be negative number" CHECK (posZ >= 0),
	note text,
	--box is containable hardawre
	hardware bigint,
	--box is containable switch
	switch bigint
	--box is containable diskarray
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
        IF NEW.parent IS NOT NULL
	THEN
		-- find parent inside dimensions
		SELECT insX,insY,insZ INTO parentX,parentY,parentZ FROM box JOIN modelbox ON (box.box = modelbox.uid)
			WHERE box.uid = NEW.box;
		-- position is inside the parent box
		IF NEW.posX > parentX THEN
			RAISE EXCEPTION 'posX is greater than parent insX: (% > %)', NEW.posX, parentX;
		END IF;
		IF NEW.posY > parentY THEN
			RAISE EXCEPTION 'posY is greater than parent insY: (% > %)', NEW.posY, parentY;
		END IF;
		IF NEW.posZ > parentZ THEN
			RAISE EXCEPTION 'posZ is greater than parent insZ: (% > %)', NEW.posZ, parentZ;
		END IF;
		nextX = parentX;
		nextY = parentY;
		nextZ = parentZ;
		prevX = 0;
		prevY = 0;
		prevZ = 0;
		-- find next position in parent box
		SELECT min(posX) INTO nextX FROM box
			WHERE box = NEW.box AND
			posX > NEW.posX;
		SELECT min(posY) INTO nextY FROM box
			WHERE box = NEW.box AND
			posY > NEW.posY;
		SELECT min(posZ) INTO nextZ FROM box
			WHERE box = NEW.box AND
			posZ > NEW.posZ;

		-- find previous position in parent box
		SELECT max(posX) INTO prevX FROM box
			WHERE box = NEW.box AND
			posX <= NEW.posX;
		SELECT max(posY) INTO prevY FROM box
			WHERE box = NEW.box AND
			posY <= NEW.posY;
		SELECT max(posZ) INTO prevZ FROM box
			WHERE box = NEW.box AND
			posZ <= NEW.posZ;

		-- find sizes
		SELECT modelbox.sizeX,modelbox.sizeY,modelbox.sizeZ INTO sizeX,sizeY,sizeZ FROM modelbox
			WHERE uid = NEW.modelbox;

		-- size is inside the parent box, and before next box
		IF (NEW.posX + sizeX > nextX) OR (NEW.posX <= prevX) THEN
			RAISE EXCEPTION 'There is not enough free space in box';
		END IF;
		IF (NEW.posY + sizeY > nextY) OR (NEW.posY <= prevY) THEN
			RAISE EXCEPTION 'There is not enough free space in box';
		END IF;
		IF (NEW.posZ + sizeZ > nextZ) OR (NEW.posZ <= prevZ) THEN
			RAISE EXCEPTION 'There is not enough free space in box';
		END IF;
	END IF;	
        RETURN NEW;
END
$$
LANGUAGE plpgsql;

CREATE TRIGGER box_trigger BEFORE INSERT OR UPDATE ON box FOR EACH ROW
        EXECUTE PROCEDURE box_check()
