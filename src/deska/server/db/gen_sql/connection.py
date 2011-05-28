import psycopg2

class Connection:
	def __init__(self,db,dbuser):
		try:
			conn = psycopg2.connect(database=db, user=dbuser)
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

