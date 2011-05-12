#!/usr/bin/python2

import random, datetime
from randdom import Names

class ModulGenerator():

	 create_template = "CREATE TABLE large_modul ( {columns} );"
	 column_template = "{0} {1}"
	 
	 def __init__(self, count = 10):
		  self.count = count
		  count = self.count
		  self.names = Names("names.txt")
		  #text is present more times to be more times generated
		  self.types = ["bigint", "int", "text", "date", "text"]
		  
	 def gen_table(self, count = 0):
		  if (count == 0):
				count = self.count
		  columns_list = list()
		  types_list = list()
		  for i in range(count):
				types_list.append(self.types[random.randint(0,len(self.types)-1)])
		  names_list = self.names.rset(count)
		  columns_list = map(self.column_template.format, names_list, types_list)
		  return self.create_template.format(columns = (",\n\t".join(columns_list)))

mg = ModulGenerator(12)
print mg.gen_table();