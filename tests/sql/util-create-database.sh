#/bin/bash

cd `dirname $0`

. ./util-config.sh

psql -q -U $DESKA_SU -c "CREATE USER ${DESKA_USER};" || die "Create user"
for role in deska_admin deska_user; do
    psql -q -U $DESKA_SU -c "GRANT ${role} TO ${DESKA_USER};" || die "Grant ${role}"
done
psql -q -U $DESKA_SU -c "CREATE DATABASE ${DESKA_DB} OWNER deska_admin;" || die "Create database"

psql -q -U $DESKA_SU -d $DESKA_DB -c "BEGIN;
CREATE SCHEMA __python__;
SET search_path TO __python__;
CREATE FUNCTION handler() RETURNS LANGUAGE_HANDLER
 LANGUAGE C AS 'python', 'pl_handler';
CREATE FUNCTION validator(oid) RETURNS VOID
 LANGUAGE C AS 'python', 'pl_validator';
COMMIT;

BEGIN;
SET search_path TO __python__;
CREATE FUNCTION inline(INTERNAL) RETURNS VOID
 LANGUAGE C AS 'python', 'pl_inline';
CREATE LANGUAGE python
 HANDLER handler INLINE inline VALIDATOR validator;
COMMIT;" || die "Enabling PgPython"
