from apijson import Jsn

data = list([
	'{"command": "startChangeset"}',
	'{"command": "kindAttributes", "kindAttributes": { "kindName": "vendor"} }',
	'{"command": "createObject", "createObject": { "kindName": "vendor", "objectName": "DELL"} }',
	'{"command": "createObject", "createObject": { "kindName": "hardware", "objectName": "hp2"} }',
	'{"command": "setAttribute", "setAttribute": {"kindName":"hardware", "objectName":"hp2", "attributeName": "vendor", "Value":"HP"}}',
	'{"command": "objectData", "objectData": { "kindName": "hardware", "objectName": "hp2"} }',
	'{"command": "detachFromCurrentChangeset", "detachFromCurrentChangeset": { "message": "test message"}}',
	'{"command": "pendingChangesets"}'
])

for i in data:
	print "\033[1;31mINPUT:\033[1;m"
	print i
	jsn = Jsn(i)
	print "\033[1;31mOUTPUT:\033[1;m"
	print jsn.process()

