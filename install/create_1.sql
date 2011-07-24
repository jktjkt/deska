BEGIN;
-- create schemas
\i create_schemas_1.sql

-- create modules
\i modules/vendor.sql
\i modules/hwbox.sql
\i modules/cpu_type.sql
\i modules/hardware.sql
\i modules/virtual_hardware.sql
\i modules/host.sql
\i modules/service.sql
\i modules/switch.sql
\i modules/network.sql
\i modules/interface.sql
--\i modules/large_modul.sql

END;
