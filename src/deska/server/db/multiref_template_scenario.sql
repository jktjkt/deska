set search_path to public, api, deska, genproc, versioning, production, history;

select startchangeset();
select service_add('www');
select service_add('dhcp');
select commitchangeset('1');

select startchangeset();
select host_add('hpv1');
--select host_set_service('hpv1', cast(array[] as text[]));
--select host_set_service('hpv1', null);
--select host_set_service('hpv1', array['www']);
select host_set_service_insert('hpv1', 'www');
select host_set_service_insert('hpv1', 'dhcp');
--select host_set_service_remove('hpv1', 'dhcp');
--select host_set_service_remove('hpv1', 'www');
select commitchangeset('2');

select startchangeset();
select host_set_service('hpv1', array['dhcp']);
select commitchangeset('3');

select startchangeset();
select host_add('hpv8');
select host_set_service('hpv8', array['www']);
select commitchangeset('4');

select startchangeset();
select host_template_add('host_templ');
select host_add('hpv2');
select host_set_template_host('hpv2','host_templ');
select host_template_set_service('host_templ',array['www','dhcp']);
select commitchangeset('5');

--select * from inner_host_template_service_multiRef_history
--select * from inner_host_template_service_multiRef
--select * from host_template_resolved_data();

select startchangeset();
select host_template_add('host_templ2');
select host_template_set_template_host('host_templ2','host_templ');
select commitchangeset('6');

select startchangeset();
select service_add('ftp');
select host_template_set_service_insert('host_templ','ftp');
select commitchangeset('7');


select host_resolved_data();
select host_get_data('hpv1');

select * from host_get_data('hpv1');
select * from host_template_get_data('host_templ');
select * from inner_host_service_multiref_get_set(1)
SELECT count(*) FROM inner_host_service_multiRef_history WHERE host = 1 AND version = 2;
select * from inner_host_service_multiRef_history;
select host, service_get_name(service), version from inner_host_service_multiref_history;

drop table bla;

create table bla(
	a int,
	b int,
	constraint unique_a_b UNIQUE (a,b)
)

select get_current_changeset();
select * from changeset;
select * from inner_host_service_multiRef_history;
SELECT service_get_name(service) FROM inner_host_service_multiRef_history WHERE host = 1

insert into bla values(1,1);
insert into bla values(1,2);
insert into bla values(1,null);
insert into bla values(1,null);
insert into bla values(2,2);
insert into bla values(2,null);


select * from bla;
select a,b from bla where b is null;
select a,count(a) from bla where b is null group by a;

create type multiple_inner_type as (
	host bigint,
	service identifier
);

CREATE TYPE res_multiple_host_data_type AS
   ("name" deska.identifier,
    uid bigint,
    hardware bigint,
    service text[],
    note_host text,
    template_host bigint);

--id is id of base table which is templated and refers to the set
CREATE OR REPLACE FUNCTION refs_set_coal(old_array text[], new_obj_id bigint)
RETURNS text[]
AS
$$
BEGIN
--new_obj_id is always template
	raise notice 'set %', old_array;
	IF old_array IS NOT NULL THEN
		RETURN old_array;
	END IF;

	RETURN deska.ret_id_set(host_template_get_service(new_obj_id));
END;
$$
LANGUAGE plpgsql;


select * from get_templates_info();

select * from inner_host_template_service_multiref_history;
select * from inner_host_template_service_multiref;
select * from temp_host_template_data


CREATE OR REPLACE FUNCTION genproc.inner_host_template_service_multiref_commit()
  RETURNS integer AS
$$
DECLARE ver bigint;
    host_template_uid bigint;
    host_uid bigint;
    current_obj bigint;
BEGIN
	SELECT get_current_changeset() INTO ver;
	raise notice 'start of inner commit';
	CREATE TEMP TABLE temp_host_template_data AS SELECT * FROM host_template_data_version();
	CREATE TEMP TABLE temp_host_data AS SELECT * FROM host_data_version();
	CREATE TEMP TABLE temp_inner_template_data AS SELECT * FROM inner_host_template_service_multiref_data_version();
	
--correction of templates' production	
--find all templates that were affected by some modification of template in current changeset
	CREATE TEMP TABLE affected_templates AS (
		WITH RECURSIVE resolved_data AS (
			(select distinct host_template as uid from inner_host_template_service_multiref_history where version = ver
			)
			UNION ALL
			SELECT dv.uid FROM temp_host_template_data dv, resolved_data rd WHERE dv.template_host = rd.uid
		)
		SELECT uid
		FROM resolved_data

		UNION

		select h.uid from host_template_history h 
			left outer join (select distinct uid from host_template_history where version = ver) inv on (h.uid = inv.uid)
		where inv.uid is not null
		group by h.uid having count(*) = 1
	);
-- select * from affected_templates
-- select * from inner_host_template_service_multiRef
	DELETE FROM inner_host_template_service_multiRef WHERE host_template IN ( SELECT uid FROM affected_templates);
	FOR host_template_uid IN SELECT DISTINCT host_template FROM inner_host_template_service_multiRef_history WHERE version = ver LOOP
		INSERT INTO inner_host_template_service_multiRef (host_template, service) SELECT host_template, service FROM inner_host_template_service_multiRef_history WHERE version = ver AND host_template = host_template_uid;
	END LOOP;

	--for each affected template find its current data
	FOR host_template_uid IN SELECT uid FROM affected_templates LOOP
		current_obj = host_template_uid;		
		WHILE (current_obj IS NOT NULL) LOOP
			INSERT INTO inner_host_template_service_multiRef (host_template, service)
				SELECT host_template_uid AS host_template, service FROM temp_inner_template_data WHERE host_template = current_obj;
			IF exists(SELECT * FROM inner_host_template_service_multiRef WHERE host_template = host_template_uid) THEN
				EXIT;				
			ELSE
				SELECT template_host INTO current_obj FROM temp_host_template_data WHERE uid = current_obj;
			END IF;
		END LOOP;
	END LOOP;

	--correction of kind's production		
	--copy data with origin in inner table (not from template)
	DELETE FROM inner_host_service_multiRef WHERE host_template IN ( SELECT uid FROM affected_templates);
	FOR host_uid IN SELECT DISTINCT host FROM inner_host_service_multiRef_history WHERE version = ver LOOP
		INSERT INTO inner_host_service_multiRef (host, service) SELECT host, service FROM inner_host_service_multiRef_history WHERE version = ver AND host = host_uid;
	END LOOP;
	--for host where template_host in affected_templates

	CREATE TEMP TABLE affected_objects AS (
		SELECT uid FROM temp_host_data data
			LEFT OUTER JOIN affected_templates templ ON (data.template_host = templ.uid)
		WHERE templ.uid IS NOT NULL
	);
	FOR host_uid IN SELECT uid FROM affected_objects LOOP

	
	DROP TABLE temp_host_template_data;
	DROP TABLE temp_host_data;
	DROP TABLE affected_templates;
	DROP TABLE affected_objects;
	DROP TABLE temp_inner_template_data;
    RETURN 1;
END
$$
LANGUAGE plpgsql;

select * from affected_templates;
select * from temp_host_data;
select data.uid from temp_host_data data 
	join affected_templates affected on (data.template_host = affected.uid)
	left outer join (select distinct host from )



select * from host_template;
select * from temp_host_template_data;
SELECT * FROM temp_inner_template_data;
select * from affected_data_by_templ(1);
select * from  affected_templates;

CREATE OR REPLACE FUNCTION affected_data_by_templ(template_uid bigint)
RETURNS SETOF bigint
AS
$$
DECLARE
	dep_obj bigint;
	current_obj bigint;
	no_data boolean;
BEGIN
	CREATE TEMP TABLE temp_host_template_data AS SELECT * FROM host_template_data_version();
	CREATE TEMP TABLE temp_inner_template_data AS SELECT * FROM inner_host_template_service_multiref_data_version();
	CREATE TEMP TABLE affected_templates AS (
		WITH RECURSIVE resolved_data AS (
			select distinct uid from temp_host_template_data where template_host = template_uid
			UNION ALL
			SELECT dv.uid FROM temp_host_template_data dv, resolved_data rd WHERE dv.template_host = rd.uid
		)
		SELECT uid
		FROM resolved_data
	);	
	FOR dep_obj IN SELECT uid FROM affected_templates LOOP
		current_obj = dep_obj;		
		no_data = TRUE;
		WHILE (current_obj <> template_uid AND no_data) LOOP
			IF exists(SELECT * FROM temp_inner_template_data WHERE host_template = current_obj) THEN
				no_data = false;
			ELSE
				SELECT template_host INTO current_obj FROM temp_host_template_data WHERE uid = current_obj;
			END IF;
		END LOOP;
		
		IF current_obj = template_uid THEN			
			RETURN NEXT dep_obj;
		END IF;
	END LOOP;
	DROP TABLE temp_host_template_data;
	DROP TABLE temp_inner_template_data;
	DROP TABLE affected_templates;
END
$$
LANGUAGE plpgsql;


SELECT * FROM host_template;
SELECT * FROM host_template_history;
select uid from host_template_history group by uid having count(version) = 1
	
select h.uid from host_template_history h 
	left outer join (select distinct uid from host_template_history where version = 6) inv on (h.uid = inv.uid)
where inv.uid is not null
group by h.uid having count(*) = 1;