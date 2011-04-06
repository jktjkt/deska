from apijson import Jsn

data = list([
	'{"command": "kindNames"}',
	'{"command": "kindAttributes", "kindAttributes": { "kindName": "vendor"} }',
	'{"command": "kindAttributes", "kindAttributes": { "kindName": "hardware"} }',
	'{"command": "kindAttributes", "kindAttributes": { "kindName": "interface"} }',
	'{"command": "kindAttributes", "kindAttributes": { "kindName": "host"} }',
	'{"command": "kindInstances", "kindInstances": { "kindName": "vendor"} }',
	'{"command": "startChangeset"}',
	'{"command": "createObject", "createObject": { "kindName": "vendor", "objectName": "DELL"} }',
	'{"command": "createObject", "createObject": { "kindName": "hardware", "objectName": "hp2"} }',
	'{"command": "setAttribute", "setAttribute": {"kindName":"hardware", "objectName":"hp2", "attributeName": "vendor", "Value":"HP"}}',
	'{"command": "objectData", "objectData": { "kindName": "hardware", "objectName": "hp2"} }',
	'{"command": "detachFromCurrentChangeset", "detachFromCurrentChangeset": { "message": "test message"}}',
	'{"command": "pendingChangesets"}'
])

for i in data:
	print "\033[1;32mINPUT:\033[1;m" + i
	jsn = Jsn(i)
	print "\033[1;32mOUTPUT:\033[1;m" + "\033[1;33m" + jsn.process() + "\033[1;m"

