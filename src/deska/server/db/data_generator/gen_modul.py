import random, datetime
from randdom import Names, Dates, Numbers

class ModulGenerator():

	 create_template = "CREATE TABLE large_modul ( %s );"
	 column_template = "%s %s"
	 large_modul_add_template = "SELECT large_modul_add('%s');"
	 large_modul_set_template = "SELECT large_modul_set_%(column)s('%(object)s','%(val)s');";
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
		  columns_list = [self.column_template % (myname, mytype) for
					(myname, mytype) in zip(self.names_list, self.types_list)]
		  return self.create_template % (",\n\t".join(columns_list))

	 def gen_data(self, count = 10):
		  random.seed()
		  objects = list()
		  object_names = Names("names.txt")
		  if count > 1000:
				object_names.extend(1000)
				self.names.extend(1000)

		  object_count = count / 10
		  objects_list = object_names.rset(object_count)
		  randdates = Dates()
		  dates = randdates.rlist(count + 1)
		  texts = self.names.rset(count + 1)
		  self.data.append("SELECT startChangeset();")
		  while count > 0:
				r = random.randint(0,15)
				if (len(objects) > 1) and ((r < 15) or (len(objects) >= object_count) ):
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
						  str = self.large_modul_set_template % {'column': col, 'object': obj, 'val': value}
				else:
					 obj_name = objects_list[len(objects)]
					 objects.append(obj_name)
					 str = self.large_modul_add_template % obj_name
				self.data.append(str)
				count = count - 1
		  index = Numbers(len(self.data)).rset(len(self.data)/5)
		  for i in index:
				self.data.insert(i,self.commit_template)
		  self.data.append("SELECT commitChangeset('commit');")
		  return self.data


modul_file = open('../modules/large_modul.sql','w')
modul_file.write('''SET search_path TO production;
CREATE SEQUENCE large_modul_uid START 1;
''')
mg = ModulGenerator(100)
modul_file.write(mg.gen_table())
modul_file.write("\nALTER TABLE large_modul ALTER COLUMN uid SET DEFAULT nextval('production.large_modul_uid'::regclass);")
print '''SET search_path TO api,genproc,history,deska,versioning,production;
SET DATESTYLE TO 'SQL, EUROPEAN';
'''
print "\n".join(mg.gen_data(1000000))
