from randdom import Numbers, Names

num = Numbers(10000)
print num.rset(5)
num = Numbers(1,10)
print num.ritem()
print num.ritem()
print num.ritem()

vendor = Names("names.txt")
print vendor.rset(10)

