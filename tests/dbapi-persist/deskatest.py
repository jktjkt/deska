import json
import unittest
from testutils import JsonParser,tr

class DeskaTest(unittest.TestCase):
	def command(self,cmd,*args):
		jsn = cmd(*args)
		cmd = JsonParser(jsn)
		res = tr.command(jsn)
		jp = JsonParser(res)
		print jsn
		print res
		self.assertEqual(jp["response"], cmd["command"])
		return jp
	
	def commandList(self,cmdlist):
		jp = ''
		for cmd in cmdlist:
			jsn = json.dumps(cmd)
			res = tr.command(jsn)
			jp = JsonParser(res)
			print jsn
			print res
			self.assertEqual(jp["response"], cmd["command"])
			self.OK(jp.OK)
		return jp

	def OK(self,func):
		self.assertTrue(func())


