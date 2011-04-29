#/bin/bash

. ./util-config.sh

psql -q -U $SU -c "DROP USER IF EXISTS ${USER};" || die "Drop user"
psql -q -U $SU -c "DROP DATABASE IF EXISTS ${DB};" || die "Drop database"
