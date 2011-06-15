--only temporary file, work on committing templates
--change in template affects contain of another template and production tables

set search_path to deska, api, genproc, history, versioning, production;

SELECT startChangeset();
SELECT hardware_template_add('purch_templ');
SELECT hardware_template_set_purchase('purch_templ','2000-4-8');
SELECT hardware_template_add('purch_war_templ');
SELECT hardware_template_set_warranty('purch_war_templ','2008-4-8');
SELECT hardware_template_set_template('purch_war_templ','purch_templ');
SELECT commitChangeset('1');

SELECT startChangeset();
SELECT hardware_template_add('hw_ram_cpu_pw');
SELECT hardware_template_set_ram('hw_ram_cpu_pw','4000');
SELECT hardware_template_set_cpu_num('hw_ram_cpu_pw','4');
SELECT hardware_template_set_template('hw_ram_cpu_pw','purch_war_templ');
SELECT commitChangeset('1');

SELECT startChangeset();
SELECT hardware_template_add('hw_templ_all');
SELECT hardware_template_set_note('hw_templ_all','blabla note');
SELECT hardware_template_set_template('hw_templ_all','hw_ram_cpu_pw');
SELECT commitChangeset('3');

SELECT startChangeset();
SELECT hardware_template_add('purch_templ2');
SELECT hardware_template_set_purchase('purch_templ2','2011-4-8');
SELECT commitChangeset('1');


--select * from hardware_template_history;

SELECT startChangeset();
SELECT vendor_add('some_vendor');
SELECT hardware_add('hw_templ');
SELECT hardware_set_vendor('hw_templ','some_vendor');

SELECT hardware_set_template('hw_templ','hw_templ_all');
SELECT vendor_add('another_vendor');
SELECT hardware_add('hw_templ2');
SELECT hardware_set_vendor('hw_templ2','another_vendor');
SELECT hardware_set_template('hw_templ2','hw_ram_cpu_pw');
SELECT hardware_set_purchase('hw_templ2','2010-12-12');
SELECT hardware_set_warranty('hw_templ2','2016-12-12');

SELECT commitChangeset('1');

SELECT startChangeset();
SELECT hardware_add('hw_templted');
SELECT hardware_set_template('hw_templted','hw_templ_all');
SELECT commitChangeset('1');

select * from hardware;

SELECT startChangeset();
SELECT hardware_template_set_purchase('hw_templ_all','01-01-2010');
--select hardware_resolved_data('hw_templted');
SELECT commitChangeset('1');

SELECT startChangeset();
SELECT hardware_template_set_purchase('purch_templ','02-02-2002');
SELECT hardware_set_purchase('hw_templted','02-02-2002');
--select hardware_resolved_data('hw_templted');
SELECT commitChangeset('1');

CREATE FUNCTION
	hardware_template_commit()
	RETURNS integer
	AS
	$$
	DECLARE	ver bigint;
	BEGIN
		CREATE TEMP TABLE temp_hardware_template_current_changeset AS 
			WITH RECURSIVE resolved_data AS (
			SELECT warranty,purchase,vendor,cpu_num,ram,note, template,name,uid,version,dest_bit,template as orig_template
			FROM hardware_template_history
			WHERE version = get_current_changeset()
			UNION ALL
			SELECT
				COALESCE(rd.warranty,dv.warranty) AS warranty,
				COALESCE(rd.purchase,dv.purchase) AS purchase,
				COALESCE(rd.vendor,dv.vendor) AS vendor,
				COALESCE(rd.cpu_num,dv.cpu_num) AS cpu_num,
				COALESCE(rd.ram,dv.ram) AS ram,
				COALESCE(rd.note,dv.note) AS note,
				dv.template AS template, rd.name AS name, rd.uid AS uid , rd.version AS version, rd.dest_bit AS dest_bit, rd.orig_template AS orig_template
			FROM hardware_template_data_version() dv, resolved_data rd 
			WHERE dv.uid = rd.template
			)
			SELECT warranty,purchase,vendor,uid,cpu_num,ram,note,name, version,dest_bit, orig_template AS template
			FROM resolved_data WHERE template IS NULL;

			select * from temp_hardware_template_current_changeset;
			
		SELECT get_current_changeset() INTO ver;
		UPDATE hardware_template AS tbl SET warranty = new.warranty,purchase = new.purchase,vendor = new.vendor,uid = new.uid,cpu_num = new.cpu_num,ram = new.ram,note = new.note,template = new.template,name = new.name
			FROM temp_hardware_template_current_changeset as new
				WHERE tbl.uid = new.uid AND dest_bit = '0';
		INSERT INTO hardware_template (warranty,purchase,vendor,cpu_num,ram,note,name,uid,template)
			SELECT warranty,purchase,vendor,cpu_num,ram,note,name,uid,template FROM temp_hardware_template_current_changeset
				WHERE uid NOT IN ( SELECT uid FROM hardware_template ) AND dest_bit = '0';
		DELETE FROM hardware_template
			WHERE uid IN (SELECT uid FROM temp_hardware_template_current_changeset
				WHERE version = ver AND dest_bit = '1');
				
		DROP TABLE temp_hardware_template_current_changeset;

		--create temp table with uids of affected hardware_templates
		CREATE TEMP TABLE affected_templates AS (
			WITH RECURSIVE resolved_data AS (
				SELECT uid 
				FROM hardware_template_history
				WHERE version = current_changeset
				UNION ALL
				SELECT dv.uid
				FROM hardware_template_data_version() dv, resolved_data rd
				WHERE dv.template = rd.uid
				)
				SELECT uid
				FROM resolved_data
		);
		--hardware which is not modified in currentchangeset (is not updated by hardware_commit) and is templated by modified template, should be updated now
		UPDATE hardware AS tbl SET warranty = new.warranty,purchase = new.purchase,vendor = new.vendor,uid = new.uid,cpu_num = new.cpu_num,ram = new.ram,note = new.note,template = new.template,name = new.name
			FROM hardwar
		--update production.hardware as tbl set warrantty = new.warranty ... from resolved_data
		--update production.hardware_template
		
		RETURN 1;
	END
	$$
	LANGUAGE plpgsql SECURITY DEFINER;

WITH RECURSIVE affected_templates AS (
	SELECT uid
	FROM hardware_template_data_version()
	WHERE template = 1
	UNION ALL
	SELECT dv.uid
	FROM hardware_template_data_version() dv, affected_templates et
	WHERE dv.template = et.uid
	)
	SELECT *
	FROM affected_templates;


drop function hardware_to_commit();
create or replace function hardware_to_commit()
returns setof text
as
$$
DECLARE
	current_changeset bigint;
begin
current_changeset = get_current_changeset();
return query select name from hardware_data_version() where template in (
WITH RECURSIVE resolved_data AS (
	SELECT uid 
	FROM hardware_template_history
	WHERE version = current_changeset
	UNION ALL
	SELECT dv.uid
	FROM hardware_template_data_version() dv, resolved_data rd
	WHERE dv.template = rd.uid
	)
	SELECT uid
	FROM resolved_data
)
and uid not in (select uid from hardware_history where version = current_changeset);
end
$$
language plpgsql;

select hardware_to_commit();
select hardware_resolved_data(hardware_to_commit());
--works
select * from (select hardware_resolved_data(hardware_to_commit()))  as new;
hardware_to_commit()
select * from hardware_resolved_data('hw_templ');
