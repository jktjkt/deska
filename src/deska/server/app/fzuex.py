from jsonparser import CommandParser
from dbapi import DB

data = list([
	'{"command": "startChangeset", "tag":"TAG"}',
	'{"command": "createObject", "kindName": "boxmodel", "objectName": "HP" , "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"boxmodel", "objectName":"HP", "attributeName": "sizex", "attributeData":5, "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"boxmodel", "objectName":"HP", "attributeName": "insx", "attributeData":5, "tag":"TAG"}',
	'{"command": "createObject", "kindName": "boxmodel", "objectName": "HPsmall" , "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"boxmodel", "objectName":"HPsmall", "attributeName": "sizex", "attributeData":2, "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"boxmodel", "objectName":"HPsmall", "attributeName": "insx", "attributeData":2, "tag":"TAG"}',
	'{"command": "createObject", "kindName": "box", "objectName": "HP1" , "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"box", "objectName":"HP1", "attributeName": "posx", "attributeData":5, "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"box", "objectName":"HP1", "attributeName": "boxmodel", "attributeData":"HP", "tag":"TAG"}',
	'{"command": "createObject", "kindName": "box", "objectName": "HP2" , "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"box", "objectName":"HP2", "attributeName": "box", "attributeData":"HP1", "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"box", "objectName":"HP2", "attributeName": "posx", "attributeData":4, "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"box", "objectName":"HP2", "attributeName": "boxmodel", "attributeData":"HPsmall", "tag":"TAG"}',
	'{"command": "createObject", "kindName": "box", "objectName": "HP3" , "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"box", "objectName":"HP3", "attributeName": "box", "attributeData":"HP1", "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"box", "objectName":"HP3", "attributeName": "posx", "attributeData":2, "tag":"TAG"}',
	'{"command": "setAttribute", "kindName":"box", "objectName":"HP3", "attributeName": "boxmodel", "attributeData":"HPsmall", "tag":"TAG"}',

#	'{"command": "createObject", "kindName": "switch", "objectName": "sw1" , "tag":"TAG"}',
#	'{"command": "setAttribute", "kindName":"switch", "objectName":"sw1", "attributeName": "ports", "attributeData":"^[AB]_[[:digit:]]+$", "tag":"TAG"}',
#	'{"command": "createObject", "kindName": "virtual_hardware", "objectName": "vmware1" , "tag":"TAG"}',
#	'{"command": "createObject", "kindName": "host", "objectName": "host1" , "tag":"TAG"}',
#	'{"command": "setAttribute", "kindName":"host", "objectName":"host1", "attributeName": "virtual_hardware", "attributeData":"vmware1", "tag":"TAG"}',
#	'{"command": "createObject", "kindName": "interface", "objectName": "host1->eth0" , "tag":"TAG"}',
#	'{"command": "setAttribute", "kindName":"interface", "objectName":"host1->eth0", "attributeName": "switch", "attributeData":"sw1", "tag":"TAG"}',
#	'{"command": "setAttribute", "kindName":"interface", "objectName":"host1->eth0", "attributeName": "switch_pos", "attributeData":"A_03", "tag":"TAG"}',
	'{"command": "commitChangeset", "commitMessage": "test message", "tag":"TAG"}'

])

dbargs = {"database": "deska_dev"}
db = DB(**dbargs)

for i in data:
	print "\033[1;32mINPUT:\033[1;m" + i
	jsn = CommandParser(i)
	fn = jsn.getfn()
	args = jsn.getargs()
	print "\033[1;32mOUTPUT:\033[1;m" + "\033[1;33m" + db.run(fn,args) + "\033[1;m"

