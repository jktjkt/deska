--
-- just notes how to get db schema
--

-- tables in our schema
select * from pg_tables where schemaname = 'deska_dev';
-- and xml output
select query_to_xml('select * from pg_tables where schemaname = \'deska_dev\'',true,false,'');

-- more info for one table, see column oid
select * from pg_class where relname = 'vendor';

-- attributes for the table vendor - oid
-- see the attnum column - user attributes(=columns) are the positive
select * from pg_attribute where attrelid = 24576;

-- all foreign keys
select * from pg_constraint where contype='f';

-- one constraint ...
select * from pg_constraint where conname='cpu_type_fkey' ;

-- table and column for this columns
select * from pg_attribute where attrelid = 24583 and attnum = 2

--
-- so now we only need to join it
-- this is only to see what we can get from the db ...
--

--joint information about tables
select relname,attname,typname 
from 	pg_class as cl 
	join pg_tables as tab on (schemaname='deska_dev' and cl.relname = tab.tablename) 
	join pg_attribute as att on (att.attrelid = cl.oid )
	join pg_type as typ on (typ.oid = att.atttypid)
where  att.attname not in ('tableoid','cmax','xmax','cmin','xmin','ctid');


--function that returns concatenated names of columns, which are in given class with given attnum
--here we use it to convert class oid and attnum of its tables attributes to space delimited column names
-- = column names from foreignkey (constraint on and ref)
create or replace function concat_atts_name(classoid oid, idarray smallint[])
returns text
as
$$
declare
 result text;
 counter int8;
 currentval text;
begin
	result='';
	counter = array_lower(idarray, 1);
	if (counter <= array_upper(idarray, 1)) then
		select attname into result
		from pg_attribute as att 
		where att.attrelid = classoid and att.attnum = idarray[counter];		
		counter = counter + 1;
	end if;

	while (counter <= array_upper(idarray, 1)) loop
		select attname into currentval 
		from pg_attribute as att 
		where att.attrelid = classoid and att.attnum = idarray[counter];

		result = result || ' ' || currentval;
		counter = counter + 1;
	end loop;
	return result;
end;
$$
LANGUAGE plpgsql;


--foreign keys
--name of constraint, name of table t1 on which is constrain, names of columns (delimited by space) of t1 on which is constraint, t2 is referenced table, referenced columns
select conname,
 class1.relname as tableon,
 concat_atts_name(class1.oid, constr.conkey) as colson,
 class2.relname as tableref,
 concat_atts_name(class1.oid, constr.conkey) as colsref
from 	pg_constraint as constr
	--join with table which the contraint is on
	join pg_class as class1 on (constr.conrelid = class1.oid)	
	--join with referenced table
	join pg_class as class2 on (constr.confrelid = class2.oid)
where contype='f' ;
