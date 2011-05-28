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
		if self.dberr.code == '42601':
			self.message = "Syntax error, something strange happend."
		if self.dberr.code == '42883':
			self.message = "Either kindName or attribute does not exists."
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


def fcall(fname,*args):
	'''Call stored procedure with params.
	@param fname ID of stored procedure like name(text)
	atr1, atr2 ... parameters for the stored procedure
	'''
	try:
		with xact():
			func = proc(fname)
			return func(*args)
		return 1
	except Postgres.Exception as dberr:
		raise DeskaException(dberr)

def getdata(select,*args):
	'''Get data from database.
	@param select Select statement
	atr1, atr2 ... parameters for the statement
	@returns tuple of column names and data cursor
	'''
	try:
		with xact():
			plan = prepare(select)
			return plan.column_names, plan(*args)
	except Postgres.Exception as dberr:
		raise DeskaException(dberr)
		
def params(argString):
	'''Get python structure from string'''
	return json.loads(argString)	
	
class Filter():
        '''Class for handling filters'''

        opMap = {"columnEq": "="}

        def __init__(self,filterData):
                self.data = filterData

        def getWhere(self):
                '''Return where part of sql statement'''
                return "WHERE " + self.parse(self.data)

        def parse(self,data):
                data["condition"] = self.opMap[data["condition"]]
                return self.strCondition(data)

        def strCondition(self,data):
                '''Return one condition'''
                return "{column} {condition} '{value}'".format(**data)
