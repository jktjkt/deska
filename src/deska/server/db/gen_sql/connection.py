import psycopg2
import sys

class Connection:
	def __init__(self):
		try:
			conn = psycopg2.connect("dbname='{0}' user='{1}'".format(
				sys.argv[1], sys.argv[2]));
			self.mark = conn.cursor()

		except:
			print "I am unable to connect to the database"
			sys.exit(1)

	def execute(self,statement):
		self.mark.execute(statement)
		try:
			return self.mark.fetchall()
		except:
			return ""

