#!/usr/bin/python2

import sys
from schema import Schema
from connection import Connection

# connect to db
conn = Connection(sys.argv[1],sys.argv[2])

# create schema
schema = Schema(conn)

# generate sql code
schema.gen_schema(sys.argv[3])
