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
--return type
CREATE TYPE KindAttributeDataType AS (attname name, typename name);

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
--conkey[1] = 1st item from conkey, its for one column PK
  execute 'select att_name(class1.oid, constr.conkey[1])
		from	pg_constraint as constr
			--join with table which the contraint is on
			join pg_class as class1 on (constr.conrelid = class1.oid)	
		where contype=''p'' and relname = $1'
  into pkname
  using tabname;
return pkname;
end
$$
language plpgsql;

--select get_primary_key_column('vendor');

--drop function get_kindInstances(name);
CREATE or REPLACE FUNCTION get_kindInstances(tabname name) RETURNS setof text AS $$
DECLARE 
r text;
BEGIN 
	for r in EXECUTE 'select ' ||   (get_primary_key_column(tabname)) || ' from ' || tabname
	loop
	return next r;
	end loop;
return;
END;
$$ LANGUAGE plpgsql;

create sequence version_seq;
select * from version_seq;
select last_value from version_seq;
--increments value and returns the new value of sequence
select nextval('version_seq');
--selects current value of sequence
select currval('version_seq');

--insert into vendor(uid,name) values (2,'IBM');
--insert into vendor(uid,name) values (14,'Intel');
--insert into vendor(uid,name) values (25,'AMD');
--select * from vendor;
--select get_kindInstances('vendor');
--select get_primary_key_column('vendor');

--createObject( const Identifier &kindName, const Identifier &objectname )
--creates new object if doesn't exist that one
--postgresql 9.1 execute format('insert into %I ( %I ) values ( $1 )',tabname, get_primary_key_column(tabname))
create or replace function create_object(tabname name, id text) returns void
as
$$
begin
	execute 'insert into ' || tabname || ' (' || (get_primary_key_column(tabname)) ') values ( $1 )'
	using id;
end;
$$LANGUAGE plpgsql;

--drop table testtabstr;
--create table testtabstr(textid text primary key, col1 text, col2 int);
--select create_object('testtabstr', 'hoj');

--select get_primary_key_column('testtabstr');

--att_name(class1.oid, constr.conkey[1])

--drop table testtab;
--create table testtab (uid int primary key, description text, col1 int);
--insert into testtab values(1);
--select * from testtab;
--select description from testtab;
--insert into testtab values(2,NULL,NULL);
--select create_object('testtab','12');

--select create_object('testtabstr','12');
--select create_object('testtabstr','ahoj');
--select * from testtabstr;
--select create_object('testtab', '10');


--renameObject( const Identifier &kindName, const Identifier &oldName, const Identifier &newName )
create or replace function rename_object(tabname name, oldval varchar, newval varchar)
returns void as $$
begin
	execute 'update ' || tabname || ' set ' || (get_primary_key_column(tabname)) || '=$1 where ' || (get_primary_key_column(tabname)) || '=$2'
	using newval, oldval;
end;
$$
language plpgsql;

create or replace function rename_object(tabname name, oldval int, newval int)
returns void as $$
begin
	execute 'update ' || tabname || ' set ' || (get_primary_key_column(tabname)) || '=$1 where ' || (get_primary_key_column(tabname)) || '=$2'
	using newval, oldval;
end;
$$
language plpgsql;


--select rename_object('vendor',1,21);


--removeAttribute( const Identifier &kindName, const Identifier &objectName,const Identifier &attributeName )
create or replace function remove_attribute(tabname name, id varchar, colname name) returns void
as $$
begin
	execute 'update ' || tabname || ' set ' || colname || '=NULL where ' || (get_primary_key_column(tabname)) || '=$1'
	using id;
end;
$$ language plpgsql;

--select remove_attribute('testtabstr','prvni','col1');
--select * from testtabstr;
--insert into testtabstr values('prvni','ahoj',1);

--setAttribute( const Identifier &kindName, const Identifier &objectName,const Identifier &attributeName, const Value &value )
create or replace function set_attribute(tabname name, id varchar, colname name, newval varchar)
returns void as $$
begin
	execute 'update ' || tabname || ' set ' || colname || '=$1 where ' || (get_primary_key_column(tabname)) || '=$2'
	using newval, id;
end;
$$ language plpgsql;

--select set_attribute('testtabstr', 'prvni', 'col1', 'ahoj 2');
--select * from testtabstr;

create table objectRelationKind(
	uid smallint PRIMARY KEY,
	description varchar
);

insert into objectRelationKind values (0,'relation merge with');
insert into objectRelationKind values (1,'relation embended into');
insert into objectRelationKind values (2,'relation template');
insert into objectRelationKind values (3,'relation invalid');


--drop table objectRelation;
create table objectRelation(
	tablename name,
	col name,
	kind smallint REFERENCES objectRelationKind(uid),
	refedtab name,	
	refedcol name
);


