--
-- function(s) to get the database schema
--

--
-- function that returns concatenated names of columns, which are in given class with given attnum
-- here we use it to convert class oid and attnum of its tables attributes to space delimited column names
-- = column names from foreignkey (constraint on and ref)
--
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

--
-- function returns info about tables in db
--
create or replace function get_table_info()
returns text
as
$$
begin
	return select relname,attname,typname 
		from 	pg_class as cl 
			join pg_tables as tab on (schemaname='deska_dev' and cl.relname = tab.tablename) 
			join pg_attribute as att on (att.attrelid = cl.oid )
			join pg_type as typ on (typ.oid = att.atttypid)
		where  att.attname not in ('tableoid','cmax','xmax','cmin','xmin','ctid');
end;
$$
LANGUAGE plpgsql;

--
-- function returns info about dependencies between data - foreign keys
--
create or replace function get_dependency_info()
returns text
as
$$
begin
	return select conname, class1.relname as tableon, concat_atts_name(class1.oid, constr.conkey) as colson,
		class2.relname as tableref, concat_atts_name(class1.oid, constr.conkey) as colsref
		from	pg_constraint as constr
			--join with table which the contraint is on
			join pg_class as class1 on (constr.conrelid = class1.oid)	
			--join with referenced table
			join pg_class as class2 on (constr.confrelid = class2.oid)
		where contype='f' ;
end;
$$
LANGUAGE plpgsql;

--
-- function returns list of names of tables from deska_dev = Top-level Kinds like enclosure etc
--
create or replace function get_kindNames() returns setof text as
$$
declare
r text;
begin
for r in select distinct cl.relname 
		from 	pg_class as cl 
			join pg_tables as tab on (schemaname='deska_dev' and cl.relname = tab.tablename) 
			join pg_attribute as att on (att.attrelid = cl.oid )
			join pg_type as typ on (typ.oid = att.atttypid)
		loop
return next r;
end loop;
return;
end
$$
language plpgsql;

--select get_kindNames();

--
-- function returns list of attributes' names and types = attributes of tables from deska_dev
--
create or replace function get_kindAttributes(
	tabname name
) returns setof KindAttributeDataType as
$$
declare
r KindAttributeDataType%rowtype;
begin
for r in select attname,typname
		from 	pg_class as cl 
			join pg_tables as tab on (schemaname='deska_dev' and cl.relname = tab.tablename) 
			join pg_attribute as att on (att.attrelid = cl.oid )
			join pg_type as typ on (typ.oid = att.atttypid)
		where cl.relname = tabname and  att.attname not in ('tableoid','cmax','xmax','cmin','xmin','ctid')
		loop
return next r;
end loop;
return;
end
$$
language plpgsql;

--select get_kindAttributes('hardware');


--this function is needed for selecting identifiers of all concrete objects of a given Kind
--returns name of column that is primary key in table tabname
create or replace function get_primary_key_column(tabname name) returns name as
$$
declare
pkname name;
begin

select conname into pkname
from 	pg_constraint as constr
	--join with table which the contraint is on
	join pg_class as class on (constr.conrelid = class.oid)
where relname = tabname and contype='p';
return pkname;
end
$$
language plpgsql;

--select get_primary_key_column('vendor');