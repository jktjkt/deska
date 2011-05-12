#!/usr/bin/python2

import random, datetime
from randdom import Names, Dates, Numbers

class ModulGenerator():

	 create_template = "CREATE TABLE large_modul ( {columns} );"
	 column_template = "{0} {1}"
	 large_modul_add_template = "SELECT large_modul_add('{object}');"
	 large_modul_set_template = "SELECT large_modul_set_{column}('{object}','{val}')";
	 commit_template = '''
SELECT commitChangeset('commit');
SELECT startChangeset();
'''

	 
	 def __init__(self, count = 10):
		  self.count = count
		  count = self.count
		  self.names = Names("names.txt")
		  #text is present more times to be more times generated
		  self.types = ["bigint", "int", "text", "date", "text"]
		  self.types_list = list()
		  self.names_list = list()
		  self.data = list()
		  
	 def gen_table(self, count = 0):
		  if (count == 0):
				count = self.count
		  columns_list = list()
		  for i in range(count):
				self.types_list.append(self.types[random.randint(0,len(self.types)-1)])
		  self.names_list = self.names.rset(count)
		  self.names_list.append('name')
		  self.names_list.append('uid')
		  self.types_list.append('text')
		  self.types_list.append('bigint')
		  columns_list = map(self.column_template.format, self.names_list, self.types_list)
		  return self.create_template.format(columns = (",\n\t".join(columns_list)))

	 def gen_data(self, count = 10):
		  objects = list()
		  object_names = Names("names.txt")
		  if count > 1000:
				self.object_names.extend(1000)
		  objects_list = object_names.rset(count + 1)
		  randdates = Dates()
		  dates = randdates.rlist(count + 1)
		  texts = self.names.rset(count + 1)
		  self.data.append("SELECT startChangeset();")
		  while count >= 0:
				r = random.randint(0,4)
				if (len(objects) > 1) and (r < 4):
					 #choos which object should be modified
					 obj = objects[random.randint(0, len(objects) - 1)]
					 #choos which column should be set
					 index = random.randint(0, len(self.names_list) - 3)
					 col = self.names_list[index]
					 #set value
					 if self.types_list[index] == "bigint":
						  value = random.randint(0,1000000)
					 elif self.types_list[index] == "int":
						  value = random.randint(0,10000)
					 elif self.types_list[index] == "text":
								value = texts[count]
					 elif self.types_list[index] == "date":
								value = dates[count]
					 else:
						  value = ""
					 str = self.large_modul_set_template.format(column = col, object = obj, val = value)
				else:
					 obj_name = objects_list[count]
					 objects.append(obj_name)
					 str = self.large_modul_add_template.format(object = obj_name)
				self.data.append(str)
				count = count - 1
				index = Numbers(len(self.data)).rset(len(self.data)/100)
				for i in index:
					 self.data.insert(i,self.commit_template)
		  self.data.append("SELECT commitChangeset('commit');")
		  return self.data

mg = ModulGenerator(12)
print mg.gen_table()
print "\n".join(mg.gen_data(200))