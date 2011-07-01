#!/usr/bin/python2

import sys
from template import Template
from connection import Connection

# connect to db
conn = Connection(sys.argv[1],sys.argv[2])

# create schema
schema = Template(conn)

# generate sql code
schema.gen_templates(sys.argv[3])
