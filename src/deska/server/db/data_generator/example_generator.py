from randdom import Numbers, Names, IPv4s, Dates, Macs

n = 10
# init names generator
names = Names("names.txt")

# gen set of N random (and unique) names
vendor = names.rset(n)

# generate list of select strings
str = map("SELECT vendor_add('{0}');".format, vendor)
print "\n".join(str)

# get set of N random and unique names
hw = names.rset(n)
# generate list of select strings
str = map("SELECT hardware_add('{0}');".format, hw)
print "\n".join(str)

# get list on N random (not unique) names from set vendor
hw_vendor = vendor.rlist(n)
# generate list of select strings from hw and hw_vendor lists
str = map("SELECT hardware_set_vendor('{0}','{1}');".format, hw, hw_vendor)
print "\n".join(str)

# init dates generator (default params)
dates = Dates()

# get list (not unique) of N dates
date = dates.rlist(n)
str = map("SELECT hardware_set_purchase('{0}','{1}');".format, hw, date)
print "\n".join(str)

# init dates generator with new params
dates = Dates(2011,4)
# get list (not unique) of N dates
date = dates.rlist(n)
str = map("SELECT hardware_set_warranty('{0}','{1}');".format, hw, date)
print "\n".join(str)
