SET search_path TO genproc,history,deska,production;
-- after connect, register session
BEGIN;
SELECT start_changeset();

-- works?
SELECT my_version();

-- some changes follows
SELECT vendor_add('DELL');
SELECT hardware_add('hwDELL');
SELECT hardware_set_vendor('hwDELL','DELL');
SELECT hardware_set_purchase('hwDELL','10.1.2010');
SELECT hardware_set_warranty('hwDELL','10.1.2012');

-- commit
SELECT vendor_commit();
SELECT hardware_commit();

-- end session, part of vendor commit? of vendor commit part of this?
SELECT version_commit();

SELECT * FROM vendor_history;
SELECT * FROM hardware_history;
SELECT * FROM vendor;
SELECT * FROM hardware;

SELECT start_changeset();
SELECT hardware_del('hwDELL');
SELECT vendor_del('DELL');
SELECT vendor_add('HP');
SELECT vendor_add('IBM');
SELECT hardware_commit();
SELECT vendor_commit();
SELECT version_commit();

SELECT * FROM vendor_history;
SELECT * FROM vendor;

END;
