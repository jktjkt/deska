#/bin/bash

cd `dirname $0`

. ./util-config.sh

psql -q -U $DESKA_SU -c "DROP DATABASE IF EXISTS ${DESKA_DB};" || die "Drop database"
psql -q -U $DESKA_SU -c "DROP USER IF EXISTS ${DESKA_USER};" || die "Drop user"
