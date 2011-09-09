--

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
--select host_resolved_object_data('hpv1');
select commitchangeset('2');

select startchangeset();
select host_set_service('hpv1', array['dhcp']);
--select host_resolved_object_data('hpv1');
select commitchangeset('3');
--select host_resolved_object_data('hpv1');

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

select startchangeset();
select host_add('hpv3');
select host_set_template_host('hpv3','host_templ2');
select commitchangeset('8');

select startchangeset();
select host_template_add('host_templ3');
select host_template_set_service('host_templ3',array['ftp']);
select host_set_template_host('hpv3','host_templ3');
select commitchangeset('9');


select host_resolved_data();
select host_template_resolved_data();
select host_get_data('hpv1');
select host_resolved_object_data('hpv1');