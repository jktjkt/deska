--
-- function(s) to get the database schema
--

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
			join pg_tables AS tab ON (schemaname='deska_dev' and cl.relname = tab.tablename)
			join pg_attribute AS att ON (att.attrelid = cl.oid )
			join pg_type AS typ ON (typ.oid = att.atttypid)
		WHERE  att.attname NOT IN ('tableoid','cmax','xmax','cmin','xmin','ctid');
END
$$
LANGUAGE plpgsql;
--
-- function returns info about dependencies between data - foreign keys
-- TODO: get rid of concat 
--
CREATE OR REPLACE FUNCTION get_dependency_info()
RETURNS SETOF table_info
AS
$$
BEGIN
	RETURN QUERY SELECT conname, class1.relname AS tableon, concat_atts_name(class1.oid, constr.conkey) AS colson,
		class2.relname AS tableref, concat_atts_name(class1.oid, constr.conkey) AS colsref
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
CREATE TYPE KindAttributeDataType AS (attname name, typename name);

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

