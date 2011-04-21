SET search_path TO api,genproc,history,deska,production;
-- after connect, register session
BEGIN;
SELECT startChangeset();

-- some changes follows
SELECT createObject('vendor','DELL');
SELECT createObject('hardware','hwDELL');
SELECT setAttribute('hardware','hwDELL','vendor','DELL');
SELECT setAttribute('hardware','hwDELL','purchase','10.1.2010');
SELECT setAttribute('hardware','hwDELL','warranty','10.1.2013');
SELECT setAttribute('hardware','hwDELL','note','cesky text v repozitari');

-- commit
-- end session, part of vendor commit? of vendor commit part of this?
SELECT commitChangeset('c1');

END;
BEGIN;

SELECT startChangeset();
--SELECT deleteObject('hardware','hwDELL');
--SELECT deleteObject('vendor','DELL');
SELECT createObject('vendor','HP');
SELECT createObject('vendor','IBM');
SELECT setAttribute('hardware','hwDELL','note','test');
SELECT setAttribute('hardware','hwDELL','vendor','IBM');
SELECT commitChangeset('c2');

END;

BEGIN;
SELECT startChangeset();
SELECT createObject('host','hpv2');
SELECT setAttribute('host','hpv2','hardware','hwDELL');
SELECT commitChangeset('c3');
END;
BEGIN;
SELECT startChangeset();
SELECT createObject('interface','hpv2->eth0');
SELECT commitChangeset('c4');
END;
--delete of embed into
BEGIN;
SELECT startChangeset();
SELECT deleteObject('interface','hpv2->eth0');
SELECT commitChangeset('c4');
END;
