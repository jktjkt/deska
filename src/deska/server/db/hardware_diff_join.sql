set search_path to deska, api, production, history, versioning, genproc;

create or replace function hardware_data_version(data_version bigint)
returns setof hardware_history
as
$$
declare
begin
	return query SELECT h.*
	FROM hardware_history h
		 JOIN version v ON (h.version = v.id) 
	WHERE dest_bit = '0' AND v.num = (
		 SELECT max(v2.num) 
		 FROM hardware_history h2 
			  JOIN version v2 ON (h2.uid = h.uid AND h2.version = v2.id) 
		 WHERE v2.num <= data_version);
end
$$
language plpgsql;

select hardware_data_version(4);

create or replace function hardware_changes_between_versions(from_version bigint, to_version bigint)
returns setof hardware_history
as
$$
begin
	return query SELECT h.*
	 FROM hardware_history h
		  JOIN version v ON (h.version = v.id) 
	 WHERE v.num = (
		  SELECT max(v2.num) 
		  FROM hardware_history h2 
		  JOIN version v2 ON (h2.uid = h.uid AND h2.version = v2.id) 
		  WHERE v2.num >from_version AND v2.num <= to_version);
end
$$
language plpgsql;

-- DROP TYPE genproc.diff_set_attribute_type cascade;

CREATE TYPE genproc.diff_set_attribute_type AS
   (objkind "name",
    "objname" text,
    command text,
    attribute "name",
    olddata text,
    newdata text);


CREATE OR REPLACE FUNCTION 
	hardware_diff_set_attributes()
	 RETURNS SETOF diff_set_attribute_type
	 AS
	 $$
	 DECLARE
		  old_data hardware_history%rowtype;
		  new_data hardware_history%rowtype;
		  result diff_set_attribute_type;
	 BEGIN
		  result.command = 'setAttribute';
		  result.objkind = 'hardware';
		  FOR old_data.name, old_data.vendor, old_data.purchase, old_data.warranty, old_data.note,
			new_data.name, new_data.vendor, new_data.purchase, new_data.warranty, new_data.note IN 
			SELECT old_name, old_vendor, old_purchase, old_warranty, old_note, new_name, new_vendor, new_purchase, new_warranty, new_note
			FROM diff_data
			WHERE new_name IS NOT NULL
		  LOOP
				IF (old_data.name IS NOT NULL) AND (old_data.name <> new_data.name) THEN
					 --first change is changed name
					 result.objname = old_data.name;
					 result.attribute = 'name';
					 result.olddata = old_data.name;
					 result.newdata = new_data.name;
					 RETURN NEXT result;
				END IF;
					 
				result.objname = new_data.name;
				
	 IF (old_data.warranty <> new_data.warranty) OR ((old_data.warranty IS NULL OR new_data.warranty IS NULL) 
		  AND NOT(old_data.warranty IS NULL AND new_data.warranty IS NULL))
	 THEN
		  result.attribute = 'warranty';
		  result.olddata = old_data.warranty;
		  result.newdata = new_data.warranty;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.purchase <> new_data.purchase) OR ((old_data.purchase IS NULL OR new_data.purchase IS NULL) 
		  AND NOT(old_data.purchase IS NULL AND new_data.purchase IS NULL))
	 THEN
		  result.attribute = 'purchase';
		  result.olddata = old_data.purchase;
		  result.newdata = new_data.purchase;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.vendor <> new_data.vendor) OR ((old_data.vendor IS NULL OR new_data.vendor IS NULL) 
		  AND NOT(old_data.vendor IS NULL AND new_data.vendor IS NULL))
	 THEN
		  result.attribute = 'vendor';
		  result.olddata = old_data.vendor;
		  result.newdata = new_data.vendor;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.cpu_num <> new_data.cpu_num) OR ((old_data.cpu_num IS NULL OR new_data.cpu_num IS NULL) 
		  AND NOT(old_data.cpu_num IS NULL AND new_data.cpu_num IS NULL))
	 THEN
		  result.attribute = 'cpu_num';
		  result.olddata = old_data.cpu_num;
		  result.newdata = new_data.cpu_num;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.ram <> new_data.ram) OR ((old_data.ram IS NULL OR new_data.ram IS NULL) 
		  AND NOT(old_data.ram IS NULL AND new_data.ram IS NULL))
	 THEN
		  result.attribute = 'ram';
		  result.olddata = old_data.ram;
		  result.newdata = new_data.ram;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.note <> new_data.note) OR ((old_data.note IS NULL OR new_data.note IS NULL) 
		  AND NOT(old_data.note IS NULL AND new_data.note IS NULL))
	 THEN
		  result.attribute = 'note';
		  result.olddata = old_data.note;
		  result.newdata = new_data.note;
		  RETURN NEXT result;			
	 END IF;
	 

		  END LOOP;	
	 END
	 $$
	 LANGUAGE plpgsql; 


create or replace function init_diff(from_version bigint, to_version bigint)
returns void as
$$
declare
begin
	create temp table diff_data 
	AS select dv.uid as old_uid,dv.name as old_name, dv.vendor as old_vendor, dv.warranty as old_warranty, dv.purchase as old_purchase, dv.note as old_note,
		chv.uid as new_uid,chv.name as new_name,chv.vendor as new_vendor, chv.warranty as new_warranty, chv.purchase as new_purchase, chv.note as new_note
		from hardware_data_version(from_version) dv full outer join hardware_changes_between_versions(from_version,to_version) chv on (dv.uid = chv.uid);
end
$$
language plpgsql;

CREATE OR REPLACE FUNCTION diff(x bigint, y bigint)
RETURNS SETOF text
AS
$$
import Postgres
@pytypes
def main(x,y):
	names = ["name","vendor","note","warranty","purchase"]
	oldnames = list(map(str.__add__,["o"]*len(names),names))
	newnames = list(map(str.__add__,["n"]*len(names),names))
	opart = list(map("dv.{0} AS {1}".format,names,oldnames))
	npart = list(map("chv.{0} AS {1}".format,names,newnames))
	coldef = ",".join(opart) + "," + ",".join(npart)
	plan = prepare('SELECT {coldef} FROM hardware_data_version($1) dv join hardware_changes_between_versions($1,$2) chv on (dv.uid = chv.uid)'.format(coldef = coldef))
	a = plan(x,y)

	for i in range(len(a)):
		line = a[i]
		for col in range(len(names)):
			if line[oldnames[col]] != line[newnames[col]]:
				diffline = '{oldname}: "{oldvalue}", {newname}: "{newvalue}"'.format(
					oldname = oldnames[col], newname = newnames[col],
					oldvalue = line[oldnames[col]], newvalue = line[newnames[col]])
				yield diffline
$$
LANGUAGE python;


drop function diff_del(bigint,bigint);
CREATE OR REPLACE FUNCTION diff_del(x bigint, y bigint)
RETURNS SETOF text
AS
$$
import Postgres
@pytypes
def main(x,y):
	names = ["name","vendor","note","warranty","purchase"]
	npart = list(map("dv.{0} AS {0}".format,names))
	coldef = ",".join(npart)
	plan = prepare('SELECT {coldef} FROM hardware_data_version($1) AS dv LEFT OUTER JOIN hardware_changes_between_versions($1,$2) AS chv ON dv.uid = chv.uid WHERE chv.name IS NULL'.format(coldef = coldef))
	a = plan(x,y)

	for line in a:
		base = dict()
		base["command"] = "removeObject"
		base["kindName"] = "hardware"
		base["objectName"] = line[names[0]]
		yield base
$$
LANGUAGE python;

CREATE OR REPLACE FUNCTION diff_add(x bigint, y bigint)
RETURNS SETOF text
AS
$$
import Postgres
@pytypes
def main(x,y):
	names = ["name","vendor","note","warranty","purchase"]
	npart = list(map("chv.{0} AS {0}".format,names))
	coldef = ",".join(npart)
	plan = prepare('SELECT {coldef} FROM hardware_data_version($1) AS dv RIGHT OUTER JOIN hardware_changes_between_versions($1,$2) AS chv ON dv.uid = chv.uid WHERE dv.name IS NULL'.format(coldef = coldef))
	a = plan(x,y)

	for line in a:
		#diffline = list()
		base = dict()
		base["command"] = "createObject"
		base["kindName"] = "hardware"
		base["objectName"] = line[names[0]]
		yield base
		#diffline.append(base)
		#base = base.copy()
		base["command"] = "setAttribute"
		for col in range(1,len(names)):
			if line[col] is not None:
				#setd = base.copy()
				base["value"] = line[col]
				base["attributeName"] = line[col]
				yield base

		#yield diffline
$$
LANGUAGE python;

--select init_diff(30,60);
--select hardware_diff_set_attributes(30,60);
--drop table diff_data;

create or replace function hardware_diff_set_attributes(from_version bigint, to_version bigint)
returns setof diff_set_attribute_type
as
$$
begin
perform init_diff(from_version,to_version);
return query select * from hardware_diff_set_attributes();
end
$$
language plpgsql;
 
CREATE OR REPLACE FUNCTION 
hardware_diff_created()
RETURNS SETOF text
AS
$$
BEGIN
	RETURN QUERY SELECT new_name FROM diff_data WHERE old_name IS NULL AND new_dest_bit = '0';
END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION 
hardware_diff_deleted()
RETURNS SETOF text
AS
$$
BEGIN
	RETURN QUERY SELECT old_name FROM diff_data WHERE new_dest_bit = '1';
END;
$$
LANGUAGE plpgsql; 
