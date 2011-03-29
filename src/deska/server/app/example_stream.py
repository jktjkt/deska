import json

jsn = json.JSONDecoder()


# incomplete json:
data =	'{"command": "startChanges'
try:
	obj, end = jsn.raw_decode(data)
except ValueError as (text):
	print text
	# need to buffer more input, determine if it can help 

# more data in buffer
data =	'{"command": "startChangeset"}{"command": "kindAttributes", "kindAttributes": { "kindName": "vendor"} }{"command": "createObject", "createObject": { "kindName": "vendor", "objectName": "DELL"} }'
obj, end = jsn.raw_decode(data)
print obj
while end < len(data):
	print "more data in buffer"
	data = data[end:len(data)]
	obj, end = jsn.raw_decode(data)
	print obj
	
