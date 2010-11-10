--
-- mini "how to" work with xml column
--

-- create test table with one xml column
create table test (a xml);

-- insert xml as a text
insert into test values('<x><x>23</x><y>12</y></x>');

-- get xml as a text
select * from test;

-- and as one xml doc
SELECT xmlagg(a) from test;

-- get values of x elements
SELECT xpath('//x/text()', a) from test ;

-- use xpath in where
SELECT * from test where xpath('//x/text()', a)::text = '{23}' ;
