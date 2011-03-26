from apijson import Jsn

data = '{"command": "startChangeset"}'
jsn = Jsn(data)
print jsn.process()

data = '{"command": "kindAttributes", "kindAttributes": { "kindName": "vendor"} }'
jsn = Jsn(data)
print jsn.process()

#data = '{"command": "setAttribute", "setAttribute": {"kindName":"vendor", "objectName":"DELL", "attributeName": "name", "Value":"xxx"}}'
data = '{"command": "createObject", "createObject": { "kindName": "vendor", "objectName": "DELL"} }'
jsn = Jsn(data)
print jsn.process()

data = '{"command": "createObject", "createObject": { "kindName": "hardware", "objectName": "hp2"} }'
jsn = Jsn(data)
print jsn.process()

data = '{"command": "setAttribute", "setAttribute": {"kindName":"hardware", "objectName":"hp2", "attributeName": "vendor", "Value":"HP"}}'
jsn = Jsn(data)
print jsn.process()

data = '{"command": "objectData", "objectData": { "kindName": "hardware", "objectName": "hp2"} }'
jsn = Jsn(data)
print jsn.process()

#data = '{"command": "commitChangeset"}'
data = '{"command": "abortCurrentChangeset"}'
jsn = Jsn(data)
print jsn.process()

