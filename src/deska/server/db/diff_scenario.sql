 
set search_path to deska, api, genproc, history, versioning, production;

SELECT startChangeset();
SELECT vendor_add('Casonarcei_91');
SELECT hardware_add('Wintia_276');
SELECT hardware_set_vendor('Wintia_276','Casonarcei_91');
SELECT hardware_set_purchase('Wintia_276','2000-4-8');
SELECT hardware_set_warranty('Wintia_276','2014-7-25');
SELECT commitChangeset('1');

SELECT startChangeset();
SELECT vendor_add('Gallandre_273');
SELECT hardware_add('Amariniarc_809');
SELECT hardware_set_vendor('Amariniarc_809','Gallandre_273');
SELECT hardware_set_purchase('Amariniarc_809','2008-9-13');
SELECT hardware_set_warranty('Amariniarc_809','2011-6-12');
SELECT hardware_add('Newone');
SELECT hardware_set_vendor('Newone','Casonarcei_91');
SELECT hardware_set_purchase('Newone','2000-4-8');
SELECT hardware_set_warranty('Newone','2014-7-25');
SELECT commitChangeset('2');

SELECT startChangeset();
SELECT vendor_add('Dabriary_88');
SELECT hardware_add('Kercellien_647');
SELECT hardware_set_vendor('Kercellien_647','Dabriary_88');
SELECT hardware_set_purchase('Kercellien_647','2007-10-16');
SELECT hardware_set_warranty('Kercellien_647','2011-3-16');
SELECT commitChangeset('3');

SELECT startChangeset();
SELECT hardware_add('Antonianne_976');
SELECT hardware_set_vendor('Antonianne_976','Gallandre_273');
SELECT hardware_set_purchase('Antonianne_976','2007-11-3');
SELECT hardware_set_warranty('Antonianne_976','2012-2-6');
SELECT hardware_add('Tressanton_344');
SELECT hardware_set_vendor('Tressanton_344','Gallandre_273');
SELECT hardware_set_purchase('Tressanton_344','2008-8-19');
SELECT hardware_set_warranty('Tressanton_344','2011-11-12');
SELECT commitChangeset('4');

SELECT startChangeset();
SELECT host_add('Caliandria_328');
SELECT host_set_hardware('Caliandria_328','Tressanton_344');
SELECT commitChangeset('5');

SELECT startChangeset();
SELECT host_add('Meriandres_208');
SELECT host_set_hardware('Meriandres_208','Antonianne_976');
SELECT host_add('Delliara_980');
SELECT host_set_hardware('Delliara_980','Amariniarc_809');
SELECT commitChangeset('6');

SELECT startChangeset();
SELECT host_add('Jetinarced_651');
SELECT host_set_hardware('Jetinarced_651','Kercellien_647');
SELECT commitChangeset('7');

SELECT startChangeset();
SELECT host_add('Quelitherc_872');
SELECT host_set_hardware('Quelitherc_872','Kercellien_647');
SELECT commitChangeset('8');



--select * from host_history;
--select * from host;
--select * from interface_history;
--select * from interface;


select startchangeset();
	select interface_add('Caliandria_328->eth0');
select commitchangeset('9');

select startchangeset();
	select interface_add('Caliandria_328->eth1');
	select interface_add('Quelitherc_872->eth0');
select commitchangeset('10');

select startchangeset();
	select interface_del('Caliandria_328->eth1');
select commitchangeset('11');

select startchangeset();
	SELECT hardware_del('Wintia_276');
select commitchangeset('12');

SELECT startChangeset();
	SELECT hardware_del('Newone');
	SELECT hardware_add('Martina_007');
	SELECT hardware_set_vendor('Martina_007','Gallandre_273');
	SELECT hardware_set_purchase('Martina_007','2005-8-19');
	SELECT hardware_set_warranty('Martina_007','2014-6-18');
SELECT commitChangeset('13');

--part for testing diffs in opened changeset
-- select startchangeset();
-- 	SELECT vendor_add('JustVendor');
-- 	SELECT hardware_add('JustHW');
-- 	SELECT hardware_set_vendor('JustHW','JustVendor');
-- 	SELECT hardware_set_purchase('JustHW','2001-8-19');
-- 	SELECT hardware_set_warranty('JustHW','2005-6-18');
-- 	SELECT hardware_set_vendor('Martina_007','JustVendor');
-- --dont use commit before test
-- --SELECT commitChangeset('end');

SELECT startChangeset();
	SELECT hardware_set_name('Martina_007', 'Olderone');
SELECT commitChangeset('13');


--select * from hardware_init_diff(13,15);
--select * from hardware_diff_created();
--select * from hardware_diff_deleted();
--select * from hardware_diff_set_attributes(13,15);
--select * from hardware_diff_rename();
--select * from hardware_terminate_diff();

--select * from hardware_diff_data;

--select * from hardware_init_diff(14,15);
--select * from hardware_diff_created();
--select * from hardware_diff_deleted();
--select * from hardware_diff_set_attributes(14,15);
--select * from hardware_diff_rename();
--select * from hardware_terminate_diff();
