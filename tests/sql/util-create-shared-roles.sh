#/bin/bash

. ./util-config.sh

for role in deska_admin deska_user; do
    psql -q -U $SU -c "CREATE ROLE ${role};" || die "Create role ${role}"
done
