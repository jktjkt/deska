time echo '{"command": "kindInstances", "kindName":"hardware"}' | python ../app/deska_server.py > out0
time echo '{"command": "jsnKindInstances", "kindName":"hardware"}' | python ../app/deska_server.py > out1

time echo '{"command": "jsnVersions"}' | python ../app/deska_server.py > out2
time echo '{"command": "listVersions"}' | python ../app/deska_server.py > out3

exit

time echo '{"command": "large_modul_init_diff", "from":'$1', "to":'$2'}
{"command": "large_modul_diff_created"}
{"command": "large_modul_diff_set_attributes"}
{"command": "large_modul_diff_deleted"}' | python ../app/deska_server.py > out1

time echo '{"command": "lm_diff", "from":'$1', "to":'$2'}' | python ../app/deska_server.py > out2
time echo '{"command": "lm_diff_json", "from":'$1', "to":'$2'}' | python ../app/deska_server.py > out3
