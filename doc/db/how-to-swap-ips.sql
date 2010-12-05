-- 
-- example how to swap 2 record in columns with unique option
-- 
-- note: this works only in postgresql versions 9 and heigher
--

-- works only inside transaction
BEGIN;

-- set all (or just name of the one you need) constraints 
-- to be checked at the end of transaction
SET CONSTRANITS ALL DEFERRED;

-- make changes
UPDATE interface SET ip = '10.0.0.5' where id = 6;
UPDATE interface SET ip = '10.0.0.6' where id = 5;

-- commit, constraints will be checked now
COMMIT;

