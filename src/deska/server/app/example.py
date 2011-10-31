from jsonparser import CommandParser
from dbapi import DB

data = list([
	'{"command": "startChangeset", "tag":"TAG"}',
	'{"command": "createObject", "kindName": "host", "objectName": "test_host" , "tag":"TAG"}',
	'{"command": "createObject", "kindName": "service", "objectName": "www" , "tag":"TAG"}',
	'{"command": "createObject", "kindName": "service", "objectName": "dhcp" , "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"host", "objectName":"test_host", "attributeName": "service", "attributeData":["www","dhcp"], "tag":"TAG"}',
	'{"command": "setAttributeRemove", "kindName":"host", "objectName":"test_host", "attributeName": "service", "attributeData":"dhcp", "tag":"TAG"}',
	'{"command": "objectData", "kindName": "host", "objectName": "test_host" , "tag":"TAG"}',
	'{"command": "setAttributeInsert", "kindName":"host", "objectName":"test_host", "attributeName": "service", "attributeData":"dhcp", "tag":"TAG"}',
	'{"command": "objectData", "kindName": "host", "objectName": "test_host" , "tag":"TAG"}',
	'{"command": "resolvedObjectData", "kindName": "host", "objectName": "test_host" , "tag":"TAG"}',
	'{"command": "resolvedObjectDataWithOrigin", "kindName": "host", "objectName": "test_host" , "tag":"TAG"}',
	'{"command": "multipleObjectData", "kindName": "host", "tag":"TAG"}',
	'{"command": "multipleResolvedObjectData", "kindName": "host", "tag":"TAG"}',
	'{"command": "multipleResolvedObjectDataWithOrigin", "kindName": "host", "tag":"TAG"}'
])

dbargs = {}
dbargs["database"] = "deska_dev"
db = DB(dbOptions=dbargs, cfggenBackend=None, cfggenOptions=None)


for i in data:
	print "\033[1;32mINPUT:\033[1;m" + i
	jsn = CommandParser(i)
	fn = jsn.getfn()
	args = jsn.getargs()
	print "\033[1;32mOUTPUT:\033[1;m" + "\033[1;33m" + db.run(fn,args) + "\033[1;m"

