-- switch to deska_dev SCHEMA
SET search_path TO deska_dev,public;

-- cut here: destroy script
DROP TABLE os;
DROP TABLE machine;
DROP TABLE enclosure;
DROP TABLE trunk;
DROP TABLE lan_adapter;
DROP TABLE switch;
DROP TABLE interface;
DROP TABLE cpu;
DROP TABLE cpu_type;
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
	-- uid přetíží potomci, unikátní v nich...
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
	-- size in GB
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

-- cpu type
CREATE TABLE cpu_type (
	uid char(36) PRIMARY KEY,
	cores INT NOT NULL
		CONSTRAINT cpu_cores_positive
			CHECK (cores > 0),
	frequency INT NOT NULL
		CONSTRAINT cpu_frequency_positive
			CHECK (frequency > 0),
	hypert BOOLEAN NOT NULL
);
GRANT ALL ON cpu_type TO deska_team;

-- cpu
CREATE TABLE cpu (
	uid char(36) PRIMARY KEY,
	type char(36) references cpu_type(uid)
--	machine char(36) references machine(uid),
) INHERITS (hardware);
GRANT ALL ON cpu TO deska_team;


-- interface
CREATE TABLE interface (
	uid int PRIMARY KEY,
	name text NOT NULL,
	ip INET 
		constraint interface_ip_unique UNIQUE,
	-- svázat s lan_adapter, nebo trunk, nebo jinej interface?
	dns_name text
);
GRANT ALL ON interface TO deska_team;

-- switch
CREATE TABLE switch (
	uid char(36) PRIMARY KEY,
	-- number of ports
	ports int
		CONSTRAINT switch_ports_positive
			CHECK (ports > 0)
) INHERITS (hardware);
GRANT ALL ON switch TO deska_team;

-- lan_adapter
CREATE TABLE lan_adapter (
	uid char(36) PRIMARY KEY,
	mac TEXT UNIQUE,
	--	CONSTRAINT lan_adapter_mac_ok
	-- od jakého portu a do jakýho switche, nebo port identifikuje switch?
	-- rychlost drátu, jednotky?
	rate int
		CONSTRAINT lan_adapter_rate_positive
			CHECK (rate > 0),
	-- číslo trunku nebo 0 pokud není
	trunk int DEFAULT 0
		CONSTRAINT lan_adapter_trunk_not_negative
			CHECK (trunk >= 0),
	switch char(36) references switch(uid)
) INHERITS (hardware);
GRANT ALL ON lan_adapter TO deska_team;

-- trunk
CREATE TABLE trunk (
	uid int PRIMARY KEY
	-- možná se bude hodit
);
GRANT ALL ON trunk TO deska_team;


-- enclosure, rack a další ...
CREATE TABLE enclosure (
	uid int PRIMARY KEY,
	-- nějaká velikost, počet pozic, umístění
	capacity int 
		CONSTRAINT enclosure_capacity_positive
			CHECK (capacity > 0),
	-- enclosure position or speciální enclosure room.
	placement int references enclosure(uid)
);
GRANT ALL ON enclosure TO deska_team;

-- machine
CREATE TABLE machine (
	uid char(36) PRIMARY KEY,
	-- tyhle 2 položky možná vyhodit
	-- typ procesorů,musí odpovídat tabulce cpu
	cpu_type char(36) references cpu_type(uid),
	-- počet procesorů,musí odpovídat tabulce cpu
	cpu_number int
		CONSTRAINT machine_cpu_number_positive
			CHECK (cpu_number > 0),
	ram char(36) references ram(uid),
	hdd char(36) references hdd(uid),
	lan_adapter char(36) references lan_adapter(uid),
	-- enclosure position...
	placement int references enclosure(uid)
) INHERITS (hardware);
GRANT ALL ON machine TO deska_team;

-- os
CREATE TABLE os (
	uid char(36) PRIMARY KEY,
	name text NOT NULL,
	version text NOT NULL,
	note text
	-- software?
	-- virtual/ref machine
	-- machine char(36) references machine(uid),
	-- os char(36) references os(uid),
	-- interface(s)
);
GRANT ALL ON os TO deska_team;

CREATE TABLE boxmodel (
	uid int PRIMARY KEY,
	name varchar(20),

	--outer size
	--dimension_type - bay-units(FALSE) / absolute number(TRUE)
	dimension_type boolean,
	--in milimeters
	width int
		CONSTRAINT boxmodel_width_positive CHECK (width > 0),
	height int
		CONSTRAINT boxmodel_height_positive CHECK (height > 0),	
	depth int
		CONSTRAINT boxmodel_depth_positive CHECK (depth > 0),
	--number of bays that occupies in parent rack

	--inner size
	--in bay units
	--number of bay units in the width/height/depth direction
	bay_width int
		CONSTRAINT boxmodel_bay_width CHECK (bay_width > 0),
	bay_height int
		CONSTRAINT boxmodel_bay_height CHECK (bay_height > 0),
	bay_depth int
		CONSTRAINT boxmodel_bay_depth CHECK (bay_depth > 0),

	--BOTTOM_TO_UP=1, UP_TO_BOTTOM=2, LEFT_TO_RIGHT=4, RIGHT_TO_LEFT=8, FRONT_TO_BACK=16, BACK_TO_FRONT=32 
	--!!!!!! TODO control by trigger
	bay_ordering_type int,
	parent_box int references boxmodel(uid)
);
GRANT ALL ON boxmodel TO deska_team;
