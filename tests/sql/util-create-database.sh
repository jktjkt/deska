#/bin/bash

cd `dirname $0`

. ./util-config.sh

psql -q -U $DESKA_SU -c "CREATE USER ${DESKA_USER};" || die "Create user"
for role in deska_admin deska_user; do
    psql -q -U $DESKA_SU -c "GRANT ${role} TO ${DESKA_USER};" || die "Grant ${role}"
done
psql -q -U $DESKA_SU -c "CREATE DATABASE ${DESKA_DB} OWNER deska_admin;" || die "Create database"

psql -q -U $DESKA_SU -d $DESKA_DB -v ON_ERROR_STOP=1 -f ../../src/deska/server/db/enable_pgpython.sql || die "Enable PgPython"
