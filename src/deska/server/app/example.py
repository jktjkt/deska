from jsonparser import CommandParser
from dbapi import DB

data = list([
	'{"command": "kindNames"}',
	'{"command": "kindAttributes", "kindName": "vendor" }',
	'{"command": "kindAttributes", "kindName": "hardware" }',
	'{"command": "kindAttributes", "kindName": "interface" }',
	'{"command": "kindAttributes", "kindName": "host"}',
	'{"command": "kindInstances",  "kindName": "vendor" }',
	'{"command": "kindRelations", "kindName": "interface" }',
	'{"command": "kindRelations", "kindName": "vendor" }',
	'{"command": "startChangeset"}',
	'{"command": "createObject", "kindName": "vendor", "objectName": "HP" }',
	'{"command": "createObject", "kindName": "hardware", "objectName": "hp2" }',
	'{"command": "setAttribute", "kindName":"hardware", "objectName":"hp2", "attributeName": "vendor", "attributeData":"HP"}',
	'{"command": "setAttribute", "kindName":"hardware", "objectName":"hp2", "attributeName": "purchase", "attributeData":"10/10/2011"}',
	'{"command": "objectData", "kindName": "hardware", "objectName": "hp2" }',
	'{"command": "detachFromCurrentChangeset", "message": "test message"}',
	'{"command": "pendingChangesets"}',
	'{"command": "listVersions"}'
])

#data = list(['{"command": "pendingChangesets"}'])
#data = list(['{"command": "setAttribute", "kindName":"hardware", "objectName":"hp2", "attributeName": "vendor", "attributeData":"HP"}'])

db = DB("deska_dev")

for i in data:
	print "\033[1;32mINPUT:\033[1;m" + i
	jsn = CommandParser(i)
	fn = jsn.getfn()
	args = jsn.getargs()
	print "\033[1;32mOUTPUT:\033[1;m" + "\033[1;33m" + db.run(fn,args) + "\033[1;m"

