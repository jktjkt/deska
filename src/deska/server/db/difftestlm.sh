time psql -d deska_dev luke -c "
SET search_path TO api,genproc,history,deska,versioning,production;
select * from large_modul_init_diff($1,$2);
select * from large_modul_diff_created();
select * from large_modul_diff_set_attributes();
select * from large_modul_diff_deleted();
" > test.out1

time echo '{"command": "large_modul_init_diff", "from":'$1', "to":'$2'}
{"command": "large_modul_diff_created"}
{"command": "large_modul_diff_set_attributes"}
{"command": "large_modul_diff_deleted"}' | python ../app/deska_server.py > out

time psql -d deska_dev luke -c "
SET search_path TO api,genproc,history,deska,versioning,production;
select * from lm_diff_add($1,$2);
select * from lm_diff_set($1,$2);
select * from lm_diff_del($1,$2);
" > test.out2

time echo '{"command": "lm_diff_add", "from":'$1', "to":'$2'}
{"command": "lm_diff_set", "from":'$1', "to":'$2'}
{"command": "lm_diff_del", "from":'$1', "to":'$2'}' | python ../app/deska_server.py > out
