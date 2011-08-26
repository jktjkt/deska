import sys
from multiRef import MultiRef
from connection import Connection

# connect to db
conn = Connection(sys.argv[1],sys.argv[2])

# create schema
schema = MultiRef(conn)

# generate sql code
schema.gen_multiRef(sys.argv[3])
 
