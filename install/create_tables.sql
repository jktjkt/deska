CREATE SCHEMA versioning AUTHORIZATION deska_admin;
SET search_path TO versioning;

CREATE SEQUENCE version_num;

CREATE TYPE changeset_status AS ENUM ( 'DETACHED', 'INPROGRESS' );

-- COMMITED versions
CREATE TABLE version(
	id bigint
		CONSTRAINT version_pk PRIMARY KEY,
	-- human readable id
	num int
		CONSTRAINT version_num_unique UNIQUE
			DEFAULT nextval('version_num'),
	-- who created
	author text,
	-- time of commit
	timestamp timestamp without time zone NOT NULL DEFAULT now(),
	-- commit message text
	--FIXME: is better "" instead of NULL - see #191, fix to "" for now
	message text NOT NULL
);

-- INPROGRESS and DETACHED changesets
CREATE TABLE changeset(
	-- internal id
	id bigserial
		CONSTRAINT changeset_pk PRIMARY KEY,
	author text NOT NULL,
	-- backend pid - like session id
	pid int
		CONSTRAINT changeset_pid_unique UNIQUE,
	-- revision of db, when new changset is started
	parentrevision bigint
		-- only for commited (with num assigned)
		CONSTRAINT changeset_parentrevision_uid REFERENCES version(id),
	status changeset_status NOT NULL DEFAULT 'INPROGRESS',
	-- time of creation
	timestamp timestamp without time zone NOT NULL DEFAULT now(),
	message text NOT NULL DEFAULT ''
		-- check message not null when detached
		CONSTRAINT changeset_message_not_null
			CHECK ((length(message) > 0)
				OR (status = 'INPROGRESS')),
	is_generated boolean DEFAULT false
);

-- create first empty initial revision 0
INSERT INTO version (id,author,message)
        VALUES ('0',current_user,'Initial revision');


