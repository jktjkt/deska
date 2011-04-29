#/bin/bash

. util-config.sh

psql -q -U $SU -c "CREATE USER ${USER};" || die "Create user"
for role in deska_admin deska_user; do
    psql -q -U $SU -c "GRANT ${role} TO ${USER};" || die "Grant ${role}"
done
psql -q -U $SU -c "CREATE DATABASE ${DB} OWNER deska_admin;" || die "Create database"
psql -q -U $SU -d $DB -c "CREATE LANGUAGE plpythonu;" || die "Create language"
