from unittest import TestLoader,TestResult,TextTestRunner,TestSuite
import sys
import os

def getSuiteFromModule(module):
	hack_code = '''
import {0}
suite = TestLoader().loadTestsFromModule({0})'''.format(module)
	exec(hack_code)
	return suite

def runAll():
	# predent env and argv settings if not set
	try:
		server = sys.argv[1]
	except:
		sys.argv.append("../../src/deska/server/app/deska_server.py")
	if "DESKA_DB" not in os.environ:
		os.environ["DESKA_DB"] = "deska_dev"

	ls = os.popen("ls test_*.py")
	files = ls.read().split()
	modules = [m.split(".")[0] for m in files]
	suites = [getSuiteFromModule(m) for m in modules]
	return TestSuite(suites)

def runOne():
	return getSuiteFromModule(module)


try:
	# when argv[2] exist, super cmake test tool is calling us
	module = sys.argv[2]
	function = runOne
except:
	# called with no parameters, run all tests in this directory
	function = runAll

suite = function()

# run tests now
from testutils import JsonBuilder,DeskaRunner
TextTestRunner(verbosity=2).run(suite)
