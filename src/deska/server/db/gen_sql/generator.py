#!/usr/bin/python2

from schema import Schema
from connection import Connection

# connect to db
conn = Connection()

# create schema
schema = Schema(conn)

# generate sql code
schema.gen_schema('gen_schema.sql')
