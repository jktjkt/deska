-- switch to deska_dev SCHEMA
SET search_path TO deska_dev,public;

-- add some vendors
select vendor_add('--IBM--001','IBM',1);
select vendor_add('--DELL--001','Dell',1);
select vendor_add('--HP--0_1','HP',1);

-- commit
select vendor_commit(1);

-- add another
select vendor_add('--INTEL--211_13123440','Intel',2);
select vendor_add('--DELL--001','DELL',2);

-- commit
select vendor_commit(2);

-- delete vendor
select vendor_del('--DELL--001','Dell',3);

-- commit
select vendor_commit(3);
