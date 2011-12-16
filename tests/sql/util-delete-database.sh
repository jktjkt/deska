#!/bin/bash

cd `dirname $0`

. ./util-config.sh

# Dropping the DB occasionally fails on EL5, so let's try it multiple times
psql -q -U $DESKA_SU -c "DROP DATABASE IF EXISTS ${DESKA_DB};"
if [[ $? -ne 0 ]]; then
    sleep 5
    psql -q -U $DESKA_SU -c "DROP DATABASE IF EXISTS ${DESKA_DB};" || die "Drop database"
fi

psql -q -U $DESKA_SU -c "DROP USER IF EXISTS ${DESKA_USER};" || die "Drop user"
psql -q -U $DESKA_SU -c "DROP USER IF EXISTS ${DESKA_ADMIN};" || die "Drop user"
