SET search_path TO genproc,history,deska,production;
-- after connect, register session

SELECT start_changeset();

-- works?
SELECT my_version();

-- some changes follows
SELECT vendor_add('DELL');

-- commit
SELECT vendor_commit();

-- end session, part of vendor commit? of vendor commit part of this?
SELECT version_commit();

SELECT * FROM vendor_history;
SELECT * FROM vendor;

SELECT start_changeset();
SELECT vendor_del('DELL');
SELECT vendor_add('HP');
SELECT vendor_add('IBM');
SELECT vendor_commit();
SELECT version_commit();

SELECT * FROM vendor_history;
SELECT * FROM vendor;

