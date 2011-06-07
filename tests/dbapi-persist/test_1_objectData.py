from testutils import js,updateRev
from deskatest import DeskaTest

class objectDataTest(DeskaTest):
	'''test list actual kindInstances'''
	def objectData(self,kind,name,revision = None):
		res = self.command(js.objectData,kind,name,revision)
		self.OK(res.OK)
		return res.result()
	
	def delHardware(self,obj):
		'''delete object of kind named name'''
		res = self.command(js.startChangeset)
		self.OK(res.OK)
		
		res = self.command(js.deleteObject,"hardware",obj)
		self.OK(res.OK)
		# commit
		res = self.command(js.commitChangeset,"hardware add")
		self.OK(res.OK)
		return res.result()
		
	def setHardware(self,obj,data):
		'''add object of kind named name'''
		res = self.command(js.setAttribute,"hardware",obj,"vendor",data[0])
		self.OK(res.OK)
		res = self.command(js.setAttribute,"hardware",obj,"purchase",data[1])
		self.OK(res.OK)
		res = self.command(js.setAttribute,"hardware",obj,"warranty",data[2])
		self.OK(res.OK)

	def addHardware(self,obj,data):
		'''add object of kind named name'''
		res = self.command(js.startChangeset)
		self.OK(res.OK)
		
		res = self.command(js.createObject,"hardware",obj)
		self.OK(res.OK)
		res = self.command(js.setAttribute,"hardware",obj,"vendor",data[0])
		self.OK(res.OK)
		res = self.command(js.setAttribute,"hardware",obj,"purchase",data[1])
		self.OK(res.OK)
		res = self.command(js.setAttribute,"hardware",obj,"warranty",data[2])
		self.OK(res.OK)
		# commit
		res = self.command(js.commitChangeset,"hardware add")
		self.OK(res.OK)
		return res.result()

	def test_hardware(self):
		'''test objectData with hardware kind'''
		objectName = "HP"
		data = ["test1","2011-09-09","2012-09-09"]
		res = self.command(js.objectData,"hardware",objectName)
		if res.OK():
			#it exists, delete it
			self.delHardware(objectName)
		revision = self.addHardware(objectName,data)
			
		firstOD = self.objectData("hardware",objectName)

		# test OD in changeset
		res = self.command(js.startChangeset)
		self.OK(res.OK)
		newdata = ["test1","2011-01-09","2013-09-09"]
		newrevision = self.setHardware(objectName,newdata)
		changesetOD = self.objectData("hardware",objectName)
		# commit
		res = self.command(js.commitChangeset,"hardware set")
		self.OK(res.OK)

		# after commit
		# compare before and after commit results
		newOD = self.objectData("hardware",objectName)
		self.assertEqual(newOD,changesetOD)
		
		# and test revision parameter
		revNewOD = self.objectData("hardware",objectName,revision)
		self.assertEqual(revNewOD,firstOD)
