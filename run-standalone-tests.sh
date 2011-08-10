#!/bin/bash

SHMDIR=/dev/shm/deska-${USER}-$$
mkdir ${SHMDIR}
initdb -U postgres -A trust ${SHMDIR}
echo "listen_addresses = ''" >> ${SHMDIR}/postgresql.conf
postgres -D ${SHMDIR} -F -k ${SHMDIR} > /dev/null 2> /dev/null &
PGPID=$!
PGHOST=${SHMDIR}
export PGHOST

# wait for the server to start
sleep 1

for role in deska_user deska_admin; do
    psql -q -U postgres -c "CREATE ROLE ${role};"
done

ctest --output-on-failure $@

kill $PGPID
rm -rf ${SHMDIR}
