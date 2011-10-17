--

set search_path to public, api, deska, genproc, versioning, production, history;

select startchangeset();
select service_add('www');
select service_add('dhcp');
select commitchangeset('1');

--select * from service

select startchangeset();
select host_add('hpv1');
--select host_set_service('hpv1', cast(array[] as text[]));
--select host_set_service('hpv1', null);
--select host_set_service('hpv1', array['www']);
select host_set_service_insert('hpv1', 'www');
select host_set_service_insert('hpv1', 'dhcp');
--select host_set_service_remove('hpv1', 'dhcp');
--select host_set_service_remove('hpv1', 'www');
--select host_resolved_object_data('hpv1');
select commitchangeset('2');

select startchangeset();
select host_set_service('hpv1', array['dhcp']);
--select host_resolved_object_data('hpv1');
select commitchangeset('3');
--select host_resolved_object_data('hpv1');
--select * from inner_host_template_service_multiRef_history
--select * from inner_host_service_multiRef_history
--SELECT service_get_name(service) FROM inner_host_service_multiRef WHERE host = 1 AND flag = '1'
--select get_current_changeset_or_null();
--SELECT service_get_name(service) FROM inner_host_service_multiRef_history WHERE host = 1 AND version = 5  AND flag = '1'
--select id2num(parent(5)); --5


select startchangeset();
select host_add('hpv8');
select host_set_service('hpv8', array['www']);
--select host_resolved_object_data('hpv8');
select commitchangeset('4');

select startchangeset();
select host_template_add('host_templ');
select host_add('hpv2');
select host_set_template_host('hpv2','host_templ');
select host_template_set_service('host_templ',array['www','dhcp']);
--select host_resolved_object_data('hpv2');
--select * from host_resolved_data();
--select * from host_template_resolved_data();
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

--select * from host_resolved_data_template_info();
SELECT uid,name,hardware,host_get_service(uid, 0) AS service,note_host, 
		CASE	WHEN hardware IS NULL THEN NULL
			ELSE name
		END AS hardware_templ,
		CASE	WHEN host_get_service(uid, 0) IS NULL THEN NULL
			ELSE name
		END AS service_templ,
		CASE	WHEN note_host IS NULL THEN NULL
			ELSE name
		END AS note_host_templ, template_host, template_host as orig_template
		FROM host_data_version(0)
--select * from host_resolved_data();
--select host_template_service_ref_set_coal(array['www'],1,0);
--select host_template_service_ref_set_coal(NULL,1,0);

select startchangeset();
select host_add('hpv3');
select host_set_template_host('hpv3','host_templ2');
select commitchangeset('8');

select startchangeset();
select host_template_add('host_templ3');
select host_template_set_service('host_templ3',array['ftp']);
select host_set_template_host('hpv3','host_templ3');
select commitchangeset('9');

select startchangeset();
select host_set_service_remove('hpv8', 'www');
--select host_resolved_object_data('hpv8');
--select host_get_data('hpv8');
--select 
select commitchangeset('10');


select host_resolved_data();
select host_template_resolved_data();
select host_get_data('hpv1');
select host_resolved_object_data('hpv1');

select startchangeset();
select host_add('hpv10');
select host_set_service('hpv10', cast(array[] as text[]));
select commitchangeset('12');

select startchangeset();
select host_set_service('hpv10', NULL);
--select * from host_resolved_data()
--select * from inner_host_service_multiref_history;
--select * from host_get_service(1);
--select * from inner_host_service_multiref_get_set(1);
--select * from inner_host_service_multiref_get_object_resolved_set(1);
select host_set_service_insert('hpv10', 'www');
select commitchangeset('12');


select host_init_diff(1,12);
select host_diff_refs_set_set_attributes(1,12);

select host_init_resolved_diff(1,12);

select host_terminate_diff();
select * from pg_temp_4.inner_host_service_multiref_diff_data;

select * from inner_host_service_multiref_get_object_resolved_set(2);



WITH RECURSIVE resolved_data AS(
SELECT s.host, s.service, h.template_host
FROM inner_host_service_multiRef_data_version() s, host_data_version() h WHERE (s.host = h.uid)

UNION ALL

SELECT rd.host AS host, s.service AS service, dv.template_host AS template_host
FROM template_data_version dv JOIN resolved_data rd ON (rd.template_host = dv.uid),
    inner_template_data_version s 
WHERE s.host NOT IN (SELECT host FROM resolved_data) AND dv.uid = s.host_template
)
SELECT service FROM resolved_data WHERE service IS NOT NULL;



SELECT schemaname||'.'||tablename
FROM pg_tables
WHERE has_table_privilege ('martina',schemaname||'.'||tablename,'select')
	AND schemaname NOT IN ('pg_catalog','information_schema');





    SELECT get_current_changeset() INTO ver;

    CREATE TEMP TABLE temp_host_template_data AS SELECT * FROM host_template_data_version();
    CREATE TEMP TABLE temp_host_data AS SELECT * FROM host_data_version();
    CREATE TEMP TABLE temp_inner_template_data AS SELECT * FROM inner_host_template_service_multiRef_data_version();
    CREATE TEMP TABLE temp_inner_data AS SELECT * FROM inner_host_service_multiRef_data_version();
    
--correction of templates' production   
--find all templates that were affected by some modification of template in current changeset
    CREATE TEMP TABLE affected_templates AS (
        WITH RECURSIVE resolved_data AS (
            (SELECT DISTINCT host_template as uid FROM inner_host_template_service_multiRef_history WHERE version = ver
            )
            UNION ALL
            SELECT dv.uid FROM temp_host_template_data dv, resolved_data rd WHERE dv.template_host = rd.uid
        )
        SELECT uid
        FROM resolved_data
    );

--delete from templated inner table which are templated by affected templates and has no own data
    DELETE FROM inner_host_template_service_multiRef WHERE host_template IN ( SELECT uid FROM affected_templates);

    INSERT INTO inner_host_template_service_multiRef (host_template, service)
        WITH RECURSIVE resolved_data AS(
            SELECT inner_templ.host_template AS uid, inner_templ.service, inner_templ.flag, dv.template_host AS template
            FROM inner_host_template_service_multiRef inner_templ
            --join is faster then IN operator, we neeed to resolve only affected templates
                LEFT OUTER JOIN affected_templates affected ON (inner_templ.host_template = affected.uid)
            --we need to know with which template is object templated, we should join this data
                LEFT OUTER JOIN temp_host_template_data dv ON (inner_templ.host_template = dv.uid)
            WHERE affected.uid IS NOT NULL
                
            UNION ALL
            
            SELECT rd.uid AS uid, s.service AS service, s.flag, dv.template_host AS template
            FROM temp_host_template_data dv JOIN resolved_data rd ON (rd.template = dv.uid)
                LEFT OUTER JOIN temp_inner_template_data s ON (rd.service IS NULL AND rd.flag = '0' AND dv.uid = s.host_template)
        )
        SELECT uid AS host_template, service FROM resolved_data WHERE flag = '1';
    
    CREATE TEMP TABLE temp_affected_host_template_data AS
    WITH RECURSIVE resolved_data AS(
    SELECT tdata.host_template AS host_template, tdata.service AS service, tdata.flag, affected.uid AS template
    FROM affected_templates affected 
        LEFT OUTER JOIN temp_inner_template_data tdata ON (affected.uid = tdata.host_template)
    WHERE tdata.flag = '0'
    
    UNION ALL
    
    SELECT rd.host_template AS host_template, s.service AS service, s.flag, dv.template_host AS template
    FROM temp_host_template_data dv JOIN resolved_data rd ON (rd.template = dv.uid)
        LEFT OUTER JOIN temp_inner_template_data s ON (rd.service IS NULL AND s.flag = '1' AND dv.uid = s.host_template)
    )
    SELECT host_template AS host_template, service AS service, flag FROM resolved_data WHERE (service IS NOT NULL OR host_template IS NOT NULL) AND template IS NULL;

    --correction of kind's production       
    --copy data with origin in inner table (not from template)

--objects that are templated by modified template and objects that where just added
    CREATE TEMP TABLE affected_objects AS (
        SELECT data.uid FROM temp_host_data data
            LEFT OUTER JOIN affected_templates templ ON (data.template_host = templ.uid)
            LEFT OUTER JOIN temp_inner_data idata ON (data.uid = idata.host)
        WHERE templ.uid IS NOT NULL AND idata.host IS NULL

        UNION

        SELECT h.uid FROM host_history h 
            LEFT OUTER JOIN (SELECT DISTINCT uid FROM host_history WHERE version = ver) inv ON (h.uid = inv.uid)
        WHERE inv.uid IS NOT NULL
        GROUP BY h.uid HAVING COUNT(*) = 1
    );

--delete all objects that has not its own data
    DELETE FROM inner_host_service_multiRef WHERE host IN ( SELECT affected.uid FROM affected_objects affected 
        LEFT OUTER JOIN temp_inner_data data ON (affected.uid = data.host) WHERE data.host IS NULL );

    --resolve inner data for all affected objects that has not its own data
    
    FOR host_uid IN (SELECT affected.uid FROM affected_objects affected 
        LEFT OUTER JOIN temp_inner_data data ON (affected.uid = data.host) WHERE data.host IS NULL) LOOP
        --templates are already resolved, we can use data from production.template
        SELECT template_host INTO host_template_uid FROM temp_host_data WHERE uid = host_uid;
        INSERT INTO inner_host_service_multiRef (host, service)
                SELECT host_uid AS host, service FROM inner_host_template_service_multiRef WHERE host_template = host_template_uid;
    END LOOP;

    DROP TABLE temp_host_template_data;
    DROP TABLE temp_host_data;
    DROP TABLE affected_templates;
    DROP TABLE affected_objects;
    DROP TABLE temp_inner_template_data;
    DROP TABLE temp_inner_data;
    DROP TABLE temp_affected_host_template_data;