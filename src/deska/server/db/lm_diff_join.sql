set search_path to deska, api, production, history, versioning, genproc;

create or replace function large_module_data_version(data_version bigint)
returns setof large_module_history
as
$$
declare
begin
	return query select h1.* from large_module_history h1 
	join version v1 on (v1.id = h1.version)
	join (select uid, max(num) as maxnum 
		from large_module_history h join version v on (v.id = h.version )
		where v.num <= data_version
		group by uid) vmax1 
	on (h1.uid = vmax1.uid and v1.num = vmax1.maxnum)
	WHERE dest_bit = '0';

end
$$
language plpgsql;

--select large_module_data_version(30000);

create or replace function large_module_changes_between_versions(from_version bigint, to_version bigint)
returns setof large_module_history
as
$$
begin
	return query select h1.* from large_module_history h1 
	join version v1 on (v1.id = h1.version)
	join (select uid, max(num) as maxnum 
		from large_module_history h join version v on (v.id = h.version )
		where v.num <= to_version and v.num > from_version
		group by uid) vmax1 
	on (h1.uid = vmax1.uid and v1.num = vmax1.maxnum);
end
$$
language plpgsql;

--select large_module_changes_between_versions(30000,60000);

DROP TYPE genproc.diff_set_attribute_type cascade;

CREATE TYPE genproc.diff_set_attribute_type AS
   (objkind "name",
    "objname" text,
    command text,
    attribute "name",
    olddata text,
    newdata text);


CREATE OR REPLACE FUNCTION 
	large_module_diff_set_attributes()
	 RETURNS SETOF diff_set_attribute_type
	 AS
	 $$
	 DECLARE
		  old_data large_module_history%rowtype;
		  new_data large_module_history%rowtype;
		  result diff_set_attribute_type;
	 BEGIN
		  result.command = 'setAttribute';
		  result.objkind = 'large_module';
		  FOR 
		  ld_data.deveriary, old_data.delitheria, old_data.solianta, old_data.killitha, old_data.gallitherr, old_data.meriarceir, old_data.riondrinti, old_data.kercellien, old_data.severy_ant, old_data.adaryshath, old_data.perdracie, old_data.percellian, old_data.lillitheri, old_data.kiliarcedd, old_data.therdricel, old_data.soliandria, old_data.merintin, old_data.galith, old_data.bericelie, old_data.winiara, old_data.koretinta, old_data.name, old_data.keriarica, old_data.heddarist, old_data.basoniann, old_data.valitheria, old_data.kallintona, old_data.antonianne, old_data.ebondrice, old_data.jerdracill, old_data.wintia, old_data.jeria, old_data.calita, old_data.sidonian, old_data.chatherin, old_data.saliarcede, old_data.fandrianne, old_data.quellarin, old_data.ciantintia, old_data.kaliniann, old_data.dominianta, old_data.duverianna, old_data.kacelither, old_data.amariary_a, old_data.adressanto, old_data.therceiren, old_data.salliarica, old_data.alvithea, old_data.beriaracei, old_data.carcellie, old_data.delithery, old_data.amaracilli, old_data.perynaraci, old_data.gallantia, old_data.tressanton, old_data.daracei, old_data.callithath, old_data.amarcedda, old_data.balianne, old_data.lucilia, old_data.miaricei, old_data.sirdrah, old_data.korevesta, old_data.severry_an, old_data.calaraceir, old_data.ciarice, old_data.bry_annari, old_data.daraceline, old_data.drucretine, old_data.kercelores, old_data.carcellith, old_data.duveryshat, old_data.amariniarc, old_data.jerdrinara, old_data.callienn, old_data.alvithathe, old_data.fantintina, old_data.hantintia, old_data.adracelien, old_data.handricedd, old_data.uid, old_data.salanta, old_data.dellantona, old_data.alandrevi, old_data.darceddarc, old_data.jercellita, old_data.treverista, old_data.antintony, old_data.calliaryss, old_data.sidomin, old_data.meriarcei, old_data.kagaminary, old_data.sirdrevesd, old_data.delory_ann, old_data.solita, old_data.darintona, old_data.kimberyl, old_data.kerry_anna, old_data.kacilarysh, old_data.therceddar, old_data.ballianne, old_data.drucillan,
		  new_data.deveriary, new_data.delitheria, new_data.solianta, new_data.killitha, new_data.gallitherr, new_data.meriarceir, new_data.riondrinti, new_data.kercellien, new_data.severy_ant, new_data.adaryshath, new_data.perdracie, new_data.percellian, new_data.lillitheri, new_data.kiliarcedd, new_data.therdricel, new_data.soliandria, new_data.merintin, new_data.galith, new_data.bericelie, new_data.winiara, new_data.koretinta, new_data.name, new_data.keriarica, new_data.heddarist, new_data.basoniann, new_data.valitheria, new_data.kallintona, new_data.antonianne, new_data.ebondrice, new_data.jerdracill, new_data.wintia, new_data.jeria, new_data.calita, new_data.sidonian, new_data.chatherin, new_data.saliarcede, new_data.fandrianne, new_data.quellarin, new_data.ciantintia, new_data.kaliniann, new_data.dominianta, new_data.duverianna, new_data.kacelither, new_data.amariary_a, new_data.adressanto, new_data.therceiren, new_data.salliarica, new_data.alvithea, new_data.beriaracei, new_data.carcellie, new_data.delithery, new_data.amaracilli, new_data.perynaraci, new_data.gallantia, new_data.tressanton, new_data.daracei, new_data.callithath, new_data.amarcedda, new_data.balianne, new_data.lucilia, new_data.miaricei, new_data.sirdrah, new_data.korevesta, new_data.severry_an, new_data.calaraceir, new_data.ciarice, new_data.bry_annari, new_data.daraceline, new_data.drucretine, new_data.kercelores, new_data.carcellith, new_data.duveryshat, new_data.amariniarc, new_data.jerdrinara, new_data.callienn, new_data.alvithathe, new_data.fantintina, new_data.hantintia, new_data.adracelien, new_data.handricedd, new_data.uid, new_data.salanta, new_data.dellantona, new_data.alandrevi, new_data.darceddarc, new_data.jercellita, new_data.treverista, new_data.antintony, new_data.calliaryss, new_data.sidomin, new_data.meriarcei, new_data.kagaminary, new_data.sirdrevesd, new_data.delory_ann, new_data.solita, new_data.darintona, new_data.kimberyl, new_data.kerry_anna, new_data.kacilarysh, new_data.therceddar, new_data.ballianne, new_data.drucillan
			 IN 
			SELECT old_deveriary, old_delitheria, old_solianta, old_killitha, old_gallitherr, old_meriarceir, old_riondrinti, old_kercellien, old_severy_ant, old_adaryshath, old_perdracie, old_percellian, old_lillitheri, old_kiliarcedd, old_therdricel, old_soliandria, old_merintin, old_galith, old_bericelie, old_winiara, old_koretinta, old_name, old_keriarica, old_heddarist, old_basoniann, old_valitheria, old_kallintona, old_antonianne, old_ebondrice, old_jerdracill, old_wintia, old_jeria, old_calita, old_sidonian, old_chatherin, old_saliarcede, old_fandrianne, old_quellarin, old_ciantintia, old_kaliniann, old_dominianta, old_duverianna, old_kacelither, old_amariary_a, old_adressanto, old_therceiren, old_salliarica, old_alvithea, old_beriaracei, old_carcellie, old_delithery, old_amaracilli, old_perynaraci, old_gallantia, old_tressanton, old_daracei, old_callithath, old_amarcedda, old_balianne, old_lucilia, old_miaricei, old_sirdrah, old_korevesta, old_severry_an, old_calaraceir, old_ciarice, old_bry_annari, old_daraceline, old_drucretine, old_kercelores, old_carcellith, old_duveryshat, old_amariniarc, old_jerdrinara, old_callienn, old_alvithathe, old_fantintina, old_hantintia, old_adracelien, old_handricedd, old_uid, old_salanta, old_dellantona, old_alandrevi, old_darceddarc, old_jercellita, old_treverista, old_antintony, old_calliaryss, old_sidomin, old_meriarcei, old_kagaminary, old_sirdrevesd, old_delory_ann, old_solita, old_darintona, old_kimberyl, old_kerry_anna, old_kacilarysh, old_therceddar, old_ballianne, old_drucillan,
				new_deveriary, new_delitheria, new_solianta, new_killitha, new_gallitherr, new_meriarceir, new_riondrinti, new_kercellien, new_severy_ant, new_adaryshath, new_perdracie, new_percellian, new_lillitheri, new_kiliarcedd, new_therdricel, new_soliandria, new_merintin, new_galith, new_bericelie, new_winiara, new_koretinta, new_name, new_keriarica, new_heddarist, new_basoniann, new_valitheria, new_kallintona, new_antonianne, new_ebondrice, new_jerdracill, new_wintia, new_jeria, new_calita, new_sidonian, new_chatherin, new_saliarcede, new_fandrianne, new_quellarin, new_ciantintia, new_kaliniann, new_dominianta, new_duverianna, new_kacelither, new_amariary_a, new_adressanto, new_therceiren, new_salliarica, new_alvithea, new_beriaracei, new_carcellie, new_delithery, new_amaracilli, new_perynaraci, new_gallantia, new_tressanton, new_daracei, new_callithath, new_amarcedda, new_balianne, new_lucilia, new_miaricei, new_sirdrah, new_korevesta, new_severry_an, new_calaraceir, new_ciarice, new_bry_annari, new_daraceline, new_drucretine, new_kercelores, new_carcellith, new_duveryshat, new_amariniarc, new_jerdrinara, new_callienn, new_alvithathe, new_fantintina, new_hantintia, new_adracelien, new_handricedd, new_uid, new_salanta, new_dellantona, new_alandrevi, new_darceddarc, new_jercellita, new_treverista, new_antintony, new_calliaryss, new_sidomin, new_meriarcei, new_kagaminary, new_sirdrevesd, new_delory_ann, new_solita, new_darintona, new_kimberyl, new_kerry_anna, new_kacilarysh, new_therceddar, new_ballianne, new_drucillan
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


create or replace function large_module_init_diff(from_version bigint, to_version bigint)
returns void as
$$
declare
begin
	create temp table diff_data 
	AS select dv.uid as old_uid,dv.name as old_name, dv.vendor as old_vendor, dv.warranty as old_warranty, dv.purchase as old_purchase, dv.note as old_note, dv.dest_bit as old_dest_bit,
		chv.uid as new_uid,chv.name as new_name,chv.vendor as new_vendor, chv.warranty as new_warranty, chv.purchase as new_purchase, chv.note as new_note, chv.dest_bit as new_dest_bit
		from large_module_data_version(from_version) dv full outer join large_module_changes_between_versions(from_version,to_version) chv on (dv.uid = chv.uid);
end
$$
language plpgsql;

CREATE OR REPLACE FUNCTION diff_set(x bigint, y bigint)
RETURNS SETOF text
AS
$$
import Postgres

@pytypes
def main(x,y):
	names = ["deveriary","delitheria","solianta","killitha","gallitherr","meriarceir","riondrinti","kercellien","severy_ant","adaryshath","perdracie","percellian","lillitheri","kiliarcedd","therdricel","soliandria","merintin","galith","bericelie","winiara","koretinta","name","keriarica","heddarist","basoniann","valitheria","kallintona","antonianne","ebondrice","jerdracill","wintia","jeria","calita","sidonian","chatherin","saliarcede","fandrianne","quellarin","ciantintia","kaliniann","dominianta","duverianna","kacelither","amariary_a","adressanto","therceiren","salliarica","alvithea","beriaracei","carcellie","delithery","amaracilli","perynaraci","gallantia","tressanton","daracei","callithath","amarcedda","balianne","lucilia","miaricei","sirdrah","korevesta","severry_an","calaraceir","ciarice","bry_annari","daraceline","drucretine","kercelores","carcellith","duveryshat","amariniarc","jerdrinara","callienn","alvithathe","fantintina","hantintia","adracelien","handricedd","salanta","dellantona","alandrevi","darceddarc","jercellita","treverista","antintony","calliaryss","sidomin","meriarcei","kagaminary","sirdrevesd","delory_ann","solita","darintona","kimberyl","kerry_anna","kacilarysh","therceddar","ballianne","drucillan"]
	oldnames = list(map(str.__add__,["o"]*len(names),names))
	newnames = list(map(str.__add__,["n"]*len(names),names))
	opart = list(map("dv.{0} AS {1}".format,names,oldnames))
	npart = list(map("chv.{0} AS {1}".format,names,newnames))
	coldef = ",".join(opart) + "," + ",".join(npart)
	plan = prepare('SELECT {coldef} FROM large_module_data_version($1) dv join large_module_changes_between_versions($1,$2) chv on (dv.uid = chv.uid)'.format(coldef = coldef))
	a = plan(x,y)

	for line in a:
		for old,new in zip(oldnames,newnames):
			if line[old] != line[new]:
				diffline = '{oldname}: "{oldvalue}", {newname}: "{newvalue}"'.format(
					oldname = old, newname = new,
					oldvalue = line[old], newvalue = line[new])
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
	plan = prepare("SELECT name FROM large_module_changes_between_versions($1,$2) WHERE dest_bit=B'1'")
	a = plan(x,y)

	for line in a:
		base = dict()
		base["command"] = "removeObject"
		base["kindName"] = "large_module"
		base["objectName"] = line["name"]
		yield base
$$
LANGUAGE python;

CREATE OR REPLACE FUNCTION diff_add(x bigint, y bigint)
RETURNS SETOF text
AS
$$
import Postgres
def genset(line,col):
	return	

@pytypes
def main(x,y):
	names = ["deveriary","delitheria","solianta","killitha","gallitherr","meriarceir","riondrinti","kercellien","severy_ant","adaryshath","perdracie","percellian","lillitheri","kiliarcedd","therdricel","soliandria","merintin","galith","bericelie","winiara","koretinta","keriarica","heddarist","basoniann","valitheria","kallintona","antonianne","ebondrice","jerdracill","wintia","jeria","calita","sidonian","chatherin","saliarcede","fandrianne","quellarin","ciantintia","kaliniann","dominianta","duverianna","kacelither","amariary_a","adressanto","therceiren","salliarica","alvithea","beriaracei","carcellie","delithery","amaracilli","perynaraci","gallantia","tressanton","daracei","callithath","amarcedda","balianne","lucilia","miaricei","sirdrah","korevesta","severry_an","calaraceir","ciarice","bry_annari","daraceline","drucretine","kercelores","carcellith","duveryshat","amariniarc","jerdrinara","callienn","alvithathe","fantintina","hantintia","adracelien","handricedd","salanta","dellantona","alandrevi","darceddarc","jercellita","treverista","antintony","calliaryss","sidomin","meriarcei","kagaminary","sirdrevesd","delory_ann","solita","darintona","kimberyl","kerry_anna","kacilarysh","therceddar","ballianne","drucillan"]
	npart = list(map("chv.{0} AS {0}".format,names))
	coldef = ",".join(npart)
	plan = prepare('SELECT {coldef},chv.name as name FROM large_module_data_version($1) AS dv RIGHT OUTER JOIN large_module_changes_between_versions($1,$2) AS chv ON dv.uid = chv.uid WHERE dv.name IS NULL'.format(coldef = coldef))
	a = plan(x,y)

	setlist = list()
	for line in a:
		yield '"command":"createObject", "kind": "large_module", "{0}": "{1}"'.format("create",line["name"])
		setlist.extend(['"command":"setAttribute", "kind": "large_module",, "{0}": "{1}"'.format(new,line[new]) for new in names])
	for x in setlist:
		yield x
		

$$
LANGUAGE python;

--select init_diff(30,60);
--select large_module_diff_set_attributes(30,60);
--drop table diff_data;

create or replace function large_module_diff_set_attributes(from_version bigint, to_version bigint)
returns setof diff_set_attribute_type
as
$$
begin
perform init_diff(from_version,to_version);
return query select * from large_module_diff_deleted();
end
$$
language plpgsql;
 
CREATE OR REPLACE FUNCTION 
large_module_diff_created()
RETURNS SETOF text
AS
$$
BEGIN
	RETURN QUERY SELECT new_name FROM diff_data WHERE old_name IS NULL AND new_dest_bit = '0';
END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION 
large_module_diff_deleted()
RETURNS SETOF text
AS
$$
BEGIN
	RETURN QUERY select old_name FROM diff_data WHERE new_dest_bit = '1';
END;
$$
LANGUAGE plpgsql; 

