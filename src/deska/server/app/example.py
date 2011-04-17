from apijson import Jsn

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
	'{"command": "createObject", "kindName": "vendor", "objectName": "DELL" }',
	'{"command": "createObject", "kindName": "hardware", "objectName": "hp2" }',
	'{"command": "setAttribute", "kindName":"hardware", "objectName":"hp2", "attributeName": "vendor", "attributeData":"HP"}',
	'{"command": "setAttribute", "kindName":"hardware", "objectName":"hp2", "attributeName": "purchase", "attributeData":"10/10/2011"}',
	'{"command": "objectData", "kindName": "hardware", "objectName": "hp2" }',
	'{"command": "detachFromCurrentChangeset", "message": "test message"}',
	'{"command": "pendingChangesets"}'
])

#data = list(['{"command": "pendingChangesets"}'])

for i in data:
	print "\033[1;32mINPUT:\033[1;m" + i
	jsn = Jsn(i)
	print "\033[1;32mOUTPUT:\033[1;m" + "\033[1;33m" + jsn.process() + "\033[1;m"

