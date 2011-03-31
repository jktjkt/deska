#!/usr/bin/python

from apijson import Jsn
import sys

while True:
	line = sys.stdin.readline()
	if not line:
		break
	jsn = Jsn(line)
	sys.stdout.write(str(jsn.process()))

