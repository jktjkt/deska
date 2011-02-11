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
