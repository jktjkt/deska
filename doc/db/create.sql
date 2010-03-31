-- switch to deska_dev SCHEMA
SET search_path TO deska_dev,public;

-- cut here: destroy script
DROP TABLE lan_adapter;
DROP TABLE cpu;
DROP TABLE hdd;
DROP TABLE ram;
DROP TABLE hardware;
DROP TABLE vendor;
-- cut here

-- vendors of hw
CREATE TABLE vendor (
	uid char(36) PRIMARY KEY,
	name text
);
GRANT ALL ON vendor TO deska_team;


-- hw
CREATE TABLE hardware (
	uid char(36),-- UNIQUE NOT NULL,
	vendor char(36) references vendor(uid),
	-- hw type
	-- cpu
	-- ram, hdd
	-- size, power
	purchase date,
	warranty date,
	note text
);
GRANT ALL ON hardware TO deska_team;

-- cut here: test
insert into vendor values (1,'HP');
-- cut here

-- ram
CREATE TABLE ram (
	uid char(36) PRIMARY KEY,
	size INT NOT NULL
		CONSTRAINT ram_size_positive
			CHECK (size > 0)
) INHERITS (hardware);
GRANT ALL ON ram TO deska_team;

-- cut here: test
insert into ram values (1,1,'2009-12-02','2011-12-02','',4);
-- cut here

CREATE TABLE hdd (
	uid char(36) PRIMARY KEY,
	size INT NOT NULL
		CONSTRAINT ram_size_positive
			CHECK (size > 0)
) INHERITS (hardware);
GRANT ALL ON hdd TO deska_team;

-- cpu
CREATE TABLE cpu (
	uid char(36) PRIMARY KEY,
	cores INT NOT NULL
		CONSTRAINT cpu_cores_positive
			CHECK (cores > 0),
	frequency INT NOT NULL
		CONSTRAINT cpu_frequency_positive
			CHECK (frequency > 0),
	hypert BOOLEAN NOT NULL
) INHERITS (hardware);
GRANT ALL ON cpu TO deska_team;

CREATE TABLE lan_adapter (
	uid char(36) PRIMARY KEY,
	mac TEXT UNIQUE
	--	CONSTRAINT lan_adapter_mac_ok
) INHERITS (hardware);
GRANT ALL ON lan_adapter TO deska_team;
