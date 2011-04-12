import psycopg2

class Connection:
	def __init__(self):
		try:
			conn = psycopg2.connect("dbname='deska_dev'");
			self.mark = conn.cursor()

		except:
			print "I am unable to connect to the database"

	def execute(self,statement):
		self.mark.execute(statement)
		try:
			return self.mark.fetchall()
		except:
			return ""

