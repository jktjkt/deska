SET search_path TO api,genproc,history,deska,production;
-- after connect, register session
BEGIN;
SELECT start_changeset();

-- works?
SELECT my_version();

-- some changes follows
SELECT createObject('vendor','DELL');
SELECT createObject('hardware','hwDELL');
SELECT setAttribute('hardware','hwDELL','vendor','DELL');
SELECT setAttribute('hardware','hwDELL','purchase','10.1.2010');
SELECT setAttribute('hardware','hwDELL','warranty','10.1.2013');
SELECT setAttribute('hardware','hwDELL','note','cesky text v repozitari');

-- commit
-- end session, part of vendor commit? of vendor commit part of this?
SELECT commit();

SELECT * FROM vendor_history;
SELECT * FROM hardware_history;
SELECT * FROM vendor;
SELECT * FROM hardware;

SELECT start_changeset();
SELECT deleteObject('hardware','hwDELL');
SELECT deleteObject('vendor','DELL');
SELECT createObject('vendor','HP');
SELECT createObject('vendor','IBM');
SELECT commit();

SELECT * FROM vendor_history;
SELECT * FROM vendor;

END;
