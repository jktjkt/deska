from apiUtils import *

def imperative(r):
    objectNames = set(["test1", "test2", "test3"])
    firstKI = r.c(kindInstances("vendor"))
    objectNames = objectNames - set(firstKI)

    r.c(startChangeset())
    for obj in objectNames:
        r.c(createObject("vendor", obj))

    changesetKI = r.c(kindInstances("vendor"))
    # FIXME: subset



#from testutils import js,updateRev
#from deskatest import DeskaTest

class kindInstancesTest:#(DeskaTest):
	'''test list actual kindInstances'''
	def listKI(self,kind,revision = None):
		res = self.command(js.kindInstances,kind,revision)
		self.OK(res.OK)
		return res.result()

	def addKI(self,kind,name):
		'''add object of kind named name'''
		res = self.command(js.createObject,kind,name)
		self.OK(res.OK)

	def test_vendorKI(self):
		'''test kindInstances on with vendor kind'''
		objectNames = set(["test1", "test2", "test3"])
		firstKI = self.listKI("vendor")
		# only add object that are not already in db
		objectNames = objectNames - set(firstKI)

		res = self.command(js.startChangeset)
		self.OK(res.OK)
		for obj in objectNames:
			self.addKI("vendor",obj)

		changesetKI = self.listKI("vendor")
		# changesetKI should containt whole objectNames
		self.subset(objectNames, changesetKI)

		res = self.command(js.commitChangeset,"test")
		self.OK(res.OK)
		revision = res.result()
		# test KI after commit
		newKI = self.listKI("vendor")
		self.assertTrue(objectNames <= set(newKI))

		# and test revision parameter
		revision = updateRev(revision,-1)
		revNewKI = self.listKI("vendor",revision)
		self.equals(revNewKI,firstKI)
