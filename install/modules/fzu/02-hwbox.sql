--
-- every module must be place in schema production
--
SET search_path TO production,deska;

CREATE SEQUENCE hwbox_uid START 1;

-- boxes for hw
CREATE TABLE hwbox (
	-- nextval can be used - will aplly to history table
	uid bigint DEFAULT nextval('hwbox_uid')
		CONSTRAINT hwbox_pk PRIMARY KEY,
-- this column is required in all plugins
	name identifier
		CONSTRAINT "hwbox with this name already exists" UNIQUE NOT NULL,
	-- hwbox can be another whbox
	hwbox bigint
		CONSTRAINT hwbox_fk_hwbox REFERENCES hwbox(uid) DEFERRABLE,
	-- size
	sizeX int
		CONSTRAINT "sizex cannot be negative number" CHECK (sizeX >= 0),
	sizeY int
		CONSTRAINT "sizey cannot be negative number" CHECK (sizeX >= 0),
	sizeZ int
		CONSTRAINT "sizez cannot be negative number" CHECK (sizeX >= 0),
	-- position
	posX int
		CONSTRAINT "posx cannot be negative number" CHECK (posX >= 0),
	posY int
		CONSTRAINT "posy cannot be negative number" CHECK (posY >= 0),
	posZ int
		CONSTRAINT "posz cannot be negative number" CHECK (posZ >= 0),
	-- inside dimensions
	insX int
		CONSTRAINT "insx cannot be negative number" CHECK (insX >= 0),
	insY int
		CONSTRAINT "insy cannot be negative number" CHECK (insY >= 0),
	insZ int
		CONSTRAINT "insz cannot be negative number" CHECK (insZ >= 0),
	note text
);

-- function for trigger, checking position number
CREATE FUNCTION hwbox_check()
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
BEGIN
        IF NEW.hwbox IS NOT NULL
	THEN
		SELECT insX,insY,insZ INTO parentX,parentY,parentZ FROM hwbox
			WHERE uid = NEW.hwbox;
		-- position is inside the parent hwbox
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
		-- find next position in parent hwbox
		SELECT min(posX) INTO nextX FROM hwbox
			WHERE hwbox = NEW.hwbox AND
			posX > NEW.posX;
		SELECT min(posY) INTO nextY FROM hwbox
			WHERE hwbox = NEW.hwbox AND
			posY > NEW.posY;
		SELECT min(posZ) INTO nextZ FROM hwbox
			WHERE hwbox = NEW.hwbox AND
			posZ > NEW.posZ;

		-- find previous position in parent hwbox
		SELECT max(posX) INTO prevX FROM hwbox
			WHERE hwbox = NEW.hwbox AND
			posX <= NEW.posX;
		SELECT max(posY) INTO prevY FROM hwbox
			WHERE hwbox = NEW.hwbox AND
			posY <= NEW.posY;
		SELECT max(posZ) INTO prevZ FROM hwbox
			WHERE hwbox = NEW.hwbox AND
			posZ <= NEW.posZ;

		-- size is inside the parent hwbox, and before next hwbox
		IF (NEW.posX + NEW.sizeX > nextX) OR (NEW.posX <= prevX) THEN
			RAISE EXCEPTION 'There is not enough free space in hwbox';
		END IF;
		IF (NEW.posY + NEW.sizeY > nextY) OR (NEW.posY <= prevY) THEN
			RAISE EXCEPTION 'There is not enough free space in hwbox';
		END IF;
		IF (NEW.posZ + NEW.sizeZ > nextZ) OR (NEW.posZ <= prevZ) THEN
			RAISE EXCEPTION 'There is not enough free space in hwbox';
		END IF;
	END IF;	
        RETURN NEW;
END
$$
LANGUAGE plpgsql;

CREATE TRIGGER hwbox_trigger BEFORE INSERT OR UPDATE ON hwbox FOR EACH ROW
        EXECUTE PROCEDURE hwbox_check()
