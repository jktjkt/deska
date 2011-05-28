# this is deska util module with helper functions for jsonapi

import Postgres
import json

class DeskaException(Exception):
	'''Exception class for deska exceptions'''

	def __init__(self,dberr):
		'''Construct DeskaException from Postgres.dberr exception'''
		self.dberr = dberr
		self.parseDberr()
		
	def parseDberr(self):
		'''Parse Postgres.dberr into variables used for json dump'''
		self.type = "ServerError"
		if self.dberr.code == '42883':
			self.message = "Either kindName or attribute does not exists"
		else:
			self.message = self.dberr.message

	def json(self,command):
		'''Create json error representation for deska API'''
		err = dict()
		err["type"] = self.type
		err["message"] = self.message

		res = dict()
		res["dbException"] = err
		res["response"] = command

		return json.dumps(res)


def fcall(fname,atr1,atr2 = None):
	'''Call stored procedure with params.
	@param fname ID of stored procedure like name(text)
	atr1, atr2 ... parameters for the stored procedure
	'''
	try:
		with xact():
			func = proc(fname)
			if atr2 is None:
				func(atr1)
			else:
				func(atr1, atr2)
		return 1
	except Postgres.Exception as dberr:
		raise DeskaException(dberr)

def getdata(select,atr1 = None, atr2 = None):
	'''Get data from database.
	@param select Select statement
	atr1, atr2 ... parameters for the statement
	'''
	try:
		with xact():
			plan = prepare(select)
			if atr2 is None:
				if atr1 is None:
					return plan.column_names, plan()
				return plan.column_names, plan(atr1)
			return plan.column_names, plan(atr1, atr2)
	except Postgres.Exception as dberr:
		raise DeskaException(dberr)
		
