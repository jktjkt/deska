SET search_path TO api,genproc,history,deska,production;
BEGIN;
SELECT startChangeset();
SELECT createObject('vendor','DELL');
SELECT createObject('hardware','DELL');
SELECT setAttribute('hardware','DELL','vendor','DELL');
SELECT setAttribute('hardware','DELL','purchase','10.1.2010');
SELECT setAttribute('hardware','DELL','warranty','10.1.2013');
SELECT createObject('hardware','IBM');
SELECT setAttribute('hardware','IBM','vendor','DELL');
SELECT setAttribute('hardware','IBM','purchase','10.1.2010');
SELECT setAttribute('hardware','IBM','warranty','10.1.2013');

SELECT commitChangeset('1');


SELECT startChangeset();
-- new object
SELECT createObject('hardware','newhw');
-- set attribute
SELECT setAttribute('hardware','DELL','warranty','22.2.2013');
-- new attribute
SELECT setAttribute('hardware','DELL','note','cesky text v repozitari');

SELECT setAttribute('hardware','newhw','vendor','DELL');
SELECT setAttribute('hardware','newhw','purchase','10.1.2010');
SELECT setAttribute('hardware','newhw','warranty','10.1.2013');

-- del object
SELECT deleteObject('hardware','IBM');

SELECT commitChangeset('2');

END;

CREATE OR REPLACE VIEW a AS SELECT * FROM hardware_history WHERE version <= 22411 AND dest_bit = B'0';
CREATE OR REPLACE VIEW b AS SELECT * FROM hardware_history WHERE version > 22411 AND version <= 42421 AND dest_bit = B'0';

--SELECT 'changed:',a.name,b.name FROM a JOIN b ON a.uid = b.uid;
--SELECT 'deleted:',a.name
--	FROM a LEFT OUTER JOIN b ON a.uid = b.uid
--	WHERE b.name IS NULL;
--SELECT 'created',b.name
--	FROM a RIGHT OUTER JOIN b ON a.uid = b.uid
--	WHERE a.name IS NULL;

CREATE OR REPLACE FUNCTION diff(x bigint, y bigint)
RETURNS text
AS
$$
names = ["name","vendor","note","warranty","purchase"]
oldnames = map(str.__add__,["o"]*len(names),names)
newnames = map(str.__add__,["n"]*len(names),names)
opart = map("a.{0} AS {1}".format,names,oldnames)
npart = map("b.{0} AS {1}".format,names,newnames)
coldef = ",".join(opart) + "," + ",".join(npart)
a = plpy.execute('SELECT {coldef} FROM a JOIN b ON (a.uid = b.uid)'.format(coldef = coldef));
text = ""
for i in range(a.nrows()):
	line = a[i]
	for col in range(len(names)):
		if line[oldnames[col]] != line[newnames[col]]:
			diffline = '{oldname}: "{oldvalue}", {newname}: "{newvalue}"'.format(
				oldname = oldnames[col], newname = newnames[col],
				oldvalue = line[oldnames[col]], newvalue = line[newnames[col]])
			text = text + "\n" + diffline

return text
$$
LANGUAGE plpythonu;

CREATE OR REPLACE FUNCTION diff_created(from_rev bigint,to_rev bigint)
RETURNS SETOF hardware_history
AS
$$
BEGIN
RETURN QUERY SELECT b.*
      FROM a RIGHT OUTER JOIN b ON a.uid = b.uid
	        WHERE a.name IS NULL;
END;
$$
LANGUAGE plpgsql;

