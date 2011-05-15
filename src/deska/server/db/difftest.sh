time psql deska_dev luke -c "
SET search_path TO api,genproc,history,deska,versioning,production;
select * from init_diff($1,$2);
select * from hardware_diff_created();
select * from hardware_diff_set_attributes();
select * from hardware_diff_deleted();
" > test.out1

time echo '{"command": "init_diff", "from":'$1', "to":'$2'}
{"command": "hardware_diff_created"}
{"command": "hardware_diff_set_attributes"}
{"command": "hardware_diff_deleted"}' | python ../app/deska_server.py > out

time psql deska_dev luke -c "
SET search_path TO api,genproc,history,deska,versioning,production;
select * from diff_add($1,$2);
select * from diff_set($1,$2);
select * from diff_del($1,$2);
" > test.out2

time echo '{"command": "diff_add", "from":'$1', "to":'$2'}
{"command": "diff_set", "from":'$1', "to":'$2'}
{"command": "diff_del", "from":'$1', "to":'$2'}' | python ../app/deska_server.py > out
