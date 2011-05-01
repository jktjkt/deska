time psql deska_dev luke -c "
SET search_path TO api,genproc,history,deska,production;
SELECT * from diff_created(10000,10010) as x JOIN version as v ON (v.id = x.version);
" > test.out1

time psql deska_dev luke -c "
SET search_path TO api,genproc,history,deska,production;
SELECT * FROM hardware_diff_created(22392,42375);
" > test.out2

time psql deska_dev luke -c "
SET search_path TO api,genproc,history,deska,production;
SELECT * FROM diff(10000,10010);
" > test.out11

time psql deska_dev luke -c "
SET search_path TO api,genproc,history,deska,production;
SELECT * FROM hardware_diff_set_attributes(22392,42375);
" > test.out22
