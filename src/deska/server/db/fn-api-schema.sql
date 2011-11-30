--
-- function(s) to get the database schema
--
-- these functions should be in schema deska
--
SET search_path TO deska,versioning;
--
-- function returns info about tables in db
--
CREATE TYPE table_info AS(
relname	text,
attname	text,
typname	text
);

CREATE OR REPLACE FUNCTION get_table_info()
RETURNS SETOF table_info
AS
$$
BEGIN
		RETURN QUERY SELECT relname::text,attname::text,typname::text
		  FROM pg_class cl 
				JOIN pg_namespace nm ON (cl.relnamespace = nm.oid and nm.nspname = 'production')
				JOIN pg_attribute att ON (att.attrelid = cl.oid)
				JOIN pg_type typ ON (typ.oid = att.atttypid)
				JOIN pg_tables tab ON (cl.relname = tab.tablename)
		  WHERE att.attname NOT IN ('tableoid','cmax','xmax','cmin','xmin','ctid');
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

-- and here is also view
CREATE VIEW table_info_view AS SELECT relname::text,attname::text,typname::text
	FROM	pg_class AS cl
		join pg_tables AS tab ON (schemaname='production' and cl.relname = tab.tablename)
		join pg_attribute AS att ON (att.attrelid = cl.oid )
		join pg_type AS typ ON (typ.oid = att.atttypid)
	WHERE  att.attname NOT IN ('tableoid','cmax','xmax','cmin','xmin','ctid');


--this function rewrites array of ids of atributes to names of atributes joind with ","
--needed for example for function get_dependency_info
CREATE OR REPLACE FUNCTION concat_atts_name(classoid oid, idarray smallint[])	
RETURNS text
AS
$$	
DECLARE
 result text;
 counter int8;
 currentval text;
BEGIN
	result='';
	counter = array_lower(idarray, 1);
	IF (counter <= array_upper(idarray, 1)) THEN
		SELECT attname INTO result
		FROM pg_attribute AS att 
		WHERE att.attrelid = classoid AND att.attnum = idarray[counter];		
		counter = counter + 1;
	END IF;
	WHILE (counter <= array_upper(idarray, 1)) LOOP
		SELECT attname INTO currentval 
		FROM pg_attribute AS att 
		WHERE att.attrelid = classoid AND att.attnum = idarray[counter];
		result = result || ' ' || currentval;
		counter = counter + 1;
	END LOOP;
	RETURN result;	
END;	
$$	
LANGUAGE plpgsql SECURITY DEFINER;

CREATE OR REPLACE FUNCTION uid_pos(classoid oid, idarray smallint[])
RETURNS int
AS
$$
DECLARE
	counter int = 0;
	currentval name;
BEGIN
	counter = array_lower(idarray, 1);
	WHILE (counter <= array_upper(idarray, 1)) LOOP
		SELECT attname INTO currentval 
		FROM pg_attribute AS att 
		WHERE att.attrelid = classoid AND att.attnum = idarray[counter];
		IF currentval = 'uid' THEN
			RETURN counter;
		END IF;
		counter = counter + 1;
	END LOOP;	
END
$$
LANGUAGE plpgsql;

CREATE TYPE attribute_table_type AS(
	attname name,
	tabname name
);

CREATE OR REPLACE FUNCTION cols_ref_uid(tabname name)
RETURNS SETOF attribute_table_type
AS
$$
DECLARE 
	toid oid;
	tckey smallint[];
	roid oid;
	rckey smallint[];
	pos int;
BEGIN
	FOR toid,tckey,roid,rckey IN 
		SELECT 
			class1.oid, constr.conkey,
			class2.oid, constr.confkey
		FROM	pg_constraint AS constr
			--join with TABLE which the contraint is ON
			join pg_class AS class1 ON (constr.conrelid = class1.oid)	
			--join with referenced TABLE
			join pg_class AS class2 ON (constr.confrelid = class2.oid)
		WHERE contype='f' AND class1.relname = tabname
	LOOP
		pos = uid_pos(roid,rckey);
		IF pos > 0 THEN
			RETURN QUERY SELECT attname, relname
				FROM pg_attribute AS att, pg_class AS class
				WHERE att.attrelid = toid AND att.attnum = tckey[pos] AND class.oid = roid;
		END IF;
	END LOOP;
END
$$
LANGUAGE plpgsql;

--
-- function returns info about dependencies between data - foreign keys
-- TODO: get rid of concat 
--
CREATE TYPE table_dependency_info AS(
conname	name,
relname	name,
attnames	text,
refrelname	name,
refattnames	text
);

CREATE OR REPLACE FUNCTION get_dependency_info()
RETURNS SETOF table_dependency_info
AS
$$
BEGIN
	RETURN QUERY SELECT conname, class1.relname AS tableon, concat_atts_name(class1.oid, constr.conkey) AS colson,
		class2.relname AS tableref, concat_atts_name(class2.oid, constr.confkey) AS colsref
		FROM	pg_constraint AS constr
			--join with TABLE which the contraint is ON
			join pg_class AS class1 ON (constr.conrelid = class1.oid)
			join pg_tables AS tab1 ON (tab1.schemaname='production' and class1.relname = tab1.tablename)	
			--join with referenced TABLE
			join pg_class AS class2 ON (constr.confrelid = class2.oid)
			join pg_tables AS tab2 ON (tab2.schemaname='production' and class2.relname = tab2.tablename)	
		WHERE contype='f' ;

END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- function returns list of names of tables from production = Top-level Kinds like enclosure etc
--
CREATE OR REPLACE FUNCTION kindNames()
RETURNS SETOF name
AS
$$
BEGIN
	RETURN QUERY SELECT distinct cl.relname
		FROM pg_class AS cl
			JOIN pg_tables AS tab ON (schemaname='production' AND cl.relname = tab.tablename)
			JOIN pg_attribute AS att ON (att.attrelid = cl.oid )
			JOIN pg_type AS typ ON (typ.oid = att.atttypid);
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

--
-- function returns list of attributes' names and types = attributes of tables from production
--
CREATE TYPE attr_info AS (attname name, typename name);

CREATE OR REPLACE FUNCTION kindAttributes(tabname name)
RETURNS SETOF attr_info
AS
$$
DECLARE
	base_table name;
BEGIN
	SELECT kind INTO base_table FROM get_templates_info() WHERE template = tabname AND template <> kind;
	IF base_table IS NULL THEN
		base_table := tabname;
	END IF;
	RETURN QUERY 
	SELECT attname, CASE
						WHEN typname <> 'identifier_set' AND attname IN (SELECT attname FROM cols_ref_uid(base_table)) THEN 'identifier'
						ELSE typname
					END
		FROM pg_class AS cl
			JOIN pg_tables AS tab ON (schemaname ='production' and cl.relname = tab.tablename)
			JOIN pg_attribute AS att ON (att.attrelid = cl.oid )
			JOIN pg_type AS typ ON (typ.oid = att.atttypid)
		-- don't return also uid and name columns - internal
		WHERE cl.relname = tabname AND  att.attname NOT IN ('tableoid','cmax','xmax','cmin','xmin','ctid','uid','name');
END
$$
LANGUAGE plpgsql SECURITY DEFINER;



CREATE TYPE kind_relation AS(
--relkind text,
relation text,
refkind	name
);

--gets relations between kind kindname and other kinds - type of relation, name of related kind
CREATE OR REPLACE FUNCTION kindRelations(kindname name)
RETURNS SETOF kind_relation
AS $$
DECLARE
BEGIN
	RETURN QUERY 
		SELECT relation, refkind 
		FROM kindRelations_full_info(kindname)
			WHERE relation <> 'INVALID';
END
$$
LANGUAGE plpgsql SECURITY DEFINER;



--DROP TYPE kind_relation;

--type for relations of a kind
CREATE TYPE kind_relation_full AS(
--relkind text,
relation text,
attname	text,
refkind	name,
refattname	text
);

--DROP FUNCTION get_relations(name);

--function returns relations of a given kind
CREATE OR REPLACE FUNCTION kindRelations_full_info(kindname name)
RETURNS SETOF kind_relation_full
AS $$
DECLARE
BEGIN
    RETURN QUERY SELECT 
            CASE 
                WHEN conname LIKE 'rconta_%' THEN 'CONTAINS'
                WHEN conname LIKE 'rcoble_%' THEN 'CONTAINABLE'
                WHEN conname LIKE 'rtempl_%' THEN 'TEMPLATIZED'
                WHEN conname LIKE 'rembed_%' THEN 'EMBED_INTO'
                WHEN conname LIKE 'rrefer_%' THEN 'REFERS_TO'
                WHEN ((SELECT typ.typname FROM pg_attribute AS att 
                    JOIN pg_type AS typ ON (typ.oid = att.atttypid)
                    WHERE att.attrelid = class1.oid AND att.attname = ref_att_name(class1.oid,constr.conkey,class2.oid,constr.confkey)) = 'identifier_set') THEN
                    'REFERS_TO_SET'
                ELSE 'INVALID'
            END,
            concat_atts_name(class1.oid, constr.conkey),
            class2.relname, concat_atts_name(class2.oid, constr.confkey)
            FROM    pg_constraint AS constr
                --join with TABLE which the contraint is ON
                JOIN pg_class AS class1 ON (constr.conrelid = class1.oid)
                --join with referenced TABLE
                JOIN pg_class AS class2 ON (constr.confrelid = class2.oid)
            WHERE contype='f' AND class1.relname = kindname;
END
$$
LANGUAGE plpgsql SECURITY DEFINER;

CREATE TYPE templates_info_type AS(
	template name,
	kind name
);

--returns list of templates
--is temporary before object relition functions would be ready to use
--returns pair template table, table that is templated by template table
CREATE FUNCTION get_templates_info()
RETURNS SETOF templates_info_type
AS
$$
BEGIN
RETURN QUERY SELECT  class2.relname, class1.relname
	FROM	pg_constraint AS constr
		--join with TABLE which the contraint is ON
		join pg_class AS class1 ON (constr.conrelid = class1.oid)	
		--join with referenced TABLE
		join pg_class AS class2 ON (constr.confrelid = class2.oid)
	WHERE contype='f' AND conname LIKE 'rtempl_%';
END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION ref_att_name(class oid, attid smallint[], refclass oid, refattid smallint[])	
RETURNS text
AS
$$	
DECLARE
 uid_pos int;
 result text;
BEGIN
	uid_pos = uid_pos(refclass, refattid);
	SELECT attname INTO result
	FROM pg_attribute AS att 
	WHERE att.attrelid = class AND att.attnum = attid[uid_pos];		
	RETURN result;	
END;	
$$	
LANGUAGE plpgsql SECURITY DEFINER;

