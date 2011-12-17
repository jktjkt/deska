#!/bin/bash

cd `dirname $0`

if [[ x"$1" == "x--with-git" ]]; then
    export DESKA_WITH_GIT=1
    shift
fi

. ./util-config.sh

while [[ -n "${1}" ]]; do
    case "${1}" in
        --dbscheme)
            shift
            export DESKA_SCHEME="${1}"
            shift
            ;;
        *)
            break
            ;;
    esac
done

if [[ -z "${DESKA_SCHEME}" ]]; then
    export DESKA_SCHEME=demo
fi

TESTMODE="${1}"
TESTCASE="${2}"

if [[ -z "${TESTMODE}" ]]; then
    die "SQL/DBAPI/whatever selection"
fi

if [[ -z $TESTCASE ]]; then
    die "No test case to run. Execution"
fi

# We might be asked to run in a different scheme, so let's make sure the DB is gone
psql -q -U $DESKA_SU -c "DROP DATABASE IF EXISTS ${DESKA_DB};"
if [[ $? -ne 0 ]]; then
    # Dropping the DB occasionally fails on EL5, so let's try it multiple times
    sleep 5
    psql -q -U $DESKA_SU -c "DROP DATABASE IF EXISTS ${DESKA_DB};" || die "Drop database"
fi

if [[ -z ${DESKA_TABLESPACE} ]]; then
    TABLESPACE=""
else
    TABLESPACE=" WITH TABLESPACE = ${DESKA_TABLESPACE}"
fi
psql -q -U $DESKA_SU -c "CREATE DATABASE ${DESKA_DB} ${TABLESPACE} OWNER deska_admin;" || die "Create database"


DESKA_SERVER_SIDE_DESTINATION="${DESKA_GENERATED_FILES}/target-${DESKA_SCHEME}"

DESKA_SQL_DB_TO_RESTORE="${DESKA_TEST_VANILLA_DB}/${DESKA_SCHEME}"

if [[ -f "${DESKA_SQL_DB_TO_RESTORE}" ]]; then
    # Restore the DB quickly
    pg_restore -U "${DESKA_SU}" -d "${DESKA_DB}" --single-transaction --exit-on-error "${DESKA_SQL_DB_TO_RESTORE}" \
        || die "Restoring the DB"
else
    # The users might be already there, so we do not want to fail in that case
    psql -q -U $DESKA_SU -c "CREATE USER ${DESKA_USER};"
    psql -q -U $DESKA_SU -c "CREATE USER ${DESKA_ADMIN};"
    for role in deska_admin deska_user; do
        psql -q -U $DESKA_SU -c "GRANT ${role} TO ${DESKA_ADMIN};" || die "Grant ${role}"
    done
    psql -q -U $DESKA_SU -c "GRANT deska_user TO ${DESKA_USER};" || die "Grant ${role}"

    PGTAP_FILE="${DESKA_SOURCES}/install/pgtap.sql"
    PGPYTHON_FILE="${DESKA_SOURCES}/install/pgpython.sql"
    psql -q -U $DESKA_SU -d $DESKA_DB -v ON_ERROR_STOP=1 -f $PGPYTHON_FILE || die "Enable PgPython"
    psql -q -U $DESKA_ADMIN -d $DESKA_DB -v ON_ERROR_STOP=1 -f $PGTAP_FILE || die "Installing pgTAP"

    pushd "${DESKA_SOURCES}/install"
    ./deska-deploy-database.sh -U "${DESKA_SU}" -d "${DESKA_DB}" -t "${DESKA_SERVER_SIDE_DESTINATION}" || die "Running deploy-database"
    popd

    # Now take a DB backup for the following executions
    pg_dump -U "${DESKA_SU}" -f "${DESKA_SQL_DB_TO_RESTORE}" \
        --format=custom "${DESKA_DB}" > /dev/null \
        || die "Cannot take a DB backup for further tests"
fi

if [[ -n "${DESKA_WITH_GIT}" ]]; then
    . ./util-manage-git.sh || die "git stuff"
    rm -rf "${DESKA_GENERATED_FILES}/git"
    deska_init_git "${DESKA_GENERATED_FILES}/git"
    export GIT_PYTHON_TRACE=full
fi

export DESKA_SOURCES

# clean the deska_server.log
> deska_server.log

case "${TESTMODE}" in
    sql)
        if [[ ! -f $TESTCASE ]]; then
            die "Locating SQL testcase"
        fi

        pg_prove -U $DESKA_ADMIN -d $DESKA_DB $TESTCASE
        TEST_RESULT=$?
        ;;
    dbapi)
        export DESKA_USER
        export DESKA_DB
        python ${DESKA_SOURCES}/tests/dbapi-application/testdbapi.py ${DESKA_SOURCES}/src/deska/server/app/deska-server $TESTCASE
        TEST_RESULT=$?
        ;;
    run)
        export DESKA_USER
        export DESKA_DB
        ${TESTCASE}
        TEST_RESULT=$?
        ;;
    *)
        die "Unknown test"
esac

if [[ ${TEST_RESULT} -ne 0 ]]; then
    cat deska_server.log
    die "Test failed"
fi
