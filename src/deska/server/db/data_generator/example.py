from randdom import Numbers, Names, IPv4s, Dates, Macs

num = Numbers(10000)
print num.rset(5)
num = Numbers(1,10)
print num.ritem()
print num.ritem()
print num.ritem()

vendor = Names("names.txt")
print vendor.rset(10)

# now we can use ritem / rset on result of rset
subvendor = vendor.rset(6)
print subvendor
ssvendor = subvendor.rset(4)
print ssvendor
print ssvendor.ritem()
print ssvendor.rset(2)


ip = IPv4s()
ip.setBlock("A",10)
ip.setBlock("B",0)
ip.setBlock("C",10,2)
ip.setBlock("D",0,25)
print ip.ritem()
print ip.ritem()
print ip.rset(10)

date = Dates()
print date.ritem()
print date.ritem()
print date.ritem()

date = Dates(2011,3)
print date.ritem()
print date.ritem()
print date.ritem()

mac = Macs()
print mac.ritem()
print mac.ritem()


