#/bin/bash
iconv -f utf16 -t utf8 aliases.txt > dbo.Aliases.Table.sql
iconv -f utf16 -t utf8 deleteditems.txt > dbo.DeletedItems.Table.sql 
iconv -f utf16 -t utf8 hardware.txt > dbo.Hardware.Table.sql
iconv -f utf16 -t utf8 historyevents.txt > dbo.HistoryEvents.Table.sql 
iconv -f utf16 -t utf8 interfaces.txt > dbo.Interfaces.Table.sql
iconv -f utf16 -t utf8 machines.txt > dbo.Machines.Table.sql 
iconv -f utf16 -t utf8 networks.txt > dbo.Networks.Table.sql 
iconv -f utf16 -t utf8 racks.txt > dbo.Racks.Table.sql
iconv -f utf16 -t utf8 services.txt > dbo.Services.Table.sql 
iconv -f utf16 -t utf8 servicestomachines.txt > dbo.ServicesToMachines.Table.sql 
iconv -f utf16 -t utf8 vendors.txt > dbo.Vendors.Table.sql
