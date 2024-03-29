from deska_server_utils.jsonparser import CommandParser
from deska_server_utils.dbapi import DB

data = list([
	'{"command": "startChangeset", "tag":"TAG"}',
	'{"command": "createObject", "kindName": "host", "objectName": "test_host" , "tag":"TAG"}',
	'{"command": "createObject", "kindName": "host", "objectName": "test_host2" , "tag":"TAG"}',
	'{"command": "createObject", "kindName": "host_template", "objectName": "t1" , "tag":"TAG"}',
	'{"command": "createObject", "kindName": "hardware", "objectName": "hw" , "tag":"TAG"}',
	'{"command": "createObject", "kindName": "service", "objectName": "www" , "tag":"TAG"}',
	'{"command": "createObject", "kindName": "service", "objectName": "dhcp" , "tag":"TAG"}',
	'{"command": "createObject", "kindName": "service", "objectName": "ftp" , "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"host", "objectName":"test_host", "attributeName": "service", "attributeData":["www","dhcp"], "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"host", "objectName":"test_host", "attributeName": "template_host", "attributeData":"t1", "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"host", "objectName":"test_host", "attributeName": "hardware", "attributeData": "hw", "tag":"TAG"}',
	'{"command": "deleteObject", "kindName": "hardware", "objectName": "hw" , "tag":"TAG"}',
	'{"command": "setAttributeRemove", "kindName":"host", "objectName":"test_host", "attributeName": "service", "attributeData":"dhcp", "tag":"TAG"}',
	'{"command": "objectData", "kindName": "host", "objectName": "test_host" , "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"hardware", "objectName":"hw", "attributeName": "purchase", "attributeData":"2001-09-09", "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"hardware", "objectName":"hw", "attributeName": "warranty", "attributeData":"2011-09-09", "tag":"TAG"}',
	'{"command": "objectData", "kindName": "host", "objectName": "test_host" , "tag":"TAG"}',
	'{"command": "commitChangeset", "tag":"TAG", "commitMessage":"test"}',
])

dbargs = {}
dbargs["database"] = "deska_dev"
db = DB(dbOptions=dbargs, cfggenBackend="fake", cfggenOptions=None)


for i in data:
	print "\033[1;32mINPUT:\033[1;m" + i
	jsn = CommandParser(i)
	fn = jsn.getfn()
	args = jsn.getargs()
	print "\033[1;32mOUTPUT:\033[1;m" + "\033[1;33m" + db.run(fn,args) + "\033[1;m"

