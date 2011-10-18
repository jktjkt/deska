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
--select * from inner_host_service_multiRef
--select * from host_get_service(3);
--select * from host_template_get_service(1);
--select * from host_history where uid = 3;
--select * from host_template_resolved_data();

select startchangeset();
select host_template_add('host_templ2');
select host_template_set_template_host('host_templ2','host_templ');
select commitchangeset('6');

select startchangeset();
select service_add('ftp');
select host_template_set_service_insert('host_templ','ftp');
select commitchangeset('7');

--select * from host_resolved_object_data_template_info('hpv2');
--select * from inner_host_service_multiref_get_object_resolved_set(3);
--select * from host_resolved_data_template_info();
    
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

