/****************************
*	proceding messages		*
****************************/

CREATE OR REPLACE FUNCTION raise_message()
RETURNS void
AS
$$
BEGIN
	RAISE 'raise_message';
END
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION extand_message()
RETURNS void
AS
$$
BEGIN
	SELECT raise_message();
EXCEPTION
	WHEN raise_exception THEN
		RAISE 'extand_message %',SQLERRM;
END
$$
LANGUAGE plpgsql;

select extand_message();

/****************************
*	SQLSTATE				*
****************************/

--in this function is raised built-in exception division_by_zero
--then we catch it at the end of the block in exception block
--and we raise our exception with errorcode (SQLSTATE) '23505' and message 'div by zero'
CREATE OR REPLACE FUNCTION test_raise()
RETURNS void
AS
$$
BEGIN
	SELECT 1/0;
	EXCEPTION
		WHEN division_by_zero THEN
			RAISE 'div by zero' USING ERRCODE = '23505';
END
$$
LANGUAGE plpgsql;



--this function has the same effect as the function above
CREATE OR REPLACE FUNCTION test_raise_with_message()
RETURNS void
AS
$$
BEGIN
	SELECT 1/0;
	EXCEPTION
		WHEN division_by_zero THEN
			RAISE SQLSTATE '23505' using MESSAGE ='div by zero';
END
$$
LANGUAGE plpgsql;


--this function calls the function above which raises our exception
--we can catch this exception by its SQLSTATE
--after it we raise another exception
CREATE OR REPLACE FUNCTION test_catch_raise()
RETURNS void
AS
$$
BEGIN
	SELECT test_raise();
	EXCEPTION
		WHEN SQLSTATE '23505' THEN
			RAISE 'resend exception' USING ERRCODE = '23111';
END
$$
LANGUAGE plpgsql;

--here you can see results of functions that raise excetions
select test_raise();
select test_raise_with_message();

--as mentioned, these functions have the same result
/*
ERROR:  dev by zero


********** Error **********

ERROR: dev by zero
SQL state: 23505
*/

/*
ERROR:  dev by zero


********** Error **********

ERROR: dev by zero
SQL state: 23505
*/


select test_catch_raise();
/*ERROR:  resend exception


********** Error **********

ERROR: resend exception
SQL state: 23111
*/

--here we use RAISE without parameters, it should only reraise the exception
CREATE OR REPLACE FUNCTION test_catch_reraise()
RETURNS void
AS
$$
BEGIN
	SELECT test_raise();	
EXCEPTION	
	WHEN SQLSTATE '23505' THEN
		RAISE;
END
$$
LANGUAGE plpgsql;

--this function calls the function B that called function A with exception
--it caused the same exception as in case this function called dircetly A
--it means in B (test_catch_reraise) the exception was reraised = it raised exactly the same as it caught
CREATE OR REPLACE FUNCTION test_catch_reraised1()
RETURNS void
AS
$$
BEGIN
	SELECT test_catch_reraise();
EXCEPTION
	WHEN SQLSTATE '23505' THEN
		RAISE 'resend exception' USING ERRCODE = '23112';
END
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION test_catch_reraised2()
RETURNS void
AS
$$
BEGIN
	SELECT test_raise();
EXCEPTION
	WHEN SQLSTATE '23505' THEN
		RAISE 'resend exception' USING ERRCODE = '23112';
END
$$
LANGUAGE plpgsql;



select test_catch_reraised1();
/*
ERROR:  resend exception


********** Error **********

ERROR: resend exception
SQL state: 23112
*/

select test_catch_reraised2();
/*ERROR:  resend exception


********** Error **********

ERROR: resend exception
SQL state: 23112
*/


/****************************
*	without catching		*
****************************/
CREATE OR REPLACE FUNCTION raise_message()
RETURNS void
AS
$$
BEGIN
	RAISE 'raise_message';
END
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION only_call()
RETURNS void
AS
$$
BEGIN
	SELECT raise_message();
END
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION only_call_caller()
RETURNS void
AS
$$
BEGIN
	SELECT only_call();
END
$$
LANGUAGE plpgsql;

select only_call_caller();
/*
ERROR:  raise_message
CONTEXT:  SQL statement "SELECT raise_message()"
PL/pgSQL function "only_call" line 2 at SQL statement
SQL statement "SELECT only_call()"
PL/pgSQL function "only_call_caller" line 2 at SQL statement


********** Error **********

ERROR: raise_message
SQL state: P0001
Context: SQL statement "SELECT raise_message()"
PL/pgSQL function "only_call" line 2 at SQL statement
SQL statement "SELECT only_call()"
PL/pgSQL function "only_call_caller" line 2 at SQL statement
*/

--Pgadmin3 gives information about call stack