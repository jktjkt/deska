#!/bin/bash

SHMDIR=/dev/shm/deska-${USER}-$$
mkdir ${SHMDIR}
initdb -U postgres -A trust ${SHMDIR}
echo "listen_addresses = ''" >> ${SHMDIR}/postgresql.conf

if [[ -n "${DESKA_TRACE_SQL}" ]]; then
    echo "log_destination = stderr
    log_directory = 'pg_log'
    logging_collector = on
    #log_statement = all
    log_filename = error_log" >> ${SHMDIR}/postgresql.conf
fi
postgres -D ${SHMDIR} -F -k ${SHMDIR} > /dev/null 2> /dev/null &
PGPID=$!
PGHOST=${SHMDIR}
export PGHOST

# wait for the server to start
sleep 2

for role in deska_user deska_admin; do
    psql -q -U postgres -c "CREATE ROLE ${role};"
done

ctest --output-on-failure $@

kill $PGPID

if [[ -n "${DESKA_TRACE_SQL}" ]]; then
    cat "${SHMDIR}/pg_log/error_log"
fi

rm -rf ${SHMDIR}
