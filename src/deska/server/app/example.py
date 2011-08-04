from jsonparser import CommandParser
from dbapi import DB

data = list([
	'{"command": "freezeView", "tag":"TAG"}',
	'{"command": "unFreezeView", "tag":"TAG"}',
	'{"command": "kindNames", "tag":"TAG"}',
	'{"command": "kindAttributes", "kindName": "vendor" , "tag":"TAG"}',
	'{"command": "kindAttributes", "kindName": "hardware" , "tag":"TAG"}',
	'{"command": "kindAttributes", "kindName": "interface" , "tag":"TAG"}',
	'{"command": "kindAttributes", "kindName": "host", "tag":"TAG"}',
	'{"command": "kindInstances",  "kindName": "vendor" , "tag":"TAG"}',
	'{"command": "kindRelations", "kindName": "interface" , "tag":"TAG"}',
	'{"command": "kindRelations", "kindName": "vendor" , "tag":"TAG"}',
	'{"command": "startChangeset", "tag":"TAG"}',
	'{"command": "createObject", "kindName": "vendor", "objectName": "HP" , "tag":"TAG"}',
	'{"command": "createObject", "kindName": "hardware", "objectName": "hp2" , "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"hardware", "objectName":"hp2", "attributeName": "vendor", "attributeData":"HP", "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"hardware", "objectName":"hp2", "attributeName": "purchase", "attributeData":"10/10/2011", "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"hardware", "objectName":"hp2", "attributeName": "warranty", "attributeData":"10/10/2013", "tag":"TAG"}',
	'{"command": "objectData", "kindName": "hardware", "objectName": "hp2" , "tag":"TAG"}',
	'{"command": "objectData", "kindName": "hardware", "objectName": "noneexisting" , "tag":"TAG"}',
	'{"command": "commitChangeset", "commitMessage": "test message", "tag":"TAG"}',
	'{"command": "pendingChangesets", "tag":"TAG"}',
	'{"command": "dataDifference", "revisionA":"r1", "revisionB":"r2", "tag":"TAG"}'
])

dbargs = {"database": "deska_dev"}
db = DB(**dbargs)

for i in data:
	print "\033[1;32mINPUT:\033[1;m" + i
	jsn = CommandParser(i)
	fn = jsn.getfn()
	args = jsn.getargs()
	print "\033[1;32mOUTPUT:\033[1;m" + "\033[1;33m" + db.run(fn,args) + "\033[1;m"

