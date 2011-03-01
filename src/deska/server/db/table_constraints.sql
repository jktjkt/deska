CREATE TYPE constraints_on_table_type AS(
conname	name,
attname	name,
reftabname	name,
refattname	name
);

CREATE OR REPLACE FUNCTION constraints_on_table(tabname name)
RETURNS SETOF constraints_on_table_type
AS
$$
DECLARE
	--contraint name
	cname name;
	--name of table that is referenced from this table
	refname name;
	--counter for referencing and referenced columns
	counter int;
	--name of currently processeding column name
	currname name;
	--name of currently processeding referenced column name
	refcurrname name;
	--list of column ids
	idarray smallint[];
	--list of referenced column ids
	refidarray smallint[];
	--class oid of table
	coid int;
	--class oid of referenced table
	refcoid int;
	--one row of result (constraint_name, columns_names, referenced_table_name, referenced_columns_names)
	r constraints_on_table_type;
BEGIN
	--select primary key constraint on this table
	SELECT conname, class.oid, conkey INTO cname, coid, idarray
		FROM	pg_constraint AS constr
			--join with TABLE which the contraint is ON
			join pg_class AS class ON (constr.conrelid = class.oid)	
		WHERE class.relname=tabname AND contype='p';

	counter = array_lower(idarray, 1);
		
	WHILE (counter <= array_upper(idarray, 1)) LOOP
		--selecting columns
		SELECT attname INTO currname
		FROM pg_attribute AS att
		WHERE att.attrelid = coid AND att.attnum = idarray[counter];
		r = ROW(cname, currname, NULL, NULL);
		RETURN NEXT r;
		counter = counter + 1;
	END LOOP;

	--select foreign key constraints on this table
	--constraint name, column names, oid of referenced table, name of referenced table
	FOR cname, idarray, refidarray, refcoid, refname IN SELECT conname, constr.conkey, constr.confkey, class2.oid, class2.relname
			FROM	pg_constraint AS constr
			--join with TABLE which the contraint is ON
			join pg_class AS class1 ON (constr.conrelid = class1.oid)	
			--join with referenced TABLE
			join pg_class AS class2 ON (constr.confrelid = class2.oid)
			WHERE class1.relname=tabname AND contype='f'					
	LOOP			
		counter = array_lower(idarray, 1);
		
		WHILE (counter <= array_upper(idarray, 1)) LOOP
			--selecting columns
			SELECT attname INTO currname
			FROM pg_attribute AS att
			WHERE att.attrelid = coid AND att.attnum = idarray[counter];

			--selecting referenced columns
			SELECT attname INTO refcurrname
			FROM pg_attribute AS att
			WHERE att.attrelid = refcoid AND att.attnum = refidarray[counter];
		
			r = ROW(cname, currname, refname, refcurrname);
			RETURN NEXT r;
			counter = counter + 1;
		END LOOP;
	END LOOP;
END
$$
LANGUAGE plpgsql;

drop table deska.test;

create table deska.test(
	uid int,
	uid2 int,	
	ruid int,
	ruid2 int,
	constraint pk_test primary key (uid,uid2),
	constraint fk_test foreign key (ruid,ruid2) references reftest (uid,uid2)
);

drop table deska.reftest;

create table deska.reftest(
	uid int,
	uid2 int,
	constraint pk_reftest primary key (uid,uid2)
);


select constraints_on_table('test'); 
