CREATE TYPE pk_constraints_on_table_type AS(
conname	name,
attname	name
);

CREATE FUNCTION pk_constraints_on_table(tabname name)
RETURNS SETOF pk_constraints_on_table_type
AS
$$
DECLARE
	--contraint name
	cname name;
	--counter of pk columns
	counter int;
	--name of currently processeding column name
	currname name;
	--list of column ids
	idarray smallint[];
	--class oid of table
	coid int;
	--one row of result (constraint_name, columns_names, referenced_table_name, referenced_columns_names)
	r pk_constraints_on_table_type;
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
		r = ROW(cname, currname);
		RETURN NEXT r;
		counter = counter + 1;
	END LOOP;
END
$$
LANGUAGE plpgsql;

--SELECT pk_constraints_on_table('test');



CREATE TYPE fk_constraints_on_table_type AS(
conname	name,
attname	name,
reftabname	name,
refattname	name
);

--DROP FUNCTION fk_constraints_on_table(name);

CREATE FUNCTION fk_constraints_on_table(tabname name)
RETURNS SETOF fk_constraints_on_table_type
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
	r fk_constraints_on_table_type;
BEGIN
	--select foreign key constraints on this table
	--constraint name, column names, oid of referenced table, name of referenced table
	FOR cname, idarray, refidarray, coid, refcoid, refname IN 
		SELECT conname, constr.conkey, constr.confkey, class1.oid, class2.oid, class2.relname
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

--SELECT fk_constraints_on_table('test');



CREATE TYPE c_constraints_on_table_type AS(
conname	name,
attname	name,
condition text
);

--DROP FUNCTION c_constraints_on_table(name);

CREATE FUNCTION c_constraints_on_table(tabname name)
RETURNS SETOF c_constraints_on_table_type
AS
$$
DECLARE
	--contraint name
	cname name;
	--name of table that is referenced from this table
	counter int;
	--name of currently processeding column name
	currname name;
	--list of column ids
	idarray smallint[];
	--class oid of table
	coid int;
	--one row of result (constraint_name, columns_names, referenced_table_name, referenced_columns_names)
	r c_constraints_on_table_type;
	--text of check constraint = consrc
	constrtext text;
BEGIN
	--select check constraints on this table
	--constraint name, column names, oid of referenced table, name of referenced table
	FOR cname, coid, idarray, constrtext IN SELECT constr.conname, class.oid, constr.conkey, constr.consrc
		FROM	pg_constraint AS constr
			--join with TABLE which the contraint is ON
			join pg_class AS class ON (constr.conrelid = class.oid)	
		WHERE class.relname=tabname AND constr.contype='c'					
	LOOP			
		counter = array_lower(idarray, 1);
		
		WHILE (counter <= array_upper(idarray, 1)) LOOP
			--selecting columns
			SELECT attname INTO currname
			FROM pg_attribute AS att
			WHERE att.attrelid = coid AND att.attnum = idarray[counter];
		
			r = ROW(cname, currname, constrtext);
			RETURN NEXT r;
			counter = counter + 1;
		END LOOP;
	END LOOP;
END
$$
LANGUAGE plpgsql;

--SELECT c_constraints_on_table('test');



CREATE TYPE u_constraints_on_table_type AS(
conname	name,
attname	name
);

CREATE FUNCTION u_constraints_on_table(tabname name)
RETURNS SETOF u_constraints_on_table_type
AS
$$
DECLARE
	--contraint name
	cname name;
	--counter of pk columns
	counter int;
	--name of currently processeding column name
	currname name;
	--list of column ids
	idarray smallint[];
	--class oid of table
	coid int;
	--one row of result (constraint_name, columns_names, referenced_table_name, referenced_columns_names)
	r u_constraints_on_table_type;
BEGIN
	--select primary key constraint on this table
	FOR cname, coid, idarray IN SELECT constr.conname, class.oid, constr.conkey
	FROM	pg_constraint AS constr
		--join with TABLE which the contraint is ON
		join pg_class AS class ON (constr.conrelid = class.oid)	
	WHERE class.relname=tabname AND constr.contype='u'
	LOOP
		counter = array_lower(idarray, 1);
		
		WHILE (counter <= array_upper(idarray, 1)) LOOP
			--selecting columns
			SELECT attname INTO currname
			FROM pg_attribute AS att
			WHERE att.attrelid = coid AND att.attnum = idarray[counter];
			r = ROW(cname, currname);
			RETURN NEXT r;
			counter = counter + 1;
		END LOOP;
	END LOOP;
END
$$
LANGUAGE plpgsql;

--SELECT u_constraints_on_table('test');



--DROP FUNCTION n_constraints_on_table(name);

CREATE FUNCTION n_constraints_on_table(tabname name)
RETURNS SETOF name
AS
$$
DECLARE
BEGIN
--not null constraint information is marked in pg_attribute
	RETURN QUERY SELECT att.attname
		FROM	pg_class AS class join pg_attribute AS att on (att.attrelid = class.oid)		
		WHERE class.relname = tabname AND att.attnotnull = 't' AND att.attname NOT IN ('tableoid','cmax','xmax','cmin','xmin','ctid');
END
$$
LANGUAGE plpgsql;

--SELECT n_constraints_on_table('test');
