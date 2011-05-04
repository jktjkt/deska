import psycopg2
import sys

class Connection:
	def __init__(self):
		conn = psycopg2.connect(database=sys.argv[1], user=sys.argv[2])
		try:
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

