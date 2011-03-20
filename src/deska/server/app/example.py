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

data = '{"command": "commitChangeset"}'
jsn = Jsn(data)
print jsn.process()

