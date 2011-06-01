from apiUtils import *

j = [
    kindNames().returns(
        AnyOrderList(('interface', 'vendor', 'host', 'hardware'))),
    kindAttributes("interface").returns(
        {"note": "string", "ip": "string", "host": "int", "mac": "string"}),
    kindRelations("hardware").returns([])
]
