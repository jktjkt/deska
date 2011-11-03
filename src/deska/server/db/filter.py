# this is filter implementation

#import Postgres
import json
import re
import generated
from dutil import DutilException
from dutil import mystr
from dutil import fcall

class Condition():
	'''Class to store and handle column/value/operator data'''

	# standart operators
	opMap = {
		"columnEq": "=",
		"columnNe": "!=",
		"columnGt": ">",
		"columnGe": ">=",
		"columnLe": "<=",
		"columnLt": "<"
	}
	# operators for sets
	opSetMap = {
		"columnContains": "="
	}

	def __init__(self,data,condId):
		'''Constructor, set local data and parse condition'''
		self.newcond = None
		if "value" not in data:
			raise DutilException("FilterError","Item 'value' is missing in condition.")
		if "condition" not in data:
			raise DutilException("FilterError","Item 'condition' is missing in condition.")

		self.val = data["value"]
		self.op = data["condition"]
		self.counter = condId
		self.id = "${0}".format(condId)
		self.metadata = False
		self.idSet = False
		if "metadata" in data:
			self.kind = "metadata"
			self.metadata = True
			self.col = data["metadata"]
			# we cannot see now, if it is about pending changeset or list revisions
			if self.col not in ["revision","message","author","timestamp","changeset"]:
				raise DutilException("FilterError","Attribute {0} does not exists.".format(self.col))
		elif "kind" in data:
			self.kind = data["kind"]
			if "attribute" not in data:
				raise DutilException("FilterError","Item 'attribute' is missing in condition.")
			self.col = data["attribute"]
			if self.kind not in generated.kinds():
				raise DutilException("FilterError","Kind {0} does not exists.".format(self.col))
			# add also name
			if self.col not in generated.atts(self.kind) and self.col != "name":
				raise DutilException("FilterError","Attribute {0} does not exists.".format(self.col))

			if self.col != "name" and generated.atts(self.kind)[self.col] == "identifier_set":
				self.idSet = True
		else:
			raise DutilException("FilterError","Item 'kind' or 'metadata' must be in condition.")
		self.parse()

	def relationParse(self):
		'''Work with relations'''
		if self.metadata:
			'''skip this checks'''
			return
		embedNames = generated.embedNames()
		# asking for embed name
		for relName in embedNames:
			if self.col == "name" and generated.relFromTbl(relName) == self.kind:
				# FIXME: delimiter
				parent, name = fcall("embed_name(text,text)",self.val,'->')
				self.val = mystr(name)
				newcond = dict()
				newcond["attribute"] = generated.relToTbl(relName)
				newcond["kind"] = self.kind
				newcond["condition"] = self.op
				newcond["value"] = mystr(parent)
				self.newcond = AdditionalEmbedCondition(newcond,self.counter + 1)
		refNames = generated.refNames()
		# find referenced columns
		for relName in refNames:
			fromTbl = generated.relFromTbl(relName)
			fromCol = generated.relFromCol(relName)
			if self.kind == fromTbl and self.col == fromCol:
				# update coldef for identifier references
				#FIXME: delete same if-else when not needed
				if generated.atts(self.kind)[self.col] == "identifier_set":
					self.id = "{0}_get_uid({1},$1)".format(generated.relToTbl(relName),self.id)
				else:
					self.id = "{0}_get_uid({1},$1)".format(generated.relToTbl(relName),self.id)

	def operatorParse(self):
		'''Work with operators'''
		# check contains operator, not for metadata or col = name
		if self.idSet:
			if self.op not in self.opSetMap:
				raise DutilException("FilterError","Operator '{0}' is not supported for sets.".format(self.op))
			self.op = self.opSetMap[self.op]
		else:
			if self.op not in self.opMap:
				raise DutilException("FilterError","Operator '{0}' is not supported.".format(self.op))
			self.op = self.opMap[self.op]

		# FZU: This is special fzu db logic, drop it if you use standart SQL logic
		# if you don't want it, just delete use of this variable
		self.nullCompensation = ""
		if self.op == '!=':
			self.nullCompensation = " OR {0}.{1} IS NULL"


		# propper work with nulls
		if self.val is None:
			self.null = True
			if self.op == '=':
				self.op = 'IS NULL'
			elif self.op == '!=':
				self.op = 'IS NOT NULL'
				self.nullCompensation = ""
			else:
				raise DutilException("FilterError","Operator '{0}' is not supported for NULL values.".format(self.op))

	def parse(self):
		'''Update condition data for easy creation of Deska SQL condition'''
		if self.col == "changeset" and self.kind == "metadata":
			self.col = "id"
			self.id = "changeset2id({0})".format(self.id)
		if self.col == "revision" and self.kind == "metadata":
			self.col = "num"
			self.id = "revision2num({0})".format(self.id)
		self.relationParse()
		self.operatorParse()

	def get(self):
		'''Return deska SQL condition'''
		if self.idSet:
			'''Change kind to inner_'''
			self.kind = "inner_"+self.col
		if self.newcond is None:
			if self.val is None:
				'''do not return none'''
				return "{0}.{1} {2}".format(self.kind,self.col,self.op,self.id), []
			else:
				return ("{0}.{1} {2} {3}"+self.nullCompensation).format(self.kind,self.col,self.op,self.id), [self.val]
		else:
			'''We need to add one condition'''
			if self.val is None:
				'''do not return none'''
				cond1 = "{0}.{1} {2}".format(self.kind,self.col,self.op,self.id)
				cond2, val2 = self.newcond.get()
				return "( ({0}) AND ({1}) )".format(cond1,cond2), [self.val]
			else:
				cond1 = ("{0}.{1} {2} {3}"+self.nullCompensation).format(self.kind,self.col,self.op,self.id)
				cond2, val2 = self.newcond.get()
				return "( ({0}) AND ({1}) )".format(cond1,cond2), [self.val]+val2

	def getIdSetInfo(self):
		'''Return kind,col if there is identifier_set condition'''
		if self.idSet:
			return self.kind, self.col
		else:
			return None, None

	def getAffectedKind(self):
		'''Return kind in condition'''
		return self.kind

class AdditionalEmbedCondition(Condition):
	def relationParse(self):
		# We are called from else part - self.newcond...
		# because this is column refers to another table
		#FIXME: version parametr, $1 every time, check for conflicts
		self.id = "{0}_get_uid({1},$1)".format(self.col,self.id)

class Filter():
	'''Class for handling filters'''

	def __init__(self,filterData,start):
		'''loads json filter data, start = fisrt number of parameter index'''
		# counter for value id's in select string
		self.counter = start
		# list of values for select string
		self.values = list()
		# set of kinds
		self.kinds = set()
		# dict for identifier_set info kind:column
		self.idSetInfo = dict()
		self.where = ''
		if filterData is None:
			self.data = None
			return
		try:
			self.data = json.loads(filterData)
		except Exception as err:
			raise DutilException("FilterError","Syntax error in filter.")
		self.where = self.parse(self.data)

	def getWhere(self):
		'''Return where part of sql statement'''
		if self.data is None:
			return '',[]
		if self.where is None:
			return '',[]
		return "WHERE " + self.where, self.values

	def getIdSetJoin(self):
		'''Return join part for identifier_set joining'''
		ret = ''
		for kind in self.idSetInfo:
			col = self.idSetInfo[kind]
			joincond = "{0}.uid = inner_{1}.{0}".format(kind,col)
			#FIXME: or {1}_{0} - get it from relation info
			#FIXME: until wait for function, use just the table
			#ret = ret + " JOIN inner_{0}_{1}_data_version($1) AS inner_{1} ON {2} ".format(kind, col, joincond)
			ret = ret + " JOIN inner_{0}_{1}_multiref_history AS inner_{1} ON {2} ".format(kind, col, joincond)
		return ret


	def getJoin(self,mykind):
		'''Return join part of sql statement'''
		ret = ''
		self.kinds = self.kinds - set([mykind])
		for kind in self.kinds:
			if mykind == "metadata":
				joincond = "{0}.id = {1}.version".format(mykind,kind)
				ret = ret + " JOIN {tbl}_history AS {tbl} ON {cond} ".format(tbl = kind, cond = joincond)
			else:
				if kind not in generated.kinds():
					raise DutilException("FilterError","Kind {0} does not exists.".format(kind))

				# check for refs
				refNames = generated.refNames()
				findJoinable = False
				# find if there is ref relation from mykind to kind or kind to mykind
				for relName in refNames:
					fromTbl = generated.relFromTbl(relName)
					toTbl = generated.relToTbl(relName)
					if fromTbl == kind and toTbl == mykind:
						joincond = "{0}.uid = {1}.{2}".format(mykind,kind,generated.relFromCol(relName))
						ret = ret + " JOIN {tbl}_data_version($1) AS {tbl} ON {cond} ".format(tbl = kind, cond = joincond)
						findJoinable = True
					if toTbl == kind and fromTbl == mykind:
						joincond = "{0}.{2} = {1}.uid".format(mykind,kind,generated.relFromCol(relName))
						ret = ret + " JOIN {tbl}_data_version($1) AS {tbl} ON {cond} ".format(tbl = kind, cond = joincond)
						findJoinable = True
						
				# find if there is embeding
				embedNames = generated.embedNames()
				for relName in embedNames:
					fromTbl = generated.relFromTbl(relName)
					toTbl = generated.relToTbl(relName)
					if fromTbl == kind and toTbl == mykind:
						joincond = "{0}.uid = {1}.{2}".format(mykind,kind,generated.relFromCol(relName))
						ret = ret + " JOIN {tbl}_data_version($1) AS {tbl} ON {cond} ".format(tbl = kind, cond = joincond)
						findJoinable = True
					if toTbl == kind and fromTbl == mykind:
						joincond = "{0}.{2} = {1}.uid".format(mykind,kind,generated.relFromCol(relName))
						ret = ret + " JOIN {tbl}_data_version($1) AS {tbl} ON {cond} ".format(tbl = kind, cond = joincond)
						findJoinable = True
						
				if not findJoinable:
					raise DutilException("FilterError","Kind {0} cannot be joined with kind {1}.".format(kind,mykind))
				#TODO: merge, template relations
		return ret + self.getIdSetJoin()

	def parse(self,data):
		'''Parse filter data and create SQL WHERE part'''
		if self.data == '':
			return ''
		if "operator" in data:
			operator = data["operator"]
			if "operands" not in data:
				raise DutilException("FilterError","Missing operands.")

			if operator == "and":
				if data["operands"] == []:
					return None
				res = [self.parse(expresion) for expresion in data["operands"]]
				return "(" + ") AND (".join(res) + ")"
			elif operator == "or":
				if data["operands"] == []:
					return None
				res = [self.parse(expresion) for expresion in data["operands"]]
				return "(" + ") OR (".join(res) + ")"
			else:
				raise DutilException("FilterError","Bad operands.")
		cond = Condition(data,self.counter)
		# collect affected kinds (need for join)
		self.kinds.add(cond.getAffectedKind())
		# also add identifier_set information for joining
		idSetKind, idSetColumn = cond.getIdSetInfo()
		if idSetKind is not None and idSetColumn is not None:
			self.idSetInfo[idSetKind] = idSetColumn
		ret, newValues = cond.get()
		self.counter = self.counter + len(newValues)
		#add values into list
		self.values.extend(newValues)
		return ret

