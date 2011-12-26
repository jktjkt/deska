SET search_path TO production,deska;

CREATE SEQUENCE event_uid START 1;

-- Event log for physical boxes
CREATE TABLE event (
    -- Deska-internal
    uid bigint DEFAULT nextval('event_uid')
        CONSTRAINT event_pk PRIMARY KEY,
    name identifier,
    -- Reference to a box which is related to this event
    box bigint
        CONSTRAINT rembed_event_fk_box REFERENCES box(uid) DEFERRABLE,
    -- What is it about?
    note text,
    -- When did it occur? Default value is provided by the database.
    time timestamp DEFAULT NOW(),

    -- It's an embedded kind
    CONSTRAINT "Event with this name already exists in this context" UNIQUE (name, box)
);

CREATE INDEX idx_event_box ON event(box);

