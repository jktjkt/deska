#!/bin/bash

SHMDIR=/dev/shm/deska-${USER}-$$
mkdir ${SHMDIR}
initdb -U postgres -A trust ${SHMDIR}
echo "listen_addresses = ''
unix_socket_directory = '${SHMDIR}'
" >> ${SHMDIR}/postgresql.conf

if [[ -n "${DESKA_TRACE_SQL}" ]]; then
    echo "log_destination = stderr
    log_directory = 'pg_log'
    logging_collector = on
    #log_statement = all
    log_filename = error_log" >> ${SHMDIR}/postgresql.conf
    CTL_LOG="${SHMDIR}/pg_ctl_log"
else
    CTL_LOG=/dev/null
fi
PGHOST=${SHMDIR}
export PGHOST
PGUSER=postgres pg_ctl start -D ${SHMDIR} -w -l ${CTL_LOG} -o "-F"

for role in deska_user deska_admin; do
    psql -q -U postgres -c "CREATE ROLE ${role};"
done

ctest --output-on-failure $@

pg_ctl stop -D ${SHMDIR}

if [[ -n "${DESKA_TRACE_SQL}" ]]; then
    cat "${SHMDIR}/pg_log/error_log"
    cat $CTL_LOG
fi

rm -rf ${SHMDIR}
