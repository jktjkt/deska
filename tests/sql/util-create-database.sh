#!/bin/bash

cd `dirname $0`

. ./util-config.sh

PGTAP_FILE="${DESKA_SOURCES}/install/pgtap.sql"
PGPYTHON_FILE="${DESKA_SOURCES}/install/pgpython.sql"

psql -q -U $DESKA_SU -c "CREATE USER ${DESKA_USER};" || die "Create user"
for role in deska_admin deska_user; do
    psql -q -U $DESKA_SU -c "GRANT ${role} TO ${DESKA_USER};" || die "Grant ${role}"
done
psql -q -U $DESKA_SU -c "CREATE DATABASE ${DESKA_DB} OWNER deska_admin;" || die "Create database"

psql -q -U $DESKA_SU -d $DESKA_DB -v ON_ERROR_STOP=1 -f $PGPYTHON_FILE || die "Enable PgPython"

psql -q -U $DESKA_USER -d $DESKA_DB -v ON_ERROR_STOP=1 -f $PGTAP_FILE || die "Installing pgTAP"

