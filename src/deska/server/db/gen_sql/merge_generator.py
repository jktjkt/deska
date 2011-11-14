#!/usr/bin/python2

import sys
from merge import Composition
from connection import Connection

# connect to db
conn = Connection(sys.argv[1],sys.argv[2])

# create schema
schema = Composition(conn)

# generate sql code
schema.gen_composition(sys.argv[3])
 
