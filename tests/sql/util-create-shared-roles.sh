#!/bin/bash

cd `dirname $0`

. ./util-config.sh

for role in deska_admin deska_user; do
    psql -q -U $DESKA_SU -c "CREATE ROLE ${role};" || die "Create role ${role}"
done
