#!/bin/bash

SHMDIR=/dev/shm/deska-${USER}
mkdir ${SHMDIR}
initdb -U postgres -A trust ${SHMDIR}
echo "listen_addresses = ''" >> ${SHMDIR}/postgresql.conf
postgres -D ${SHMDIR} -F -k ${SHMDIR} > /dev/null 2> /dev/null &
PGPID=$!
PGHOST=${SHMDIR}
export PGHOST

# wait for the server to start
sleep 1

./tests/sql/util-create-shared-roles.sh

ctest --output-on-failure

kill $PGPID
rm -rf ${SHMDIR}
