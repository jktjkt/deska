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
	-- switch coble
	switch bigint,
	-- hardware coble
	hardware bigint,
	-- extrahw containable
	extrahw bigint,
	-- position, checked by trigger
	position text,
	-- alternatively, do it via the good old numbers
	x int
		CONSTRAINT "'x' cannot be negative number" CHECK (x >= 0),
	y int
		CONSTRAINT "'y' cannot be negative number" CHECK (y >= 0),
	z int
		CONSTRAINT "'z' cannot be negative number" CHECK (z >= 0),
	note text
);

-- function for trigger, checking position number
CREATE FUNCTION box_check_position()
RETURNS TRIGGER
AS
$$
DECLARE pos_regexp text;
BEGIN
	if NEW.position IS NULL THEN
		-- FIXME: Unfortunately, this checking won't work at all for the multi-unit
		-- boxes.  The regular expression can check that a machine fits in a given
		-- slot, but the problems start to pop out when a machine occupies more than
		-- one slot.
		RETURN NEW;
	END IF;
	SELECT bays_validity_regexp INTO pos_regexp FROM modelbox WHERE modelbox.uid = NEW.inside;
	IF NEW.position !~ pos_regexp THEN
		RAISE EXCEPTION 'Box position % does not match bays_validity_regexp "%"!', NEW.position, pos_regexp;
	END IF;
	RETURN NEW;
END
$$
LANGUAGE plpgsql;

CREATE TRIGGER box_trigger_2 BEFORE INSERT OR UPDATE ON box FOR EACH ROW
	EXECUTE PROCEDURE box_check_position();

