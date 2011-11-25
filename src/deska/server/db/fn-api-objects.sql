SET search_path TO api,deska;

CREATE FUNCTION setAttribute(kindname text, objectname text, attributename text, value text)
RETURNS integer
AS
$$
import Postgres

def call(fname,atr1,atr2):
	try:
		with xact():
			func = proc(fname)
			func(atr1, atr2)
		return 1
	except Postgres.Exception as dberr:
		if dberr.pg_errordata.code == "42883":
			# wait with raising exception
			pass
		else:
			raise
	# we cannot raise exception from except part, so wait for here
	raise Postgres.ERROR('Kind "{kind}" does not exists.'.format(kind = kindname),code = '10111')

@pytypes
def main(kindname,objectname,attributename,value):
	fname = kindname+"_set_"+attributename+"(text,text)"
	return call(fname,objectname,value)

$$
LANGUAGE python SECURITY DEFINER;

CREATE FUNCTION removeAttribute(kindname text, objectname text, attributename text)
RETURNS integer
AS
$$
import Postgres

def call(fname,atr1):
	try:
		with xact():
			func = proc(fname)
			func(atr1)
		return 1
	except Postgres.Exception as dberr:
		if dberr.pg_errordata.code == "42883":
			# wait with raising exception
			pass
		else:
			raise
	# we cannot raise exception from except part, so wait for here
	raise Postgres.ERROR('Kind "{kind}" does not exists.'.format(kind = kindname),code = '10111')

@pytypes
def main(kindname,objectname,attributename):
	fname = kindname+"_rem_"+attributename+"(text)"
	return call(fname,objectname)

$$
LANGUAGE python SECURITY DEFINER;

CREATE FUNCTION changeObjectName(kindname text, oldname text, newname text)
RETURNS integer
AS
$$
import Postgres

def call(fname,atr1,atr2):
	try:
		with xact():
			func = proc(fname)
			func(atr1)
		return 1
	except Postgres.Exception as dberr:
		if dberr.pg_errordata.code == "42883":
			# wait with raising exception
			pass
		else:
			raise
	# we cannot raise exception from except part, so wait for here
	raise Postgres.ERROR('Kind "{kind}" does not exists.'.format(kind = kindname),code = '10111')

@pytypes
def main(kindname,oldname,newname):
	fname = kindname+"_set_name(text,text)"
	return call(fname,oldname,newname)
$$
LANGUAGE python SECURITY DEFINER;

CREATE FUNCTION createObject(kindname text, objectname text)
RETURNS integer
AS
$$
import Postgres

def call(fname,atr1):
	try:
		with xact():
			func = proc(fname)
			func(atr1)
		return 1
	except Postgres.Exception as dberr:
		if dberr.pg_errordata.code == "42883":
			# wait with raising exception
			pass
		else:
			raise
	# we cannot raise exception from except part, so wait for here
	raise Postgres.ERROR('Kind "{kind}" does not exists.'.format(kind = kindname),code = '10111')

@pytypes
def main(kindname,objectname):
	fname = kindname+"_add(text)"
	return call(fname,objectname)
$$
LANGUAGE python SECURITY DEFINER;

CREATE FUNCTION deleteObject(kindname text, objectname text)
RETURNS integer
AS
$$
import Postgres

def call(fname,atr1):
	try:
		with xact():
			func = proc(fname)
			func(atr1)
		return 1
	except Postgres.Exception as dberr:
		if dberr.pg_errordata.code == "42883":
			# wait with raising exception
			pass
		else:
			raise
	# we cannot raise exception from except part, so wait for here
	raise Postgres.ERROR('Kind "{kind}" does not exists.'.format(kind = kindname),code = '10111')

@pytypes
def main(kindname,objectname):
	fname = kindname+"_del(text)"
	return call(fname,objectname)
$$
LANGUAGE python SECURITY DEFINER;

