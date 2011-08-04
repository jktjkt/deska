#!/usr/bin/python2

import sys
from merge import Merge
from connection import Connection

# connect to db
conn = Connection(sys.argv[1],sys.argv[2])

# create schema
schema = Merge(conn)

# generate sql code
schema.gen_merge(sys.argv[3])
 
