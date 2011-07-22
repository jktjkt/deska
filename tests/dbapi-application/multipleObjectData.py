'''Test multipleObjectData with revisions with pre-existing data'''

from apiUtils import *

expectedHardwareData = {
	"hw1": {"vendor": "vendor1", "warranty": "2010-10-10", "purchase": "2008-10-10", "cpu_num": None, "ram": None, "note": None, "template": None},
	"hw2": {"vendor": "vendor1", "warranty": "2006-06-06", "purchase": "2010-06-06", "cpu_num": None, "ram": None, "note": None, "template": None}
}
expectedHardwareData2 = {
	"hw1": {"vendor": "vendor1", "warranty": "2010-10-10", "purchase": "2008-10-10", "cpu_num": None, "ram": None, "note": None, "template": None},
	"hw2": {"vendor": "vendor1", "warranty": "2006-06-06", "purchase": "2010-06-06", "cpu_num": None, "ram": None, "note": None, "template": None},
	"hw3": {"vendor": "vendor2", "warranty": "2010-10-10", "purchase": "2012-10-10", "cpu_num": None, "ram": None, "note": None, "template": None},
	"hw4": {"vendor": "vendor2", "warranty": "2010-10-10", "purchase": "2012-10-10", "cpu_num": None, "ram": None, "note": None, "template": None}
}

def imperative(r):
	doStuff(r)
	doStuff_embed(r)
	
def doStuff(r):
	vendorNames = set(["vendor1", "vendor2"])
	presentVendors = set(r.c(kindInstances("vendor")))
	vendorNames = vendorNames - presentVendors

	r.c(startChangeset())
	for obj in vendorNames:
		r.cvoid(createObject("vendor", obj))
        
	hardwareNames = set(["hw1", "hw2", "hw3"])
	presentHW = set(r.c(kindInstances("hardware")))
	hardwareNames = hardwareNames - presentHW

	for obj in hardwareNames:
		r.cvoid(createObject("hardware", obj))

	r.cvoid(setAttribute("hardware", "hw1", "vendor", "vendor1"))
	r.cvoid(setAttribute("hardware", "hw1", "purchase", "2008-10-10"))
	r.cvoid(setAttribute("hardware", "hw1", "warranty", "2010-10-10"))
	r.cvoid(setAttribute("hardware", "hw2", "vendor", "vendor1"))
	r.cvoid(setAttribute("hardware", "hw2", "purchase", "2006-06-06"))
	r.cvoid(setAttribute("hardware", "hw2", "warranty", "2010-06-06"))

	hardwareData = r.c(multipleObjectData("hardware"))
	r.assertEqual(hardwareData, expectedHardwareData)
	revision = r.c(commitChangeset("test"))
	
	hardwareData = r.c(multipleObjectData("hardware", revision))
	r.assertEqual(hardwareData, expectedHardwareData)

	hardwareNames = set(["hw3","hw4"])
	presentHW = set(r.c(kindInstances("hardware")))
	hardwareNames = hardwareNames - presentHW

	r.c(startChangeset())
	
	for obj in hardwareNames:
		r.cvoid(createObject("hardware", obj))
	
	r.cvoid(setAttribute("hardware", "hw3", "vendor", "vendor2"))
	r.cvoid(setAttribute("hardware", "hw3", "purchase", "2010-10-10"))
	r.cvoid(setAttribute("hardware", "hw3", "warranty", "2012-10-10"))
	r.cvoid(setAttribute("hardware", "hw4", "vendor", "vendor2"))
	r.cvoid(setAttribute("hardware", "hw4", "purchase", "2010-10-10"))
	r.cvoid(setAttribute("hardware", "hw4", "warranty", "2012-10-10"))
	r.c(commitChangeset("test2"))
