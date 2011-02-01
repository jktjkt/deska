--
-- function(s) to get the database schema
--
-- these functions should be in schema deska
--
SET search_path TO deska,production;
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
		FROM 	pg_class AS cl
			join pg_tables AS tab ON (schemaname='production' and cl.relname = tab.tablename)
			join pg_attribute AS att ON (att.attrelid = cl.oid )
			join pg_type AS typ ON (typ.oid = att.atttypid)
		WHERE  att.attname NOT IN ('tableoid','cmax','xmax','cmin','xmin','ctid');
END
$$
LANGUAGE plpgsql;

DROP VIEW table_info_view;
-- and here is also view
CREATE VIEW table_info_view AS SELECT relname::text,attname::text,typname::text
	FROM	pg_class AS cl
		join pg_tables AS tab ON (schemaname='production' and cl.relname = tab.tablename)
		join pg_attribute AS att ON (att.attrelid = cl.oid )
		join pg_type AS typ ON (typ.oid = att.atttypid)
	WHERE  att.attname NOT IN ('tableoid','cmax','xmax','cmin','xmin','ctid');

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
			--join with referenced TABLE
			join pg_class AS class2 ON (constr.confrelid = class2.oid)
		WHERE contype='f' ;
END
$$
LANGUAGE plpgsql;

--
-- function returns list of names of tables from deska_dev = Top-level Kinds like enclosure etc
--
CREATE OR REPLACE FUNCTION get_kind_names()
RETURNS SETOF name
AS
$$
BEGIN
	RETURN QUERY SELECT distinct cl.relname
		FROM pg_class AS cl
			JOIN pg_tables AS tab ON (schemaname='deska_dev' AND cl.relname = tab.tablename)
			JOIN pg_attribute AS att ON (att.attrelid = cl.oid )
			JOIN pg_type AS typ ON (typ.oid = att.atttypid);
END
$$
LANGUAGE plpgsql;

--
-- function returns list of attributes' names and types = attributes of tables from deska_dev
--
CREATE TYPE attr_info AS (attname name, typename name);

CREATE OR REPLACE FUNCTION get_kind_attributes(tabname name)
RETURNS SETOF attr_info
AS
$$
BEGIN
RETURN QUERY SELECT attname,typname
		FROM pg_class AS cl
			JOIN pg_tables AS tab ON (schemaname='deska_dev' and cl.relname = tab.tablename)
			JOIN pg_attribute AS att ON (att.attrelid = cl.oid )
			JOIN pg_type AS typ ON (typ.oid = att.atttypid)
		WHERE cl.relname = tabname AND  att.attname NOT IN ('tableoid','cmax','xmax','cmin','xmin','ctid');
END
$$
LANGUAGE plpgsql;

--DROP TYPE deska_dev.kind_relation;

--type for relations of a kind
CREATE TYPE deska_dev.kind_relation AS(
relkind text,
attname	text,
refkind	name,
refattname	text
);

--DROP FUNCTION deska_dev.get_relations(name);

--function returns relations of a given kind
CREATE OR REPLACE FUNCTION deska_dev.get_relations(kindname name)
RETURNS SETOF kind_relation
AS $$
DECLARE
BEGIN
	RETURN QUERY 
		EXECUTE 'SELECT 
				CASE 
				WHEN conname LIKE ''rmerge_%'' THEN ''MERGE''
				WHEN conname LIKE ''rtempl_%'' THEN ''TEMPLATE''
				WHEN conname LIKE ''rembed_%'' THEN ''EMBED''				
				ELSE ''INVALID''
				END,
				concat_atts_name(class1.oid, constr.conkey),
				class2.relname, concat_atts_name(class2.oid, constr.confkey)
			FROM	pg_constraint AS constr
				--join with TABLE which the contraint is ON
				join pg_class AS class1 ON (constr.conrelid = class1.oid)	
				--join with referenced TABLE
				join pg_class AS class2 ON (constr.confrelid = class2.oid)
			WHERE contype=''f'' AND class1.relname = $1'			
		USING kindname;
END
$$
LANGUAGE plpgsql;

