set search_path to deska, api, production, history, versioning, genproc;

create or replace function large_modul_data_version(data_version bigint)
returns setof large_modul_history
as
$$
declare
begin
	return query select h1.* from large_modul_history h1 
	join version v1 on (v1.id = h1.version)
	join (select uid, max(num) as maxnum 
		from large_modul_history h join version v on (v.id = h.version )
		where v.num <= data_version
		group by uid) vmax1 
	on (h1.uid = vmax1.uid and v1.num = vmax1.maxnum)
	WHERE dest_bit = '0';

end
$$
language plpgsql;

--select large_modul_data_version(30000);

create or replace function large_modul_changes_between_versions(from_version bigint, to_version bigint)
returns setof large_modul_history
as
$$
begin
	return query select h1.* from large_modul_history h1 
	join version v1 on (v1.id = h1.version)
	join (select uid, max(num) as maxnum 
		from large_modul_history h join version v on (v.id = h.version )
		where v.num <= to_version and v.num > from_version
		group by uid) vmax1 
	on (h1.uid = vmax1.uid and v1.num = vmax1.maxnum);
end
$$
language plpgsql;

--select large_modul_changes_between_versions(30000,60000);

DROP TYPE genproc.diff_set_attribute_type cascade;

CREATE TYPE genproc.diff_set_attribute_type AS
   (objkind "name",
    "objname" text,
    command text,
    attribute "name",
    olddata text,
    newdata text);


CREATE OR REPLACE FUNCTION 
	large_modul_diff_set_attributes()
	 RETURNS SETOF diff_set_attribute_type
	 AS
	 $$
	 DECLARE
		  old_data large_modul_history%rowtype;
		  new_data large_modul_history%rowtype;
		  result diff_set_attribute_type;
	 BEGIN
		  result.command = 'setAttribute';
		  result.objkind = 'large_modul';
		  FOR old_data.deveriary, old_data.delitheria, old_data.solianta, old_data.killitha, old_data.gallitherr, old_data.meriarceir, old_data.riondrinti, old_data.kercellien, old_data.severy_ant, old_data.adaryshath, old_data.perdracie, old_data.percellian, old_data.lillitheri, old_data.kiliarcedd, old_data.therdricel, old_data.soliandria, old_data.merintin, old_data.galith, old_data.bericelie, old_data.winiara, old_data.koretinta, old_data.name, old_data.keriarica, old_data.heddarist, old_data.basoniann, old_data.valitheria, old_data.kallintona, old_data.antonianne, old_data.ebondrice, old_data.jerdracill, old_data.wintia, old_data.jeria, old_data.calita, old_data.sidonian, old_data.chatherin, old_data.saliarcede, old_data.fandrianne, old_data.quellarin, old_data.ciantintia, old_data.kaliniann, old_data.dominianta, old_data.duverianna, old_data.kacelither, old_data.amariary_a, old_data.adressanto, old_data.therceiren, old_data.salliarica, old_data.alvithea, old_data.beriaracei, old_data.carcellie, old_data.delithery, old_data.amaracilli, old_data.perynaraci, old_data.gallantia, old_data.tressanton, old_data.daracei, old_data.callithath, old_data.amarcedda, old_data.balianne, old_data.lucilia, old_data.miaricei, old_data.sirdrah, old_data.korevesta, old_data.severry_an, old_data.calaraceir, old_data.ciarice, old_data.bry_annari, old_data.daraceline, old_data.drucretine, old_data.kercelores, old_data.carcellith, old_data.duveryshat, old_data.amariniarc, old_data.jerdrinara, old_data.callienn, old_data.alvithathe, old_data.fantintina, old_data.hantintia, old_data.adracelien, old_data.handricedd, old_data.salanta, old_data.dellantona, old_data.alandrevi, old_data.darceddarc, old_data.jercellita, old_data.treverista, old_data.antintony, old_data.calliaryss, old_data.sidomin, old_data.meriarcei, old_data.kagaminary, old_data.sirdrevesd, old_data.delory_ann, old_data.solita, old_data.darintona, old_data.kimberyl, old_data.kerry_anna, old_data.kacilarysh, old_data.therceddar, old_data.ballianne, old_data.drucillan,
		  new_data.deveriary, new_data.delitheria, new_data.solianta, new_data.killitha, new_data.gallitherr, new_data.meriarceir, new_data.riondrinti, new_data.kercellien, new_data.severy_ant, new_data.adaryshath, new_data.perdracie, new_data.percellian, new_data.lillitheri, new_data.kiliarcedd, new_data.therdricel, new_data.soliandria, new_data.merintin, new_data.galith, new_data.bericelie, new_data.winiara, new_data.koretinta, new_data.name, new_data.keriarica, new_data.heddarist, new_data.basoniann, new_data.valitheria, new_data.kallintona, new_data.antonianne, new_data.ebondrice, new_data.jerdracill, new_data.wintia, new_data.jeria, new_data.calita, new_data.sidonian, new_data.chatherin, new_data.saliarcede, new_data.fandrianne, new_data.quellarin, new_data.ciantintia, new_data.kaliniann, new_data.dominianta, new_data.duverianna, new_data.kacelither, new_data.amariary_a, new_data.adressanto, new_data.therceiren, new_data.salliarica, new_data.alvithea, new_data.beriaracei, new_data.carcellie, new_data.delithery, new_data.amaracilli, new_data.perynaraci, new_data.gallantia, new_data.tressanton, new_data.daracei, new_data.callithath, new_data.amarcedda, new_data.balianne, new_data.lucilia, new_data.miaricei, new_data.sirdrah, new_data.korevesta, new_data.severry_an, new_data.calaraceir, new_data.ciarice, new_data.bry_annari, new_data.daraceline, new_data.drucretine, new_data.kercelores, new_data.carcellith, new_data.duveryshat, new_data.amariniarc, new_data.jerdrinara, new_data.callienn, new_data.alvithathe, new_data.fantintina, new_data.hantintia, new_data.adracelien, new_data.handricedd, new_data.salanta, new_data.dellantona, new_data.alandrevi, new_data.darceddarc, new_data.jercellita, new_data.treverista, new_data.antintony, new_data.calliaryss, new_data.sidomin, new_data.meriarcei, new_data.kagaminary, new_data.sirdrevesd, new_data.delory_ann, new_data.solita, new_data.darintona, new_data.kimberyl, new_data.kerry_anna, new_data.kacilarysh, new_data.therceddar, new_data.ballianne, new_data.drucillan
			 IN 
			SELECT old_deveriary, old_delitheria, old_solianta, old_killitha, old_gallitherr, old_meriarceir, old_riondrinti, old_kercellien, old_severy_ant, old_adaryshath, old_perdracie, old_percellian, old_lillitheri, old_kiliarcedd, old_therdricel, old_soliandria, old_merintin, old_galith, old_bericelie, old_winiara, old_koretinta, old_name, old_keriarica, old_heddarist, old_basoniann, old_valitheria, old_kallintona, old_antonianne, old_ebondrice, old_jerdracill, old_wintia, old_jeria, old_calita, old_sidonian, old_chatherin, old_saliarcede, old_fandrianne, old_quellarin, old_ciantintia, old_kaliniann, old_dominianta, old_duverianna, old_kacelither, old_amariary_a, old_adressanto, old_therceiren, old_salliarica, old_alvithea, old_beriaracei, old_carcellie, old_delithery, old_amaracilli, old_perynaraci, old_gallantia, old_tressanton, old_daracei, old_callithath, old_amarcedda, old_balianne, old_lucilia, old_miaricei, old_sirdrah, old_korevesta, old_severry_an, old_calaraceir, old_ciarice, old_bry_annari, old_daraceline, old_drucretine, old_kercelores, old_carcellith, old_duveryshat, old_amariniarc, old_jerdrinara, old_callienn, old_alvithathe, old_fantintina, old_hantintia, old_adracelien, old_handricedd, old_salanta, old_dellantona, old_alandrevi, old_darceddarc, old_jercellita, old_treverista, old_antintony, old_calliaryss, old_sidomin, old_meriarcei, old_kagaminary, old_sirdrevesd, old_delory_ann, old_solita, old_darintona, old_kimberyl, old_kerry_anna, old_kacilarysh, old_therceddar, old_ballianne, old_drucillan,
				new_deveriary, new_delitheria, new_solianta, new_killitha, new_gallitherr, new_meriarceir, new_riondrinti, new_kercellien, new_severy_ant, new_adaryshath, new_perdracie, new_percellian, new_lillitheri, new_kiliarcedd, new_therdricel, new_soliandria, new_merintin, new_galith, new_bericelie, new_winiara, new_koretinta, new_name, new_keriarica, new_heddarist, new_basoniann, new_valitheria, new_kallintona, new_antonianne, new_ebondrice, new_jerdracill, new_wintia, new_jeria, new_calita, new_sidonian, new_chatherin, new_saliarcede, new_fandrianne, new_quellarin, new_ciantintia, new_kaliniann, new_dominianta, new_duverianna, new_kacelither, new_amariary_a, new_adressanto, new_therceiren, new_salliarica, new_alvithea, new_beriaracei, new_carcellie, new_delithery, new_amaracilli, new_perynaraci, new_gallantia, new_tressanton, new_daracei, new_callithath, new_amarcedda, new_balianne, new_lucilia, new_miaricei, new_sirdrah, new_korevesta, new_severry_an, new_calaraceir, new_ciarice, new_bry_annari, new_daraceline, new_drucretine, new_kercelores, new_carcellith, new_duveryshat, new_amariniarc, new_jerdrinara, new_callienn, new_alvithathe, new_fantintina, new_hantintia, new_adracelien, new_handricedd, new_salanta, new_dellantona, new_alandrevi, new_darceddarc, new_jercellita, new_treverista, new_antintony, new_calliaryss, new_sidomin, new_meriarcei, new_kagaminary, new_sirdrevesd, new_delory_ann, new_solita, new_darintona, new_kimberyl, new_kerry_anna, new_kacilarysh, new_therceddar, new_ballianne, new_drucillan
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
				
	 IF (old_data.daracei <> new_data.daracei) OR ((old_data.daracei IS NULL OR new_data.daracei IS NULL) 
		  AND NOT(old_data.daracei IS NULL AND new_data.daracei IS NULL))
	 THEN
		  result.attribute = 'daracei';
		  result.olddata = old_data.daracei;
		  result.newdata = new_data.daracei;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.deveriary <> new_data.deveriary) OR ((old_data.deveriary IS NULL OR new_data.deveriary IS NULL) 
		  AND NOT(old_data.deveriary IS NULL AND new_data.deveriary IS NULL))
	 THEN
		  result.attribute = 'deveriary';
		  result.olddata = old_data.deveriary;
		  result.newdata = new_data.deveriary;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.delitheria <> new_data.delitheria) OR ((old_data.delitheria IS NULL OR new_data.delitheria IS NULL) 
		  AND NOT(old_data.delitheria IS NULL AND new_data.delitheria IS NULL))
	 THEN
		  result.attribute = 'delitheria';
		  result.olddata = old_data.delitheria;
		  result.newdata = new_data.delitheria;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.solianta <> new_data.solianta) OR ((old_data.solianta IS NULL OR new_data.solianta IS NULL) 
		  AND NOT(old_data.solianta IS NULL AND new_data.solianta IS NULL))
	 THEN
		  result.attribute = 'solianta';
		  result.olddata = old_data.solianta;
		  result.newdata = new_data.solianta;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.balianne <> new_data.balianne) OR ((old_data.balianne IS NULL OR new_data.balianne IS NULL) 
		  AND NOT(old_data.balianne IS NULL AND new_data.balianne IS NULL))
	 THEN
		  result.attribute = 'balianne';
		  result.olddata = old_data.balianne;
		  result.newdata = new_data.balianne;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.lucilia <> new_data.lucilia) OR ((old_data.lucilia IS NULL OR new_data.lucilia IS NULL) 
		  AND NOT(old_data.lucilia IS NULL AND new_data.lucilia IS NULL))
	 THEN
		  result.attribute = 'lucilia';
		  result.olddata = old_data.lucilia;
		  result.newdata = new_data.lucilia;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.miaricei <> new_data.miaricei) OR ((old_data.miaricei IS NULL OR new_data.miaricei IS NULL) 
		  AND NOT(old_data.miaricei IS NULL AND new_data.miaricei IS NULL))
	 THEN
		  result.attribute = 'miaricei';
		  result.olddata = old_data.miaricei;
		  result.newdata = new_data.miaricei;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.killitha <> new_data.killitha) OR ((old_data.killitha IS NULL OR new_data.killitha IS NULL) 
		  AND NOT(old_data.killitha IS NULL AND new_data.killitha IS NULL))
	 THEN
		  result.attribute = 'killitha';
		  result.olddata = old_data.killitha;
		  result.newdata = new_data.killitha;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.gallitherr <> new_data.gallitherr) OR ((old_data.gallitherr IS NULL OR new_data.gallitherr IS NULL) 
		  AND NOT(old_data.gallitherr IS NULL AND new_data.gallitherr IS NULL))
	 THEN
		  result.attribute = 'gallitherr';
		  result.olddata = old_data.gallitherr;
		  result.newdata = new_data.gallitherr;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.korevesta <> new_data.korevesta) OR ((old_data.korevesta IS NULL OR new_data.korevesta IS NULL) 
		  AND NOT(old_data.korevesta IS NULL AND new_data.korevesta IS NULL))
	 THEN
		  result.attribute = 'korevesta';
		  result.olddata = old_data.korevesta;
		  result.newdata = new_data.korevesta;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.severry_an <> new_data.severry_an) OR ((old_data.severry_an IS NULL OR new_data.severry_an IS NULL) 
		  AND NOT(old_data.severry_an IS NULL AND new_data.severry_an IS NULL))
	 THEN
		  result.attribute = 'severry_an';
		  result.olddata = old_data.severry_an;
		  result.newdata = new_data.severry_an;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.calaraceir <> new_data.calaraceir) OR ((old_data.calaraceir IS NULL OR new_data.calaraceir IS NULL) 
		  AND NOT(old_data.calaraceir IS NULL AND new_data.calaraceir IS NULL))
	 THEN
		  result.attribute = 'calaraceir';
		  result.olddata = old_data.calaraceir;
		  result.newdata = new_data.calaraceir;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.beriaracei <> new_data.beriaracei) OR ((old_data.beriaracei IS NULL OR new_data.beriaracei IS NULL) 
		  AND NOT(old_data.beriaracei IS NULL AND new_data.beriaracei IS NULL))
	 THEN
		  result.attribute = 'beriaracei';
		  result.olddata = old_data.beriaracei;
		  result.newdata = new_data.beriaracei;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.meriarceir <> new_data.meriarceir) OR ((old_data.meriarceir IS NULL OR new_data.meriarceir IS NULL) 
		  AND NOT(old_data.meriarceir IS NULL AND new_data.meriarceir IS NULL))
	 THEN
		  result.attribute = 'meriarceir';
		  result.olddata = old_data.meriarceir;
		  result.newdata = new_data.meriarceir;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.riondrinti <> new_data.riondrinti) OR ((old_data.riondrinti IS NULL OR new_data.riondrinti IS NULL) 
		  AND NOT(old_data.riondrinti IS NULL AND new_data.riondrinti IS NULL))
	 THEN
		  result.attribute = 'riondrinti';
		  result.olddata = old_data.riondrinti;
		  result.newdata = new_data.riondrinti;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.callithath <> new_data.callithath) OR ((old_data.callithath IS NULL OR new_data.callithath IS NULL) 
		  AND NOT(old_data.callithath IS NULL AND new_data.callithath IS NULL))
	 THEN
		  result.attribute = 'callithath';
		  result.olddata = old_data.callithath;
		  result.newdata = new_data.callithath;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.kercellien <> new_data.kercellien) OR ((old_data.kercellien IS NULL OR new_data.kercellien IS NULL) 
		  AND NOT(old_data.kercellien IS NULL AND new_data.kercellien IS NULL))
	 THEN
		  result.attribute = 'kercellien';
		  result.olddata = old_data.kercellien;
		  result.newdata = new_data.kercellien;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.severy_ant <> new_data.severy_ant) OR ((old_data.severy_ant IS NULL OR new_data.severy_ant IS NULL) 
		  AND NOT(old_data.severy_ant IS NULL AND new_data.severy_ant IS NULL))
	 THEN
		  result.attribute = 'severy_ant';
		  result.olddata = old_data.severy_ant;
		  result.newdata = new_data.severy_ant;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.daraceline <> new_data.daraceline) OR ((old_data.daraceline IS NULL OR new_data.daraceline IS NULL) 
		  AND NOT(old_data.daraceline IS NULL AND new_data.daraceline IS NULL))
	 THEN
		  result.attribute = 'daraceline';
		  result.olddata = old_data.daraceline;
		  result.newdata = new_data.daraceline;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.adaryshath <> new_data.adaryshath) OR ((old_data.adaryshath IS NULL OR new_data.adaryshath IS NULL) 
		  AND NOT(old_data.adaryshath IS NULL AND new_data.adaryshath IS NULL))
	 THEN
		  result.attribute = 'adaryshath';
		  result.olddata = old_data.adaryshath;
		  result.newdata = new_data.adaryshath;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.amarcedda <> new_data.amarcedda) OR ((old_data.amarcedda IS NULL OR new_data.amarcedda IS NULL) 
		  AND NOT(old_data.amarcedda IS NULL AND new_data.amarcedda IS NULL))
	 THEN
		  result.attribute = 'amarcedda';
		  result.olddata = old_data.amarcedda;
		  result.newdata = new_data.amarcedda;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.perdracie <> new_data.perdracie) OR ((old_data.perdracie IS NULL OR new_data.perdracie IS NULL) 
		  AND NOT(old_data.perdracie IS NULL AND new_data.perdracie IS NULL))
	 THEN
		  result.attribute = 'perdracie';
		  result.olddata = old_data.perdracie;
		  result.newdata = new_data.perdracie;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.drucretine <> new_data.drucretine) OR ((old_data.drucretine IS NULL OR new_data.drucretine IS NULL) 
		  AND NOT(old_data.drucretine IS NULL AND new_data.drucretine IS NULL))
	 THEN
		  result.attribute = 'drucretine';
		  result.olddata = old_data.drucretine;
		  result.newdata = new_data.drucretine;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.percellian <> new_data.percellian) OR ((old_data.percellian IS NULL OR new_data.percellian IS NULL) 
		  AND NOT(old_data.percellian IS NULL AND new_data.percellian IS NULL))
	 THEN
		  result.attribute = 'percellian';
		  result.olddata = old_data.percellian;
		  result.newdata = new_data.percellian;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.lillitheri <> new_data.lillitheri) OR ((old_data.lillitheri IS NULL OR new_data.lillitheri IS NULL) 
		  AND NOT(old_data.lillitheri IS NULL AND new_data.lillitheri IS NULL))
	 THEN
		  result.attribute = 'lillitheri';
		  result.olddata = old_data.lillitheri;
		  result.newdata = new_data.lillitheri;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.kiliarcedd <> new_data.kiliarcedd) OR ((old_data.kiliarcedd IS NULL OR new_data.kiliarcedd IS NULL) 
		  AND NOT(old_data.kiliarcedd IS NULL AND new_data.kiliarcedd IS NULL))
	 THEN
		  result.attribute = 'kiliarcedd';
		  result.olddata = old_data.kiliarcedd;
		  result.newdata = new_data.kiliarcedd;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.kercelores <> new_data.kercelores) OR ((old_data.kercelores IS NULL OR new_data.kercelores IS NULL) 
		  AND NOT(old_data.kercelores IS NULL AND new_data.kercelores IS NULL))
	 THEN
		  result.attribute = 'kercelores';
		  result.olddata = old_data.kercelores;
		  result.newdata = new_data.kercelores;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.carcellith <> new_data.carcellith) OR ((old_data.carcellith IS NULL OR new_data.carcellith IS NULL) 
		  AND NOT(old_data.carcellith IS NULL AND new_data.carcellith IS NULL))
	 THEN
		  result.attribute = 'carcellith';
		  result.olddata = old_data.carcellith;
		  result.newdata = new_data.carcellith;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.therdricel <> new_data.therdricel) OR ((old_data.therdricel IS NULL OR new_data.therdricel IS NULL) 
		  AND NOT(old_data.therdricel IS NULL AND new_data.therdricel IS NULL))
	 THEN
		  result.attribute = 'therdricel';
		  result.olddata = old_data.therdricel;
		  result.newdata = new_data.therdricel;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.soliandria <> new_data.soliandria) OR ((old_data.soliandria IS NULL OR new_data.soliandria IS NULL) 
		  AND NOT(old_data.soliandria IS NULL AND new_data.soliandria IS NULL))
	 THEN
		  result.attribute = 'soliandria';
		  result.olddata = old_data.soliandria;
		  result.newdata = new_data.soliandria;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.duveryshat <> new_data.duveryshat) OR ((old_data.duveryshat IS NULL OR new_data.duveryshat IS NULL) 
		  AND NOT(old_data.duveryshat IS NULL AND new_data.duveryshat IS NULL))
	 THEN
		  result.attribute = 'duveryshat';
		  result.olddata = old_data.duveryshat;
		  result.newdata = new_data.duveryshat;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.merintin <> new_data.merintin) OR ((old_data.merintin IS NULL OR new_data.merintin IS NULL) 
		  AND NOT(old_data.merintin IS NULL AND new_data.merintin IS NULL))
	 THEN
		  result.attribute = 'merintin';
		  result.olddata = old_data.merintin;
		  result.newdata = new_data.merintin;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.galith <> new_data.galith) OR ((old_data.galith IS NULL OR new_data.galith IS NULL) 
		  AND NOT(old_data.galith IS NULL AND new_data.galith IS NULL))
	 THEN
		  result.attribute = 'galith';
		  result.olddata = old_data.galith;
		  result.newdata = new_data.galith;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.bericelie <> new_data.bericelie) OR ((old_data.bericelie IS NULL OR new_data.bericelie IS NULL) 
		  AND NOT(old_data.bericelie IS NULL AND new_data.bericelie IS NULL))
	 THEN
		  result.attribute = 'bericelie';
		  result.olddata = old_data.bericelie;
		  result.newdata = new_data.bericelie;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.winiara <> new_data.winiara) OR ((old_data.winiara IS NULL OR new_data.winiara IS NULL) 
		  AND NOT(old_data.winiara IS NULL AND new_data.winiara IS NULL))
	 THEN
		  result.attribute = 'winiara';
		  result.olddata = old_data.winiara;
		  result.newdata = new_data.winiara;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.koretinta <> new_data.koretinta) OR ((old_data.koretinta IS NULL OR new_data.koretinta IS NULL) 
		  AND NOT(old_data.koretinta IS NULL AND new_data.koretinta IS NULL))
	 THEN
		  result.attribute = 'koretinta';
		  result.olddata = old_data.koretinta;
		  result.newdata = new_data.koretinta;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.amariniarc <> new_data.amariniarc) OR ((old_data.amariniarc IS NULL OR new_data.amariniarc IS NULL) 
		  AND NOT(old_data.amariniarc IS NULL AND new_data.amariniarc IS NULL))
	 THEN
		  result.attribute = 'amariniarc';
		  result.olddata = old_data.amariniarc;
		  result.newdata = new_data.amariniarc;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.jerdrinara <> new_data.jerdrinara) OR ((old_data.jerdrinara IS NULL OR new_data.jerdrinara IS NULL) 
		  AND NOT(old_data.jerdrinara IS NULL AND new_data.jerdrinara IS NULL))
	 THEN
		  result.attribute = 'jerdrinara';
		  result.olddata = old_data.jerdrinara;
		  result.newdata = new_data.jerdrinara;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.keriarica <> new_data.keriarica) OR ((old_data.keriarica IS NULL OR new_data.keriarica IS NULL) 
		  AND NOT(old_data.keriarica IS NULL AND new_data.keriarica IS NULL))
	 THEN
		  result.attribute = 'keriarica';
		  result.olddata = old_data.keriarica;
		  result.newdata = new_data.keriarica;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.alvithathe <> new_data.alvithathe) OR ((old_data.alvithathe IS NULL OR new_data.alvithathe IS NULL) 
		  AND NOT(old_data.alvithathe IS NULL AND new_data.alvithathe IS NULL))
	 THEN
		  result.attribute = 'alvithathe';
		  result.olddata = old_data.alvithathe;
		  result.newdata = new_data.alvithathe;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.heddarist <> new_data.heddarist) OR ((old_data.heddarist IS NULL OR new_data.heddarist IS NULL) 
		  AND NOT(old_data.heddarist IS NULL AND new_data.heddarist IS NULL))
	 THEN
		  result.attribute = 'heddarist';
		  result.olddata = old_data.heddarist;
		  result.newdata = new_data.heddarist;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.basoniann <> new_data.basoniann) OR ((old_data.basoniann IS NULL OR new_data.basoniann IS NULL) 
		  AND NOT(old_data.basoniann IS NULL AND new_data.basoniann IS NULL))
	 THEN
		  result.attribute = 'basoniann';
		  result.olddata = old_data.basoniann;
		  result.newdata = new_data.basoniann;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.amariary_a <> new_data.amariary_a) OR ((old_data.amariary_a IS NULL OR new_data.amariary_a IS NULL) 
		  AND NOT(old_data.amariary_a IS NULL AND new_data.amariary_a IS NULL))
	 THEN
		  result.attribute = 'amariary_a';
		  result.olddata = old_data.amariary_a;
		  result.newdata = new_data.amariary_a;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.hantintia <> new_data.hantintia) OR ((old_data.hantintia IS NULL OR new_data.hantintia IS NULL) 
		  AND NOT(old_data.hantintia IS NULL AND new_data.hantintia IS NULL))
	 THEN
		  result.attribute = 'hantintia';
		  result.olddata = old_data.hantintia;
		  result.newdata = new_data.hantintia;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.adracelien <> new_data.adracelien) OR ((old_data.adracelien IS NULL OR new_data.adracelien IS NULL) 
		  AND NOT(old_data.adracelien IS NULL AND new_data.adracelien IS NULL))
	 THEN
		  result.attribute = 'adracelien';
		  result.olddata = old_data.adracelien;
		  result.newdata = new_data.adracelien;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.valitheria <> new_data.valitheria) OR ((old_data.valitheria IS NULL OR new_data.valitheria IS NULL) 
		  AND NOT(old_data.valitheria IS NULL AND new_data.valitheria IS NULL))
	 THEN
		  result.attribute = 'valitheria';
		  result.olddata = old_data.valitheria;
		  result.newdata = new_data.valitheria;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.callienn <> new_data.callienn) OR ((old_data.callienn IS NULL OR new_data.callienn IS NULL) 
		  AND NOT(old_data.callienn IS NULL AND new_data.callienn IS NULL))
	 THEN
		  result.attribute = 'callienn';
		  result.olddata = old_data.callienn;
		  result.newdata = new_data.callienn;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.handricedd <> new_data.handricedd) OR ((old_data.handricedd IS NULL OR new_data.handricedd IS NULL) 
		  AND NOT(old_data.handricedd IS NULL AND new_data.handricedd IS NULL))
	 THEN
		  result.attribute = 'handricedd';
		  result.olddata = old_data.handricedd;
		  result.newdata = new_data.handricedd;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.kallintona <> new_data.kallintona) OR ((old_data.kallintona IS NULL OR new_data.kallintona IS NULL) 
		  AND NOT(old_data.kallintona IS NULL AND new_data.kallintona IS NULL))
	 THEN
		  result.attribute = 'kallintona';
		  result.olddata = old_data.kallintona;
		  result.newdata = new_data.kallintona;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.antonianne <> new_data.antonianne) OR ((old_data.antonianne IS NULL OR new_data.antonianne IS NULL) 
		  AND NOT(old_data.antonianne IS NULL AND new_data.antonianne IS NULL))
	 THEN
		  result.attribute = 'antonianne';
		  result.olddata = old_data.antonianne;
		  result.newdata = new_data.antonianne;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.ebondrice <> new_data.ebondrice) OR ((old_data.ebondrice IS NULL OR new_data.ebondrice IS NULL) 
		  AND NOT(old_data.ebondrice IS NULL AND new_data.ebondrice IS NULL))
	 THEN
		  result.attribute = 'ebondrice';
		  result.olddata = old_data.ebondrice;
		  result.newdata = new_data.ebondrice;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.salanta <> new_data.salanta) OR ((old_data.salanta IS NULL OR new_data.salanta IS NULL) 
		  AND NOT(old_data.salanta IS NULL AND new_data.salanta IS NULL))
	 THEN
		  result.attribute = 'salanta';
		  result.olddata = old_data.salanta;
		  result.newdata = new_data.salanta;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.dellantona <> new_data.dellantona) OR ((old_data.dellantona IS NULL OR new_data.dellantona IS NULL) 
		  AND NOT(old_data.dellantona IS NULL AND new_data.dellantona IS NULL))
	 THEN
		  result.attribute = 'dellantona';
		  result.olddata = old_data.dellantona;
		  result.newdata = new_data.dellantona;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.alandrevi <> new_data.alandrevi) OR ((old_data.alandrevi IS NULL OR new_data.alandrevi IS NULL) 
		  AND NOT(old_data.alandrevi IS NULL AND new_data.alandrevi IS NULL))
	 THEN
		  result.attribute = 'alandrevi';
		  result.olddata = old_data.alandrevi;
		  result.newdata = new_data.alandrevi;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.sirdrah <> new_data.sirdrah) OR ((old_data.sirdrah IS NULL OR new_data.sirdrah IS NULL) 
		  AND NOT(old_data.sirdrah IS NULL AND new_data.sirdrah IS NULL))
	 THEN
		  result.attribute = 'sirdrah';
		  result.olddata = old_data.sirdrah;
		  result.newdata = new_data.sirdrah;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.darceddarc <> new_data.darceddarc) OR ((old_data.darceddarc IS NULL OR new_data.darceddarc IS NULL) 
		  AND NOT(old_data.darceddarc IS NULL AND new_data.darceddarc IS NULL))
	 THEN
		  result.attribute = 'darceddarc';
		  result.olddata = old_data.darceddarc;
		  result.newdata = new_data.darceddarc;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.jerdracill <> new_data.jerdracill) OR ((old_data.jerdracill IS NULL OR new_data.jerdracill IS NULL) 
		  AND NOT(old_data.jerdracill IS NULL AND new_data.jerdracill IS NULL))
	 THEN
		  result.attribute = 'jerdracill';
		  result.olddata = old_data.jerdracill;
		  result.newdata = new_data.jerdracill;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.jercellita <> new_data.jercellita) OR ((old_data.jercellita IS NULL OR new_data.jercellita IS NULL) 
		  AND NOT(old_data.jercellita IS NULL AND new_data.jercellita IS NULL))
	 THEN
		  result.attribute = 'jercellita';
		  result.olddata = old_data.jercellita;
		  result.newdata = new_data.jercellita;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.wintia <> new_data.wintia) OR ((old_data.wintia IS NULL OR new_data.wintia IS NULL) 
		  AND NOT(old_data.wintia IS NULL AND new_data.wintia IS NULL))
	 THEN
		  result.attribute = 'wintia';
		  result.olddata = old_data.wintia;
		  result.newdata = new_data.wintia;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.jeria <> new_data.jeria) OR ((old_data.jeria IS NULL OR new_data.jeria IS NULL) 
		  AND NOT(old_data.jeria IS NULL AND new_data.jeria IS NULL))
	 THEN
		  result.attribute = 'jeria';
		  result.olddata = old_data.jeria;
		  result.newdata = new_data.jeria;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.treverista <> new_data.treverista) OR ((old_data.treverista IS NULL OR new_data.treverista IS NULL) 
		  AND NOT(old_data.treverista IS NULL AND new_data.treverista IS NULL))
	 THEN
		  result.attribute = 'treverista';
		  result.olddata = old_data.treverista;
		  result.newdata = new_data.treverista;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.calita <> new_data.calita) OR ((old_data.calita IS NULL OR new_data.calita IS NULL) 
		  AND NOT(old_data.calita IS NULL AND new_data.calita IS NULL))
	 THEN
		  result.attribute = 'calita';
		  result.olddata = old_data.calita;
		  result.newdata = new_data.calita;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.sidonian <> new_data.sidonian) OR ((old_data.sidonian IS NULL OR new_data.sidonian IS NULL) 
		  AND NOT(old_data.sidonian IS NULL AND new_data.sidonian IS NULL))
	 THEN
		  result.attribute = 'sidonian';
		  result.olddata = old_data.sidonian;
		  result.newdata = new_data.sidonian;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.chatherin <> new_data.chatherin) OR ((old_data.chatherin IS NULL OR new_data.chatherin IS NULL) 
		  AND NOT(old_data.chatherin IS NULL AND new_data.chatherin IS NULL))
	 THEN
		  result.attribute = 'chatherin';
		  result.olddata = old_data.chatherin;
		  result.newdata = new_data.chatherin;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.antintony <> new_data.antintony) OR ((old_data.antintony IS NULL OR new_data.antintony IS NULL) 
		  AND NOT(old_data.antintony IS NULL AND new_data.antintony IS NULL))
	 THEN
		  result.attribute = 'antintony';
		  result.olddata = old_data.antintony;
		  result.newdata = new_data.antintony;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.calliaryss <> new_data.calliaryss) OR ((old_data.calliaryss IS NULL OR new_data.calliaryss IS NULL) 
		  AND NOT(old_data.calliaryss IS NULL AND new_data.calliaryss IS NULL))
	 THEN
		  result.attribute = 'calliaryss';
		  result.olddata = old_data.calliaryss;
		  result.newdata = new_data.calliaryss;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.sidomin <> new_data.sidomin) OR ((old_data.sidomin IS NULL OR new_data.sidomin IS NULL) 
		  AND NOT(old_data.sidomin IS NULL AND new_data.sidomin IS NULL))
	 THEN
		  result.attribute = 'sidomin';
		  result.olddata = old_data.sidomin;
		  result.newdata = new_data.sidomin;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.saliarcede <> new_data.saliarcede) OR ((old_data.saliarcede IS NULL OR new_data.saliarcede IS NULL) 
		  AND NOT(old_data.saliarcede IS NULL AND new_data.saliarcede IS NULL))
	 THEN
		  result.attribute = 'saliarcede';
		  result.olddata = old_data.saliarcede;
		  result.newdata = new_data.saliarcede;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.fandrianne <> new_data.fandrianne) OR ((old_data.fandrianne IS NULL OR new_data.fandrianne IS NULL) 
		  AND NOT(old_data.fandrianne IS NULL AND new_data.fandrianne IS NULL))
	 THEN
		  result.attribute = 'fandrianne';
		  result.olddata = old_data.fandrianne;
		  result.newdata = new_data.fandrianne;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.meriarcei <> new_data.meriarcei) OR ((old_data.meriarcei IS NULL OR new_data.meriarcei IS NULL) 
		  AND NOT(old_data.meriarcei IS NULL AND new_data.meriarcei IS NULL))
	 THEN
		  result.attribute = 'meriarcei';
		  result.olddata = old_data.meriarcei;
		  result.newdata = new_data.meriarcei;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.quellarin <> new_data.quellarin) OR ((old_data.quellarin IS NULL OR new_data.quellarin IS NULL) 
		  AND NOT(old_data.quellarin IS NULL AND new_data.quellarin IS NULL))
	 THEN
		  result.attribute = 'quellarin';
		  result.olddata = old_data.quellarin;
		  result.newdata = new_data.quellarin;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.kagaminary <> new_data.kagaminary) OR ((old_data.kagaminary IS NULL OR new_data.kagaminary IS NULL) 
		  AND NOT(old_data.kagaminary IS NULL AND new_data.kagaminary IS NULL))
	 THEN
		  result.attribute = 'kagaminary';
		  result.olddata = old_data.kagaminary;
		  result.newdata = new_data.kagaminary;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.sirdrevesd <> new_data.sirdrevesd) OR ((old_data.sirdrevesd IS NULL OR new_data.sirdrevesd IS NULL) 
		  AND NOT(old_data.sirdrevesd IS NULL AND new_data.sirdrevesd IS NULL))
	 THEN
		  result.attribute = 'sirdrevesd';
		  result.olddata = old_data.sirdrevesd;
		  result.newdata = new_data.sirdrevesd;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.ciantintia <> new_data.ciantintia) OR ((old_data.ciantintia IS NULL OR new_data.ciantintia IS NULL) 
		  AND NOT(old_data.ciantintia IS NULL AND new_data.ciantintia IS NULL))
	 THEN
		  result.attribute = 'ciantintia';
		  result.olddata = old_data.ciantintia;
		  result.newdata = new_data.ciantintia;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.kaliniann <> new_data.kaliniann) OR ((old_data.kaliniann IS NULL OR new_data.kaliniann IS NULL) 
		  AND NOT(old_data.kaliniann IS NULL AND new_data.kaliniann IS NULL))
	 THEN
		  result.attribute = 'kaliniann';
		  result.olddata = old_data.kaliniann;
		  result.newdata = new_data.kaliniann;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.delory_ann <> new_data.delory_ann) OR ((old_data.delory_ann IS NULL OR new_data.delory_ann IS NULL) 
		  AND NOT(old_data.delory_ann IS NULL AND new_data.delory_ann IS NULL))
	 THEN
		  result.attribute = 'delory_ann';
		  result.olddata = old_data.delory_ann;
		  result.newdata = new_data.delory_ann;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.dominianta <> new_data.dominianta) OR ((old_data.dominianta IS NULL OR new_data.dominianta IS NULL) 
		  AND NOT(old_data.dominianta IS NULL AND new_data.dominianta IS NULL))
	 THEN
		  result.attribute = 'dominianta';
		  result.olddata = old_data.dominianta;
		  result.newdata = new_data.dominianta;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.duverianna <> new_data.duverianna) OR ((old_data.duverianna IS NULL OR new_data.duverianna IS NULL) 
		  AND NOT(old_data.duverianna IS NULL AND new_data.duverianna IS NULL))
	 THEN
		  result.attribute = 'duverianna';
		  result.olddata = old_data.duverianna;
		  result.newdata = new_data.duverianna;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.kacelither <> new_data.kacelither) OR ((old_data.kacelither IS NULL OR new_data.kacelither IS NULL) 
		  AND NOT(old_data.kacelither IS NULL AND new_data.kacelither IS NULL))
	 THEN
		  result.attribute = 'kacelither';
		  result.olddata = old_data.kacelither;
		  result.newdata = new_data.kacelither;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.solita <> new_data.solita) OR ((old_data.solita IS NULL OR new_data.solita IS NULL) 
		  AND NOT(old_data.solita IS NULL AND new_data.solita IS NULL))
	 THEN
		  result.attribute = 'solita';
		  result.olddata = old_data.solita;
		  result.newdata = new_data.solita;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.fantintina <> new_data.fantintina) OR ((old_data.fantintina IS NULL OR new_data.fantintina IS NULL) 
		  AND NOT(old_data.fantintina IS NULL AND new_data.fantintina IS NULL))
	 THEN
		  result.attribute = 'fantintina';
		  result.olddata = old_data.fantintina;
		  result.newdata = new_data.fantintina;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.adressanto <> new_data.adressanto) OR ((old_data.adressanto IS NULL OR new_data.adressanto IS NULL) 
		  AND NOT(old_data.adressanto IS NULL AND new_data.adressanto IS NULL))
	 THEN
		  result.attribute = 'adressanto';
		  result.olddata = old_data.adressanto;
		  result.newdata = new_data.adressanto;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.therceiren <> new_data.therceiren) OR ((old_data.therceiren IS NULL OR new_data.therceiren IS NULL) 
		  AND NOT(old_data.therceiren IS NULL AND new_data.therceiren IS NULL))
	 THEN
		  result.attribute = 'therceiren';
		  result.olddata = old_data.therceiren;
		  result.newdata = new_data.therceiren;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.salliarica <> new_data.salliarica) OR ((old_data.salliarica IS NULL OR new_data.salliarica IS NULL) 
		  AND NOT(old_data.salliarica IS NULL AND new_data.salliarica IS NULL))
	 THEN
		  result.attribute = 'salliarica';
		  result.olddata = old_data.salliarica;
		  result.newdata = new_data.salliarica;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.alvithea <> new_data.alvithea) OR ((old_data.alvithea IS NULL OR new_data.alvithea IS NULL) 
		  AND NOT(old_data.alvithea IS NULL AND new_data.alvithea IS NULL))
	 THEN
		  result.attribute = 'alvithea';
		  result.olddata = old_data.alvithea;
		  result.newdata = new_data.alvithea;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.kimberyl <> new_data.kimberyl) OR ((old_data.kimberyl IS NULL OR new_data.kimberyl IS NULL) 
		  AND NOT(old_data.kimberyl IS NULL AND new_data.kimberyl IS NULL))
	 THEN
		  result.attribute = 'kimberyl';
		  result.olddata = old_data.kimberyl;
		  result.newdata = new_data.kimberyl;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.kerry_anna <> new_data.kerry_anna) OR ((old_data.kerry_anna IS NULL OR new_data.kerry_anna IS NULL) 
		  AND NOT(old_data.kerry_anna IS NULL AND new_data.kerry_anna IS NULL))
	 THEN
		  result.attribute = 'kerry_anna';
		  result.olddata = old_data.kerry_anna;
		  result.newdata = new_data.kerry_anna;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.ciarice <> new_data.ciarice) OR ((old_data.ciarice IS NULL OR new_data.ciarice IS NULL) 
		  AND NOT(old_data.ciarice IS NULL AND new_data.ciarice IS NULL))
	 THEN
		  result.attribute = 'ciarice';
		  result.olddata = old_data.ciarice;
		  result.newdata = new_data.ciarice;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.carcellie <> new_data.carcellie) OR ((old_data.carcellie IS NULL OR new_data.carcellie IS NULL) 
		  AND NOT(old_data.carcellie IS NULL AND new_data.carcellie IS NULL))
	 THEN
		  result.attribute = 'carcellie';
		  result.olddata = old_data.carcellie;
		  result.newdata = new_data.carcellie;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.darintona <> new_data.darintona) OR ((old_data.darintona IS NULL OR new_data.darintona IS NULL) 
		  AND NOT(old_data.darintona IS NULL AND new_data.darintona IS NULL))
	 THEN
		  result.attribute = 'darintona';
		  result.olddata = old_data.darintona;
		  result.newdata = new_data.darintona;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.bry_annari <> new_data.bry_annari) OR ((old_data.bry_annari IS NULL OR new_data.bry_annari IS NULL) 
		  AND NOT(old_data.bry_annari IS NULL AND new_data.bry_annari IS NULL))
	 THEN
		  result.attribute = 'bry_annari';
		  result.olddata = old_data.bry_annari;
		  result.newdata = new_data.bry_annari;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.delithery <> new_data.delithery) OR ((old_data.delithery IS NULL OR new_data.delithery IS NULL) 
		  AND NOT(old_data.delithery IS NULL AND new_data.delithery IS NULL))
	 THEN
		  result.attribute = 'delithery';
		  result.olddata = old_data.delithery;
		  result.newdata = new_data.delithery;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.amaracilli <> new_data.amaracilli) OR ((old_data.amaracilli IS NULL OR new_data.amaracilli IS NULL) 
		  AND NOT(old_data.amaracilli IS NULL AND new_data.amaracilli IS NULL))
	 THEN
		  result.attribute = 'amaracilli';
		  result.olddata = old_data.amaracilli;
		  result.newdata = new_data.amaracilli;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.kacilarysh <> new_data.kacilarysh) OR ((old_data.kacilarysh IS NULL OR new_data.kacilarysh IS NULL) 
		  AND NOT(old_data.kacilarysh IS NULL AND new_data.kacilarysh IS NULL))
	 THEN
		  result.attribute = 'kacilarysh';
		  result.olddata = old_data.kacilarysh;
		  result.newdata = new_data.kacilarysh;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.perynaraci <> new_data.perynaraci) OR ((old_data.perynaraci IS NULL OR new_data.perynaraci IS NULL) 
		  AND NOT(old_data.perynaraci IS NULL AND new_data.perynaraci IS NULL))
	 THEN
		  result.attribute = 'perynaraci';
		  result.olddata = old_data.perynaraci;
		  result.newdata = new_data.perynaraci;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.therceddar <> new_data.therceddar) OR ((old_data.therceddar IS NULL OR new_data.therceddar IS NULL) 
		  AND NOT(old_data.therceddar IS NULL AND new_data.therceddar IS NULL))
	 THEN
		  result.attribute = 'therceddar';
		  result.olddata = old_data.therceddar;
		  result.newdata = new_data.therceddar;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.ballianne <> new_data.ballianne) OR ((old_data.ballianne IS NULL OR new_data.ballianne IS NULL) 
		  AND NOT(old_data.ballianne IS NULL AND new_data.ballianne IS NULL))
	 THEN
		  result.attribute = 'ballianne';
		  result.olddata = old_data.ballianne;
		  result.newdata = new_data.ballianne;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.gallantia <> new_data.gallantia) OR ((old_data.gallantia IS NULL OR new_data.gallantia IS NULL) 
		  AND NOT(old_data.gallantia IS NULL AND new_data.gallantia IS NULL))
	 THEN
		  result.attribute = 'gallantia';
		  result.olddata = old_data.gallantia;
		  result.newdata = new_data.gallantia;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.tressanton <> new_data.tressanton) OR ((old_data.tressanton IS NULL OR new_data.tressanton IS NULL) 
		  AND NOT(old_data.tressanton IS NULL AND new_data.tressanton IS NULL))
	 THEN
		  result.attribute = 'tressanton';
		  result.olddata = old_data.tressanton;
		  result.newdata = new_data.tressanton;
		  RETURN NEXT result;			
	 END IF;
	 

	 IF (old_data.drucillan <> new_data.drucillan) OR ((old_data.drucillan IS NULL OR new_data.drucillan IS NULL) 
		  AND NOT(old_data.drucillan IS NULL AND new_data.drucillan IS NULL))
	 THEN
		  result.attribute = 'drucillan';
		  result.olddata = old_data.drucillan;
		  result.newdata = new_data.drucillan;
		  RETURN NEXT result;			
	 END IF;
	 

		  END LOOP;	
	 END
	 $$
	 LANGUAGE plpgsql; 


create or replace function large_modul_init_diff(from_version bigint, to_version bigint)
returns void as
$$
declare
begin
	create temp table diff_data 
	AS select dv.deveriary AS old_deveriary, dv.delitheria AS old_delitheria, dv.solianta AS old_solianta, dv.killitha AS old_killitha, dv.gallitherr AS old_gallitherr, dv.meriarceir AS old_meriarceir, dv.riondrinti AS old_riondrinti, dv.kercellien AS old_kercellien, dv.severy_ant AS old_severy_ant, dv.adaryshath AS old_adaryshath, dv.perdracie AS old_perdracie, dv.percellian AS old_percellian, dv.lillitheri AS old_lillitheri, dv.kiliarcedd AS old_kiliarcedd, dv.therdricel AS old_therdricel, dv.soliandria AS old_soliandria, dv.merintin AS old_merintin, dv.galith AS old_galith, dv.bericelie AS old_bericelie, dv.winiara AS old_winiara, dv.koretinta AS old_koretinta, dv.name AS old_name, dv.keriarica AS old_keriarica, dv.heddarist AS old_heddarist, dv.basoniann AS old_basoniann, dv.valitheria AS old_valitheria, dv.kallintona AS old_kallintona, dv.antonianne AS old_antonianne, dv.ebondrice AS old_ebondrice, dv.jerdracill AS old_jerdracill, dv.wintia AS old_wintia, dv.jeria AS old_jeria, dv.calita AS old_calita, dv.sidonian AS old_sidonian, dv.chatherin AS old_chatherin, dv.saliarcede AS old_saliarcede, dv.fandrianne AS old_fandrianne, dv.quellarin AS old_quellarin, dv.ciantintia AS old_ciantintia, dv.kaliniann AS old_kaliniann, dv.dominianta AS old_dominianta, dv.duverianna AS old_duverianna, dv.kacelither AS old_kacelither, dv.amariary_a AS old_amariary_a, dv.adressanto AS old_adressanto, dv.therceiren AS old_therceiren, dv.salliarica AS old_salliarica, dv.alvithea AS old_alvithea, dv.beriaracei AS old_beriaracei, dv.carcellie AS old_carcellie, dv.delithery AS old_delithery, dv.amaracilli AS old_amaracilli, dv.perynaraci AS old_perynaraci, dv.gallantia AS old_gallantia, dv.tressanton AS old_tressanton, dv.daracei AS old_daracei, dv.callithath AS old_callithath, dv.amarcedda AS old_amarcedda, dv.balianne AS old_balianne, dv.lucilia AS old_lucilia, dv.miaricei AS old_miaricei, dv.sirdrah AS old_sirdrah, dv.korevesta AS old_korevesta, dv.severry_an AS old_severry_an, dv.calaraceir AS old_calaraceir, dv.ciarice AS old_ciarice, dv.bry_annari AS old_bry_annari, dv.daraceline AS old_daraceline, dv.drucretine AS old_drucretine, dv.kercelores AS old_kercelores, dv.carcellith AS old_carcellith, dv.duveryshat AS old_duveryshat, dv.amariniarc AS old_amariniarc, dv.jerdrinara AS old_jerdrinara, dv.callienn AS old_callienn, dv.alvithathe AS old_alvithathe, dv.fantintina AS old_fantintina, dv.hantintia AS old_hantintia, dv.adracelien AS old_adracelien, dv.handricedd AS old_handricedd, dv.salanta AS old_salanta, dv.dellantona AS old_dellantona, dv.alandrevi AS old_alandrevi, dv.darceddarc AS old_darceddarc, dv.jercellita AS old_jercellita, dv.treverista AS old_treverista, dv.antintony AS old_antintony, dv.calliaryss AS old_calliaryss, dv.sidomin AS old_sidomin, dv.meriarcei AS old_meriarcei, dv.kagaminary AS old_kagaminary, dv.sirdrevesd AS old_sirdrevesd, dv.delory_ann AS old_delory_ann, dv.solita AS old_solita, dv.darintona AS old_darintona, dv.kimberyl AS old_kimberyl, dv.kerry_anna AS old_kerry_anna, dv.kacilarysh AS old_kacilarysh, dv.therceddar AS old_therceddar, dv.ballianne AS old_ballianne, dv.drucillan AS old_drucillan,dv.dest_bit as old_dest_bit,
		chv.deveriary AS new_deveriary, chv.delitheria AS new_delitheria, chv.solianta AS new_solianta, chv.killitha AS new_killitha, chv.gallitherr AS new_gallitherr, chv.meriarceir AS new_meriarceir, chv.riondrinti AS new_riondrinti, chv.kercellien AS new_kercellien, chv.severy_ant AS new_severy_ant, chv.adaryshath AS new_adaryshath, chv.perdracie AS new_perdracie, chv.percellian AS new_percellian, chv.lillitheri AS new_lillitheri, chv.kiliarcedd AS new_kiliarcedd, chv.therdricel AS new_therdricel, chv.soliandria AS new_soliandria, chv.merintin AS new_merintin, chv.galith AS new_galith, chv.bericelie AS new_bericelie, chv.winiara AS new_winiara, chv.koretinta AS new_koretinta, chv.name AS new_name, chv.keriarica AS new_keriarica, chv.heddarist AS new_heddarist, chv.basoniann AS new_basoniann, chv.valitheria AS new_valitheria, chv.kallintona AS new_kallintona, chv.antonianne AS new_antonianne, chv.ebondrice AS new_ebondrice, chv.jerdracill AS new_jerdracill, chv.wintia AS new_wintia, chv.jeria AS new_jeria, chv.calita AS new_calita, chv.sidonian AS new_sidonian, chv.chatherin AS new_chatherin, chv.saliarcede AS new_saliarcede, chv.fandrianne AS new_fandrianne, chv.quellarin AS new_quellarin, chv.ciantintia AS new_ciantintia, chv.kaliniann AS new_kaliniann, chv.dominianta AS new_dominianta, chv.duverianna AS new_duverianna, chv.kacelither AS new_kacelither, chv.amariary_a AS new_amariary_a, chv.adressanto AS new_adressanto, chv.therceiren AS new_therceiren, chv.salliarica AS new_salliarica, chv.alvithea AS new_alvithea, chv.beriaracei AS new_beriaracei, chv.carcellie AS new_carcellie, chv.delithery AS new_delithery, chv.amaracilli AS new_amaracilli, chv.perynaraci AS new_perynaraci, chv.gallantia AS new_gallantia, chv.tressanton AS new_tressanton, chv.daracei AS new_daracei, chv.callithath AS new_callithath, chv.amarcedda AS new_amarcedda, chv.balianne AS new_balianne, chv.lucilia AS new_lucilia, chv.miaricei AS new_miaricei, chv.sirdrah AS new_sirdrah, chv.korevesta AS new_korevesta, chv.severry_an AS new_severry_an, chv.calaraceir AS new_calaraceir, chv.ciarice AS new_ciarice, chv.bry_annari AS new_bry_annari, chv.daraceline AS new_daraceline, chv.drucretine AS new_drucretine, chv.kercelores AS new_kercelores, chv.carcellith AS new_carcellith, chv.duveryshat AS new_duveryshat, chv.amariniarc AS new_amariniarc, chv.jerdrinara AS new_jerdrinara, chv.callienn AS new_callienn, chv.alvithathe AS new_alvithathe, chv.fantintina AS new_fantintina, chv.hantintia AS new_hantintia, chv.adracelien AS new_adracelien, chv.handricedd AS new_handricedd, chv.salanta AS new_salanta, chv.dellantona AS new_dellantona, chv.alandrevi AS new_alandrevi, chv.darceddarc AS new_darceddarc, chv.jercellita AS new_jercellita, chv.treverista AS new_treverista, chv.antintony AS new_antintony, chv.calliaryss AS new_calliaryss, chv.sidomin AS new_sidomin, chv.meriarcei AS new_meriarcei, chv.kagaminary AS new_kagaminary, chv.sirdrevesd AS new_sirdrevesd, chv.delory_ann AS new_delory_ann, chv.solita AS new_solita, chv.darintona AS new_darintona, chv.kimberyl AS new_kimberyl, chv.kerry_anna AS new_kerry_anna, chv.kacilarysh AS new_kacilarysh, chv.therceddar AS new_therceddar, chv.ballianne AS new_ballianne, chv.drucillan AS new_drucillan,chv.dest_bit as new_dest_bit
		from large_modul_data_version(from_version) dv full outer join large_modul_changes_between_versions(from_version,to_version) chv on (dv.uid = chv.uid);
end
$$
language plpgsql;

CREATE OR REPLACE FUNCTION lm_diff_set(x bigint, y bigint)
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
	plan = prepare('SELECT {coldef} FROM large_modul_data_version($1) dv join large_modul_changes_between_versions($1,$2) chv on (dv.uid = chv.uid)'.format(coldef = coldef))
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


drop function lm_diff_del(bigint,bigint);
CREATE OR REPLACE FUNCTION lm_diff_del(x bigint, y bigint)
RETURNS SETOF text
AS
$$
import Postgres
@pytypes
def main(x,y):
	plan = prepare("SELECT name FROM large_modul_changes_between_versions($1,$2) WHERE dest_bit=B'1'")
	a = plan(x,y)

	for line in a:
		base = dict()
		base["command"] = "removeObject"
		base["kindName"] = "large_modul"
		base["objectName"] = line["name"]
		yield base
$$
LANGUAGE python;

CREATE OR REPLACE FUNCTION lm_diff_add(x bigint, y bigint)
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
	plan = prepare('SELECT {coldef},chv.name as name FROM large_modul_data_version($1) AS dv RIGHT OUTER JOIN large_modul_changes_between_versions($1,$2) AS chv ON dv.uid = chv.uid WHERE dv.name IS NULL'.format(coldef = coldef))
	a = plan(x,y)

	setlist = list()
	for line in a:
		yield '"command":"createObject", "kind": "large_modul", "{0}": "{1}"'.format("create",line["name"])
		setlist.extend(['"command":"setAttribute", "kind": "large_modul",, "{0}": "{1}"'.format(new,line[new]) for new in names])
	for x in setlist:
		yield x
		

$$
LANGUAGE python;

CREATE OR REPLACE FUNCTION 
large_modul_diff_created()
RETURNS SETOF text
AS
$$
BEGIN
	RETURN QUERY SELECT new_name FROM diff_data WHERE old_name IS NULL AND new_dest_bit = '0';
END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION 
large_modul_diff_deleted()
RETURNS SETOF text
AS
$$
BEGIN
	RETURN QUERY select old_name FROM diff_data WHERE new_dest_bit = '1';
END;
$$
LANGUAGE plpgsql; 

