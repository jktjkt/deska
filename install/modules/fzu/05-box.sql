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

	-- Defining a link to "something" which provides the information about the model of the box
	-- switch coble
	switch bigint,
	-- hardware coble
	hardware bigint,
	-- extrahw containable
	extrahw bigint,
	-- fallback: direct specification
	direct_modelbox bigint
		CONSTRAINT box_fk_modelbox REFERENCES modelbox(uid) DEFERRABLE,

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

-- Make sure that the direct_modelbox is not present if there's an indirect reference through composition
CREATE FUNCTION box_check_modelbox()
RETURNS TRIGGER
AS
$$
BEGIN
	IF ((NEW.switch IS NOT NULL) or (NEW.hardware IS NOT NULL) or (NEW.extrahw IS NOT NULL)) and (NEW.direct_modelbox IS NOT NULL) THEN
		RAISE EXCEPTION E'Box "%s" specifies a direct reference to a modelbox, even though there\'s already one through the composition', NEW.name;
	END IF;
	RETURN NEW;
END
$$
LANGUAGE plpgsql;


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

CREATE TRIGGER box_trigger_1 BEFORE INSERT OR UPDATE ON box FOR EACH ROW
	EXECUTE PROCEDURE box_check_modelbox();

CREATE TRIGGER box_trigger_2 BEFORE INSERT OR UPDATE ON box FOR EACH ROW
	EXECUTE PROCEDURE box_check_position();


CREATE FUNCTION box_not_in_cycle(box_uid bigint)
RETURNS BOOLEAN
AS
$$
BEGIN
IF box_uid IN (
    WITH recursive rdata AS(
        SELECT inside AS uid FROM box WHERE uid = box_uid AND inside IS NOT NULL
        UNION
        SELECT inside FROM box b JOIN rdata rd ON (b.uid = rd.uid)
        WHERE inside IS NOT NULL
    )
    SELECT uid FROM rdata
)
THEN
    RETURN FALSE;
ELSE
    RETURN TRUE;
END IF;

END;
$$
LANGUAGE plpgsql;

ALTER TABLE box ADD CONSTRAINT box_chck_inside CHECK (box_not_in_cycle(uid) = true);
